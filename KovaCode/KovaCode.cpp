#pragma comment(linker, "/SUBSYSTEM:windows")

#pragma comment(lib, "Comdlg32.lib")

#pragma comment(lib, "Comctl32.lib")

#pragma comment(lib, "Shell32.lib")

#pragma comment(lib, "Msimg32.lib")

#pragma comment(lib, "Dwmapi.lib")



#include <windows.h>

#include <windowsx.h>

#include <commdlg.h>

#include <commctrl.h>

#include <shellapi.h>

#include <dwmapi.h>

#include <stdio.h>

#include <string.h>

#include <ctype.h>

#include <vector>

#include <string>

#include <unordered_set>

#include <algorithm>

#include <cwctype>

#include "Scintilla.h"



// ============================================================

//  КОНСТАНТЫ ИНТЕРФЕЙСА И SCINTILLA

// ============================================================

#define IDI_APP_ICON            1



#define MARGIN_LINE_NUMBERS     0

#define MARGIN_MARKERS          1

#define MARGIN_FOLD             2



#define MARKER_UNSAVED          0

#define MARKER_SAVED            1



#define IDM_FILE_NEW            1000

#define IDM_FILE_OPEN           1001

#define IDM_FILE_SAVE           1002

#define IDM_FILE_SAVEAS         1003

#define IDM_FILE_OPEN_FOLDER    1004

#define IDM_FILE_EXIT           1005



#define IDM_EDIT_FIND_REPLACE   1006

#define IDM_EDIT_GOTO_LINE      1007

#define IDM_EDIT_DUPLICATE      1008

#define IDM_EDIT_COMMENT        1009

#define IDM_EDIT_SNIPPET_SP     1010

#define IDM_EDIT_CHAT_COLORS    1011

#define IDM_EDIT_LINE_UP        1012

#define IDM_EDIT_LINE_DOWN      1013



#define IDM_RUN_SMART           1014

#define IDM_SET_COMPILER        1015

#define IDM_CLEAR_CONSOLE       1016

#define IDM_ABOUT               1017



#define IDM_TAB_CLOSE           1100

#define IDM_TAB_CLOSE_OTHERS    1101

#define IDM_TAB_OPEN_DIR        1102

#define IDM_TAB_COPY_PATH       1103



#define IDM_THEME_DARK          2001

#define IDM_THEME_LIGHT         2002

#define IDM_THEME_TOGGLE        2003



#define IDM_LANG_RU             3001

#define IDM_LANG_EN             3002

#define IDM_LANG_DE             3003

#define IDM_LANG_FR             3004

#define IDM_LANG_IT             3005



#define LOG_NORMAL              0

#define LOG_ERROR               1

#define LOG_WARNING             2

#define LOG_SUCCESS             3

#define LOG_ACCENT              4



// ============================================================

//  ЯЗЫКИ И СТИЛИ ПОДСВЕТКИ (СОБСТВЕННЫЙ ЛЕКСЕР)

// ============================================================

enum Lang {

    LANG_NONE = 0,

    LANG_SP,

    LANG_PY,

    LANG_HTML

};



enum {

    S_DEFAULT = 0,

    S_COMMENT,

    S_BLOCKCOMMENT,

    S_STRING,

    S_TRIPLE,

    S_NUMBER,

    S_KEYWORD,

    S_CONTROL,

    S_TYPE,

    S_PREPROC,

    S_OPERATOR,

    S_FUNCTION,

    S_CONST,

    S_TAG,

    S_ATTR,

    S_HTMLCOMMENT

};



// ============================================================

//  ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ

// ============================================================

HINSTANCE hInst = NULL;

HWND hwndMain = NULL;

HWND hwndConsole = NULL;

HWND hwndFindDlg = NULL;

HWND hwndGotoDlg = NULL;

HWND hwndColorsDlg = NULL;

HWND hwndAboutDlg = NULL;

HMODULE hModScintilla = NULL;

HICON hGlobalAppIconBig = NULL;

HICON hGlobalAppIconSmall = NULL;



HFONT hUIFont = NULL;

HFONT hUIBoldFont = NULL;

HFONT hTitleFont = NULL;

HFONT hSmallFont = NULL;

HBRUSH hDarkEditBrush = NULL;



HWND hEditFindBox = NULL;

HWND hEditReplaceBox = NULL;

HWND hEditGotoBox = NULL;



bool isDarkTheme = true;

int currentLang = 0; // 0=RU, 1=EN, 2=DE, 3=FR, 4=IT



wchar_t szCompilerPath[MAX_PATH] = L"";

wchar_t szIniPath[MAX_PATH] = L"";

wchar_t szExeDir[MAX_PATH] = L"";

wchar_t szFindWhat[256] = L"";

wchar_t szReplaceWith[256] = L"";



// Геометрия интерфейса

int g_toolbarHeight = 38;

int g_tabHeight = 34;

int g_consoleHeaderHeight = 26;

int g_consoleHeight = 165;

int g_statusHeight = 25;



bool g_isDraggingSplitter = false;

int g_hoverToolbarBtn = -1;

int g_hoverTabIdx = -1;

int g_hoverTabCloseIdx = -1;

bool g_hoverPlusBtn = false;

int g_contextMenuTabIdx = -1;



struct TabData {

    HWND hwndEditor;

    std::wstring filePath;

    Lang lang;

    bool isModified;

    RECT rcTab;

    RECT rcClose;

};



std::vector<TabData> tabs;

int currentTab = -1;

RECT g_rcPlusBtn = { 0, 0, 0, 0 };



struct ToolbarButton {

    int id;

    const wchar_t* label;

    RECT rc;

    bool isAccent;

};



std::vector<ToolbarButton> g_toolbarButtons;



// ============================================================

//  ЛОКАЛИЗАЦИЯ (5 ЯЗЫКОВ)

// ============================================================

const wchar_t* loc[][16] = {

    // RU

    {

        L"Файл", L"Создать\tCtrl+N", L"Открыть...\tCtrl+O", L"Сохранить\tCtrl+S",

        L"Сохранить как...", L"Открыть папку с файлом", L"Выход",

        L"Правка", L"Найти и заменить...\tCtrl+F", L"Перейти к строке...\tCtrl+G",

        L"Дублировать строку\tCtrl+D", L"Вставить шаблон плагина\tCtrl+J",

        L"Сборка", L"Скомпилировать / Запустить\tF5", L"Выбрать spcomp.exe...",

        L"Настройки"

    },

    // EN

    {

        L"File", L"New\tCtrl+N", L"Open...\tCtrl+O", L"Save\tCtrl+S",

        L"Save As...", L"Open Containing Folder", L"Exit",

        L"Edit", L"Find & Replace...\tCtrl+F", L"Go to Line...\tCtrl+G",

        L"Duplicate Line\tCtrl+D", L"Insert Plugin Template\tCtrl+J",

        L"Build", L"Compile / Run\tF5", L"Select spcomp.exe...",

        L"Settings"

    },

    // DE

    {

        L"Datei", L"Neu\tCtrl+N", L"Öffnen...\tCtrl+O", L"Speichern\tCtrl+S",

        L"Speichern unter...", L"Ordner öffnen", L"Beenden",

        L"Bearbeiten", L"Suchen & Ersetzen...\tCtrl+F", L"Gehe zu Zeile...\tCtrl+G",

        L"Zeile duplizieren\tCtrl+D", L"Plugin-Vorlage einfügen\tCtrl+J",

        L"Erstellen", L"Kompilieren / Ausführen\tF5", L"Wähle spcomp.exe...",

        L"Einstellungen"

    },

    // FR

    {

        L"Fichier", L"Nouveau\tCtrl+N", L"Ouvrir...\tCtrl+O", L"Enregistrer\tCtrl+S",

        L"Enregistrer sous...", L"Ouvrir le dossier", L"Quitter",

        L"Édition", L"Rechercher et remplacer...\tCtrl+F", L"Aller à la ligne...\tCtrl+G",

        L"Dupliquer la ligne\tCtrl+D", L"Insérer modèle de plugin\tCtrl+J",

        L"Construire", L"Compiler / Exécuter\tF5", L"Sélectionner spcomp.exe...",

        L"Paramètres"

    },

    // IT

    {

        L"File", L"Nuovo\tCtrl+N", L"Apri...\tCtrl+O", L"Salva\tCtrl+S",

        L"Salva con nome...", L"Apri cartella file", L"Esci",

        L"Modifica", L"Trova e Sostituisci...\tCtrl+F", L"Vai alla riga...\tCtrl+G",

        L"Duplica riga\tCtrl+D", L"Inserisci modello plugin\tCtrl+J",

        L"Costruisci", L"Compila / Esegui\tF5", L"Seleziona spcomp.exe...",

        L"Impostazioni"

    }

};



// ============================================================

//  УМНЫЙ АВТОПОИСК SCINTILLA.DLL И ТВОЕЙ ОРИГИНАЛЬНОЙ LOGO.ICO

// ============================================================

void InitExePaths() {

    GetModuleFileNameW(NULL, szExeDir, MAX_PATH);

    wchar_t* lastSlash = wcsrchr(szExeDir, L'\\');

    if (lastSlash) *(lastSlash + 1) = 0;

    swprintf(szIniPath, MAX_PATH, L"%skovacode.ini", szExeDir);

}



HMODULE LoadScintillaSmart() {

    const wchar_t* dllNames[] = { L"SciLexer.dll", L"Scintilla.dll" };

    const wchar_t* relDirs[] = {

        L"",

        L"..\\",

        L"..\\..\\",

        L"..\\KovaCode\\",

        L"..\\..\\KovaCode\\",

        L"x64\\Debug\\",

        L"x64\\Release\\",

        L"..\\x64\\Debug\\"

    };



    for (int d = 0; d < 2; d++) {

        wchar_t localDll[MAX_PATH];

        swprintf(localDll, MAX_PATH, L"%s%s", szExeDir, dllNames[d]);

        HMODULE h = LoadLibraryW(localDll);

        if (h) return h;

    }



    for (int r = 0; r < 8; r++) {

        for (int d = 0; d < 2; d++) {

            wchar_t candidate[MAX_PATH];

            swprintf(candidate, MAX_PATH, L"%s%s%s", szExeDir, relDirs[r], dllNames[d]);

            if (GetFileAttributesW(candidate) != INVALID_FILE_ATTRIBUTES) {

                wchar_t targetLocal[MAX_PATH];

                swprintf(targetLocal, MAX_PATH, L"%s%s", szExeDir, dllNames[d]);

                CopyFileW(candidate, targetLocal, FALSE);

                HMODULE h = LoadLibraryW(targetLocal);

                if (!h) h = LoadLibraryW(candidate);

                if (h) return h;

            }

        }

    }



    for (int d = 0; d < 2; d++) {

        HMODULE h = LoadLibraryW(dllNames[d]);

        if (h) return h;

    }

    return NULL;

}



void LoadAppIconSmart(HINSTANCE hInstance) {

    // ПЕРВЫМ ДЕЛОМ ищем именно твой файл logo.ico на диске (чтобы не грузилась чужая/стандартная иконка)

    const wchar_t* relDirs[] = {

        L"",

        L"..\\",

        L"..\\..\\",

        L"..\\KovaCode\\",

        L"..\\..\\KovaCode\\"

    };

    for (int r = 0; r < 5; r++) {

        wchar_t candidate[MAX_PATH];

        swprintf(candidate, MAX_PATH, L"%s%slogo.ico", szExeDir, relDirs[r]);

        if (GetFileAttributesW(candidate) != INVALID_FILE_ATTRIBUTES) {

            wchar_t localIco[MAX_PATH];

            swprintf(localIco, MAX_PATH, L"%slogo.ico", szExeDir);

            if (r > 0) CopyFileW(candidate, localIco, FALSE);



            hGlobalAppIconBig = (HICON)LoadImageW(NULL, candidate, IMAGE_ICON, 48, 48, LR_LOADFROMFILE);

            if (!hGlobalAppIconBig) {

                hGlobalAppIconBig = (HICON)LoadImageW(NULL, candidate, IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);

            }

            hGlobalAppIconSmall = (HICON)LoadImageW(NULL, candidate, IMAGE_ICON, 16, 16, LR_LOADFROMFILE);

            if (!hGlobalAppIconSmall) {

                hGlobalAppIconSmall = hGlobalAppIconBig;

            }

            if (hGlobalAppIconBig) return;

        }

    }



    // Если файла на диске нет (например, упаковано в один .exe) — берем из ресурса IDI_APP_ICON

    hGlobalAppIconBig = (HICON)LoadImageW(hInstance, MAKEINTRESOURCEW(IDI_APP_ICON), IMAGE_ICON, 48, 48, 0);

    hGlobalAppIconSmall = (HICON)LoadImageW(hInstance, MAKEINTRESOURCEW(IDI_APP_ICON), IMAGE_ICON, 16, 16, 0);

    if (!hGlobalAppIconBig) {

        hGlobalAppIconBig = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_APP_ICON));

        hGlobalAppIconSmall = hGlobalAppIconBig;

    }

}



// ============================================================

//  СОХРАНЕНИЕ И ЗАГРУЗКА КОНФИГУРАЦИИ + СЕССИИ ВКЛАДОК (kovacode.ini)

// ============================================================

void LoadConfig() {

    isDarkTheme = GetPrivateProfileIntW(L"Settings", L"DarkTheme", 1, szIniPath) != 0;

    currentLang = GetPrivateProfileIntW(L"Settings", L"Language", 0, szIniPath);

    if (currentLang < 0 || currentLang > 4) currentLang = 0;

    GetPrivateProfileStringW(L"Compiler", L"SpcompPath", L"", szCompilerPath, MAX_PATH, szIniPath);

}



void SaveConfig() {

    WritePrivateProfileStringW(L"Settings", L"DarkTheme", isDarkTheme ? L"1" : L"0", szIniPath);

    wchar_t buf[32];

    swprintf(buf, 32, L"%d", currentLang);

    WritePrivateProfileStringW(L"Settings", L"Language", buf, szIniPath);

    WritePrivateProfileStringW(L"Compiler", L"SpcompPath", szCompilerPath, szIniPath);



    int savedCount = 0;

    for (size_t i = 0; i < tabs.size(); i++) {

        if (tabs[i].filePath.find(L"Новый файл") == std::wstring::npos &&

            GetFileAttributesW(tabs[i].filePath.c_str()) != INVALID_FILE_ATTRIBUTES) {

            wchar_t key[32];

            swprintf(key, 32, L"Tab%d", savedCount);

            WritePrivateProfileStringW(L"Session", key, tabs[i].filePath.c_str(), szIniPath);

            savedCount++;

        }

    }

    swprintf(buf, 32, L"%d", savedCount);

    WritePrivateProfileStringW(L"Session", L"Count", buf, szIniPath);

    swprintf(buf, 32, L"%d", currentTab);

    WritePrivateProfileStringW(L"Session", L"ActiveTab", buf, szIniPath);

}



// ============================================================

//  ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ И ТЕМНЫЙ ЗАГОЛОВОК ОКНА (DWM)

// ============================================================

LRESULT SendEd(HWND hwnd, UINT Msg, WPARAM wParam = 0, LPARAM lParam = 0) {

    return SendMessage(hwnd, Msg, wParam, lParam);

}



LRESULT SendActive(UINT Msg, WPARAM wParam = 0, LPARAM lParam = 0) {

    if (currentTab >= 0 && currentTab < (int)tabs.size()) {

        return SendMessage(tabs[currentTab].hwndEditor, Msg, wParam, lParam);

    }

    return 0;

}



void ApplyDwmDarkTitleBar(HWND hwnd, bool dark) {

    BOOL value = dark ? TRUE : FALSE;

    DwmSetWindowAttribute(hwnd, 20, &value, sizeof(value));

    DwmSetWindowAttribute(hwnd, 19, &value, sizeof(value));

}



int FindTab(HWND h) {

    for (int i = 0; i < (int)tabs.size(); i++) {

        if (tabs[i].hwndEditor == h) {

            return i;

        }

    }

    return -1;

}



Lang GetLang(const std::wstring& path) {

    size_t dot = path.find_last_of(L'.');

    size_t slash = path.find_last_of(L"\\/");

    if (dot == std::wstring::npos || (slash != std::wstring::npos && dot < slash)) {

        return LANG_NONE;

    }

    std::wstring ext = path.substr(dot + 1);

    std::transform(ext.begin(), ext.end(), ext.begin(), [](wchar_t c) {

        return (wchar_t)towlower(c);

        });

    if (ext == L"sp" || ext == L"inc" || ext == L"c" || ext == L"cpp" || ext == L"h" || ext == L"hpp") {

        return LANG_SP;

    }

    if (ext == L"py" || ext == L"pyw") {

        return LANG_PY;

    }

    if (ext == L"html" || ext == L"htm" || ext == L"php" || ext == L"xml" || ext == L"css" || ext == L"js") {

        return LANG_HTML;

    }

    return LANG_NONE;

}



const wchar_t* GetLangName(Lang l) {

    switch (l) {

    case LANG_SP:   return L"SourcePawn / C++";

    case LANG_PY:   return L"Python";

    case LANG_HTML: return L"HTML / Web";

    default:        return L"Plain Text (.txt)";

    }

}



