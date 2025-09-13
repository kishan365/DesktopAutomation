#define _CRT_SECURE_NO_WARNINGS  
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>   // optional, useful for modern socket API

#pragma comment(lib, "Ws2_32.lib") // link winsock lib
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <stdlib.h>
#include <shellapi.h>   // for ShellExecute
#include "main.h"
static int HotKey_ID;

#define MAX_HISTORY 10     // Maximum number of clipboard entries to store
#define BUFFER_SIZE 200    // Maximum size of each clipboard entry
#define MAX_TASK_NUMBER 20 // Maximum number of tasks allowed

typedef void (*FuncPtr)(const char*);

typedef struct {
    int id;
    char *filePath;
    FuncPtr func;
}Task;

static Task *task = NULL;
static int taskCount;
bool textOverflow = false;
DWORD mainThreadID;
DWORD pipeThreadID;
HANDLE hPipe;
char directory[100] = "C:\\Users\\kishan sah\\source\\repos\\Desktop Automation\\Tasks Files";

enum Task_Action {
    Task_Action_Invalid,
    Task_Action_Add,
    Task_Action_Edit,
    Task_Action_Delete,
};

enum HotKey_Action {
    HotKey_Action_Invalid,
    HotKey_Action_Add,
    HotKey_Action_Edit,
    HotKey_Action_Delete,
};

enum TaskName {
    Task_Undefined,
    Task_Clipboard,
    Task_OpenHelloFolder,
    Task_OpenLinFolder,
    Task_KairanMail,
};




void OpenProgram(const char *path) {
    char command[512];
    snprintf(command, sizeof(command), "\"%s\"", path);  // wrap path in quotes
    system(command);
}

void OpenGrep() {
    char path[200];
    char fileName[] = "追加ラベル検索.bat";
    char new_path[300];
    char command[512];

    // Step 1: Ask user for destination folder path
    printf("Enter the destination folder path (e.g., C:\\Backup):\n");

    // Use scanf_s to safely read the path
    scanf_s("%s", path, (unsigned)_countof(path));

    // Step 2: Open the original .bat file in Notepad for editing
    system("notepad \"C:\\Users\\71163572\\Desktop\\personal\\TEJYUN\\GrepKensa\\追加ラベル検索.bat\"");

    // Step 3: Create new full destination path
    sprintf_s(new_path, sizeof(new_path), "%s\\%s", path, fileName);

    // Step 4: Copy the file to the new path
    sprintf_s(command, sizeof(command),
        "copy \"C:\\Users\\71163572\\Desktop\\personal\\TEJYUN\\GrepKensa\\追加ラベル検索.bat\" \"%s\"", new_path);
    system(command);

    // Step 5: Execute the copied .bat file
    sprintf_s(command, sizeof(command), "\"%s\"", new_path);
    // system(command); // Uncomment if you want it to run
}

void GetClipboardText(char *buffer, size_t bufferSize) {
    if (!OpenClipboard(NULL)) {
        strcpy_s(buffer, bufferSize, "");
        return;
    }
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == NULL) {
        CloseClipboard();
        strcpy_s(buffer, bufferSize, "");
        return;
    }
    char *pszText = (char *)GlobalLock(hData);
    if (pszText == NULL) {
        CloseClipboard();
        strcpy_s(buffer, bufferSize, "");
        return;
    }
    strncpy_s(buffer, bufferSize, pszText, bufferSize - 1);
    GlobalUnlock(hData);
    CloseClipboard();
}
int strtoInt(char *str) {
    int num = 0;
    for (int iter = 0; str[iter] != 0; iter++) {
        num = num * iter * 10;
        num = num + str[iter] - '0';
    }
    return num;
}
char* toUpperString(char *string) {
    char *upper = (char *)malloc(strlen(string)+1);
    if (!upper) { return 0; }
    memset(upper, 0, strlen(string)+1);
    for (int iter = 0; string[iter] != 0; iter++) {
        if (string[iter] < 'A' || string[iter]>'z') {
            printf("Invalid character present in the string\n");
            upper[iter] = string[iter];
            continue;
        }
        upper[iter] = string[iter] - 'a' >= 0 ? string[iter] - 32 : string[iter];
    }
    return upper;
    
}
char toUpperChar(char c) {
    if (c < 'A' || c>'z') {
        return c;
    }
    return c - 'a' >= 0 ? c - 32 : c;
}

