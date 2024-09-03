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
//����·����·���Ľ��
void buildTree(const fs::path& rootPath, TreeNode* parentNode, ofstream& logFile, std::unordered_map<fs::path, TreeNode*>& directoryMap) {
    try {
        for (const auto& entry : fs::directory_iterator(rootPath)) {
            if (fs::is_directory(entry)) {
                TreeNode* newNode = new TreeNode{ entry.path(), {}, nullptr, {},0,0,0,0 };//·�������ӣ��ֵܣ��ļ���Ϣ���ļ��ܴ�С���ļ������������޸�ʱ�䣬�����޸�ʱ��
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
            else {//������ļ�
                stat(entry.path().string().c_str(), &fileInfo);
                // ��ȡ����޸�ʱ��
                std::time_t ModifiedTime = fileInfo.st_mtime;
                size_t fileSize = fs::file_size(entry);
                // ����ļ���Ϣ�����ڵ���ļ��б���
                parentNode->files.push_back(FileInfo(entry.path(), ModifiedTime, fileSize));
            }
        }
        // �ڹ�ϣ���д洢Ŀ¼·���Ͷ�Ӧ�Ľڵ�ָ��
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

// �ǵݹ��㷨���������ֵ������������
int bfsTraversalAndDepth(TreeNode* root) {
    if (root == nullptr) {
        return 0;
    }
    // ����һ���������ڲ�α���
    queue<TreeNode*> nodeQueue;
    nodeQueue.push(root);
    int depth = 0;

    while (!nodeQueue.empty()) {
        int levelSize = nodeQueue.size(); // ��ǰ��Ľڵ���
        depth++; // ������һ��

        // ������ǰ��Ľڵ�
        for (int i = 0; i < levelSize; i++) {
            TreeNode* currentNode = nodeQueue.front();
            nodeQueue.pop();

            // ����ǰ�ڵ�ĺ��ӽڵ����
            if (currentNode->firstChild != nullptr) {
                nodeQueue.push(currentNode->firstChild);
            }

            // ����ǰ�ڵ���ֵܽڵ����
            if (currentNode->nextSibling != nullptr) {
                nodeQueue.push(currentNode->nextSibling);
            }
        }
    }

    return depth;
}

//�����ļ������ڽ��,ͳ��Ŀ¼���ļ�����Ϣ
void calculateTreeFolderStats(TreeNode* folderNode) {
    if (folderNode == nullptr) {
        std::cerr << "Error: Folder node is nullptr." << std::endl;
        return;
    }
    // ��ʼ��ͳ����Ϣ
    int fileCount = 0;
    size_t totalBytes = 0;
    std::time_t earliestModifiedTime = 0; // ��ʼ��Ϊ 0
    std::time_t latestModifiedTime = 0;   // ��ʼ��Ϊ 0
    bool isFirstFile = true; // ����Ƿ��ǵ�һ���ļ�
    bool directoryIsEmpty = true;//���Ŀ¼�Ƿ�Ϊ��
    // �����ļ��б�ͳ����Ϣ
    // ����Ŀ¼�µ��ļ�
    for (const auto& entry : fs::directory_iterator(folderNode->path)) {
        if (fs::is_regular_file(entry)) {
            directoryIsEmpty = false;
            fileCount++;
            totalBytes += fs::file_size(entry);
            stat(entry.path().string().c_str(), &fileInfo);
            // ��ȡ����޸�ʱ��
            std::time_t modifiedTime = fileInfo.st_mtime;//����޸�ʱ��
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
    folderNode->AllfileSize = totalBytes;//�ļ��ܴ�С
    folderNode->fileCount = fileCount;//�ļ�����
    // ���Ŀ¼Ϊ�գ��� earliestModifiedTime �� latestModifiedTime ����ΪĿ¼�Ĵ���ʱ��
    if (directoryIsEmpty) {
        stat(folderNode->path.string().c_str(), &fileInfo);
        // ��ȡ����޸�ʱ��
        std::time_t modifiedTime = fileInfo.st_mtime;
        earliestModifiedTime = modifiedTime;
        latestModifiedTime = earliestModifiedTime;
    }
    folderNode->earliest = earliestModifiedTime;//�����޸�ʱ��
    folderNode->latest = latestModifiedTime;//�����޸�ʱ��
    // ��ӡͳ�ƽ��
    std::cout << "Folder: " << folderNode->path << std::endl;
    std::cout << "File Count: " << fileCount << std::endl;
    std::cout << "Total Bytes: " << totalBytes << std::endl;
    if (fileCount > 0) {
        std::cout << "Earliest Modified Time: " << std::asctime(std::localtime(&earliestModifiedTime));
        std::cout << "Latest Modified Time: " << std::asctime(std::localtime(&latestModifiedTime));
    }
    std::cout << std::endl;
}
//ͨ���ļ���·�����ҽ��
TreeNode* findNode(TreeNode* root, const fs::path& filePath, std::unordered_map<fs::path, TreeNode*>& directoryMap) {
    // ������ڵ�Ϊ�գ���ֱ�ӷ��ؿ�ָ��
    if (root == nullptr) {

        return nullptr;
    }
    // ���Դӹ�ϣ���в���Ŀ���ļ���·����Ӧ�Ľڵ�
    auto it = directoryMap.find(filePath);
    if (it != directoryMap.end()) {
        return it->second; // �ҵ�Ŀ���ļ���·���Ľ��
    }
    std::queue<TreeNode*> nodeQueue;
    nodeQueue.push(root);
    std::string filePathString = filePath.string();
    while (!nodeQueue.empty()) {
        TreeNode* currentNode = nodeQueue.front();
        nodeQueue.pop();
        // ����ǰ�ڵ��·��ת��Ϊ�ַ���
        std::string currentNodePathString = currentNode->path.string();
        // ��鵱ǰ�ڵ��·���Ƿ�ƥ��Ŀ���ļ���·��
        if (currentNodePathString.size() >= 11 &&
            currentNodePathString.substr(10) == filePathString.substr(10)) {
            return currentNode; // �ҵ�Ŀ���ļ���·��
        }

        // ����ǰ�ڵ�ĺ��ӽڵ����
        if (currentNode->firstChild != nullptr) {
            nodeQueue.push(currentNode->firstChild);
        }

        // ����ǰ�ڵ���ֵܽڵ����
        if (currentNode->nextSibling != nullptr) {
            nodeQueue.push(currentNode->nextSibling);
        }
    }
    // �����������������δ�ҵ�Ŀ���ļ��У��򷵻ؿ�ָ��
    cout << "δ�ҵ�Ŀ�����ļ��У�" << filePath << endl;
    return nullptr;
}

//�Ա���Ϣ,�����ļ������ڽ��
void compareNode(TreeNode* folderNode, std::ofstream& logFile) {
    if (folderNode == nullptr) {
        std::cerr << "Error: Folder node is nullptr." << std::endl;
        //logFile << "Error: Folder node is nullptr." << std::endl;
        return ;
    }
    // ��ʼ��ͳ����Ϣ
    int fileCount = 0;
    size_t totalBytes = 0;
    std::time_t earliestModifiedTime = 0; // ��ʼ��Ϊ 0
    std::time_t latestModifiedTime = 0;   // ��ʼ��Ϊ 0
    bool isFirstFile = true; // ����Ƿ��ǵ�һ���ļ�
    bool directoryIsEmpty = true;//���Ŀ¼�Ƿ�Ϊ��
    // �����ļ��б�ͳ����Ϣ
    for (const auto& fileInfo : folderNode->files) {
        //if (fs::is_regular_file(fileInfo.path)) {
            fileCount++;
            totalBytes += fileInfo.fileSize;
            // ��ȡ����޸�ʱ��
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
    // ���Ŀ¼Ϊ�գ��� earliestModifiedTime �� latestModifiedTime ����ΪĿ¼�Ĵ���ʱ��
    if (fileCount == 0) {
        stat(folderNode->path.string().c_str(), &fileInfo);
        // ��ȡ����޸�ʱ��
        std::time_t modifiedTime = fileInfo.st_mtime;
        earliestModifiedTime = modifiedTime;
        latestModifiedTime = earliestModifiedTime;
    }
    // ��ӡͳ�ƽ��
    if (folderNode->fileCount != fileCount || folderNode->AllfileSize != totalBytes || folderNode->earliest != earliestModifiedTime || folderNode->latest != latestModifiedTime) {
        std::cout << "�б仯, Folder: " << folderNode->path << std::endl;
        std::cout << "�ļ�������: " << (fileCount - folderNode->fileCount) << std::endl;
        std::cout << "�ļ��ܴ�С����: " << static_cast<long long>(totalBytes) - static_cast<long long>(folderNode->AllfileSize) << " bytes" << std::endl;
        std::cout << "Earliest Modified Time: " << std::asctime(std::localtime(&earliestModifiedTime));
        std::cout << "Latest Modified Time: " << std::asctime(std::localtime(&latestModifiedTime));
        std::cout << std::endl;
        logFile << "�б仯, Folder: " << folderNode->path << std::endl;
        logFile << "�ļ�������: " << (fileCount - folderNode->fileCount) << std::endl;
        logFile << "�ļ��ܴ�С����: " << static_cast<long long>(totalBytes) - static_cast<long long>(folderNode->AllfileSize) << " bytes" << std::endl;
        logFile << "Earliest Modified Time: " << std::asctime(std::localtime(&earliestModifiedTime));
        logFile << "Latest Modified Time: " << std::asctime(std::localtime(&latestModifiedTime));
        logFile << std::endl;
    }
}

// ��ָ�������ɾ���ļ���node ���ļ������ļ��н�㣬fileName �Ǵ��ļ���ȫ·��
bool removeFileFromNode(TreeNode* node, const fs::path& fileName) {
    // �����ļ��б�
    std::string filePathString = fileName.string();//�ļ�ȫ·��
    for (auto it = node->files.begin(); it != node->files.end(); ++it) {
        // ����ǰ�ļ���ȫ·��ת��Ϊ�ַ���
        std::string currentNodePathString = it->path.string();
        if (currentNodePathString.size() >= 11 &&
            currentNodePathString.substr(10) == filePathString.substr(10)) {
            // ���ļ��б���ɾ�����ļ�
            node->files.erase(it);
            return true; // ɾ���ɹ�
        }
    }
    return false; // û���ҵ�Ҫɾ�����ļ�
}

// �����ļ���Ϣ�������ļ�ȫ·����ʱ�䣬��С
int updateFileAttributes(TreeNode* node, const fs::path& filePath, long int time, long int size) {
    // �����ļ��б�
    std::string filePathString = filePath.string();//�ļ�ȫ·��
    for (auto it = node->files.begin(); it != node->files.end(); ++it) {
        // ����ǰ�ļ���ȫ·��ת��Ϊ�ַ���
        std::string currentNodePathString = it->path.string();
        if (currentNodePathString.size() >= 11 &&
            currentNodePathString.substr(10) == filePathString.substr(10)) {
            // �ҵ�Ҫ�������Ե��ļ�
            it->lastModifiedTime = time;
            it->fileSize = size;
            return 0; // ���³ɹ�
        }
    }
    cout << "����ʧ��" << filePath << endl;
    return -1; // �ļ�δ�ҵ�������ʧ��
}

// �����ļ��ڵ㲢��ӵ��ļ��б���,����һ��ָ�� TreeNode��ָ��node���Լ�Ҫ�������ļ���ȫ·�� filePath��ʱ�� time �ʹ�С size
void createFileNode(TreeNode* node, const fs::path& filePath, std::time_t time, size_t size) {
    fs::path parentPath = filePath.parent_path();//�ҵ��ļ������ļ��е�·��parentPath
    // ����ļ�·���Ƿ����
    if (!fs::exists(parentPath)) {
        std::cerr << "Error: File path does not exist: " << parentPath << std::endl;
        return;
    }

    // ���� FileInfo ������ӵ��ļ��б���
    FileInfo newFile(filePath, time, size);
    node->files.push_back(newFile);
    //std::cerr << "��ӳɹ���" << filePath << std::endl;
}

//ɾ��ָ��Ŀ¼�Ľ�㣬��������root��Ŀ¼·��folderPath
bool deleteFolderFromNode(TreeNode* root, fs::path folderPath, std::unordered_map<fs::path, TreeNode*>& directoryMap) {
    if (root == nullptr) {
        return false; // ����
    }

    // ���Դӹ�ϣ���в���Ŀ���ļ���·����Ӧ�Ľڵ�
    auto it = directoryMap.find(folderPath);
    if (it == directoryMap.end()) {
        cout << "δ�ҵ�Ŀ���ļ��У�" << folderPath << endl;
        return false; // δ�ҵ�Ŀ���ļ���
    }
    TreeNode* targetNode = it->second; // Ŀ���ļ��еĽ��
    /*�ڹ�ϣ����ɾ����Ŀ¼��������Ŀ¼�ļ�¼*/
    for (auto kid = directoryMap.begin(); kid != directoryMap.end();) {//���� directoryMap �е����м�ֵ��
        const fs::path& key = kid->first;//��ȡ��ǰ��ֵ���еļ�����·����key 
        TreeNode* node = kid->second;//��ȡ��ǰ��ֵ���е�ֵ�����ڵ�ָ�룩node
        // �жϵ�ǰ���Ƿ���Ŀ��Ŀ¼����Ŀ¼
        if (key != folderPath && key.string().find(folderPath.string() + "\\") == 0) {
            // �ӹ�ϣ�����Ƴ�����Ŀ¼�ļ�¼
            kid = directoryMap.erase(kid);
        }
        /*ɾ���˵�ǰ��ֵ�Ժ�it �Ѿ�ָ���˱�ɾ��Ԫ�ص���һ��Ԫ��*/
        else {
            ++kid; // ָ����һ��Ԫ��
        }
    }
    // ������ϣ��Ѱ��Ŀ��ڵ�ĸ��ڵ�
    auto itToDelete = directoryMap.end(); // ���ڱ���Ҫɾ���ĵ�����λ��
    for (auto it = directoryMap.begin(); it != directoryMap.end(); ++it) {
        TreeNode* currentNode = it->second;
        // �жϵ�ǰ�ڵ�� firstChild �Ƿ���Ŀ��ڵ�
        if (currentNode->firstChild == targetNode) {
            currentNode->firstChild = targetNode->nextSibling;
            itToDelete = it; // ����Ҫɾ���ĵ�����λ��
            break; // �ҵ�Ŀ��ڵ�ĸ��ڵ㣬�˳�ѭ��
        }
        // �жϵ�ǰ�ڵ�� nextSibling �Ƿ���Ŀ��ڵ�
        if (currentNode->nextSibling == targetNode) {
            currentNode->nextSibling = targetNode->nextSibling;
            itToDelete = it; // ����Ҫɾ���ĵ�����λ��
            break; // �ҵ�Ŀ��ڵ�ĸ��ڵ㣬�˳�ѭ��
        }
    }

    // ɾ����ϣ���е�Ŀ��ڵ��¼
    if (itToDelete != directoryMap.end()) {
        directoryMap.erase(itToDelete);
        delete targetNode;
        return true;
    }

    // ���û���ҵ�Ŀ��ڵ�ĸ��ڵ㣬˵��ɾ��ʧ��

    return false;
}