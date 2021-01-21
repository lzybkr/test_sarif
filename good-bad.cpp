#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <vector>
#include <filesystem>
#include <fstream>

std::vector<char> read_file_stdcpp(wchar_t* filename)
{
    std::ifstream istrm(filename, std::ios::binary);
    auto size = std::filesystem::file_size(filename);
    std::vector<char> result(size);
    istrm.read(&result[0], size);

    return result;
}

std::vector<char> read_file_win32(wchar_t* filename)
{
    HANDLE hFile = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        throw "ERROR: unable to open file";
    }

    DWORD size = GetFileSize(hFile, 0);
    std::vector<char> result(size);

    DWORD read;
    BOOL res = ReadFile(hFile, &result[0], size, &read, NULL);
    if (res == FALSE) {
        throw "ERROR: unable to read file %S";
    }

    CloseHandle(hFile);

    return result;
}

std::vector<char> read_file_memory_mapped(wchar_t* filename)
{
    HANDLE hFile = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        throw "ERROR: unable to open file";
    }

    DWORD size = GetFileSize(hFile, 0);
    std::vector<char> result(size);

    HANDLE hMapFile = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, size, NULL);
    if (hMapFile == NULL) {
        throw "ERROR: unable to map file";
    }

    LPVOID lpMapAddress = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, size);
    if (lpMapAddress == NULL) {
        throw "ERROR: mapview failed";
    }

    memcpy(&result[0], lpMapAddress, size);
    return result;
}

std::vector<char> read_file(char method, wchar_t* filename)
{
    switch (method) {
    case '1': return read_file_stdcpp(filename);
    case '2': return read_file_win32(filename);
    case '3': return read_file_memory_mapped(filename);
    default: throw "Unexpected method to read file";
    }
}

void test1(char *buffer, int len)
{
    int cnt = 0;

    if (buffer[0] == 'b')
        cnt++;

    if (buffer[1] == 'a')
        cnt++;

    if (buffer[2] == 'd')
        cnt++;

    if (buffer[3] == '!')
        cnt++;

    if (cnt >= 4)
        * (volatile int*)NULL; // printf("false\n");
}

void test2(char *buffer)
{
    int cnt = 0;
    const char *sig = "bad!";

    for (int i = 0; i < 4; i++) {
        if (buffer[i] == sig[i])
            cnt++;
    }

    if (cnt >= 4)
        * (volatile int*)NULL;
}

int usage() {
    printf("USAGE: good-bad <TEST-ID> <INPUT-FILE>\n");
    printf("    <TEST-ID> must be one of: 1, 2\n");
    return 1;
}

int wmain(int argc, __in wchar_t** argv)
{
    if (argc != 3) {
        return usage();
    }

    printf("reading data file %S\n", argv[2]);

    try {
        std::vector<char> buffer = read_file(argv[1][0], argv[2]);
        test1(&buffer[0], buffer.size());
        printf("data file read - no crash\n");
    } catch(char* msg) {
        printf("%s\n", msg);
    }

    return 0;
}
