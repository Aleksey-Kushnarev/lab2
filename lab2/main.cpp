    #define _CRT_SECURE_NO_WARNINGS
    #include <Windows.h>  // ��������� ��� ������ � WinAPI
    #include <windowsx.h> // ��� ������� �������� ������ � ������������ � �����������
    #include <vector>      // ��� ������������� ������������ ��������
    #include <fstream>     // ��� ������ � ��������� �������� (����������/������ ������������)
    #include <sstream>     // ��� �������� ����� (��������, ���������������� ������)
    #include <iostream>
    #include <string>

    #include <cstdio>

    // ��������� ��� �������� ���������� � ������� (����� � ��������)
    struct Shape {
        int col, row;  // ���������� ������ � �����
        bool isCircle; // true - ����, false - �������

    };
    
    // ��������� ��� �������� ������������ ����������
    struct Config {
        int N = 5;                  // ������ ���� (N x N ������)
        int windowWidth = 320;      // ������ ����
        int windowHeight = 240;     // ������ ����
        COLORREF bgColor = RGB(0, 0, 255);  // ���� ���� (�� ��������� �����)
        COLORREF gridColor = RGB(255, 0, 0);  // ���� ����� (�� ��������� �������)
    };

    // ������������� �������� ����
    RECT rect;  // ���������� ����
    int red = 255, green = 0, blue = 0; // ��������� ���� (�������)
    int N;
    const int colorStep = 2;  // ��� ��������� ����� (�� ���� ��������� ������ ����)
    COLORREF gridLineColor;  // ���� ����� �����
    COLORREF backgroundColor;  // ���� ����
    int WIDTH, HEIGHT; // ������ � ������ ����
    int SizeCellsX, SizeCellsY; // ������ ����� ������ �� ����������� � ���������
    int chosenVariant = 1;
    int nFromCommandLine;

    const int initialX = 0;
    const int initialY = 0;
    int offsetX = 400; // �������� �� X (������ ���� + �����)
    int offsetY = 300; // �������� �� Y (������ ���� + �����)
    const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    const int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    struct SharedData {
        int instances;     // ������� �������� ��������
        int field[20][20];        
        COLORREF bgColor;
        int nextWindowX;          // ��������� ������� �� X
        int nextWindowY;          // ��������� ������� �� Y
    };


    const std::string configFileName = "config.txt";  // ��� ����� ��� ������������
    const std::wstring configFileNameW = L"config.txt";  // ��� ����� ��� ������������ (��� ������� ��������)

    Config config;  // ���������� ��� �������� ������������

    std::vector<Shape> shapes; // ������ ��� �������� ����� (����� � ��������)

    // ���������� ��������� ����
    LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // ������� ��� ������ � ������������� (������ � ����������)
    void ChangeBackgroundColor(HWND hwnd);

    // ���������������� ��������� ��� ���������� �������� ����
    // ������������ ��� ������������� ��������� ���� ����� ������������ ����������
    #define WM_GAME_UPDATE (WM_USER + 1)

    SharedData* sharedData = nullptr;
    HANDLE hMapping = NULL;
    HWND hwndMain = NULL;
    const wchar_t* SHARED_MEMORY_NAME = L"Global\\MainWinAPI";
    // ������� ������ � �������
    bool InitSharedMemory() {
        hMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEMORY_NAME);
        if (!hMapping) {
            hMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SharedData), SHARED_MEMORY_NAME);
            if (!hMapping) {
                MessageBox(NULL, L"�� ������� ������� ����������� ������.", L"������", MB_OK | MB_ICONERROR);
                return false;
            }
        }

        sharedData = (SharedData*)MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedData));
        if (!sharedData) {
            MessageBox(NULL, L"�� ������� ���������� ����������� ������.", L"������", MB_OK | MB_ICONERROR);
            return false;
        }
        return true;
    }

    void IncrementInstanceCount() {
        if (sharedData) {
            sharedData->instances++;
            if (sharedData->instances == 1) {
                ZeroMemory(sharedData->field, sizeof(sharedData->field));
            }
        }
    }

    void DecrementInstanceCount() {
        if (sharedData) {
            sharedData->instances--;
            if (sharedData->instances == 0) {
                ZeroMemory(sharedData->field, sizeof(sharedData->field));
            }
        }
    }


