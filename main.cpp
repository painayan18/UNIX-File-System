#include <iostream>
#include <string>
#include <cstring>
#include <vector>
using namespace std;

const int DISK_SIZE = 100;
const int BLOCK_SIZE = 512;

struct Directory {
    int back;
    int frwd;
    int free;
    char filler[4];
    struct {
        char type;
        char name[9];
        int link;
        short size;
    } dir[31];
};

struct DataBlock {
    int back;
    int frwd;
    char userData[504];
};

vector<Directory> dirBlocks;
vector<DataBlock> dataBlocks;
int freeBlock;
string currentFile;
int currentPointer;

void initializeFileSystem() {
    dirBlocks.resize(1);
    dataBlocks.resize(DISK_SIZE);
    freeBlock = 1;
    dirBlocks[0].back = 0;
    dirBlocks[0].frwd = 0;
    dirBlocks[0].free = 1;
    for (int i = 0; i < 31; i++) {
        dirBlocks[0].dir[i].type = 'F';
        dirBlocks[0].dir[i].link = 0;
        dirBlocks[0].dir[i].size = 0;
    }
    for (int i = 1; i < DISK_SIZE - 1; i++) {
        dataBlocks[i].back = i - 1;
        dataBlocks[i].frwd = i + 1;
    }
    dataBlocks[DISK_SIZE - 1].back = DISK_SIZE - 2;
    dataBlocks[DISK_SIZE - 1].frwd = 0;
}

void createFile(char type, string name) {
    int dirBlock = 0;
    int entryIndex = -1;
    string path;
    string filePath = name;
    size_t pos = 0;
    while ((pos = filePath.find('/')) != string::npos) {
        path = filePath.substr(0, pos);
        filePath.erase(0, pos + 1);
        bool found = false;
        for (int i = 0; i < 31; i++) {
            if (dirBlocks[dirBlock].dir[i].type == 'D' && strcasecmp(dirBlocks[dirBlock].dir[i].name, path.c_str()) == 0) {
                dirBlock = dirBlocks[dirBlock].dir[i].link;
                found = true;
                break;
            }
        }
        if (!found) {
            if (freeBlock == 0) {
                cout << "Disk is full. Cannot create directory." << endl;
                return;
            }
            entryIndex = -1;
            for (int i = 0; i < 31; i++) {
                if (dirBlocks[dirBlock].dir[i].type == 'F') {
                    entryIndex = i;
                    break;
                }
            }
            if (entryIndex == -1) {
                Directory newDir;
                newDir.back = dirBlock;
                newDir.frwd = 0;
                for (int i = 0; i < 31; i++) {
                    newDir.dir[i].type = 'F';
                    newDir.dir[i].link = 0;
                    newDir.dir[i].size = 0;
                }
                dirBlocks.push_back(newDir);
                dirBlocks[dirBlock].frwd = dirBlocks.size() - 1;
                dirBlock = dirBlocks.size() - 1;
                entryIndex = 0;
            }
            dirBlocks[dirBlock].dir[entryIndex].type = 'D';
            strncpy(dirBlocks[dirBlock].dir[entryIndex].name, path.c_str(), 9);
            dirBlocks[dirBlock].dir[entryIndex].link = freeBlock;
            dirBlocks[dirBlock].dir[entryIndex].size = 0;

            if (freeBlock != 0) {
                freeBlock = dataBlocks[freeBlock].frwd;
                dirBlocks[0].free = freeBlock;
            } else {
                dirBlocks[0].free = 0;
            }
        }
    }
    entryIndex = -1;
    for (int i = 0; i < 31; i++) {
        if (dirBlocks[dirBlock].dir[i].type == 'F') {
            entryIndex = i;
            break;
        }
    }
    if (entryIndex == -1) {
        Directory newDir;
        newDir.back = dirBlock;
        newDir.frwd = 0;
        for (int i = 0; i < 31; i++) {
            newDir.dir[i].type = 'F';
            newDir.dir[i].link = 0;
            newDir.dir[i].size = 0;
        }
        dirBlocks.push_back(newDir);
        dirBlocks[dirBlock].frwd = dirBlocks.size() - 1;
        dirBlock = dirBlocks.size() - 1;
        entryIndex = 0;
    }
    dirBlocks[dirBlock].dir[entryIndex].type = type;
    strncpy(dirBlocks[dirBlock].dir[entryIndex].name, filePath.c_str(), 9);
    dirBlocks[dirBlock].dir[entryIndex].link = freeBlock;
    dirBlocks[dirBlock].dir[entryIndex].size = 0;

    if (freeBlock != 0) {
        freeBlock = dataBlocks[freeBlock].frwd;
        dirBlocks[0].free = freeBlock;
    } else {
        dirBlocks[0].free = 0;
    }

    currentFile = name;
    currentPointer = 0;
}

