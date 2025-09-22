#define _CRT_SECURE_NO_WARNINGS  

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <stdlib.h>
#include "main.h"

/*================DEFINE=====================================================*/
#define MAX_TASK_NUMBER 20 // Maximum number of tasks allowed

typedef void (*FuncPtr)(const char*);
/*===============================STRUCTURE===================================*/
typedef struct {
    int id;
    char *filePath;
    FuncPtr func;
    char *hotkey;
}Task;
/*===============================GLOBAL VARIABLES=============================*/
static Task *task = NULL;
static int taskCount;
DWORD mainThreadID;
DWORD pipeThreadID;
HANDLE hPipe;
char directory[100] = "C:\\Users\\kishan sah\\source\\repos\\Desktop Automation\\Tasks Files";
const char *file = "text.txt";
/*=================================ENUMs=======================================*/
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
/*=======================================TASK REGISTER FUNCTION=========================================*/
void OpenProgram(const char *path) {
    char command[512];
    snprintf(command, sizeof(command), "\"%s\"", path);  // wrap path in quotes
    system(command);
}
/*========================================HELPER FUNCTIONS========================================*/
char *allocateMemory(int size) {
    char *var = (char *)malloc(size + 1);
    if (!var) {
        return 0;
    }
    memset(var, 0, size + 1);
    return var;
}
char *removeWhitespace(char *text) {
    char *newString = (char *)malloc(strlen(text) + 1);
    if (newString == NULL) {
        return 0;
    }
    memset(newString, 0, strlen(text) + 1);
    for (int iter = 0, i = 0; iter < strlen(text); iter++) {
        if (text[iter] == ' ') {
            continue;
        }
        newString[i] = text[iter];
        i++;
    }
    return newString;
}
int strtoInt(char *str) {
    int num = 0;
    if (strlen(str) <= 0) {
        return 0;
    }
    for (int iter = 0; iter < strlen(str); iter++) {
        if (str[iter] < '0' or str[iter]>'9') {
            printf("Non-Numerical value is present in the string\n");
            return 0;
        }
    }
    for (int iter = 0; str[iter] != 0; iter++) {
        num = num * iter * 10;
        num = num + str[iter] - '0';
    }
    return num;
}
char* toUpperString(char *string) {
    char *upper = allocateMemory(strlen(string));
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
int generateUniqueID() {
    int id = 1;
    bool found = false;
    for (int iter = 0; iter < MAX_TASK_NUMBER; iter++) {
        if (task[iter].id == id) {
            found = true;
        }
        if (found == true) {
            id++;
            found = false;
            iter = -1;
        }
    }
    return id;
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
/*============================DIFFERENT THREAD FUNCTION======================================*/
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
    if (hPipe == INVALID_HANDLE_VALUE) {
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
    while (1) {
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
            char *msgcopy = allocateMemory(strlen(buf));
            strcpy(msgcopy, buf);
            if (msgcopy == NULL) {
                printf("[pipe thread] failed\n");
            }
            else {
                if (!PostThreadMessage(mainThreadID, WM_APP + 100, (WPARAM)msgcopy, 0)) {
                    printf("[pipe thread] PostThreadMessage failed: %lu\n", GetLastError());
                    free(msgcopy);
                }
            }
        }
    }
}
/*========================================HOTKEY CRUD FUNCTIONS========================================*/
int ParseHotkeyAdd(char *input_hotkeys) {
    int winKey;
    char *hotkeys = removeWhitespace(input_hotkeys);
    char *actionID = (char *)malloc(2);
    if (!actionID) { return 0; }
    actionID[0] = getDelimitedWord(hotkeys, '/', false).singleChar;
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
    char hotkey = getDelimitedWord(hotkeys, '+', false).singleChar;
    if (!hotkey) {
        return 0;
    }
    if (!RegisterHotKey(NULL, ID, winKey, (unsigned char)hotkey)) {
        printf("Failed to register %s hotkey\n", hotkeys);
        return 0;
    }
    free(hotkeys);
    return 1;
}
void ParseHotkeyEdit(char *hotkeys) {
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
    ParseHotkeyAdd(new_hotkey);
}
void ParseHotkeyDelete(char *hotkeys) {
    char taskId[2];
    taskId[0] = getDelimitedWord(hotkeys, '/', false).singleChar;
    taskId[1] = 0;
    int Id = strtoInt(taskId);
    UnregisterHotKey(NULL, Id);
}
/*========================================TASKS CRUD FUNCTIONS========================================*/
char *GetTaskJsonValue(const char *key, char *jsonMsg) {
    char* value = strstr(jsonMsg, key);
    if (!value) {
        fprintf(stderr, "Error: The %s was not found inside %s\n", key, jsonMsg);
        return 0;
    }
    char *value2 = allocateMemory(30);
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
void WriteTaskToFile() {
    FILE *fp = fopen(file, "w");
    if (fp == NULL) {
        printf("Error opening file!\n");
        return;
    }
    for (int iter = 0; iter < MAX_TASK_NUMBER; iter++) {
        if (task[iter].id == 0 || task[iter].filePath == NULL || task[iter].hotkey == NULL) {
            continue;
        }
        fprintf(fp, "%d, ", task[iter].id);
        fprintf(fp, "%s, ", task[iter].filePath);
        fprintf(fp, "%s\n ", task[iter].hotkey);
    }
    fclose(fp);
}
void ReloadSavedTask() {
    FILE *fp = fopen(file, "r");
    int iter = 0, count = 0, charCount = 0, structIndex = 0;
    char *hotkey = allocateMemory(50);
    char *buf = allocateMemory(200);
    char line[300]; //file format = id,filepath,hotkey

    while (fgets(line, sizeof(line), fp)) { //fgets stops reading when it sees '\n'
        if (strlen(line) > 0) {
            while (line[iter] != '\n' && line[iter] != '\0') {
                while (line[iter] != ',' && line[iter] != '\n' && line[iter] != '\0') {
                    buf[charCount] = line[iter];
                    iter++;
                    charCount++;
                }
                if (buf[0] == ' ') {
                    buf = buf + 1;
                }
                if (buf[strlen(buf)] == ' ') {
                    buf[strlen(buf)] = 0;
                }
                structIndex++;
                switch (structIndex) {
                case 1:
                    task[count].id = strtoInt(buf);
                    break;
                case 2:
                    task[count].filePath = allocateMemory(200);
                    if (strlen(buf) > 0) {
                        strcpy(task[count].filePath, buf);
                        taskCount++;
                    }
                    break;
                case 3:
                    task[count].hotkey = allocateMemory(50);
                    strcpy(task[count].hotkey, buf);
                    break;
                default:
                    printf("More than 3 items inserted in one line in the database file\n");
                    break;
                }
                iter++;
                memset(buf, 0, sizeof(*buf) * 200);
                charCount = 0;
            }
            task[count].func = OpenProgram;
            strcpy(hotkey, task[count].hotkey);
            hotkey[strlen(task[count].hotkey)] = '/';
            hotkey[strlen(task[count].hotkey) + 1] = '\0';
            char taskId[5];
            sprintf(taskId, "%d", task[count].id);
            strcat(hotkey, taskId);
            ParseHotkeyAdd(hotkey);
            count++;
        }
        iter = 0;
        structIndex = 0;
    }
    fclose(fp);
}
void AssignTask(int Id,char* hotkey, FuncPtr f,const char *filepath) {
    //The task should be assigned at the empty place i.e. id = 0
    int count = 0;
    for (int iter = 0; iter < MAX_TASK_NUMBER; iter++) {
        if (task[iter].id == 0) {
            count = iter;
            break;
        }
    }
    task[count].id = Id;
    task[count].hotkey = allocateMemory(strlen(hotkey));
    strcpy(task[count].hotkey, removeWhitespace(hotkey));
    task[count].func = f;
    task[count].filePath = (char *)malloc(sizeof(char) * 150);
    if (task[count].filePath == NULL) {
        return;
    }
    memset(task[count].filePath, 0, 150);
    strcpy(task[count].filePath, filepath);

    WriteTaskToFile();
}
void ParseTaskJson(char *jsonMessage) {  // Getting hotkey, id, filename from the json message.
    char *hotkey = GetTaskJsonValue("hotkey", jsonMessage);
    char *Id = GetTaskJsonValue("id", jsonMessage);
    const char *fileName = GetTaskJsonValue("file", jsonMessage);

    char *directoryBuff = allocateMemory(150);
    strncpy(directoryBuff, directory, strlen(directory));
    directoryBuff[strlen(directory)] = 0;
    strcat(directoryBuff, "\\");
    const char *filePath = strcat(directoryBuff, fileName);

    char *hotkeyInfo = allocateMemory(strlen(hotkey) + strlen(Id) + 3);
    strncpy(hotkeyInfo, hotkey, strlen(hotkey));
    hotkeyInfo[strlen(hotkey)] = '/';
    char *hotkeyID = allocateMemory(5);
    int uniqueID = generateUniqueID();
    sprintf(hotkeyID, "%d", uniqueID);  //changing from string to integer;
    strcat(hotkeyInfo, hotkeyID);
    
    AssignTask(uniqueID, hotkey, OpenProgram, filePath);
    ParseHotkeyAdd(hotkeyInfo);
}
bool ParseTaskDelete(char* s) {
    char *hotkey = allocateMemory(5);
    int taskID = 0;
    Delimiter d = getDelimitedWord(s, '=', true);
    strcpy(hotkey,removeWhitespace(d.fullString));
    for (int iter = 0; iter < taskCount; iter++) {
        if (task[iter].id != 0 && !strcmp(task[iter].hotkey,hotkey)) {
            taskID = task[iter].id;
            task[iter].id = 0;
            task[iter].hotkey = NULL;
            task[iter].func = NULL;
            task[iter].filePath = NULL;
            UnregisterHotKey(NULL, taskID);
            return true;
        }
    }
    return false;
}

int main(void) {
    mainThreadID = GetCurrentThreadId();
    CreateThread(NULL, 0, CreateNamedPipe, NULL, 0, NULL);
    task = (Task *)malloc(sizeof(Task) * MAX_TASK_NUMBER);
    if (!task) {
        return 0;
    }
    memset(task, 0, sizeof(Task) * MAX_TASK_NUMBER);
    char buffer[128];
    ReloadSavedTask();
    DWORD dwRead = 0;

    printf("AutoDesk Started......\n");

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
            }
            if (msg.message == WM_APP + 100){  //Message from frontend(Another Thread)
                printf("Got NamedPipe Message...\n");
                char *s = (char *)msg.wParam;
                if (s) {
                    printf("Received from pipe: %s\n", s);
                    Task_Action Task_Action_type = parsePipeTask(s);
                    if (Task_Action_type == Task_Action_Invalid) {
                        HotKey_Action Action_type = parsePipeHotkey(s);
                        switch (Action_type) {
                        case HotKey_Action_Add:
                            ParseHotkeyAdd(s);
                            break;
                        case HotKey_Action_Edit:
                            ParseHotkeyEdit(s);
                            break;
                        case HotKey_Action_Delete:
                            ParseHotkeyDelete(s);
                            break;
                        default:
                            printf("Todo\n");
                        }
                        free(s);
                    }
                    else {
                        //Handle the hotkey tasks...
                        switch (Task_Action_type) {
                        case Task_Action_Add:
                            ParseTaskJson(s);
                            taskCount++;
                            break;
                        case Task_Action_Edit:
                            printf("Edit Task Not supported\n");
                            break;
                        case Task_Action_Delete:
                           if (ParseTaskDelete(s)) {
                                taskCount = taskCount > 0 ? (taskCount - 1) : taskCount;
                           }
                            break;
                        default:
                            printf("Todo\n");
                        }
                        free(s); // free heap copy
                    }
                }
            }
        }
        Sleep(500); // Reduce CPU usage
    }
    CloseHandle(hPipe);
    return 0;
}
