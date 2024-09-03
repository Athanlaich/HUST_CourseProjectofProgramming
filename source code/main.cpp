#define _CRT_SECURE_NO_WARNINGS 1
#pragma warning(disable:6031)
#include <iostream>
#include <filesystem>
#include <stack>
#include <queue>
#include <ctime>
#include <chrono>
#include <vector>
#include <windows.h>
#include <sys/stat.h>
#include <sstream>
#include <fstream>
#include "scan_Win.h"
#include "OP_Tree.h"
#include "op_file_dir.h"

using namespace std;
namespace fs = filesystem;
char sql[256];
// ��ʱ��ת��Ϊ SQL Server DATETIME ��ʽ���ַ���
std::string convertToSQLDateTime(long int time) {
    std::time_t t = static_cast<std::time_t>(time);
    std::tm* timeinfo = std::localtime(&t);
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return std::string(buffer);
}

// ���ɲ����ļ���Ϣ�� SQL ���
std::string generateFileInsertSQL(const std::string& filePath, long int fileSize, long int lastWriteTime, const std::string& upperFolder) {
    std::stringstream ss;
    ss << "INSERT INTO All_Files (file_Path, file_size, last_write_time, upper_folder) VALUES ('" << filePath << "', '" << fileSize << "', '" << convertToSQLDateTime(lastWriteTime) << "', '" << upperFolder << "')";
    return ss.str();
}

// ����ɾ���ļ��� SQL ���
std::string generateFileDeleteSQL(const std::string& filePath) {
    std::stringstream ss;
    ss << "DELETE FROM All_Files WHERE file_Path = '" << filePath << "'";

    return ss.str();
}

// �����޸��ļ���Ϣ�� SQL ���
std::string generateFileModifySQL(const std::string& filePath, long int fileSize, long int lastWriteTime) {
    std::stringstream ss;
    ss << "UPDATE All_Files SET file_size = '" << fileSize << "', last_write_time = '" << convertToSQLDateTime(lastWriteTime) << "' WHERE file_Path = '" << filePath << "'";
    return ss.str();
}

// ��myfile�ж�ȡ�������ݣ���������Ӧ�� SQL ���д�뵽����ļ���
void generateSQLFromFile(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream input(inputFile);
    if (!input.is_open()) {
        std::cerr << "Error: Failed to open input file." << std::endl;
        return;
    }

    std::ofstream output(outputFile);
    if (!output.is_open()) {
        std::cerr << "Error: Failed to open output file." << std::endl;
        return;
    }

    std::string line;
    bool insideSelectedFiles = false;
    std::vector<std::string> foldersToCheck;
    while (std::getline(input, line)) {
        if (line == "selected files") {
            insideSelectedFiles = true;
            continue;
        }
        if (line == "end of files") {
            break;
        }
        if (insideSelectedFiles) {
            std::string filePath;
            string operation;
            long int time, size;
            std::istringstream iss(line);
            if (std::getline(iss, filePath, ',') && std::getline(iss, operation, ',')&& iss >> time&& iss.ignore() &&
                iss >> size) {
                // ��ȡ�ļ�����Ŀ¼·��
                std::string folderPath = fs::path(filePath).parent_path().string();
                if (operation == "D") {
                    output << generateFileDeleteSQL(filePath) << ";" << std::endl;
                    // ���� Dir ���ӦĿ¼���ļ�����������޸�ʱ��
                    output << "UPDATE Dir SET file_count = file_count - 1 WHERE dir_Path = "<< "'" << folderPath << "';" << std::endl;
                    // ���ɾ�����ļ����޸�ʱ����ԭ��������޸�ʱ�䣬����¸�Ŀ¼������޸�ʱ��
                    output << "UPDATE Dir SET last_write_time = (SELECT MAX(last_write_time) FROM All_Files WHERE upper_folder = '" << folderPath << "') WHERE dir_Path = '" << folderPath << "' AND last_write_time = " << time << ";" << std::endl;
                }
                    
                if (operation == "M") {
                    output << generateFileModifySQL(filePath, size, time) << ";" << std::endl;
                    output << "UPDATE Dir SET last_write_time = GREATEST(last_write_time, " << time << ") WHERE dir_Path = '" << folderPath << "';" << std::endl;
                }
                    
                if (operation == "A") {

                    output << generateFileInsertSQL(filePath, size, time, folderPath) << ";" << std::endl;
                    output << "UPDATE Dir SET file_count = file_count + 1, last_write_time = GREATEST(last_write_time, " << time << ") WHERE dir_Path = '" << folderPath << "';" << std::endl;

                }         
                    //std::cerr << "Error: Invalid operation type." << std::endl;
                
            }
            //else {
               // std::cerr << "Error: Invalid line format." << std::endl;
            //}
        }
    }

    std::cout << "SQL statements generated successfully." << std::endl;

    input.close();
    output.close();
}

