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
#include "OP_Tree.h"
using namespace std;
namespace fs = filesystem;
struct stat fileInfo = {};
//传入路径，路径的结点
void buildTree(const fs::path& rootPath, TreeNode* parentNode, ofstream& logFile, std::unordered_map<fs::path, TreeNode*>& directoryMap) {
    try {
        for (const auto& entry : fs::directory_iterator(rootPath)) {
            if (fs::is_directory(entry)) {
                TreeNode* newNode = new TreeNode{ entry.path(), {}, nullptr, {},0,0,0,0 };//路径，孩子，兄弟，文件信息，文件总大小，文件总数，最早修改时间，最晚修改时间
                if (parentNode->firstChild == nullptr) {
                    parentNode->firstChild = newNode;
                }
                else {
                    TreeNode* sibling = parentNode->firstChild;
                    while (sibling->nextSibling != nullptr) {
                        sibling = sibling->nextSibling;
                    }
                    sibling->nextSibling = newNode;
                }
                buildTree(entry, newNode, logFile, directoryMap);
            }
            else {//如果是文件
                stat(entry.path().string().c_str(), &fileInfo);
                // 获取最后修改时间
                std::time_t ModifiedTime = fileInfo.st_mtime;
                size_t fileSize = fs::file_size(entry);
                // 添加文件信息到父节点的文件列表中
                parentNode->files.push_back(FileInfo(entry.path(), ModifiedTime, fileSize));
            }
        }
        // 在哈希表中存储目录路径和对应的节点指针
        directoryMap[rootPath] = parentNode;
    }
    catch (const fs::filesystem_error& e) {
        if (e.code().value() == ERROR_ACCESS_DENIED) {
            logFile << "Access denied error accessing directory: " << rootPath << endl;
        }
        else {
            logFile << "Filesystem error accessing directory: " << rootPath << endl;
        }
    }
}

// 非递归算法遍历孩子兄弟树并计算深度
int bfsTraversalAndDepth(TreeNode* root) {
    if (root == nullptr) {
        return 0;
    }
    // 创建一个队列用于层次遍历
    queue<TreeNode*> nodeQueue;
    nodeQueue.push(root);
    int depth = 0;

    while (!nodeQueue.empty()) {
        int levelSize = nodeQueue.size(); // 当前层的节点数
        depth++; // 进入下一层

        // 遍历当前层的节点
        for (int i = 0; i < levelSize; i++) {
            TreeNode* currentNode = nodeQueue.front();
            nodeQueue.pop();

            // 将当前节点的孩子节点入队
            if (currentNode->firstChild != nullptr) {
                nodeQueue.push(currentNode->firstChild);
            }

            // 将当前节点的兄弟节点入队
            if (currentNode->nextSibling != nullptr) {
                nodeQueue.push(currentNode->nextSibling);
            }
        }
    }

    return depth;
}

//传入文件夹所在结点,统计目录下文件的信息
void calculateTreeFolderStats(TreeNode* folderNode) {
    if (folderNode == nullptr) {
        std::cerr << "Error: Folder node is nullptr." << std::endl;
        return;
    }
    // 初始化统计信息
    int fileCount = 0;
    size_t totalBytes = 0;
    std::time_t earliestModifiedTime = 0; // 初始化为 0
    std::time_t latestModifiedTime = 0;   // 初始化为 0
    bool isFirstFile = true; // 标记是否是第一个文件
    bool directoryIsEmpty = true;//标记目录是否为空
    // 遍历文件列表，统计信息
    // 遍历目录下的文件
    for (const auto& entry : fs::directory_iterator(folderNode->path)) {
        if (fs::is_regular_file(entry)) {
            directoryIsEmpty = false;
            fileCount++;
            totalBytes += fs::file_size(entry);
            stat(entry.path().string().c_str(), &fileInfo);
            // 获取最后修改时间
            std::time_t modifiedTime = fileInfo.st_mtime;//最后修改时间
            if (isFirstFile) {
                earliestModifiedTime = modifiedTime;
                latestModifiedTime = modifiedTime;
                isFirstFile = false;
            }
            else {
                if (modifiedTime < earliestModifiedTime) {
                    earliestModifiedTime = modifiedTime;
                }
                if (modifiedTime > latestModifiedTime) {
                    latestModifiedTime = modifiedTime;
                }
            }
        }
    }
    folderNode->AllfileSize = totalBytes;//文件总大小
    folderNode->fileCount = fileCount;//文件总数
    // 如果目录为空，则将 earliestModifiedTime 和 latestModifiedTime 设置为目录的创建时间
    if (directoryIsEmpty) {
        stat(folderNode->path.string().c_str(), &fileInfo);
        // 获取最后修改时间
        std::time_t modifiedTime = fileInfo.st_mtime;
        earliestModifiedTime = modifiedTime;
        latestModifiedTime = earliestModifiedTime;
    }
    folderNode->earliest = earliestModifiedTime;//最早修改时间
    folderNode->latest = latestModifiedTime;//最晚修改时间
    // 打印统计结果
    std::cout << "Folder: " << folderNode->path << std::endl;
    std::cout << "File Count: " << fileCount << std::endl;
    std::cout << "Total Bytes: " << totalBytes << std::endl;
    if (fileCount > 0) {
        std::cout << "Earliest Modified Time: " << std::asctime(std::localtime(&earliestModifiedTime));
        std::cout << "Latest Modified Time: " << std::asctime(std::localtime(&latestModifiedTime));
    }
    std::cout << std::endl;
}
//通过文件夹路径查找结点
TreeNode* findNode(TreeNode* root, const fs::path& filePath, std::unordered_map<fs::path, TreeNode*>& directoryMap) {
    // 如果根节点为空，则直接返回空指针
    if (root == nullptr) {

        return nullptr;
    }
    // 尝试从哈希表中查找目标文件夹路径对应的节点
    auto it = directoryMap.find(filePath);
    if (it != directoryMap.end()) {
        return it->second; // 找到目标文件夹路径的结点
    }
    std::queue<TreeNode*> nodeQueue;
    nodeQueue.push(root);
    std::string filePathString = filePath.string();
    while (!nodeQueue.empty()) {
        TreeNode* currentNode = nodeQueue.front();
        nodeQueue.pop();
        // 将当前节点的路径转换为字符串
        std::string currentNodePathString = currentNode->path.string();
        // 检查当前节点的路径是否匹配目标文件夹路径
        if (currentNodePathString.size() >= 11 &&
            currentNodePathString.substr(10) == filePathString.substr(10)) {
            return currentNode; // 找到目标文件夹路径
        }

        // 将当前节点的孩子节点入队
        if (currentNode->firstChild != nullptr) {
            nodeQueue.push(currentNode->firstChild);
        }

        // 将当前节点的兄弟节点入队
        if (currentNode->nextSibling != nullptr) {
            nodeQueue.push(currentNode->nextSibling);
        }
    }
    // 如果遍历完整个树都未找到目标文件夹，则返回空指针
    cout << "未找到目标文文件夹：" << filePath << endl;
    return nullptr;
}

