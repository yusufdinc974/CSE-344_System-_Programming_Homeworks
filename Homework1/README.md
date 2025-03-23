# 🔐 CSE344 – Systems Programming  
## 📝 Homework 1: Secure File and Directory Management System  

---

## 📌 Project Overview  
This project is a **Secure File and Directory Management System** written in **C** using **Linux system calls**.  
It provides a **command-line tool** to perform various file and directory operations with integrated **logging**, **process management**, and **file locking** mechanisms.

---

## 🛠️ Features  
✅ Create & delete **files and directories**  
✅ **List directory contents** or files by extension  
✅ **Read & append** to files with file locking  
✅ View a complete **log history** of all operations  
✅ **Fork-based child processes** for certain commands  
✅ Displays a **help menu** when no arguments are provided  

---

## 🧾 Available Commands  
```bash
./fileManager createDir "folderName"
./fileManager createFile "fileName"
./fileManager listDir "folderName"
./fileManager listFilesByExtension "folderName" ".ext"
./fileManager readFile "fileName"
./fileManager appendToFile "fileName" "your content here"
./fileManager deleteFile "fileName"
./fileManager deleteDir "folderName"
./fileManager showLogs
```

---

## 🧪 Example Usage  
```bash
./fileManager createDir testDir
./fileManager createFile testDir/notes.txt
./fileManager appendToFile testDir/notes.txt "New entry added"
./fileManager listFilesByExtension testDir ".txt"
./fileManager readFile testDir/notes.txt
./fileManager deleteFile testDir/notes.txt
./fileManager deleteDir testDir
./fileManager showLogs
```

---

## ⚙️ How to Compile and Run  
Make sure you're on **Debian 11 (64-bit)** or a similar Linux environment. Then run:

```bash
make
./fileManager
```

To clean compiled files:

```bash
make clean
```

---

## 📂 Project Structure  
```
├── fileManager.c        # Main program file  
├── makefile             # Compilation instructions  
├── log.txt              # Operation logs  
├── README.md            # Project documentation  
```

---

## 💡 Notes  
- Works **only with system calls** (no stdio.h I/O functions).  
- Proper error messages are shown for invalid commands or missing files.  
- **Every operation is logged** in `log.txt`.  
- **File locking** is used when writing to prevent race conditions.  
- **fork()** is used for `list` and `delete` operations as required.  

---

## 🎓 Learning Outcomes  
- Mastery of **Linux system calls** (open, read, write, unlink, etc.)  
- Implementation of **file locking** with `flock()`  
- Process handling via **fork()** and `wait()`  
- Secure, low-level manipulation of files and directories  

---

🚀 **Happy Coding!**  
