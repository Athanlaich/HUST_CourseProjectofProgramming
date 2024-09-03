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

//����mystat�ļ�
void stat_file(TreeNode* root, const std::string& filename,  std::unordered_map<fs::path, TreeNode*>& directoryMap);

// ģ���ļ���������,����ÿһ������������root���ڵ㣬�ļ���ȫ·��filePath����������operation��ʱ�䣬��С
void simulateFileOperation(TreeNode* root, fs::path filePath, const std::string& operation, long int time, long int size, std::unordered_map<fs::path, TreeNode*>& directoryMap);

// ��myfile.txt�ļ��ж�ȡ������ģ��
void readOperationsAndSimulate(TreeNode* root, const std::string& filename, std::unordered_map<fs::path, TreeNode*>& directoryMap);

// ��mydir.txt�ļ��ж�ȡ������ģ��
void DirOperation(TreeNode* root, const std::string& filename, std::unordered_map<fs::path, TreeNode*>& directoryMap);

//����mystat���Ա�ǰ��仯�������ļ������ļ��ܴ�С�������޸�ʱ�䣬�����޸�ʱ��
void compareInfo(TreeNode* root, const std::string& filename,std::ofstream& logFile, std::unordered_map<fs::path, TreeNode*>& directoryMap);