Delimiter getDelimitedWord(char *hotkeys, char delimiter,bool fulltext ) {
    d = { 0 };
    d.fullString = (char *)malloc(strlen(hotkeys)+1);
    if (d.fullString == NULL) {
        return d;
    }
    memset(d.fullString, 0, strlen(hotkeys)+1);
    int len = strlen(hotkeys);
    char *start = hotkeys;
    int iter = 0;
    int i = 0;
    while (iter < len) {
        if (hotkeys[iter] == delimiter) {
            start = &hotkeys[iter+1];
            start[0] = toUpperChar(start[0]);
            d.singleChar = start[0];
            iter = 0;
            break;
        }
        iter++;
    }
    if (iter) {
        printf("No %c was found\n", delimiter);
        strncpy(d.fullString, hotkeys, strlen(hotkeys));
        d.singleChar = 0;
        return d;
    }
    while (iter < len) {
        if (hotkeys[iter] != delimiter) {
            iter++;
            continue;
        }
        while (iter < len) {
            d.fullString[i] = toUpperChar(hotkeys[iter + 1]);
            iter++;
            i++;
        }
        if (iter >= len) {
            break;
        }
    }
    if (!fulltext) {
        memset(d.fullString, 0, strlen(d.fullString));
    }
    return d;
}
/*============PATTERN====================*/
/*
Add; add=ALT+A/2
Edit: edit=ALT+A/2(old),ALT+B/2(new)== Here the hotkeys only should be edited not the task assigned.
Delete: delete=ALT+A/2


*/

/*=======================================*/
void parseEdit(char *hotkeys) {
    int iter = 0;
    char *buf_hotkeys=(char*)malloc(strlen(hotkeys)+1);
    if (!buf_hotkeys) { return; }
    memset(buf_hotkeys, 0, strlen(hotkeys)+1);
    buf_hotkeys = toUpperString(hotkeys);
    char taskId[2];
    taskId[0] = getDelimitedWord(hotkeys, '/', false).singleChar;
    taskId[1] = 0;
    int Id = strtoInt(taskId);
    UnregisterHotKey(NULL, Id);
    char *new_hotkey = getDelimitedWord(hotkeys, ',', true).fullString;
    RegisterHotKey_user(new_hotkey);
}
void ParseDelete(char *hotkeys) {
    char taskId[2];
    taskId[0] = getDelimitedWord(hotkeys, '/', false).singleChar;
    taskId[1] = 0;
    int Id = strtoInt(taskId);
    UnregisterHotKey(NULL, Id);
}

int RegisterHotKey_user(char *hotkeys) {
    int winKey;
    char *actionID = (char *)malloc(2);
    if (!actionID) { return 0; }
    actionID[0] = getDelimitedWord(hotkeys, '/',false).singleChar;
    actionID[1] = 0;
    int ID = strtoInt((char *)actionID);
    if (ID == 0) {
        printf("ID=0 is not allowed\n");
        return 0;
    }
    hotkeys = getDelimitedWord(hotkeys, '=', true).fullString;
    
    if (!strncmp(hotkeys, "ALT", 3)) {
        winKey = MOD_ALT;
    }
    else  if (!strncmp(hotkeys, "CTRL", 4)) {
        winKey = MOD_CONTROL;
    }
    else if (!strncmp(hotkeys, "SHIFT", 5)) {
        winKey = MOD_SHIFT;
    }
    else {
        printf("HotKeys not matched\n");
        return 0;
    }
   
    char hotkey = getDelimitedWord(hotkeys, '+',false).singleChar;
    if (!hotkey) {
        return 0;
    }
    if (!RegisterHotKey(NULL, ID, winKey, (unsigned char)hotkey)) {
        printf("Failed to register %s hotkey\n",hotkeys); 
        return 0; 
    }
    // if (!RegisterHotKey(NULL, ID, winKey, 'B')) {

   //if (!RegisterHotKey(NULL, 1, MOD_ALT, 'A')) { printf("Failed to register Alt+A hotkey\n"); return 1; }
    free(hotkeys);
    return 1;
}

