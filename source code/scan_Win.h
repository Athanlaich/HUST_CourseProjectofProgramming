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
// Ŀ¼��Ϣ�ṹ
struct DirectoryInfo {
    fs::path path;  // Ŀ¼·��
    int depth;      // Ŀ¼���
    int filesCount; // Ŀ¼���ļ�����
    int subDirectoriesCount; // Ŀ¼����Ŀ¼����
    std::time_t lastModifiedTime; // ��Ŀ¼�������ļ�������޸�ʱ��
};
// �ļ���Ϣ�ṹ
struct FileInfo {
    std::string name;					   // �ļ���
    fs::path path;                          // �ļ�·��
    std::time_t lastModifiedTime; // ����޸�ʱ��
    size_t fileSize;                        // �ļ���С
    FileInfo(const fs::path& p, time_t lmt, size_t size)
        :name(p.filename().string()), path(p), lastModifiedTime(lmt), fileSize(size) {}
};

// �����ֵ����ڵ�����ݽṹ
struct TreeNode {
    fs::path path;  // Ŀ¼·��
    TreeNode* firstChild;         // ��һ�����ӽڵ�ָ��
    TreeNode* nextSibling;        // ��һ���ֵܽڵ�ָ��
    vector<FileInfo> files;       // �ļ���Ϣ
    ///
    size_t AllfileSize;       // �ļ��ܴ�С
    int fileCount; // Ŀ¼���ļ�����
    std::time_t earliest;   // �����޸�ʱ��
    std::time_t latest;     // �����޸�ʱ��
};
void scan_directory(const fs::path& rootPath, string& maxPath, const std::string& sqlDir, const std::string& sqlFile, ofstream& logFile);