void openFile(char mode, string name) {
    int dirBlock = 0;
    int entryIndex = -1;
    string path;
    size_t pos = 0;
    while ((pos = name.find('/')) != string::npos) {
        path = name.substr(0, pos);
        name.erase(0, pos + 1);
        bool found = false;
        for (int i = 0; i < 31; i++) {
            if (dirBlocks[dirBlock].dir[i].type == 'D' && strcasecmp(dirBlocks[dirBlock].dir[i].name, path.c_str()) == 0) {
                dirBlock = dirBlocks[dirBlock].dir[i].link;
                found = true;
                break;
            }
        }
        if (!found) {
            cout << "Directory not found." << endl;
            return;
        }
    }
    bool found = false;
    for (int i = 0; i < 31; i++) {
        if (dirBlocks[dirBlock].dir[i].type == 'U' && strcasecmp(dirBlocks[dirBlock].dir[i].name, name.c_str()) == 0) {
            entryIndex = i;
            found = true;
            break;
        }
    }
    if (!found) {
        cout << "File not found." << endl;
        return;
    }
    currentFile = name;
    if (mode == 'I' || mode == 'U') {
        currentPointer = 0;
    } else if (mode == 'O') {
        currentPointer = dirBlocks[dirBlock].dir[entryIndex].size;
    }
}

void closeFile() {
    currentFile = "";
    currentPointer = 0;
}

void deleteFile(string name) {
    int dirBlock = 0;
    int entryIndex = -1;
    string path;
    size_t pos = 0;
    while ((pos = name.find('/')) != string::npos) {
        path = name.substr(0, pos);
        name.erase(0, pos + 1);
        bool found = false;
        for (int i = 0; i < 31; i++) {
            if (dirBlocks[dirBlock].dir[i].type == 'D' && strcmp(dirBlocks[dirBlock].dir[i].name, path.c_str()) == 0) {
                dirBlock = dirBlocks[dirBlock].dir[i].link;
                found = true;
                break;
            }
        }
        if (!found) {
            cout << "File not found." << endl;
            return;
        }
    }
    bool found = false;
    for (int i = 0; i < 31; i++) {
        if ((dirBlocks[dirBlock].dir[i].type == 'U' || dirBlocks[dirBlock].dir[i].type == 'D') && strcmp(dirBlocks[dirBlock].dir[i].name, name.c_str()) == 0) {
            entryIndex = i;
            found = true;
            break;
        }
    }
    if (!found) {
        cout << "File not found." << endl;
        return;
    }
    int blockToDelete = dirBlocks[dirBlock].dir[entryIndex].link;
    dirBlocks[dirBlock].dir[entryIndex].type = 'F';
    dirBlocks[dirBlock].dir[entryIndex].link = 0;
    dirBlocks[dirBlock].dir[entryIndex].size = 0;
    while (blockToDelete != 0) {
        int nextBlock = dataBlocks[blockToDelete].frwd;
        dataBlocks[blockToDelete].frwd = freeBlock;
        freeBlock = blockToDelete;
        blockToDelete = nextBlock;
    }
}

