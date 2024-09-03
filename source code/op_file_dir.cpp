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
//读入mystat文件
void stat_file(TreeNode* root, const std::string& filename, std::unordered_map<fs::path, TreeNode*>& directoryMap) {
    std::ifstream file(filename);//打开文件
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

        // 在每次读取行后，检查该行是否以换行符结尾，如果是的话，将其移除
        if (!line.empty() && line.back() == '\\') {
            line.pop_back();
        }

        fs::path directoryPath(line);

        // 检查路径是否存在
        if (!fs::exists(directoryPath)) {
            std::cerr << "Error: Directory " << line << " does not exist." << std::endl;
            continue;
        }

        // 检查路径是否为目录
        if (!fs::is_directory(directoryPath)) {
            std::cerr << "Error: " << line << " is not a directory." << std::endl;
            continue;
        }

        // 统计目录下文件数量、字节数总和，以及最早和最晚的文件信息
        calculateTreeFolderStats(findNode(root, directoryPath, directoryMap));
    }

    file.close();
}

// 模拟文件操作函数,对于每一条操作，传入root根节点，文件的全路径filePath，操作类型operation，时间，大小
void simulateFileOperation(TreeNode* root, fs::path filePath, const std::string& operation, long int time, long int size, std::unordered_map<fs::path, TreeNode*>& directoryMap) {
    fs::path parentPath = filePath.parent_path();//找到文件所在文件夹的路径parentPath

    // 尝试从哈希表中查找目标文件所在的文件夹结点
    auto it = directoryMap.find(parentPath);
    if (it == directoryMap.end()) {//如果没找到
        std::cerr << "Error: Folder path " << parentPath << " not found." << std::endl;
        return;
    }

    TreeNode* targetNode = it->second;//文件夹结点

    // 根据操作类型执行相应的模拟操作
    if (operation == "D") { 
        // 删除文件节点
        //deleteFileNode(root, filePath);
        if (!removeFileFromNode(targetNode, filePath))
            std::cout << "删除失败：" << filePath << " 未找到" << std::endl;
    }
    else if (operation == "M") { 
        // 更新文件节点的时间和大小属性
        //TreeNode* targetNode = findFolderNode(root, filePath); // 找到文件所在目录的结点
        updateFileAttributes(targetNode, filePath, time, size);
    }
    else if (operation == "A") { 
        // 创建新的文件节点并添加到目录树中
        createFileNode(targetNode, filePath, time, size);
    }
    else {
        std::cerr << "Error: Invalid operation type." << std::endl;
    }
}

// 从myfile.txt文件中读取操作并在树中模拟
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
            // 解析文件路径和操作
            std::istringstream iss(line);
            std::string filePathStr;
            std::string operation;
            long int time = 0; // 将时间设为0，在删除操作中没有意义
            long int size = 0; // 将大小设为0，在删除操作中没有意义
            if (std::getline(iss, filePathStr, ',') &&
                std::getline(iss, operation, ',')) {
                // 将字符串路径转换为 fs::path
                fs::path directoryPath(filePathStr);
                // 如果操作是删除（D），则执行删除文件操作，忽略时间和大小属性
                if (operation == "D") {
                    simulateFileOperation(root, directoryPath, "D", 0, 0,directoryMap);
                }
                else {
                    // 如果操作是其他类型，则解析时间和大小属性，并执行相应的模拟文件操作
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

// 从mydir.txt文件中读取操作并模拟
void DirOperation(TreeNode* root, const std::string& filename, std::unordered_map<fs::path, TreeNode*>& directoryMap) {
    //root, "D:\\study\\课设\\mydir.txt", directoryMap
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
            // 解析目录路径和操作
            std::istringstream iss(line);
            std::string dirPathStr;
            std::string operation;
            if (std::getline(iss, dirPathStr, ',') &&
                std::getline(iss, operation, ',')) {
                // 将字符串路径转换为 fs::path
                if (!dirPathStr.empty() && dirPathStr.back() == '\\') {
                    // 移除最后一个字符
                    dirPathStr.pop_back();
                }
                fs::path directoryPath(dirPathStr);
                // 如果操作是删除（D），则执行删除目录操作，忽略时间和大小属性
                if (operation == "D") {
                    if (!deleteFolderFromNode(root, directoryPath, directoryMap))
                        std::cerr << "删除目录失败：" << dirPathStr << std::endl;
                }
            }
            else {
                std::cerr << "Error: Invalid operation format." << std::endl;
            }
        }

    }

    file.close();
}

//读入mystat，对比前后变化，包括文件数，文件总大小，最早修改时间，最晚修改时间
void compareInfo(TreeNode* root, const std::string& filename, std::ofstream& logFile,std::unordered_map<fs::path, TreeNode*>& directoryMap) {
    
    std::ifstream file(filename);//打开文件
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

        // 在每次读取行后，检查该行是否以换行符结尾，如果是的话，将其移除
        if (!line.empty() && line.back() == '\\') {
            line.pop_back();
        }

        fs::path directoryPath(line);
        if (line == "C:\\windows\\WinSxS\\wow64_microsoft-windows-i..raries-servercommon_31bf3856ad364e35_10.0.22621.2506_none_93ba9b79200ee181\\f")
            system("pause");
        // 检查路径是否存在
        if (!fs::exists(directoryPath)) {
            std::cerr << "Error: Directory " << line << " does not exist." << std::endl;
            continue;
        }

        // 检查路径是否为目录
        if (!fs::is_directory(directoryPath)) {
            std::cerr << "Error: " << line << " is not a directory." << std::endl;
            continue;
        }
        
        TreeNode* folderNode = findNode(root, directoryPath, directoryMap);
        if (folderNode == nullptr) {
            cout << "文件夹已被删除" << endl;
        }
        else {
            // 比较节点信息，并将结果写入日志文件
            logFile << "比较目录：" << directoryPath << endl;
            compareNode(folderNode, logFile);
            logFile << endl; // 添加换行符以分隔不同目录的比较结果
        }
    }

    file.close();
}