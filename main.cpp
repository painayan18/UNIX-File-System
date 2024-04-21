#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstring>

using namespace std;

const int DISK_SIZE = 100;
const int BLOCK_SIZE = 512;
const int MAX_DIR_ENTRIES = 31;
const int MAX_FILENAME_SIZE = 9;

struct DirectoryEntry {
    char type;
    char name[MAX_FILENAME_SIZE + 1];
    int link;
    short size;
};

struct DirectoryBlock {
    int BACK;
    int FRWD;
    int FREE;
    char filler[4];
    DirectoryEntry dir[MAX_DIR_ENTRIES];
};

struct DataBlock {
    int BACK;
    int FRWD;
    char user_data[BLOCK_SIZE - 8];
};

union Block {
    DirectoryBlock dir_block;
    DataBlock data_block;
};

vector<Block> disk(DISK_SIZE);
unordered_map<string, int> open_files;
int current_file_block = -1;
int current_file_offset = 0;

void init_disk();
int allocate_block();
void free_block(int);
int find_free_dir_entry(int);
int find_dir_entry(int, const string&);
void create_file(char, const string&);
void open_file(char, const string&);
void close_file();
void delete_file(const string&);
void read_file(int);
void write_file(int, const string&);
void seek_file(int, int);
void display_directory(int, int);
int count_free_blocks();

int main() {
    init_disk();

    string command;
    while (cin >> command) {
        if (command == "CREATE") {
            char type;
            string name;
            cin >> type >> name;
            create_file(type, name);
        } else if (command == "OPEN") {
            char mode;
            string name;
            cin >> mode >> name;
            open_file(mode, name);
        } else if (command == "CLOSE") {
            close_file();
        } else if (command == "DELETE") {
            string name;
            cin >> name;
            delete_file(name);
        } else if (command == "READ") {
            int n;
            cin >> n;
            read_file(n);
        } else if (command == "WRITE") {
            int n;
            string data;
            cin >> n;
            getline(cin, data);
            write_file(n, data.substr(1));
        } else if (command == "SEEK") {
            int base, offset;
            cin >> base >> offset;
            seek_file(base, offset);
        }
    }

    cout << "\nDirectory structure:" << endl;
    display_directory(0, 0);

    cout << "\nFree blocks: " << count_free_blocks() << endl;

    return 0;
}

void init_disk() {
    disk[0].dir_block.BACK = 0;
    disk[0].dir_block.FRWD = 0;
    disk[0].dir_block.FREE = 1;
    for (int i = 1; i < DISK_SIZE; i++) {
        disk[i].data_block.FRWD = i + 1;
    }
    disk[DISK_SIZE - 1].data_block.FRWD = 0;
}

int allocate_block() {
    int block_num = disk[0].dir_block.FREE;
    if (block_num != 0) {
        disk[0].dir_block.FREE = disk[block_num].data_block.FRWD;
    }
    return block_num;
}

void free_block(int block_num) {
    disk[block_num].data_block.FRWD = disk[0].dir_block.FREE;
    disk[0].dir_block.FREE = block_num;
}

int find_free_dir_entry(int dir_block) {
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (disk[dir_block].dir_block.dir[i].type == 'F') {
            return i;
        }
    }
    if (disk[dir_block].dir_block.FRWD != 0) {
        return find_free_dir_entry(disk[dir_block].dir_block.FRWD);
    }
    int new_dir_block = allocate_block();
    if (new_dir_block == 0) {
        return -1;
    }
    disk[dir_block].dir_block.FRWD = new_dir_block;
    disk[new_dir_block].dir_block.BACK = dir_block;
    disk[new_dir_block].dir_block.FRWD = 0;
    return find_free_dir_entry(new_dir_block);
}

int find_dir_entry(int dir_block, const string& name) {
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (disk[dir_block].dir_block.dir[i].type != 'F' &&
            string(disk[dir_block].dir_block.dir[i].name) == name) {
            return i;
        }
    }
    if (disk[dir_block].dir_block.FRWD != 0) {
        return find_dir_entry(disk[dir_block].dir_block.FRWD, name);
    }
    return -1;
}

void create_file(char type, const string& name) {
    int dir_block = 0;
    size_t pos = name.find_last_of('/');
    if (pos != string::npos) {
        string dir_name = name.substr(0, pos);
        dir_block = find_dir_entry(0, dir_name);
        if (dir_block == -1) {
            cout << "Directory not found: " << dir_name << endl;
            return;
        }
    }
    int entry_index = find_free_dir_entry(dir_block);
    if (entry_index == -1) {
        cout << "No FREE directory entries available." << endl;
        return;
    }
    DirectoryEntry& entry = disk[dir_block].dir_block.dir[entry_index];
    entry.type = type;
    strncpy(entry.name, name.c_str(), MAX_FILENAME_SIZE);
    entry.link = allocate_block();
    entry.size = 0;
    open_files[name] = entry.link;
    current_file_block = entry.link;
    current_file_offset = 0;
}

void open_file(char mode, const string& name) {
    int dir_block = 0;
    size_t pos = name.find_last_of('/');
    if (pos != string::npos) {
        string dir_name = name.substr(0, pos);
        dir_block = find_dir_entry(0, dir_name);
        if (dir_block == -1) {
            cout << "Directory not found: " << dir_name << endl;
            return;
        }
    }
    int entry_index = find_dir_entry(dir_block, name);
    if (entry_index == -1) {
        cout << "File not found: " << name << endl;
        return;
    }
    DirectoryEntry& entry = disk[dir_block].dir_block.dir[entry_index];
    open_files[name] = entry.link;
    current_file_block = entry.link;
    if (mode == 'I' || mode == 'U') {
        current_file_offset = 0;
    } else if (mode == 'O') {
        current_file_offset = entry.size;
    }
}

