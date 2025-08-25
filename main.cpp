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



///////////HTTP SERVER///////////////
#include <iostream>
#include <thread>
#include <chrono>

extern "C" {
    #include "mongoose/mongoose.h"

    // Declare the callback implemented in callback.c
    void event_handler(struct mg_connection *c, int ev, void *ev_data);
}


#define MAX_HISTORY 20    // Maximum number of clipboard entries to store
#define BUFFER_SIZE 700   // Maximum size of each clipboard entry

bool textOverflow = false;

struct Tasks {
    int ID;
    char *hotkeys;
    void (*task)(char *userInput);
};





// HTTP server thread
void http_server_thread() {
    struct mg_mgr mgr;
    mg_mgr_init(&mgr); // Initialize Mongoose manager

    // Start HTTP server on port 5001
    mg_http_listen(&mgr, "http://localhost:5001", event_handler, NULL);

    std::cout << "HTTP server running on port 5001\n";

    // Poll loop
    while (true) {
        mg_mgr_poll(&mgr, 1000); // Poll every 1 second
    }

    mg_mgr_free(&mgr);
}




void OpenProgram(const char *path) {
    system(path);
}

void OpenGrep() {
    char path[200];
    char fileName[] = "’Ç‰Áƒ‰ƒxƒ‹ŒŸõ.bat";
    char new_path[300];
    char command[512];

    // Step 1: Ask user for destination folder path
    printf("Enter the destination folder path (e.g., C:\\Backup):\n");

    // Use scanf_s to safely read the path
    scanf_s("%s", path, (unsigned)_countof(path));

    // Step 2: Open the original .bat file in Notepad for editing
    system("notepad \"C:\\Users\\71163572\\Desktop\\personal\\TEJYUN\\GrepKensa\\’Ç‰Áƒ‰ƒxƒ‹ŒŸõ.bat\"");

    // Step 3: Create new full destination path
    sprintf_s(new_path, sizeof(new_path), "%s\\%s", path, fileName);

    // Step 4: Copy the file to the new path
    sprintf_s(command, sizeof(command),
        "copy \"C:\\Users\\71163572\\Desktop\\personal\\TEJYUN\\GrepKensa\\’Ç‰Áƒ‰ƒxƒ‹ŒŸõ.bat\" \"%s\"", new_path);
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

char PlusDelimiter(char *hotkeys) {
    int len = strlen(hotkeys);
    char *start = hotkeys;
    int iter = 0;
    while (iter < len) {
        if (hotkeys[iter] == '+') {
            start = &hotkeys[iter+1];
            return start[0];
        }
        iter++;
    }
    printf("No key combination character '+' was found.\n");
    return 0;
}

int RegisterHotKey_user(char *hotkeys, int actionID) {
    int winKey;
    char *Upper_keys = (char*)malloc(sizeof(*hotkeys)*strlen(hotkeys)+1);
    memset(Upper_keys, 0, sizeof(*Upper_keys)*strlen(hotkeys)+1);
    if (Upper_keys == NULL) return 0;
    for (int iter = 0; hotkeys[iter] != '\0'; iter++) {
        if (hotkeys[iter] == '+') {
            break;
        }
        Upper_keys[iter] = hotkeys[iter] - 'a' >= 0 ? hotkeys[iter] - 32 : hotkeys[iter];
    }
    Upper_keys[strlen(hotkeys)] = 0;
    if (!strncmp(Upper_keys, "ALT", 3)) {
        winKey = MOD_ALT;
    }
    else  if (!strncmp(Upper_keys, "CTRL", 4)) {
        winKey = MOD_CONTROL;
    }
    else if (!strncmp(Upper_keys, "SHIFT", 5)) {
        winKey = MOD_SHIFT;
    }
    else {
        printf("HotKeys not matched\n");
        return 0;
    }
    if (HotKey_ID > 9) {
        printf("THe maximum Number of hotkeys has been registered already\n. Please delete the existing one to add another\n");
        return 0;
    }
    if (!RegisterHotKey(NULL, actionID, winKey, PlusDelimiter(hotkeys))) {
        printf("Failed to register %s hotkey\n",hotkeys); 
        return 0; 
    }
    free(Upper_keys);
    return 1;
}

int main() {
	mainthreadId = GetCurrentThreadId();
    // Start HTTP server in a separate thread
    std::thread serverThread(http_server_thread);

    // Your existing hotkey/automation logic goes here
    std::cout << "Backend main running..." << std::endl;




   /* // Register hotkeys
    if (!RegisterHotKey(NULL, 1, MOD_ALT, 'A')) { printf("Failed to register Alt+A hotkey\n"); return 1; }
    if (!RegisterHotKey(NULL, 2, MOD_ALT, 'U')) { printf("Failed to register Alt+U hotkey\n"); return 1; }
    if (!RegisterHotKey(NULL, 3, MOD_ALT, 'L')) { printf("Failed to register Alt+L hotkey\n"); return 1; }
    if (!RegisterHotKey(NULL, 4, MOD_ALT, 'C')) { printf("Failed to register Alt+C hotkey\n"); return 1; }
    if (!RegisterHotKey(NULL, 5, MOD_ALT, 'S')) { printf("Failed to register Alt+S hotkey\n"); return 1; }
    if (!RegisterHotKey(NULL, 6, MOD_ALT, 'K')) { printf("Failed to register Alt+K hotkey\n"); return 1; }
    if (!RegisterHotKey(NULL, 7, MOD_ALT, 'P')) { printf("Failed to register Alt+P hotkey\n"); return 1; }
    if (!RegisterHotKey(NULL, 8, MOD_ALT, 'G')) { printf("Failed to register Alt+G hotkey\n"); return 1; }
    */
    char clipboardHistory[MAX_HISTORY][BUFFER_SIZE];
    int historyCount = 0;
    char previousClipboard[BUFFER_SIZE];
    GetClipboardText(previousClipboard, BUFFER_SIZE);

    FILE *file = NULL;
    errno_t err;

    printf("Monitoring clipboard... Press Ctrl+Q to EXIT.\n");
    printf("Ctrl+S = Save \n\n Alt+A = Save \n Alt+S = Source_Sabun \n Alt+K = Kairan Mail \n"
        "Alt+C = Chatgpt \n Alt+U = Uego_Folder \n Alt+L = Lin_Folder\n Alt+P = Personal_Folder\n"
        "Shift+Q = QAC\n Alt+G = Grep\n Shift+S = StampCheck\n");

    time_t startTime = 0;
    int iter = 0;
    bool fsave = FALSE;
    bool running = FALSE;

    while (1) {
        MSG msg;
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_USER + 100) {
                HotkeyRequest *req = (HotkeyRequest *)msg.lParam;
                if (!RegisterHotKey_user(req->hotkeys, req->id)) {
                    printf("Failed to register %s hotkey\n", req->hotkeys);
                }
                free(req->hotkeys); // free the string
                free(req); // free the struct
            }
            if (msg.message == WM_HOTKEY) {
                if (msg.wParam == 1) { 
                    printf("Alt+A pressed.\n"); 
                    fsave = TRUE; }
                if (msg.wParam == 2) { 
                    printf("Alt+U pressed.\n"); 
                    system("start \"\" \"C:\\Users\\kishan sah\\source\\repos\\Helloworld\"");
                }
                if (msg.wParam == 3) { 
                    printf("Alt+L pressed.\n"); 
                    system(" start C:\\Users\\71163572\\Desktop\\temp\\A_LIN‘Î‰ž"); }
               // if (msg.wParam == 4) { printf("Alt+C pressed.\n"); ShellExecute(NULL, "open", "https://chat.openai.com", NULL, NULL, SW_SHOWNORMAL); }
                if (msg.wParam == 5) { 
                    printf("Alt+S pressed.\n"); 
                    OpenProgram("C:\\Users\\71163572\\Desktop\\personal\\personal\\WinmergeAuto.py"); }
                if (msg.wParam == 6) { 
                    printf("Alt+K pressed.\n"); 
                    OpenProgram("C:\\Users\\71163572\\Desktop\\personal\\personal\\kairanMail.py"); }
                if (msg.wParam == 7) { 
                    printf("Alt+P pressed.\n"); 
                    system(" start C:\\Users\\71163572\\Desktop\\personal\\personal"); }
                if (msg.wParam == 8) { 
                    printf("Alt+G pressed.\n"); 
                    OpenGrep(); }
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
            if (ch == 81) {
                printf("Shift+Q pressed.\n");
                system("\"C:\\Program Files (x86)\\sakura\\sakura.exe\" C:\\Users\\71163572\\Desktop\\personal\\TEJYUN\\QAC_readMe.txt");
            }
            if (ch == 83) {
                printf("Shift+S pressed.\n");
                OpenProgram("C:\\Users\\71163572\\Desktop\\personal\\personal\\StampCheck.py");
            }
            if (ch == 19 || ch == 17) { // Ctrl+S
                printf("\nSaving...\n");
                err = fopen_s(&file, "C:\\Users\\71163572\\Desktop\\personal\\personal\\content_new.txt", "a");
                if (file != NULL && running == FALSE) {
                    fprintf(file, "\n\n\n");
                    running = TRUE;
                }
                if (file != NULL) {
                    for (int i = 0; i < historyCount; i++) {
                        fprintf(file, "%s\n", clipboardHistory[i]);
                    }
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
    }

    // Clean up hotkeys
    UnregisterHotKey(NULL, 1);
    UnregisterHotKey(NULL, 2);
    UnregisterHotKey(NULL, 3);
    UnregisterHotKey(NULL, 4);
    UnregisterHotKey(NULL, 5);
    UnregisterHotKey(NULL, 6);
    UnregisterHotKey(NULL, 7);
    UnregisterHotKey(NULL, 8);


    // Wait for HTTP server thread (never returns in infinite loop)
    serverThread.join();
    return 0;
}