void LogToConsoleW(const wchar_t* msg, int logType = LOG_NORMAL) {

    if (!hwndConsole) return;

    int size = WideCharToMultiByte(CP_UTF8, 0, msg, -1, NULL, 0, NULL, NULL);

    if (size > 0) {

        std::vector<char> utf8(size);

        WideCharToMultiByte(CP_UTF8, 0, msg, -1, &utf8[0], size, NULL, NULL);

        int startPos = (int)SendEd(hwndConsole, SCI_GETLENGTH);

        SendEd(hwndConsole, SCI_SETREADONLY, FALSE);

        SendEd(hwndConsole, SCI_APPENDTEXT, size - 1, (LPARAM)utf8.data());

        SendEd(hwndConsole, SCI_APPENDTEXT, 1, (LPARAM)"\n");

        SendEd(hwndConsole, SCI_STARTSTYLING, startPos, 0);

        SendEd(hwndConsole, SCI_SETSTYLING, size, logType);

        SendEd(hwndConsole, SCI_GOTOPOS, SendEd(hwndConsole, SCI_GETLENGTH));

        SendEd(hwndConsole, SCI_SETREADONLY, TRUE);

    }

}



void ClearConsole() {

    if (!hwndConsole) return;

    SendEd(hwndConsole, SCI_SETREADONLY, FALSE);

    SendEd(hwndConsole, SCI_CLEARALL);

    SendEd(hwndConsole, SCI_SETREADONLY, TRUE);

    LogToConsoleW(L"> Консоль очищена.", LOG_NORMAL);

}



// ============================================================

//  ЛЕКСЕР + СВОРАЧИВАНИЕ КОДА (CODE FOLDING)

// ============================================================

struct LangSets {

    std::unordered_set<std::string> ctrl;

    std::unordered_set<std::string> kw;

    std::unordered_set<std::string> types;

    std::unordered_set<std::string> consts;

};



static std::unordered_set<std::string> MakeSet(const char* words) {

    std::unordered_set<std::string> r;

    std::string cur;

    for (const char* p = words; ; p++) {

        if (*p == ' ' || *p == 0) {

            if (!cur.empty()) {

                r.insert(cur);

                cur.clear();

            }

            if (!*p) break;

        }

        else {

            cur += *p;

        }

    }

    return r;

}



static const LangSets& GetSets(Lang l) {

    static LangSets sp, py, none;

    static bool init = false;

    if (!init) {

        init = true;

        sp.ctrl = MakeSet("if else for while do switch case default break continue return goto");

        sp.kw = MakeSet(

            "new decl static stock public native forward const enum struct methodmap property "

            "typedef typeset funcenum functag using sizeof view_as delete this "

            "int float bool char void any class union namespace template typename "

            "unsigned signed long short double auto extern volatile inline virtual private protected"

        );

        sp.types = MakeSet(

            "Handle Action Plugin Float String ConVar ArrayList StringMap DataPack Event Menu "

            "Panel KeyValues Database DBResultSet DBStatement Regex File DirectoryListing "

            "Cookie Function Timer GameData Address Profiler SMCParser Transaction TopMenu "

            "QueryCookie ConVarQueryResult ReplySource ParamType APLRes"

        );

        sp.consts = MakeSet(

            "true false null INVALID_HANDLE Plugin_Continue Plugin_Handled Plugin_Stop "

            "Plugin_Changed MAXPLAYERS MAX_NAME_LENGTH PLATFORM_MAX_PATH FCVAR_NOTIFY "

            "FCVAR_PLUGIN FCVAR_NONE TIMER_REPEAT TIMER_FLAG_NO_MAPCHANGE myinfo MaxClients"

        );



        py.ctrl = MakeSet("if elif else for while try except finally with return break continue pass raise yield import from as assert del");

        py.kw = MakeSet("def class lambda global nonlocal async await in is not and or self");

        py.types = MakeSet("int str float list dict set tuple bool bytes object type range complex frozenset bytearray Exception ValueError TypeError KeyError print len open");

        py.consts = MakeSet("True False None __name__ __main__");

    }

    if (l == LANG_SP) return sp;

    if (l == LANG_PY) return py;

    return none;

}



static void Fill(std::vector<unsigned char>& st, int a, int b, unsigned char s) {

    if (a < 0) a = 0;

    for (int k = a; k < b && k < (int)st.size(); k++) {

        st[k] = s;

    }

}



static bool IsUpperConst(const std::string& w) {

    if (w.size() < 2) return false;

    bool hasUpper = false;

    for (char c : w) {

        if (c >= 'A' && c <= 'Z') {

            hasUpper = true;

        }

        else if (!((c >= '0' && c <= '9') || c == '_')) {

            return false;

        }

    }

    return hasUpper;

}



static bool IsPyStrPrefix(const std::string& w) {

    static const char* p[] = { "r", "u", "b", "f", "rb", "br", "fr", "rf", "ur" };

    std::string l;

    for (char c : w) l += (char)tolower((unsigned char)c);

    for (auto x : p) {

        if (l == x) return true;

    }

    return false;

}



static void LexCode(const char* s, int n, std::vector<unsigned char>& st, int init, bool py, const LangSets& L) {

    int i = 0;

    if (init == S_BLOCKCOMMENT && !py) {

        int j = 0;

        while (j + 1 < n && !(s[j] == '*' && s[j + 1] == '/')) j++;

        j = (j + 1 < n) ? j + 2 : n;

        Fill(st, 0, j, S_BLOCKCOMMENT);

        i = j;

    }

    else if (init == S_TRIPLE && py) {

        int j = 0;

        while (j + 2 < n && !((s[j] == '"' || s[j] == '\'') && s[j + 1] == s[j] && s[j + 2] == s[j])) j++;

        j = (j + 2 < n) ? j + 3 : n;

        Fill(st, 0, j, S_TRIPLE);

        i = j;

    }



    std::string prevWord;

    while (i < n) {

        unsigned char c = (unsigned char)s[i];

        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {

            i++;

            continue;

        }



        if (!py && c == '/' && i + 1 < n && s[i + 1] == '/') {

            int j = i;

            while (j < n && s[j] != '\n') j++;

            Fill(st, i, j, S_COMMENT);

            i = j;

            continue;

        }

        if (!py && c == '/' && i + 1 < n && s[i + 1] == '*') {

            int j = i + 2;

            while (j + 1 < n && !(s[j] == '*' && s[j + 1] == '/')) j++;

            j = (j + 1 < n) ? j + 2 : n;

            Fill(st, i, j, S_BLOCKCOMMENT);

            i = j;

            continue;

        }

        if (py && c == '#') {

            int j = i;

            while (j < n && s[j] != '\n') j++;

            Fill(st, i, j, S_COMMENT);

            i = j;

            continue;

        }



        if (c == '"' || c == '\'') {

            prevWord.clear();

            if (py && i + 2 < n && s[i + 1] == (char)c && s[i + 2] == (char)c) {

                int j = i + 3;

                while (j + 2 < n && !(s[j] == (char)c && s[j + 1] == (char)c && s[j + 2] == (char)c)) {

                    if (s[j] == '\\') j++;

                    j++;

                }

                j = (j + 2 < n) ? j + 3 : n;

                Fill(st, i, j, S_TRIPLE);

                i = j;

                continue;

            }

            int j = i + 1;

            while (j < n && s[j] != (char)c && s[j] != '\n') {

                if (s[j] == '\\' && j + 1 < n && s[j + 1] != '\n') j++;

                j++;

            }

            if (j < n && s[j] == (char)c) j++;

            Fill(st, i, j, S_STRING);

            i = j;

            continue;

        }



        if (isdigit(c) || (c == '.' && i + 1 < n && isdigit((unsigned char)s[i + 1]))) {

            int j = i + 1;

            while (j < n && (isalnum((unsigned char)s[j]) || s[j] == '.' || s[j] == '_')) j++;

            Fill(st, i, j, S_NUMBER);

            prevWord.clear();

            i = j;

            continue;

        }



        if (!py && c == '#') {

            int k = i - 1;

            while (k >= 0 && (s[k] == ' ' || s[k] == '\t')) k--;

            if (k < 0 || s[k] == '\n') {

                int j = i + 1;

                while (j < n && (s[j] == ' ' || s[j] == '\t')) j++;

                int w = j;

                while (j < n && isalpha((unsigned char)s[j])) j++;

                Fill(st, i, j, S_PREPROC);

                std::string d(s + w, j - w);

                if (d == "include" || d == "tryinclude") {

                    int q = j;

                    while (q < n && (s[q] == ' ' || s[q] == '\t')) q++;

                    if (q < n && s[q] == '<') {

                        int e = q;

                        while (e < n && s[e] != '>' && s[e] != '\n') e++;

                        if (e < n && s[e] == '>') e++;

                        Fill(st, q, e, S_STRING);

                        j = e;

                    }

                }

                i = j;

                prevWord.clear();

                continue;

            }

        }



        if (py && c == '@') {

            int j = i + 1;

            while (j < n && (isalnum((unsigned char)s[j]) || s[j] == '_' || s[j] == '.')) j++;

            Fill(st, i, j, S_FUNCTION);

            i = j;

            continue;

        }



        if (isalpha(c) || c == '_') {

            int j = i + 1;

            while (j < n && (isalnum((unsigned char)s[j]) || s[j] == '_')) j++;

            std::string w(s + i, j - i);

            unsigned char sty = S_DEFAULT;

            if (py && j < n && (s[j] == '"' || s[j] == '\'') && w.size() <= 2 && IsPyStrPrefix(w)) {

                sty = S_STRING;

            }

            else if (L.ctrl.count(w)) {

                sty = S_CONTROL;

            }

            else if (L.kw.count(w)) {

                sty = S_KEYWORD;

            }

            else if (L.types.count(w)) {

                sty = S_TYPE;

            }

            else if (L.consts.count(w)) {

                sty = S_CONST;

            }

            else if (prevWord == "class" || prevWord == "enum" || prevWord == "struct" || prevWord == "methodmap" || prevWord == "typedef") {

                sty = S_TYPE;

            }

            else {

                int q = j;

                while (q < n && (s[q] == ' ' || s[q] == '\t')) q++;

                if (q < n && s[q] == '(') {

                    sty = S_FUNCTION;

                }

                else if (IsUpperConst(w)) {

                    sty = S_CONST;

                }

            }

            Fill(st, i, j, sty);

            prevWord = w;

            i = j;

            continue;

        }



        if (c < 128 && ispunct(c)) {

            st[i] = S_OPERATOR;

            prevWord.clear();

        }

        i++;

    }

}



static void LexHtml(const char* s, int n, std::vector<unsigned char>& st, int init) {

    int i = 0;

    if (init == S_HTMLCOMMENT) {

        int j = 0;

        while (j + 2 < n && !(s[j] == '-' && s[j + 1] == '-' && s[j + 2] == '>')) j++;

        j = (j + 2 < n) ? j + 3 : n;

        Fill(st, 0, j, S_HTMLCOMMENT);

        i = j;

    }

    while (i < n) {

        if (s[i] == '<' && i + 3 < n && strncmp(s + i, "<!--", 4) == 0) {

            int j = i + 4;

            while (j + 2 < n && !(s[j] == '-' && s[j + 1] == '-' && s[j + 2] == '>')) j++;

            j = (j + 2 < n) ? j + 3 : n;

            Fill(st, i, j, S_HTMLCOMMENT);

            i = j;

            continue;

        }

        if (s[i] == '<' && i + 1 < n && s[i + 1] == '?') {

            int j = i + 2;

            while (j + 1 < n && !(s[j] == '?' && s[j + 1] == '>')) j++;

            j = (j + 1 < n) ? j + 2 : n;

            Fill(st, i, j, S_PREPROC);

            i = j;

            continue;

        }

        if (s[i] == '<' && i + 1 < n && (isalpha((unsigned char)s[i + 1]) || s[i + 1] == '/' || s[i + 1] == '!')) {

            st[i] = S_OPERATOR;

            int j = i + 1;

            if (s[j] == '/' || s[j] == '!') {

                st[j] = S_OPERATOR;

                j++;

            }

            int a = j;

            while (j < n && (isalnum((unsigned char)s[j]) || s[j] == '-' || s[j] == ':' || s[j] == '_')) j++;

            Fill(st, a, j, S_TAG);

            while (j < n && s[j] != '>') {

                unsigned char c = (unsigned char)s[j];

                if (c == '"' || c == '\'') {

                    int e = j + 1;

                    while (e < n && s[e] != (char)c) e++;

                    if (e < n) e++;

                    Fill(st, j, e, S_STRING);

                    j = e;

                }

                else if (isalpha(c) || c == '_' || c == '@' || c == ':') {

                    int e = j + 1;

                    while (e < n && (isalnum((unsigned char)s[e]) || s[e] == '-' || s[e] == '_' || s[e] == ':' || s[e] == '.' || s[e] == '@')) e++;

                    Fill(st, j, e, S_ATTR);

                    j = e;

                }

                else {

                    if (c == '=' || c == '/') st[j] = S_OPERATOR;

                    j++;

                }

            }

            if (j < n) {

                st[j] = S_OPERATOR;

                j++;

            }

            i = j;

            continue;

        }

        i++;

    }

}



void UpdateCodeFolding(HWND h, Lang lg) {

    if (lg != LANG_SP) return;

    int lines = (int)SendEd(h, SCI_GETLINECOUNT);

    int level = SC_FOLDLEVELBASE;

    for (int line = 0; line < lines; line++) {

        int start = (int)SendEd(h, SCI_POSITIONFROMLINE, line);

        int end = (int)SendEd(h, SCI_GETLINEENDPOSITION, line);

        int nextLevel = level;

        for (int p = start; p < end; p++) {

            int sty = (int)SendEd(h, SCI_GETSTYLEAT, p);

            if (sty == S_COMMENT || sty == S_BLOCKCOMMENT || sty == S_STRING) continue;

            int ch = (int)SendEd(h, SCI_GETCHARAT, p);

            if (ch == '{') nextLevel++;

            else if (ch == '}' && nextLevel > SC_FOLDLEVELBASE) nextLevel--;

        }

        int flags = level;

        if (nextLevel > level) flags |= SC_FOLDLEVELHEADERFLAG;

        SendEd(h, SCI_SETFOLDLEVEL, line, flags);

        level = nextLevel;

    }

}



void StyleEditor(TabData& t, int reqPos) {

    HWND h = t.hwndEditor;

    int len = (int)SendEd(h, SCI_GETLENGTH);

    if (len <= 0) return;



    const int SMALL_DOC = 250000;

    int start, end;

    if (len < SMALL_DOC) {

        start = 0;

        end = len;

    }

    else {

        int endStyled = (int)SendEd(h, SCI_GETENDSTYLED);

        start = (int)SendEd(h, SCI_POSITIONFROMLINE, SendEd(h, SCI_LINEFROMPOSITION, endStyled));

        int l = (int)SendEd(h, SCI_LINEFROMPOSITION, reqPos);

        int lines = (int)SendEd(h, SCI_GETLINECOUNT);

        end = (l + 1 < lines) ? (int)SendEd(h, SCI_POSITIONFROMLINE, l + 1) : len;

        if (end <= start) end = len;

    }

    int n = end - start;

    if (n <= 0) return;



    std::vector<char> buf(n + 1, 0);

    Sci_TextRange tr;

    tr.chrg.cpMin = start;

    tr.chrg.cpMax = end;

    tr.lpstrText = buf.data();

    SendEd(h, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);



    int init = (start > 0) ? (int)SendEd(h, SCI_GETSTYLEAT, start - 1) : S_DEFAULT;

    std::vector<unsigned char> st(n, S_DEFAULT);



    if (t.lang == LANG_SP) {

        LexCode(buf.data(), n, st, init, false, GetSets(LANG_SP));

    }

    else if (t.lang == LANG_PY) {

        LexCode(buf.data(), n, st, init, true, GetSets(LANG_PY));

    }

    else if (t.lang == LANG_HTML) {

        LexHtml(buf.data(), n, st, init);

    }



    SendEd(h, SCI_STARTSTYLING, start, 0);

    int i = 0;

    while (i < n) {

        int j = i + 1;

        while (j < n && st[j] == st[i]) j++;

        SendEd(h, SCI_SETSTYLING, j - i, st[i]);

        i = j;

    }

    if (len < 100000) {

        UpdateCodeFolding(h, t.lang);

    }

}



void RelexTab(HWND h) {

    SendEd(h, SCI_STARTSTYLING, 0, 0);

    SendEd(h, SCI_COLOURISE, 0, -1);

}



// ============================================================

//  УМНОЕ ПОВЕДЕНИЕ РЕДАКТОРА (СКОБКИ, АВТОЗАКРЫТИЕ, ШАБЛОНЫ)

// ============================================================

static bool IsBraceCh(int c) {

    return c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}';

}



