// bin2h.cpp : Define el punto de entrada de la aplicación.
//

#include "framework.h"
#include "bin2h.h"
#include <vector>
#include <fstream>
#include <string>
#include <commdlg.h>
#include <sstream>
#include <iomanip>
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
    headerFile << "// Archivo generado automáticamente\n";
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

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

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

    return (int)msg.wParam;
}

//
//  FUNCIÓN: MyRegisterClass()
//
//  PROPÓSITO: Registra la clase de ventana.
//
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
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON3));

    return RegisterClassExW(&wcex);
}

//
//   FUNCIÓN: InitInstance(HINSTANCE, int)
//
//   PROPÓSITO: Guarda el identificador de instancia y crea la ventana principal
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Almacenar identificador de instancia en una variable global

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  FUNCIÓN: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PROPÓSITO: Procesa mensajes de la ventana principal.
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Analizar las selecciones de menú:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case ID_ARCHIVO_CONVERTIR:
        {
            // Seleccionar archivo .h
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

            if (GetOpenFileName(&ofnOpen))
            {
                // Seleccionar destino binario
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

                if (GetSaveFileName(&ofnSave))
                {
                    std::wstring errorMsg;
                    if (ConvertirHeaderABinario(szHeader, szBin, errorMsg))
                    {
                        MessageBox(hWnd, L"Binario generado correctamente.", L"Éxito", MB_OK);
                    }
                    else
                    {
                        MessageBox(hWnd, errorMsg.c_str(), L"Error", MB_ICONERROR);
                    }
                }
            }
            break;
        }
        case ID_ARCHIVO_CARGARBINARIO:
        {
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

            if (GetOpenFileName(&ofn))
            {
                std::vector<unsigned char> buffer;
                std::wstring errorMsg;
                if (AbreArchivoBinario(ofn.lpstrFile, buffer, errorMsg))
                {
                    // Preguntar dónde guardar el header
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

                    if (GetSaveFileName(&ofnSave))
                    {
                        std::string variableName = GetFileNameNoExt(ofn.lpstrFile);
                        if (variableName.empty()) variableName = "bin_data";
                        std::wstring errorMsgHeader;
                        if (GuardarComoCabeceraCpp(buffer, variableName, ofnSave.lpstrFile, errorMsgHeader))
                        {
                            MessageBox(hWnd, L"Cabecera generada correctamente.", L"Éxito", MB_OK);
                        }
                        else
                        {
                            MessageBox(hWnd, errorMsgHeader.c_str(), L"Error", MB_ICONERROR);
                        }
                    }
                }
                else
                {
                    MessageBox(hWnd, errorMsg.c_str(), L"Error", MB_ICONERROR);
                }
            }
            break;
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Agregar cualquier código de dibujo que use hDC aquí...
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

// Controlador de mensajes del cuadro Acerca de.
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

        return (INT_PTR)TRUE;
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}