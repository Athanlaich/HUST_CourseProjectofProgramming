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
#include "scan_Win.h"
#include "OP_Tree.h"
using namespace std;
namespace fs = filesystem;

//读入mystat文件
void stat_file(TreeNode* root, const std::string& filename,  std::unordered_map<fs::path, TreeNode*>& directoryMap);

// 模拟文件操作函数,对于每一条操作，传入root根节点，文件的全路径filePath，操作类型operation，时间，大小
void simulateFileOperation(TreeNode* root, fs::path filePath, const std::string& operation, long int time, long int size, std::unordered_map<fs::path, TreeNode*>& directoryMap);

// 从myfile.txt文件中读取操作并模拟
void readOperationsAndSimulate(TreeNode* root, const std::string& filename, std::unordered_map<fs::path, TreeNode*>& directoryMap);

// 从mydir.txt文件中读取操作并模拟
void DirOperation(TreeNode* root, const std::string& filename, std::unordered_map<fs::path, TreeNode*>& directoryMap);

//读入mystat，对比前后变化，包括文件数，文件总大小，最早修改时间，最晚修改时间
void compareInfo(TreeNode* root, const std::string& filename,std::ofstream& logFile, std::unordered_map<fs::path, TreeNode*>& directoryMap);