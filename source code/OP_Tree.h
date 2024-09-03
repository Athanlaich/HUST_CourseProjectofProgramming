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

//����·����·���Ľ�㣬������
void buildTree(const fs::path& rootPath, TreeNode* parentNode, ofstream& logFile, std::unordered_map<fs::path, TreeNode*>& directoryMap);
// �ǵݹ��㷨���������ֵ������������
int bfsTraversalAndDepth(TreeNode* root);

//�����ļ������ڽ��,ͳ��Ŀ¼���ļ�����Ϣ
void calculateTreeFolderStats(TreeNode* folderNode);

//�����ļ���·�����ҵ��ڵ�
TreeNode* findNode(TreeNode* root, const fs::path& filePath, std::unordered_map<fs::path, TreeNode*>& directoryMap);

//�Ա���Ϣ,�����ļ������ڽ��
void compareNode(TreeNode* folderNode, std::ofstream& logFile);

// ��ָ�������ɾ���ļ���node ���ļ������ļ��У�fileName �Ǵ��ļ���ȫ·��
bool removeFileFromNode(TreeNode* node, const fs::path& fileName);

// �����ļ���Ϣ�������ļ�ȫ·����ʱ�䣬��С
int updateFileAttributes(TreeNode* node, const fs::path& filePath, long int time, long int size);

// �����ļ��ڵ㲢��ӵ��ļ��б���,����һ��ָ�� TreeNode��ָ��node���Լ�Ҫ�������ļ���ȫ·�� filePath��ʱ�� time �ʹ�С size
void createFileNode(TreeNode* node, const fs::path& filePath, std::time_t time, size_t size);

//ɾ��ָ��Ŀ¼�Ľ�㣬��������root��Ŀ¼·��folderPath
bool deleteFolderFromNode(TreeNode* root, fs::path folderPath, std::unordered_map<fs::path, TreeNode*>& directoryMap);