void close_file() {
    open_files.clear();
    current_file_block = -1;
    current_file_offset = 0;
}

void delete_file(const string& name) {
    int dir_block = 0;
    size_t pos = name.find_last_of('/');
    if (pos != string::npos) {
        string dir_name = name.substr(0, pos);
        dir_block = find_dir_entry(0, dir_name);
        if (dir_block == -1) {
            cout << "Directory not found: " << dir_name << endl;
            return;
        }
    }
    int entry_index = find_dir_entry(dir_block, name);
    if (entry_index == -1) {
        cout << "File not found: " << name << endl;
        return;
    }
    DirectoryEntry& entry = disk[dir_block].dir_block.dir[entry_index];
    int block_num = entry.link;
    while (block_num != 0) {
        int next_block = disk[block_num].data_block.FRWD;
        free_block(block_num);
        block_num = next_block;
    }
    entry.type = 'F';
    entry.link = 0;
    entry.size = 0;
}

void read_file(int n) {
    if (current_file_block == -1) {
        cout << "No file open for reading." << endl;
        return;
    }
    int bytes_read = 0;
    while (bytes_read < n && current_file_block != 0) {
        int bytes_to_read = min(n - bytes_read, BLOCK_SIZE - 8 - current_file_offset);
        cout.write(disk[current_file_block].data_block.user_data + current_file_offset, bytes_to_read);
        bytes_read += bytes_to_read;
        current_file_offset += bytes_to_read;
        if (current_file_offset == BLOCK_SIZE - 8) {
            current_file_block = disk[current_file_block].data_block.FRWD;
            current_file_offset = 0;
        }
    }
    if (bytes_read < n) {
        cout << "\nEnd of file reached." << endl;
    }
}

void write_file(int n, const string& data) {
    if (current_file_block == -1) {
        cout << "No file open for writing." << endl;
        return;
    }
    int bytes_written = 0;
    while (bytes_written < n) {
        int bytes_to_write = min(n - bytes_written, BLOCK_SIZE - 8 - current_file_offset);
        if (current_file_block == 0) {
            current_file_block = allocate_block();
            if (current_file_block == 0) {
                cout << "Disk is full. Could not write all data." << endl;
                return;
            }
            disk[current_file_block].data_block.BACK = 0;
            disk[current_file_block].data_block.FRWD = 0;
        }
        strncpy(disk[current_file_block].data_block.user_data + current_file_offset,
                data.c_str() + bytes_written, bytes_to_write);
        bytes_written += bytes_to_write;
        current_file_offset += bytes_to_write;
        if (current_file_offset == BLOCK_SIZE - 8) {
            int new_block = allocate_block();
            if (new_block == 0) {
                cout << "Disk is full. Could not write all data." << endl;
                return;
            }
            disk[current_file_block].data_block.FRWD = new_block;
            disk[new_block].data_block.BACK = current_file_block;
            disk[new_block].data_block.FRWD = 0;
            current_file_block = new_block;
            current_file_offset = 0;
        }
    }
}

void seek_file(int base, int offset) {
    if (current_file_block == -1) {
        cout << "No file open for seeking." << endl;
        return;
    }
    if (base == -1) {
        current_file_block = open_files.begin()->second;
        current_file_offset = 0;
    } else if (base == 1) {
        while (disk[current_file_block].data_block.FRWD != 0) {
            current_file_block = disk[current_file_block].data_block.FRWD;
        }
        current_file_offset = BLOCK_SIZE - 8;
    }
    int bytes_to_move = offset;
    while (bytes_to_move != 0) {
        if (bytes_to_move > 0) {
            int bytes_to_next_block = BLOCK_SIZE - 8 - current_file_offset;
            if (bytes_to_move <= bytes_to_next_block) {
                current_file_offset += bytes_to_move;
                bytes_to_move = 0;
            } else {
                bytes_to_move -= bytes_to_next_block;
                current_file_block = disk[current_file_block].data_block.FRWD;
                current_file_offset = 0;
            }
        } else {
            int bytes_to_prev_block = current_file_offset;
            if (-bytes_to_move <= bytes_to_prev_block) {
                current_file_offset += bytes_to_move;
                bytes_to_move = 0;
            } else {
                bytes_to_move += bytes_to_prev_block;
                current_file_block = disk[current_file_block].data_block.BACK;
                current_file_offset = BLOCK_SIZE - 8;
            }
        }
    }
}

void display_directory(int dir_block, int level) {
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        DirectoryEntry& entry = disk[dir_block].dir_block.dir[i];
        if (entry.type == 'D') {
            cout << string(level * 2, ' ') << entry.name << '/' << endl;
            display_directory(entry.link, level + 1);
        } else if (entry.type == 'U') {
            cout << string(level * 2, ' ') << entry.name << " (size: " << entry.size << " bytes)" << endl;
        }
    }
    if (disk[dir_block].dir_block.FRWD != 0) {
        display_directory(disk[dir_block].dir_block.FRWD, level);
    }
}

int count_free_blocks() {
    int count = 0;
    int block_num = disk[0].dir_block.FREE;
    while (block_num != 0) {
        count++;
        block_num = disk[block_num].data_block.FRWD;
    }
    return count;
}

