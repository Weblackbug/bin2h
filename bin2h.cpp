//////////////////////////////////////////////////////////////////////////////////////////////////
// bin2h.cpp : 1.0 17/06/2025
// ---------------------------------------------------------------------------------------------
// By Sergi Serrano weblackbug.gmail.com
// canalinformatika.es   sergi@canalinformatika.es
// Invita una café, son 0,000021 BTC ( 2 EUROS ) A... ( bc1q9320qqdkevs2yxkpaf9a05apvql3w29qp8pahl )
//
// ----------------------------------------------------------------------------------------------
// Este es un programa de ejemplo que convierte archivos binarios a cabeceras C++ (.h) y viceversa
//////////////////////////////////////////////////////////////////////////////////////////////////
#include <Windows.h>
#include "framework.h"

#include <vector>
#include <fstream>
#include <string>
#include <commdlg.h>
#include <sstream>
#include <iomanip>
#include <mmsystem.h>

#include <gdiplus.h>

#include <math.h>
#include "resource.h"
extern "C" {
#include "titchysid.h"
}
#include "bin2h.h"
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

Gdiplus::Bitmap* g_rotozoomImage = nullptr;
float g_rotozoomAngle = 0.0f;
float g_rotozoomZoom = 1.0f;
UINT_PTR g_rotozoomTimer = 0;
#define MAX_LOADSTRING 100

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

float g_cubeAngleX = 0.0f;
float g_cubeAngleY = 0.0f;
UINT_PTR g_cubeTimer = 0;
float g_textAnimPhase = 0.0f;
props sid_props;
char g_paused = 0;
bool sid_sonando = false;

void OnArchivoConvertir(HWND hWnd);
void OnArchivoCargarBinario(HWND hWnd);
void OnAbout(HWND hWnd);

void DrawCustomButton(Graphics& g, int x, int y, int width, int height)
{

    LinearGradientBrush bgBrush(
        Point(x, y), Point(x, y + height),
        Color(255, 220, 220, 220), // Gris claro arriba
        Color(255, 180, 180, 180)  // Gris más oscuro abajo
    );

    // Rectángulo redondeado
    const int radius = 24;
    GraphicsPath path;
    path.AddArc(x, y, radius, radius, 180, 90);
    path.AddArc(x + width - radius, y, radius, radius, 270, 90);
    path.AddArc(x + width - radius, y + height - radius, radius, radius, 0, 90);
    path.AddArc(x, y + height - radius, radius, radius, 90, 90);
    path.CloseFigure();

    // Relleno
    g.FillPath(&bgBrush, &path);

    // Borde blanco
    Pen borderPen(Color(255, 255, 255, 255), 3.0f);
    g.DrawPath(&borderPen, &path);

    // Texto "weBlackbug" en azul marino, centrado
    FontFamily fontFamily(L"Segoe UI");
    Font font(&fontFamily, 22, FontStyleBold, UnitPixel);
    SolidBrush textBrush(Color(255, 0, 32, 96)); // Azul marino
    StringFormat format;
    format.SetAlignment(StringAlignmentCenter);
    format.SetLineAlignment(StringAlignmentCenter);

    RectF textRect((REAL)x, (REAL)y, (REAL)width, (REAL)height);
    g.DrawString(L"weBlackbug", -1, &font, textRect, &format, &textBrush);
}

void DrawRotatingCube(Graphics& g, int cx, int cy, int size, float angleX, float angleY)
{
    struct Vec3 { float x, y, z; };
    Vec3 verts[8] = {
        {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1},
        {-1,-1, 1}, {1,-1, 1}, {1,1, 1}, {-1,1, 1}
    };
    auto rot = [](const Vec3& v, float ax, float ay) -> Vec3 {
        float cosa = cosf(ax), sina = sinf(ax);
        float y1 = v.y * cosa - v.z * sina;
        float z1 = v.y * sina + v.z * cosa;
        float cosb = cosf(ay), sinb = sinf(ay);
        float x2 = v.x * cosb + z1 * sinb;
        float z2 = -v.x * sinb + z1 * cosb;
        return { x2, y1, z2 };
        };
    auto proj = [cx, cy, size](const Vec3& v) -> PointF {
        float fov = 3.0f;
        float factor = size / (fov - v.z);
        return PointF(cx + v.x * factor, cy + v.y * factor);
        };
    PointF points2D[8];
    for (int i = 0; i < 8; ++i)
        points2D[i] = proj(rot(verts[i], angleX, angleY));
    int edges[12][2] = {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };
    Pen pen(Color(255, 0, 128, 255), 2.0f);
    for (int i = 0; i < 12; ++i)
        g.DrawLine(&pen, points2D[edges[i][0]], points2D[edges[i][1]]);
}