void HandleBraces(HWND h) {

    int pos = (int)SendEd(h, SCI_GETCURRENTPOS);

    int bp = -1;

    int c1 = pos > 0 ? (int)SendEd(h, SCI_GETCHARAT, pos - 1) : 0;

    if (IsBraceCh(c1)) {

        bp = pos - 1;

    }

    else if (IsBraceCh((int)SendEd(h, SCI_GETCHARAT, pos))) {

        bp = pos;

    }

    if (bp >= 0) {

        int m = (int)SendEd(h, SCI_BRACEMATCH, bp, 0);

        if (m >= 0) {

            SendEd(h, SCI_BRACEHIGHLIGHT, bp, m);

        }

        else {

            SendEd(h, SCI_BRACEBADLIGHT, bp, 0);

        }

    }

    else {

        SendEd(h, SCI_BRACEHIGHLIGHT, (WPARAM)-1, -1);

    }

}



void HandleCharAdded(TabData& t, int ch) {

    HWND h = t.hwndEditor;

    int pos = (int)SendEd(h, SCI_GETCURRENTPOS);

    int line = (int)SendEd(h, SCI_LINEFROMPOSITION, pos);



    if (ch == '\n') {

        if (line <= 0) return;

        int ind = (int)SendEd(h, SCI_GETLINEINDENTATION, line - 1);

        int lineStart = (int)SendEd(h, SCI_POSITIONFROMLINE, line - 1);

        int k = (int)SendEd(h, SCI_GETLINEENDPOSITION, line - 1) - 1;

        while (k >= lineStart) {

            int c = (int)SendEd(h, SCI_GETCHARAT, k);

            if (c == ' ' || c == '\t') k--; else break;

        }

        int last = (k >= lineStart) ? (int)SendEd(h, SCI_GETCHARAT, k) : 0;

        int tw = (int)SendEd(h, SCI_GETTABWIDTH);

        if (last == '{' || last == '(' || last == '[' || (t.lang == LANG_PY && last == ':')) {

            ind += tw;

        }

        SendEd(h, SCI_SETLINEINDENTATION, line, ind);

        SendEd(h, SCI_GOTOPOS, SendEd(h, SCI_GETLINEINDENTPOSITION, line));

    }

    else if (ch == '}') {

        int ip = (int)SendEd(h, SCI_GETLINEINDENTPOSITION, line);

        if (ip == pos - 1) {

            int m = (int)SendEd(h, SCI_BRACEMATCH, pos - 1, 0);

            if (m >= 0) {

                int ml = (int)SendEd(h, SCI_LINEFROMPOSITION, m);

                SendEd(h, SCI_SETLINEINDENTATION, line, SendEd(h, SCI_GETLINEINDENTATION, ml));

                SendEd(h, SCI_GOTOPOS, SendEd(h, SCI_GETLINEINDENTPOSITION, line) + 1);

            }

        }

    }

    else if (ch == '(' || ch == '[' || ch == '{') {

        int nextCh = (int)SendEd(h, SCI_GETCHARAT, pos);

        if (nextCh == 0 || nextCh == ' ' || nextCh == '\r' || nextCh == '\n' || nextCh == ')' || nextCh == ']' || nextCh == ';') {

            const char* closeStr = (ch == '(') ? ")" : (ch == '[') ? "]" : "}";

            SendEd(h, SCI_INSERTTEXT, pos, (LPARAM)closeStr);

        }

    }

}



void UpdateLineMargin(HWND h) {

    int lines = (int)SendEd(h, SCI_GETLINECOUNT);

    int digits = (int)std::to_string(lines).size();

    if (digits < 3) digits = 3;

    std::string probe(digits, '9');

    int w = (int)SendEd(h, SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM)probe.c_str()) + 16;

    SendEd(h, SCI_SETMARGINWIDTHN, MARGIN_LINE_NUMBERS, w);

}



void InsertSourcePawnTemplate() {

    if (currentTab < 0) return;

    const char* tpl =

        "#pragma semicolon 1\n"

        "#pragma newdecls required\n\n"

        "#include <sourcemod>\n"

        "#include <sdktools>\n"

        "#include <sdkhooks>\n"

        "#include <morecolors>\n\n"

        "#define PLUGIN_VERSION \"1.0.0\"\n\n"

        "public Plugin myinfo = \n"

        "{\n"

        "    name        = \"New Plugin\",\n"

        "    author      = \"ANR1S\",\n"

        "    description = \"Created in Kova Code Studio v1.0.0\",\n"

        "    version     = PLUGIN_VERSION,\n"

        "    url         = \"\"\n"

        "};\n\n"

        "public void OnPluginStart()\n"

        "{\n"

        "    PrintToServer(\"[KovaCode] Plugin successfully loaded!\");\n"

        "}\n";



    SendActive(SCI_REPLACESEL, 0, (LPARAM)tpl);

    tabs[currentTab].lang = LANG_SP;

    RelexTab(tabs[currentTab].hwndEditor);

    InvalidateRect(hwndMain, NULL, FALSE);

    LogToConsoleW(L"> Вставлен шаблон плагина SourcePawn (ANR1S).", LOG_ACCENT);

}



void DuplicateCurrentLine() {

    if (currentTab < 0) return;

    SendActive(SCI_LINEDUPLICATE);

}



void MoveSelectedLinesUp() {

    if (currentTab < 0) return;

    SendActive(SCI_MOVESELECTEDLINESUP);

}



void MoveSelectedLinesDown() {

    if (currentTab < 0) return;

    SendActive(SCI_MOVESELECTEDLINESDOWN);

}



void ToggleLineComment() {

    if (currentTab < 0) return;

    HWND h = tabs[currentTab].hwndEditor;

    int pos = (int)SendEd(h, SCI_GETCURRENTPOS);

    int line = (int)SendEd(h, SCI_LINEFROMPOSITION, pos);

    int indentPos = (int)SendEd(h, SCI_GETLINEINDENTPOSITION, line);

    int c1 = (int)SendEd(h, SCI_GETCHARAT, indentPos);

    int c2 = (int)SendEd(h, SCI_GETCHARAT, indentPos + 1);



    if (tabs[currentTab].lang == LANG_PY) {

        if (c1 == '#') {

            SendEd(h, SCI_DELETERANGE, indentPos, (c2 == ' ') ? 2 : 1);

        }

        else {

            SendEd(h, SCI_INSERTTEXT, indentPos, (LPARAM)"# ");

        }

    }

    else {

        if (c1 == '/' && c2 == '/') {

            int c3 = (int)SendEd(h, SCI_GETCHARAT, indentPos + 2);

            SendEd(h, SCI_DELETERANGE, indentPos, (c3 == ' ') ? 3 : 2);

        }

        else {

            SendEd(h, SCI_INSERTTEXT, indentPos, (LPARAM)"// ");

        }

    }

}



void OpenContainingFolder(int tabIdx) {

    if (tabIdx < 0 || tabIdx >= (int)tabs.size()) return;

    std::wstring path = tabs[tabIdx].filePath;

    if (path.find(L"Новый файл") != std::wstring::npos) {

        LogToConsoleW(L"[INFO] Файл еще не сохранен на диск.", LOG_WARNING);

        return;

    }

    std::wstring param = L"/select,\"" + path + L"\"";

    ShellExecuteW(NULL, L"open", L"explorer.exe", param.c_str(), NULL, SW_SHOWNORMAL);

}



void CopyFilePathToClipboard(int tabIdx) {

    if (tabIdx < 0 || tabIdx >= (int)tabs.size()) return;

    const std::wstring& s = tabs[tabIdx].filePath;

    if (OpenClipboard(hwndMain)) {

        EmptyClipboard();

        size_t bytes = (s.size() + 1) * sizeof(wchar_t);

        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);

        if (hMem) {

            void* ptr = GlobalLock(hMem);

            memcpy(ptr, s.c_str(), bytes);

            GlobalUnlock(hMem);

            SetClipboardData(CF_UNICODETEXT, hMem);

        }

        CloseClipboard();

        LogToConsoleW(L"> Путь к файлу скопирован в буфер обмена.", LOG_SUCCESS);

    }

}



// ============================================================

//  ПАЛИТРА И ТЕМЫ (VS CODE DARK+ / LIGHT+ БЕЗ БЕЛЫХ ПЯТЕН)

// ============================================================

struct Pal {

    COLORREF bg, fg, lineNumBg, lineNumFg, sel, caretLine;

    COLORREF comment, str, num, kw, ctrl, type, func, cons, pre, op, tag, attr, guide, braceBg, braceFg;

    COLORREF uiTopBar, uiTabBar, uiTabActive, uiTabInactive, uiTabHover, uiBorder, uiStatusBg, uiStatusFg, uiAccent;

};



Pal GetPalette() {

    if (isDarkTheme) {

        return {

            RGB(24, 24, 27),    // bg редактора

            RGB(212, 212, 216), // fg текста

            RGB(20, 20, 23),    // фон колонки номеров строк

            RGB(115, 115, 128), // цвет цифр строк

            RGB(38, 79, 120),   // выделение

            RGB(32, 32, 38),    // текущая строка

            RGB(106, 153, 85),  // комментарии

            RGB(206, 145, 120), // строки

            RGB(181, 206, 168), // числа

            RGB(86, 156, 214),  // ключевые слова

            RGB(197, 134, 192), // управляющие конструкции

            RGB(78, 201, 176),  // типы данных

            RGB(220, 220, 170), // функции

            RGB(79, 193, 255),  // константы

            RGB(197, 134, 192), // препроцессор

            RGB(212, 212, 212), // операторы

            RGB(86, 156, 214),  // HTML теги

            RGB(156, 220, 254), // HTML атрибуты

            RGB(55, 55, 62),    // направляющие отступов

            RGB(65, 65, 80),    // подсветка скобок фон

            RGB(255, 215, 0),   // подсветка скобок текст

            RGB(28, 28, 34),    // uiTopBar

            RGB(18, 18, 22),    // uiTabBar

            RGB(24, 24, 27),    // активная вкладка

            RGB(31, 31, 37),    // неактивная вкладка

            RGB(42, 42, 50),    // наведение на вкладку

            RGB(52, 52, 62),    // рамки и разделители

            RGB(16, 16, 20),    // статус-бар

            RGB(195, 195, 210), // текст статус-бара

            RGB(124, 58, 237)   // фирменный акцент KovaCode

        };

    }

    return {

        RGB(255, 255, 255), RGB(20, 20, 20), RGB(245, 245, 248), RGB(110, 110, 120),

        RGB(173, 214, 255), RGB(245, 246, 250),

        RGB(0, 128, 0), RGB(163, 21, 21), RGB(9, 134, 88), RGB(0, 0, 255),

        RGB(175, 0, 219), RGB(38, 127, 153), RGB(121, 94, 38), RGB(0, 112, 193),

        RGB(175, 0, 219), RGB(0, 0, 0), RGB(128, 0, 0), RGB(229, 0, 0),

        RGB(215, 215, 220), RGB(210, 235, 210), RGB(0, 0, 0),

        RGB(238, 238, 242), RGB(222, 222, 230), RGB(255, 255, 255), RGB(232, 232, 238),

        RGB(242, 242, 248), RGB(195, 195, 205), RGB(0, 122, 204), RGB(255, 255, 255), RGB(0, 122, 204)

    };

}



void SetupEditor(HWND h) {

    SendEd(h, SCI_SETCODEPAGE, SC_CP_UTF8, 0);

    SendEd(h, SCI_SETMODEVENTMASK, SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT, 0);

    SendEd(h, SCI_SETUSETABS, 0, 0);

    SendEd(h, SCI_SETTABWIDTH, 4, 0);

    SendEd(h, SCI_SETINDENT, 4, 0);

    SendEd(h, SCI_SETTABINDENTS, 1, 0);

    SendEd(h, SCI_SETBACKSPACEUNINDENTS, 1, 0);

    SendEd(h, SCI_SETINDENTATIONGUIDES, SC_IV_LOOKBOTH, 0);

    SendEd(h, SCI_SETCARETLINEVISIBLE, 1, 0);

    SendEd(h, SCI_SETCARETWIDTH, 2, 0);

    SendEd(h, SCI_SETMULTIPLESELECTION, 1, 0);

    SendEd(h, SCI_SETADDITIONALSELECTIONTYPING, 1, 0);

    SendEd(h, SCI_SETSCROLLWIDTH, 1, 0);

    SendEd(h, SCI_SETSCROLLWIDTHTRACKING, 1, 0);

    SendEd(h, SCI_SETMARGINLEFT, 0, 8);

    UpdateLineMargin(h);

}



void ApplyThemeToEditor(HWND h) {

    Pal p = GetPalette();



    SendEd(h, SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM)"Consolas");

    SendEd(h, SCI_STYLESETSIZE, STYLE_DEFAULT, 11);

    SendEd(h, SCI_STYLESETBACK, STYLE_DEFAULT, p.bg);

    SendEd(h, SCI_STYLESETFORE, STYLE_DEFAULT, p.fg);

    SendEd(h, SCI_STYLECLEARALL, 0, 0);



    SendEd(h, SCI_SETMARGINTYPEN, MARGIN_LINE_NUMBERS, SC_MARGIN_NUMBER);

    SendEd(h, SCI_STYLESETBACK, STYLE_LINENUMBER, p.lineNumBg);

    SendEd(h, SCI_STYLESETFORE, STYLE_LINENUMBER, p.lineNumFg);



    SendEd(h, SCI_SETMARGINTYPEN, MARGIN_MARKERS, SC_MARGIN_SYMBOL);

    SendEd(h, SCI_SETMARGINWIDTHN, MARGIN_MARKERS, 6);

    SendEd(h, SCI_SETMARGINMASKN, MARGIN_MARKERS, (1 << MARKER_UNSAVED) | (1 << MARKER_SAVED));

    SendEd(h, SCI_SETMARGINBACKN, MARGIN_MARKERS, p.lineNumBg);



    SendEd(h, SCI_MARKERDEFINE, MARKER_UNSAVED, SC_MARK_LEFTRECT);

    SendEd(h, SCI_MARKERSETFORE, MARKER_UNSAVED, RGB(255, 140, 0));

    SendEd(h, SCI_MARKERSETBACK, MARKER_UNSAVED, RGB(255, 140, 0));



    SendEd(h, SCI_MARKERDEFINE, MARKER_SAVED, SC_MARK_LEFTRECT);

    SendEd(h, SCI_MARKERSETFORE, MARKER_SAVED, RGB(46, 204, 113));

    SendEd(h, SCI_MARKERSETBACK, MARKER_SAVED, RGB(46, 204, 113));



    SendEd(h, SCI_SETMARGINTYPEN, MARGIN_FOLD, SC_MARGIN_SYMBOL);

    SendEd(h, SCI_SETMARGINMASKN, MARGIN_FOLD, SC_MASK_FOLDERS);

    SendEd(h, SCI_SETMARGINWIDTHN, MARGIN_FOLD, 14);

    SendEd(h, SCI_SETMARGINSENSITIVEN, MARGIN_FOLD, TRUE);

    SendEd(h, SCI_SETFOLDMARGINCOLOUR, TRUE, p.lineNumBg);

    SendEd(h, SCI_SETFOLDMARGINHICOLOUR, TRUE, p.lineNumBg);



    int foldMarkers[] = {

        SC_MARKNUM_FOLDEROPEN, SC_MARKNUM_FOLDER, SC_MARKNUM_FOLDERSUB,

        SC_MARKNUM_FOLDERTAIL, SC_MARKNUM_FOLDEREND, SC_MARKNUM_FOLDEROPENMID, SC_MARKNUM_FOLDERMIDTAIL

    };

    int foldShapes[] = {

        SC_MARK_BOXMINUS, SC_MARK_BOXPLUS, SC_MARK_VLINE,

        SC_MARK_LCORNER, SC_MARK_BOXPLUSCONNECTED, SC_MARK_BOXMINUSCONNECTED, SC_MARK_TCORNER

    };

    for (int i = 0; i < 7; i++) {

        SendEd(h, SCI_MARKERDEFINE, foldMarkers[i], foldShapes[i]);

        SendEd(h, SCI_MARKERSETFORE, foldMarkers[i], p.lineNumBg);

        SendEd(h, SCI_MARKERSETBACK, foldMarkers[i], p.lineNumFg);

    }



    SendEd(h, SCI_SETCARETFORE, isDarkTheme ? RGB(255, 255, 255) : RGB(0, 0, 0), 0);

    SendEd(h, SCI_SETSELBACK, TRUE, p.sel);

    SendEd(h, SCI_SETCARETLINEBACK, p.caretLine, 0);



    SendEd(h, SCI_STYLESETBACK, STYLE_BRACELIGHT, p.braceBg);

    SendEd(h, SCI_STYLESETFORE, STYLE_BRACELIGHT, p.braceFg);

    SendEd(h, SCI_STYLESETBOLD, STYLE_BRACELIGHT, 1);

    SendEd(h, SCI_STYLESETBACK, STYLE_BRACEBAD, RGB(200, 50, 50));

    SendEd(h, SCI_STYLESETFORE, STYLE_BRACEBAD, RGB(255, 255, 255));

    SendEd(h, SCI_STYLESETFORE, STYLE_INDENTGUIDE, p.guide);

    SendEd(h, SCI_STYLESETBACK, STYLE_INDENTGUIDE, p.bg);



    auto S = [&](int id, COLORREF fg, bool bold = false) {

        SendEd(h, SCI_STYLESETFORE, id, fg);

        if (bold) SendEd(h, SCI_STYLESETBOLD, id, 1);

        };

    S(S_DEFAULT, p.fg);

    S(S_COMMENT, p.comment);

    S(S_BLOCKCOMMENT, p.comment);

    S(S_HTMLCOMMENT, p.comment);

    S(S_STRING, p.str);

    S(S_TRIPLE, p.str);

    S(S_NUMBER, p.num);

    S(S_KEYWORD, p.kw, true);

    S(S_CONTROL, p.ctrl, true);

    S(S_TYPE, p.type);

    S(S_PREPROC, p.pre);

    S(S_OPERATOR, p.op);

    S(S_FUNCTION, p.func);

    S(S_CONST, p.cons);

    S(S_TAG, p.tag, true);

    S(S_ATTR, p.attr);



    UpdateLineMargin(h);

    RelexTab(h);

}