//对比信息,传入文件夹所在结点
void compareNode(TreeNode* folderNode, std::ofstream& logFile) {
    if (folderNode == nullptr) {
        std::cerr << "Error: Folder node is nullptr." << std::endl;
        //logFile << "Error: Folder node is nullptr." << std::endl;
        return ;
    }
    // 初始化统计信息
    int fileCount = 0;
    size_t totalBytes = 0;
    std::time_t earliestModifiedTime = 0; // 初始化为 0
    std::time_t latestModifiedTime = 0;   // 初始化为 0
    bool isFirstFile = true; // 标记是否是第一个文件
    bool directoryIsEmpty = true;//标记目录是否为空
    // 遍历文件列表，统计信息
    for (const auto& fileInfo : folderNode->files) {
        //if (fs::is_regular_file(fileInfo.path)) {
            fileCount++;
            totalBytes += fileInfo.fileSize;
            // 获取最后修改时间
            std::time_t modifiedTime = fileInfo.lastModifiedTime;
            if (fileCount == 1) {
                earliestModifiedTime = modifiedTime;
                latestModifiedTime = modifiedTime;
            }
            else {
                if (modifiedTime < earliestModifiedTime) {
                    earliestModifiedTime = modifiedTime;
                }
                if (modifiedTime > latestModifiedTime) {
                    latestModifiedTime = modifiedTime;
                }
            }
        //}
    }
    // 如果目录为空，则将 earliestModifiedTime 和 latestModifiedTime 设置为目录的创建时间
    if (fileCount == 0) {
        stat(folderNode->path.string().c_str(), &fileInfo);
        // 获取最后修改时间
        std::time_t modifiedTime = fileInfo.st_mtime;
        earliestModifiedTime = modifiedTime;
        latestModifiedTime = earliestModifiedTime;
    }
    // 打印统计结果
    if (folderNode->fileCount != fileCount || folderNode->AllfileSize != totalBytes || folderNode->earliest != earliestModifiedTime || folderNode->latest != latestModifiedTime) {
        std::cout << "有变化, Folder: " << folderNode->path << std::endl;
        std::cout << "文件数差异: " << (fileCount - folderNode->fileCount) << std::endl;
        std::cout << "文件总大小差异: " << static_cast<long long>(totalBytes) - static_cast<long long>(folderNode->AllfileSize) << " bytes" << std::endl;
        std::cout << "Earliest Modified Time: " << std::asctime(std::localtime(&earliestModifiedTime));
        std::cout << "Latest Modified Time: " << std::asctime(std::localtime(&latestModifiedTime));
        std::cout << std::endl;
        logFile << "有变化, Folder: " << folderNode->path << std::endl;
        logFile << "文件数差异: " << (fileCount - folderNode->fileCount) << std::endl;
        logFile << "文件总大小差异: " << static_cast<long long>(totalBytes) - static_cast<long long>(folderNode->AllfileSize) << " bytes" << std::endl;
        logFile << "Earliest Modified Time: " << std::asctime(std::localtime(&earliestModifiedTime));
        logFile << "Latest Modified Time: " << std::asctime(std::localtime(&latestModifiedTime));
        logFile << std::endl;
    }
}