DWORD WINAPI CreateNamedPipe(LPVOID lpParameter) {
    pipeThreadID = GetCurrentThreadId();
    hPipe = CreateNamedPipe(
        TEXT("\\\\.\\pipe\\MyPipe"),
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1,
        128,
        128,
        0,
        NULL
	);
    if(hPipe == INVALID_HANDLE_VALUE) {
        printf("Error creating named pipe: %lu\n", GetLastError());
        return 0;
	}
    printf("Waiting for client....\n");
    BOOL connected = ConnectNamedPipe(hPipe, NULL);
    if (!connected) {
        printf("[pipe thread] ConnectNamedPipe failed: %lu\n", GetLastError());
        CloseHandle(hPipe);
        Sleep(500);
        return 0;
    }
    printf("Client Connected....\n");
    char buf[256];
    DWORD bytesRead;
    while(1){

        BOOL ok = ReadFile(hPipe, buf, (DWORD)sizeof(buf) - 1, &bytesRead, NULL);
        if (!ok) {
            DWORD e = GetLastError();
            if (e == ERROR_BROKEN_PIPE || e == ERROR_NO_DATA) {
                printf("[pipe thread] Client disconnected (err %lu).\n", e);
            }
            else {
                printf("[pipe thread] ReadFile failed: %lu\n", e);
            }
            break;
        }
        if (bytesRead > 0) {
            buf[bytesRead] = '\0';
            // duplicate buffer to heap and post pointer to main thread
            char *msgcopy = _strdup(buf);
            if (msgcopy == NULL) {
                printf("[pipe thread] strdup failed\n");
            }
            else {
                if (!PostThreadMessage(mainThreadID, WM_APP+100, (WPARAM)msgcopy, 0)) {
                    printf("[pipe thread] PostThreadMessage failed: %lu\n", GetLastError());
                    free(msgcopy);
                }
            }
        }
	}
}


HotKey_Action parsePipeHotkey(char *message) {
    if (!strncmp(message, "add", 3))
        return HotKey_Action_Add;
    if (!strncmp(message, "edit", 4))
        return HotKey_Action_Edit;
    if (!strncmp(message, "delete", 6))
        return HotKey_Action_Delete;
    return HotKey_Action_Invalid;
}

Task_Action parsePipeTask(char *message) {
    if (!strncmp(message, "AddTask", 7))
        return Task_Action_Add;
    if (!strncmp(message, "EditTask", 8))
        return Task_Action_Edit;
    if (!strncmp(message, "DeleteTask", 10))
        return Task_Action_Delete;
    return Task_Action_Invalid;
}

