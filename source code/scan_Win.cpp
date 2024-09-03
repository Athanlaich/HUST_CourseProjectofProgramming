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
// 将时间转换为 SQL Server DATETIME 格式的字符串
std::string convertToSQLDateTime(std::time_t time) {
    // 将 time_t 转换为 tm 结构
    struct tm* timeinfo;
    timeinfo = std::localtime(&time);

    // 格式化为 SQL Server DATETIME 格式的字符串
    std::stringstream ss;
    ss << std::put_time(timeinfo, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}
// 生成插入文件信息的 SQL 语句

std::string generateFileInsertSQL(const fs::directory_entry& entry, time_t time, const fs::path upper_folder) {
    std::string sql = "INSERT INTO All_Files (file_Path, file_size, last_write_time,upper_folder) VALUES (";
    sql += "'" + entry.path().string() + "', ";
    sql += std::to_string(static_cast<std::int64_t>(fs::file_size(entry))) + ", ";
    sql += "'" + convertToSQLDateTime(time) + "',";
    sql += "'" + upper_folder.string() + "')";
    return sql;
}

// 生成插入目录信息的 SQL 语句
std::string generateInsertSQL(const DirectoryInfo& directory) {
    std::string sql = "INSERT INTO Dir (dir_Path, depth,file_count,folder_count, last_write_time) VALUES (";
    sql += "'" + directory.path.string() + "', ";
    sql += std::to_string(directory.depth) + ", ";
    sql += std::to_string(directory.filesCount) + ", ";
    sql += std::to_string(directory.subDirectoriesCount) + ", ";
    sql += "'" + convertToSQLDateTime(directory.lastModifiedTime) + "')";
    return sql;
}

// 非递归遍历统计目录信息
void scan_directory(const fs::path& rootPath, string& maxPath, const std::string& sqlDir, const std::string& sqlFile, ofstream& logFile) {
    stack<DirectoryInfo> directoryStack;  // 用于存储待处理的目录信息的堆栈
    directoryStack.push({ rootPath, 1, 0, 0 });   // 将根目录信息压入堆栈
    int maxDepth = 0;// 目录深度
    int directoryCount = 0;// 子目录总数
    int fileCount = 0;// 文件总数
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
        // 遍历当前目录
        try {
            int currentDirFilesCount = 0;// 当前目录中文件总数
            int currentDirSubDirectoriesCount = 0;// 当前目录中子目录总数
            // //directory_iterator是指定目录的输入迭代器，遍历顺序不指定，每个目录只遍历一次。对于特殊目录如"."和".."则会跳过
            for (const auto& entry : fs::directory_iterator(currentDir.path)) {// 遍历当前目录
                if (entry.is_directory()) {// 如果是目录，则将目录信息压入堆栈
                    if (entry.path().filename().string() == "." || entry.path().filename().string() == "..") {
                        continue;
                    }
                    directoryStack.push({ entry.path(), currentDir.depth + 1, 0, 0 });
                    currentDirSubDirectoriesCount++;
                    directoryCount++;
                }
                else {// 如果是文件
                    fileCount++;
                    currentDirFilesCount++;

                    string filePath = entry.path().string();
                    if (filePath.length() > maxPathLength) {
                        maxPathLength = filePath.length();
                        maxPath = filePath;
                    }
                    stat(entry.path().string().c_str(), &file_Info);
                    // 获取最后修改时间
                    std::time_t ModifiedTime = file_Info.st_mtime;
                    if (ModifiedTime > lastModifiedTime) {
                        lastModifiedTime = ModifiedTime;
                    }
                    std::string insertfileSQL = generateFileInsertSQL(entry, ModifiedTime, currentDir.path);

                    sql_File << insertfileSQL << ";" << std::endl;// 写入 SQL 语句
                }
            }
            // 更新目录中的文件总数和子目录总数
            currentDir.filesCount = currentDirFilesCount;
            currentDir.subDirectoriesCount = currentDirSubDirectoriesCount;
            currentDir.lastModifiedTime = lastModifiedTime;
            // 将时间转换为 SQL Server DATETIME 格式的字符串
            std::string sqlDateTime = convertToSQLDateTime(lastModifiedTime);
            // 生成并写入 SQL 语句
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
    // 关闭文件
    sql_DIR.close();
    sql_File.close();
    cout << "子目录总数：" << directoryCount << endl;
    cout << "文件总数：" << fileCount << endl;
    cout << "目录深度：" << maxDepth << endl;
    cout << "最长文件路径字符数：" << maxPathLength << endl << "路径名称" << maxPath << endl;
    cout << "统计完成！" << endl;
    logFile << "子目录总数：" << directoryCount << endl;
    logFile << "文件总数：" << fileCount << endl;
    logFile << "目录深度：" << maxDepth << endl;
    logFile << "最长文件路径字符数：" << maxPathLength << endl << "路径名称" << maxPath << endl;
    logFile << "统计完成！" << endl;
}