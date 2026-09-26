# Advanced Operating System Project

本專案使用 **C++、POSIX Socket 與 pthread** 實作 Multi-client File Access System。

透過 Client-Server Architecture 模擬 Operating System 中的 Multi-user File Access、File Permission、Concurrent Access 與 Synchronization。

---

## 系統架構

```text
Client 1 ──┐
           │
Client 2 ──┼──► TCP Server ──► File Repository
           │         │
Client 3 ──┘         │
                     ▼
               pthread / mutex
```

Server 可同時接受多個 Client Connection，並利用 Thread 處理不同 Client Request。

---

## 使用技術

- C++
- POSIX Socket
- TCP
- POSIX pthread
- Mutex
- Multithreading
- File Permission
- User / Group
- Concurrent Programming

---

## Network Architecture

系統使用 TCP Socket：

```text
IP   : 127.0.0.1
Port : 7000
Type : TCP
```

### Server

```text
socket()
   │
   ▼
bind()
   │
   ▼
listen()
   │
   ▼
accept()
   │
   ▼
pthread_create()
```

### Client

```text
socket()
   │
   ▼
connect()
   │
   ▼
send()
   │
   ▼
recv()
```

---

## Multi-client Handling

當 Client 建立 Connection 後，Server 建立獨立 Thread 處理該 Client。

```text
          Server
            │
     ┌──────┼──────┐
     ▼      ▼      ▼
 Thread 1 Thread 2 Thread 3
     │      │      │
     └──────┼──────┘
            ▼
     Shared Resource
```

藉此模擬多個 User 同時操作共享資源的情境。

---

## User / Group

系統將 Client 分配至不同 User 與 Group，例如：

```text
AOS-0
CSE-0
AOS-2
CSE-2
```

Group Information 會被用於 File Permission Check。

---

## File Permission

File Repository 中記錄：

- Filename
- Owner
- Group
- Content
- Permission
- Read / Write State

Permission 分成：

```text
Owner
Group
Others
```

並分別管理 Read / Write 權限。

概念類似 Unix File Permission：

```text
        File
         │
    ┌────┼────┐
    ▼    ▼    ▼
  Owner Group Others
    │    │     │
   R/W  R/W   R/W
```

---

## Supported Commands

### Create

```text
create <filename> <permissions>
```

建立 File，並記錄：

- Owner
- Group
- Permission

---

### Read

```text
read <filename>
```

Server 根據 Client 身分判斷：

```text
Owner ?
  │
  ├── Yes → Owner Permission
  │
  └── No
       │
       ▼
    Same Group ?
       │
       ├── Yes → Group Permission
       │
       └── No  → Others Permission
```

---

### Write

```text
write <filename> <mode>
```

支援：

```text
o → overwrite
a → append
```

Write 時會檢查 Permission 與目前 File Access State。

---

### Change Mode

```text
changemode <filename> <permissions>
```

修改 File Permission，模擬 Operating System 的 Access Control。

---

## Synchronization

多個 Client Thread 可能同時操作 Shared File Repository，因此使用 Mutex 保護 Critical Section。

```cpp
pthread_mutex_lock();

/* Critical Section */

pthread_mutex_unlock();
```

概念：

```text
Thread A ──┐
           │
Thread B ──┼──► Mutex ──► Shared Resource
           │
Thread C ──┘
```

用於避免：

- Race Condition
- Concurrent Modification
- Inconsistent Shared State

---

## Repository Structure

```text
Advanced-OS-Project/
│
├── client.cpp
├── server.cpp
└── Makefile
```

### server.cpp

主要負責：

- Socket Initialization
- Client Connection
- Thread Creation
- User / Group Management
- File Repository
- Permission Check
- Synchronization

### client.cpp

主要負責：

- Connect Server
- User Command Input
- Send Request
- Receive Server Response

---

## Build & Run

可透過 Makefile 建置，或手動編譯：

```bash
g++ server.cpp -o server -pthread
g++ client.cpp -o client
```

啟動 Server：

```bash
./server
```

開啟另一個 Terminal 執行 Client：

```bash
./client
```

也可以同時啟動多個 Client，測試 Concurrent Access。

---

## 實作重點

- TCP Socket Programming
- Client-Server Architecture
- POSIX pthread
- Multithreading
- Mutex
- Critical Section
- Race Condition
- Shared Resource
- File Permission
- User / Group Access Control
- Concurrent Programming