TaskName generateIdFromTask(char *task) {
    TaskName t;
    if (!strncmp(task, "ClipBoard", 9)) {
        t = Task_Clipboard;
    }
    else if (!strncmp(task, "OpenHello", 9)) {
        t = Task_OpenHelloFolder;
    }
    else if (strncmp(task, "OpenLin", 7)) {
        t = Task_OpenLinFolder;
    }
    else if (strncmp(task, "Kairan", 6)) {
        t = Task_KairanMail;
    }
    else {
        t = Task_Undefined;
    }
    return t;
}
static char *getJsonValue(const char *key, char *jsonMsg) {
    char* value = strstr(jsonMsg, key);
    char *value2 = (char*)malloc(30);
    if (!value2) {
        return 0;
    }
    memset(value2, 0, 30);
    int iter = 0;
    while (value[iter]!= '\"') {
        iter++;
    }
    value = &value[iter+3];
    iter = 0;
    int iter2 = 0;
    while (value[iter] != '\"') {
        if (value[iter] == ' ') { //Removing the spaces if present
            iter++;
            continue;
        }
        value2[iter2] = value[iter];
        iter++;
        iter2++;
    }
    return value2;
}
void parseTaskJson(char *jsonMessage) {
    char *hotkey = getJsonValue("hotkey", jsonMessage);
    char *Id = getJsonValue("id", jsonMessage);
    const char *fileName = getJsonValue("file", jsonMessage);
    char *directoryBuff = (char *)malloc(150);
    if (directoryBuff == NULL) { return; }
    memset(directoryBuff, 0, 150);
    strncpy(directoryBuff, directory, strlen(directory));
    directoryBuff[strlen(directory)] = 0;
    strcat(directoryBuff, "\\");
    const char *filePath = strcat(directoryBuff, fileName);
    char *hotkeyInfo = (char *)malloc(strlen(hotkey) + strlen(Id) + 3);
    if (hotkeyInfo == NULL) { return; }
    memset(hotkeyInfo, 0, strlen(hotkey) + strlen(Id) + 3);
    strncpy(hotkeyInfo, hotkey, strlen(hotkey));
    hotkeyInfo[strlen(hotkey)] = '/';
    strcat(hotkeyInfo, Id);

    
    task[taskCount].id = strtoInt(Id);
    task[taskCount].func = OpenProgram;
    task[taskCount].filePath = (char *)malloc(sizeof(char) * 150);
    if (task[taskCount].filePath == NULL) {
        return;
    }
    memset(task[taskCount].filePath, 0, 150);
    strcpy(task[taskCount].filePath, filePath);
    RegisterHotKey_user(hotkeyInfo);
    

}