// 从指定结点中删除文件，node 是文件所在文件夹结点，fileName 是带文件名全路径
bool removeFileFromNode(TreeNode* node, const fs::path& fileName) {
    // 遍历文件列表
    std::string filePathString = fileName.string();//文件全路径
    for (auto it = node->files.begin(); it != node->files.end(); ++it) {
        // 将当前文件的全路径转换为字符串
        std::string currentNodePathString = it->path.string();
        if (currentNodePathString.size() >= 11 &&
            currentNodePathString.substr(10) == filePathString.substr(10)) {
            // 从文件列表中删除该文件
            node->files.erase(it);
            return true; // 删除成功
        }
    }
    return false; // 没有找到要删除的文件
}

// 更新文件信息，传入文件全路径，时间，大小
int updateFileAttributes(TreeNode* node, const fs::path& filePath, long int time, long int size) {
    // 遍历文件列表
    std::string filePathString = filePath.string();//文件全路径
    for (auto it = node->files.begin(); it != node->files.end(); ++it) {
        // 将当前文件的全路径转换为字符串
        std::string currentNodePathString = it->path.string();
        if (currentNodePathString.size() >= 11 &&
            currentNodePathString.substr(10) == filePathString.substr(10)) {
            // 找到要更新属性的文件
            it->lastModifiedTime = time;
            it->fileSize = size;
            return 0; // 更新成功
        }
    }
    cout << "更新失败" << filePath << endl;
    return -1; // 文件未找到，更新失败
}

// 创建文件节点并添加到文件列表中,接收一个指向 TreeNode的指针node，以及要创建的文件的全路径 filePath、时间 time 和大小 size
void createFileNode(TreeNode* node, const fs::path& filePath, std::time_t time, size_t size) {
    fs::path parentPath = filePath.parent_path();//找到文件所在文件夹的路径parentPath
    // 检查文件路径是否存在
    if (!fs::exists(parentPath)) {
        std::cerr << "Error: File path does not exist: " << parentPath << std::endl;
        return;
    }

    // 创建 FileInfo 对象并添加到文件列表中
    FileInfo newFile(filePath, time, size);
    node->files.push_back(newFile);
    //std::cerr << "添加成功：" << filePath << std::endl;
}

//删除指定目录的结点，传入根结点root和目录路径folderPath
bool deleteFolderFromNode(TreeNode* root, fs::path folderPath, std::unordered_map<fs::path, TreeNode*>& directoryMap) {
    if (root == nullptr) {
        return false; // 空树
    }

    // 尝试从哈希表中查找目标文件夹路径对应的节点
    auto it = directoryMap.find(folderPath);
    if (it == directoryMap.end()) {
        cout << "未找到目标文件夹：" << folderPath << endl;
        return false; // 未找到目标文件夹
    }
    TreeNode* targetNode = it->second; // 目标文件夹的结点
    /*在哈希表中删除该目录下所有子目录的记录*/
    for (auto kid = directoryMap.begin(); kid != directoryMap.end();) {//遍历 directoryMap 中的所有键值对
        const fs::path& key = kid->first;//获取当前键值对中的键（即路径）key 
        TreeNode* node = kid->second;//获取当前键值对中的值（即节点指针）node
        // 判断当前键是否是目标目录的子目录
        if (key != folderPath && key.string().find(folderPath.string() + "\\") == 0) {
            // 从哈希表中移除该子目录的记录
            kid = directoryMap.erase(kid);
        }
        /*删除了当前键值对后，it 已经指向了被删除元素的下一个元素*/
        else {
            ++kid; // 指向下一个元素
        }
    }
    // 遍历哈希表，寻找目标节点的父节点
    auto itToDelete = directoryMap.end(); // 用于保存要删除的迭代器位置
    for (auto it = directoryMap.begin(); it != directoryMap.end(); ++it) {
        TreeNode* currentNode = it->second;
        // 判断当前节点的 firstChild 是否是目标节点
        if (currentNode->firstChild == targetNode) {
            currentNode->firstChild = targetNode->nextSibling;
            itToDelete = it; // 保存要删除的迭代器位置
            break; // 找到目标节点的父节点，退出循环
        }
        // 判断当前节点的 nextSibling 是否是目标节点
        if (currentNode->nextSibling == targetNode) {
            currentNode->nextSibling = targetNode->nextSibling;
            itToDelete = it; // 保存要删除的迭代器位置
            break; // 找到目标节点的父节点，退出循环
        }
    }

    // 删除哈希表中的目标节点记录
    if (itToDelete != directoryMap.end()) {
        directoryMap.erase(itToDelete);
        delete targetNode;
        return true;
    }

    // 如果没有找到目标节点的父节点，说明删除失败

    return false;
}