void ApplyThemeToAll() {

    ApplyDwmDarkTitleBar(hwndMain, isDarkTheme);

    for (auto& tab : tabs) {

        ApplyThemeToEditor(tab.hwndEditor);

    }

    COLORREF conBg = isDarkTheme ? RGB(15, 15, 18) : RGB(248, 248, 250);

    SendEd(hwndConsole, SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM)"Consolas");

    SendEd(hwndConsole, SCI_STYLESETSIZE, STYLE_DEFAULT, 10);

    SendEd(hwndConsole, SCI_STYLESETBACK, STYLE_DEFAULT, conBg);

    SendEd(hwndConsole, SCI_STYLECLEARALL, 0, 0);



    SendEd(hwndConsole, SCI_STYLESETFORE, LOG_NORMAL, isDarkTheme ? RGB(180, 180, 195) : RGB(40, 40, 40));

    SendEd(hwndConsole, SCI_STYLESETFORE, LOG_ERROR, RGB(255, 85, 85));

    SendEd(hwndConsole, SCI_STYLESETFORE, LOG_WARNING, RGB(255, 184, 77));

    SendEd(hwndConsole, SCI_STYLESETFORE, LOG_SUCCESS, RGB(80, 220, 120));

    SendEd(hwndConsole, SCI_STYLESETFORE, LOG_ACCENT, RGB(170, 130, 255));



    InvalidateRect(hwndMain, NULL, TRUE);

}



// ============================================================

//  ПАЛИТРА ЦВЕТОВ SOURCEMOD + ГЕНЕРАТОР СНИППЕТОВ (CTRL+M)

// ============================================================

struct ChatColorItem {

    const wchar_t* title;

    const char* insertText;

    COLORREF color;

    RECT rc;

};



std::vector<ChatColorItem> g_chatColors = {

    { L"{default}",      "{default}",    RGB(220, 220, 220), {0,0,0,0} },

    { L"{green}",        "{green}",      RGB(60, 220, 60),   {0,0,0,0} },

    { L"{lightgreen}",   "{lightgreen}", RGB(153, 255, 153), {0,0,0,0} },

    { L"{red}",          "{red}",        RGB(255, 64, 64),   {0,0,0,0} },

    { L"{blue}",         "{blue}",       RGB(153, 204, 255), {0,0,0,0} },

    { L"{olive}",        "{olive}",      RGB(158, 205, 50),  {0,0,0,0} },

    { L"{lime}",         "{lime}",       RGB(180, 255, 80),  {0,0,0,0} },

    { L"{gold}",         "{gold}",       RGB(255, 200, 35),  {0,0,0,0} },

    { L"{orange}",       "{orange}",     RGB(255, 140, 25),  {0,0,0,0} },

    { L"{purple}",       "{purple}",     RGB(185, 110, 255), {0,0,0,0} },

    { L"{cyan}",         "{cyan}",       RGB(60, 225, 255),  {0,0,0,0} },

    { L"{pink}",         "{pink}",       RGB(255, 135, 200), {0,0,0,0} },

    { L"⚡ Команда !cmd",

      "RegConsoleCmd(\"sm_test\", Command_Test);\n\npublic Action Command_Test(int client, int args)\n{\n    if (!client || !IsClientInGame(client)) return Plugin_Handled;\n    CPrintToChat(client, \"{green}[Server]{default} Команда выполнена!\");\n    return Plugin_Handled;\n}\n",

      RGB(160, 120, 255), {0,0,0,0} },

    { L"⚡ Событие Death",

      "HookEvent(\"player_death\", Event_PlayerDeath);\n\npublic void Event_PlayerDeath(Event event, const char[] name, bool dontBroadcast)\n{\n    int victim = GetClientOfUserId(event.GetInt(\"userid\"));\n    int attacker = GetClientOfUserId(event.GetInt(\"attacker\"));\n}\n",

      RGB(160, 120, 255), {0,0,0,0} },

    { L"⚡ Цикл игроков",

      "for (int i = 1; i <= MaxClients; i++)\n{\n    if (IsClientInGame(i) && !IsFakeClient(i) && IsPlayerAlive(i))\n    {\n        \n    }\n}\n",

      RGB(160, 120, 255), {0,0,0,0} }

};



int g_hoverColorIdx = -1;



LRESULT CALLBACK ColorsDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    switch (uMsg) {

    case WM_CREATE: {

        ApplyDwmDarkTitleBar(hwnd, isDarkTheme);

        int idx = 0;

        for (int row = 0; row < 5; row++) {

            for (int col = 0; col < 3; col++) {

                if (idx < (int)g_chatColors.size()) {

                    int x = 18 + col * 142;

                    int y = 50 + row * 42;

                    g_chatColors[idx].rc = { x, y, x + 134, y + 34 };

                    idx++;

                }

            }

        }

        return 0;

    }

    case WM_PAINT: {

        PAINTSTRUCT ps;

        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc;

        GetClientRect(hwnd, &rc);

        Pal pal = GetPalette();



        HBRUSH hbrBg = CreateSolidBrush(isDarkTheme ? RGB(24, 24, 29) : RGB(242, 242, 247));

        FillRect(hdc, &rc, hbrBg);

        DeleteObject(hbrBg);



        RECT rcTop = { 0, 0, rc.right, 3 };

        HBRUSH hbrAcc = CreateSolidBrush(pal.uiAccent);

        FillRect(hdc, &rcTop, hbrAcc);

        DeleteObject(hbrAcc);



        HPEN hPen = CreatePen(PS_SOLID, 1, pal.uiAccent);

        HGDIOBJ oldPen = SelectObject(hdc, hPen);

        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

        Rectangle(hdc, 0, 0, rc.right, rc.bottom);



        SetBkMode(hdc, TRANSPARENT);

        SelectObject(hdc, hUIBoldFont);

        SetTextColor(hdc, isDarkTheme ? RGB(255, 255, 255) : RGB(20, 20, 20));

        RECT rHdr = { 18, 12, rc.right - 40, 40 };

        DrawTextW(hdc, L"🎨 Цвета MoreColors и Быстрые заготовки SourcePawn:", -1, &rHdr, DT_LEFT | DT_VCENTER | DT_SINGLELINE);



        RECT rcCloseX = { rc.right - 34, 8, rc.right - 8, 34 };

        SetTextColor(hdc, RGB(235, 85, 85));

        DrawTextW(hdc, L"✕", -1, &rcCloseX, DT_CENTER | DT_VCENTER | DT_SINGLELINE);



        for (int i = 0; i < (int)g_chatColors.size(); i++) {

            RECT r = g_chatColors[i].rc;

            bool hov = (i == g_hoverColorIdx);

            HBRUSH hCard = CreateSolidBrush(hov ? RGB(52, 52, 64) : RGB(35, 35, 42));

            FillRect(hdc, &r, hCard);

            DeleteObject(hCard);



            RECT rSwatch = { r.left + 8, r.top + 8, r.left + 22, r.bottom - 8 };

            HBRUSH hSw = CreateSolidBrush(g_chatColors[i].color);

            FillRect(hdc, &rSwatch, hSw);

            DeleteObject(hSw);



            SetTextColor(hdc, g_chatColors[i].color);

            RECT rTxt = { r.left + 28, r.top, r.right - 4, r.bottom };

            DrawTextW(hdc, g_chatColors[i].title, -1, &rTxt, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        }



        SelectObject(hdc, oldBr);

        SelectObject(hdc, oldPen);

        DeleteObject(hPen);

        EndPaint(hwnd, &ps);

        return 0;

    }

    case WM_MOUSEMOVE: {

        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

        int hov = -1;

        for (int i = 0; i < (int)g_chatColors.size(); i++) {

            if (PtInRect(&g_chatColors[i].rc, pt)) {

                hov = i;

                break;

            }

        }

        if (hov != g_hoverColorIdx) {

            g_hoverColorIdx = hov;

            InvalidateRect(hwnd, NULL, FALSE);

        }

        return 0;

    }

    case WM_LBUTTONDOWN: {

        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

        RECT rc;

        GetClientRect(hwnd, &rc);

        RECT rcCloseX = { rc.right - 34, 8, rc.right - 8, 34 };

        if (PtInRect(&rcCloseX, pt)) {

            DestroyWindow(hwnd);

            return 0;

        }

        for (int i = 0; i < (int)g_chatColors.size(); i++) {

            if (PtInRect(&g_chatColors[i].rc, pt)) {

                SendActive(SCI_REPLACESEL, 0, (LPARAM)g_chatColors[i].insertText);

                LogToConsoleW((std::wstring(L"> Вставлено в код: ") + g_chatColors[i].title).c_str(), LOG_ACCENT);

                if (currentTab >= 0) {

                    RelexTab(tabs[currentTab].hwndEditor);

                    SetFocus(tabs[currentTab].hwndEditor);

                }

                return 0;

            }

        }

        SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);

        return 0;

    }

    case WM_KEYDOWN:

        if (wParam == VK_ESCAPE) DestroyWindow(hwnd);

        return 0;

    case WM_DESTROY:

        hwndColorsDlg = NULL;

        return 0;

    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);

}



void ShowChatColorsDialog() {

    if (hwndColorsDlg) {

        SetForegroundWindow(hwndColorsDlg);

        return;

    }

    WNDCLASSW wc = { 0 };

    wc.lpfnWndProc = ColorsDlgProc;

    wc.hInstance = hInst;

    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    wc.lpszClassName = L"KovaChatColorsCls";

    RegisterClassW(&wc);



    RECT rcMain;

    GetWindowRect(hwndMain, &rcMain);

    int w = 456, h = 274;

    int x = rcMain.left + (rcMain.right - rcMain.left - w) / 2;

    int y = rcMain.top + 95;

    hwndColorsDlg = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, L"KovaChatColorsCls", L"",

        WS_POPUP | WS_VISIBLE, x, y, w, h, hwndMain, NULL, hInst, NULL);

}



// ============================================================

//  КОМПИЛЯЦИЯ / SMART RUN (F5) + ПРЫЖОК ПО ОШИБКАМ ИЗ КОНСОЛИ

// ============================================================

bool SelectCompilerDlg() {

    OPENFILENAMEW ofn = { sizeof(ofn) };

    wchar_t szFile[MAX_PATH] = L"";

    ofn.hwndOwner = hwndMain;

    ofn.lpstrFilter = L"SourcePawn Compiler (spcomp.exe)\0spcomp.exe\0Все файлы (*.*)\0*.*\0";

    ofn.lpstrFile = szFile;

    ofn.nMaxFile = MAX_PATH;

    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    if (GetOpenFileNameW(&ofn)) {

        wcscpy_s(szCompilerPath, szFile);

        SaveConfig();

        LogToConsoleW((std::wstring(L"> Выбран и сохранен компилятор spcomp.exe: ") + szCompilerPath).c_str(), LOG_SUCCESS);

        InvalidateRect(hwndMain, NULL, FALSE);

        return true;

    }

    return false;

}



void SaveCurrentFile(bool forceSaveAs = false);



void RunSmartDispatcher() {

    if (currentTab < 0) return;

    std::wstring file = tabs[currentTab].filePath;

    if (file.find(L"Новый файл") != std::wstring::npos) {

        LogToConsoleW(L"[WARNING] Сначала сохраните новый файл на диск (выберите имя и расширение).", LOG_WARNING);

        SaveCurrentFile(true);

        file = tabs[currentTab].filePath;

        if (file.find(L"Новый файл") != std::wstring::npos) {

            return;

        }

    }

    if (SendActive(SCI_GETMODIFY)) {

        SaveCurrentFile(false);

    }



    Lang lg = GetLang(file);

    LogToConsoleW(L"────────────────────────────────────────────────────────────", LOG_ACCENT);

    LogToConsoleW((L"> Запуск Smart Run: " + file).c_str(), LOG_ACCENT);



    if (lg == LANG_HTML) {

        LogToConsoleW(L"> Открытие интерактивного веб-просмотра в браузере...", LOG_SUCCESS);

        ShellExecuteW(NULL, L"open", file.c_str(), NULL, NULL, SW_SHOWNORMAL);

        return;

    }



    std::wstring workDir = file;

    size_t lastSlash = workDir.find_last_of(L"\\/");

    if (lastSlash != std::wstring::npos) {

        workDir = workDir.substr(0, lastSlash);

    }



    HANDLE hRead, hWrite;

    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    CreatePipe(&hRead, &hWrite, &sa, 0);

    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);



    STARTUPINFOW si = { sizeof(STARTUPINFOW) };

    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

    si.hStdOutput = hWrite;

    si.hStdError = hWrite;

    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi = { 0 };



    std::wstring cmd;

    if (lg == LANG_PY) {

        cmd = L"python \"" + file + L"\"";

    }

    else if (lg == LANG_SP) {

        if (wcslen(szCompilerPath) == 0 || GetFileAttributesW(szCompilerPath) == INVALID_FILE_ATTRIBUTES) {

            LogToConsoleW(L"[WARNING] Путь к spcomp.exe не указан. Укажите его в открывшемся окне.", LOG_WARNING);

            if (!SelectCompilerDlg()) {

                LogToConsoleW(L"[ОШИБКА] Компиляция отменена (spcomp.exe не выбран).", LOG_ERROR);

                CloseHandle(hWrite);

                CloseHandle(hRead);

                return;

            }

        }

        std::wstring outSmx = file;

        size_t dot = outSmx.find_last_of(L'.');

        if (dot != std::wstring::npos) outSmx = outSmx.substr(0, dot) + L".smx";

        cmd = std::wstring(L"\"") + szCompilerPath + L"\" \"" + file + L"\" -o\"" + outSmx + L"\"";

    }

    else {

        LogToConsoleW(L"[WARNING] Для запуска сохраните файл как .sp, .py или .html!", LOG_WARNING);

        CloseHandle(hWrite);

        CloseHandle(hRead);

        return;

    }



    std::vector<wchar_t> cmdBuf(cmd.begin(), cmd.end());

    cmdBuf.push_back(0);



    if (CreateProcessW(NULL, cmdBuf.data(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, workDir.c_str(), &si, &pi)) {

        CloseHandle(hWrite);

        char buffer[1024];

        DWORD read;

        while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &read, NULL) && read > 0) {

            buffer[read] = 0;

            int wLen = MultiByteToWideChar(CP_ACP, 0, buffer, read, NULL, 0);

            std::wstring wStr(wLen, 0);

            MultiByteToWideChar(CP_ACP, 0, buffer, read, &wStr[0], wLen);



            int lType = LOG_NORMAL;

            if (wcsstr(wStr.c_str(), L"error") || wcsstr(wStr.c_str(), L"fatal")) lType = LOG_ERROR;

            else if (wcsstr(wStr.c_str(), L"warning")) lType = LOG_WARNING;



            LogToConsoleW(wStr.c_str(), lType);

        }

        CloseHandle(hRead);

        CloseHandle(pi.hProcess);

        CloseHandle(pi.hThread);

        LogToConsoleW(L"[SUCCESS] Выполнение завершено (двойной клик по строке с ошибкой перенесет к коду).", LOG_SUCCESS);

    }

    else {

        CloseHandle(hWrite);

        CloseHandle(hRead);

        LogToConsoleW(L"[ОШИБКА] Не удалось запустить процесс. Проверьте пути к компилятору/интерпретатору.", LOG_ERROR);

    }

}



void JumpToErrorFromConsole() {

    int pos = (int)SendEd(hwndConsole, SCI_GETCURRENTPOS);

    int line = (int)SendEd(hwndConsole, SCI_LINEFROMPOSITION, pos);

    int len = (int)SendEd(hwndConsole, SCI_LINELENGTH, line);

    if (len <= 0 || currentTab < 0) return;



    std::vector<char> buf(len + 1, 0);

    SendEd(hwndConsole, SCI_GETLINE, line, (LPARAM)buf.data());

    std::string s = buf.data();



    int targetLine = -1;

    size_t openP = s.find('(');

    size_t closeP = s.find(')', openP == std::string::npos ? 0 : openP);

    if (openP != std::string::npos && closeP != std::string::npos && closeP > openP + 1) {

        std::string numStr = s.substr(openP + 1, closeP - openP - 1);

        bool allDigits = true;

        for (char c : numStr) if (!isdigit((unsigned char)c)) allDigits = false;

        if (allDigits) targetLine = atoi(numStr.c_str());

    }

    if (targetLine < 0) {

        size_t lp = s.find("line ");

        if (lp != std::string::npos) {

            targetLine = atoi(s.c_str() + lp + 5);

        }

    }



    if (targetLine > 0) {

        HWND ed = tabs[currentTab].hwndEditor;

        SendEd(ed, SCI_GOTOLINE, targetLine - 1);

        int ls = (int)SendEd(ed, SCI_POSITIONFROMLINE, targetLine - 1);

        int le = (int)SendEd(ed, SCI_GETLINEENDPOSITION, targetLine - 1);

        SendEd(ed, SCI_SETSEL, ls, le);

        SendEd(ed, SCI_SCROLLCARET);

        SetFocus(ed);

    }

}



