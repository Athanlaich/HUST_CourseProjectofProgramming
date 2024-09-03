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
// 将时间转换为 SQL Server DATETIME 格式的字符串
std::string convertToSQLDateTime(long int time) {
    std::time_t t = static_cast<std::time_t>(time);
    std::tm* timeinfo = std::localtime(&t);
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return std::string(buffer);
}

// 生成插入文件信息的 SQL 语句
std::string generateFileInsertSQL(const std::string& filePath, long int fileSize, long int lastWriteTime, const std::string& upperFolder) {
    std::stringstream ss;
    ss << "INSERT INTO All_Files (file_Path, file_size, last_write_time, upper_folder) VALUES ('" << filePath << "', '" << fileSize << "', '" << convertToSQLDateTime(lastWriteTime) << "', '" << upperFolder << "')";
    return ss.str();
}

// 生成删除文件的 SQL 语句
std::string generateFileDeleteSQL(const std::string& filePath) {
    std::stringstream ss;
    ss << "DELETE FROM All_Files WHERE file_Path = '" << filePath << "'";

    return ss.str();
}

// 生成修改文件信息的 SQL 语句
std::string generateFileModifySQL(const std::string& filePath, long int fileSize, long int lastWriteTime) {
    std::stringstream ss;
    ss << "UPDATE All_Files SET file_size = '" << fileSize << "', last_write_time = '" << convertToSQLDateTime(lastWriteTime) << "' WHERE file_Path = '" << filePath << "'";
    return ss.str();
}

// 从myfile中读取操作数据，并生成相应的 SQL 语句写入到输出文件中
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
                // 获取文件所在目录路径
                std::string folderPath = fs::path(filePath).parent_path().string();
                if (operation == "D") {
                    output << generateFileDeleteSQL(filePath) << ";" << std::endl;
                    // 更新 Dir 表对应目录的文件计数和最后修改时间
                    output << "UPDATE Dir SET file_count = file_count - 1 WHERE dir_Path = "<< "'" << folderPath << "';" << std::endl;
                    // 如果删除的文件的修改时间是原来的最后修改时间，则更新该目录的最后修改时间
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

// 从mydir中读取操作数据，并生成相应的 SQL 语句写入到输出文件中
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
                    // 移除最后一个字符
                    dirPath.pop_back();
                }
                // 删除 Dir 表中对应目录的记录
                output << "DELETE FROM Dir WHERE dir_Path = '" << dirPath << "';" << std::endl;
                // 删除 Files 表中对应目录下的文件记录
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
        std::cout << "\n=====想要知道修改是否成功？欢迎查询！=====" << std::endl;
        std::cout << "1. 我要查询某个目录下有哪些文件" << std::endl;
        std::cout << "2. 我要查询某个文件的所有信息" << std::endl;
        std::cout << "3. 我不想查" << std::endl;
        std::cout << "请选择操作（输入对应数字）：" << std::endl;
        std::getline(std::cin, input);

        if (input == "1") {
            std::cout << "请输入要查询的文件夹路径：" << std::endl;
            std::getline(std::cin, input);
            fs::path folderPath(input);
            auto it = directoryMap.find(folderPath);
            if (it != directoryMap.end()) {
                TreeNode* folderNode = it->second;
                if (!folderNode->files.empty()) {
                    std::cout << "文件夹 " << folderPath << " 中的文件有：" << std::endl;
                    for (const auto& file : folderNode->files) {
                        std::cout << file.path << std::endl;
                    }
                }
                else {
                    std::cout << "文件夹 " << folderPath << " 中没有文件。" << std::endl;
                }
            }
            else {
                std::cout << "文件夹路径不存在：" << folderPath << std::endl;
            }
        }
        else if (input == "2") {
            std::cout << "请输入要查询的文件的全路径：" << std::endl;
            std::getline(std::cin, input);
            fs::path filePath(input);
            // 文件所在目录路径存在
            auto it = directoryMap.find(filePath.parent_path());
            if (it != directoryMap.end()) {
                TreeNode* folderNode = it->second;
                // 遍历文件列表
                bool fileFound = false;
                for (const auto& fileInfo : folderNode->files) {
                    if (fileInfo.path == filePath) {
                        // 找到文件，打印文件信息
                        std::cout << "文件信息：" << std::endl;
                        std::cout << "文件名: " << fileInfo.path.filename() << std::endl;
                        std::cout << "文件路径: " << fileInfo.path << std::endl;
                        std::cout << "最后修改时间: " << std::asctime(std::localtime(&fileInfo.lastModifiedTime));
                        std::cout << "文件大小: " << fileInfo.fileSize << " bytes" << std::endl;
                        fileFound = true;
                        break;
                    }
                }
                if (!fileFound) {
                    std::cout << "文件未找到：" << filePath << std::endl;
                }
            }
            else {
                std::cout << "文件所在目录路径不存在：" << filePath.parent_path() << std::endl;
            }
        }
        else if (input == "3") {
            std::cout << "退出程序。" << std::endl;
            break;
        }
        else {
            std::cout << "无效的选项，请重新输入。" << std::endl;
        }
    }
}

