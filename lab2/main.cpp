#include <Windows.h>  // ��������� ��� ������ � WinAPI
#include <windowsx.h> // ��� ������� �������� ������ � ������������ � �����������
#include <vector>      // ��� ������������� ������������ ��������
#include <fstream>     // ��� ������ � ��������� �������� (����������/������ ������������)
#include <sstream>     // ��� �������� ����� (��������, ���������������� ������)


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

const std::string configFileName = "config.txt";  // ��� ����� ��� ������������
Config config;  // ���������� ��� �������� ������������

std::vector<Shape> shapes; // ������ ��� �������� ����� (����� � ��������)

// ���������� ��������� ����
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// ������� ��� ������ � ������������� (������ � ����������)
void ChangeBackgroundColor(HWND hwnd);
bool SaveConfig(const std::string& filename, const Config& config);
bool LoadConfig(const std::string& filename, Config& config);

// �������� �������, ������� �������������� � ��������� ����������
int WINAPI wWinMain(HINSTANCE hInt, HINSTANCE hPreve, PWSTR pCom, int nCmdShow)
{
    // ��������� ������������ �� �����
    if (!LoadConfig(configFileName, config)) {
        // ���� ���� �� ����������, ������� ��� � ����������� �� ���������
        SaveConfig(configFileName, config);
    }
    rect = { 0, 0, config.windowHeight, config.windowHeight };
    gridLineColor = config.gridColor;
    backgroundColor = config.bgColor;
    N = config.N;
    // ������������ ������� ���� ��� ������� ����
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    // ���������, ��� �� ������� �������� ��������� ������ (�������� N)
    if (pCom && *pCom) {
        int userN = _wtoi(pCom); // ����������� ������ � ����� �����
        if (userN >= 3 && userN <= 20) { // ������������ ���������� �������� N (�� 3 �� 20)
            N = userN;
        }
    }
    config.N = N;

    // ��������� ������� ���� ���, ����� ��� �������� �� ����� ������
    WIDTH = (config.windowWidth) / N * N;
    HEIGHT = (config.windowHeight) / N * N;
    SizeCellsX = WIDTH / N;
    SizeCellsY = HEIGHT / N;

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
        L"MainWinAPICLass",
        L"Firsr C++ WinApi Application",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        100,
        100,
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

// ������� ��� ��������� ��������� ����
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        // ��������� ������������ ����� ��������� ����������
        SaveConfig(configFileName, config);
        PostQuitMessage(0); // ��������� ����������
        return 0;

    case WM_PAINT: {
        // ���������� ��������� ����
        PAINTSTRUCT pnt;
        HDC hdc = BeginPaint(hwnd, &pnt);

        // ������ �����
        HPEN hPen = CreatePen(PS_SOLID, 1, gridLineColor); // ������� ���� ��� ����� �����
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        for (int i = 0; i <= N; i++) {
            // ������ ������������ ����� �����
            MoveToEx(hdc, (SizeCellsX * i), 0, NULL);
            LineTo(hdc, SizeCellsX * i, HEIGHT);

            // ������ �������������� ����� �����
            MoveToEx(hdc, 0, (SizeCellsY * i), NULL);
            LineTo(hdc, WIDTH, (SizeCellsY * i));
        }
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen); // ����������� �������

        // ������ ����������� ������ (����� � ��������)
        HPEN hShapePen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));  // ����� ����� ��� �����
        HPEN hOldShapePen = (HPEN)SelectObject(hdc, hShapePen);

        for (const auto& shape : shapes) {
            // ��������� ���������� ��� ������ ������ � �����
            int left = shape.col * SizeCellsX;
            int top = shape.row * SizeCellsY;
            int right = (shape.col + 1) * SizeCellsX;
            int bottom = (shape.row + 1) * SizeCellsY;

            if (shape.isCircle) {
                Arc(hdc, left, top, right, bottom, 0, 0, 0, 0);  // ������ ����
            }
            else {
                // ������ �������
                MoveToEx(hdc, left, top, NULL);
                LineTo(hdc, right, bottom);
                MoveToEx(hdc, right, top, NULL);
                LineTo(hdc, left, bottom);
            }
        }

        SelectObject(hdc, hOldShapePen);
        DeleteObject(hShapePen); // ����������� �������

        EndPaint(hwnd, &pnt); // ��������� ���������
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
        InvalidateRect(hwnd, NULL, TRUE); // �������������� ����
        return 0;
    }

    case WM_LBUTTONDOWN: { // ��� - ����
        // ���������� ������� ����� ������ ����
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        if (x >= SizeCellsX * N || y >= SizeCellsY * N) return 0;  // ���� ������� �� ��������� ����

        int col = x / SizeCellsX;  // ���������� �������
        int row = y / SizeCellsY;  // ���������� ������
        for (const Shape& shape : shapes) {
            if (shape.col == col && shape.row == row) {
                return 0; // ���� ������ ��� ���������� � ���� ������
            }
        }
        shapes.push_back({ col, row, true });  // ��������� ����
        
        InvalidateRect(hwnd, NULL, FALSE);  // �������������� ����
        return 0;
    }

    case WM_RBUTTONDOWN: { // ��� - �������
        // ���������� ������� ������ ������ ����
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        if (x >= SizeCellsX * N || y >= SizeCellsY * N) return 0;  // ���� ������� �� ��������� ����

        int col = x / SizeCellsX;
        int row = y / SizeCellsY;
        for (const Shape& shape : shapes) {
            if (shape.col == col && shape.row == row) {
                return 0; // ���� ������ ��� ����������
            }
        }
        shapes.push_back({ col, row, false });  // ��������� �������

        InvalidateRect(hwnd, NULL, FALSE);  // �������������� ����
        return 0;
    }

    case WM_KEYDOWN: {
        // ���������� ������� ������
        if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && wParam == 'Q') {
            // ��������� ���� �� ���������� Ctrl+Q
            SaveConfig(configFileName, config);
            PostMessage(hwnd, WM_CLOSE, 0, 0);
        }
        if (wParam == VK_ESCAPE) {
            // ��������� ���� �� ������� ESC
            SaveConfig(configFileName, config);
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
    InvalidateRect(hwnd, NULL, TRUE);  // �������������� ����
}

// ������� ��� �������� ������������ �� �����
// ������ ������������ �� �����
bool LoadConfig(const std::string& filename, Config& config) {
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


// ������� ��� ���������� ������������ � ����
bool SaveConfig(const std::string& filename, const Config& config) {
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