// ============================================================

//  КАСТОМНОЕ ТЕМНОЕ ОКНО "ПЕРЕЙТИ К СТРОКЕ" (CTRL+G)

// ============================================================

LRESULT CALLBACK GotoDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    switch (uMsg) {

    case WM_CREATE: {

        ApplyDwmDarkTitleBar(hwnd, isDarkTheme);

        if (hDarkEditBrush) DeleteObject(hDarkEditBrush);

        hDarkEditBrush = CreateSolidBrush(isDarkTheme ? RGB(38, 38, 44) : RGB(255, 255, 255));

        hEditGotoBox = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,

            20, 44, 220, 26, hwnd, (HMENU)201, hInst, NULL);

        SendMessage(hEditGotoBox, WM_SETFONT, (WPARAM)hUIFont, TRUE);

        SetFocus(hEditGotoBox);

        return 0;

    }

    case WM_CTLCOLOREDIT: {

        HDC hdc = (HDC)wParam;

        SetTextColor(hdc, isDarkTheme ? RGB(240, 240, 245) : RGB(10, 10, 10));

        SetBkColor(hdc, isDarkTheme ? RGB(38, 38, 44) : RGB(255, 255, 255));

        return (LRESULT)hDarkEditBrush;

    }

    case WM_PAINT: {

        PAINTSTRUCT ps;

        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc;

        GetClientRect(hwnd, &rc);

        Pal pal = GetPalette();



        HBRUSH hbrBg = CreateSolidBrush(isDarkTheme ? RGB(26, 26, 31) : RGB(242, 242, 247));

        FillRect(hdc, &rc, hbrBg);

        DeleteObject(hbrBg);



        HPEN hPen = CreatePen(PS_SOLID, 1, pal.uiAccent);

        HGDIOBJ oldPen = SelectObject(hdc, hPen);

        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

        Rectangle(hdc, 0, 0, rc.right, rc.bottom);



        SetBkMode(hdc, TRANSPARENT);

        SelectObject(hdc, hUIBoldFont);

        SetTextColor(hdc, isDarkTheme ? RGB(255, 255, 255) : RGB(20, 20, 20));

        RECT rHdr = { 20, 12, rc.right - 20, 38 };

        DrawTextW(hdc, L"Перейти к номеру строки (Ctrl+G):", -1, &rHdr, DT_LEFT | DT_VCENTER | DT_SINGLELINE);



        RECT bGo = { 20, 82, 135, 112 };

        RECT bCancel = { 145, 82, 240, 112 };

        HBRUSH hb1 = CreateSolidBrush(pal.uiAccent);

        FillRect(hdc, &bGo, hb1);

        DeleteObject(hb1);

        SetTextColor(hdc, RGB(255, 255, 255));

        DrawTextW(hdc, L"Перейти", -1, &bGo, DT_CENTER | DT_VCENTER | DT_SINGLELINE);



        HBRUSH hb2 = CreateSolidBrush(isDarkTheme ? RGB(45, 45, 54) : RGB(220, 220, 230));

        FillRect(hdc, &bCancel, hb2);

        DeleteObject(hb2);

        SetTextColor(hdc, isDarkTheme ? RGB(255, 255, 255) : RGB(20, 20, 20));

        DrawTextW(hdc, L"Отмена", -1, &bCancel, DT_CENTER | DT_VCENTER | DT_SINGLELINE);



        SelectObject(hdc, oldBr);

        SelectObject(hdc, oldPen);

        DeleteObject(hPen);

        EndPaint(hwnd, &ps);

        return 0;

    }

    case WM_LBUTTONDOWN: {

        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

        RECT bGo = { 20, 82, 135, 112 };

        RECT bCancel = { 145, 82, 240, 112 };

        if (PtInRect(&bGo, pt)) {

            wchar_t numBuf[64] = { 0 };

            GetWindowTextW(hEditGotoBox, numBuf, 64);

            int line = _wtoi(numBuf);

            if (line > 0 && currentTab >= 0) {

                SendActive(SCI_GOTOLINE, line - 1);

                SendActive(SCI_SCROLLCARET);

                SetFocus(tabs[currentTab].hwndEditor);

            }

            DestroyWindow(hwnd);

            return 0;

        }

        if (PtInRect(&bCancel, pt)) {

            DestroyWindow(hwnd);

            return 0;

        }

        SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);

        return 0;

    }

    case WM_DESTROY:

        hwndGotoDlg = NULL;

        return 0;

    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);

}



void ShowGotoLineDialog() {

    if (hwndGotoDlg) {

        SetForegroundWindow(hwndGotoDlg);

        return;

    }

    WNDCLASSW wc = { 0 };

    wc.lpfnWndProc = GotoDlgProc;

    wc.hInstance = hInst;

    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    wc.lpszClassName = L"KovaGotoCls";

    RegisterClassW(&wc);



    RECT rcMain;

    GetWindowRect(hwndMain, &rcMain);

    int w = 260, h = 128;

    int x = rcMain.left + (rcMain.right - rcMain.left - w) / 2;

    int y = rcMain.top + 120;

    hwndGotoDlg = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, L"KovaGotoCls", L"",

        WS_POPUP | WS_VISIBLE, x, y, w, h, hwndMain, NULL, hInst, NULL);

}



// ============================================================

//  КАСТОМНОЕ ТЕМНОЕ ОКНО ПОИСКА И ЗАМЕНЫ (CTRL+F)

// ============================================================

void DoFindNext() {

    wchar_t buf[256] = { 0 };

    GetWindowTextW(hEditFindBox, buf, 256);

    if (wcslen(buf) == 0) return;

    wcscpy_s(szFindWhat, buf);



    int size = WideCharToMultiByte(CP_UTF8, 0, buf, -1, NULL, 0, NULL, NULL);

    std::vector<char> utf8(size);

    WideCharToMultiByte(CP_UTF8, 0, buf, -1, &utf8[0], size, NULL, NULL);



    int curEnd = (int)SendActive(SCI_GETSELECTIONEND);

    SendActive(SCI_GOTOPOS, curEnd);

    SendActive(SCI_SEARCHANCHOR, 0, 0);

    int pos = (int)SendActive(SCI_SEARCHNEXT, 0, (LPARAM)utf8.data());

    if (pos == -1) {

        SendActive(SCI_GOTOPOS, 0);

        SendActive(SCI_SEARCHANCHOR, 0, 0);

        pos = (int)SendActive(SCI_SEARCHNEXT, 0, (LPARAM)utf8.data());

    }

    if (pos != -1) {

        SendActive(SCI_SCROLLCARET, 0, 0);

    }

    else {

        LogToConsoleW(L"[ПОИСК] Совпадений не найдено.", LOG_WARNING);

    }

}



void DoReplaceOne(bool all) {

    wchar_t fBuf[256] = { 0 }, rBuf[256] = { 0 };

    GetWindowTextW(hEditFindBox, fBuf, 256);

    GetWindowTextW(hEditReplaceBox, rBuf, 256);

    if (wcslen(fBuf) == 0) return;

    wcscpy_s(szFindWhat, fBuf);

    wcscpy_s(szReplaceWith, rBuf);



    int fSize = WideCharToMultiByte(CP_UTF8, 0, fBuf, -1, NULL, 0, NULL, NULL);

    std::vector<char> fUtf8(fSize);

    WideCharToMultiByte(CP_UTF8, 0, fBuf, -1, &fUtf8[0], fSize, NULL, NULL);



    int rSize = WideCharToMultiByte(CP_UTF8, 0, rBuf, -1, NULL, 0, NULL, NULL);

    std::vector<char> rUtf8(rSize);

    WideCharToMultiByte(CP_UTF8, 0, rBuf, -1, &rUtf8[0], rSize, NULL, NULL);



    if (!all) {

        DoFindNext();

        if (SendActive(SCI_GETSELECTIONSTART) != SendActive(SCI_GETSELECTIONEND)) {

            SendActive(SCI_REPLACESEL, 0, (LPARAM)rUtf8.data());

        }

    }

    else {

        int count = 0;

        SendActive(SCI_GOTOPOS, 0);

        while (true) {

            SendActive(SCI_SEARCHANCHOR, 0, 0);

            int pos = (int)SendActive(SCI_SEARCHNEXT, 0, (LPARAM)fUtf8.data());

            if (pos == -1) break;

            SendActive(SCI_REPLACESEL, 0, (LPARAM)rUtf8.data());

            SendActive(SCI_GOTOPOS, pos + (int)strlen(rUtf8.data()));

            count++;

        }

        wchar_t msg[128];

        swprintf(msg, 128, L"[ЗАМЕНА] Заменено совпадений: %d", count);

        LogToConsoleW(msg, LOG_SUCCESS);

    }

}



LRESULT CALLBACK FindDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    switch (uMsg) {

    case WM_CREATE: {

        ApplyDwmDarkTitleBar(hwnd, isDarkTheme);

        if (hDarkEditBrush) DeleteObject(hDarkEditBrush);

        hDarkEditBrush = CreateSolidBrush(isDarkTheme ? RGB(38, 38, 44) : RGB(255, 255, 255));



        hEditFindBox = CreateWindowExW(0, L"EDIT", szFindWhat, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,

            110, 48, 280, 24, hwnd, (HMENU)101, hInst, NULL);

        hEditReplaceBox = CreateWindowExW(0, L"EDIT", szReplaceWith, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,

            110, 84, 280, 24, hwnd, (HMENU)102, hInst, NULL);



        SendMessage(hEditFindBox, WM_SETFONT, (WPARAM)hUIFont, TRUE);

        SendMessage(hEditReplaceBox, WM_SETFONT, (WPARAM)hUIFont, TRUE);

        SetFocus(hEditFindBox);

        return 0;

    }

    case WM_CTLCOLOREDIT: {

        HDC hdc = (HDC)wParam;

        SetTextColor(hdc, isDarkTheme ? RGB(240, 240, 245) : RGB(10, 10, 10));

        SetBkColor(hdc, isDarkTheme ? RGB(38, 38, 44) : RGB(255, 255, 255));

        return (LRESULT)hDarkEditBrush;

    }

    case WM_PAINT: {

        PAINTSTRUCT ps;

        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc;

        GetClientRect(hwnd, &rc);

        Pal pal = GetPalette();



        HBRUSH hbrBg = CreateSolidBrush(isDarkTheme ? RGB(26, 26, 31) : RGB(242, 242, 247));

        FillRect(hdc, &rc, hbrBg);

        DeleteObject(hbrBg);



        RECT rcTop = { 0, 0, rc.right, 3 };

        HBRUSH hbrAcc = CreateSolidBrush(pal.uiAccent);

        FillRect(hdc, &rcTop, hbrAcc);

        DeleteObject(hbrAcc);



        HPEN hPen = CreatePen(PS_SOLID, 1, pal.uiAccent);

        HGDIOBJ oldPen = SelectObject(hdc, hPen);

        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

        Rectangle(hdc, 0, 0, rc.right, rc.bottom);



        SetBkMode(hdc, TRANSPARENT);

        SelectObject(hdc, hUIBoldFont);

        SetTextColor(hdc, isDarkTheme ? RGB(255, 255, 255) : RGB(20, 20, 20));

        RECT rcHdr = { 16, 12, rc.right - 20, 36 };

        DrawTextW(hdc, L"Найти и заменить (Kova Search)", -1, &rcHdr, DT_LEFT | DT_VCENTER | DT_SINGLELINE);



        SelectObject(hdc, hUIFont);

        SetTextColor(hdc, isDarkTheme ? RGB(190, 190, 205) : RGB(60, 60, 70));

        RECT r1 = { 16, 48, 105, 72 };

        DrawTextW(hdc, L"Что искать:", -1, &r1, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        RECT r2 = { 16, 84, 105, 108 };

        DrawTextW(hdc, L"Заменить на:", -1, &r2, DT_LEFT | DT_VCENTER | DT_SINGLELINE);



        struct Btn { RECT r; const wchar_t* txt; bool accent; };

        Btn btns[] = {

            { { 16, 126, 118, 156 },  L"Найти далее", true },

            { { 126, 126, 224, 156 }, L"Заменить",    false },

            { { 232, 126, 330, 156 }, L"Заменить всё",false },

            { { 338, 126, 396, 156 }, L"Закрыть",     false }

        };

        for (auto& b : btns) {

            HBRUSH hb = CreateSolidBrush(b.accent ? pal.uiAccent : (isDarkTheme ? RGB(45, 45, 54) : RGB(220, 220, 230)));

            FillRect(hdc, &b.r, hb);

            DeleteObject(hb);

            SetTextColor(hdc, (b.accent || isDarkTheme) ? RGB(255, 255, 255) : RGB(20, 20, 20));

            DrawTextW(hdc, b.txt, -1, &b.r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        }



        SelectObject(hdc, oldBr);

        SelectObject(hdc, oldPen);

        DeleteObject(hPen);

        EndPaint(hwnd, &ps);

        return 0;

    }

    case WM_LBUTTONDOWN: {

        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

        RECT b1 = { 16, 126, 118, 156 };

        RECT b2 = { 126, 126, 224, 156 };

        RECT b3 = { 232, 126, 330, 156 };

        RECT b4 = { 338, 126, 396, 156 };

        if (PtInRect(&b1, pt)) { DoFindNext(); return 0; }

        if (PtInRect(&b2, pt)) { DoReplaceOne(false); return 0; }

        if (PtInRect(&b3, pt)) { DoReplaceOne(true); return 0; }

        if (PtInRect(&b4, pt)) { DestroyWindow(hwnd); return 0; }

        SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);

        return 0;

    }

    case WM_DESTROY:

        hwndFindDlg = NULL;

        return 0;

    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);

}



void ShowCustomFindDialog() {

    if (hwndFindDlg) {

        SetForegroundWindow(hwndFindDlg);

        return;

    }

    WNDCLASSW wc = { 0 };

    wc.lpfnWndProc = FindDlgProc;

    wc.hInstance = hInst;

    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    wc.lpszClassName = L"KovaCustomFindCls";

    RegisterClassW(&wc);



    RECT rcMain;

    GetWindowRect(hwndMain, &rcMain);

    int w = 412, h = 172;

    int x = rcMain.right - w - 30;

    int y = rcMain.top + 85;



    hwndFindDlg = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, L"KovaCustomFindCls", L"",

        WS_POPUP | WS_VISIBLE, x, y, w, h, hwndMain, NULL, hInst, NULL);

}



// ============================================================

//  ОКНО "О ПРОГРАММЕ" (С ТВОЕЙ LOGO.ICO И БЕЗ ЛИШНИХ ССЫЛОК)

// ============================================================

bool g_hoverAboutCloseBtn = false;



