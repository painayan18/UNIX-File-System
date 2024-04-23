//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//
//#define DISK_SIZE 100
//#define BLOCK_SIZE 512
//
//typedef struct {
//    int back;
//    int frwd;
//    char user_data[504];
//} DataBlock;
//
//typedef struct {
//    int back;
//    int fr_id;
//    int free;
//    char filler[4];
//    struct {
//        char type;
//        char name[9];
//        int link;
//        short size;
//    } dir[31];
//} DirectoryBlock;
//
//char disk[DISK_SIZE][BLOCK_SIZE];
//int current_block = -1;
//char current_file[100];
//
//void DREAD(int, void*);
//void DWRITE(int, void*);
//void create_file(char, char*);
//void open_file(char, char*);
//void close_file();
//void delete_file(char*);
//void read_file(int);
//void write_file(int, char*);
//void seek_file(int, int);
//void display_directory(int, int);
//void display_file_system_info();
//
//
//int main() {
//    // Initialize the disk and file system
//    memset(disk, 0, sizeof(disk));
//    DirectoryBlock block;
//    memset(&block, 0, sizeof(block));
//    block.free = 1;
//    DWRITE(0, &block);
//
//    char command[100];
//    int consecutive_newlines = 0;
//
//    while (fgets(command, sizeof(command), stdin) != NULL) {
//
//        if (strcmp(command, "\n") == 0) {
//            consecutive_newlines++;
//            if (consecutive_newlines == 2) {
//                break;  // Terminate the program when Enter is pressed twice
//            }
//        } else {
//            consecutive_newlines = 0;
//        }
//
//        command[strcspn(command, "\n")] = '\0';  // Remove trailing newline
//
//        if (strncmp(command, "CREATE", 6) == 0) {
//            char type = command[7];
//            char* name = command + 9;
//            create_file(type, name);
//        } else if (strncmp(command, "OPEN", 4) == 0) {
//            char mode = command[5];
//            char* name = command + 7;
//            open_file(mode, name);
//        } else if (strcmp(command, "CLOSE") == 0) {
//            close_file();
//        } else if (strncmp(command, "DELETE", 6) == 0) {
//            char* name = command + 7;
//            delete_file(name);
//        } else if (strncmp(command, "READ", 4) == 0) {
//            int n = atoi(command + 5);
//            read_file(n);
//        } else if (strncmp(command, "WRITE", 5) == 0) {
//            int n = atoi(command + 6);
//            char* data = strchr(command, '\"') + 1;
//            data[strcspn(data, "\"")] = '\0';  // Remove trailing quote
//            write_file(n, data);
//        } else if (strncmp(command, "SEEK", 4) == 0) {
//            int base = atoi(command + 5);
//            char* offset_str = strchr(command, ' ') + 1;
//            int offset = atoi(offset_str);
//            seek_file(base, offset);
//        }
//    }
//
//    display_file_system_info();
//
//    return 0;
//}
//
//void DREAD(int block_num, void* block) {
//    memcpy(block, disk[block_num], BLOCK_SIZE);
//}
//
//void DWRITE(int block_num, void* block) {
//    memcpy(disk[block_num], block, BLOCK_SIZE);
//}
//
//void create_file(char type, char* name) {
//    DirectoryBlock block;
//    int block_num = 0;
//
//    while (block_num != -1) {
//        DREAD(block_num, &block);
//
//        int i;
//        for (i = 0; i < 31; i++) {
//            if (block.dir[i].type == 'F') {
//                block.dir[i].type = type;
//                strncpy(block.dir[i].name, name, 9);
//                block.dir[i].link = block.free;
//                block.dir[i].size = 0;
//                DWRITE(block_num, &block);
//                current_block = block.free;
//                strcpy(current_file, name);
//                return;
//            }
//        }
//
//        if (block.fr_id == 0) {
//            // Allocate a new directory block
//            DirectoryBlock new_block;
//            memset(&new_block, 0, sizeof(new_block));
//            new_block.back = block_num;
//            new_block.free = block.free;
//
//            // Update the free block list
//            DirectoryBlock root_block;
//            DREAD(0, &root_block);
//            block.free = root_block.free;
//            root_block.free = new_block.free;
//            DWRITE(0, &root_block);
//
//            // Link the current block to the new block
//            block.fr_id = new_block.free;
//            DWRITE(block_num, &block);
//
//            // Write the new block to disk
//            DWRITE(new_block.free, &new_block);
//        }
//
//        block_num = block.fr_id;
//    }
//
//    printf("Error: Directory is full.\n");
//}
//
//void open_file(char mode, char* name) {
//    DirectoryBlock block;
//    DREAD(0, &block);
//
//    int i;
//    for (i = 0; i < 31; i++) {
//        if (block.dir[i].type != 'F' && strcmp(block.dir[i].name, name) == 0) {
//            current_block = block.dir[i].link;
//            strcpy(current_file, name);
//            if (mode == 'I' || mode == 'U') {
//                // Position at the beginning of the file
//                DataBlock data_block;
//                DREAD(current_block, &data_block);
//                current_block = data_block.frwd;
//            } else if (mode == 'O') {
//                // Position at the end of the file
//                while (current_block != 0) {
//                    DataBlock data_block;
//                    DREAD(current_block, &data_block);
//                    current_block = data_block.frwd;
//                }
//            }
//            return;
//        }
//    }
//
//    printf("Error: File not found.\n");
//}
//
//void close_file() {
//    current_block = -1;
//    memset(current_file, 0, sizeof(current_file));
//}
//
//void delete_file(char* name) {
//    DirectoryBlock block;
//    int block_num = 0;
//    int prev_block_num = -1;
//
//    while (block_num != -1) {
//        DREAD(block_num, &block);
//
//        int i;
//        for (i = 0; i < 31; i++) {
//            if (block.dir[i].type != 'F' && strcmp(block.dir[i].name, name) == 0) {
//                block.dir[i].type = 'F';
//                block.dir[i].link = block.free;
//                block.free = block.dir[i].link;
//                DWRITE(block_num, &block);
//
//                // Check if the directory block becomes empty
//                int empty = 1;
//                for (int j = 0; j < 31; j++) {
//                    if (block.dir[j].type != 'F') {
//                        empty = 0;
//                        break;
//                    }
//                }
//
//                if (empty && block_num != 0) {
//                    // Remove the empty block from the linked list
//                    DirectoryBlock prev_block;
//                    DREAD(prev_block_num, &prev_block);
//                    prev_block.fr_id = block.fr_id;
//                    DWRITE(prev_block_num, &prev_block);
//
//                    // Add the empty block to the free block list
//                    DirectoryBlock root_block;
//                    DREAD(0, &root_block);
//                    block.free = root_block.free;
//                    root_block.free = block_num;
//                    DWRITE(0, &root_block);
//                }
//
//                return;
//            }
//        }
//
//        prev_block_num = block_num;
//        block_num = block.fr_id;
//    }
//
//    printf("Error: File not found.\n");
//}
//
//
//void read_file(int n) {
//    if (current_block == -1) {
//        printf("Error: No file is currently open.\n");
//        return;
//    }
//
//    DataBlock data_block;
//    int bytes_read = 0;
//    while (bytes_read < n && current_block != 0) {
//        DREAD(current_block, &data_block);
//        int bytes_to_read = (n - bytes_read < 504) ? n - bytes_read : 504;
//        printf("%.*s", bytes_to_read, data_block.user_data);
//        bytes_read += bytes_to_read;
//        current_block = data_block.frwd;
//    }
//
//    if (bytes_read < n) {
//        printf("\nEnd of file reached.\n");
//    }
//}
//
//void write_file(int n, char* data) {
//    if (current_block == -1) {
//        printf("Error: No file is currently open.\n");
//        return;
//    }
//
//    DataBlock data_block;
//    int bytes_written = 0;
//    while (bytes_written < n) {
//        DREAD(current_block, &data_block);
//        int bytes_to_write = (n - bytes_written < 504) ? n - bytes_written : 504;
//        memcpy(data_block.user_data, data + bytes_written, bytes_to_write);
//        DWRITE(current_block, &data_block);
//        bytes_written += bytes_to_write;
//
//        if (data_block.frwd == 0) {
//            DirectoryBlock dir_block;
//            DREAD(0, &dir_block);
//            if (dir_block.free == 0) {
//                printf("Error: Disk is full.\n");
//                return;
//            }
//            data_block.frwd = dir_block.free;
//            dir_block.free = data_block.frwd;
//            DWRITE(current_block, &data_block);
//            DWRITE(0, &dir_block);
//        }
//
//        current_block = data_block.frwd;
//    }
//}
//
//void seek_file(int base, int offset) {
//    if (current_block == -1) {
//        printf("Error: No file is currently open.\n");
//        return;
//    }
//
//    if (base == -1) {
//        // Seek from the beginning of the file
//        DirectoryBlock block;
//        DREAD(0, &block);
//
//        int i;
//        for (i = 0; i < 31; i++) {
//            if (block.dir[i].type != 'F' && strcmp(block.dir[i].name, current_file) == 0) {
//                current_block = block.dir[i].link;
//                break;
//            }
//        }
//    } else if (base == 1) {
//        // Seek from the end of the file
//        while (current_block != 0) {
//            DataBlock data_block;
//            DREAD(current_block, &data_block);
//            current_block = data_block.frwd;
//        }
//    }
//
//    // Adjust the current block based on the offset
//    while (offset > 0 && current_block != 0) {
//        DataBlock data_block;
//        DREAD(current_block, &data_block);
//        current_block = data_block.frwd;
//        offset--;
//    }
//
//    while (offset < 0) {
//        DataBlock data_block;
//        DREAD(current_block, &data_block);
//        current_block = data_block.back;
//        offset++;
//    }
//}
//
//void display_directory(int block_num, int level) {
//    DirectoryBlock block;
//    DREAD(block_num, &block);
//
//    int i;
//    for (i = 0; i < 31; i++) {
//        if (block.dir[i].type != 'F') {
//            printf("%*s%s\n", level * 2, "", block.dir[i].name);
//            if (block.dir[i].type == 'D') {
//                display_directory(block.dir[i].link, level + 1);
//            }
//        }
//    }
//
//    if (block.fr_id != 0) {
//        display_directory(block.fr_id, level);
//    }
//}
//
//void display_file_system_info() {
//    printf("Directory:\n");
//    display_directory(0, 0);
//
//    printf("\nFile Lengths:\n");
//    DirectoryBlock block;
//    DREAD(0, &block);
//
//    int i;
//    for (i = 0; i < 31; i++) {
//        if (block.dir[i].type == 'U') {
//            int file_length = 0;
//            int current_block = block.dir[i].link;
//            while (current_block != 0) {
//                DataBlock data_block;
//                DREAD(current_block, &data_block);
//                file_length += 504;
//                current_block = data_block.frwd;
//            }
//            file_length -= 504 - block.dir[i].size;
//            printf("%s: %d bytes\n", block.dir[i].name, file_length);
//        }
//    }
//
//    printf("\nFree Blocks:\n");
//    int free_blocks = 0;
//    int current_block = block.free;
//    while (current_block != 0) {
//        free_blocks++;
//        DirectoryBlock dir_block;
//        DREAD(current_block, &dir_block);
//        current_block = dir_block.free;
//    }
//    printf("Directory Blocks: %d\n", 100 - free_blocks);
//    printf("User Data Blocks: %d\n", free_blocks);
//}