int main() {
    fs::path directoryPath = "C:\\Windows";
    string maxPath;
    TreeNode* root = new TreeNode{ directoryPath, {}, nullptr, {} };
    std::unordered_map<fs::path, TreeNode*> directoryMap; // 哈希表，存储目录路径和对应的节点指针
    fs::path exePath = std::filesystem::current_path(); // 获取当前程序所在的文件夹路径
    std::string sqlDIRPath = exePath.string() + "\\mysqlDIR.sql";// SQL 文件路径
    std::string sqlFILEPath = exePath.string() + "\\mysqlFILE.sql"; // SQL 文件路径
    std::string logFilePath = exePath.string() + "\\My_Log.txt"; // 日志文件路径
    std::string sql_output1 = exePath.string() + "\\output1.sql"; // 对数据库操作的文件路径
    std::string sql_output2 = exePath.string() + "\\output2.sql"; // 对数据库第二次操作的文件路径
    ofstream logFile(logFilePath);
    if (!logFile.is_open()) {
        cerr << "Failed to create/open log file for writing." << endl;
        return 1;
    }
    // 用户输入文件路径
    std::string myfile, mystat, mydir;
    std::cout << "请输入 myfile.txt 的全路径：" << std::endl;
    std::getline(std::cin, myfile);
    std::cout << "请输入 mystat.txt 的全路径：" << std::endl;
    std::getline(std::cin, mystat);
    std::cout << "请输入 mydir.txt 的全路径：" << std::endl;
    std::getline(std::cin, mydir);
    // 扫描目录
    std::cout << "开始扫描..." << std::endl;
    auto scanStart = std::chrono::steady_clock::now();
    scan_directory(directoryPath, maxPath, sqlDIRPath, sqlFILEPath, logFile);
    auto scanEnd = std::chrono::steady_clock::now();
    auto scanDuration = std::chrono::duration_cast<std::chrono::milliseconds>(scanEnd - scanStart);
    std::cout << "扫描完成，耗时：" << scanDuration.count() << " 毫秒" << std::endl;
    //完成扫描
    cout << "点击继续，建立孩子兄弟树..." << endl;
    system("pause");
    cout << "开始建树..." << endl;
    auto buildStart = std::chrono::steady_clock::now();
    buildTree(directoryPath, root, logFile, directoryMap);
    auto buildEnd = std::chrono::steady_clock::now();
    auto buildDuration = std::chrono::duration_cast<std::chrono::milliseconds>(buildEnd - buildStart);
    std::cout << "建树完成，耗时：" << buildDuration.count() << " 毫秒" << std::endl;
    // 计算树的深度
    int treeDepth = bfsTraversalAndDepth(root);
    cout << "根目录的路径" << root->path << endl;
    cout << "目录树深度：" << treeDepth << endl;

    logFile << "根目录的路径" << root->path << endl;
    logFile << "目录树深度：" << treeDepth << endl;
    //std::string filename = "mystat.txt";
    cout << "点击继续，读取mystat，开始统计信息..." << endl;
    system("pause");
    auto statStart = std::chrono::steady_clock::now();
    stat_file(root, mystat,directoryMap);//第一次统计信息
    auto statEnd = std::chrono::steady_clock::now();
    auto statDuration = std::chrono::duration_cast<std::chrono::milliseconds>(statEnd - statStart);
    std::cout << "统计信息完成，耗时：" << statDuration.count() << " 毫秒" << std::endl;
    cout << "点击继续，对文件进行操作" << endl;
    system("pause");

    // 读取操作并在树中模拟
    statStart = std::chrono::steady_clock::now();
    readOperationsAndSimulate(root, myfile, directoryMap);//对文件进行操作
    statEnd = std::chrono::steady_clock::now();
    statDuration = std::chrono::duration_cast<std::chrono::milliseconds>(statEnd - statStart);
    std::cout << "文件操作完成，耗时：" << statDuration.count() << " 毫秒" << std::endl;  
    cout << "点击继续，生成对数据库操作的sql文件" << endl;
    system("pause");
    //在数据库中做出改变
    std::ofstream output1(sql_output1);
    if (!output1.is_open()) {
        std::cerr << "Error: Failed to open output file." << std::endl;
        return 1;
    }
    generateSQLFromFile(myfile, "D:\\study\\output1.sql");

    // 和第一次进行对比
    cout << "第一次比较" << endl;
    auto compareStart = std::chrono::steady_clock::now();
    compareInfo(root, mystat, logFile, directoryMap);
    auto compareEnd = std::chrono::steady_clock::now();
    auto compareDuration = std::chrono::duration_cast<std::chrono::milliseconds>(compareEnd - compareStart);
    std::cout << "对比完成，耗时：" << compareDuration.count() << " 毫秒" << std::endl;

    std::cout << "继续对目录进行操作" << std::endl;

    system("pause");

    //对目录进行操作
    auto dirOperationStart = std::chrono::steady_clock::now();
    DirOperation(root, mydir, directoryMap);
    auto dirOperationEnd = std::chrono::steady_clock::now();
    auto dirOperationDuration = std::chrono::duration_cast<std::chrono::milliseconds>(dirOperationEnd - dirOperationStart);
    std::cout << "目录操作完成，耗时：" << dirOperationDuration.count() << " 毫秒" << std::endl;
    system("pause");

    std::cout << "第二次比较" << std::endl;
    auto compareStart2 = std::chrono::steady_clock::now();
    compareInfo(root, mystat, logFile, directoryMap);
    auto compareEnd2 = std::chrono::steady_clock::now();
    auto compareDuration2 = std::chrono::duration_cast<std::chrono::milliseconds>(compareEnd2 - compareStart2);
    std::cout << "对比完成，耗时：" << compareDuration2.count() << " 毫秒" << std::endl;
    cout << "点击继续，生成对数据库操作的sql文件" << endl;
    system("pause");
    //在数据库中做出改变
    std::ofstream output2(sql_output2);
    if (!output2.is_open()) {
        std::cerr << "Error: Failed to open output file." << std::endl;
        return 1;
    }
    generateSQLFromDir(mydir, "D:\\study\\output2.sql");

    showMenu(root, directoryMap);
    logFile.close();
    
    // 释放内存
    delete root;

    system("pause");
    return 0;
}

