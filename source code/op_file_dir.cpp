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
//����mystat�ļ�
void stat_file(TreeNode* root, const std::string& filename, std::unordered_map<fs::path, TreeNode*>& directoryMap) {
    std::ifstream file(filename);//���ļ�
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open file " << filename << std::endl;
        return;
    }

    std::string line;
    bool insideDirectories = false;

    while (std::getline(file, line)) {
        if (line == "stat dirs") {
            insideDirectories = true;
            continue;
        }
        if (line == "end of dirs") {
            break;
        }
        if (!insideDirectories) {
            continue;
        }

        // ��ÿ�ζ�ȡ�к󣬼������Ƿ��Ի��з���β������ǵĻ��������Ƴ�
        if (!line.empty() && line.back() == '\\') {
            line.pop_back();
        }

        fs::path directoryPath(line);

        // ���·���Ƿ����
        if (!fs::exists(directoryPath)) {
            std::cerr << "Error: Directory " << line << " does not exist." << std::endl;
            continue;
        }

        // ���·���Ƿ�ΪĿ¼
        if (!fs::is_directory(directoryPath)) {
            std::cerr << "Error: " << line << " is not a directory." << std::endl;
            continue;
        }

        // ͳ��Ŀ¼���ļ��������ֽ����ܺͣ��Լ������������ļ���Ϣ
        calculateTreeFolderStats(findNode(root, directoryPath, directoryMap));
    }

    file.close();
}

// ģ���ļ���������,����ÿһ������������root���ڵ㣬�ļ���ȫ·��filePath����������operation��ʱ�䣬��С
void simulateFileOperation(TreeNode* root, fs::path filePath, const std::string& operation, long int time, long int size, std::unordered_map<fs::path, TreeNode*>& directoryMap) {
    fs::path parentPath = filePath.parent_path();//�ҵ��ļ������ļ��е�·��parentPath

    // ���Դӹ�ϣ���в���Ŀ���ļ����ڵ��ļ��н��
    auto it = directoryMap.find(parentPath);
    if (it == directoryMap.end()) {//���û�ҵ�
        std::cerr << "Error: Folder path " << parentPath << " not found." << std::endl;
        return;
    }

    TreeNode* targetNode = it->second;//�ļ��н��

    // ���ݲ�������ִ����Ӧ��ģ�����
    if (operation == "D") { 
        // ɾ���ļ��ڵ�
        //deleteFileNode(root, filePath);
        if (!removeFileFromNode(targetNode, filePath))
            std::cout << "ɾ��ʧ�ܣ�" << filePath << " δ�ҵ�" << std::endl;
    }
    else if (operation == "M") { 
        // �����ļ��ڵ��ʱ��ʹ�С����
        //TreeNode* targetNode = findFolderNode(root, filePath); // �ҵ��ļ�����Ŀ¼�Ľ��
        updateFileAttributes(targetNode, filePath, time, size);
    }
    else if (operation == "A") { 
        // �����µ��ļ��ڵ㲢��ӵ�Ŀ¼����
        createFileNode(targetNode, filePath, time, size);
    }
    else {
        std::cerr << "Error: Invalid operation type." << std::endl;
    }
}

// ��myfile.txt�ļ��ж�ȡ������������ģ��
void readOperationsAndSimulate(TreeNode* root, const std::string& filename, std::unordered_map<fs::path, TreeNode*>& directoryMap) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open file " << filename << std::endl;
        return;
    }

    std::string line;
    bool insideSelectedFiles = false;

    while (std::getline(file, line)) {
        if (line == "selected files") {
            insideSelectedFiles = true;
            continue;
        }
        if (line == "end of files") {
            break;
        }
        if (insideSelectedFiles) {
            // �����ļ�·���Ͳ���
            std::istringstream iss(line);
            std::string filePathStr;
            std::string operation;
            long int time = 0; // ��ʱ����Ϊ0����ɾ��������û������
            long int size = 0; // ����С��Ϊ0����ɾ��������û������
            if (std::getline(iss, filePathStr, ',') &&
                std::getline(iss, operation, ',')) {
                // ���ַ���·��ת��Ϊ fs::path
                fs::path directoryPath(filePathStr);
                // ���������ɾ����D������ִ��ɾ���ļ�����������ʱ��ʹ�С����
                if (operation == "D") {
                    simulateFileOperation(root, directoryPath, "D", 0, 0,directoryMap);
                }
                else {
                    // ����������������ͣ������ʱ��ʹ�С���ԣ���ִ����Ӧ��ģ���ļ�����
                    if (iss >> time &&
                        iss.ignore() &&
                        iss >> size) {
                        simulateFileOperation(root, directoryPath, operation, time, size,directoryMap);
                    }
                    else {
                        std::cerr << "Error: Invalid operation format." << std::endl;
                    }
                }
            }
            else {
                std::cerr << "Error: Invalid operation format." << std::endl;
            }
        }

    }

    file.close();
}