// ������� ��� �������� ������������ �� �����
// ������ ������������ �� �����
bool LoadConfigFstream(const std::string& filename, Config& config) {
    std::ifstream file(filename);
    if (!file) {
        return false; // ���� �� ����������, ���������� false
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        if (std::getline(iss, key, '=')) {
            if (key == "N") {
                iss >> config.N;
            }
            else if (key == "windowWidth") {
                iss >> config.windowWidth;
            }
            else if (key == "windowHeight") {
                iss >> config.windowHeight;
            }
            else if (key == "bgColor") {
                int r, g, b;
                char comma1, comma2;
                if (iss >> r >> comma1 >> g >> comma2 >> b && comma1 == ',' && comma2 == ',') {
                    config.bgColor = RGB(r, g, b);
                }
            }
            else if (key == "gridColor") {
                int r, g, b;
                char comma1, comma2;
                if (iss >> r >> comma1 >> g >> comma2 >> b && comma1 == ',' && comma2 == ',') {
                    config.gridColor = RGB(r, g, b);
                }
            }
        }
    }
    return true;
}

bool LoadConfigMemoryMapped(const std::wstring& filename, Config& config) {
    HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    HANDLE hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!hMap) {
        CloseHandle(hFile);
        return false;
    }

    LPVOID pMap = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
    if (!pMap) {
        CloseHandle(hMap);
        CloseHandle(hFile);
        return false;
    }

    std::string fileContent(static_cast<char*>(pMap)); // ����������� ������ � ������
    std::istringstream iss(fileContent);
    std::string line;

    while (std::getline(iss, line)) {
        std::istringstream lineStream(line);
        std::string key;
        if (std::getline(lineStream, key, '=')) {
            if (key == "N") {
                lineStream >> config.N;
            }
            else if (key == "windowWidth") {
                lineStream >> config.windowWidth;
            }
            else if (key == "windowHeight") {
                lineStream >> config.windowHeight;
            }
            else if (key == "bgColor") {
                int r, g, b;
                char comma1, comma2;
                if (lineStream >> r >> comma1 >> g >> comma2 >> b && comma1 == ',' && comma2 == ',') {
                    config.bgColor = RGB(r, g, b);
                }
            }
            else if (key == "gridColor") {
                int r, g, b;
                char comma1, comma2;
                if (lineStream >> r >> comma1 >> g >> comma2 >> b && comma1 == ',' && comma2 == ',') {
                    config.gridColor = RGB(r, g, b);
                }
            }
        }
    }

    UnmapViewOfFile(pMap);
    CloseHandle(hMap);
    CloseHandle(hFile);
    return true;
}


bool LoadConfigFOpen(const char* filename, Config& config) {
    FILE* file = nullptr;
    errno_t err = fopen_s(&file, filename, "r");  // ���������� fopen_s ������ fopen
    if (!file) return false;

    char key[50];
    while (fscanf(file, "%49[^=]=", key) == 1) {
        if (strcmp(key, "N") == 0) {
            fscanf(file, "%d\n", &config.N);
        }
        else if (strcmp(key, "windowWidth") == 0) {
            fscanf(file, "%d\n", &config.windowWidth);
        }
        else if (strcmp(key, "windowHeight") == 0) {
            fscanf(file, "%d\n", &config.windowHeight);
        }
        else if (strcmp(key, "bgColor") == 0) {
            int r, g, b;
            fscanf(file, "%d,%d,%d\n", &r, &g, &b);
            config.bgColor = RGB(r, g, b);
        }
        else if (strcmp(key, "gridColor") == 0) {
            int r, g, b;
            fscanf(file, "%d,%d,%d\n", &r, &g, &b);
            config.gridColor = RGB(r, g, b);
        }
    }

    fclose(file);
    return true;
}