void readData(int n) {
    if (currentFile == "") {
        cout << "No file opened for reading." << endl;
        return;
    }
    int dirBlock = 0;
    int entryIndex = -1;
    string path;
    size_t pos = 0;
    while ((pos = currentFile.find('/')) != string::npos) {
        path = currentFile.substr(0, pos);
        currentFile.erase(0, pos + 1);
        bool found = false;
        for (int i = 0; i < 31; i++) {
            if (dirBlocks[dirBlock].dir[i].type == 'D' && strcmp(dirBlocks[dirBlock].dir[i].name, path.c_str()) == 0) {
                dirBlock = dirBlocks[dirBlock].dir[i].link;
                found = true;
                break;
            }
        }
        if (!found) {
            cout << "File not found." << endl;
            return;
        }
    }
    bool found = false;
    for (int i = 0; i < 31; i++) {
        if (dirBlocks[dirBlock].dir[i].type == 'U' && strcmp(dirBlocks[dirBlock].dir[i].name, currentFile.c_str()) == 0) {
            entryIndex = i;
            found = true;
            break;
        }
    }
    if (!found) {
        cout << "File not found." << endl;
        return;
    }
    int bytesRead = 0;
    int blockToRead = dirBlocks[dirBlock].dir[entryIndex].link;
    while (bytesRead < n && blockToRead != 0) {
        int bytesToRead = min(n - bytesRead, 504);
        cout << string(dataBlocks[blockToRead].userData, bytesToRead);
        bytesRead += bytesToRead;
        currentPointer += bytesToRead;
        blockToRead = dataBlocks[blockToRead].frwd;
    }
    if (bytesRead < n) {
        cout << "\nEnd of file reached." << endl;
    }
}

void writeData(int n, string data) {
    if (currentFile == "") {
        cout << "No file opened for writing." << endl;
        return;
    }
    int dirBlock = 0;
    int entryIndex = -1;
    string path;
    string filePath = currentFile;
    size_t pos = 0;
    while ((pos = filePath.find('/')) != string::npos) {
        path = filePath.substr(0, pos);
        filePath.erase(0, pos + 1);
        bool found = false;
        for (int i = 0; i < 31; i++) {
            if (dirBlocks[dirBlock].dir[i].type == 'D' && strcasecmp(dirBlocks[dirBlock].dir[i].name, path.c_str()) == 0) {
                dirBlock = dirBlocks[dirBlock].dir[i].link;
                found = true;
                break;
            }
        }
        if (!found) {
            cout << "File not found." << endl;
            return;
        }
    }
    bool found = false;
    for (int i = 0; i < 31; i++) {
        if (dirBlocks[dirBlock].dir[i].type == 'U' && strcasecmp(dirBlocks[dirBlock].dir[i].name, filePath.c_str()) == 0) {
            entryIndex = i;
            found = true;
            break;
        }
    }
    if (!found) {
        cout << "File not found." << endl;
        return;
    }
    int bytesWritten = 0;
    int blockToWrite = dirBlocks[dirBlock].dir[entryIndex].link;
    while (bytesWritten < n) {
        int bytesToWrite = min(n - bytesWritten, 504);
        if (blockToWrite == 0) {
            if (freeBlock == 0) {
                cout << "Disk is full. Cannot write more data." << endl;
                break;
            }
            int newBlock = freeBlock;
            freeBlock = dataBlocks[freeBlock].frwd;
            dataBlocks[newBlock].back = dirBlocks[dirBlock].dir[entryIndex].link;
            dataBlocks[newBlock].frwd = 0;
            if (dirBlocks[dirBlock].dir[entryIndex].link != 0) {
                dataBlocks[dirBlocks[dirBlock].dir[entryIndex].link].frwd = newBlock;
            }
            dirBlocks[dirBlock].dir[entryIndex].link = newBlock;
            blockToWrite = newBlock;
        }
        strncpy(dataBlocks[blockToWrite].userData, data.substr(bytesWritten, bytesToWrite).c_str(), bytesToWrite);
        bytesWritten += bytesToWrite;
        currentPointer += bytesToWrite;
        blockToWrite = dataBlocks[blockToWrite].frwd;
    }
    dirBlocks[dirBlock].dir[entryIndex].size = currentPointer;
    dirBlocks[0].free = freeBlock;
    for (int i = freeBlock; i < DISK_SIZE - 1; i++) {
        dataBlocks[i].frwd = i + 1;
    }
    dataBlocks[DISK_SIZE - 1].frwd = 0;
}

