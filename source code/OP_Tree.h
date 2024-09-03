#pragma once
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
#include <unordered_map>
#include "scan_Win.h"
using namespace std;
namespace fs = filesystem;

//传入路径，路径的结点，建立树
void buildTree(const fs::path& rootPath, TreeNode* parentNode, ofstream& logFile, std::unordered_map<fs::path, TreeNode*>& directoryMap);
// 非递归算法遍历孩子兄弟树并计算深度
int bfsTraversalAndDepth(TreeNode* root);

//传入文件夹所在结点,统计目录下文件的信息
void calculateTreeFolderStats(TreeNode* folderNode);

//传入文件夹路径，找到节点
TreeNode* findNode(TreeNode* root, const fs::path& filePath, std::unordered_map<fs::path, TreeNode*>& directoryMap);

//对比信息,传入文件夹所在结点
void compareNode(TreeNode* folderNode, std::ofstream& logFile);

// 从指定结点中删除文件，node 是文件所在文件夹，fileName 是带文件名全路径
bool removeFileFromNode(TreeNode* node, const fs::path& fileName);

// 更新文件信息，传入文件全路径，时间，大小
int updateFileAttributes(TreeNode* node, const fs::path& filePath, long int time, long int size);

// 创建文件节点并添加到文件列表中,接收一个指向 TreeNode的指针node，以及要创建的文件的全路径 filePath、时间 time 和大小 size
void createFileNode(TreeNode* node, const fs::path& filePath, std::time_t time, size_t size);

//删除指定目录的结点，传入根结点root和目录路径folderPath
bool deleteFolderFromNode(TreeNode* root, fs::path folderPath, std::unordered_map<fs::path, TreeNode*>& directoryMap);