bool LoadConfigWinAPI(const std::wstring& filename, Config& config) {
    HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    char buffer[1024] = { 0 };
    DWORD bytesRead;
    if (!ReadFile(hFile, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
        CloseHandle(hFile);
        return false;
    }

    std::string fileContent(buffer, bytesRead); // ���������� bytesRead, ����� ��������� ������ ��������� ������
    std::istringstream iss(fileContent);
    std::string line;

    while (std::getline(iss, line)) {
        std::istringstream lineStream(line);
        std::string key;
        if (std::getline(lineStream, key, '=')) {
            if (key == "N") {
                lineStream >> config.N;
            }
            else if (key == "windowWidth") {
                lineStream >> config.windowWidth;
            }
            else if (key == "windowHeight") {
                lineStream >> config.windowHeight;
            }
            else if (key == "bgColor") {
                int r, g, b;
                char comma1, comma2;
                if (lineStream >> r >> comma1 >> g >> comma2 >> b && comma1 == ',' && comma2 == ',') {
                    config.bgColor = RGB(r, g, b);
                }
            }
            else if (key == "gridColor") {
                int r, g, b;
                char comma1, comma2;
                if (lineStream >> r >> comma1 >> g >> comma2 >> b && comma1 == ',' && comma2 == ',') {
                    config.gridColor = RGB(r, g, b);
                }
            }
        }
    }

    CloseHandle(hFile);
    return true;
}

// ������� ��� ���������� ������������ � ����
bool SaveConfigFstream(const std::string& filename, const Config& config) {
    std::ofstream file(filename);
    if (!file) {
        return false; // ������ ��� �������� ����� ��� ������
    }

    file << "N=" << config.N << std::endl;
    file << "windowWidth=" << config.windowWidth << std::endl;
    file << "windowHeight=" << config.windowHeight << std::endl;
    // ������ ������ � ���������������� ����
     // ��������� ����� � ������� R,G,B (������ ��������� �����)
    file << "bgColor=" << (short)GetRValue(config.bgColor) << ","
        << (short)GetGValue(config.bgColor) << ","
        << (short)GetBValue(config.bgColor) << std::endl;

    file << "gridColor=" << (short)GetRValue(config.gridColor) << ","
        << (short)GetGValue(config.gridColor) << ","
        << (short)GetBValue(config.gridColor) << std::endl;
}


bool SaveConfigMemoryMapped(const std::wstring& filename, const Config& config) {
    HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    HANDLE hMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 1024, NULL);
    if (!hMap) {
        CloseHandle(hFile);
        return false;
    }

    LPVOID pMap = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!pMap) {
        CloseHandle(hMap);
        CloseHandle(hFile);
        return false;
    }

    // ��������� ������ ������������
    std::string configStr =
        "N=" + std::to_string(config.N) + "\n" +
        "windowWidth=" + std::to_string(config.windowWidth) + "\n" +
        "windowHeight=" + std::to_string(config.windowHeight) + "\n" +
        "bgColor=" + std::to_string(GetRValue(config.bgColor)) + "," +
        std::to_string(GetGValue(config.bgColor)) + "," +
        std::to_string(GetBValue(config.bgColor)) + "\n" +
        "gridColor=" + std::to_string(GetRValue(config.gridColor)) + "," +
        std::to_string(GetGValue(config.gridColor)) + "," +
        std::to_string(GetBValue(config.gridColor)) + "\n";

    CopyMemory(pMap, configStr.c_str(), configStr.size() + 1);

    UnmapViewOfFile(pMap);
    CloseHandle(hMap);
    CloseHandle(hFile);
    return true;
}


bool SaveConfigFopen(const char* filename, const Config& config) {
    FILE* file = nullptr;
    errno_t err = fopen_s(&file, filename, "w");  // ���������� fopen_s ������ fopen
    if (!file) return false;

    fprintf(file, "N=%d\n", config.N);
    fprintf(file, "windowWidth=%d\n", config.windowWidth);
    fprintf(file, "windowHeight=%d\n", config.windowHeight);
    fprintf(file, "bgColor=%d,%d,%d\n",
        GetRValue(config.bgColor), GetGValue(config.bgColor), GetBValue(config.bgColor));
    fprintf(file, "gridColor=%d,%d,%d\n",
        GetRValue(config.gridColor), GetGValue(config.gridColor), GetBValue(config.gridColor));

    fclose(file);
    return true;
}

