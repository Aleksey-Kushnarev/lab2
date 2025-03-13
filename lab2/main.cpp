#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>  // Заголовок для работы с WinAPI
#include <windowsx.h> // Для удобных макросов работы с координатами и сообщениями
#include <vector>      // Для использования динамических массивов
#include <fstream>     // Для работы с файловыми потоками (сохранение/чтение конфигурации)
#include <sstream>     // Для парсинга строк (например, конфигурационных файлов)
#include <iostream>
#include <string>

#include <cstdio>

// Структура для хранения информации о фигурах (круги и крестики)
struct Shape {
    int col, row;  // Координаты фигуры в сетке
    bool isCircle; // true - круг, false - крестик

};

// Структура для хранения конфигурации приложения
struct Config {
    int N = 5;                  // Размер поля (N x N клеток)
    int windowWidth = 320;      // Ширина окна
    int windowHeight = 240;     // Высота окна
    COLORREF bgColor = RGB(0, 0, 255);  // Цвет фона (по умолчанию синий)
    COLORREF gridColor = RGB(255, 0, 0);  // Цвет сетки (по умолчанию красный)
};

// Инициализация размеров окна
RECT rect;  // Координаты окна
int red = 255, green = 0, blue = 0; // Начальный цвет (красный)
int N;
const int colorStep = 2;  // Шаг изменения цвета (по мере прокрутки колеса мыши)
COLORREF gridLineColor;  // Цвет линий сетки
COLORREF backgroundColor;  // Цвет фона
int WIDTH, HEIGHT; // Ширина и высота окна
int SizeCellsX, SizeCellsY; // Размер одной клетки по горизонтали и вертикали
int chosenVariant = 1;
int nFromCommandLine;


const std::string configFileName = "config.txt";  // Имя файла для конфигурации
const std::wstring configFileNameW = L"config.txt";  // Имя файла для конфигурации (для широких символов)

Config config;  // Переменная для хранения конфигурации

std::vector<Shape> shapes; // Массив для хранения фигур (круги и крестики)

// Обработчик сообщений окна
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Функции для работы с конфигурацией (чтение и сохранение)
void ChangeBackgroundColor(HWND hwnd);