LRESULT CALLBACK AboutWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    switch (uMsg) {

    case WM_PAINT: {

        PAINTSTRUCT ps;

        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc;

        GetClientRect(hwnd, &rc);



        TRIVERTEX vertex[2];

        if (isDarkTheme) {

            vertex[0] = { 0, 0, 0x1400, 0x1400, 0x1B00, 0 };

            vertex[1] = { rc.right, rc.bottom, 0x2200, 0x1F00, 0x2D00, 0 };

        }

        else {

            vertex[0] = { 0, 0, 0xF500, 0xF500, 0xFA00, 0 };

            vertex[1] = { rc.right, rc.bottom, 0xE600, 0xE800, 0xF200, 0 };

        }

        GRADIENT_RECT gRect = { 0, 1 };

        GradientFill(hdc, vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_V);



        Pal pal = GetPalette();

        RECT rcTopBar = { 0, 0, rc.right, 4 };

        HBRUSH hbrAcc = CreateSolidBrush(pal.uiAccent);

        FillRect(hdc, &rcTopBar, hbrAcc);

        DeleteObject(hbrAcc);



        HPEN hPen = CreatePen(PS_SOLID, 1, pal.uiAccent);

        HGDIOBJ oldPen = SelectObject(hdc, hPen);

        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

        Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);

        SelectObject(hdc, oldBrush);

        SelectObject(hdc, oldPen);

        DeleteObject(hPen);



        if (hGlobalAppIconBig) {

            DrawIconEx(hdc, 26, 22, hGlobalAppIconBig, 48, 48, 0, NULL, DI_NORMAL);

        }



        SetBkMode(hdc, TRANSPARENT);



        SetTextColor(hdc, isDarkTheme ? RGB(255, 255, 255) : RGB(20, 20, 25));

        HGDIOBJ oldFnt = SelectObject(hdc, hTitleFont);

        RECT rtTitle = { 86, 20, rc.right - 28, 56 };

        DrawTextW(hdc, L"Kova Code Studio v1.0.0", -1, &rtTitle, DT_LEFT | DT_SINGLELINE);



        SelectObject(hdc, hUIBoldFont);

        SetTextColor(hdc, RGB(255, 155, 45));

        RECT rtSub = { 86, 52, rc.right - 28, 78 };

        DrawTextW(hdc, L"Release Build 1.0.0  |  Разработчик: ANR1S", -1, &rtSub, DT_LEFT | DT_SINGLELINE);



        SelectObject(hdc, hUIFont);

        SetTextColor(hdc, isDarkTheme ? RGB(205, 205, 218) : RGB(50, 50, 60));

        RECT rtDesc = { 28, 92, rc.right - 28, rc.bottom - 65 };

        const wchar_t* aboutText =

            L"Молниеносная нативная среда разработки (C++ Win32 API / Scintilla).\n"

            L"Создана специально для профессионального скриптинга SourcePawn (CS:S v34),\n"

            L"разработки Python-утилит и верстки Web-модулей.\n\n"

            L"Ключевые возможности:\n"

            L" • Собственный высокоскоростной лексер с палитрой VS Code Dark+ и Code Folding\n"

            L" • Smart Run (F5): сборка в .smx рядом с исходником + прыжок к ошибке по клику\n"

            L" • Встроенная палитра MoreColors + генератор кода (Ctrl+M) и сохранение сессии\n"

            L" • Git-индикаторы модификации строк (оранжевый — изменено, зеленый — сохранено)\n\n"

            L"Список изменений v1.0.0 Release:\n"

            L" • Плавное масштабирование окна без артефактов (DeferWindowPos)\n"

            L" • Нативный рендер вкладок с меню ПКМ и сохранение открытых файлов при выходе";

        DrawTextW(hdc, aboutText, -1, &rtDesc, DT_LEFT | DT_WORDBREAK);



        RECT rcBtn = { (rc.right - 160) / 2, rc.bottom - 52, (rc.right + 160) / 2, rc.bottom - 18 };

        HBRUSH hbrBtn = CreateSolidBrush(g_hoverAboutCloseBtn ? RGB(145, 80, 255) : pal.uiAccent);

        FillRect(hdc, &rcBtn, hbrBtn);

        DeleteObject(hbrBtn);



        SelectObject(hdc, hUIBoldFont);

        SetTextColor(hdc, RGB(255, 255, 255));

        DrawTextW(hdc, L"Закрыть окно", -1, &rcBtn, DT_CENTER | DT_VCENTER | DT_SINGLELINE);



        SelectObject(hdc, oldFnt);

        EndPaint(hwnd, &ps);

        return 0;

    }

    case WM_MOUSEMOVE: {

        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

        RECT rc;

        GetClientRect(hwnd, &rc);

        RECT rcBtn = { (rc.right - 160) / 2, rc.bottom - 52, (rc.right + 160) / 2, rc.bottom - 18 };

        bool hov = PtInRect(&rcBtn, pt) != 0;

        if (hov != g_hoverAboutCloseBtn) {

            g_hoverAboutCloseBtn = hov;

            InvalidateRect(hwnd, &rcBtn, FALSE);

        }

        return 0;

    }

    case WM_LBUTTONDOWN: {

        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

        RECT rc;

        GetClientRect(hwnd, &rc);

        RECT rcBtn = { (rc.right - 160) / 2, rc.bottom - 52, (rc.right + 160) / 2, rc.bottom - 18 };

        if (PtInRect(&rcBtn, pt)) {

            DestroyWindow(hwnd);

            return 0;

        }

        SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);

        return 0;

    }

    case WM_KEYDOWN: {

        if (wParam == VK_ESCAPE || wParam == VK_RETURN) DestroyWindow(hwnd);

        return 0;

    }

    case WM_DESTROY:

        hwndAboutDlg = NULL;

        return 0;

    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);

}



void ShowAboutWindow() {

    if (hwndAboutDlg) {

        SetForegroundWindow(hwndAboutDlg);

        return;

    }

    WNDCLASSW wc = { 0 };

    wc.lpfnWndProc = AboutWndProc;

    wc.hInstance = hInst;

    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    wc.lpszClassName = L"KovaAboutClassV1";

    RegisterClassW(&wc);



    int w = 560, h = 435;

    RECT rcMain;

    GetWindowRect(hwndMain, &rcMain);

    int x = rcMain.left + (rcMain.right - rcMain.left - w) / 2;

    int y = rcMain.top + (rcMain.bottom - rcMain.top - h) / 2;



    hwndAboutDlg = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, L"KovaAboutClassV1", L"",

        WS_POPUP | WS_VISIBLE, x, y, w, h, hwndMain, NULL, hInst, NULL);

}



// ============================================================

//  МЕНЮ И ТУЛБАР БЫСТРЫХ ДЕЙСТВИЙ

// ============================================================

void InitToolbarButtons() {

    g_toolbarButtons.clear();

    g_toolbarButtons.push_back({ IDM_FILE_NEW,          L"+ Новый файл",       { 10,  5, 112, 32 }, false });

    g_toolbarButtons.push_back({ IDM_FILE_OPEN,         L"Открыть",            { 118, 5, 198, 32 }, false });

    g_toolbarButtons.push_back({ IDM_FILE_SAVE,         L"Сохранить",          { 204, 5, 294, 32 }, false });

    g_toolbarButtons.push_back({ IDM_RUN_SMART,         L"▶ Запуск / Сборка",  { 304, 5, 448, 32 }, true });

    g_toolbarButtons.push_back({ IDM_SET_COMPILER,      L"⚙ spcomp.exe",       { 456, 5, 562, 32 }, false });

    g_toolbarButtons.push_back({ IDM_EDIT_SNIPPET_SP,   L"★ Шаблон .SP",       { 570, 5, 678, 32 }, false });

    g_toolbarButtons.push_back({ IDM_EDIT_CHAT_COLORS,  L"🎨 Цвета чата",      { 686, 5, 796, 32 }, false });

    g_toolbarButtons.push_back({ IDM_EDIT_FIND_REPLACE, L"🔍 Поиск",           { 804, 5, 880, 32 }, false });

    g_toolbarButtons.push_back({ IDM_THEME_TOGGLE,      L"◐ Тема",             { 888, 5, 956, 32 }, false });

}



void RebuildMenu() {

    if (!hwndMain) return;

    HMENU hOld = GetMenu(hwndMain);

    HMENU hMenu = CreateMenu();



    HMENU hFile = CreatePopupMenu();

    AppendMenuW(hFile, MF_STRING, IDM_FILE_NEW, loc[currentLang][1]);

    AppendMenuW(hFile, MF_STRING, IDM_FILE_OPEN, loc[currentLang][2]);

    AppendMenuW(hFile, MF_STRING, IDM_FILE_SAVE, loc[currentLang][3]);

    AppendMenuW(hFile, MF_STRING, IDM_FILE_SAVEAS, loc[currentLang][4]);

    AppendMenuW(hFile, MF_SEPARATOR, 0, NULL);

    AppendMenuW(hFile, MF_STRING, IDM_FILE_OPEN_FOLDER, loc[currentLang][5]);

    AppendMenuW(hFile, MF_SEPARATOR, 0, NULL);

    AppendMenuW(hFile, MF_STRING, IDM_FILE_EXIT, loc[currentLang][6]);



    HMENU hEdit = CreatePopupMenu();

    AppendMenuW(hEdit, MF_STRING, IDM_EDIT_FIND_REPLACE, loc[currentLang][8]);

    AppendMenuW(hEdit, MF_STRING, IDM_EDIT_GOTO_LINE, loc[currentLang][9]);

    AppendMenuW(hEdit, MF_STRING, IDM_EDIT_DUPLICATE, loc[currentLang][10]);

    AppendMenuW(hEdit, MF_STRING, IDM_EDIT_COMMENT, L"Закомментировать строку\tCtrl+/");

    AppendMenuW(hEdit, MF_STRING, IDM_EDIT_LINE_UP, L"Переместить строку вверх\tAlt+Up");

    AppendMenuW(hEdit, MF_STRING, IDM_EDIT_LINE_DOWN, L"Переместить строку вниз\tAlt+Down");

    AppendMenuW(hEdit, MF_SEPARATOR, 0, NULL);

    AppendMenuW(hEdit, MF_STRING, IDM_EDIT_SNIPPET_SP, loc[currentLang][11]);

    AppendMenuW(hEdit, MF_STRING, IDM_EDIT_CHAT_COLORS, L"Палитра цветов чата SourceMod\tCtrl+M");



    HMENU hBuild = CreatePopupMenu();

    AppendMenuW(hBuild, MF_STRING, IDM_RUN_SMART, loc[currentLang][13]);

    AppendMenuW(hBuild, MF_SEPARATOR, 0, NULL);

    AppendMenuW(hBuild, MF_STRING, IDM_SET_COMPILER, loc[currentLang][14]);

    AppendMenuW(hBuild, MF_STRING, IDM_CLEAR_CONSOLE, L"Очистить консоль");



    HMENU hSettings = CreatePopupMenu();

    HMENU hTheme = CreatePopupMenu();

    AppendMenuW(hTheme, MF_STRING | (isDarkTheme ? MF_CHECKED : MF_UNCHECKED), IDM_THEME_DARK, L"Темная (Dark+)");

    AppendMenuW(hTheme, MF_STRING | (!isDarkTheme ? MF_CHECKED : MF_UNCHECKED), IDM_THEME_LIGHT, L"Светлая (Light+)");



    HMENU hLang = CreatePopupMenu();

    AppendMenuW(hLang, MF_STRING | (currentLang == 0 ? MF_CHECKED : MF_UNCHECKED), IDM_LANG_RU, L"Русский");

    AppendMenuW(hLang, MF_STRING | (currentLang == 1 ? MF_CHECKED : MF_UNCHECKED), IDM_LANG_EN, L"English");

    AppendMenuW(hLang, MF_STRING | (currentLang == 2 ? MF_CHECKED : MF_UNCHECKED), IDM_LANG_DE, L"Deutsch");

    AppendMenuW(hLang, MF_STRING | (currentLang == 3 ? MF_CHECKED : MF_UNCHECKED), IDM_LANG_FR, L"Français");

    AppendMenuW(hLang, MF_STRING | (currentLang == 4 ? MF_CHECKED : MF_UNCHECKED), IDM_LANG_IT, L"Italiano");



    AppendMenuW(hSettings, MF_POPUP, (UINT_PTR)hTheme, L"Тема оформления");

    AppendMenuW(hSettings, MF_POPUP, (UINT_PTR)hLang, L"Language / Язык");

    AppendMenuW(hSettings, MF_SEPARATOR, 0, NULL);

    AppendMenuW(hSettings, MF_STRING, IDM_ABOUT, L"О программе Kova Code (v1.0.0)");



    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hFile, loc[currentLang][0]);

    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hEdit, loc[currentLang][7]);

    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hBuild, loc[currentLang][12]);

    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hSettings, loc[currentLang][15]);



    SetMenu(hwndMain, hMenu);

    DrawMenuBar(hwndMain);

    if (hOld) DestroyMenu(hOld);

}



// ============================================================

//  СОБСТВЕННЫЙ ДВИЖОК ВКЛАДОК И БЕСШОВНОГО РЕСАЙЗА БЕЗ АРТЕФАКТОВ

// ============================================================

void RecalcCustomTabRects() {

    RECT rc;

    GetClientRect(hwndMain, &rc);

    int topY = g_toolbarHeight;

    int bottomY = topY + g_tabHeight;

    int n = (int)tabs.size();

    if (n == 0) {

        g_rcPlusBtn = { 8, topY + 4, 36, bottomY - 4 };

        return;

    }



    int availW = rc.right - 48;

    if (availW < 100) availW = 100;

    int tabW = 200;

    if (n * tabW > availW) {

        tabW = availW / n;

        if (tabW < 75) tabW = 75;

    }



    int x = 0;

    for (int i = 0; i < n; i++) {

        tabs[i].rcTab = { x, topY, x + tabW, bottomY };

        tabs[i].rcClose = { x + tabW - 26, topY + 6, x + tabW - 4, bottomY - 6 };

        x += tabW;

    }

    g_rcPlusBtn = { x + 6, topY + 4, x + 34, bottomY - 4 };

}



void ResizeLayout() {

    if (!hwndConsole) return;

    RECT rc;

    GetClientRect(hwndMain, &rc);

    if (rc.right <= 0 || rc.bottom <= 0) return;



    RecalcCustomTabRects();



    int editorTop = g_toolbarHeight + g_tabHeight;

    int maxConH = rc.bottom - editorTop - g_consoleHeaderHeight - g_statusHeight - 80;

    if (maxConH < 60) maxConH = 60;

    if (g_consoleHeight > maxConH) g_consoleHeight = maxConH;

    if (g_consoleHeight < 60) g_consoleHeight = 60;



    int consoleBottom = rc.bottom - g_statusHeight;

    int consoleTop = consoleBottom - g_consoleHeight;

    int consoleHeaderTop = consoleTop - g_consoleHeaderHeight;

    int editorHeight = consoleHeaderTop - editorTop;

    if (editorHeight < 40) editorHeight = 40;



    // Синхронное изменение размеров окон за один проход без разрывов при растягивании

    HDWP hdwp = BeginDeferWindowPos(2);

    if (hdwp) {

        hdwp = DeferWindowPos(hdwp, hwndConsole, NULL, 0, consoleTop, rc.right, g_consoleHeight,

            SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);

        if (currentTab >= 0 && currentTab < (int)tabs.size()) {

            hdwp = DeferWindowPos(hdwp, tabs[currentTab].hwndEditor, NULL, 0, editorTop, rc.right, editorHeight,

                SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_SHOWWINDOW);

        }

        EndDeferWindowPos(hdwp);

    }



    for (int i = 0; i < (int)tabs.size(); i++) {

        if (i != currentTab) {

            ShowWindow(tabs[i].hwndEditor, SW_HIDE);

        }

    }



    RedrawWindow(hwndMain, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);

}



void SwitchTab(int index) {

    if (index >= 0 && index < (int)tabs.size()) {

        currentTab = index;

        ResizeLayout();

        SetFocus(tabs[currentTab].hwndEditor);

    }

}



void CreateOrReplaceTab(const wchar_t* path = L"Новый файл.txt");



void CloseTab(int index) {

    if (index < 0 || index >= (int)tabs.size()) return;

    DestroyWindow(tabs[index].hwndEditor);

    tabs.erase(tabs.begin() + index);

    if (tabs.empty()) {

        currentTab = -1;

        CreateOrReplaceTab(L"Новый файл.txt");

    }

    else {

        if (currentTab >= (int)tabs.size()) currentTab = (int)tabs.size() - 1;

        SwitchTab(currentTab);

    }

}



void CloseOtherTabs(int keepIndex) {

    if (keepIndex < 0 || keepIndex >= (int)tabs.size()) return;

    TabData keep = tabs[keepIndex];

    for (int i = 0; i < (int)tabs.size(); i++) {

        if (i != keepIndex) DestroyWindow(tabs[i].hwndEditor);

    }

    tabs.clear();

    tabs.push_back(keep);

    currentTab = 0;

    SwitchTab(0);

}



void CreateOrReplaceTab(const wchar_t* path) {

    if (wcscmp(path, L"Новый файл.txt") != 0 && tabs.size() == 1 && tabs[0].filePath == L"Новый файл.txt") {

        if (SendMessage(tabs[0].hwndEditor, SCI_GETLENGTH, 0, 0) == 0 && !tabs[0].isModified) {

            DestroyWindow(tabs[0].hwndEditor);

            tabs.clear();

            currentTab = -1;

        }

    }



    HWND hEd = CreateWindowExW(0, L"Scintilla", L"", WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, 0, 0, 0, hwndMain, NULL, hInst, NULL);

    SetupEditor(hEd);

    TabData td = { hEd, path, GetLang(path), false, {0,0,0,0}, {0,0,0,0} };

    tabs.push_back(td);

    ApplyThemeToEditor(hEd);

    SwitchTab((int)tabs.size() - 1);

}



// ============================================================

//  РАБОТА С ФАЙЛАМИ И ВОССТАНОВЛЕНИЕ СЕССИИ

// ============================================================

bool LoadFileToActive(const wchar_t* path) {

    FILE* fp;

    if (_wfopen_s(&fp, path, L"rb") != 0 || !fp) return false;

    fseek(fp, 0, SEEK_END);

    long size = ftell(fp);

    fseek(fp, 0, SEEK_SET);

    if (size < 0) {

        fclose(fp);

        return false;

    }

    std::vector<char> buffer((size_t)size + 1, 0);

    size_t rd = fread(buffer.data(), 1, (size_t)size, fp);

    fclose(fp);

    buffer[rd] = 0;



    const char* text = buffer.data();

    if (rd >= 3 && (unsigned char)text[0] == 0xEF && (unsigned char)text[1] == 0xBB && (unsigned char)text[2] == 0xBF) {

        text += 3;

    }

    SendActive(SCI_SETTEXT, 0, (LPARAM)text);

    SendActive(SCI_EMPTYUNDOBUFFER, 0, 0);

    SendActive(SCI_MARKERDELETEALL, (WPARAM)-1, 0);

    SendActive(SCI_SETSAVEPOINT, 0, 0);

    if (currentTab >= 0) {

        tabs[currentTab].isModified = false;

        UpdateLineMargin(tabs[currentTab].hwndEditor);

    }

    InvalidateRect(hwndMain, NULL, FALSE);

    return true;

}