// ��mydir�ж�ȡ�������ݣ���������Ӧ�� SQL ���д�뵽����ļ���
void generateSQLFromDir(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream input(inputFile);
    if (!input.is_open()) {
        std::cerr << "Error: Failed to open input file." << std::endl;
        return;
    }

    std::ofstream output(outputFile);
    if (!output.is_open()) {
        std::cerr << "Error: Failed to open output file." << std::endl;
        return;
    }

    std::string line;
    bool insideSelectedFiles = false;

    while (std::getline(input, line)) {
        if (line == "selected dirs") {
            insideSelectedFiles = true;
            continue;
        }
        if (line == "end of dirs") {
            break;
        }
        if (insideSelectedFiles) {
            std::string dirPath;
            string operation;
            long int time, size;
            std::istringstream iss(line);
            if (std::getline(iss, dirPath, ',') && std::getline(iss, operation, ',')) {
                if (!dirPath.empty() && dirPath.back() == '\\') {
                    // �Ƴ����һ���ַ�
                    dirPath.pop_back();
                }
                // ɾ�� Dir ���ж�ӦĿ¼�ļ�¼
                output << "DELETE FROM Dir WHERE dir_Path = '" << dirPath << "';" << std::endl;
                // ɾ�� Files ���ж�ӦĿ¼�µ��ļ���¼
                output << "DELETE FROM Files WHERE file_Path LIKE '" << dirPath + "\\%" << "';" << std::endl;
                //std::cerr << "Error: Invalid operation type." << std::endl;

            }
            else {
                std::cerr << "Error: Invalid line format." << std::endl;
            }
        }
    }

    std::cout << "SQL statements generated successfully." << std::endl;

    input.close();
    output.close();
}

void showMenu(TreeNode* root,std::unordered_map<fs::path, TreeNode*>& directoryMap) {
    std::string input;
    while (true) {
        std::cout << "\n=====��Ҫ֪���޸��Ƿ�ɹ�����ӭ��ѯ��=====" << std::endl;
        std::cout << "1. ��Ҫ��ѯĳ��Ŀ¼������Щ�ļ�" << std::endl;
        std::cout << "2. ��Ҫ��ѯĳ���ļ���������Ϣ" << std::endl;
        std::cout << "3. �Ҳ����" << std::endl;
        std::cout << "��ѡ������������Ӧ���֣���" << std::endl;
        std::getline(std::cin, input);

        if (input == "1") {
            std::cout << "������Ҫ��ѯ���ļ���·����" << std::endl;
            std::getline(std::cin, input);
            fs::path folderPath(input);
            auto it = directoryMap.find(folderPath);
            if (it != directoryMap.end()) {
                TreeNode* folderNode = it->second;
                if (!folderNode->files.empty()) {
                    std::cout << "�ļ��� " << folderPath << " �е��ļ��У�" << std::endl;
                    for (const auto& file : folderNode->files) {
                        std::cout << file.path << std::endl;
                    }
                }
                else {
                    std::cout << "�ļ��� " << folderPath << " ��û���ļ���" << std::endl;
                }
            }
            else {
                std::cout << "�ļ���·�������ڣ�" << folderPath << std::endl;
            }
        }
        else if (input == "2") {
            std::cout << "������Ҫ��ѯ���ļ���ȫ·����" << std::endl;
            std::getline(std::cin, input);
            fs::path filePath(input);
            // �ļ�����Ŀ¼·������
            auto it = directoryMap.find(filePath.parent_path());
            if (it != directoryMap.end()) {
                TreeNode* folderNode = it->second;
                // �����ļ��б�
                bool fileFound = false;
                for (const auto& fileInfo : folderNode->files) {
                    if (fileInfo.path == filePath) {
                        // �ҵ��ļ�����ӡ�ļ���Ϣ
                        std::cout << "�ļ���Ϣ��" << std::endl;
                        std::cout << "�ļ���: " << fileInfo.path.filename() << std::endl;
                        std::cout << "�ļ�·��: " << fileInfo.path << std::endl;
                        std::cout << "����޸�ʱ��: " << std::asctime(std::localtime(&fileInfo.lastModifiedTime));
                        std::cout << "�ļ���С: " << fileInfo.fileSize << " bytes" << std::endl;
                        fileFound = true;
                        break;
                    }
                }
                if (!fileFound) {
                    std::cout << "�ļ�δ�ҵ���" << filePath << std::endl;
                }
            }
            else {
                std::cout << "�ļ�����Ŀ¼·�������ڣ�" << filePath.parent_path() << std::endl;
            }
        }
        else if (input == "3") {
            std::cout << "�˳�����" << std::endl;
            break;
        }
        else {
            std::cout << "��Ч��ѡ����������롣" << std::endl;
        }
    }
}