// Функция для загрузки конфигурации из файла
// Чтение конфигурации из файла
bool LoadConfigFstream(const std::string& filename, Config& config) {
    std::ifstream file(filename);
    if (!file) {
        return false; // Файл не существует, возвращаем false
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

    std::string fileContent(static_cast<char*>(pMap)); // Преобразуем память в строку
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
    errno_t err = fopen_s(&file, filename, "r");  // Используем fopen_s вместо fopen
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

    std::string fileContent(buffer, bytesRead); // Используем bytesRead, чтобы учитывать только считанные данные
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

// Функция для сохранения конфигурации в файл
bool SaveConfigFstream(const std::string& filename, const Config& config) {
    std::ofstream file(filename);
    if (!file) {
        return false; // Ошибка при открытии файла для записи
    }

    file << "N=" << config.N << std::endl;
    file << "windowWidth=" << config.windowWidth << std::endl;
    file << "windowHeight=" << config.windowHeight << std::endl;
    // Запись цветов в конфигурационный файл
     // Сохраняем цвета в формате R,G,B (каждый компонент числа)
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

    // Формируем строку конфигурации
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
    errno_t err = fopen_s(&file, filename, "w");  // Используем fopen_s вместо fopen
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




// Основная функция, которая инициализирует и запускает приложение
int WINAPI wWinMain(HINSTANCE hInt, HINSTANCE hPreve, PWSTR pCom, int nCmdShow)
{
    if (pCom && *pCom) {
        std::wistringstream wiss(pCom);
        std::wstring arg;
        std::vector<std::wstring> args;

        while (wiss >> arg) { // Разделяем по пробелам
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
        // Загружаем конфигурацию из файла
        if (!LoadConfigMemoryMapped(configFileNameW, config)) {
            // Если файл не существует, создаем его с настройками по умолчанию
            SaveConfigMemoryMapped(configFileNameW, config);

        }
        break;
    case 2:
        if (!LoadConfigFOpen(configFileName.c_str(), config)) {
            // Если файл не существует, создаем его с настройками по умолчанию
            SaveConfigFopen(configFileName.c_str(), config);
        }
        break;
    case 3:
        if (!LoadConfigFstream(configFileName, config)) {
            // Если файл не существует, создаем его с настройками по умолчанию
            SaveConfigFstream(configFileName, config);
        }

    case 4:
        if (!LoadConfigWinAPI(configFileNameW, config)) {
            // Если файл не существует, создаем его с настройками по умолчанию
            SaveConfigWinAPI(configFileNameW, config);
        }
        break;
        
    }



    
    rect = { 0, 0, config.windowWidth, config.windowHeight };
    gridLineColor = config.gridColor;
    backgroundColor = config.bgColor;
    N = config.N;

    // Корректируем размеры окна под размеры поля
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    // Проверяем, был ли передан аргумент командной строки (параметр N)
    if (nFromCommandLine) {
        N = nFromCommandLine;
        config.N = N;
    }
    
    

    // Подгоняем размеры окна так, чтобы оно делилось на целые клетки
    WIDTH = (config.windowWidth) / N * N;
    HEIGHT = (config.windowHeight) / N * N;
    SizeCellsX = WIDTH / N;
    SizeCellsY = HEIGHT / N;

    // Регистрация класса окна
    // Инициализация структуры WNDCLASS, которая используется для регистрации класса окна.
    // Эта структура задает параметры, связанные с окнами, такие как иконка, курсор, обработчик сообщений и т.д.
    WNDCLASS SoftwareWindowsClass = { 0 };  // Все поля структуры WNDCLASS инициализируются нулями для безопасности и корректности

    // Загружаем иконку для окна. Здесь используется стандартная иконка "вопрос" из системных ресурсов.
    // Можно заменить на свою иконку.
    SoftwareWindowsClass.hIcon = LoadIcon(NULL, IDI_QUESTION);  // Устанавливаем иконку окна (вопросительный знак)

    // Загружаем стандартный курсор "стрелка", который будет отображаться при наведении мыши на окно.
    // Можно заменить его на любой другой курсор, если необходимо.
    SoftwareWindowsClass.hCursor = LoadCursor(NULL, IDC_ARROW);  // Устанавливаем стандартный курсор (стрелка)

    // Устанавливаем указатель на экземпляр приложения (hInt). Этот параметр нужен для обработки ресурсов и окон в рамках одного приложения.
    SoftwareWindowsClass.hInstance = hInt;  // Присваиваем экземпляр приложения

    // Назначаем уникальное имя классу окна. Это имя будет использоваться при создании окна.
    // Имя должно быть уникально в рамках вашего приложения, чтобы избежать конфликтов.
    SoftwareWindowsClass.lpszClassName = L"MainWinAPIClass";  // Имя класса окна, уникальное для вашего приложения

    // Устанавливаем цвет фона окна. Здесь используется функция CreateSolidBrush для создания кисти с заданным цветом.
    // В данном случае выбран синий цвет (RGB(0, 0, 255)).
    SoftwareWindowsClass.hbrBackground = CreateSolidBrush(backgroundColor);  // Кисть для фона окна (синий)

    // Устанавливаем указатель на функцию, которая будет обрабатывать все сообщения, поступающие в это окно.
    // Эта функция будет обрабатывать такие события, как нажатие клавиш, изменение размеров окна и другие сообщения.
    SoftwareWindowsClass.lpfnWndProc = WindowProc;  // Функция, обрабатывающая сообщения для окон этого класса



    // Регистрация окна
    if (!RegisterClassW(&SoftwareWindowsClass)) {
        return -1;
    }

    // Создание окна с нужными размерами
    CreateWindowW(
        L"MainWinAPICLass",
        L"Firsr C++ WinApi Application",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        100,
        100,
        WIDTH + (GetSystemMetrics(SM_CXFRAME) * 2) + 10, // Ширина с учетом рамок и отступов
        HEIGHT + GetSystemMetrics(SM_CYFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION) + 10,
        NULL,
        NULL,
        NULL,
        NULL
    );

    // Основной цикл обработки сообщений
    MSG SoftwareMsg = { 0 };
    while (GetMessage(&SoftwareMsg, NULL, NULL, NULL)) {
        // Цикл обработки сообщений. Он извлекает сообщения из очереди сообщений и передает их для обработки.

        // Эта функция преобразует сообщения от клавиатуры в другие события, такие как WM_CHAR или другие сообщения,
        // которые более удобно обрабатывать в окне. Она автоматически вызовется, если в сообщении содержится информация о клавишах.
        TranslateMessage(&SoftwareMsg);  // Переводит сообщения от клавиатуры в другие события, например, символы или коды клавиш

        // Эта функция отправляет сообщение для обработки в процедуру обработки сообщений окна (WindowProc).
        // Все сообщения, такие как нажатие клавиш, движения мыши, изменения окна и прочее, будут обработаны в функции WindowProc.
        DispatchMessage(&SoftwareMsg);   // Отправляет сообщение в функцию обработки сообщений для дальнейшей обработки в окне

    }
    return 0;
}

// Функция для обработки сообщений окна
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        // Сохраняем конфигурацию перед закрытием приложения
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
        PostQuitMessage(0); // Завершаем приложение
        return 0;

    case WM_PAINT: {
        // Обработчик отрисовки окна
        PAINTSTRUCT pnt;
        HDC hdc = BeginPaint(hwnd, &pnt);

        // Рисуем сетку
        HPEN hPen = CreatePen(PS_SOLID, 1, gridLineColor); // Создаем перо для линий сетки
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        for (int i = 0; i <= N; i++) {
            // Рисуем вертикальные линии сетки
            MoveToEx(hdc, (SizeCellsX * i), 0, NULL);
            LineTo(hdc, SizeCellsX * i, HEIGHT);

            // Рисуем горизонтальные линии сетки
            MoveToEx(hdc, 0, (SizeCellsY * i), NULL);
            LineTo(hdc, WIDTH, (SizeCellsY * i));
        }
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen); // Освобождаем ресурсы

        // Рисуем сохраненные фигуры (круги и крестики)
        HPEN hShapePen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));  // Белые линии для фигур
        HPEN hOldShapePen = (HPEN)SelectObject(hdc, hShapePen);

        for (const auto& shape : shapes) {
            // Вычисляем координаты для каждой фигуры в сетке
            int left = shape.col * SizeCellsX;
            int top = shape.row * SizeCellsY;
            int right = (shape.col + 1) * SizeCellsX;
            int bottom = (shape.row + 1) * SizeCellsY;

            if (shape.isCircle) {
                Arc(hdc, left, top, right, bottom, 0, 0, 0, 0);  // Рисуем круг
            }
            else {
                // Рисуем крестик
                MoveToEx(hdc, left, top, NULL);
                LineTo(hdc, right, bottom);
                MoveToEx(hdc, right, top, NULL);
                LineTo(hdc, left, bottom);
            }
        }

        SelectObject(hdc, hOldShapePen);
        DeleteObject(hShapePen); // Освобождаем ресурсы

        EndPaint(hwnd, &pnt); // Завершаем отрисовку
        return 0;
    }

    case WM_SIZE:
    {
        // Обработчик изменения размеров окна
        WIDTH = LOWORD(lParam);  // Новая ширина окна
        HEIGHT = HIWORD(lParam); // Новая высота окна
        config.windowWidth = WIDTH;
        config.windowHeight = HEIGHT;
        SizeCellsX = WIDTH / N;  // Новый размер клетки
        SizeCellsY = HEIGHT / N;
        InvalidateRect(hwnd, NULL, TRUE); // Перерисовываем окно
        return 0;
    }

    case WM_LBUTTONDOWN: { // ЛКМ - круг
        // Обработчик нажатия левой кнопки мыши
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        if (x >= SizeCellsX * N || y >= SizeCellsY * N) return 0;  // Если кликаем за пределами поля

        int col = x / SizeCellsX;  // Определяем колонку
        int row = y / SizeCellsY;  // Определяем строку
        for (const Shape& shape : shapes) {
            if (shape.col == col && shape.row == row) {
                return 0; // Если фигура уже существует в этой ячейке
            }
        }
        shapes.push_back({ col, row, true });  // Добавляем круг
        
        InvalidateRect(hwnd, NULL, FALSE);  // Перерисовываем окно
        return 0;
    }

    case WM_RBUTTONDOWN: { // ПКМ - крестик
        // Обработчик нажатия правой кнопки мыши
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        if (x >= SizeCellsX * N || y >= SizeCellsY * N) return 0;  // Если кликаем за пределами поля

        int col = x / SizeCellsX;
        int row = y / SizeCellsY;
        for (const Shape& shape : shapes) {
            if (shape.col == col && shape.row == row) {
                return 0; // Если фигура уже существует
            }
        }
        shapes.push_back({ col, row, false });  // Добавляем крестик

        InvalidateRect(hwnd, NULL, FALSE);  // Перерисовываем окно
        return 0;
    }

    case WM_KEYDOWN: {
        // Обработчик нажатий клавиш
        if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && wParam == 'Q') {
            // Закрываем окно по комбинации Ctrl+Q
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
            // Закрываем окно по клавише ESC
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
            // Запуск Блокнота по комбинации Shift+C
            ShellExecute(NULL, L"open", L"notepad.exe", NULL, NULL, SW_SHOWNORMAL);
        }

        if (wParam == VK_RETURN) {
            // Изменяем цвет фона на случайный при нажатии Enter
            ChangeBackgroundColor(hwnd);
        }
        return 0;
    }

    case WM_MOUSEWHEEL: {
        // Плавное изменение цвета линий сетки при прокрутке колеса мыши
        int wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        if (wheelDelta > 0) {  // Прокрутка вверх
            red = (red + colorStep) % 256;  // Увеличиваем красный компонент
        }
        else {  // Прокрутка вниз
            red = (red - colorStep + 256) % 256;  // Уменьшаем красный компонент
        }
        gridLineColor = RGB(red, blue, green);
        config.gridColor = gridLineColor;
        InvalidateRect(hwnd, NULL, TRUE);  // Перерисовываем окно

        return 0;
    }

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);  // Обработка остальных сообщений
    }
}

// Функция для изменения цвета фона
void ChangeBackgroundColor(HWND hwnd) {
    // Генерация случайного цвета для фона
    backgroundColor = RGB(rand() % 256, rand() % 256, rand() % 256);
    config.bgColor = backgroundColor;
    // Устанавливаем новый цвет фона
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(backgroundColor));
    InvalidateRect(hwnd, NULL, TRUE);  // Перерисовываем окно
}