bool SaveConfigWinAPI(const std::wstring& filename, const Config& config) {
    HANDLE hFile = CreateFile(filename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    std::string configStr =
        "N=" + std::to_string(config.N) + "\n" +
        "windowWidth=" + std::to_string(config.windowWidth) + "\n" +
        "windowHeight=" + std::to_string(config.windowHeight) + "\n" +
        "bgColor=" + std::to_string(GetRValue(config.bgColor)) + "," +
        std::to_string(GetGValue(config.bgColor)) + "," +
        std::to_string(GetBValue(config.bgColor)) + "\n" +
        "gridColor=" + std::to_string(GetRValue(config.gridColor)) + "," +
        std::to_string(GetGValue(config.gridColor)) + "," +
        std::to_string(GetBValue(config.gridColor)) + "\n";

    DWORD bytesWritten;
    WriteFile(hFile, configStr.c_str(), configStr.size(), &bytesWritten, NULL);

    CloseHandle(hFile);
    return true;
}




// �������� �������, ������� �������������� � ��������� ����������
int WINAPI wWinMain(HINSTANCE hInt, HINSTANCE hPreve, PWSTR pCom, int nCmdShow)
{
    if (pCom && *pCom) {
        std::wistringstream wiss(pCom);
        std::wstring arg;
        std::vector<std::wstring> args;

        while (wiss >> arg) { // ��������� �� ��������
            args.push_back(arg);
        }

        if (!args.empty()) {
            int userVariant = _wtoi(args[0].c_str());
            if (userVariant > 0 && userVariant <= 4) {
                chosenVariant = userVariant;
            }
        }

        if (args.size() > 1) {
            int userN = _wtoi(args[1].c_str());
            if (userN >= 3 && userN <= 20) {
                nFromCommandLine = userN;
            }
        }
    }

    switch (chosenVariant)
    {
    case 1:
        // ��������� ������������ �� �����
        if (!LoadConfigMemoryMapped(configFileNameW, config)) {
            // ���� ���� �� ����������, ������� ��� � ����������� �� ���������
            SaveConfigMemoryMapped(configFileNameW, config);

        }
        break;
    case 2:
        if (!LoadConfigFOpen(configFileName.c_str(), config)) {
            // ���� ���� �� ����������, ������� ��� � ����������� �� ���������
            SaveConfigFopen(configFileName.c_str(), config);
        }
        break;
    case 3:
        if (!LoadConfigFstream(configFileName, config)) {
            // ���� ���� �� ����������, ������� ��� � ����������� �� ���������
            SaveConfigFstream(configFileName, config);
        }

    case 4:
        if (!LoadConfigWinAPI(configFileNameW, config)) {
            // ���� ���� �� ����������, ������� ��� � ����������� �� ���������
            SaveConfigWinAPI(configFileNameW, config);
        }
        break;
        
    }



    
    rect = { 0, 0, config.windowWidth, config.windowHeight };
    gridLineColor = config.gridColor;
    
    N = config.N;

    // ������������ ������� ���� ��� ������� ����
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    // ���������, ��� �� ������� �������� ��������� ������ (�������� N)
    if (nFromCommandLine) {
        N = nFromCommandLine;
        config.N = N;
    }
    
    if (!InitSharedMemory()) {
        return -1; // ������ �������������
    }
    sharedData->bgColor = config.bgColor;
    backgroundColor = sharedData->bgColor;
    // ��������� ������� ���� ���, ����� ��� �������� �� ����� ������
    WIDTH = (config.windowWidth) / N * N;
    HEIGHT = (config.windowHeight) / N * N;
    SizeCellsX = WIDTH / N;
    SizeCellsY = HEIGHT / N;
    offsetY = HEIGHT;
    offsetX = WIDTH;

    // ���������� ���� �� ��������� �������
    int windowX = sharedData->nextWindowX;
    int windowY = sharedData->nextWindowY;

    // ����������� ���������� ��� ���������� ����
    sharedData->nextWindowX += offsetX;

    // ���� ���� ������� �� ������ ���� ������, ��������� ��� �� ����� ������
    if (sharedData->nextWindowX + config.windowWidth > screenWidth) {
        sharedData->nextWindowX = initialX;  // ������� �� ��������� ������� �� X
        sharedData->nextWindowY += offsetY;  // ����������� ���� �� Y
    }

    // ���� ���� ������� �� ������ ���� ������, ����� ������������ ��� �������� (���� �����)
    if (sharedData->nextWindowY + config.windowHeight > screenHeight) {
        sharedData->nextWindowY = initialY;  // ������� �� ��������� ������� �� Y (���� �����)
        // ����� �������� sharedData->nextWindowX, ���� ������ ������ � ������ �������� ����
        sharedData->nextWindowX = initialX;
    }

    // ����������� ������ ����
    // ������������� ��������� WNDCLASS, ������� ������������ ��� ����������� ������ ����.
    // ��� ��������� ������ ���������, ��������� � ������, ����� ��� ������, ������, ���������� ��������� � �.�.
    WNDCLASS SoftwareWindowsClass = { 0 };  // ��� ���� ��������� WNDCLASS ���������������� ������ ��� ������������ � ������������

    // ��������� ������ ��� ����. ����� ������������ ����������� ������ "������" �� ��������� ��������.
    // ����� �������� �� ���� ������.
    SoftwareWindowsClass.hIcon = LoadIcon(NULL, IDI_QUESTION);  // ������������� ������ ���� (�������������� ����)

    // ��������� ����������� ������ "�������", ������� ����� ������������ ��� ��������� ���� �� ����.
    // ����� �������� ��� �� ����� ������ ������, ���� ����������.
    SoftwareWindowsClass.hCursor = LoadCursor(NULL, IDC_ARROW);  // ������������� ����������� ������ (�������)

    // ������������� ��������� �� ��������� ���������� (hInt). ���� �������� ����� ��� ��������� �������� � ���� � ������ ������ ����������.
    SoftwareWindowsClass.hInstance = hInt;  // ����������� ��������� ����������

    // ��������� ���������� ��� ������ ����. ��� ��� ����� �������������� ��� �������� ����.
    // ��� ������ ���� ��������� � ������ ������ ����������, ����� �������� ����������.
    SoftwareWindowsClass.lpszClassName = L"MainWinAPIClass";  // ��� ������ ����, ���������� ��� ������ ����������

    // ������������� ���� ���� ����. ����� ������������ ������� CreateSolidBrush ��� �������� ����� � �������� ������.
    // � ������ ������ ������ ����� ���� (RGB(0, 0, 255)).
    SoftwareWindowsClass.hbrBackground = CreateSolidBrush(backgroundColor);  // ����� ��� ���� ���� (�����)

    // ������������� ��������� �� �������, ������� ����� ������������ ��� ���������, ����������� � ��� ����.
    // ��� ������� ����� ������������ ����� �������, ��� ������� ������, ��������� �������� ���� � ������ ���������.
    SoftwareWindowsClass.lpfnWndProc = WindowProc;  // �������, �������������� ��������� ��� ���� ����� ������



    // ����������� ����
    if (!RegisterClassW(&SoftwareWindowsClass)) {
        return -1;
    }

    // �������� ���� � ������� ���������
    CreateWindowW(
        L"MainWinAPIClass",
        L"Firsr C++ WinApi Application",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        windowX,
        windowY,
        WIDTH + (GetSystemMetrics(SM_CXFRAME) * 2) + 10, // ������ � ������ ����� � ��������
        HEIGHT + GetSystemMetrics(SM_CYFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION) + 10,
        NULL,
        NULL,
        NULL,
        NULL
    );

    // �������� ���� ��������� ���������
    MSG SoftwareMsg = { 0 };
    while (GetMessage(&SoftwareMsg, NULL, NULL, NULL)) {
        // ���� ��������� ���������. �� ��������� ��������� �� ������� ��������� � �������� �� ��� ���������.

        // ��� ������� ����������� ��������� �� ���������� � ������ �������, ����� ��� WM_CHAR ��� ������ ���������,
        // ������� ����� ������ ������������ � ����. ��� ������������� ���������, ���� � ��������� ���������� ���������� � ��������.
        TranslateMessage(&SoftwareMsg);  // ��������� ��������� �� ���������� � ������ �������, ��������, ������� ��� ���� ������

        // ��� ������� ���������� ��������� ��� ��������� � ��������� ��������� ��������� ���� (WindowProc).
        // ��� ���������, ����� ��� ������� ������, �������� ����, ��������� ���� � ������, ����� ���������� � ������� WindowProc.
        DispatchMessage(&SoftwareMsg);   // ���������� ��������� � ������� ��������� ��������� ��� ���������� ��������� � ����

    }
    return 0;
}

HWND hwndSource = NULL; // ���������� ����������

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    wchar_t className[256];
    GetClassName(hwnd, className, sizeof(className) / sizeof(wchar_t));

    if (wcscmp(className, L"MainWinAPIClass") == 0 && hwnd != hwndSource)
    {
        PostMessage(hwnd, WM_GAME_UPDATE, 0, 0);
    }
    return TRUE;
}


