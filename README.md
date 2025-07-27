# 🗃️ File System Simulator (Operating Systems Final Project)

A simulation of a basic file system implemented in C++.  
This project demonstrates key file system concepts including inode structure, file descriptors, and block allocation.

---

## 👩‍💻 Author

**Raghad Alyan**  
Azrieli College of Engineering

---

## 📄 Project Structure

- `main.cpp`: Entry point and simulation interface

---

## 📦 Classes Overview

### 🔹 `fsInode`

Manages file metadata and block pointers.

- Tracks file size
- Uses direct, single-indirect, and double-indirect block addressing
- Helps allocate and manage blocks

---

### 🔹 `FileDescriptor`

Handles:

- File name
- Pointer to `fsInode`
- Usage status (open/closed)

---

### 🔹 `fsDisk`

Core file system class responsible for:

- Formatting the virtual disk
- Creating, opening, writing, reading, copying, renaming, and deleting files
- Managing file descriptors and the main directory

---

## 🧠 Key Functions in `fsDisk`

- `fsFormat()`: Format disk with given block size
- `CreateFile()`, `OpenFile()`, `CloseFile()`
- `WriteToFile()`, `ReadFromFile()`
- `DelFile()`, `CopyFile()`, `RenameFile()`

### 🔧 Internal Helpers

- `seekInFile()`: Move file pointer
- `findAnEmptyBlock()`: Search available blocks
- `freeBlocks()`: Release used blocks
- `write()`, `read()`, `writeForBlocks()`: Handle low-level disk I/O

---

## 🚀 How to Run

### 🛠️ Compile

```bash
g++ main.cpp -o FileSystemSimulator

