#ifndef VERSION_DUMP_H
#define VERSION_DUMP_H

#include <Windows.h>
#include <Shlobj.h>
#include <string>
#include <mutex>
#include <map>

using namespace std;

mutex dumpMutex;
map<u_short, int *> socketPortLastDirectionMap;

bool getFilePath(wstring &path, const char *name) {
    path.resize(MAX_PATH);
    if (!SHGetSpecialFolderPathW(HWND_DESKTOP, path.data(), CSIDL_DESKTOP, FALSE)) {
        return FALSE;
    }
    path.resize(wcslen(path.data()));

    wstring fixedName(MAX_PATH, '\x00');
    mbstowcs(fixedName.data(), name, strlen(name));
    fixedName.resize(wcslen(fixedName.data()));

    path += L"\\SOCKDUMP\\";
    path += fixedName;
    path += L".txt";

    return TRUE;
}

void printBuf(FILE *fp, const unsigned char *buf, int len, int *lastDirection, int direction) {
    if (*lastDirection == 0 || *lastDirection != direction) {
        if (direction == 1) {
            fprintf(fp, "\nSEND: ");
        } else if (direction == 2) {
            fprintf(fp, "\nRECV: ");
        } else {
            fprintf(fp, "\nUNKN: ");
        }
        *lastDirection = direction;
    }
    for (int i = 0; i < len; ++i)
        fprintf(fp, "%02X", buf[i]);
}

int *getLastDirection(u_short port) {
    if (auto lastDirectionResult = socketPortLastDirectionMap.find(port); lastDirectionResult !=
                                                                          socketPortLastDirectionMap.end()) {
        return lastDirectionResult->second;
    } else {
        int *ptr = new int;
        *ptr = 0;
        socketPortLastDirectionMap[port] = ptr;
        return ptr;
    }
}

void DoDump(const char *buf, int len, u_short port, int direction) {
    if (!buf || len < 1) {
        return;
    }
    lock_guard<mutex> lock(dumpMutex);
    FILE *fp;
    wstring path;
    string name = to_string(port);
    int *lastDirection = getLastDirection(port);
    if (getFilePath(path, name.c_str())) {
        fp = _wfopen(path.c_str(), L"ab");
        printBuf(fp, (const unsigned char *) buf, len, lastDirection, direction);
        fclose(fp);
    } else {
        MessageBox(nullptr, "Error 1", "Error", MB_OK);
    }
}

#endif //VERSION_DUMP_H
