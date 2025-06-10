// bin2h.cpp : 1.0
// ---------------------------------------------------------------------------------------------
// By Sergi Serrano weblackbug.gmail.com
// canalinformatika.es   sergi@canalinformatika.es
// Invita una café, son 0,000021 BTC ( 2 EUROS ) A... ( bc1q9320qqdkevs2yxkpaf9a05apvql3w29qp8pahl )
//
// ----------------------------------------------------------------------------------------------
// Este es un programa de ejemplo que convierte archivos binarios a cabeceras C++ (.h) y viceversa
//


#include <Windows.h>
#include "framework.h"
#include "bin2h.h"
#include <vector>
#include <fstream>
#include <string>
#include <commdlg.h>
#include <sstream>
#include <iomanip>
#include "TextShadow.h"
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;
Gdiplus::Bitmap* g_rotozoomImage = nullptr;
float g_rotozoomAngle = 0.0f;
float g_rotozoomZoom = 1.0f;
UINT_PTR g_rotozoomTimer = 0;
#define MAX_LOADSTRING 100

// Variables globales:
HINSTANCE hInst;                                // instancia actual
WCHAR szTitle[MAX_LOADSTRING];                  // Texto de la barra de título
WCHAR szWindowClass[MAX_LOADSTRING];            // nombre de clase de la ventana principal

// Declaraciones de funciones adelantadas incluidas en este módulo de código:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void OnArchivoConvertir(HWND hWnd);
void OnArchivoCargarBinario(HWND hWnd);
void OnAbout(HWND hWnd);
TextControl* g_aboutTextControl = nullptr;

// Función para cargar un archivo binario en un buffer
bool AbreArchivoBinario(const std::wstring& rutaArchivo, std::vector<unsigned char>& buffer, std::wstring& errorMsg)
{
    std::ifstream archivo(rutaArchivo, std::ios::binary | std::ios::ate);
    if (!archivo)
    {
        errorMsg = L"No se pudo abrir el archivo.";
        return false;
    }

    std::streamsize tam = archivo.tellg();
    if (tam < 0)
    {
        errorMsg = L"No se pudo determinar el tamaño del archivo.";
        return false;
    }
    archivo.seekg(0, std::ios::beg);

    buffer.resize(static_cast<size_t>(tam));
    if (!archivo.read(reinterpret_cast<char*>(buffer.data()), tam))
    {
        errorMsg = L"Error al leer el archivo.";
        return false;
    }

    return true;
}

// Convierte un archivo de cabecera C++ generado por GuardarComoCabeceraCpp a binario
bool ConvertirHeaderABinario(const std::wstring& headerPath, const std::wstring& binPath, std::wstring& errorMsg)
{
    std::ifstream headerFile(headerPath);
    if (!headerFile)
    {
        errorMsg = L"No se pudo abrir el archivo de cabecera.";
        return false;
    }

    std::vector<unsigned char> buffer;
    std::string line;
    bool dentroArray = false;
    while (std::getline(headerFile, line))
    {
        // Buscar el inicio del array
        if (!dentroArray)
        {
            auto pos = line.find('{');
            if (pos != std::string::npos)
            {
                dentroArray = true;
                line = line.substr(pos + 1);
            }
            else
            {
                continue;
            }
        }

        // Buscar el final del array
        auto fin = line.find('}');
        if (fin != std::string::npos)
        {
            line = line.substr(0, fin);
            dentroArray = false;
        }

        std::istringstream iss(line);
        std::string token;
        while (std::getline(iss, token, ','))
        {
            // Eliminar espacios
            size_t start = token.find_first_not_of(" \t\n\r");
            if (start == std::string::npos) continue;
            token = token.substr(start);

            if (token.size() >= 3 && token[0] == '0' && (token[1] == 'x' || token[1] == 'X'))
            {
                unsigned int byte;
                std::istringstream hexstr(token);
                hexstr >> std::hex >> byte;
                buffer.push_back(static_cast<unsigned char>(byte));
            }
        }
        if (!dentroArray) break;
    }
    headerFile.close();

    if (buffer.empty())
    {
        errorMsg = L"No se encontraron datos válidos en el archivo de cabecera.";
        return false;
    }

    std::ofstream binFile(binPath, std::ios::binary);
    if (!binFile)
    {
        errorMsg = L"No se pudo crear el archivo binario.";
        return false;
    }
    binFile.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    binFile.close();
    return true;
}