int main() {
    fs::path directoryPath = "C:\\Windows";
    string maxPath;
    TreeNode* root = new TreeNode{ directoryPath, {}, nullptr, {} };
    std::unordered_map<fs::path, TreeNode*> directoryMap; // ��ϣ���洢Ŀ¼·���Ͷ�Ӧ�Ľڵ�ָ��
    fs::path exePath = std::filesystem::current_path(); // ��ȡ��ǰ�������ڵ��ļ���·��
    std::string sqlDIRPath = exePath.string() + "\\mysqlDIR.sql";// SQL �ļ�·��
    std::string sqlFILEPath = exePath.string() + "\\mysqlFILE.sql"; // SQL �ļ�·��
    std::string logFilePath = exePath.string() + "\\My_Log.txt"; // ��־�ļ�·��
    std::string sql_output1 = exePath.string() + "\\output1.sql"; // �����ݿ�������ļ�·��
    std::string sql_output2 = exePath.string() + "\\output2.sql"; // �����ݿ�ڶ��β������ļ�·��
    ofstream logFile(logFilePath);
    if (!logFile.is_open()) {
        cerr << "Failed to create/open log file for writing." << endl;
        return 1;
    }
    // �û������ļ�·��
    std::string myfile, mystat, mydir;
    std::cout << "������ myfile.txt ��ȫ·����" << std::endl;
    std::getline(std::cin, myfile);
    std::cout << "������ mystat.txt ��ȫ·����" << std::endl;
    std::getline(std::cin, mystat);
    std::cout << "������ mydir.txt ��ȫ·����" << std::endl;
    std::getline(std::cin, mydir);
    // ɨ��Ŀ¼
    std::cout << "��ʼɨ��..." << std::endl;
    auto scanStart = std::chrono::steady_clock::now();
    scan_directory(directoryPath, maxPath, sqlDIRPath, sqlFILEPath, logFile);
    auto scanEnd = std::chrono::steady_clock::now();
    auto scanDuration = std::chrono::duration_cast<std::chrono::milliseconds>(scanEnd - scanStart);
    std::cout << "ɨ����ɣ���ʱ��" << scanDuration.count() << " ����" << std::endl;
    //���ɨ��
    cout << "������������������ֵ���..." << endl;
    system("pause");
    cout << "��ʼ����..." << endl;
    auto buildStart = std::chrono::steady_clock::now();
    buildTree(directoryPath, root, logFile, directoryMap);
    auto buildEnd = std::chrono::steady_clock::now();
    auto buildDuration = std::chrono::duration_cast<std::chrono::milliseconds>(buildEnd - buildStart);
    std::cout << "������ɣ���ʱ��" << buildDuration.count() << " ����" << std::endl;
    // �����������
    int treeDepth = bfsTraversalAndDepth(root);
    cout << "��Ŀ¼��·��" << root->path << endl;
    cout << "Ŀ¼����ȣ�" << treeDepth << endl;

    logFile << "��Ŀ¼��·��" << root->path << endl;
    logFile << "Ŀ¼����ȣ�" << treeDepth << endl;
    //std::string filename = "mystat.txt";
    cout << "�����������ȡmystat����ʼͳ����Ϣ..." << endl;
    system("pause");
    auto statStart = std::chrono::steady_clock::now();
    stat_file(root, mystat,directoryMap);//��һ��ͳ����Ϣ
    auto statEnd = std::chrono::steady_clock::now();
    auto statDuration = std::chrono::duration_cast<std::chrono::milliseconds>(statEnd - statStart);
    std::cout << "ͳ����Ϣ��ɣ���ʱ��" << statDuration.count() << " ����" << std::endl;
    cout << "������������ļ����в���" << endl;
    system("pause");

    // ��ȡ������������ģ��
    statStart = std::chrono::steady_clock::now();
    readOperationsAndSimulate(root, myfile, directoryMap);//���ļ����в���
    statEnd = std::chrono::steady_clock::now();
    statDuration = std::chrono::duration_cast<std::chrono::milliseconds>(statEnd - statStart);
    std::cout << "�ļ�������ɣ���ʱ��" << statDuration.count() << " ����" << std::endl;  
    cout << "������������ɶ����ݿ������sql�ļ�" << endl;
    system("pause");
    //�����ݿ��������ı�
    std::ofstream output1(sql_output1);
    if (!output1.is_open()) {
        std::cerr << "Error: Failed to open output file." << std::endl;
        return 1;
    }
    generateSQLFromFile(myfile, "D:\\study\\output1.sql");

    // �͵�һ�ν��жԱ�
    cout << "��һ�αȽ�" << endl;
    auto compareStart = std::chrono::steady_clock::now();
    compareInfo(root, mystat, logFile, directoryMap);
    auto compareEnd = std::chrono::steady_clock::now();
    auto compareDuration = std::chrono::duration_cast<std::chrono::milliseconds>(compareEnd - compareStart);
    std::cout << "�Ա���ɣ���ʱ��" << compareDuration.count() << " ����" << std::endl;

    std::cout << "������Ŀ¼���в���" << std::endl;

    system("pause");

    //��Ŀ¼���в���
    auto dirOperationStart = std::chrono::steady_clock::now();
    DirOperation(root, mydir, directoryMap);
    auto dirOperationEnd = std::chrono::steady_clock::now();
    auto dirOperationDuration = std::chrono::duration_cast<std::chrono::milliseconds>(dirOperationEnd - dirOperationStart);
    std::cout << "Ŀ¼������ɣ���ʱ��" << dirOperationDuration.count() << " ����" << std::endl;
    system("pause");

    std::cout << "�ڶ��αȽ�" << std::endl;
    auto compareStart2 = std::chrono::steady_clock::now();
    compareInfo(root, mystat, logFile, directoryMap);
    auto compareEnd2 = std::chrono::steady_clock::now();
    auto compareDuration2 = std::chrono::duration_cast<std::chrono::milliseconds>(compareEnd2 - compareStart2);
    std::cout << "�Ա���ɣ���ʱ��" << compareDuration2.count() << " ����" << std::endl;
    cout << "������������ɶ����ݿ������sql�ļ�" << endl;
    system("pause");
    //�����ݿ��������ı�
    std::ofstream output2(sql_output2);
    if (!output2.is_open()) {
        std::cerr << "Error: Failed to open output file." << std::endl;
        return 1;
    }
    generateSQLFromDir(mydir, "D:\\study\\output2.sql");

    showMenu(root, directoryMap);
    logFile.close();
    
    // �ͷ��ڴ�
    delete root;

    system("pause");
    return 0;
}

