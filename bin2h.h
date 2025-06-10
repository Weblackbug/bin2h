#pragma once

#include "resource.h"
#include <windows.h>
// Definimos 30 Colores Basicos.
class Colores {
public:
    static constexpr COLORREF Negro = RGB(0, 0, 0);
    static constexpr COLORREF Blanco = RGB(255, 255, 255);
    static constexpr COLORREF Rojo = RGB(255, 0, 0);
    static constexpr COLORREF Verde = RGB(0, 255, 0);
    static constexpr COLORREF Azul = RGB(0, 0, 255);
    static constexpr COLORREF Amarillo = RGB(255, 255, 0);
    static constexpr COLORREF Cyan = RGB(0, 255, 255);
    static constexpr COLORREF Magenta = RGB(255, 0, 255);
    static constexpr COLORREF Gris = RGB(128, 128, 128);
    static constexpr COLORREF GrisClaro = RGB(192, 192, 192);
    static constexpr COLORREF GrisOscuro = RGB(64, 64, 64);
    static constexpr COLORREF Naranja = RGB(255, 165, 0);
    static constexpr COLORREF Marron = RGB(139, 69, 19);
    static constexpr COLORREF Rosa = RGB(255, 192, 203);
    static constexpr COLORREF Violeta = RGB(128, 0, 128);
    static constexpr COLORREF VerdeOscuro = RGB(0, 100, 0);
    static constexpr COLORREF AzulMarino = RGB(0, 0, 128);
    static constexpr COLORREF Oro = RGB(255, 215, 0);
    static constexpr COLORREF Plata = RGB(192, 192, 192);
    static constexpr COLORREF Turquesa = RGB(64, 224, 208);
};
//Declaramos el tipo de fuente para usar
class Fuentes {
public:
    static constexpr const wchar_t* Arial = L"Arial";
    static constexpr const wchar_t* Calibri = L"Calibri";
    static constexpr const wchar_t* Cambria = L"Cambria";
    static constexpr const wchar_t* Candara = L"Candara";
    static constexpr const wchar_t* ComicSansMS = L"Comic Sans MS";
    static constexpr const wchar_t* Consolas = L"Consolas";
    static constexpr const wchar_t* Constantia = L"Constantia";
    static constexpr const wchar_t* Corbel = L"Corbel";
    static constexpr const wchar_t* CourierNew = L"Courier New";
    static constexpr const wchar_t* FranklinGothic = L"Franklin Gothic Medium";
    static constexpr const wchar_t* Garamond = L"Garamond";
    static constexpr const wchar_t* Georgia = L"Georgia";
    static constexpr const wchar_t* Impact = L"Impact";
    static constexpr const wchar_t* LucidaConsole = L"Lucida Console";
    static constexpr const wchar_t* LucidaSansUnicode = L"Lucida Sans Unicode";
    static constexpr const wchar_t* MicrosoftSansSerif = L"Microsoft Sans Serif";
    static constexpr const wchar_t* PalatinoLinotype = L"Palatino Linotype";
    static constexpr const wchar_t* SegoeUI = L"Segoe UI";
    static constexpr const wchar_t* SegoePrint = L"Segoe Print";
    static constexpr const wchar_t* SegoeScript = L"Segoe Script";
    static constexpr const wchar_t* Tahoma = L"Tahoma";
    static constexpr const wchar_t* TimesNewRoman = L"Times New Roman";
    static constexpr const wchar_t* TrebuchetMS = L"Trebuchet MS";
    static constexpr const wchar_t* Verdana = L"Verdana";
    static constexpr const wchar_t* BookmanOldStyle = L"Bookman Old Style";
    static constexpr const wchar_t* CenturyGothic = L"Century Gothic";
    static constexpr const wchar_t* CalistoMT = L"Calisto MT";
    static constexpr const wchar_t* GillSansMT = L"Gill Sans MT";
    static constexpr const wchar_t* Perpetua = L"Perpetua";
    static constexpr const wchar_t* Symbol = L"Symbol";
};
void OnArchivoConvertir(HWND hWnd);
void OnArchivoCargarBinario(HWND hWnd);
void OnAbout(HWND hWnd);