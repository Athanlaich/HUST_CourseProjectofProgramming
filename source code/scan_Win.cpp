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
using namespace std;
namespace fs = filesystem;
struct stat file_Info = {};
// ��ʱ��ת��Ϊ SQL Server DATETIME ��ʽ���ַ���
std::string convertToSQLDateTime(std::time_t time) {
    // �� time_t ת��Ϊ tm �ṹ
    struct tm* timeinfo;
    timeinfo = std::localtime(&time);

    // ��ʽ��Ϊ SQL Server DATETIME ��ʽ���ַ���
    std::stringstream ss;
    ss << std::put_time(timeinfo, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}
// ���ɲ����ļ���Ϣ�� SQL ���

std::string generateFileInsertSQL(const fs::directory_entry& entry, time_t time, const fs::path upper_folder) {
    std::string sql = "INSERT INTO All_Files (file_Path, file_size, last_write_time,upper_folder) VALUES (";
    sql += "'" + entry.path().string() + "', ";
    sql += std::to_string(static_cast<std::int64_t>(fs::file_size(entry))) + ", ";
    sql += "'" + convertToSQLDateTime(time) + "',";
    sql += "'" + upper_folder.string() + "')";
    return sql;
}

// ���ɲ���Ŀ¼��Ϣ�� SQL ���
std::string generateInsertSQL(const DirectoryInfo& directory) {
    std::string sql = "INSERT INTO Dir (dir_Path, depth,file_count,folder_count, last_write_time) VALUES (";
    sql += "'" + directory.path.string() + "', ";
    sql += std::to_string(directory.depth) + ", ";
    sql += std::to_string(directory.filesCount) + ", ";
    sql += std::to_string(directory.subDirectoriesCount) + ", ";
    sql += "'" + convertToSQLDateTime(directory.lastModifiedTime) + "')";
    return sql;
}

// �ǵݹ����ͳ��Ŀ¼��Ϣ
void scan_directory(const fs::path& rootPath, string& maxPath, const std::string& sqlDir, const std::string& sqlFile, ofstream& logFile) {
    stack<DirectoryInfo> directoryStack;  // ���ڴ洢�������Ŀ¼��Ϣ�Ķ�ջ
    directoryStack.push({ rootPath, 1, 0, 0 });   // ����Ŀ¼��Ϣѹ���ջ
    int maxDepth = 0;// Ŀ¼���
    int directoryCount = 0;// ��Ŀ¼����
    int fileCount = 0;// �ļ�����
    size_t maxPathLength = 0;
    std::ofstream sql_DIR(sqlDir);
    if (!sql_DIR.is_open()) {
        std::cerr << "Failed to create/open SQL file for writing." << std::endl;
        return;
    }
    std::ofstream sql_File(sqlFile);
    if (!sql_File.is_open()) {
        std::cerr << "Failed to create/open SQL file for writing." << std::endl;
        return;
    }
    while (!directoryStack.empty()) {
        DirectoryInfo currentDir = directoryStack.top();
        directoryStack.pop();
        maxDepth = max(maxDepth, currentDir.depth);
        std::time_t lastModifiedTime = 0;
        // ������ǰĿ¼
        try {
            int currentDirFilesCount = 0;// ��ǰĿ¼���ļ�����
            int currentDirSubDirectoriesCount = 0;// ��ǰĿ¼����Ŀ¼����
            // //directory_iterator��ָ��Ŀ¼�����������������˳��ָ����ÿ��Ŀ¼ֻ����һ�Ρ���������Ŀ¼��"."��".."�������
            for (const auto& entry : fs::directory_iterator(currentDir.path)) {// ������ǰĿ¼
                if (entry.is_directory()) {// �����Ŀ¼����Ŀ¼��Ϣѹ���ջ
                    if (entry.path().filename().string() == "." || entry.path().filename().string() == "..") {
                        continue;
                    }
                    directoryStack.push({ entry.path(), currentDir.depth + 1, 0, 0 });
                    currentDirSubDirectoriesCount++;
                    directoryCount++;
                }
                else {// ������ļ�
                    fileCount++;
                    currentDirFilesCount++;

                    string filePath = entry.path().string();
                    if (filePath.length() > maxPathLength) {
                        maxPathLength = filePath.length();
                        maxPath = filePath;
                    }
                    stat(entry.path().string().c_str(), &file_Info);
                    // ��ȡ����޸�ʱ��
                    std::time_t ModifiedTime = file_Info.st_mtime;
                    if (ModifiedTime > lastModifiedTime) {
                        lastModifiedTime = ModifiedTime;
                    }
                    std::string insertfileSQL = generateFileInsertSQL(entry, ModifiedTime, currentDir.path);

                    sql_File << insertfileSQL << ";" << std::endl;// д�� SQL ���
                }
            }
            // ����Ŀ¼�е��ļ���������Ŀ¼����
            currentDir.filesCount = currentDirFilesCount;
            currentDir.subDirectoriesCount = currentDirSubDirectoriesCount;
            currentDir.lastModifiedTime = lastModifiedTime;
            // ��ʱ��ת��Ϊ SQL Server DATETIME ��ʽ���ַ���
            std::string sqlDateTime = convertToSQLDateTime(lastModifiedTime);
            // ���ɲ�д�� SQL ���
            std::string insertSQL = generateInsertSQL(currentDir);

            sql_DIR << insertSQL << ";" << std::endl;
        }
        catch (const fs::filesystem_error& e) {
            if (e.code().value() == ERROR_ACCESS_DENIED) {
                cerr << "Access denied error accessing directory: " << currentDir.path << endl;
                

                continue;
            }
            else {
                cerr << "Filesystem error accessing directory: " << e.what() << endl;
                logFile << "Filesystem error accessing directory: " << e.what() << endl;
                break;
            }
        }
    }
    // �ر��ļ�
    sql_DIR.close();
    sql_File.close();
    cout << "��Ŀ¼������" << directoryCount << endl;
    cout << "�ļ�������" << fileCount << endl;
    cout << "Ŀ¼��ȣ�" << maxDepth << endl;
    cout << "��ļ�·���ַ�����" << maxPathLength << endl << "·������" << maxPath << endl;
    cout << "ͳ����ɣ�" << endl;
    logFile << "��Ŀ¼������" << directoryCount << endl;
    logFile << "�ļ�������" << fileCount << endl;
    logFile << "Ŀ¼��ȣ�" << maxDepth << endl;
    logFile << "��ļ�·���ַ�����" << maxPathLength << endl << "·������" << maxPath << endl;
    logFile << "ͳ����ɣ�" << endl;
}