bool AbreArchivoBinario(const std::wstring& rutaArchivo, std::vector<unsigned char>& buffer, std::wstring& errorMsg)
{
    std::ifstream archivo(rutaArchivo, std::ios::binary | std::ios::ate);
    if (!archivo) {
        errorMsg = L"No se pudo abrir el archivo.";
        return false;
    }
    std::streamsize tam = archivo.tellg();
    if (tam < 0) {
        errorMsg = L"No se pudo determinar el tamaño del archivo.";
        return false;
    }
    archivo.seekg(0, std::ios::beg);
    buffer.resize(static_cast<size_t>(tam));
    if (!archivo.read(reinterpret_cast<char*>(buffer.data()), tam)) {
        errorMsg = L"Error al leer el archivo.";
        return false;
    }
    return true;
}

bool ConvertirHeaderABinario(const std::wstring& headerPath, const std::wstring& binPath, std::wstring& errorMsg)
{
    std::ifstream headerFile(headerPath);
    if (!headerFile) {
        errorMsg = L"No se pudo abrir el archivo de cabecera.";
        return false;
    }
    std::vector<unsigned char> buffer;
    std::string line;
    bool dentroArray = false;
    while (std::getline(headerFile, line)) {
        if (!dentroArray) {
            auto pos = line.find('{');
            if (pos != std::string::npos) {
                dentroArray = true;
                line = line.substr(pos + 1);
            }
            else {
                continue;
            }
        }
        auto fin = line.find('}');
        if (fin != std::string::npos) {
            line = line.substr(0, fin);
            dentroArray = false;
        }
        std::istringstream iss(line);
        std::string token;
        while (std::getline(iss, token, ',')) {
            size_t start = token.find_first_not_of(" \t\n\r");
            if (start == std::string::npos) continue;
            token = token.substr(start);
            if (token.size() >= 3 && token[0] == '0' && (token[1] == 'x' || token[1] == 'X')) {
                unsigned int byte;
                std::istringstream hexstr(token);
                hexstr >> std::hex >> byte;
                buffer.push_back(static_cast<unsigned char>(byte));
            }
        }
        if (!dentroArray) break;
    }
    headerFile.close();
    if (buffer.empty()) {
        errorMsg = L"No se encontraron datos válidos en el archivo de cabecera.";
        return false;
    }
    std::ofstream binFile(binPath, std::ios::binary);
    if (!binFile) {
        errorMsg = L"No se pudo crear el archivo binario.";
        return false;
    }
    binFile.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    binFile.close();
    return true;
}

bool GuardarComoCabeceraCpp(const std::vector<unsigned char>& buffer, const std::string& variableName, const std::wstring& headerPath, std::wstring& errorMsg)
{
    std::ofstream headerFile(headerPath, std::ios::out | std::ios::trunc);
    if (!headerFile) {
        errorMsg = L"No se pudo crear el archivo de cabecera.";
        return false;
    }
    headerFile << "#pragma once\n";
    headerFile << "// Archivo generado automáticamente con Bin2h By Sergi WeBlackbug 2025\n";
    headerFile << "// Si te ha invitado esta aplicacion a ser Feliz con tu Programa ?..\n\n";
    headerFile << "// Envia una propina para tomarme un Café a esta direccion de mi cartera BitCoin en USDC\n\n";
    headerFile << "// DIRECCION BITCOIN bc1q9320qqdkevs2yxkpaf9a05apvql3w29qp8pahl\n\n";
    headerFile << "#include <cstddef>\n\n";
    headerFile << "const unsigned char " << variableName << "[] = {\n    ";
    size_t count = 0;
    for (size_t i = 0; i < buffer.size(); ++i) {
        headerFile << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)buffer[i];
        if (i != buffer.size() - 1)
            headerFile << ", ";
        count++;
        if (count == 16 && i != buffer.size() - 1) {
            headerFile << "\n    ";
            count = 0;
        }
    }
    headerFile << "\n};\n";
    headerFile << "const size_t " << variableName << "_size = sizeof(" << variableName << ");\n";
    headerFile.close();
    return true;
}