// Función para guardar el buffer como cabecera C++
bool GuardarComoCabeceraCpp(const std::vector<unsigned char>& buffer, const std::string& variableName, const std::wstring& headerPath, std::wstring& errorMsg)
{
    std::ofstream headerFile(headerPath, std::ios::out | std::ios::trunc);
    if (!headerFile)
    {
        errorMsg = L"No se pudo crear el archivo de cabecera.";
        return false;
    }

    headerFile << "#pragma once\n";
    headerFile << "// Archivo generado automáticamente con Bin2h By Sergi WeBlackbug 2025\n";
    headerFile << "// Si te ha invitado esta aplicacion a ser Feliz con tu Programa ?..\n\n";
    headerFile << "// Envia una propina para tomarme un Café a esta direccion de mi cartera BitCoin en USDC\n\n";
    headerFile << "// DIRECCION BITCOIN bc1q9320qqdkevs2yxkpaf9a05apvql3w29qp8pahl\n\n";
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
                    // hBuffer se libera automáticamente al liberar el stream
                }
            }
        }
    }
    headerFile << "#include <cstddef>\n\n";
    headerFile << "const unsigned char " << variableName << "[] = {\n    ";

    size_t count = 0;
    for (size_t i = 0; i < buffer.size(); ++i)
    {
        headerFile << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)buffer[i];
        if (i != buffer.size() - 1)
            headerFile << ", ";

        count++;
        if (count == 16 && i != buffer.size() - 1)
        {
            headerFile << "\n    ";
            count = 0;
        }
    }
    headerFile << "\n};\n";
    headerFile << "const size_t " << variableName << "_size = sizeof(" << variableName << ");\n";

    headerFile.close();
    return true;
}

// Utilidad para obtener el nombre base de un archivo (sin ruta ni extensión)
std::string GetFileNameNoExt(const std::wstring& path)
{
    size_t slash = path.find_last_of(L"\\/");
    size_t dot = path.find_last_of(L'.');
    size_t start = (slash == std::wstring::npos) ? 0 : slash + 1;
    size_t end = (dot == std::wstring::npos || dot < start) ? path.length() : dot;
    std::wstring name = path.substr(start, end - start);

    // Convertir a std::string (ASCII simple)
    std::string result(name.begin(), name.end());
    // Reemplazar caracteres no válidos para variables
    for (auto& c : result) {
        if (!isalnum(static_cast<unsigned char>(c)))
            c = '_';
    }
    return result;
}




/*
// Dibuja texto con outline y sombra
void DrawTextWithOutlineAndShadow(
    HDC hdc,
    const std::wstring& text,
    int x, int y,
    COLORREF textColor,
    COLORREF outlineColor,
    COLORREF shadowColor,
    int outlineWidth = 2,
    int shadowOffset = 2,
    int fontSize = 32,
    const wchar_t* fontName = L"Segoe UI",
    const wchar_t* align = L"left" // "left", "center", "right"
) {
    HFONT hFont = CreateFontW(
        -fontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, VARIABLE_PITCH, fontName
    );
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    SetBkMode(hdc, TRANSPARENT);

    // Medir el ancho del texto para la alineación
    SIZE sz;
    GetTextExtentPoint32W(hdc, text.c_str(), (int)text.length(), &sz);
    int drawX = x;
    if (wcscmp(align, L"center") == 0) {
        drawX = x - sz.cx / 2;
    }
    else if (wcscmp(align, L"right") == 0) {
        drawX = x - sz.cx;
    }
    // Sombra
    SetTextColor(hdc, shadowColor);
    TextOutW(hdc, drawX + shadowOffset, y + shadowOffset, text.c_str(), (int)text.length());

    // Outline (contorno)
    SetTextColor(hdc, outlineColor);
    for (int dx = -outlineWidth; dx <= outlineWidth; ++dx) {
        for (int dy = -outlineWidth; dy <= outlineWidth; ++dy) {
            if (dx == 0 && dy == 0) continue;
            TextOutW(hdc, drawX + dx, y + dy, text.c_str(), (int)text.length());
        }
    }

    // Texto principal
    SetTextColor(hdc, textColor);
    TextOutW(hdc, drawX, y, text.c_str(), (int)text.length());

    // Restaurar fuente
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}
*/
void DrawTextWithOutlineAndShadow(
    HDC hdc,
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
    const wchar_t* align = L"left" // "left", "center", "right"
) {
    // Obtener el área cliente
    RECT rc;
    GetClientRect(hwnd, &rc);
    int width = rc.right - rc.left;

    // Crear un bitmap GDI+ para dibujar la sombra
    Bitmap bmp(width, rc.bottom - rc.top, PixelFormat32bppARGB);
    Graphics gBmp(&bmp);
    gBmp.SetSmoothingMode(SmoothingModeAntiAlias);
    gBmp.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    // Fuente GDI+
    FontFamily fontFamily(fontName);
    Font font(&fontFamily, (REAL)fontSize, FontStyleBold, UnitPixel);

    // Medir el texto
    RectF layoutRect(0, 0, (REAL)width, 1000.0f);
    RectF textRect;
    StringFormat format;
    format.SetLineAlignment(StringAlignmentNear);
    format.SetAlignment(StringAlignmentNear); // Medimos siempre como Near

    gBmp.MeasureString(text.c_str(), (INT)text.length(), &font, layoutRect, &format, &textRect);

    float drawX = (float)x;
    if (wcscmp(align, L"center") == 0) {
        drawX = x - textRect.Width / 2.0f;
    }
    else if (wcscmp(align, L"right") == 0) {
        drawX = x - textRect.Width;
    }
    // Sombra difuminada
    SolidBrush shadowBrush(Color(128, GetRValue(shadowColor), GetGValue(shadowColor), GetBValue(shadowColor)));
    for (int dx = -2; dx <= 2; ++dx) {
        for (int dy = -2; dy <= 2; ++dy) {
            if (dx == 0 && dy == 0) continue;
            gBmp.DrawString(
                text.c_str(), (INT)text.length(), &font,
                PointF(drawX + shadowOffset + dx, (REAL)y + shadowOffset + dy),
                &format, &shadowBrush
            );
        }
    }
    gBmp.DrawString(
        text.c_str(), (INT)text.length(), &font,
        PointF(drawX + shadowOffset, (REAL)y + shadowOffset),
        &format, &shadowBrush
    );

    // Outline (contorno)
    Pen outlinePen(Color(255, GetRValue(outlineColor), GetGValue(outlineColor), GetBValue(outlineColor)), (REAL)outlineWidth);
    GraphicsPath path;
    path.AddString(
        text.c_str(), (INT)text.length(), &fontFamily, FontStyleBold, (REAL)fontSize,
        PointF(drawX, (REAL)y), &format
    );
    gBmp.DrawPath(&outlinePen, &path);

    // Texto principal
    SolidBrush textBrush(Color(255, GetRValue(textColor), GetGValue(textColor), GetBValue(textColor)));
    gBmp.DrawString(
        text.c_str(), (INT)text.length(), &font,
        PointF(drawX, (REAL)y),
        &format, &textBrush
    );

    // Pintar el bitmap en el HDC destino
    Graphics gDest(hdc);
    gDest.DrawImage(&bmp, 0, 0);
}