void OpenPath(const wchar_t* path) {

    for (int i = 0; i < (int)tabs.size(); i++) {

        if (_wcsicmp(tabs[i].filePath.c_str(), path) == 0) {

            SwitchTab(i);

            return;

        }

    }

    CreateOrReplaceTab(path);

    if (LoadFileToActive(path)) {

        LogToConsoleW((std::wstring(L"> Открыт файл: ") + path).c_str(), LOG_SUCCESS);

    }

    else {

        LogToConsoleW(L"[ОШИБКА] Не удалось прочитать файл.", LOG_ERROR);

    }

}



void RestoreSavedSession() {

    int count = GetPrivateProfileIntW(L"Session", L"Count", 0, szIniPath);

    int opened = 0;

    for (int i = 0; i < count && i < 20; i++) {

        wchar_t key[32], path[MAX_PATH] = { 0 };

        swprintf(key, 32, L"Tab%d", i);

        GetPrivateProfileStringW(L"Session", key, L"", path, MAX_PATH, szIniPath);

        if (wcslen(path) > 0 && GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES) {

            OpenPath(path);

            opened++;

        }

    }

    if (opened == 0) {

        CreateOrReplaceTab(L"Новый файл.txt");

    }

    else {

        int act = GetPrivateProfileIntW(L"Session", L"ActiveTab", 0, szIniPath);

        if (act >= 0 && act < (int)tabs.size()) SwitchTab(act);

    }

}



void OpenFileDlg() {

    OPENFILENAMEW ofn = { sizeof(ofn) };

    wchar_t szFile[MAX_PATH] = L"";

    ofn.hwndOwner = hwndMain;

    ofn.lpstrFilter =

        L"Все поддерживаемые (*.txt;*.sp;*.inc;*.py;*.html;*.cpp)\0*.txt;*.sp;*.inc;*.py;*.html;*.php;*.cpp;*.h\0"

        L"SourcePawn (*.sp;*.inc)\0*.sp;*.inc\0"

        L"Текстовые файлы (*.txt)\0*.txt\0"

        L"Python (*.py)\0*.py\0"

        L"Web (*.html;*.php)\0*.html;*.php\0"

        L"Все файлы (*.*)\0*.*\0";

    ofn.lpstrFile = szFile;

    ofn.nMaxFile = MAX_PATH;

    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    if (GetOpenFileNameW(&ofn)) {

        OpenPath(szFile);

    }

}



void SaveCurrentFile(bool forceSaveAs) {

    if (currentTab < 0) return;

    std::wstring file = tabs[currentTab].filePath;



    if (forceSaveAs || file.find(L"Новый файл") != std::wstring::npos) {

        OPENFILENAMEW ofn = { sizeof(ofn) };

        wchar_t szFile[MAX_PATH] = L"Новый файл.txt";

        if (file.find(L"Новый файл") == std::wstring::npos) {

            wcscpy_s(szFile, file.c_str());

        }

        ofn.hwndOwner = hwndMain;

        ofn.lpstrFilter =

            L"Текстовый документ (*.txt)\0*.txt\0"

            L"Плагин SourcePawn (*.sp)\0*.sp\0"

            L"Инклюд SourcePawn (*.inc)\0*.inc\0"

            L"Скрипт Python (*.py)\0*.py\0"

            L"Веб-страница HTML (*.html)\0*.html\0"

            L"Исходник C/C++ (*.cpp;*.h)\0*.cpp;*.h\0"

            L"Все файлы (*.* — можно вписать любое расширение)\0*.*\0";

        ofn.nFilterIndex = 1;

        ofn.lpstrFile = szFile;

        ofn.nMaxFile = MAX_PATH;

        ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;



        if (!GetSaveFileNameW(&ofn)) return;



        file = szFile;

        size_t slash = file.find_last_of(L"\\/");

        size_t dot = file.find_last_of(L'.');

        if (dot == std::wstring::npos || (slash != std::wstring::npos && dot < slash)) {

            switch (ofn.nFilterIndex) {

            case 1: file += L".txt";  break;

            case 2: file += L".sp";   break;

            case 3: file += L".inc";  break;

            case 4: file += L".py";   break;

            case 5: file += L".html"; break;

            case 6: file += L".cpp";  break;

            default: break;

            }

        }



        tabs[currentTab].filePath = file;

        tabs[currentTab].lang = GetLang(file);

        RelexTab(tabs[currentTab].hwndEditor);

    }



    long len = (long)SendActive(SCI_GETLENGTH);

    std::vector<char> buffer((size_t)len + 1, 0);

    SendActive(SCI_GETTEXT, len + 1, (LPARAM)buffer.data());



    FILE* fp;

    if (_wfopen_s(&fp, file.c_str(), L"wb") == 0 && fp) {

        fwrite(buffer.data(), 1, len, fp);

        fclose(fp);

        LogToConsoleW((std::wstring(L"> Файл успешно сохранен: ") + file).c_str(), LOG_SUCCESS);

        SendActive(SCI_SETSAVEPOINT, 0, 0);

        tabs[currentTab].isModified = false;



        int lines = (int)SendActive(SCI_GETLINECOUNT);

        for (int i = 0; i < lines; i++) {

            if (SendActive(SCI_MARKERGET, i) & (1 << MARKER_UNSAVED)) {

                SendActive(SCI_MARKERDELETE, i, MARKER_UNSAVED);

                SendActive(SCI_MARKERADD, i, MARKER_SAVED);

            }

        }

        SaveConfig();

        InvalidateRect(hwndMain, NULL, FALSE);

    }

    else {

        LogToConsoleW(L"[ОШИБКА] Не удалось сохранить файл на диск.", LOG_ERROR);

    }

}



// ============================================================

//  ОТРИСОВКА ТУЛБАРА, КАСТОМНОЙ ПОЛОСЫ ВКЛАДОК И СТАТУС-БАРА

// ============================================================