std::string GetFileNameNoExt(const std::wstring& path)
{
    size_t slash = path.find_last_of(L"\\/");
    size_t dot = path.find_last_of(L'.');
    size_t start = (slash == std::wstring::npos) ? 0 : slash + 1;
    size_t end = (dot == std::wstring::npos || dot < start) ? path.length() : dot;
    std::wstring name = path.substr(start, end - start);
    std::string result(name.begin(), name.end());
    for (auto& c : result) {
        if (!isalnum(static_cast<unsigned char>(c)))
            c = '_';
    }
    return result;
}

void DrawTextWithOutlineAndShadow(
    Graphics& gBmp,
    HWND hwnd,
    const std::wstring& text,
    int x, int y,
    COLORREF textColor,
    COLORREF outlineColor,
    COLORREF shadowColor,
    int outlineWidth = 2,
    int shadowOffset = 4,
    int fontSize = 32,
    const wchar_t* fontName = L"Segoe UI",
    const wchar_t* align = L"left"
) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;
    gBmp.SetSmoothingMode(SmoothingModeAntiAlias);
    gBmp.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
    FontFamily fontFamily(fontName);
    Font font(&fontFamily, (REAL)fontSize, FontStyleBold, UnitPixel);
    RectF layoutRect(0, 0, (REAL)width, 1000.0f);
    RectF textRect;
    StringFormat format;
    format.SetLineAlignment(StringAlignmentNear);
    format.SetAlignment(StringAlignmentNear);
    gBmp.MeasureString(text.c_str(), (INT)text.length(), &font, layoutRect, &format, &textRect);
    float drawX = (float)x;
    if (wcscmp(align, L"center") == 0) {
        drawX = x - textRect.Width / 2.0f;
    }
    else if (wcscmp(align, L"right") == 0) {
        drawX = x - textRect.Width;
    }
    GraphicsPath shadowPath;
    shadowPath.AddString(
        text.c_str(), (INT)text.length(), &fontFamily, FontStyleBold, (REAL)fontSize,
        PointF(drawX, (REAL)y), &format
    );
    for (int blur = shadowOffset + 2; blur >= 1; --blur) {
        BYTE alpha = (BYTE)(40.0 * (1.0 - (float)(blur - 1) / (shadowOffset + 1)));
        SolidBrush blurBrush(Color(alpha, GetRValue(shadowColor), GetGValue(shadowColor), GetBValue(shadowColor)));
        for (int dx = -blur; dx <= blur; ++dx) {
            for (int dy = -blur; dy <= blur; ++dy) {
                if (dx * dx + dy * dy > blur * blur) continue;
                if (dx == 0 && dy == 0) continue;
                Matrix m;
                m.Translate((REAL)dx, (REAL)dy);
                gBmp.SetTransform(&m);
                gBmp.FillPath(&blurBrush, &shadowPath);
            }
        }
    }
    gBmp.ResetTransform();
    GraphicsPath outlinePath;
    outlinePath.AddString(
        text.c_str(), (INT)text.length(), &fontFamily, FontStyleBold, (REAL)fontSize,
        PointF(drawX, (REAL)y), &format
    );
    Pen outlinePen(Color(255, GetRValue(outlineColor), GetGValue(outlineColor), GetBValue(outlineColor)), (REAL)outlineWidth);
    outlinePen.SetLineJoin(LineJoinRound);
    gBmp.DrawPath(&outlinePen, &outlinePath);
    SolidBrush textBrush(Color(255, GetRValue(textColor), GetGValue(textColor), GetBValue(textColor)));
    gBmp.FillPath(&textBrush, &outlinePath);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_BIN2H, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    if (!InitInstance(hInstance, nCmdShow)) {
        return FALSE;
    }
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_BIN2H));
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    GdiplusShutdown(gdiplusToken);
    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_BIN2H);
    wcex.lpszClassName = szWindowClass;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON2));
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON2));
    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;
    int winWidth = 600;
    int winHeight = 300;
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenWidth - winWidth) / 2;
    int y = (screenHeight - winHeight) / 2;
    HWND hWnd = CreateWindowW(
        szWindowClass, szTitle,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        x, y, winWidth, winHeight,
        nullptr, nullptr, hInstance, nullptr);
    if (!hWnd) {
        return FALSE;
    }
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    g_cubeTimer = SetTimer(hWnd, 2, 16, NULL);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_ABOUT:
            OnAbout(hWnd);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case ID_ARCHIVO_CONVERTIR:
            OnArchivoConvertir(hWnd);
            break;
        case ID_ARCHIVO_CARGARBINARIO:
            OnArchivoCargarBinario(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }
    case WM_TIMER:
        if (wParam == 2) {
            g_cubeAngleX += 0.03f;
            g_cubeAngleY += 0.025f;
            g_textAnimPhase += 0.07f;
            if (g_textAnimPhase > 6.2831853f) g_textAnimPhase -= 6.2831853f;
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;

    case WM_ERASEBKGND:
    {
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(hWnd, &rc);
        Graphics g(hdc);
        Color colorTop(255, 0, 32, 128);
        Color colorBottom(255, 128, 192, 255);
        LinearGradientBrush brush(
            Point(0, 0),
            Point(0, rc.bottom - rc.top),
            colorTop,
            colorBottom
        );
        g.FillRectangle(&brush, 0, 0, rc.right - rc.left, rc.bottom - rc.top);
        return 1;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT rc;
        GetClientRect(hWnd, &rc);
        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;
        Bitmap bmp(width, height, PixelFormat32bppARGB);
        Graphics gMem(&bmp);
        gMem.SetSmoothingMode(SmoothingModeAntiAlias);
        gMem.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
        Color colorTop(255, 0, 32, 128);
        Color colorBottom(255, 128, 192, 255);
        LinearGradientBrush brush(
            Point(0, 0),
            Point(0, height),
            colorTop,
            colorBottom
        );
        gMem.FillRectangle(&brush, 0, 0, width, height);
        int cx = width / 2;
        int cy = height / 2 + 20;
        int size = min(width, height) / 3;
        DrawRotatingCube(gMem, cx, cy, size, g_cubeAngleX, g_cubeAngleY);
        std::wstring texto = L"Bin2H 1.0 By WeBlackbug";
        int x = 290;
        int y = 50;
        int fontSize = 32 + (int)(10.0f * sinf(g_textAnimPhase));
        DrawTextWithOutlineAndShadow(
            gMem, hWnd, texto, x, y,
            RGB(255, 255, 255),
            RGB(0, 0, 0),
            RGB(0, 32, 128),
            1, 1, fontSize,
            L"Times New Roman",
            L"center"
        );
        Graphics gWin(hdc);
        gWin.DrawImage(&bmp, 0, 0);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        if (g_cubeTimer) {
            KillTimer(hWnd, g_cubeTimer);
            g_cubeTimer = 0;
        }
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
        RECT rcDlg;
        GetWindowRect(hDlg, &rcDlg);
        int dlgWidth = rcDlg.right - rcDlg.left;
        int dlgHeight = rcDlg.bottom - rcDlg.top;
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        int x = (screenWidth - dlgWidth) / 2;
        int y = (screenHeight - dlgHeight) / 2;
        SetWindowPos(hDlg, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        SetDlgItemTextW(hDlg, IDC_STATIC12, L"demo1.0");
        HFONT hFont = CreateFontW(
            80, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
            L"Segoe UI"
        );
        SendDlgItemMessageW(hDlg, IDC_STATIC12, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND hStatic16 = GetDlgItem(hDlg, IDC_STATIC16);
        if (hStatic16) {
            // Cambia la fuente
            HFONT hFont16 = CreateFontW(
                25, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                Fuentes::CalistoMT
            );
            SendMessageW(hStatic16, WM_SETFONT, (WPARAM)hFont16, TRUE);

            // Centra el texto si el recurso no tiene SS_CENTER
            LONG style = GetWindowLong(hStatic16, GWL_STYLE);
            if ((style & SS_CENTER) == 0) {
                SetWindowLong(hStatic16, GWL_STYLE, style | SS_CENTER);
                InvalidateRect(hStatic16, NULL, TRUE);
            }
        }
	    SIDOpen (IDR_MUSIC, 0, SID_RESOURCE, SID_DEFAULT, 0);
		SIDGetProps (&sid_props);
		SIDPlay ();
		sid_sonando = true;

        HRSRC hRes = FindResource(hInst, MAKEINTRESOURCE(IDB_WBLG), RT_RCDATA);
        if (hRes) {
            HGLOBAL hData = LoadResource(hInst, hRes);
            if (hData) {
                void* pData = LockResource(hData);
                DWORD size = SizeofResource(hInst, hRes);
                if (pData && size > 0) {
                    HGLOBAL hBuffer = GlobalAlloc(GMEM_MOVEABLE, size);
                    if (hBuffer) {
                        void* pBuffer = GlobalLock(hBuffer);
                        memcpy(pBuffer, pData, size);
                        GlobalUnlock(hBuffer);
                        IStream* pStream = nullptr;
                        if (CreateStreamOnHGlobal(hBuffer, TRUE, &pStream) == S_OK) {
                            if (g_rotozoomImage) delete g_rotozoomImage;
                            g_rotozoomImage = Gdiplus::Bitmap::FromStream(pStream);
                            pStream->Release();
                        }
                    }
                }
            }
        }
        g_rotozoomAngle = 0.0f;
        g_rotozoomZoom = 1.0f;
        g_rotozoomTimer = SetTimer(hDlg, 1, 33, NULL);
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    case WM_TIMER:
        if (wParam == 1) {
            g_rotozoomAngle += 1.0f;
            if (g_rotozoomAngle >= 360.0f) g_rotozoomAngle -= 360.0f;
            g_rotozoomZoom = 1.0f + 0.5f * sin(GetTickCount() * 0.002f);
            InvalidateRect(hDlg, NULL, FALSE);
        }
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hDlg, &ps);
        HWND hPic = GetDlgItem(hDlg, IDC_STATIC1);
        RECT rcPic;
        Graphics g(hdc);
        GetWindowRect(hPic, &rcPic);
        MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)&rcPic, 2);
        int width = rcPic.right - rcPic.left;
        int height = rcPic.bottom - rcPic.top;
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbmMem = CreateCompatibleBitmap(hdc, width, height);
        HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
        RECT rcFill = { 0, 0, width, height };
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(hdcMem, &rcFill, hBrush);
        DeleteObject(hBrush);
        if (g_rotozoomImage) {
            Bitmap bmpOut(width, height, PixelFormat32bppARGB);
            BitmapData dataOut;
            Rect rectOut(0, 0, width, height);
            if (bmpOut.LockBits(&rectOut, ImageLockModeWrite, PixelFormat32bppARGB, &dataOut) == Ok) {
                int srcW = g_rotozoomImage->GetWidth();
                int srcH = g_rotozoomImage->GetHeight();
                float zoom = g_rotozoomZoom * 0.4f;
                if (zoom < 0.2f) zoom = 0.2f;
                float angle = g_rotozoomAngle * 3.14159265f / 180.0f;
                float cosA = cosf(angle) / zoom;
                float sinA = sinf(angle) / zoom;
                float cx = width / 2.0f;
                float cy = height / 2.0f;
                for (int y = 0; y < height; ++y) {
                    uint32_t* pLine = (uint32_t*)((BYTE*)dataOut.Scan0 + y * dataOut.Stride);
                    for (int x = 0; x < width; ++x) {
                        float dx = x - cx;
                        float dy = y - cy;
                        float u = cosA * dx + sinA * dy;
                        float v = -sinA * dx + cosA * dy;
                        int srcX = ((int)(u + srcW / 2.0f)) % srcW;
                        int srcY = ((int)(v + srcH / 2.0f)) % srcH;
                        if (srcX < 0) srcX += srcW;
                        if (srcY < 0) srcY += srcH;
                        Color color;
                        g_rotozoomImage->GetPixel(srcX, srcY, &color);
                        pLine[x] = color.GetValue();
                    }
                }
                bmpOut.UnlockBits(&dataOut);
                Graphics gBmp(hdcMem);
                gBmp.DrawImage(&bmpOut, 0, 0, width, height);
            }
        }
        BitBlt(hdc, rcPic.left, rcPic.top, width, height, hdcMem, 0, 0, SRCCOPY);
        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);
        EndPaint(hDlg, &ps);
    }
    break;
    case WM_DESTROY:
        if (g_rotozoomTimer) {
            KillTimer(hDlg, g_rotozoomTimer);
            g_rotozoomTimer = 0;
        }
        if (g_rotozoomImage) {
            delete g_rotozoomImage;
            g_rotozoomImage = nullptr;
        }
        if (sid_sonando) {
            SIDStop();
            sid_sonando = 0;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void OnArchivoConvertir(HWND hWnd) {
    OPENFILENAME ofnOpen;
    WCHAR szHeader[260] = L"";
    ZeroMemory(&ofnOpen, sizeof(ofnOpen));
    ofnOpen.lStructSize = sizeof(ofnOpen);
    ofnOpen.hwndOwner = hWnd;
    ofnOpen.lpstrFile = szHeader;
    ofnOpen.lpstrFile[0] = '\0';
    ofnOpen.nMaxFile = sizeof(szHeader) / sizeof(szHeader[0]);
    ofnOpen.lpstrFilter = L"Cabeceras C++ (*.h)\0*.h\0Todos los archivos (*.*)\0*.*\0";
    ofnOpen.nFilterIndex = 1;
    ofnOpen.lpstrFileTitle = NULL;
    ofnOpen.nMaxFileTitle = 0;
    ofnOpen.lpstrInitialDir = NULL;
    ofnOpen.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if (GetOpenFileName(&ofnOpen)) {
        OPENFILENAME ofnSave;
        WCHAR szBin[260] = L"";
        ZeroMemory(&ofnSave, sizeof(ofnSave));
        ofnSave.lStructSize = sizeof(ofnSave);
        ofnSave.hwndOwner = hWnd;
        ofnSave.lpstrFile = szBin;
        ofnSave.lpstrFile[0] = '\0';
        ofnSave.nMaxFile = sizeof(szBin) / sizeof(szBin[0]);
        ofnSave.lpstrFilter = L"Archivos binarios (*.bin)\0*.bin\0Todos los archivos (*.*)\0*.*\0";
        ofnSave.nFilterIndex = 1;
        ofnSave.lpstrFileTitle = NULL;
        ofnSave.nMaxFileTitle = 0;
        ofnSave.lpstrInitialDir = NULL;
        ofnSave.Flags = OFN_OVERWRITEPROMPT;
        if (GetSaveFileName(&ofnSave)) {
            std::wstring errorMsg;
            if (ConvertirHeaderABinario(szHeader, szBin, errorMsg)) {
                MessageBox(hWnd, L"Binario generado correctamente.", L"Éxito", MB_OK);
            }
            else {
                MessageBox(hWnd, errorMsg.c_str(), L"Error", MB_ICONERROR);
            }
        }
    }
}

void OnArchivoCargarBinario(HWND hWnd) {
    OPENFILENAME ofn;
    WCHAR szFile[260] = L"";
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
    ofn.lpstrFilter = L"Archivos binarios (*.bin)\0*.bin\0Todos los archivos (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if (GetOpenFileName(&ofn)) {
        std::vector<unsigned char> buffer;
        std::wstring errorMsg;
        if (AbreArchivoBinario(ofn.lpstrFile, buffer, errorMsg)) {
            OPENFILENAME ofnSave;
            WCHAR szHeader[260] = L"";
            ZeroMemory(&ofnSave, sizeof(ofnSave));
            ofnSave.lStructSize = sizeof(ofnSave);
            ofnSave.hwndOwner = hWnd;
            ofnSave.lpstrFile = szHeader;
            ofnSave.lpstrFile[0] = '\0';
            ofnSave.nMaxFile = sizeof(szHeader) / sizeof(szHeader[0]);
            ofnSave.lpstrFilter = L"Cabeceras C++ (*.h)\0*.h\0Todos los archivos (*.*)\0*.*\0";
            ofnSave.nFilterIndex = 1;
            ofnSave.lpstrFileTitle = NULL;
            ofnSave.nMaxFileTitle = 0;
            ofnSave.lpstrInitialDir = NULL;
            ofnSave.Flags = OFN_OVERWRITEPROMPT;
            if (GetSaveFileName(&ofnSave)) {
                std::string variableName = GetFileNameNoExt(ofn.lpstrFile);
                if (variableName.empty()) variableName = "bin_data";
                std::wstring errorMsgHeader;
                if (GuardarComoCabeceraCpp(buffer, variableName, ofnSave.lpstrFile, errorMsgHeader)) {
                    MessageBox(hWnd, L"Cabecera generada correctamente.", L"Éxito", MB_OK);
                }
                else {
                    MessageBox(hWnd, errorMsgHeader.c_str(), L"Error", MB_ICONERROR);
                }
            }
        }
        else {
            MessageBox(hWnd, errorMsg.c_str(), L"Error", MB_ICONERROR);
        }
    }
}

void OnAbout(HWND hWnd) {
    DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
}