// ������� ��� ��������� ��������� ����
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {

    case WM_CREATE:
        IncrementInstanceCount();
        break;

    case WM_DESTROY:
        // ��������� ������������ ����� ��������� ����������
        switch (chosenVariant)
        {
        case 1:
                SaveConfigMemoryMapped(configFileNameW, config);
        
        case 2:
                SaveConfigFopen(configFileName.c_str(), config);
            
        case 3:
                SaveConfigFstream(configFileName, config);
            

        case 4:
                SaveConfigWinAPI(configFileNameW, config);
            

        }
        sharedData->instances--;
        DecrementInstanceCount();

        if (sharedData) {
            UnmapViewOfFile(sharedData);
            sharedData = nullptr;
        }
        if (hMapping) {
            CloseHandle(hMapping);
            hMapping = NULL;
        }
        


        PostQuitMessage(0); // ��������� ����������
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT pnt;
        HDC hdc = BeginPaint(hwnd, &pnt);
        backgroundColor = sharedData->bgColor;
        // �������� ���
        HBRUSH hBrush = CreateSolidBrush(backgroundColor);
        FillRect(hdc, &rect, hBrush);
        DeleteObject(hBrush);

        // ������ �����
        HPEN hPen = CreatePen(PS_SOLID, 1, gridLineColor);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        for (int i = 0; i <= N; i++) {
            MoveToEx(hdc, SizeCellsX * i, 0, NULL);
            LineTo(hdc, SizeCellsX * i, HEIGHT);

            MoveToEx(hdc, 0, SizeCellsY * i, NULL);
            LineTo(hdc, WIDTH, SizeCellsY * i);
        }
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);

        // ������ �������� � ������ �� sharedData->field
        HPEN hShapePen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255)); // ׸���� ����� ��� �����
        hOldPen = (HPEN)SelectObject(hdc, hShapePen);
        OutputDebugString(L"WM_PAINT ����������\n");
        for (int y = 0; y < N; y++) {
            for (int x = 0; x < N; x++) {
                int left = x * SizeCellsX;
                int top = y * SizeCellsY;
                int right = (x + 1) * SizeCellsX;
                int bottom = (y + 1) * SizeCellsY;

                if (sharedData && sharedData->field) {
                    int symbol = sharedData->field[y][x];

                    if (symbol == 1) { // �������
                        MoveToEx(hdc, left, top, NULL);
                        LineTo(hdc, right, bottom);
                        MoveToEx(hdc, right, top, NULL);
                        LineTo(hdc, left, bottom);
                    }
                    else if (symbol == 2) { // �����
                        Arc(hdc, left, top, right, bottom, 0, 0, 0, 0);
                    }
                }
            }
        }

        SelectObject(hdc, hOldPen);
        DeleteObject(hShapePen);

        EndPaint(hwnd, &pnt);
        return 0;
    }

    case WM_RBUTTONDOWN: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

        if (x >= SizeCellsX * N || y >= SizeCellsY * N) {
            MessageBox(NULL, L"���� �� ���������", L"������", MB_OK | MB_ICONERROR);
            return 0;
        }  // ���� �� ���������

        int col = x / SizeCellsX;
        int row = y / SizeCellsY;

       
        

        // �������� �� ������ �����
        if (sharedData->field[row][col] == 0) { // ���� ������ ����� (0)
            sharedData->field[row][col] = 1; // ����� ������� (1) � ����� ������
            
            // �������� ���� ����� ������������ ����
            EnumWindows(EnumWindowsProc, 0);
        }
        

        return 0;
    }


    case WM_GAME_UPDATE: {
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }


    case WM_LBUTTONDOWN: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

        if (x >= SizeCellsX * N || y >= SizeCellsY * N) {
            MessageBox(NULL, L"���� �� ���������", L"������", MB_OK | MB_ICONERROR);
            return 0;
        }  // ���� �� ���������

        int col = x / SizeCellsX;
        int row = y / SizeCellsY;




        // �������� �� ������ �����
        if (sharedData->field[row][col] == 0) { // ���� ������ ����� (0)
            sharedData->field[row][col] = 2; // ����� ������ (2) � ����� ������
            
            // �������� ���� ����� ������������ ����
            EnumWindows(EnumWindowsProc, 0);
        }


        return 0;
    }

    

    case WM_SIZE:
    {
        // ���������� ��������� �������� ����
        WIDTH = LOWORD(lParam);  // ����� ������ ����
        HEIGHT = HIWORD(lParam); // ����� ������ ����
        config.windowWidth = WIDTH;
        config.windowHeight = HEIGHT;
        SizeCellsX = WIDTH / N;  // ����� ������ ������
        SizeCellsY = HEIGHT / N;
        InvalidateRect(hwnd, NULL, TRUE); // ������ ��� ���� ������������
        //�������������� ����
        return 0;
    }
    case WM_KEYDOWN: {
        // ���������� ������� ������
        if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && wParam == 'Q') {
            // ��������� ���� �� ���������� Ctrl+Q
            switch (chosenVariant)
            {
            case 1:
                SaveConfigMemoryMapped(configFileNameW, config);

            case 2:
                SaveConfigFopen(configFileName.c_str(), config);

            case 3:
                SaveConfigFstream(configFileName, config);


            case 4:
                SaveConfigWinAPI(configFileNameW, config);


            }
            PostMessage(hwnd, WM_CLOSE, 0, 0);
        }
        if (wParam == VK_ESCAPE) {
            // ��������� ���� �� ������� ESC
            switch (chosenVariant)
            {
            case 1:
                SaveConfigMemoryMapped(configFileNameW, config);

            case 2:
                SaveConfigFopen(configFileName.c_str(), config);

            case 3:
                SaveConfigFstream(configFileName, config);


            case 4:
                SaveConfigWinAPI(configFileNameW, config);


            }
            PostMessage(hwnd, WM_CLOSE, 0, 0);
        }

        if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) && wParam == 'C') {
            // ������ �������� �� ���������� Shift+C
            ShellExecute(NULL, L"open", L"notepad.exe", NULL, NULL, SW_SHOWNORMAL);
        }

        if (wParam == VK_RETURN) {
            // �������� ���� ���� �� ��������� ��� ������� Enter
            ChangeBackgroundColor(hwnd);
        }
        return 0;
    }

    case WM_MOUSEWHEEL: {
        // ������� ��������� ����� ����� ����� ��� ��������� ������ ����
        int wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        if (wheelDelta > 0) {  // ��������� �����
            red = (red + colorStep) % 256;  // ����������� ������� ���������
        }
        else {  // ��������� ����
            red = (red - colorStep + 256) % 256;  // ��������� ������� ���������
        }
        gridLineColor = RGB(red, blue, green);
        config.gridColor = gridLineColor;
        InvalidateRect(hwnd, NULL, TRUE);  // �������������� ����

        return 0;
    }
    
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);  // ��������� ��������� ���������
    }
}

// ������� ��� ��������� ����� ����
void ChangeBackgroundColor(HWND hwnd) {
    // ��������� ���������� ����� ��� ����
    backgroundColor = RGB(rand() % 256, rand() % 256, rand() % 256);
    config.bgColor = backgroundColor;
    // ������������� ����� ���� ����
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(backgroundColor));
    sharedData->bgColor = backgroundColor;
    EnumWindows(EnumWindowsProc, 0);
}

