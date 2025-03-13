#define _CRT_SECURE_NO_WARNINGS
#include <vector> 
#include <cstdio>
#include <iostream>
#include <fstream>
#include <chrono>
#include <Windows.h>
#include <cstring> // Для memset
#include <string>


const std::string FILENAME = "testfile.txt";
const std::wstring FILENAMEW = L"testfile.txt";
std::vector<long long> s1;
std::vector<long long> s2;
std::vector<long long> s3;
std::vector<long long> s4;

long size = 1024 * 1024;

// Функция для измерения времени
template<typename F>
long long measureExecutionTime(F&& func) {
    auto start = std::chrono::high_resolution_clock::now();

    func(); // Выполняем переданную функцию

    auto end = std::chrono::high_resolution_clock::now();
    // Измерение времени в микросекундах
    std::chrono::duration<long long, std::micro> duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count();  // Возвращаем результат как long long
}

// Метод 1: Создание файла с помощью fstream
void createFileWithFstream() {
    std::ofstream file(FILENAME, std::ios::binary);
    if (file) {
        // Динамическое выделение памяти через new
        char* data = new char[size]; // 1024 KB
        memset(data, 'A', size); // Заполнение памяти символами 'A'

        // Запись данных в файл
        file.write(data, size);

        if (file.good()) {
            std::cout << "File good" << std::endl;
        }
        else {
            std::cerr << "File bad" << std::endl;
        }

        file.close();

        // Не забывайте освободить память
        delete[] data;
    }
    else {
        std::cerr << "Ошибка при создании файла с использованием fstream" << std::endl;
    }
    
}

// Метод 2: Чтение с использованием CreateFileMapping / MapViewOfFile
void readWithMemoryMappedFile() {
    HANDLE hFile = CreateFile(FILENAMEW.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Ошибка при открытии файла с использованием CreateFile" << std::endl;
        return;
    }
    HANDLE hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hMap == NULL) {
        std::cerr << "Ошибка при создании отображения файла" << std::endl;
        CloseHandle(hFile);
        return;
    }
    LPVOID pMap = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
    if (pMap == NULL) {
        std::cerr << "Ошибка при отображении файла в память" << std::endl;
        CloseHandle(hMap);
        CloseHandle(hFile);
        return;
    }

    char* buffer = new char[size];
    memcpy(buffer, pMap, sizeof(buffer));

    UnmapViewOfFile(pMap);
    CloseHandle(hMap);
    CloseHandle(hFile);
    delete[] buffer;
}

// Метод 3: Чтение с использованием fopen/fread/fclose
void readWithFopen() {
    FILE* file = fopen(FILENAME.c_str(), "rb");
    if (file == NULL) {
        std::cerr << "Ошибка при открытии файла с использованием fopen" << std::endl;
        return;
    }
    char* buffer = new char[size];
    fread(buffer, sizeof(char), sizeof(buffer), file);
    fclose(file);
    delete[] buffer;
}

// Метод 4: Чтение с использованием fstream
void readWithFstream() {
    std::ifstream file(FILENAME, std::ios::binary);
    if (!file) {
        std::cerr << "Ошибка при открытии файла с использованием fstream" << std::endl;
        return;
    }
    char* buffer = new char[size];
    file.read(buffer, sizeof(buffer));
    file.close();
    delete[] buffer;
}

// Метод 5: Чтение с использованием WinAPI / NativeAPI
void readWithWinAPI() {
    HANDLE hFile = CreateFile(FILENAMEW.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Ошибка при открытии файла с использованием WinAPI" << std::endl;
        return;
    }

    DWORD bytesRead;

    char* buffer = new char[size];
    ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL);

    CloseHandle(hFile);
    delete[] buffer;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    // Шаг 1: Создание файла размером 1024 KB
    createFileWithFstream();
    double res;

    int repeats = 100;
    // Шаг 2: Измерение времени для каждого метода
    for (int i = 0; i < repeats; i++) {
        res = static_cast<double>(measureExecutionTime([]() { readWithMemoryMappedFile(); }));
        s1.push_back(res);
        
        res = static_cast<double>(measureExecutionTime([]() { readWithFopen(); }));
        s2.push_back(res);

        res = static_cast<double>(measureExecutionTime([]() { readWithFstream(); }));
        s3.push_back(res);
        res = static_cast<double>(measureExecutionTime([]() { readWithWinAPI(); }));
        s4.push_back(res);

        
    }
    double sum = 0;
    std::cout << "=======================" << std::endl;
    std::cout << "MemoryMappedFile" << std::endl;
    for (long long value : s1) {
        sum += value;
    }
    std::cout << "Average time " << sum / repeats << std::endl;
    std::cout << "Total time " << sum << std::endl;

    std::cout << "=======================" << std::endl;
    std::cout << "Fopen" << std::endl;
    sum = 0;
    for (long long value : s2) {
        sum += value;
    }

    std::cout << "Average time " << sum / repeats << std::endl;
    std::cout << "Total time " << sum << std::endl;
    std::cout << "=======================" << std::endl;
    std::cout << "Fstream" << std::endl;
    sum = 0;
    for (long long value : s3) {
        sum += value;
    }

    std::cout << "Average time " << sum / repeats << std::endl;
    std::cout << "Total time " << sum << std::endl;

    std::cout << "=======================" << std::endl;
    std::cout << "WinAPI" << std::endl;
    sum = 0;
    for (long long value : s4) {
        sum += value;
    }

    std::cout << "Average time " << sum / repeats << std::endl;
    std::cout << "Total time " << sum << std::endl;
    

    std::cout << "Done" << std::endl;
    return 0;
}
