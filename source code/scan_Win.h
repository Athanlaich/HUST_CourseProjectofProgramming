#pragma once
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
using namespace std;
namespace fs = filesystem;
// 目录信息结构
struct DirectoryInfo {
    fs::path path;  // 目录路径
    int depth;      // 目录深度
    int filesCount; // 目录中文件总数
    int subDirectoriesCount; // 目录中子目录总数
    std::time_t lastModifiedTime; // 该目录下所有文件的最后修改时间
};
// 文件信息结构
struct FileInfo {
    std::string name;					   // 文件名
    fs::path path;                          // 文件路径
    std::time_t lastModifiedTime; // 最后修改时间
    size_t fileSize;                        // 文件大小
    FileInfo(const fs::path& p, time_t lmt, size_t size)
        :name(p.filename().string()), path(p), lastModifiedTime(lmt), fileSize(size) {}
};

// 长子兄弟树节点的数据结构
struct TreeNode {
    fs::path path;  // 目录路径
    TreeNode* firstChild;         // 第一个孩子节点指针
    TreeNode* nextSibling;        // 下一个兄弟节点指针
    vector<FileInfo> files;       // 文件信息
    ///
    size_t AllfileSize;       // 文件总大小
    int fileCount; // 目录中文件总数
    std::time_t earliest;   // 最早修改时间
    std::time_t latest;     // 最晚修改时间
};
void scan_directory(const fs::path& rootPath, string& maxPath, const std::string& sqlDir, const std::string& sqlFile, ofstream& logFile);