// ��mydir.txt�ļ��ж�ȡ������ģ��
void DirOperation(TreeNode* root, const std::string& filename, std::unordered_map<fs::path, TreeNode*>& directoryMap) {
    //root, "D:\\study\\����\\mydir.txt", directoryMap
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open file " << filename << std::endl;
        return;
    }

    std::string line;
    bool insideSelectedFiles = false;

    while (std::getline(file, line)) {
        if (line == "selected dirs") {
            insideSelectedFiles = true;
            continue;
        }
        if (line == "end of dirs") {
            break;
        }
        if (insideSelectedFiles) {
            // ����Ŀ¼·���Ͳ���
            std::istringstream iss(line);
            std::string dirPathStr;
            std::string operation;
            if (std::getline(iss, dirPathStr, ',') &&
                std::getline(iss, operation, ',')) {
                // ���ַ���·��ת��Ϊ fs::path
                if (!dirPathStr.empty() && dirPathStr.back() == '\\') {
                    // �Ƴ����һ���ַ�
                    dirPathStr.pop_back();
                }
                fs::path directoryPath(dirPathStr);
                // ���������ɾ����D������ִ��ɾ��Ŀ¼����������ʱ��ʹ�С����
                if (operation == "D") {
                    if (!deleteFolderFromNode(root, directoryPath, directoryMap))
                        std::cerr << "ɾ��Ŀ¼ʧ�ܣ�" << dirPathStr << std::endl;
                }
            }
            else {
                std::cerr << "Error: Invalid operation format." << std::endl;
            }
        }

    }

    file.close();
}

//����mystat���Ա�ǰ��仯�������ļ������ļ��ܴ�С�������޸�ʱ�䣬�����޸�ʱ��
void compareInfo(TreeNode* root, const std::string& filename, std::ofstream& logFile,std::unordered_map<fs::path, TreeNode*>& directoryMap) {
    
    std::ifstream file(filename);//���ļ�
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open file " << filename << std::endl;
        return;
    }

    std::string line;
    bool insideDirectories = false;

    while (std::getline(file, line)) {
        if (line == "stat dirs") {
            insideDirectories = true;
            continue;
        }
        if (line == "end of dirs") {
            break;
        }
        if (!insideDirectories) {
            continue;
        }

        // ��ÿ�ζ�ȡ�к󣬼������Ƿ��Ի��з���β������ǵĻ��������Ƴ�
        if (!line.empty() && line.back() == '\\') {
            line.pop_back();
        }

        fs::path directoryPath(line);
        if (line == "C:\\windows\\WinSxS\\wow64_microsoft-windows-i..raries-servercommon_31bf3856ad364e35_10.0.22621.2506_none_93ba9b79200ee181\\f")
            system("pause");
        // ���·���Ƿ����
        if (!fs::exists(directoryPath)) {
            std::cerr << "Error: Directory " << line << " does not exist." << std::endl;
            continue;
        }

        // ���·���Ƿ�ΪĿ¼
        if (!fs::is_directory(directoryPath)) {
            std::cerr << "Error: " << line << " is not a directory." << std::endl;
            continue;
        }
        
        TreeNode* folderNode = findNode(root, directoryPath, directoryMap);
        if (folderNode == nullptr) {
            cout << "�ļ����ѱ�ɾ��" << endl;
        }
        else {
            // �ȽϽڵ���Ϣ���������д����־�ļ�
            logFile << "�Ƚ�Ŀ¼��" << directoryPath << endl;
            compareNode(folderNode, logFile);
            logFile << endl; // ��ӻ��з��Էָ���ͬĿ¼�ıȽϽ��
        }
    }

    file.close();
}