// Función principal WinMain
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Inicializar GDI+
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Inicializar cadenas globales
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_BIN2H, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Realizar la inicialización de la aplicación:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_BIN2H));

    MSG msg;

    // Bucle principal de mensajes:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    GdiplusShutdown(gdiplusToken);
    return (int)msg.wParam;
}

// Registra la clase de ventana principal
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BIN2H));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_BIN2H);
    wcex.lpszClassName = szWindowClass;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON3));
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON3));
    return RegisterClassExW(&wcex);
}

// Inicializa la instancia y crea la ventana principal
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Almacenar identificador de instancia en una variable global

    // Tamaño fijo
    int winWidth = 600;
    int winHeight = 300;

    // Centrar en pantalla
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenWidth - winWidth) / 2;
    int y = (screenHeight - winHeight) / 2;

    HWND hWnd = CreateWindowW(
        szWindowClass, szTitle,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        x, y, winWidth, winHeight,
        nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

// Procedimiento de la ventana principal
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
    case WM_ERASEBKGND:
    {
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(hWnd, &rc);

        // Inicializa GDI+ para este contexto
        Graphics g(hdc);

        // Colores del degradado (ajusta a tu gusto)
        Color colorTop(255, 0, 32, 128);    // Azul oscuro
        Color colorBottom(255, 128, 192, 255); // Azul claro

        // Crea el gradiente vertical
        LinearGradientBrush brush(
            Point(0, 0),
            Point(0, rc.bottom - rc.top),
            colorTop,
            colorBottom
        );

        g.FillRectangle(&brush, 0, 0, rc.right - rc.left, rc.bottom - rc.top);

        return 1; // Indica que el fondo ya está dibujado
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        std::wstring texto = L"Bin2H 1.0 By WeBlackbug";
        int x = 290;
        int y = 50;

        DrawTextWithOutlineAndShadow(
            hdc, hWnd, texto, x, y,
            Colores::AzulMarino,
            Colores::Azul,
            Colores::GrisClaro,
            1, 1, 32,
            Fuentes::TimesNewRoman,
            L"center"
        );
        // Aquí puedes dibujar en la ventana principal si lo deseas
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:

        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Diálogo "Acerca de"
// ... (resto del código)

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
    {
        // Centrar el diálogo en la pantalla
        RECT rcDlg;
        GetWindowRect(hDlg, &rcDlg);

        int dlgWidth = rcDlg.right - rcDlg.left;
        int dlgHeight = rcDlg.bottom - rcDlg.top;

        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        int x = (screenWidth - dlgWidth) / 2;
        int y = (screenHeight - dlgHeight) / 2;

        SetWindowPos(hDlg, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

        // Crear el control personalizado sobre el placeholder
        HWND hPlaceholder = GetDlgItem(hDlg, IDC_CUSTOM2);
        RECT rc;
        GetWindowRect(hPlaceholder, &rc);
        MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)&rc, 2);

        g_aboutTextControl = new TextControl();
        g_aboutTextControl->Create(
            hDlg,
            rc.left, rc.top,
            rc.right - rc.left, rc.bottom - rc.top,
            2001, // ID único para el control
            (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE)
        );
        g_aboutTextControl->SetText(L"Acerca de Bin2H - Texto Sombreado");
        g_aboutTextControl->SetFontSize(18);
        g_aboutTextControl->SetTextColorValue(RGB(0, 128, 255));
        g_aboutTextControl->SetShadowColor(RGB(128, 128, 128));
        g_aboutTextControl->SetOutlineColor(RGB(255, 255, 255));
        g_aboutTextControl->SetOutlineWidth(2);

        // Oculta el placeholder
        ShowWindow(hPlaceholder, SW_HIDE);

        // Rotozoom: carga la imagen y configura animación
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
                        // hBuffer se libera automáticamente al liberar el stream
                    }
                }
            }
        }
        g_rotozoomAngle = 0.0f;
        g_rotozoomZoom = 1.0f;

        // Inicia un temporizador para animar el efecto (por ejemplo, 30 FPS)
        g_rotozoomTimer = SetTimer(hDlg, 1, 33, NULL);
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            if (g_aboutTextControl) {
                delete g_aboutTextControl;
                g_aboutTextControl = nullptr;
            }
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    case WM_TIMER:
        if (wParam == 1) {
            g_rotozoomAngle += 1.0f; // Velocidad de rotación
            if (g_rotozoomAngle >= 360.0f) g_rotozoomAngle -= 360.0f;
            g_rotozoomZoom = 1.0f + 0.5f * sin(GetTickCount() * 0.002f); // Zoom animado
            InvalidateRect(hDlg, NULL, FALSE); // Redibuja el diálogo
        }
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hDlg, &ps);

        // Dibuja el texto con sombra y contorno
        std::wstring texto = L"Bin2H 1.0 By WeBlackbug";
        int x = 290;
        int y = 50;

        DrawTextWithOutlineAndShadow(
            hdc, hDlg, texto, x, y,
            Colores::AzulMarino,
            Colores::Azul,
            Colores::GrisClaro,
            1, 1, 32,
            Fuentes::TimesNewRoman,
            L"center"
        );

        // --- ROTOZOOM SOBRE EL PICTURE CONTROL ---
        HWND hPic = GetDlgItem(hDlg, IDC_STATIC1);
        RECT rcPic;
        GetWindowRect(hPic, &rcPic);
        MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)&rcPic, 2);
        int width = rcPic.right - rcPic.left;
        int height = rcPic.bottom - rcPic.top;

        // Doble buffer
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbmMem = CreateCompatibleBitmap(hdc, width, height);
        HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

        // Fondo blanco (corregido: inicialización estándar de RECT)
        RECT rcFill;
        rcFill.left = 0;
        rcFill.top = 0;
        rcFill.right = width;
        rcFill.bottom = height;
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(hdcMem, &rcFill, hBrush);
        DeleteObject(hBrush);

        if (g_rotozoomImage) {
            Graphics g(hdcMem);
            g.SetSmoothingMode(SmoothingModeHighQuality);
            g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

            // Centro del control
            float cx = width / 2.0f;
            float cy = height / 2.0f;

            // Centro de la imagen
            float imgW = (float)g_rotozoomImage->GetWidth();
            float imgH = (float)g_rotozoomImage->GetHeight();

            // Transformación rotozoom
            g.TranslateTransform(cx, cy);
            g.RotateTransform(g_rotozoomAngle);
            g.ScaleTransform(g_rotozoomZoom, g_rotozoomZoom);
            g.TranslateTransform(-imgW / 2.0f, -imgH / 2.0f);

            // Dibuja la imagen
            g.DrawImage(g_rotozoomImage, 0, 0, static_cast<INT>(imgW), static_cast<INT>(imgH));
        }

        // Copia el buffer al control
        BitBlt(hdc, rcPic.left, rcPic.top, width, height, hdcMem, 0, 0, SRCCOPY);

        // Limpieza
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
        break;
    }
    return (INT_PTR)FALSE;
}
// Función para convertir .h a binario
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

// Función para convertir binario a .h
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

// Lanza el diálogo "Acerca de"
void OnAbout(HWND hWnd) {
    DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
}