void PaintMainWindowChrome(HWND hwnd, HDC hdcReal) {

    RECT rc;

    GetClientRect(hwnd, &rc);

    if (rc.right <= 0 || rc.bottom <= 0) return;



    HDC hdc = CreateCompatibleDC(hdcReal);

    HBITMAP hbm = CreateCompatibleBitmap(hdcReal, rc.right, rc.bottom);

    HBITMAP oldBm = (HBITMAP)SelectObject(hdc, hbm);



    Pal pal = GetPalette();

    SetBkMode(hdc, TRANSPARENT);

    RecalcCustomTabRects();



    // Базовая заливка всего клиентского фона (предотвращает любые артефакты при растягивании)

    HBRUSH hbrBase = CreateSolidBrush(pal.bg);

    FillRect(hdc, &rc, hbrBase);

    DeleteObject(hbrBase);



    // 1. ВЕРХНИЙ ТУЛБАР БЫСТРОГО ДОСТУПА

    RECT rcToolbar = { 0, 0, rc.right, g_toolbarHeight };

    HBRUSH hbrTop = CreateSolidBrush(pal.uiTopBar);

    FillRect(hdc, &rcToolbar, hbrTop);

    DeleteObject(hbrTop);



    SelectObject(hdc, hUIFont);

    for (size_t i = 0; i < g_toolbarButtons.size(); i++) {

        const auto& b = g_toolbarButtons[i];

        bool hov = ((int)i == g_hoverToolbarBtn);

        COLORREF btnBg;

        if (b.isAccent) {

            btnBg = hov ? RGB(145, 80, 255) : pal.uiAccent;

        }

        else {

            btnBg = hov ? (isDarkTheme ? RGB(52, 52, 62) : RGB(210, 210, 222))

                : (isDarkTheme ? RGB(38, 38, 45) : RGB(225, 225, 234));

        }

        HBRUSH hbrB = CreateSolidBrush(btnBg);

        FillRect(hdc, &b.rc, hbrB);

        DeleteObject(hbrB);



        SetTextColor(hdc, (b.isAccent || isDarkTheme) ? RGB(255, 255, 255) : RGB(25, 25, 30));

        DrawTextW(hdc, b.label, -1, (LPRECT)&b.rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    }



    if (rc.right > 1120) {

        SelectObject(hdc, hUIBoldFont);

        SetTextColor(hdc, isDarkTheme ? RGB(145, 145, 165) : RGB(100, 100, 120));

        RECT rcBadge = { rc.right - 210, 0, rc.right - 14, g_toolbarHeight };

        DrawTextW(hdc, L"Kova Code v1.0.0  •  ANR1S", -1, &rcBadge, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

    }



    // 2. ПОЛОСА ВКЛАДОК (НАТИВНЫЙ РЕНДЕР)

    RECT rcTabBar = { 0, g_toolbarHeight, rc.right, g_toolbarHeight + g_tabHeight };

    HBRUSH hbrTabBar = CreateSolidBrush(pal.uiTabBar);

    FillRect(hdc, &rcTabBar, hbrTabBar);

    DeleteObject(hbrTabBar);



    HPEN hBorderPen = CreatePen(PS_SOLID, 1, pal.uiBorder);

    HPEN oldPen = (HPEN)SelectObject(hdc, hBorderPen);

    MoveToEx(hdc, 0, rcTabBar.bottom - 1, NULL);

    LineTo(hdc, rc.right, rcTabBar.bottom - 1);



    for (int i = 0; i < (int)tabs.size(); i++) {

        RECT r = tabs[i].rcTab;

        bool isSel = (i == currentTab);

        bool isHov = (i == g_hoverTabIdx);



        COLORREF tBg = isSel ? pal.uiTabActive : (isHov ? pal.uiTabHover : pal.uiTabInactive);

        HBRUSH hbrT = CreateSolidBrush(tBg);

        FillRect(hdc, &r, hbrT);

        DeleteObject(hbrT);



        MoveToEx(hdc, r.right - 1, r.top + 4, NULL);

        LineTo(hdc, r.right - 1, r.bottom - 4);



        if (isSel) {

            RECT rTop = { r.left, r.top, r.right, r.top + 3 };

            HBRUSH hbrAcc = CreateSolidBrush(pal.uiAccent);

            FillRect(hdc, &rTop, hbrAcc);

            DeleteObject(hbrAcc);

        }



        SelectObject(hdc, hUIBoldFont);

        SetTextColor(hdc, RGB(255, 145, 0));

        std::wstring numStr = std::to_wstring(i + 1) + L".";

        RECT rNum = { r.left + 10, r.top + 2, r.left + 30, r.bottom };

        DrawTextW(hdc, numStr.c_str(), -1, &rNum, DT_LEFT | DT_VCENTER | DT_SINGLELINE);



        SelectObject(hdc, isSel ? hUIBoldFont : hUIFont);

        const wchar_t* fn = wcsrchr(tabs[i].filePath.c_str(), L'\\');

        fn = fn ? fn + 1 : tabs[i].filePath.c_str();

        std::wstring title = fn;

        if (tabs[i].isModified) title += L" ●";



        SetTextColor(hdc, isDarkTheme ? (isSel ? RGB(255, 255, 255) : RGB(160, 160, 175))

            : (isSel ? RGB(10, 10, 10) : RGB(90, 90, 100)));

        RECT rTxt = { r.left + 30, r.top + 2, r.right - 28, r.bottom };

        DrawTextW(hdc, title.c_str(), -1, &rTxt, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);



        RECT rClose = tabs[i].rcClose;

        bool closeHov = (i == g_hoverTabCloseIdx);

        if (closeHov) {

            HBRUSH hbrClose = CreateSolidBrush(RGB(220, 60, 60));

            FillRect(hdc, &rClose, hbrClose);

            DeleteObject(hbrClose);

        }

        int cx = (rClose.left + rClose.right) / 2;

        int cy = (rClose.top + rClose.bottom) / 2;

        HPEN hPenX = CreatePen(PS_SOLID, 2, closeHov ? RGB(255, 255, 255) : (isSel ? RGB(230, 85, 85) : RGB(135, 135, 145)));

        SelectObject(hdc, hPenX);

        MoveToEx(hdc, cx - 4, cy - 4, NULL);

        LineTo(hdc, cx + 4, cy + 4);

        MoveToEx(hdc, cx + 4, cy - 4, NULL);

        LineTo(hdc, cx - 4, cy + 4);

        SelectObject(hdc, hBorderPen);

        DeleteObject(hPenX);

    }



    if (g_rcPlusBtn.right < rc.right) {

        HBRUSH hbrPlus = CreateSolidBrush(g_hoverPlusBtn ? pal.uiAccent : pal.uiTabInactive);

        FillRect(hdc, &g_rcPlusBtn, hbrPlus);

        DeleteObject(hbrPlus);

        SelectObject(hdc, hUIBoldFont);

        SetTextColor(hdc, isDarkTheme || g_hoverPlusBtn ? RGB(255, 255, 255) : RGB(40, 40, 40));

        DrawTextW(hdc, L"+", -1, &g_rcPlusBtn, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    }



    // 3. ЗАГОЛОВОК КОНСОЛИ (SPLITTER BAR)

    int consoleBottom = rc.bottom - g_statusHeight;

    int consoleTop = consoleBottom - g_consoleHeight;

    int hdrTop = consoleTop - g_consoleHeaderHeight;

    RECT rcConHdr = { 0, hdrTop, rc.right, consoleTop };



    HBRUSH hbrConHdr = CreateSolidBrush(pal.uiTopBar);

    FillRect(hdc, &rcConHdr, hbrConHdr);

    DeleteObject(hbrConHdr);



    MoveToEx(hdc, 0, hdrTop, NULL);

    LineTo(hdc, rc.right, hdrTop);

    MoveToEx(hdc, 0, consoleTop - 1, NULL);

    LineTo(hdc, rc.right, consoleTop - 1);



    SelectObject(hdc, hSmallFont);

    SetTextColor(hdc, isDarkTheme ? RGB(170, 170, 190) : RGB(60, 60, 75));

    RECT rcConTxt = { 12, hdrTop, rc.right - 140, consoleTop };

    DrawTextW(hdc, L"ТЕРМИНАЛ / ВЫВОД КОМПИЛЯТОРА (двойной клик по ошибке -> переход к строке)", -1, &rcConTxt, DT_LEFT | DT_VCENTER | DT_SINGLELINE);



    RECT rcClearBtn = { rc.right - 115, hdrTop + 2, rc.right - 10, consoleTop - 2 };

    SetTextColor(hdc, RGB(230, 110, 110));

    DrawTextW(hdc, L"[ Очистить ]", -1, &rcClearBtn, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);



    // 4. КАСТОМНЫЙ ТЕМНЫЙ СТАТУС-БАР ВНИЗУ

    RECT rcStatus = { 0, consoleBottom, rc.right, rc.bottom };

    HBRUSH hbrStat = CreateSolidBrush(pal.uiStatusBg);

    FillRect(hdc, &rcStatus, hbrStat);

    DeleteObject(hbrStat);



    RECT rcStatLeft = { 0, consoleBottom, 110, rc.bottom };

    HBRUSH hbrAcc = CreateSolidBrush(pal.uiAccent);

    FillRect(hdc, &rcStatLeft, hbrAcc);

    DeleteObject(hbrAcc);



    SelectObject(hdc, hUIBoldFont);

    SetTextColor(hdc, RGB(255, 255, 255));

    DrawTextW(hdc, L"● READY", -1, &rcStatLeft, DT_CENTER | DT_VCENTER | DT_SINGLELINE);



    SelectObject(hdc, hUIFont);

    SetTextColor(hdc, pal.uiStatusFg);



    int pos = (int)SendActive(SCI_GETCURRENTPOS);

    int line = (int)SendActive(SCI_LINEFROMPOSITION, pos) + 1;

    int col = (int)SendActive(SCI_GETCOLUMN, pos) + 1;

    int totalLen = (int)SendActive(SCI_GETLENGTH);

    int zoom = 100 + (int)SendActive(SCI_GETZOOM) * 10;

    const wchar_t* langName = (currentTab >= 0) ? GetLangName(tabs[currentTab].lang) : L"Plain Text";

    const wchar_t* compState = (wcslen(szCompilerPath) > 0) ? L"spcomp: Подключен ✓" : L"spcomp: Не выбран";



    wchar_t statBuf[512];

    swprintf(statBuf, 512, L"Стр %d, Стлб %d   |   Символов: %d   |   Масштаб: %d%%   |   UTF-8   |   %s   |   %s",

        line, col, totalLen, zoom, langName, compState);



    RECT rcStatText = { 122, consoleBottom, rc.right - 12, rc.bottom };

    DrawTextW(hdc, statBuf, -1, &rcStatText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);



    SelectObject(hdc, oldPen);

    DeleteObject(hBorderPen);



    BitBlt(hdcReal, 0, 0, rc.right, rc.bottom, hdc, 0, 0, SRCCOPY);

    SelectObject(hdc, oldBm);

    DeleteObject(hbm);

    DeleteDC(hdc);

}



// ============================================================

//  ГЛАВНАЯ ОКОННАЯ ПРОЦЕДУРА

// ============================================================

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    switch (uMsg) {

    case WM_CREATE: {

        hwndMain = hwnd;

        LoadConfig();

        ApplyDwmDarkTitleBar(hwnd, isDarkTheme);



        hUIFont = CreateFontW(15, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");

        hUIBoldFont = CreateFontW(15, 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");

        hTitleFont = CreateFontW(26, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");

        hSmallFont = CreateFontW(13, 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");



        InitToolbarButtons();



        hwndConsole = CreateWindowExW(0, L"Scintilla", L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,

            0, 0, 0, 0, hwnd, NULL, hInst, NULL);

        SendEd(hwndConsole, SCI_SETCODEPAGE, SC_CP_UTF8, 0);

        SendEd(hwndConsole, SCI_SETMARGINWIDTHN, 0, 0);

        SendEd(hwndConsole, SCI_SETMARGINWIDTHN, 1, 0);

        SendEd(hwndConsole, SCI_SETREADONLY, TRUE);



        RebuildMenu();

        ApplyThemeToAll();

        DragAcceptFiles(hwnd, TRUE);



        LogToConsoleW(L"==============================================================", LOG_ACCENT);

        LogToConsoleW(L"> Kova Code Studio v1.0.0 Release (Разработчик: ANR1S) готов к работе!", LOG_SUCCESS);

        LogToConsoleW(L"> Новое: Палитра цветов чата + Генератор кода [🎨 Цвета чата] (Ctrl+M) и перемещение строк (Alt+Up/Down).", LOG_NORMAL);

        if (wcslen(szCompilerPath) > 0) {

            LogToConsoleW((std::wstring(L"> Загружен сохраненный путь к spcomp.exe: ") + szCompilerPath).c_str(), LOG_NORMAL);

        }

        RestoreSavedSession();

        return 0;

    }



    case WM_GETMINMAXINFO: {

        // Ограничиваем минимальный размер окна, чтобы при сжатии верстка никогда не ломалась

        LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;

        lpMMI->ptMinTrackSize.x = 760;

        lpMMI->ptMinTrackSize.y = 440;

        return 0;

    }



    case WM_ERASEBKGND:

        return 1;



    case WM_PAINT: {

        PAINTSTRUCT ps;

        HDC hdc = BeginPaint(hwnd, &ps);

        PaintMainWindowChrome(hwnd, hdc);

        EndPaint(hwnd, &ps);

        return 0;

    }



    case WM_MOUSEMOVE: {

        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

        RECT rc;

        GetClientRect(hwnd, &rc);



        if (g_isDraggingSplitter) {

            int newConH = (rc.bottom - g_statusHeight) - pt.y;

            if (newConH < 60) newConH = 60;

            if (newConH > rc.bottom - 220) newConH = rc.bottom - 220;

            g_consoleHeight = newConH;

            ResizeLayout();

            return 0;

        }



        int consoleTop = rc.bottom - g_statusHeight - g_consoleHeight;

        int hdrTop = consoleTop - g_consoleHeaderHeight;

        if (pt.y >= hdrTop && pt.y <= consoleTop && pt.x < rc.right - 120) {

            SetCursor(LoadCursor(NULL, IDC_SIZENS));

        }



        int hovTb = -1;

        if (pt.y < g_toolbarHeight) {

            for (size_t i = 0; i < g_toolbarButtons.size(); i++) {

                if (PtInRect(&g_toolbarButtons[i].rc, pt)) {

                    hovTb = (int)i;

                    break;

                }

            }

        }



        int hovTab = -1;

        int hovClose = -1;

        bool hovPlus = false;

        if (pt.y >= g_toolbarHeight && pt.y <= g_toolbarHeight + g_tabHeight) {

            RecalcCustomTabRects();

            for (int i = 0; i < (int)tabs.size(); i++) {

                if (PtInRect(&tabs[i].rcTab, pt)) {

                    hovTab = i;

                    if (PtInRect(&tabs[i].rcClose, pt)) hovClose = i;

                    break;

                }

            }

            if (PtInRect(&g_rcPlusBtn, pt)) hovPlus = true;

        }



        if (hovTb != g_hoverToolbarBtn || hovTab != g_hoverTabIdx || hovClose != g_hoverTabCloseIdx || hovPlus != g_hoverPlusBtn) {

            g_hoverToolbarBtn = hovTb;

            g_hoverTabIdx = hovTab;

            g_hoverTabCloseIdx = hovClose;

            g_hoverPlusBtn = hovPlus;

            RECT rcTopArea = { 0, 0, rc.right, g_toolbarHeight + g_tabHeight };

            InvalidateRect(hwnd, &rcTopArea, FALSE);

        }

        return 0;

    }



    case WM_LBUTTONDOWN: {

        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

        RECT rc;

        GetClientRect(hwnd, &rc);



        if (pt.y < g_toolbarHeight) {

            for (const auto& b : g_toolbarButtons) {

                if (PtInRect(&b.rc, pt)) {

                    SendMessageW(hwnd, WM_COMMAND, b.id, 0);

                    return 0;

                }

            }

        }



        if (pt.y >= g_toolbarHeight && pt.y <= g_toolbarHeight + g_tabHeight) {

            RecalcCustomTabRects();

            for (int i = 0; i < (int)tabs.size(); i++) {

                if (PtInRect(&tabs[i].rcTab, pt)) {

                    if (PtInRect(&tabs[i].rcClose, pt)) {

                        CloseTab(i);

                    }

                    else {

                        SwitchTab(i);

                    }

                    return 0;

                }

            }

            if (PtInRect(&g_rcPlusBtn, pt)) {

                CreateOrReplaceTab(L"Новый файл.txt");

                return 0;

            }

        }



        int consoleTop = rc.bottom - g_statusHeight - g_consoleHeight;

        int hdrTop = consoleTop - g_consoleHeaderHeight;

        if (pt.y >= hdrTop && pt.y <= consoleTop) {

            if (pt.x >= rc.right - 115) {

                ClearConsole();

            }

            else {

                g_isDraggingSplitter = true;

                SetCapture(hwnd);

            }

            return 0;

        }

        return 0;

    }



    case WM_MBUTTONDOWN: {

        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

        if (pt.y >= g_toolbarHeight && pt.y <= g_toolbarHeight + g_tabHeight) {

            RecalcCustomTabRects();

            for (int i = 0; i < (int)tabs.size(); i++) {

                if (PtInRect(&tabs[i].rcTab, pt)) {

                    CloseTab(i);

                    return 0;

                }

            }

        }

        return 0;

    }



    case WM_RBUTTONDOWN: {

        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

        if (pt.y >= g_toolbarHeight && pt.y <= g_toolbarHeight + g_tabHeight) {

            RecalcCustomTabRects();

            for (int i = 0; i < (int)tabs.size(); i++) {

                if (PtInRect(&tabs[i].rcTab, pt)) {

                    g_contextMenuTabIdx = i;

                    SwitchTab(i);

                    HMENU hPop = CreatePopupMenu();

                    AppendMenuW(hPop, MF_STRING, IDM_TAB_CLOSE, L"Закрыть вкладку");

                    AppendMenuW(hPop, MF_STRING, IDM_TAB_CLOSE_OTHERS, L"Закрыть остальные вкладки");

                    AppendMenuW(hPop, MF_SEPARATOR, 0, NULL);

                    AppendMenuW(hPop, MF_STRING, IDM_TAB_COPY_PATH, L"Скопировать полный путь");

                    AppendMenuW(hPop, MF_STRING, IDM_TAB_OPEN_DIR, L"Открыть папку с файлом в Проводнике");

                    ClientToScreen(hwnd, &pt);

                    TrackPopupMenu(hPop, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);

                    DestroyMenu(hPop);

                    return 0;

                }

            }

        }

        return 0;

    }



    case WM_LBUTTONUP: {

        if (g_isDraggingSplitter) {

            g_isDraggingSplitter = false;

            ReleaseCapture();

        }

        return 0;

    }



    case WM_NOTIFY: {

        LPNMHDR pnm = (LPNMHDR)lParam;

        if (pnm->hwndFrom == hwndConsole) {

            if (pnm->code == SCN_DOUBLECLICK) {

                JumpToErrorFromConsole();

            }

            return 0;

        }



        int ti = FindTab(pnm->hwndFrom);

        if (ti < 0) return 0;



        SCNotification* scn = (SCNotification*)lParam;

        HWND ed = tabs[ti].hwndEditor;

        switch (pnm->code) {

        case SCN_STYLENEEDED:

            StyleEditor(tabs[ti], (int)scn->position);

            break;

        case SCN_MARGINCLICK: {

            if (scn->margin == MARGIN_FOLD) {

                int lineClick = (int)SendEd(ed, SCI_LINEFROMPOSITION, scn->position);

                SendEd(ed, SCI_TOGGLEFOLD, lineClick, 0);

            }

            break;

        }

        case SCN_UPDATEUI: {

            HandleBraces(ed);

            RECT rc;

            GetClientRect(hwnd, &rc);

            RECT rcStat = { 0, rc.bottom - g_statusHeight, rc.right, rc.bottom };

            InvalidateRect(hwnd, &rcStat, FALSE);

            break;

        }

        case SCN_CHARADDED:

            HandleCharAdded(tabs[ti], scn->ch);

            break;

        case SCN_SAVEPOINTLEFT: {

            tabs[ti].isModified = true;

            RECT rc; GetClientRect(hwnd, &rc);

            RECT rcTb = { 0, g_toolbarHeight, rc.right, g_toolbarHeight + g_tabHeight };

            InvalidateRect(hwnd, &rcTb, FALSE);

            break;

        }

        case SCN_SAVEPOINTREACHED: {

            tabs[ti].isModified = false;

            RECT rc; GetClientRect(hwnd, &rc);

            RECT rcTb = { 0, g_toolbarHeight, rc.right, g_toolbarHeight + g_tabHeight };

            InvalidateRect(hwnd, &rcTb, FALSE);

            break;

        }

        case SCN_MODIFIED:

            if (scn->modificationType & (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT)) {

                int line = (int)SendEd(ed, SCI_LINEFROMPOSITION, scn->position);

                SendEd(ed, SCI_MARKERADD, line, MARKER_UNSAVED);

                SendEd(ed, SCI_MARKERDELETE, line, MARKER_SAVED);

                if (scn->linesAdded != 0) {

                    UpdateLineMargin(ed);

                }

            }

            break;

        }

        return 0;

    }



    case WM_DROPFILES: {

        HDROP hDrop = (HDROP)wParam;

        UINT count = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);

        for (UINT i = 0; i < count; i++) {

            wchar_t path[MAX_PATH];

            DragQueryFileW(hDrop, i, path, MAX_PATH);

            OpenPath(path);

        }

        DragFinish(hDrop);

        return 0;

    }



    case WM_COMMAND: {

        switch (LOWORD(wParam)) {

        case IDM_FILE_NEW:          CreateOrReplaceTab(L"Новый файл.txt"); break;

        case IDM_FILE_OPEN:         OpenFileDlg(); break;

        case IDM_FILE_SAVE:         SaveCurrentFile(false); break;

        case IDM_FILE_SAVEAS:       SaveCurrentFile(true); break;

        case IDM_FILE_OPEN_FOLDER:  OpenContainingFolder(currentTab); break;

        case IDM_FILE_EXIT:         PostQuitMessage(0); break;

        case IDM_RUN_SMART:         RunSmartDispatcher(); break;

        case IDM_SET_COMPILER:      SelectCompilerDlg(); break;

        case IDM_CLEAR_CONSOLE:     ClearConsole(); break;

        case IDM_EDIT_FIND_REPLACE: ShowCustomFindDialog(); break;

        case IDM_EDIT_GOTO_LINE:    ShowGotoLineDialog(); break;

        case IDM_EDIT_DUPLICATE:    DuplicateCurrentLine(); break;

        case IDM_EDIT_COMMENT:      ToggleLineComment(); break;

        case IDM_EDIT_LINE_UP:      MoveSelectedLinesUp(); break;

        case IDM_EDIT_LINE_DOWN:    MoveSelectedLinesDown(); break;

        case IDM_EDIT_SNIPPET_SP:   InsertSourcePawnTemplate(); break;

        case IDM_EDIT_CHAT_COLORS:  ShowChatColorsDialog(); break;

        case IDM_TAB_CLOSE:         CloseTab(g_contextMenuTabIdx); break;

        case IDM_TAB_CLOSE_OTHERS:  CloseOtherTabs(g_contextMenuTabIdx); break;

        case IDM_TAB_OPEN_DIR:      OpenContainingFolder(g_contextMenuTabIdx); break;

        case IDM_TAB_COPY_PATH:     CopyFilePathToClipboard(g_contextMenuTabIdx); break;

        case IDM_THEME_DARK:        isDarkTheme = true;  SaveConfig(); ApplyThemeToAll(); RebuildMenu(); break;

        case IDM_THEME_LIGHT:       isDarkTheme = false; SaveConfig(); ApplyThemeToAll(); RebuildMenu(); break;

        case IDM_THEME_TOGGLE:      isDarkTheme = !isDarkTheme; SaveConfig(); ApplyThemeToAll(); RebuildMenu(); break;

        case IDM_ABOUT:             ShowAboutWindow(); break;

        default:

            if (LOWORD(wParam) >= IDM_LANG_RU && LOWORD(wParam) <= IDM_LANG_IT) {

                currentLang = LOWORD(wParam) - IDM_LANG_RU;

                SaveConfig();

                RebuildMenu();

            }

            break;

        }

        return 0;

    }



    case WM_SIZE:

        ResizeLayout();

        return 0;



    case WM_SETFOCUS:

        if (currentTab >= 0) SetFocus(tabs[currentTab].hwndEditor);

        return 0;



    case WM_DESTROY:

        SaveConfig();

        PostQuitMessage(0);

        return 0;

    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);

}



// ============================================================

//  ТОЧКА ВХОДА В ПРОГРАММУ

// ============================================================

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {

    hInst = hInstance;

    InitExePaths();



    hModScintilla = LoadScintillaSmart();

    if (!hModScintilla) {

        MessageBoxW(NULL,

            L"Не найден движок Scintilla.dll или SciLexer.dll!\n\n"

            L"Положите файл Scintilla.dll в папку проекта или рядом с KovaCode.exe.",

            L"Kova Code Studio v1.0.0", MB_OK | MB_ICONERROR);

        return 1;

    }



    LoadAppIconSmart(hInstance);



    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW) };

    wc.style = CS_HREDRAW | CS_VREDRAW;

    wc.lpfnWndProc = WndProc;

    wc.hInstance = hInstance;

    wc.lpszClassName = L"KovaCodeClass";

    wc.hIcon = hGlobalAppIconBig;

    wc.hIconSm = hGlobalAppIconSmall;

    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    wc.hbrBackground = NULL; // Заливка полностью контролируется двойной буферизацией

    RegisterClassExW(&wc);



    hwndMain = CreateWindowExW(0, L"KovaCodeClass", L"Kova Code Studio v1.0.0",

        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 1180, 780, NULL, NULL, hInstance, NULL);



    SendMessageW(hwndMain, WM_SETICON, ICON_BIG, (LPARAM)hGlobalAppIconBig);

    SendMessageW(hwndMain, WM_SETICON, ICON_SMALL, (LPARAM)hGlobalAppIconSmall);



    ShowWindow(hwndMain, nCmdShow);

    UpdateWindow(hwndMain);



    ACCEL accels[] = {

        { FCONTROL | FVIRTKEY, 'N', IDM_FILE_NEW },

        { FCONTROL | FVIRTKEY, 'O', IDM_FILE_OPEN },

        { FCONTROL | FVIRTKEY, 'S', IDM_FILE_SAVE },

        { FCONTROL | FVIRTKEY, 'F', IDM_EDIT_FIND_REPLACE },

        { FCONTROL | FVIRTKEY, 'H', IDM_EDIT_FIND_REPLACE },

        { FCONTROL | FVIRTKEY, 'G', IDM_EDIT_GOTO_LINE },

        { FCONTROL | FVIRTKEY, 'D', IDM_EDIT_DUPLICATE },

        { FCONTROL | FVIRTKEY, 'J', IDM_EDIT_SNIPPET_SP },

        { FCONTROL | FVIRTKEY, 'M', IDM_EDIT_CHAT_COLORS },

        { FCONTROL | FVIRTKEY, VK_OEM_2, IDM_EDIT_COMMENT },

        { FALT | FVIRTKEY, VK_UP, IDM_EDIT_LINE_UP },

        { FALT | FVIRTKEY, VK_DOWN, IDM_EDIT_LINE_DOWN },

        { FVIRTKEY, VK_F5, IDM_RUN_SMART }

    };

    HACCEL hAccel = CreateAcceleratorTableW(accels, sizeof(accels) / sizeof(ACCEL));



    MSG msg = { 0 };

    while (GetMessageW(&msg, NULL, 0, 0)) {

        if (!TranslateAcceleratorW(hwndMain, hAccel, &msg)) {

            TranslateMessage(&msg);

            DispatchMessageW(&msg);

        }

    }



    DestroyAcceleratorTable(hAccel);

    FreeLibrary(hModScintilla);

    return 0;

}