int main(void) {
    mainThreadID = GetCurrentThreadId();
    CreateThread(NULL, 0, CreateNamedPipe, NULL, 0, NULL);
    task = (Task *)malloc(sizeof(Task) * MAX_TASK_NUMBER);
    char buffer[128];
    DWORD dwRead = 0;
    // Prepare variables for loop
    char clipboardHistory[MAX_HISTORY][BUFFER_SIZE];
    int historyCount = 0;
    char previousClipboard[BUFFER_SIZE];
    GetClipboardText(previousClipboard, BUFFER_SIZE);

    FILE *file = NULL;
    errno_t errfile;

    printf("Monitoring clipboard... Press Ctrl+Q to EXIT.\n");

    time_t startTime = 0;
    int iter = 0;
    bool fsave = FALSE;
    bool running = FALSE;

    // We'll reuse one OVERLAPPED for reads; recreate/reset per attempt.
    while (1){
        MSG msg;
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            printf("Got message: %u\n", msg.message);
            if (msg.message == WM_HOTKEY) {
                printf("Got windows Hotkey Message..\n");
                for (int iter = 0; iter < MAX_TASK_NUMBER; iter++) {
                    if (msg.wParam == task[iter].id) {
                        task[iter].func(task[iter].filePath);
                    }
                }
                
                if (msg.wParam == 1) { 
                    printf("Alt+A pressed.\n"); 
                    fsave = TRUE; 
                }
                if (msg.wParam == 2) { 
                    printf("Alt+U pressed.\n"); 
                    system("start \"\" \"C:\\Users\\kishan sah\\source\\repos\\Helloworld\""); 
                }
                if (msg.wParam == 3) { 
                    printf("Alt+L pressed.\n"); 
                    system(" start C:\\Users\\71163572\\Desktop\\temp\\A_LIN対応"); 
                }
            }
            if (msg.message == WM_APP + 100) {
                printf("Got NamedPipe Message...\n");
                char *s = (char *)msg.wParam;
                if (s) {
                    printf("[main] Received from pipe: %s\n", s);
                    // Call the RegisterHotKey_user on main thread — REQUIRED
                    Task_Action Task_Action_type = parsePipeTask(s);
                    if (Task_Action_type == Task_Action_Invalid) {
                        HotKey_Action Action_type = parsePipeHotkey(s);
                        switch (Action_type) {
                        case HotKey_Action_Add:
                            RegisterHotKey_user(s);
                            break;
                        case HotKey_Action_Edit:
                            parseEdit(s);
                            break;
                        case HotKey_Action_Delete:
                            ParseDelete(s);
                            break;
                        default:
                            printf("Todo\n");
                        }
                        free(s); // free heap copy
                    }
                    else {
                        //Handle the hotkey tasks...
                        
                        switch (Task_Action_type) {
                        case Task_Action_Add:
                            parseTaskJson(s);
                            taskCount++;
                            break;
                        case Task_Action_Edit:
                            printf("Edit Task Not supported\n");
                            break;
                        case Task_Action_Delete:
                            printf("Delete Task Not supported\n");
                            break;
                        default:
                            printf("Todo\n");
                        }
                        free(s); // free heap copy
                    }
                    
                }
                // do not pass to TranslateMessage/DispatchMessage
            }
        }

        char currentClipboard[BUFFER_SIZE];
        GetClipboardText(currentClipboard, BUFFER_SIZE);

        if (historyCount >= (MAX_HISTORY - 2)) {
            printf("Please save the file and start new session\n");
            Sleep(1000);
        }

        if (strlen(currentClipboard) > 0 &&
            strcmp(currentClipboard, previousClipboard) != 0 &&
            fsave == TRUE)
        {
            strcpy_s(previousClipboard, BUFFER_SIZE, currentClipboard);
            strcpy_s(clipboardHistory[historyCount % MAX_HISTORY], BUFFER_SIZE, currentClipboard);
            historyCount++;
            printf("Copied to history: %s\n", currentClipboard);
            startTime = time(NULL);
            fsave = FALSE;
        }

        if (_kbhit()) {
            char ch = _getch();
            if (ch == 1) { fsave = TRUE; }  // Ctrl+A
            if (ch == 81) { printf("Shift+Q pressed.\n"); system("\"C:\\Program Files (x86)\\sakura\\sakura.exe\" C:\\Users\\71163572\\Desktop\\personal\\TEJYUN\\QAC_readMe.txt"); }
            if (ch == 83) { printf("Shift+S pressed.\n"); OpenProgram("C:\\Users\\71163572\\Desktop\\personal\\personal\\StampCheck.py"); }
            if (ch == 19 || ch == 17) {
                printf("\nSaving...\n");
                errfile = fopen_s(&file, "C:\\Users\\71163572\\Desktop\\personal\\personal\\content_new.txt", "a");
                if (file != NULL && running == FALSE) { fprintf(file, "\n\n\n"); running = TRUE; }
                if (file != NULL) {
                    for (int i = 0; i < historyCount; i++) { fprintf(file, "%s\n", clipboardHistory[i]); }
                    fclose(file);
                    printf("File write complete\n");
                }
                else {
                    printf("Error: Unable to open file.\n");
                }
                fsave = FALSE;
            }
            if (ch == 17) { // Ctrl+Q
                printf("\nYou pressed Ctrl + Q. Exiting...\n");
                break;
            }
        }

        iter++;
        if (iter == 3000) {
            time_t currentTime = time(NULL);
            printf("\t%d seconds running\n", (int)(currentTime - startTime));
            printf("Press Ctrl+Q to exit the program\n");
            iter = 0;
        }

        Sleep(500); // Reduce CPU usage
    } // end while

    // cleanup
    CloseHandle(hPipe);
    return 0;
}


/*
  

*/