void seekPosition(int base, int offset) {
    if (currentFile == "") {
        cout << "No file opened for seeking." << endl;
        return;
    }
    int dirBlock = 0;
    int entryIndex = -1;
    string path;
    size_t pos = 0;
    while ((pos = currentFile.find('/')) != string::npos) {
        path = currentFile.substr(0, pos);
        currentFile.erase(0, pos + 1);
        bool found = false;
        for (int i = 0; i < 31; i++) {
            if (dirBlocks[dirBlock].dir[i].type == 'D' && strcmp(dirBlocks[dirBlock].dir[i].name, path.c_str()) == 0) {
                dirBlock = dirBlocks[dirBlock].dir[i].link;
                found = true;
                break;
            }
        }
        if (!found) {
            cout << "File not found." << endl;
            return;
        }
    }
    bool found = false;
    for (int i = 0; i < 31; i++) {
        if (dirBlocks[dirBlock].dir[i].type == 'U' && strcmp(dirBlocks[dirBlock].dir[i].name, currentFile.c_str()) == 0) {
            entryIndex = i;
            found = true;
            break;
        }
    }
    if (!found) {
        cout << "File not found." << endl;
        return;
    }
    if (base == -1) {
        currentPointer = offset;
    } else if (base == 0) {
        currentPointer += offset;
    } else if (base == 1) {
        currentPointer = dirBlocks[dirBlock].dir[entryIndex].size + offset;
    }
    if (currentPointer < 0) {
        currentPointer = 0;
    } else if (currentPointer > dirBlocks[dirBlock].dir[entryIndex].size) {
        currentPointer = dirBlocks[dirBlock].dir[entryIndex].size;
    }
}

void displayDirectoryRecursive(int dirBlock, int level) {
    for (int i = 0; i < 31; i++) {
        if (dirBlocks[dirBlock].dir[i].type == 'D') {
            for (int j = 0; j < level; j++) {
                cout << "  ";
            }
            cout << dirBlocks[dirBlock].dir[i].name << "/" << endl;
            int subDirBlock = dirBlocks[dirBlock].dir[i].link;
            if (subDirBlock != dirBlock) {
                displayDirectoryRecursive(subDirBlock, level + 1);
            }
        } else if (dirBlocks[dirBlock].dir[i].type == 'U') {
            for (int j = 0; j < level; j++) {
                cout << "  ";
            }
            cout << dirBlocks[dirBlock].dir[i].name << " (Size: " << dirBlocks[dirBlock].dir[i].size << " bytes)" << endl;
        }
    }
}

void displayDirectory() {
    cout << "Directory Structure:" << endl;
    displayDirectoryRecursive(0, 0);
}

void displayDiskStatus() {
    int freeDirectoryBlocks = 0;
    for (int i = 0; i < dirBlocks.size(); i++) {
        bool isFree = true;
        for (int j = 0; j < 31; j++) {
            if (dirBlocks[i].dir[j].type != 'F') {
                isFree = false;
                break;
            }
        }
        if (isFree) {
            freeDirectoryBlocks++;
        }
    }
    int freeDataBlocks = 0;
    int current = freeBlock;
    while (current != 0) {
        freeDataBlocks++;
        current = dataBlocks[current].frwd;
    }
    cout << "Free Directory Blocks: " << freeDirectoryBlocks << endl;
    cout << "Free Data Blocks: " << freeDataBlocks << endl;
}

int main() {
    initializeFileSystem();

    string command;
    while (true) {
        cout << "> ";
        getline(cin, command);
        if (command.substr(0, 6) == "CREATE") {
            char type = command[7];
            string name = command.substr(9);
            createFile(type, name);
        } else if (command.substr(0, 4) == "OPEN") {
            char mode = command[5];
            string name = command.substr(7);
            openFile(mode, name);
        } else if (command == "CLOSE") {
            closeFile();
        } else if (command.substr(0, 6) == "DELETE") {
            string name = command.substr(7);
            deleteFile(name);
        } else if (command.substr(0, 4) == "READ") {
            int n = stoi(command.substr(5));
            readData(n);
        } else if (command.substr(0, 5) == "WRITE") {
            int pos = command.find("\"");
            int n = stoi(command.substr(6, pos - 6));
            string data = command.substr(pos + 1, command.length() - pos - 2);
            writeData(n, data);
        } else if (command.substr(0, 4) == "SEEK") {
            int pos = command.find(" ", 5);
            int base = stoi(command.substr(5, pos - 5));
            int offset = stoi(command.substr(pos + 1));
            seekPosition(base, offset);
        } else if (command == "DIR") {
            displayDirectory();
        } else if (command == "STATUS") {
            displayDiskStatus();
        } else if (command == "EXIT") {
            break;
        } else {
            cout << "Invalid command." << endl;
        }
    }

    return 0;
}