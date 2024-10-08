
# UNIX File System

### Introduction

This project involves the design and detailed implementation of a file system inspired by the UNIX file system. The system supports a hierarchical file directory structure and uses pointers to manage individual blocks of data.

The file system simulates a disk with 100 blocks (sectors), each containing 512 bytes of data. The disk access is abstracted through two procedures, `DREAD` and `DWRITE`, which you'll implement. These procedures will handle all disk interactions. Note that you do not need to perform actual disk reads and writes; the disk can be simulated using a 50KB block of memory.

The file system will be tested using a front-end program that processes commands provided as input. These commands will test the create, open, close, delete, read, write, and seek functions of the file system. After processing the input, the following information will be displayed:
- The directory structure (showing its hierarchical organization)
- The length of each file (in bytes)
- The number of free directory and user data blocks

### Directory Structure

The file directory in this system is hierarchically ordered. The root directory always begins in block 0, which has the following structure (in a pseudo-PL/I syntax):

```pl1
DECLARE 1 BLOCK0, 
  2 BACK FIXED BIN(31),     /* ALWAYS ZERO IN THIS BLOCK */
  2 FRWD FIXED BIN(31),     /* BLOCK NUMBER OF SECOND DIRECTORY BLOCK OR '2' */
  2 FREE FIXED BIN(31),     /* BLOCK NUMBER OF FIRST UNUSED BLOCK */
  2 FILLER CHARACTER(4),    /* UNUSED */
  2 DIR (31),               /* DIRECTORY ENTRIES */
    3 TYPE CHARACTER(1),    /* 'F' = FREE, 'D' = DIRECTORY, 'U' = USER DATA */
    3 NAME CHARACTER (9),   /* FILE NAME, LEFT-JUSTIFIED, BLANK-FILLED */
    3 LINK FIXED BIN(31),   /* BLOCK NUMBER OF FIRST BLOCK OF FILE */
    3 SIZE FIXED BIN(15);   /* NUMBER OF BYTES USED IN THE LAST BLOCK OF THE FILE */
```

Subsequent blocks in the root directory, if any, will be linked to block 0 using the `FRWD` entry. These blocks, along with blocks in subordinate directories, will have the same structure as block 0, except the `FREE` entry will be unused. The `FREE` entry in block 0 points to the first unused block on the disk. Unused blocks will form a linked list, which must be initialized when the file system starts.

The `TYPE` entry of each directory entry determines the type of file it references:
- `F`: Indicates that the entry is unused.
- `D`: Points to another (subordinate) directory.
- `U`: Points to a user data file.

File names are strings of up to 9 alphanumeric characters separated by slashes (`/`). Examples of valid file names include:

- `SAMPLE`
- `SUB/SAMPLE`
- `SUB1/SUB2/SAMPLE`

The first file name would be located in the root directory as a user data file. The second name would appear in the root directory as a subordinate directory `SUB`, with `SAMPLE` in that directory as a user data file. The third name involves two subordinate directories, with `SAMPLE` located in the `SUB2` directory as a user data file.

Each block in a data file, except the last, is considered full. The `SIZE` field in the directory entry indicates the number of data bytes in the last block. Directory blocks always have exactly 31 entries, with unused entries marked by a `TYPE` field of `F`.

### Data File Format

Data files are also structured as linked lists. The structure of a data file block is as follows:

```pl1
DECLARE 1 DATA_BLOCK,
  2 BACK FIXED BIN(31),   /* BLOCK NUMBER OF PREVIOUS BLOCK */
  2 FRWD FIXED BIN(31),   /* BLOCK NUMBER OF SUCCESSOR BLOCK */
  2 USER_DATA CHAR(504);  /* USER DATA BYTES */
```

The first and last data blocks of a file will have the `BACK` and `FRWD` fields, respectively, set to zero, indicating the beginning and end of the linked data block list. The maximum number of bytes in a data block is 504.

### Commands

Each command processed by the file system corresponds to an input command line. The syntax and processing for each command are as follows:

- **CREATE type name**:  
  - `type`: Either `U` (User data file) or `D` (Directory file).
  - `name`: A full file name in the form described above.
  - **Action**: Creates a new directory entry for the specified file type. If the file already exists, it is deleted and recreated. The file is left in the same state as an `OPEN` command in output mode.

- **OPEN mode name**:  
  - `mode`: Either `I` (Input), `O` (Output), or `U` (Update).
  - `name`: The file to be opened.
  - **Action**: Opens the file in the specified mode. Input mode allows `READ` and `SEEK` commands, output mode allows `WRITE` commands, and update mode allows `READ`, `WRITE`, and `SEEK` commands. The file pointer is set accordingly.

- **CLOSE**:  
  - **Action**: Closes the last opened or created file.

- **DELETE name**:  
  - `name`: The file to be deleted.
  - **Action**: Deletes the specified file.

- **READ n**:  
  - `n`: The number of bytes to read.
  - **Action**: Reads and displays `n` bytes of data from the file. If fewer than `n` bytes remain, the remaining bytes are displayed with an end-of-file message.

- **WRITE n “data”**:  
  - `n`: The number of bytes to write.
  - `data`: The data to write, enclosed in single quotes.
  - **Action**: Writes the first `n` bytes of `data` to the file. If fewer than `n` bytes are provided, blanks are appended to make up `n` bytes. If the disk is full, the maximum possible number of bytes is written, and a message is displayed.

- **SEEK base offset**:  
  - `base`: Either `-1` (beginning of the file), `0` (current position), or `+1` (end of file).
  - `offset`: The number of bytes to move the file pointer from the `base`.
  - **Action**: Moves the file pointer as specified.

The input data is guaranteed to have a legal format, and the following syntax graph describes all possible sequences of input data:

