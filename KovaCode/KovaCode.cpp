#pragma comment(linker, "/SUBSYSTEM:windows")

#pragma comment(lib, "Comdlg32.lib")

#pragma comment(lib, "Comctl32.lib")

#pragma comment(lib, "Shell32.lib")

#pragma comment(lib, "Msimg32.lib")

#pragma comment(lib, "Dwmapi.lib")

#pragma comment(lib, "Wininet.lib")



#include <windows.h>

#include <windowsx.h>

#include <commdlg.h>

#include <commctrl.h>

#include <shellapi.h>

#include <dwmapi.h>

#include <wininet.h>

#include <stdio.h>

#include <string.h>

#include <ctype.h>

#include <vector>

#include <string>

#include <unordered_set>

#include <set>

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

#define MARKER_ERROR            2

#define INDIC_WORD_HIGHLIGHT    8



#define IDT_OUTLINE_UPDATE      101



#define IDM_FILE_NEW            1000

#define IDM_FILE_OPEN           1001

#define IDM_FILE_SAVE           1002

#define IDM_FILE_SAVEAS         1003

#define IDM_FILE_OPEN_FOLDER    1004

#define IDM_FILE_EXIT           1005

#define IDM_FILE_SAVEALL        1023



#define IDM_EDIT_FIND_REPLACE   1006

#define IDM_EDIT_GOTO_LINE      1007

#define IDM_EDIT_DUPLICATE      1008

#define IDM_EDIT_COMMENT        1009

#define IDM_EDIT_SNIPPET_SP     1010

#define IDM_EDIT_CHAT_COLORS    1011

#define IDM_EDIT_LINE_UP        1012

#define IDM_EDIT_LINE_DOWN      1013

#define IDM_EDIT_AUTOCOMPLETE   1018

#define IDM_EDIT_FORMAT_CODE    1024

#define IDM_EDIT_SELECT_ALL_OCC 1030

#define IDM_EDIT_FOLD_ALL       1031

#define IDM_EDIT_UNFOLD_ALL     1032



#define IDM_RUN_SMART           1014

#define IDM_RUN_AND_UPLOAD      1028

#define IDM_SET_COMPILER        1015

#define IDM_CLEAR_CONSOLE       1016

#define IDM_ABOUT               1017

#define IDM_CHANGELOG           1029

#define IDM_FTP_MANAGER         1026

#define IDM_FTP_UPLOAD_ACTIVE   1027



#define IDM_VIEW_OUTLINE        1019

#define IDM_VIEW_WORDWRAP       1025

#define IDM_VIEW_ZOOM_IN        1020

#define IDM_VIEW_ZOOM_OUT       1021

#define IDM_VIEW_ZOOM_RESET     1022



#define IDM_TAB_CLOSE           1100

#define IDM_TAB_CLOSE_OTHERS    1101

#define IDM_TAB_OPEN_DIR        1102

#define IDM_TAB_COPY_PATH       1103

#define IDM_TAB_CLOSE_CURRENT   1104

#define IDM_TAB_NEXT            1105

#define IDM_TAB_PREV            1106



#define IDM_THEME_DARK          2001

#define IDM_THEME_LIGHT         2002

#define IDM_THEME_CYBER         2004

#define IDM_THEME_MONOKAI       2005

#define IDM_THEME_ONEDARK       2006

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

    LANG_HTML,

    LANG_CFG

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

int currentTheme = 0; // 0=Dark+, 1=Light+, 2=Cyber Neon, 3=Monokai Pro, 4=One Dark Pro

int currentLang = 0; // 0=RU, 1=EN, 2=DE, 3=FR, 4=IT



bool g_showOutline = true;

int g_outlineWidth = 240;

int g_hoverOutlineIdx = -1;

int g_outlineScroll = 0;

bool g_hoverOutlineClose = false;

RECT g_rcOutlineClose = { 0, 0, 0, 0 };



bool g_wordWrap = false;

bool g_findMatchCase = false;



// FTP-клиент (WinINet)

HINTERNET g_hInet = NULL;

HINTERNET g_hFtp = NULL;

HWND hwndFtpDlg = NULL;

HWND hwndChangelogDlg = NULL;

wchar_t szFtpHost[128] = L"";

int g_ftpPort = 21;

wchar_t szFtpUser[128] = L"";

wchar_t szFtpPass[128] = L"";

wchar_t szFtpRemoteDir[512] = L"/";

bool g_ftpPassive = true;

bool g_ftpAutoUploadOnSave = false;



#define MAX_FTP_PROFILES 8

struct FtpProfile {

    std::wstring name;

    std::wstring host;

    int port;

    std::wstring user;

    std::wstring pass;

    std::wstring remoteDir;

    bool passive;

};

std::vector<FtpProfile> g_ftpProfiles;



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



struct OutlineItem {

    std::wstring label;

    int line;

    int kind; // 0=fn, 1=ev/cmd/cvar, 2=#def/type/cls, 3=section

    RECT rc;

};



struct TabData {

    HWND hwndEditor;

    std::wstring filePath;

    Lang lang;

    bool isModified;

    RECT rcTab;

    RECT rcClose;

    std::vector<OutlineItem> outline;

    std::wstring ftpRemotePath;

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



const wchar_t* GetThemeName(int theme) {

    switch (theme) {

    case 1:  return L"VS Code Light+";

    case 2:  return L"Cyber Neon / Dracula";

    case 3:  return L"Monokai Pro";

    case 4:  return L"One Dark Pro";

    default: return L"VS Code Dark+";

    }

}



// ============================================================

//  ЛОКАЛИЗАЦИЯ (5 ЯЗЫКОВ)

// ============================================================

const wchar_t* loc[][24] = {

    // RU

    {

        L"Файл", L"Создать\tCtrl+N", L"Открыть...\tCtrl+O", L"Сохранить\tCtrl+S",

        L"Сохранить как...", L"Открыть папку с файлом", L"Выход",

        L"Правка", L"Найти и заменить...\tCtrl+F", L"Перейти к строке...\tCtrl+G",

        L"Дублировать строку\tCtrl+D", L"Вставить шаблон плагина\tCtrl+J",

        L"Сборка", L"Скомпилировать / Запустить\tF5", L"Выбрать spcomp.exe...",

        L"Настройки", L"Сохранить всё\tCtrl+Shift+S", L"Автодополнение\tCtrl+Space",

        L"Форматировать код\tShift+Alt+F", L"Вид", L"Структура кода\tCtrl+B",

        L"Перенос по словам\tAlt+Z", L"Увеличить масштаб\tCtrl++", L"Уменьшить масштаб\tCtrl+-"

    },

    // EN

    {

        L"File", L"New\tCtrl+N", L"Open...\tCtrl+O", L"Save\tCtrl+S",

        L"Save As...", L"Open Containing Folder", L"Exit",

        L"Edit", L"Find & Replace...\tCtrl+F", L"Go to Line...\tCtrl+G",

        L"Duplicate Line\tCtrl+D", L"Insert Plugin Template\tCtrl+J",

        L"Build", L"Compile / Run\tF5", L"Select spcomp.exe...",

        L"Settings", L"Save All\tCtrl+Shift+S", L"AutoComplete\tCtrl+Space",

        L"Format Code\tShift+Alt+F", L"View", L"Code Outline\tCtrl+B",

        L"Word Wrap\tAlt+Z", L"Zoom In\tCtrl++", L"Zoom Out\tCtrl+-"

    },

    // DE

    {

        L"Datei", L"Neu\tCtrl+N", L"Öffnen...\tCtrl+O", L"Speichern\tCtrl+S",

        L"Speichern unter...", L"Ordner öffnen", L"Beenden",

        L"Bearbeiten", L"Suchen & Ersetzen...\tCtrl+F", L"Gehe zu Zeile...\tCtrl+G",

        L"Zeile duplizieren\tCtrl+D", L"Plugin-Vorlage einfügen\tCtrl+J",

        L"Erstellen", L"Kompilieren / Ausführen\tF5", L"Wähle spcomp.exe...",

        L"Einstellungen", L"Alle speichern\tCtrl+Shift+S", L"Autovervollständigung\tCtrl+Space",

        L"Code formatieren\tShift+Alt+F", L"Ansicht", L"Code-Gliederung\tCtrl+B",

        L"Zeilenumbruch\tAlt+Z", L"Vergrößern\tCtrl++", L"Verkleinern\tCtrl+-"

    },

    // FR

    {

        L"Fichier", L"Nouveau\tCtrl+N", L"Ouvrir...\tCtrl+O", L"Enregistrer\tCtrl+S",

        L"Enregistrer sous...", L"Ouvrir le dossier", L"Quitter",

        L"Édition", L"Rechercher et remplacer...\tCtrl+F", L"Aller à la ligne...\tCtrl+G",

        L"Dupliquer la ligne\tCtrl+D", L"Insérer modèle de plugin\tCtrl+J",

        L"Construire", L"Compiler / Exécuter\tF5", L"Sélectionner spcomp.exe...",

        L"Paramètres", L"Tout enregistrer\tCtrl+Shift+S", L"Complétion auto\tCtrl+Space",

        L"Formater le code\tShift+Alt+F", L"Affichage", L"Structure du code\tCtrl+B",

        L"Retour à la ligne\tAlt+Z", L"Zoom avant\tCtrl++", L"Zoom arrière\tCtrl+-"

    },

    // IT

    {

        L"File", L"Nuovo\tCtrl+N", L"Apri...\tCtrl+O", L"Salva\tCtrl+S",

        L"Salva con nome...", L"Apri cartella file", L"Esci",

        L"Modifica", L"Trova e Sostituisci...\tCtrl+F", L"Vai alla riga...\tCtrl+G",

        L"Duplica riga\tCtrl+D", L"Inserisci modello plugin\tCtrl+J",

        L"Costruisci", L"Compila / Esegui\tF5", L"Seleziona spcomp.exe...",

        L"Impostazioni", L"Salva tutto\tCtrl+Shift+S", L"Completamento auto\tCtrl+Space",

        L"Formatta codice\tShift+Alt+F", L"Visualizza", L"Struttura codice\tCtrl+B",

        L"A capo automatico\tAlt+Z", L"Ingrandisci\tCtrl++", L"Rimpicciolisci\tCtrl+-"

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

//  АВТОПОИСК SPCOMP И СОХРАНЕНИЕ / ЗАГРУЗКА КОНФИГУРАЦИИ

// ============================================================

static bool IsValidWindowsExe(const wchar_t* path) {

    if (!path || !*path) return false;

    FILE* fp = NULL;

    if (_wfopen_s(&fp, path, L"rb") != 0 || !fp) return false;

    IMAGE_DOS_HEADER dos = { 0 };

    bool ok = false;

    if (fread(&dos, sizeof(dos), 1, fp) == 1 && dos.e_magic == IMAGE_DOS_SIGNATURE) {

        if (dos.e_lfanew > 0 && dos.e_lfanew < 0x100000) {

            if (fseek(fp, dos.e_lfanew, SEEK_SET) == 0) {

                DWORD peSig = 0;

                if (fread(&peSig, sizeof(peSig), 1, fp) == 1 && peSig == IMAGE_NT_SIGNATURE) {

                    ok = true;

                }

            }

        }

    }

    fclose(fp);

    return ok;

}



void AutoDetectSpcompSmart() {

    if (wcslen(szCompilerPath) > 0 &&

        GetFileAttributesW(szCompilerPath) != INVALID_FILE_ATTRIBUTES &&

        IsValidWindowsExe(szCompilerPath)) {

        return;

    }

    szCompilerPath[0] = 0;



    const wchar_t* candidates[] = {

        L"compilers\\1.11\\addons\\sourcemod\\scripting\\spcomp64.exe",

        L"compilers\\1.11\\addons\\sourcemod\\scripting\\spcomp.exe",

        L"compilers\\1.11\\spcomp64.exe",

        L"compilers\\1.11\\spcomp.exe",

        L"..\\compilers\\1.11\\addons\\sourcemod\\scripting\\spcomp64.exe",

        L"..\\compilers\\1.11\\addons\\sourcemod\\scripting\\spcomp.exe",

        L"..\\..\\compilers\\1.11\\addons\\sourcemod\\scripting\\spcomp64.exe",

        L"..\\..\\compilers\\1.11\\addons\\sourcemod\\scripting\\spcomp.exe",

        L"..\\KovaCode\\compilers\\1.11\\addons\\sourcemod\\scripting\\spcomp64.exe",

        L"..\\KovaCode\\compilers\\1.11\\addons\\sourcemod\\scripting\\spcomp.exe",

        L"..\\KovaCode\\compilers\\1.11\\spcomp64.exe",

        L"..\\KovaCode\\compilers\\1.11\\spcomp.exe",

        L"..\\..\\KovaCode\\compilers\\1.11\\addons\\sourcemod\\scripting\\spcomp64.exe",

        L"..\\..\\KovaCode\\compilers\\1.11\\addons\\sourcemod\\scripting\\spcomp.exe",

        L"..\\..\\KovaCode\\compilers\\1.11\\spcomp64.exe",

        L"..\\..\\KovaCode\\compilers\\1.11\\spcomp.exe",

        L"spcomp64.exe",

        L"spcomp.exe"

    };



    for (size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); i++) {

        wchar_t raw[MAX_PATH] = { 0 };

        swprintf(raw, MAX_PATH, L"%s%s", szExeDir, candidates[i]);

        if (GetFileAttributesW(raw) != INVALID_FILE_ATTRIBUTES && IsValidWindowsExe(raw)) {

            wchar_t full[MAX_PATH] = { 0 };

            if (GetFullPathNameW(raw, MAX_PATH, full, NULL) > 0) {

                wcscpy_s(szCompilerPath, full);

                return;

            }

        }

    }

}



void LoadConfig() {

    int savedTheme = GetPrivateProfileIntW(L"Settings", L"Theme", -1, szIniPath);

    if (savedTheme >= 0 && savedTheme <= 4) {

        currentTheme = savedTheme;

    }

    else {

        currentTheme = (GetPrivateProfileIntW(L"Settings", L"DarkTheme", 1, szIniPath) != 0) ? 0 : 1;

    }

    isDarkTheme = (currentTheme != 1);

    currentLang = GetPrivateProfileIntW(L"Settings", L"Language", 0, szIniPath);

    if (currentLang < 0 || currentLang > 4) currentLang = 0;

    g_showOutline = GetPrivateProfileIntW(L"Settings", L"ShowOutline", 1, szIniPath) != 0;

    g_wordWrap = GetPrivateProfileIntW(L"Settings", L"WordWrap", 0, szIniPath) != 0;

    GetPrivateProfileStringW(L"Compiler", L"SpcompPath", L"", szCompilerPath, MAX_PATH, szIniPath);

    GetPrivateProfileStringW(L"FTP", L"Host", L"", szFtpHost, 128, szIniPath);

    g_ftpPort = GetPrivateProfileIntW(L"FTP", L"Port", 21, szIniPath);

    if (g_ftpPort <= 0 || g_ftpPort > 65535) g_ftpPort = 21;

    GetPrivateProfileStringW(L"FTP", L"User", L"", szFtpUser, 128, szIniPath);

    GetPrivateProfileStringW(L"FTP", L"Pass", L"", szFtpPass, 128, szIniPath);

    GetPrivateProfileStringW(L"FTP", L"RemoteDir", L"/", szFtpRemoteDir, 512, szIniPath);

    g_ftpPassive = GetPrivateProfileIntW(L"FTP", L"Passive", 1, szIniPath) != 0;

    g_ftpAutoUploadOnSave = GetPrivateProfileIntW(L"FTP", L"AutoUpload", 0, szIniPath) != 0;

    g_ftpProfiles.clear();

    int pCount = GetPrivateProfileIntW(L"FTPProfiles", L"Count", 0, szIniPath);

    if (pCount > MAX_FTP_PROFILES) pCount = MAX_FTP_PROFILES;

    for (int i = 0; i < pCount; i++) {

        wchar_t key[32], buf[512];

        FtpProfile p;

        swprintf(key, 32, L"P%d_Name", i); GetPrivateProfileStringW(L"FTPProfiles", key, L"", buf, 128, szIniPath); p.name = buf;

        swprintf(key, 32, L"P%d_Host", i); GetPrivateProfileStringW(L"FTPProfiles", key, L"", buf, 128, szIniPath); p.host = buf;

        swprintf(key, 32, L"P%d_Port", i); p.port = GetPrivateProfileIntW(L"FTPProfiles", key, 21, szIniPath);

        if (p.port <= 0 || p.port > 65535) p.port = 21;

        swprintf(key, 32, L"P%d_User", i); GetPrivateProfileStringW(L"FTPProfiles", key, L"", buf, 128, szIniPath); p.user = buf;

        swprintf(key, 32, L"P%d_Pass", i); GetPrivateProfileStringW(L"FTPProfiles", key, L"", buf, 128, szIniPath); p.pass = buf;

        swprintf(key, 32, L"P%d_Dir", i);  GetPrivateProfileStringW(L"FTPProfiles", key, L"/", buf, 512, szIniPath); p.remoteDir = buf;

        swprintf(key, 32, L"P%d_Pasv", i); p.passive = GetPrivateProfileIntW(L"FTPProfiles", key, 1, szIniPath) != 0;

        if (!p.host.empty()) g_ftpProfiles.push_back(p);

    }

    AutoDetectSpcompSmart();

}



void SaveConfig() {

    isDarkTheme = (currentTheme != 1);

    WritePrivateProfileStringW(L"Settings", L"DarkTheme", isDarkTheme ? L"1" : L"0", szIniPath);

    wchar_t buf[32];

    swprintf(buf, 32, L"%d", currentTheme);

    WritePrivateProfileStringW(L"Settings", L"Theme", buf, szIniPath);

    swprintf(buf, 32, L"%d", currentLang);

    WritePrivateProfileStringW(L"Settings", L"Language", buf, szIniPath);

    WritePrivateProfileStringW(L"Settings", L"ShowOutline", g_showOutline ? L"1" : L"0", szIniPath);

    WritePrivateProfileStringW(L"Settings", L"WordWrap", g_wordWrap ? L"1" : L"0", szIniPath);

    WritePrivateProfileStringW(L"Compiler", L"SpcompPath", szCompilerPath, szIniPath);

    WritePrivateProfileStringW(L"FTP", L"Host", szFtpHost, szIniPath);

    swprintf(buf, 32, L"%d", g_ftpPort);

    WritePrivateProfileStringW(L"FTP", L"Port", buf, szIniPath);

    WritePrivateProfileStringW(L"FTP", L"User", szFtpUser, szIniPath);

    WritePrivateProfileStringW(L"FTP", L"Pass", szFtpPass, szIniPath);

    WritePrivateProfileStringW(L"FTP", L"RemoteDir", szFtpRemoteDir, szIniPath);

    WritePrivateProfileStringW(L"FTP", L"Passive", g_ftpPassive ? L"1" : L"0", szIniPath);

    WritePrivateProfileStringW(L"FTP", L"AutoUpload", g_ftpAutoUploadOnSave ? L"1" : L"0", szIniPath);

    int pCount = (int)g_ftpProfiles.size();

    if (pCount > MAX_FTP_PROFILES) pCount = MAX_FTP_PROFILES;

    swprintf(buf, 32, L"%d", pCount);

    WritePrivateProfileStringW(L"FTPProfiles", L"Count", buf, szIniPath);

    for (int i = 0; i < pCount; i++) {

        wchar_t key[32], pbuf[16];

        swprintf(key, 32, L"P%d_Name", i); WritePrivateProfileStringW(L"FTPProfiles", key, g_ftpProfiles[i].name.c_str(), szIniPath);

        swprintf(key, 32, L"P%d_Host", i); WritePrivateProfileStringW(L"FTPProfiles", key, g_ftpProfiles[i].host.c_str(), szIniPath);

        swprintf(key, 32, L"P%d_Port", i); swprintf(pbuf, 16, L"%d", g_ftpProfiles[i].port); WritePrivateProfileStringW(L"FTPProfiles", key, pbuf, szIniPath);

        swprintf(key, 32, L"P%d_User", i); WritePrivateProfileStringW(L"FTPProfiles", key, g_ftpProfiles[i].user.c_str(), szIniPath);

        swprintf(key, 32, L"P%d_Pass", i); WritePrivateProfileStringW(L"FTPProfiles", key, g_ftpProfiles[i].pass.c_str(), szIniPath);

        swprintf(key, 32, L"P%d_Dir", i);  WritePrivateProfileStringW(L"FTPProfiles", key, g_ftpProfiles[i].remoteDir.c_str(), szIniPath);

        swprintf(key, 32, L"P%d_Pasv", i); WritePrivateProfileStringW(L"FTPProfiles", key, g_ftpProfiles[i].passive ? L"1" : L"0", szIniPath);

    }



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

static std::wstring Utf8ToWide(const char* s, int len = -1) {

    if (!s) return L"";

    int n = (len < 0) ? (int)strlen(s) : len;

    if (n <= 0) return L"";

    int wLen = MultiByteToWideChar(CP_UTF8, 0, s, n, NULL, 0);

    if (wLen <= 0) return L"";

    std::wstring res(wLen, 0);

    MultiByteToWideChar(CP_UTF8, 0, s, n, &res[0], wLen);

    return res;

}



static std::string WideToUtf8(const wchar_t* s, int len = -1) {

    if (!s) return "";

    int n = (len < 0) ? (int)wcslen(s) : len;

    if (n <= 0) return "";

    int uLen = WideCharToMultiByte(CP_UTF8, 0, s, n, NULL, 0, NULL, NULL);

    if (uLen <= 0) return "";

    std::string res(uLen, 0);

    WideCharToMultiByte(CP_UTF8, 0, s, n, &res[0], uLen, NULL, NULL);

    return res;

}



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

    if (ext == L"cfg" || ext == L"ini" || ext == L"vdf" || ext == L"kv" || ext == L"json" || ext == L"bat" || ext == L"cmd") {

        return LANG_CFG;

    }

    return LANG_NONE;

}



const wchar_t* GetLangName(Lang l) {

    switch (l) {

    case LANG_SP:   return L"SourcePawn / C++";

    case LANG_PY:   return L"Python";

    case LANG_HTML: return L"HTML / Web";

    case LANG_CFG:  return L"Config / KeyValues";

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



static void LexCfg(const char* s, int n, std::vector<unsigned char>& st) {

    int i = 0;

    while (i < n) {

        unsigned char c = (unsigned char)s[i];

        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {

            i++;

            continue;

        }

        if (c == '/' && i + 1 < n && s[i + 1] == '/') {

            int j = i;

            while (j < n && s[j] != '\n') j++;

            Fill(st, i, j, S_COMMENT);

            i = j;

            continue;

        }

        if (c == ';' || c == '#') {

            int k = i - 1;

            while (k >= 0 && (s[k] == ' ' || s[k] == '\t')) k--;

            if (k < 0 || s[k] == '\n') {

                int j = i;

                while (j < n && s[j] != '\n') j++;

                Fill(st, i, j, S_COMMENT);

                i = j;

                continue;

            }

        }

        if (c == '[') {

            int k = i - 1;

            while (k >= 0 && (s[k] == ' ' || s[k] == '\t')) k--;

            if (k < 0 || s[k] == '\n') {

                int j = i + 1;

                while (j < n && s[j] != ']' && s[j] != '\n') j++;

                if (j < n && s[j] == ']') j++;

                Fill(st, i, j, S_TAG);

                i = j;

                continue;

            }

        }

        if (c == '"' || c == '\'') {

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

        if (isdigit(c) || ((c == '-' || c == '.') && i + 1 < n && isdigit((unsigned char)s[i + 1]))) {

            int j = i + 1;

            while (j < n && (isalnum((unsigned char)s[j]) || s[j] == '.')) j++;

            Fill(st, i, j, S_NUMBER);

            i = j;

            continue;

        }

        if (isalpha(c) || c == '_' || c == '$') {

            int j = i + 1;

            while (j < n && (isalnum((unsigned char)s[j]) || s[j] == '_' || s[j] == '-' || s[j] == '.')) j++;

            std::string w(s + i, j - i);

            std::string lw = w;

            for (char& ch : lw) ch = (char)tolower((unsigned char)ch);

            unsigned char sty = S_DEFAULT;

            if (lw == "true" || lw == "false" || lw == "null" || lw == "yes" || lw == "no" || lw == "on" || lw == "off") {

                sty = S_CONST;

            }

            else if (lw == "set" || lw == "echo" || lw == "if" || lw == "else" || lw == "goto" || lw == "call" || lw == "rem") {

                sty = S_KEYWORD;

            }

            else {

                int q = j;

                while (q < n && (s[q] == ' ' || s[q] == '\t')) q++;

                if (q < n && (s[q] == '=' || s[q] == ':')) sty = S_ATTR;

            }

            Fill(st, i, j, sty);

            i = j;

            continue;

        }

        if (c < 128 && ispunct(c)) {

            st[i] = S_OPERATOR;

        }

        i++;

    }

}



void UpdateCodeFolding(HWND h, Lang lg) {

    int lines = (int)SendEd(h, SCI_GETLINECOUNT);

    if (lg == LANG_SP || lg == LANG_CFG || lg == LANG_HTML) {

        int level = SC_FOLDLEVELBASE;

        for (int line = 0; line < lines; line++) {

            int start = (int)SendEd(h, SCI_POSITIONFROMLINE, line);

            int end = (int)SendEd(h, SCI_GETLINEENDPOSITION, line);

            int nextLevel = level;

            for (int p = start; p < end; p++) {

                int sty = (int)SendEd(h, SCI_GETSTYLEAT, p);

                if (sty == S_COMMENT || sty == S_BLOCKCOMMENT || sty == S_STRING || sty == S_HTMLCOMMENT) continue;

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

    else if (lg == LANG_PY) {

        int tw = (int)SendEd(h, SCI_GETTABWIDTH);

        if (tw <= 0) tw = 4;

        std::vector<int> indents(lines, -1);

        for (int line = 0; line < lines; line++) {

            int start = (int)SendEd(h, SCI_POSITIONFROMLINE, line);

            int end = (int)SendEd(h, SCI_GETLINEENDPOSITION, line);

            int indPos = (int)SendEd(h, SCI_GETLINEINDENTPOSITION, line);

            if (indPos < end && SendEd(h, SCI_GETCHARAT, indPos) != '#') {

                indents[line] = (int)SendEd(h, SCI_GETLINEINDENTATION, line) / tw;

            }

            (void)start;

        }

        int lastLevel = 0;

        for (int line = 0; line < lines; line++) {

            if (indents[line] < 0) {

                SendEd(h, SCI_SETFOLDLEVEL, line, (SC_FOLDLEVELBASE + lastLevel) | SC_FOLDLEVELWHITEFLAG);

                continue;

            }

            int cur = indents[line];

            int next = cur;

            for (int nl = line + 1; nl < lines; nl++) {

                if (indents[nl] >= 0) {

                    next = indents[nl];

                    break;

                }

            }

            int flags = SC_FOLDLEVELBASE + cur;

            if (next > cur) flags |= SC_FOLDLEVELHEADERFLAG;

            SendEd(h, SCI_SETFOLDLEVEL, line, flags);

            lastLevel = cur;

        }

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

    else if (t.lang == LANG_CFG) {

        LexCfg(buf.data(), n, st);

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

//  СТРУКТУРА КОДА (OUTLINE SIDEBAR) И АВТОДОПОЛНЕНИЕ (INTELLISENSE)

// ============================================================

static std::string TrimCopy(const std::string& s) {

    size_t a = 0;

    while (a < s.size() && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r' || s[a] == '\n')) a++;

    size_t b = s.size();

    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r' || s[b - 1] == '\n')) b--;

    return s.substr(a, b - a);

}



static std::string ExtractQuotedLiteral(const std::string& s, size_t startAt) {

    size_t q1 = s.find('"', startAt);

    if (q1 == std::string::npos) return "";

    size_t q2 = s.find('"', q1 + 1);

    if (q2 == std::string::npos || q2 <= q1 + 1) return "";

    return s.substr(q1 + 1, q2 - q1 - 1);

}



void UpdateOutline(TabData& t) {

    t.outline.clear();

    HWND h = t.hwndEditor;

    if (!h) return;

    int len = (int)SendEd(h, SCI_GETLENGTH);

    if (len <= 0 || len > 500000) return;



    std::vector<char> buf((size_t)len + 1, 0);

    SendEd(h, SCI_GETTEXT, (WPARAM)(len + 1), (LPARAM)buf.data());



    std::vector<std::string> lines;

    lines.reserve(256);

    const char* p = buf.data();

    const char* start = p;

    while (*p) {

        if (*p == '\n') {

            lines.emplace_back(start, p - start);

            start = p + 1;

        }

        p++;

    }

    if (p > start) lines.emplace_back(start, p - start);



    if (t.lang == LANG_SP) {

        static const std::unordered_set<std::string> skipCalls = {

            "if", "else", "for", "while", "switch", "return", "sizeof", "view_as", "defined"

        };

        int braceDepth = 0;

        bool inBlockComment = false;

        for (int ln = 0; ln < (int)lines.size(); ln++) {

            std::string raw = lines[ln];

            std::string clean;

            clean.reserve(raw.size());

            bool inStr = false;

            char strCh = 0;

            int lineStartDepth = braceDepth;



            for (size_t i = 0; i < raw.size(); i++) {

                char c = raw[i];

                if (inBlockComment) {

                    if (c == '*' && i + 1 < raw.size() && raw[i + 1] == '/') {

                        inBlockComment = false;

                        i++;

                    }

                    continue;

                }

                if (inStr) {

                    clean.push_back(c);

                    if (c == '\\' && i + 1 < raw.size()) {

                        clean.push_back(raw[++i]);

                    }

                    else if (c == strCh) {

                        inStr = false;

                    }

                    continue;

                }

                if (c == '/' && i + 1 < raw.size() && raw[i + 1] == '/') break;

                if (c == '/' && i + 1 < raw.size() && raw[i + 1] == '*') {

                    inBlockComment = true;

                    i++;

                    continue;

                }

                if (c == '"' || c == '\'') {

                    inStr = true;

                    strCh = c;

                    clean.push_back(c);

                    continue;

                }

                if (c == '{') braceDepth++;

                else if (c == '}' && braceDepth > 0) braceDepth--;

                clean.push_back(c);

            }



            std::string tr = TrimCopy(clean);

            if (tr.empty()) continue;



            if (tr.rfind("#define ", 0) == 0 || tr.rfind("#define\t", 0) == 0) {

                size_t k = 7;

                while (k < tr.size() && (tr[k] == ' ' || tr[k] == '\t')) k++;

                size_t e = k;

                while (e < tr.size() && (isalnum((unsigned char)tr[e]) || tr[e] == '_')) e++;

                if (e > k) {

                    t.outline.push_back({ L"# " + Utf8ToWide(tr.substr(k, e - k).c_str()), ln, 2, {0,0,0,0} });

                }

                continue;

            }



            auto checkTypeDecl = [&](const char* kw) -> bool {

                size_t kl = strlen(kw);

                if (tr.rfind(kw, 0) == 0 && tr.size() > kl && (tr[kl] == ' ' || tr[kl] == '\t')) {

                    size_t k = kl;

                    while (k < tr.size() && (tr[k] == ' ' || tr[k] == '\t')) k++;

                    size_t e = k;

                    while (e < tr.size() && (isalnum((unsigned char)tr[e]) || tr[e] == '_')) e++;

                    if (e > k) {

                        t.outline.push_back({ Utf8ToWide((std::string(kw) + " " + tr.substr(k, e - k)).c_str()), ln, 2, {0,0,0,0} });

                        return true;

                    }

                }

                return false;

            };

            if (lineStartDepth == 0 && (checkTypeDecl("enum") || checkTypeDecl("struct") || checkTypeDecl("methodmap") || checkTypeDecl("class"))) {

                continue;

            }



            size_t pCmd = tr.find("RegConsoleCmd");

            if (pCmd == std::string::npos) pCmd = tr.find("RegAdminCmd");

            if (pCmd != std::string::npos) {

                std::string arg = ExtractQuotedLiteral(tr, pCmd);

                if (!arg.empty()) {

                    t.outline.push_back({ L"cmd: " + Utf8ToWide(arg.c_str()), ln, 1, {0,0,0,0} });

                }

            }

            size_t pEv = tr.find("HookEvent");

            if (pEv != std::string::npos) {

                std::string arg = ExtractQuotedLiteral(tr, pEv);

                if (!arg.empty()) {

                    t.outline.push_back({ L"ev: " + Utf8ToWide(arg.c_str()), ln, 1, {0,0,0,0} });

                }

            }

            size_t pCv = tr.find("CreateConVar");

            if (pCv != std::string::npos) {

                std::string arg = ExtractQuotedLiteral(tr, pCv);

                if (!arg.empty()) {

                    t.outline.push_back({ L"cvar: " + Utf8ToWide(arg.c_str()), ln, 1, {0,0,0,0} });

                }

            }



            if (lineStartDepth == 0 && tr[0] != '#') {

                bool isNativeOrFwd = (tr.rfind("native ", 0) == 0 || tr.rfind("forward ", 0) == 0);

                bool endsSemi = (!tr.empty() && tr.back() == ';');

                if (!endsSemi || isNativeOrFwd) {

                    size_t lp = tr.find('(');

                    if (lp != std::string::npos && lp > 0 && tr.find('=') == std::string::npos) {

                        size_t e = lp;

                        while (e > 0 && (tr[e - 1] == ' ' || tr[e - 1] == '\t')) e--;

                        size_t k = e;

                        while (k > 0 && (isalnum((unsigned char)tr[k - 1]) || tr[k - 1] == '_' || tr[k - 1] == ':')) k--;

                        if (e > k) {

                            std::string fn = tr.substr(k, e - k);

                            if (!skipCalls.count(fn)) {

                                t.outline.push_back({ Utf8ToWide((fn + "()").c_str()), ln, 0, {0,0,0,0} });

                            }

                        }

                    }

                }

            }

        }

    }

    else if (t.lang == LANG_PY) {

        for (int ln = 0; ln < (int)lines.size(); ln++) {

            std::string tr = TrimCopy(lines[ln]);

            if (tr.rfind("async def ", 0) == 0 || tr.rfind("def ", 0) == 0) {

                size_t k = (tr[0] == 'a') ? 10 : 4;

                while (k < tr.size() && tr[k] == ' ') k++;

                size_t e = k;

                while (e < tr.size() && (isalnum((unsigned char)tr[e]) || tr[e] == '_')) e++;

                if (e > k) {

                    t.outline.push_back({ Utf8ToWide((tr.substr(k, e - k) + "()").c_str()), ln, 0, {0,0,0,0} });

                }

            }

            else if (tr.rfind("class ", 0) == 0) {

                size_t k = 6;

                while (k < tr.size() && tr[k] == ' ') k++;

                size_t e = k;

                while (e < tr.size() && (isalnum((unsigned char)tr[e]) || tr[e] == '_')) e++;

                if (e > k) {

                    t.outline.push_back({ L"class " + Utf8ToWide(tr.substr(k, e - k).c_str()), ln, 2, {0,0,0,0} });

                }

            }

        }

    }

    else if (t.lang == LANG_HTML) {

        static const char* tags[] = { "<head", "<body", "<header", "<nav", "<main", "<section", "<footer", "<form", "<script", "<style" };

        for (int ln = 0; ln < (int)lines.size(); ln++) {

            std::string tr = TrimCopy(lines[ln]);

            std::string low = tr;

            for (char& ch : low) ch = (char)tolower((unsigned char)ch);

            for (auto tg : tags) {

                size_t tl = strlen(tg);

                if (low.rfind(tg, 0) == 0 && (low.size() == tl || low[tl] == '>' || low[tl] == ' ')) {

                    t.outline.push_back({ Utf8ToWide((std::string(tg) + ">").c_str()), ln, 3, {0,0,0,0} });

                    break;

                }

            }

            size_t pid = low.find("id=\"");

            if (pid != std::string::npos) {

                std::string idv = ExtractQuotedLiteral(tr, pid + 3);

                if (!idv.empty()) {

                    t.outline.push_back({ L"#" + Utf8ToWide(idv.c_str()), ln, 2, {0,0,0,0} });

                }

            }

        }

    }

    else if (t.lang == LANG_CFG) {

        int depth = 0;

        for (int ln = 0; ln < (int)lines.size(); ln++) {

            std::string tr = TrimCopy(lines[ln]);

            if (tr.empty() || tr.rfind("//", 0) == 0 || tr[0] == ';' || tr[0] == '#') continue;

            if (tr[0] == '[' && tr.back() == ']' && tr.size() > 2) {

                t.outline.push_back({ Utf8ToWide(tr.c_str()), ln, 3, {0,0,0,0} });

                continue;

            }

            if (depth <= 1 && tr[0] == '"') {

                std::string key = ExtractQuotedLiteral(tr, 0);

                if (!key.empty()) {

                    bool isBlock = (tr.find('{') != std::string::npos);

                    if (!isBlock && ln + 1 < (int)lines.size()) {

                        std::string nxt = TrimCopy(lines[ln + 1]);

                        if (!nxt.empty() && nxt[0] == '{') isBlock = true;

                    }

                    if (isBlock) {

                        t.outline.push_back({ Utf8ToWide(key.c_str()), ln, 3, {0,0,0,0} });

                    }

                }

            }

            for (char c : tr) {

                if (c == '{') depth++;

                else if (c == '}' && depth > 0) depth--;

            }

        }

    }

}



void TriggerAutoComplete(TabData& t, bool force) {

    HWND h = t.hwndEditor;

    if (!h) return;

    int pos = (int)SendEd(h, SCI_GETCURRENTPOS);

    if (!force && pos > 0) {

        int sty = (int)SendEd(h, SCI_GETSTYLEAT, pos - 1);

        if (sty == S_COMMENT || sty == S_BLOCKCOMMENT || sty == S_HTMLCOMMENT || sty == S_STRING || sty == S_TRIPLE) {

            return;

        }

    }



    int wordStart = (int)SendEd(h, SCI_WORDSTARTPOSITION, pos, TRUE);

    int prefixLen = pos - wordStart;

    if (!force && prefixLen < 2) {

        if (SendEd(h, SCI_AUTOCACTIVE)) SendEd(h, SCI_AUTOCCANCEL);

        return;

    }



    std::string prefix;

    if (prefixLen > 0) {

        prefix.resize((size_t)prefixLen + 1, 0);

        Sci_TextRange tr;

        tr.chrg.cpMin = wordStart;

        tr.chrg.cpMax = pos;

        tr.lpstrText = &prefix[0];

        SendEd(h, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);

        prefix.resize((size_t)prefixLen);

    }



    static const char* spApi[] = {

        "OnPluginStart", "OnPluginEnd", "OnMapStart", "OnMapEnd",

        "OnClientPutInServer", "OnClientPostAdminCheck", "OnClientConnected",

        "OnClientAuthorized", "OnClientDisconnect", "OnPlayerRunCmd", "OnTakeDamage",

        "RegConsoleCmd", "RegAdminCmd", "AddCommandListener", "ReplyToCommand",

        "GetCmdArg", "GetCmdArgInt", "GetCmdArgs",

        "HookEvent", "HookEventEx", "UnhookEvent",

        "CreateConVar", "FindConVar", "AutoExecConfig",

        "CreateTimer", "KillTimer", "TriggerTimer",

        "PrintToChat", "PrintToChatAll", "CPrintToChat", "CPrintToChatAll",

        "PrintToServer", "PrintToConsole", "PrintHintText", "PrintCenterText",

        "Format", "FormatEx", "StrEqual", "StrContains", "strcopy",

        "IntToString", "StringToInt", "StringToFloat", "FloatToString",

        "IsClientInGame", "IsClientConnected", "IsPlayerAlive", "IsFakeClient",

        "GetClientName", "GetClientAuthId", "GetClientIP", "GetClientTeam",

        "GetClientHealth", "SetEntityHealth", "GetClientOfUserId", "GetClientUserId",

        "GetUserFlagBits", "CheckCommandAccess", "GetClientAbsOrigin", "TeleportEntity",

        "CreateEntityByName", "DispatchSpawn", "AcceptEntityInput",

        "GetEntProp", "SetEntProp", "GetEntPropFloat", "SetEntPropFloat",

        "GetEntPropString", "SetEntPropString", "GetEdictClassname",

        "IsValidEntity", "IsValidEdict",

        "EmitSoundToClient", "EmitSoundToAll", "PrecacheSound", "PrecacheModel",

        "BuildPath", "OpenFile", "ReadFileLine", "WriteFileLine", "FileExists",

        "SQL_TConnect", "SQL_FormatQuery",

        "RegClientCookie", "SetClientCookie", "GetClientCookie", "AreClientCookiesCached",

        "GetEngineTime", "GetGameTime", "GetTime", "FormatTime",

        "SetHudTextParams", "ShowHudText", "ShowSyncHudText", "CreateHudSynchronizer",

        "MaxClients", "MAXPLAYERS", "MAX_NAME_LENGTH", "PLATFORM_MAX_PATH",

        "Plugin_Continue", "Plugin_Handled", "Plugin_Stop", "Plugin_Changed",

        "INVALID_HANDLE", "TIMER_REPEAT", "TIMER_FLAG_NO_MAPCHANGE",

        "FCVAR_NOTIFY", "ADMFLAG_GENERIC", "ADMFLAG_ROOT", "ADMFLAG_BAN",

        "Menu", "Panel", "ArrayList", "StringMap", "DataPack", "KeyValues",

        "Database", "DBResultSet", "Event", "ConVar", "Handle", "Action", "Cookie",

        "TopMenu", "SMCParser", "public", "stock", "native", "forward",

        "methodmap", "property", "typedef", "typeset", "view_as", "sizeof",

        "newdecls", "semicolon", "sourcemod", "sdktools", "sdkhooks", "morecolors"

    };

    static const char* pyApi[] = {

        "def", "class", "import", "from", "return", "async", "await", "lambda",

        "print", "len", "range", "enumerate", "zip", "isinstance", "open",

        "str", "int", "float", "list", "dict", "set", "tuple", "bool",

        "True", "False", "None", "__name__", "__main__", "Exception", "ValueError"

    };

    static const char* htmlApi[] = {

        "div", "span", "section", "header", "footer", "main", "nav", "article",

        "button", "input", "form", "label", "select", "option", "textarea",

        "script", "style", "link", "meta", "title", "class", "href", "src",

        "function", "const", "let", "document", "querySelector", "addEventListener",

        "console.log", "display", "flex", "grid", "background", "border-radius"

    };



    std::set<std::string> candidates;

    if (t.lang == LANG_SP) {

        for (auto s : spApi) candidates.insert(s);

    }

    else if (t.lang == LANG_PY) {

        for (auto s : pyApi) candidates.insert(s);

    }

    else if (t.lang == LANG_HTML) {

        for (auto s : htmlApi) candidates.insert(s);

    }



    int docLen = (int)SendEd(h, SCI_GETLENGTH);

    if (docLen > 0 && docLen < 300000) {

        std::vector<char> doc((size_t)docLen + 1, 0);

        SendEd(h, SCI_GETTEXT, (WPARAM)(docLen + 1), (LPARAM)doc.data());

        int idx = 0;

        while (idx < docLen) {

            unsigned char c = (unsigned char)doc[idx];

            if (isalpha(c) || c == '_') {

                int s = idx;

                idx++;

                while (idx < docLen && (isalnum((unsigned char)doc[idx]) || doc[idx] == '_')) idx++;

                if (idx - s >= 3 && s != wordStart) {

                    candidates.emplace(doc.data() + s, idx - s);

                }

            }

            else {

                idx++;

            }

        }

    }



    std::vector<std::string> matches;

    for (const auto& cand : candidates) {

        if (prefixLen == 0 || _strnicmp(cand.c_str(), prefix.c_str(), (size_t)prefixLen) == 0) {

            if (!force && (int)cand.size() == prefixLen && _stricmp(cand.c_str(), prefix.c_str()) == 0) {

                continue;

            }

            matches.push_back(cand);

        }

    }



    if (matches.empty()) {

        if (SendEd(h, SCI_AUTOCACTIVE)) SendEd(h, SCI_AUTOCCANCEL);

        return;

    }



    std::sort(matches.begin(), matches.end(), [](const std::string& a, const std::string& b) {

        int cmp = _stricmp(a.c_str(), b.c_str());

        return (cmp != 0) ? (cmp < 0) : (a < b);

        });



    std::string joined;

    size_t limit = matches.size();

    if (limit > 80) limit = 80;

    for (size_t i = 0; i < limit; i++) {

        if (i > 0) joined += ' ';

        joined += matches[i];

    }

    SendEd(h, SCI_AUTOCSHOW, prefixLen, (LPARAM)joined.c_str());

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

        return;

    }



    int nextCh = (int)SendEd(h, SCI_GETCHARAT, pos);

    if ((ch == ')' || ch == ']' || ch == '}') && nextCh == ch) {

        SendEd(h, SCI_DELETERANGE, pos - 1, 1);

        SendEd(h, SCI_GOTOPOS, pos);

        return;

    }



    if (ch == '"') {

        int lineStart = (int)SendEd(h, SCI_POSITIONFROMLINE, line);

        int quotesBefore = 0;

        for (int p = lineStart; p < pos - 1; p++) {

            int c = (int)SendEd(h, SCI_GETCHARAT, p);

            if (c == '\\') {

                p++;

                continue;

            }

            if (c == '"') quotesBefore++;

        }

        if ((quotesBefore % 2 == 1) && nextCh == '"') {

            SendEd(h, SCI_DELETERANGE, pos - 1, 1);

            SendEd(h, SCI_GOTOPOS, pos);

            return;

        }

        int sty = (pos > 1) ? (int)SendEd(h, SCI_GETSTYLEAT, pos - 2) : S_DEFAULT;

        if ((quotesBefore % 2 == 0) &&

            sty != S_COMMENT && sty != S_BLOCKCOMMENT && sty != S_HTMLCOMMENT &&

            (nextCh == 0 || nextCh == ' ' || nextCh == '\t' || nextCh == '\r' || nextCh == '\n' || nextCh == ')' || nextCh == ']' || nextCh == '}' || nextCh == ';' || nextCh == ',')) {

            SendEd(h, SCI_INSERTTEXT, pos, (LPARAM)"\"");

        }

        return;

    }



    if (ch == '}') {

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

        if (nextCh == 0 || nextCh == ' ' || nextCh == '\t' || nextCh == '\r' || nextCh == '\n' || nextCh == ')' || nextCh == ']' || nextCh == '}' || nextCh == ';' || nextCh == ',') {

            const char* closeStr = (ch == '(') ? ")" : (ch == '[') ? "]" : "}";

            SendEd(h, SCI_INSERTTEXT, pos, (LPARAM)closeStr);

        }

    }

    else if ((ch >= 0 && ch < 128 && isalnum(ch)) || ch == '_') {

        TriggerAutoComplete(t, false);

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

        "    author      = \"Athor\",\n"

        "    description = \"Created plugin in Kova Code Studio\",\n"

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

    UpdateOutline(tabs[currentTab]);

    InvalidateRect(hwndMain, NULL, FALSE);

    LogToConsoleW(L"> Вставлен шаблон плагина SourcePawn (ANR1S).", LOG_ACCENT);

}



void DuplicateCurrentLine() {

    if (currentTab < 0) return;

    SendActive(SCI_LINEDUPLICATE);

}



void HighlightWordOccurrences(HWND h) {

    if (!h) return;

    int docLen = (int)SendEd(h, SCI_GETLENGTH);

    SendEd(h, SCI_SETINDICATORCURRENT, INDIC_WORD_HIGHLIGHT);

    SendEd(h, SCI_INDICATORCLEARRANGE, 0, docLen);

    if (docLen <= 0 || docLen > 150000) return;



    int selS = (int)SendEd(h, SCI_GETSELECTIONSTART);

    int selE = (int)SendEd(h, SCI_GETSELECTIONEND);

    int wStart = selS, wEnd = selE;

    if (selS == selE) {

        wStart = (int)SendEd(h, SCI_WORDSTARTPOSITION, selS, TRUE);

        wEnd = (int)SendEd(h, SCI_WORDENDPOSITION, selS, TRUE);

    }

    int wLen = wEnd - wStart;

    if (wLen < 2 || wLen > 64) return;



    std::vector<char> word((size_t)wLen + 1, 0);

    Sci_TextRange tr;

    tr.chrg.cpMin = wStart;

    tr.chrg.cpMax = wEnd;

    tr.lpstrText = word.data();

    SendEd(h, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);

    for (int i = 0; i < wLen; i++) {

        unsigned char c = (unsigned char)word[i];

        if (!(isalnum(c) || c == '_')) return;

    }



    int flags = SCFIND_WHOLEWORD | SCFIND_MATCHCASE;

    int pos = 0;

    int count = 0;

    while (pos < docLen && count < 300) {

        Sci_TextToFind ttf;

        ttf.chrg.cpMin = pos;

        ttf.chrg.cpMax = docLen;

        ttf.lpstrText = word.data();

        int found = (int)SendEd(h, SCI_FINDTEXT, flags, (LPARAM)&ttf);

        if (found < 0) break;

        SendEd(h, SCI_INDICATORFILLRANGE, found, wLen);

        pos = (int)ttf.chrgText.cpMax;

        count++;

    }

}



void SelectAllWordOccurrences() {

    if (currentTab < 0) return;

    HWND h = tabs[currentTab].hwndEditor;

    int docLen = (int)SendEd(h, SCI_GETLENGTH);

    if (docLen <= 0) return;



    int selS = (int)SendEd(h, SCI_GETSELECTIONSTART);

    int selE = (int)SendEd(h, SCI_GETSELECTIONEND);

    if (selS == selE) {

        selS = (int)SendEd(h, SCI_WORDSTARTPOSITION, selS, TRUE);

        selE = (int)SendEd(h, SCI_WORDENDPOSITION, selE, TRUE);

    }

    int wLen = selE - selS;

    if (wLen <= 0 || wLen > 128) return;



    std::vector<char> word((size_t)wLen + 1, 0);

    Sci_TextRange tr;

    tr.chrg.cpMin = selS;

    tr.chrg.cpMax = selE;

    tr.lpstrText = word.data();

    SendEd(h, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);



    int pos = 0;

    int count = 0;

    while (pos < docLen && count < 200) {

        Sci_TextToFind ttf;

        ttf.chrg.cpMin = pos;

        ttf.chrg.cpMax = docLen;

        ttf.lpstrText = word.data();

        int found = (int)SendEd(h, SCI_FINDTEXT, SCFIND_MATCHCASE, (LPARAM)&ttf);

        if (found < 0) break;

        if (count == 0) SendEd(h, SCI_SETSELECTION, ttf.chrgText.cpMax, ttf.chrgText.cpMin);

        else SendEd(h, SCI_ADDSELECTION, ttf.chrgText.cpMax, ttf.chrgText.cpMin);

        pos = (int)ttf.chrgText.cpMax;

        count++;

    }

    if (count > 0) {

        wchar_t msg[128];

        swprintf(msg, 128, L"> Мультикурсор: выделено вхождений: %d.", count);

        LogToConsoleW(msg, LOG_ACCENT);

    }

}



void FoldAllCode(bool fold) {

    if (currentTab < 0) return;

    SendActive(SCI_FOLDALL, fold ? SC_FOLDACTION_CONTRACT : SC_FOLDACTION_EXPAND, 0);

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

    int selStart = (int)SendEd(h, SCI_GETSELECTIONSTART);

    int selEnd = (int)SendEd(h, SCI_GETSELECTIONEND);

    int lineStart = (int)SendEd(h, SCI_LINEFROMPOSITION, selStart);

    int lineEnd = (int)SendEd(h, SCI_LINEFROMPOSITION, selEnd);

    if (selEnd > selStart && lineEnd > lineStart && SendEd(h, SCI_POSITIONFROMLINE, lineEnd) == selEnd) {

        lineEnd--;

    }



    bool isHash = (tabs[currentTab].lang == LANG_PY);

    const char* prefix = isHash ? "# " : "// ";

    int prefixLen = isHash ? 1 : 2;



    bool allCommented = true;

    bool anyNonEmpty = false;

    for (int ln = lineStart; ln <= lineEnd; ln++) {

        int ls = (int)SendEd(h, SCI_POSITIONFROMLINE, ln);

        int le = (int)SendEd(h, SCI_GETLINEENDPOSITION, ln);

        int ip = (int)SendEd(h, SCI_GETLINEINDENTPOSITION, ln);

        if (ip >= le && lineStart != lineEnd) continue;

        anyNonEmpty = true;

        int c1 = (int)SendEd(h, SCI_GETCHARAT, ip);

        int c2 = (int)SendEd(h, SCI_GETCHARAT, ip + 1);

        if (isHash) {

            if (c1 != '#') { allCommented = false; break; }

        }

        else {

            if (!(c1 == '/' && c2 == '/')) { allCommented = false; break; }

        }

        (void)ls;

    }

    if (!anyNonEmpty) allCommented = false;



    SendEd(h, SCI_BEGINUNDOACTION);

    for (int ln = lineStart; ln <= lineEnd; ln++) {

        int le = (int)SendEd(h, SCI_GETLINEENDPOSITION, ln);

        int ip = (int)SendEd(h, SCI_GETLINEINDENTPOSITION, ln);

        if (ip >= le && lineStart != lineEnd) continue;

        if (allCommented) {

            int nextC = (int)SendEd(h, SCI_GETCHARAT, ip + prefixLen);

            int delLen = (nextC == ' ') ? (prefixLen + 1) : prefixLen;

            SendEd(h, SCI_DELETERANGE, ip, delLen);

        }

        else {

            SendEd(h, SCI_INSERTTEXT, ip, (LPARAM)prefix);

        }

    }

    SendEd(h, SCI_ENDUNDOACTION);

}



void FormatActiveCode() {

    if (currentTab < 0) return;

    HWND h = tabs[currentTab].hwndEditor;

    int len = (int)SendEd(h, SCI_GETLENGTH);

    if (len <= 0) return;



    std::vector<char> buf((size_t)len + 1, 0);

    SendEd(h, SCI_GETTEXT, (WPARAM)(len + 1), (LPARAM)buf.data());



    std::vector<std::string> rawLines;

    const char* p = buf.data();

    const char* s = p;

    bool usesCRLF = false;

    while (*p) {

        if (*p == '\n') {

            size_t lineLen = (size_t)(p - s);

            if (lineLen > 0 && s[lineLen - 1] == '\r') {

                usesCRLF = true;

                lineLen--;

            }

            rawLines.emplace_back(s, lineLen);

            s = p + 1;

        }

        p++;

    }

    if (p > s) rawLines.emplace_back(s, (size_t)(p - s));



    Lang lg = tabs[currentTab].lang;

    bool braceLang = (lg == LANG_SP || lg == LANG_CFG);

    int depth = 0;

    int tw = (int)SendEd(h, SCI_GETTABWIDTH);

    if (tw <= 0) tw = 4;

    bool inBlockComment = false;



    std::string out;

    out.reserve((size_t)len + 256);

    const char* eol = usesCRLF ? "\r\n" : "\n";



    for (size_t li = 0; li < rawLines.size(); li++) {

        std::string line = rawLines[li];

        while (!line.empty() && (line.back() == ' ' || line.back() == '\t')) line.pop_back();



        if (braceLang) {

            std::string trimmed = TrimCopy(line);

            if (trimmed.empty()) {

                if (li + 1 < rawLines.size()) out += eol;

                continue;

            }

            int lineDepth = depth;

            if (!inBlockComment && trimmed[0] == '}' && lineDepth > 0) {

                lineDepth--;

            }

            if (trimmed[0] != '#') {

                out.append((size_t)(lineDepth * tw), ' ');

                out += trimmed;

            }

            else {

                out += trimmed;

            }



            bool inStr = false;

            char strCh = 0;

            for (size_t i = 0; i < trimmed.size(); i++) {

                char c = trimmed[i];

                if (inBlockComment) {

                    if (c == '*' && i + 1 < trimmed.size() && trimmed[i + 1] == '/') {

                        inBlockComment = false;

                        i++;

                    }

                    continue;

                }

                if (inStr) {

                    if (c == '\\' && i + 1 < trimmed.size()) { i++; continue; }

                    if (c == strCh) inStr = false;

                    continue;

                }

                if (c == '/' && i + 1 < trimmed.size() && trimmed[i + 1] == '/') break;

                if (c == '/' && i + 1 < trimmed.size() && trimmed[i + 1] == '*') {

                    inBlockComment = true;

                    i++;

                    continue;

                }

                if (c == '"' || c == '\'') { inStr = true; strCh = c; continue; }

                if (c == '{') depth++;

                else if (c == '}' && depth > 0) depth--;

            }

        }

        else {

            out += line;

        }

        if (li + 1 < rawLines.size()) out += eol;

    }



    int curLine = (int)SendEd(h, SCI_LINEFROMPOSITION, SendEd(h, SCI_GETCURRENTPOS));

    SendEd(h, SCI_BEGINUNDOACTION);

    SendEd(h, SCI_SETTEXT, 0, (LPARAM)out.c_str());

    SendEd(h, SCI_ENDUNDOACTION);

    int maxLines = (int)SendEd(h, SCI_GETLINECOUNT);

    if (curLine >= maxLines) curLine = maxLines - 1;

    if (curLine >= 0) SendEd(h, SCI_GOTOLINE, curLine);

    RelexTab(h);

    UpdateOutline(tabs[currentTab]);

    InvalidateRect(hwndMain, NULL, FALSE);

    LogToConsoleW(L"> Код отформатирован (Shift+Alt+F).", LOG_SUCCESS);

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

    switch (currentTheme) {

    case 1: // VS Code Light+

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

    case 2: // Cyber Neon / Dracula

        return {

            RGB(19, 17, 28),    // bg

            RGB(248, 248, 242), // fg

            RGB(15, 13, 23),    // lineNumBg

            RGB(98, 114, 164),  // lineNumFg

            RGB(68, 71, 90),    // sel

            RGB(31, 28, 46),    // caretLine

            RGB(98, 114, 164),  // comment

            RGB(241, 250, 140), // str

            RGB(189, 147, 249), // num

            RGB(255, 121, 198), // kw

            RGB(255, 121, 198), // ctrl

            RGB(139, 233, 253), // type

            RGB(80, 250, 123),  // func

            RGB(189, 147, 249), // cons

            RGB(255, 184, 108), // pre

            RGB(248, 248, 242), // op

            RGB(255, 121, 198), // tag

            RGB(80, 250, 123),  // attr

            RGB(52, 46, 78),    // guide

            RGB(75, 58, 110),   // braceBg

            RGB(241, 250, 140), // braceFg

            RGB(22, 20, 34),    // uiTopBar

            RGB(13, 12, 20),    // uiTabBar

            RGB(19, 17, 28),    // uiTabActive

            RGB(26, 23, 39),    // uiTabInactive

            RGB(38, 34, 56),    // uiTabHover

            RGB(55, 48, 82),    // uiBorder

            RGB(12, 10, 18),    // uiStatusBg

            RGB(215, 210, 235), // uiStatusFg

            RGB(189, 147, 249)  // uiAccent

        };

    case 3: // Monokai Pro

        return {

            RGB(39, 40, 34),

            RGB(248, 248, 242),

            RGB(33, 34, 29),

            RGB(144, 144, 138),

            RGB(73, 72, 62),

            RGB(52, 53, 45),

            RGB(117, 113, 94),

            RGB(230, 219, 116),

            RGB(174, 129, 255),

            RGB(249, 38, 114),

            RGB(249, 38, 114),

            RGB(102, 217, 239),

            RGB(166, 226, 46),

            RGB(174, 129, 255),

            RGB(253, 151, 31),

            RGB(248, 248, 242),

            RGB(249, 38, 114),

            RGB(166, 226, 46),

            RGB(70, 71, 60),

            RGB(82, 80, 66),

            RGB(253, 151, 31),

            RGB(34, 35, 30),

            RGB(26, 27, 23),

            RGB(39, 40, 34),

            RGB(45, 46, 39),

            RGB(62, 63, 53),

            RGB(70, 71, 60),

            RGB(24, 25, 21),

            RGB(220, 220, 210),

            RGB(253, 151, 31)

        };

    case 4: // One Dark Pro

        return {

            RGB(40, 44, 52),

            RGB(171, 178, 191),

            RGB(33, 37, 43),

            RGB(99, 109, 131),

            RGB(62, 68, 81),

            RGB(44, 49, 60),

            RGB(92, 99, 112),

            RGB(152, 195, 121),

            RGB(209, 154, 102),

            RGB(198, 120, 221),

            RGB(198, 120, 221),

            RGB(229, 192, 123),

            RGB(97, 175, 239),

            RGB(86, 182, 194),

            RGB(224, 108, 117),

            RGB(86, 182, 194),

            RGB(224, 108, 117),

            RGB(209, 154, 102),

            RGB(58, 64, 76),

            RGB(70, 78, 95),

            RGB(229, 192, 123),

            RGB(33, 37, 43),

            RGB(28, 31, 36),

            RGB(40, 44, 52),

            RGB(35, 39, 46),

            RGB(52, 58, 68),

            RGB(58, 64, 76),

            RGB(24, 27, 32),

            RGB(180, 188, 200),

            RGB(97, 175, 239)

        };

    default: // 0: VS Code Dark+

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

    SendEd(h, SCI_AUTOCSETIGNORECASE, TRUE, 0);

    SendEd(h, SCI_AUTOCSETMAXHEIGHT, 10, 0);

    SendEd(h, SCI_AUTOCSETDROPRESTOFWORD, TRUE, 0);

    SendEd(h, SCI_AUTOCSETAUTOHIDE, TRUE, 0);

    SendEd(h, SCI_AUTOCSETORDER, SC_ORDER_PERFORMSORT, 0);

    SendEd(h, SCI_SETWRAPMODE, g_wordWrap ? SC_WRAP_WORD : SC_WRAP_NONE, 0);

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

    SendEd(h, SCI_SETMARGINWIDTHN, MARGIN_MARKERS, 10);

    SendEd(h, SCI_SETMARGINMASKN, MARGIN_MARKERS, (1 << MARKER_UNSAVED) | (1 << MARKER_SAVED) | (1 << MARKER_ERROR));

    SendEd(h, SCI_SETMARGINBACKN, MARGIN_MARKERS, p.lineNumBg);



    SendEd(h, SCI_MARKERDEFINE, MARKER_UNSAVED, SC_MARK_LEFTRECT);

    SendEd(h, SCI_MARKERSETFORE, MARKER_UNSAVED, RGB(255, 140, 0));

    SendEd(h, SCI_MARKERSETBACK, MARKER_UNSAVED, RGB(255, 140, 0));



    SendEd(h, SCI_MARKERDEFINE, MARKER_SAVED, SC_MARK_LEFTRECT);

    SendEd(h, SCI_MARKERSETFORE, MARKER_SAVED, RGB(46, 204, 113));

    SendEd(h, SCI_MARKERSETBACK, MARKER_SAVED, RGB(46, 204, 113));



    SendEd(h, SCI_MARKERDEFINE, MARKER_ERROR, SC_MARK_CIRCLE);

    SendEd(h, SCI_MARKERSETFORE, MARKER_ERROR, RGB(255, 255, 255));

    SendEd(h, SCI_MARKERSETBACK, MARKER_ERROR, RGB(235, 65, 65));



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



    SendEd(h, SCI_INDICSETSTYLE, INDIC_WORD_HIGHLIGHT, INDIC_ROUNDBOX);

    SendEd(h, SCI_INDICSETFORE, INDIC_WORD_HIGHLIGHT, p.uiAccent);

    SendEd(h, SCI_INDICSETALPHA, INDIC_WORD_HIGHLIGHT, 45);

    SendEd(h, SCI_INDICSETOUTLINEALPHA, INDIC_WORD_HIGHLIGHT, 140);



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



    SendEd(h, SCI_SETWRAPMODE, g_wordWrap ? SC_WRAP_WORD : SC_WRAP_NONE, 0);

    UpdateLineMargin(h);

    RelexTab(h);

}



void ApplyThemeToAll() {

    isDarkTheme = (currentTheme != 1);

    ApplyDwmDarkTitleBar(hwndMain, isDarkTheme);

    for (auto& tab : tabs) {

        ApplyThemeToEditor(tab.hwndEditor);

    }

    Pal pal = GetPalette();

    COLORREF conBg = isDarkTheme ? pal.uiTabBar : RGB(248, 248, 250);

    SendEd(hwndConsole, SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM)"Consolas");

    SendEd(hwndConsole, SCI_STYLESETSIZE, STYLE_DEFAULT, 10);

    SendEd(hwndConsole, SCI_STYLESETBACK, STYLE_DEFAULT, conBg);

    SendEd(hwndConsole, SCI_STYLECLEARALL, 0, 0);



    SendEd(hwndConsole, SCI_STYLESETFORE, LOG_NORMAL, isDarkTheme ? pal.fg : RGB(40, 40, 40));

    SendEd(hwndConsole, SCI_STYLESETFORE, LOG_ERROR, RGB(255, 85, 85));

    SendEd(hwndConsole, SCI_STYLESETFORE, LOG_WARNING, RGB(255, 184, 77));

    SendEd(hwndConsole, SCI_STYLESETFORE, LOG_SUCCESS, RGB(80, 220, 120));

    SendEd(hwndConsole, SCI_STYLESETFORE, LOG_ACCENT, pal.uiAccent);



    if (hwndFindDlg) { ApplyDwmDarkTitleBar(hwndFindDlg, isDarkTheme); InvalidateRect(hwndFindDlg, NULL, TRUE); }

    if (hwndGotoDlg) { ApplyDwmDarkTitleBar(hwndGotoDlg, isDarkTheme); InvalidateRect(hwndGotoDlg, NULL, TRUE); }

    if (hwndColorsDlg) { ApplyDwmDarkTitleBar(hwndColorsDlg, isDarkTheme); InvalidateRect(hwndColorsDlg, NULL, TRUE); }

    if (hwndAboutDlg) { InvalidateRect(hwndAboutDlg, NULL, TRUE); }



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

    { L"{teamcolor}",    "{teamcolor}",  RGB(120, 190, 255), {0,0,0,0} },

    { L"{darkred}",      "{darkred}",    RGB(195, 35, 35),   {0,0,0,0} },

    { L"{fullred}",      "{fullred}",    RGB(255, 0, 0),     {0,0,0,0} },

    { L"{grey}",         "{grey}",       RGB(165, 170, 180), {0,0,0,0} },

    { L"{yellow}",       "{yellow}",     RGB(255, 240, 90),  {0,0,0,0} },

    { L"{orchid}",       "{orchid}",     RGB(218, 112, 214), {0,0,0,0} },

    { L"⚡ Команда !cmd",

      "RegConsoleCmd(\"sm_test\", Command_Test);\n\npublic Action Command_Test(int client, int args)\n{\n    if (!client || !IsClientInGame(client)) return Plugin_Handled;\n    CPrintToChat(client, \"{green}[Server]{default} Команда выполнена!\");\n    return Plugin_Handled;\n}\n",

      RGB(160, 120, 255), {0,0,0,0} },

    { L"⚡ Админ !cmd",

      "RegAdminCmd(\"sm_admincmd\", Command_Admin, ADMFLAG_GENERIC, \"Описание команды\");\n\npublic Action Command_Admin(int client, int args)\n{\n    if (!client || !IsClientInGame(client)) return Plugin_Handled;\n    return Plugin_Handled;\n}\n",

      RGB(160, 120, 255), {0,0,0,0} },

    { L"⚡ Событие Death",

      "HookEvent(\"player_death\", Event_PlayerDeath);\n\npublic void Event_PlayerDeath(Event event, const char[] name, bool dontBroadcast)\n{\n    int victim = GetClientOfUserId(event.GetInt(\"userid\"));\n    int attacker = GetClientOfUserId(event.GetInt(\"attacker\"));\n}\n",

      RGB(160, 120, 255), {0,0,0,0} },

    { L"⚡ Раунд Start",

      "HookEvent(\"round_start\", Event_RoundStart);\n\npublic void Event_RoundStart(Event event, const char[] name, bool dontBroadcast)\n{\n    \n}\n",

      RGB(160, 120, 255), {0,0,0,0} },

    { L"⚡ Цикл игроков",

      "for (int i = 1; i <= MaxClients; i++)\n{\n    if (IsClientInGame(i) && !IsFakeClient(i) && IsPlayerAlive(i))\n    {\n        \n    }\n}\n",

      RGB(160, 120, 255), {0,0,0,0} },

    { L"⚡ Меню (Menu)",

      "Menu menu = new Menu(MenuHandler_Main);\nmenu.SetTitle(\"Заголовок меню:\");\nmenu.AddItem(\"item1\", \"Пункт 1\");\nmenu.ExitButton = true;\nmenu.Display(client, MENU_TIME_FOREVER);\n\npublic int MenuHandler_Main(Menu menu, MenuAction action, int param1, int param2)\n{\n    if (action == MenuAction_Select)\n    {\n        char info[32];\n        menu.GetItem(param2, info, sizeof(info));\n    }\n    else if (action == MenuAction_End)\n    {\n        delete menu;\n    }\n    return 0;\n}\n",

      RGB(80, 210, 175), {0,0,0,0} },

    { L"⚡ Таймер (Timer)",

      "CreateTimer(1.0, Timer_Callback, GetClientUserId(client), TIMER_FLAG_NO_MAPCHANGE);\n\npublic Action Timer_Callback(Handle timer, int userid)\n{\n    int client = GetClientOfUserId(userid);\n    if (!client || !IsClientInGame(client)) return Plugin_Stop;\n    return Plugin_Continue;\n}\n",

      RGB(80, 210, 175), {0,0,0,0} },

    { L"⚡ Конвар (ConVar)",

      "ConVar g_cvEnabled;\ng_cvEnabled = CreateConVar(\"sm_plugin_enabled\", \"1\", \"Enable/disable plugin\", _, true, 0.0, true, 1.0);\nAutoExecConfig(true, \"plugin_config\");\n",

      RGB(80, 210, 175), {0,0,0,0} },

    { L"⚡ KeyValues",

      "char sPath[PLATFORM_MAX_PATH];\nBuildPath(Path_SM, sPath, sizeof(sPath), \"configs/my_config.cfg\");\nKeyValues kv = new KeyValues(\"Config\");\nif (kv.ImportFromFile(sPath))\n{\n    if (kv.GotoFirstSubKey())\n    {\n        do {\n            \n        } while (kv.GotoNextKey());\n    }\n}\ndelete kv;\n",

      RGB(80, 210, 175), {0,0,0,0} }

};



int g_hoverColorIdx = -1;



LRESULT CALLBACK ColorsDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    switch (uMsg) {

    case WM_CREATE: {

        ApplyDwmDarkTitleBar(hwnd, isDarkTheme);

        int rows = ((int)g_chatColors.size() + 2) / 3;

        int idx = 0;

        for (int row = 0; row < rows; row++) {

            for (int col = 0; col < 3; col++) {

                if (idx < (int)g_chatColors.size()) {

                    int x = 18 + col * 144;

                    int y = 48 + row * 38;

                    g_chatColors[idx].rc = { x, y, x + 136, y + 32 };

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



        HBRUSH hbrBg = CreateSolidBrush(isDarkTheme ? pal.uiTopBar : RGB(242, 242, 247));

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

        RECT rHdr = { 18, 10, rc.right - 40, 38 };

        DrawTextW(hdc, L"🎨 Цвета MoreColors и Быстрые заготовки SourcePawn:", -1, &rHdr, DT_LEFT | DT_VCENTER | DT_SINGLELINE);



        RECT rcCloseX = { rc.right - 34, 8, rc.right - 8, 34 };

        SetTextColor(hdc, RGB(235, 85, 85));

        DrawTextW(hdc, L"✕", -1, &rcCloseX, DT_CENTER | DT_VCENTER | DT_SINGLELINE);



        SelectObject(hdc, hUIFont);

        for (int i = 0; i < (int)g_chatColors.size(); i++) {

            RECT r = g_chatColors[i].rc;

            bool hov = (i == g_hoverColorIdx);

            HBRUSH hCard = CreateSolidBrush(hov ? pal.uiTabHover : (isDarkTheme ? pal.uiTabInactive : RGB(226, 226, 236)));

            FillRect(hdc, &r, hCard);

            DeleteObject(hCard);



            RECT rSwatch = { r.left + 8, r.top + 8, r.left + 20, r.bottom - 8 };

            HBRUSH hSw = CreateSolidBrush(g_chatColors[i].color);

            FillRect(hdc, &rSwatch, hSw);

            DeleteObject(hSw);



            SetTextColor(hdc, isDarkTheme ? g_chatColors[i].color : RGB(25, 25, 35));

            RECT rTxt = { r.left + 26, r.top, r.right - 4, r.bottom };

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

                    UpdateOutline(tabs[currentTab]);

                    InvalidateRect(hwndMain, NULL, FALSE);

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

    int rows = ((int)g_chatColors.size() + 2) / 3;

    int w = 460, h = 58 + rows * 38;

    int x = rcMain.left + (rcMain.right - rcMain.left - w) / 2;

    int y = rcMain.top + 80;

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

    ofn.lpstrFilter = L"SourcePawn Compiler (spcomp*.exe)\0spcomp*.exe\0Все файлы (*.*)\0*.*\0";

    ofn.lpstrFile = szFile;

    ofn.nMaxFile = MAX_PATH;

    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    if (GetOpenFileNameW(&ofn)) {

        wcscpy_s(szCompilerPath, szFile);

        SaveConfig();

        LogToConsoleW((std::wstring(L"> Выбран и сохранен компилятор: ") + szCompilerPath).c_str(), LOG_SUCCESS);

        InvalidateRect(hwndMain, NULL, FALSE);

        return true;

    }

    return false;

}



void SaveCurrentFile(bool forceSaveAs = false);



static std::wstring ConvertProcessLineToWide(const std::string& line) {

    if (line.empty()) return L"";

    int wLen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, line.data(), (int)line.size(), NULL, 0);

    UINT cp = CP_UTF8;

    if (wLen <= 0) {

        cp = CP_ACP;

        wLen = MultiByteToWideChar(CP_ACP, 0, line.data(), (int)line.size(), NULL, 0);

    }

    if (wLen <= 0) return L"";

    std::wstring res(wLen, 0);

    MultiByteToWideChar(cp, (cp == CP_UTF8) ? MB_ERR_INVALID_CHARS : 0, line.data(), (int)line.size(), &res[0], wLen);

    return res;

}



static int ExtractErrorLineNumber(const std::string& s) {

    size_t pos = 0;

    while (true) {

        size_t openP = s.find('(', pos);

        if (openP == std::string::npos) break;

        size_t closeP = s.find(')', openP + 1);

        if (closeP == std::string::npos) break;

        std::string inside = s.substr(openP + 1, closeP - openP - 1);

        if (!inside.empty()) {

            size_t endNum = 0;

            while (endNum < inside.size() && isdigit((unsigned char)inside[endNum])) endNum++;

            if (endNum > 0 && (endNum == inside.size() || inside.substr(endNum, 4) == " -- ")) {

                int val = atoi(inside.substr(0, endNum).c_str());

                if (val > 0) return val;

            }

        }

        pos = openP + 1;

    }

    size_t lp = s.find("line ");

    if (lp != std::string::npos) {

        int val = atoi(s.c_str() + lp + 5);

        if (val > 0) return val;

    }

    return -1;

}



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

    SendActive(SCI_MARKERDELETEALL, MARKER_ERROR, 0);



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

        AutoDetectSpcompSmart();

        if (wcslen(szCompilerPath) == 0 || GetFileAttributesW(szCompilerPath) == INVALID_FILE_ATTRIBUTES || !IsValidWindowsExe(szCompilerPath)) {

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



        std::wstring spcompDir = szCompilerPath;

        size_t spSlash = spcompDir.find_last_of(L"\\/");

        if (spSlash != std::wstring::npos) spcompDir = spcompDir.substr(0, spSlash);

        std::vector<std::wstring> incCandidates = {

            workDir + L"\\include",

            spcompDir + L"\\include",

            spcompDir + L"\\addons\\sourcemod\\scripting\\include",

            spcompDir + L"\\..\\include"

        };

        std::vector<std::wstring> addedIncs;

        for (const auto& incRaw : incCandidates) {

            wchar_t incFull[MAX_PATH] = { 0 };

            if (GetFullPathNameW(incRaw.c_str(), MAX_PATH, incFull, NULL) > 0) {

                DWORD attr = GetFileAttributesW(incFull);

                if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {

                    bool dup = false;

                    for (const auto& ex : addedIncs) {

                        if (_wcsicmp(ex.c_str(), incFull) == 0) { dup = true; break; }

                    }

                    if (!dup) {

                        addedIncs.push_back(incFull);

                        cmd += std::wstring(L" -i\"") + incFull + L"\"";

                    }

                }

            }

        }

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

        std::string rawOut;

        char buffer[2048];

        DWORD read;

        while (ReadFile(hRead, buffer, sizeof(buffer), &read, NULL) && read > 0) {

            rawOut.append(buffer, read);

        }

        CloseHandle(hRead);

        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD exitCode = 0;

        GetExitCodeProcess(pi.hProcess, &exitCode);

        CloseHandle(pi.hProcess);

        CloseHandle(pi.hThread);



        size_t lineStart = 0;

        while (lineStart < rawOut.size()) {

            size_t nl = rawOut.find('\n', lineStart);

            std::string line = (nl == std::string::npos) ? rawOut.substr(lineStart) : rawOut.substr(lineStart, nl - lineStart);

            if (!line.empty() && line.back() == '\r') line.pop_back();



            if (!line.empty()) {

                std::wstring wStr = ConvertProcessLineToWide(line);

                std::wstring wLow = wStr;

                std::transform(wLow.begin(), wLow.end(), wLow.begin(), [](wchar_t c) { return (wchar_t)towlower(c); });



                int lType = LOG_NORMAL;

                if (wLow.find(L"error") != std::wstring::npos || wLow.find(L"fatal") != std::wstring::npos || wLow.find(L"traceback") != std::wstring::npos) {

                    lType = LOG_ERROR;

                }

                else if (wLow.find(L"warning") != std::wstring::npos) {

                    lType = LOG_WARNING;

                }

                else if (wStr.find(L"Code size:") != std::wstring::npos || wStr.find(L"Data size:") != std::wstring::npos ||

                    wStr.find(L"Stack/heap size:") != std::wstring::npos || wStr.find(L"Total requirements:") != std::wstring::npos ||

                    wStr.find(L"Done.") != std::wstring::npos) {

                    lType = LOG_SUCCESS;

                }

                LogToConsoleW(wStr.c_str(), lType);



                if (lType == LOG_ERROR && currentTab >= 0) {

                    int errLine = ExtractErrorLineNumber(line);

                    if (errLine > 0) {

                        SendActive(SCI_MARKERADD, errLine - 1, MARKER_ERROR);

                    }

                }

            }

            if (nl == std::string::npos) break;

            lineStart = nl + 1;

        }



        if (exitCode == 0) {

            LogToConsoleW(L"[SUCCESS] Выполнение завершено (двойной клик по строке с ошибкой перенесет к коду).", LOG_SUCCESS);

        }

        else {

            wchar_t errMsg[192];

            swprintf(errMsg, 192, L"[ОШИБКА] Процесс завершился с кодом %lu (двойной клик по ошибке перенесет к коду).", (unsigned long)exitCode);

            LogToConsoleW(errMsg, LOG_ERROR);

        }

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

    int targetLine = ExtractErrorLineNumber(buf.data());



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

void ExecuteGotoLine() {

    if (!hwndGotoDlg) return;

    wchar_t numBuf[64] = { 0 };

    GetWindowTextW(hEditGotoBox, numBuf, 64);

    int line = _wtoi(numBuf);

    if (line > 0 && currentTab >= 0) {

        SendActive(SCI_GOTOLINE, line - 1);

        SendActive(SCI_SCROLLCARET);

        SetFocus(tabs[currentTab].hwndEditor);

    }

    DestroyWindow(hwndGotoDlg);

}



LRESULT CALLBACK GotoDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    switch (uMsg) {

    case WM_CREATE: {

        ApplyDwmDarkTitleBar(hwnd, isDarkTheme);

        Pal pal = GetPalette();

        if (hDarkEditBrush) DeleteObject(hDarkEditBrush);

        hDarkEditBrush = CreateSolidBrush(isDarkTheme ? pal.uiTabInactive : RGB(255, 255, 255));

        hEditGotoBox = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,

            20, 44, 220, 26, hwnd, (HMENU)201, hInst, NULL);

        SendMessage(hEditGotoBox, WM_SETFONT, (WPARAM)hUIFont, TRUE);

        SetFocus(hEditGotoBox);

        return 0;

    }

    case WM_CTLCOLOREDIT: {

        HDC hdc = (HDC)wParam;

        Pal pal = GetPalette();

        SetTextColor(hdc, isDarkTheme ? pal.fg : RGB(10, 10, 10));

        SetBkColor(hdc, isDarkTheme ? pal.uiTabInactive : RGB(255, 255, 255));

        return (LRESULT)hDarkEditBrush;

    }

    case WM_PAINT: {

        PAINTSTRUCT ps;

        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc;

        GetClientRect(hwnd, &rc);

        Pal pal = GetPalette();



        HBRUSH hbrBg = CreateSolidBrush(isDarkTheme ? pal.uiTopBar : RGB(242, 242, 247));

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



        HBRUSH hb2 = CreateSolidBrush(isDarkTheme ? pal.uiTabHover : RGB(220, 220, 230));

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

            ExecuteGotoLine();

            return 0;

        }

        if (PtInRect(&bCancel, pt)) {

            DestroyWindow(hwnd);

            return 0;

        }

        SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);

        return 0;

    }

    case WM_KEYDOWN:

        if (wParam == VK_ESCAPE) { DestroyWindow(hwnd); return 0; }

        if (wParam == VK_RETURN) { ExecuteGotoLine(); return 0; }

        break;

    case WM_DESTROY:

        hwndGotoDlg = NULL;

        return 0;

    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);

}



void ShowGotoLineDialog() {

    if (hwndGotoDlg) {

        SetForegroundWindow(hwndGotoDlg);

        SetFocus(hEditGotoBox);

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

    if (!hEditFindBox) return;

    wchar_t buf[256] = { 0 };

    GetWindowTextW(hEditFindBox, buf, 256);

    if (wcslen(buf) == 0) return;

    wcscpy_s(szFindWhat, buf);



    std::string utf8 = WideToUtf8(buf);

    int flags = g_findMatchCase ? SCFIND_MATCHCASE : 0;



    int curEnd = (int)SendActive(SCI_GETSELECTIONEND);

    SendActive(SCI_GOTOPOS, curEnd);

    SendActive(SCI_SEARCHANCHOR, 0, 0);

    int pos = (int)SendActive(SCI_SEARCHNEXT, flags, (LPARAM)utf8.c_str());

    if (pos == -1) {

        SendActive(SCI_GOTOPOS, 0);

        SendActive(SCI_SEARCHANCHOR, 0, 0);

        pos = (int)SendActive(SCI_SEARCHNEXT, flags, (LPARAM)utf8.c_str());

    }

    if (pos != -1) {

        SendActive(SCI_SCROLLCARET, 0, 0);

    }

    else {

        LogToConsoleW(L"[ПОИСК] Совпадений не найдено.", LOG_WARNING);

    }

}



void DoFindPrev() {

    if (!hEditFindBox) return;

    wchar_t buf[256] = { 0 };

    GetWindowTextW(hEditFindBox, buf, 256);

    if (wcslen(buf) == 0) return;

    wcscpy_s(szFindWhat, buf);



    std::string utf8 = WideToUtf8(buf);

    int flags = g_findMatchCase ? SCFIND_MATCHCASE : 0;



    int curStart = (int)SendActive(SCI_GETSELECTIONSTART);

    SendActive(SCI_GOTOPOS, curStart);

    SendActive(SCI_SEARCHANCHOR, 0, 0);

    int pos = (int)SendActive(SCI_SEARCHPREV, flags, (LPARAM)utf8.c_str());

    if (pos == -1) {

        int len = (int)SendActive(SCI_GETLENGTH);

        SendActive(SCI_GOTOPOS, len);

        SendActive(SCI_SEARCHANCHOR, 0, 0);

        pos = (int)SendActive(SCI_SEARCHPREV, flags, (LPARAM)utf8.c_str());

    }

    if (pos != -1) {

        SendActive(SCI_SCROLLCARET, 0, 0);

    }

    else {

        LogToConsoleW(L"[ПОИСК] Совпадений не найдено.", LOG_WARNING);

    }

}



void DoReplaceOne(bool all) {

    if (!hEditFindBox || !hEditReplaceBox) return;

    wchar_t fBuf[256] = { 0 }, rBuf[256] = { 0 };

    GetWindowTextW(hEditFindBox, fBuf, 256);

    GetWindowTextW(hEditReplaceBox, rBuf, 256);

    if (wcslen(fBuf) == 0) return;

    wcscpy_s(szFindWhat, fBuf);

    wcscpy_s(szReplaceWith, rBuf);



    std::string fUtf8 = WideToUtf8(fBuf);

    std::string rUtf8 = WideToUtf8(rBuf);

    int flags = g_findMatchCase ? SCFIND_MATCHCASE : 0;



    if (!all) {

        DoFindNext();

        if (SendActive(SCI_GETSELECTIONSTART) != SendActive(SCI_GETSELECTIONEND)) {

            SendActive(SCI_REPLACESEL, 0, (LPARAM)rUtf8.c_str());

        }

    }

    else {

        int count = 0;

        SendActive(SCI_BEGINUNDOACTION);

        SendActive(SCI_GOTOPOS, 0);

        while (true) {

            SendActive(SCI_SEARCHANCHOR, 0, 0);

            int pos = (int)SendActive(SCI_SEARCHNEXT, flags, (LPARAM)fUtf8.c_str());

            if (pos == -1) break;

            SendActive(SCI_REPLACESEL, 0, (LPARAM)rUtf8.c_str());

            SendActive(SCI_GOTOPOS, pos + (int)rUtf8.size());

            count++;

        }

        SendActive(SCI_ENDUNDOACTION);

        wchar_t msg[128];

        swprintf(msg, 128, L"[ЗАМЕНА] Заменено совпадений: %d", count);

        LogToConsoleW(msg, LOG_SUCCESS);

    }

}



LRESULT CALLBACK FindDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    switch (uMsg) {

    case WM_CREATE: {

        ApplyDwmDarkTitleBar(hwnd, isDarkTheme);

        Pal pal = GetPalette();

        if (hDarkEditBrush) DeleteObject(hDarkEditBrush);

        hDarkEditBrush = CreateSolidBrush(isDarkTheme ? pal.uiTabInactive : RGB(255, 255, 255));



        hEditFindBox = CreateWindowExW(0, L"EDIT", szFindWhat, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,

            110, 48, 286, 24, hwnd, (HMENU)101, hInst, NULL);

        hEditReplaceBox = CreateWindowExW(0, L"EDIT", szReplaceWith, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,

            110, 84, 330, 24, hwnd, (HMENU)102, hInst, NULL);



        SendMessage(hEditFindBox, WM_SETFONT, (WPARAM)hUIFont, TRUE);

        SendMessage(hEditReplaceBox, WM_SETFONT, (WPARAM)hUIFont, TRUE);

        SendMessage(hEditFindBox, EM_SETSEL, 0, -1);

        SetFocus(hEditFindBox);

        return 0;

    }

    case WM_CTLCOLOREDIT: {

        HDC hdc = (HDC)wParam;

        Pal pal = GetPalette();

        SetTextColor(hdc, isDarkTheme ? pal.fg : RGB(10, 10, 10));

        SetBkColor(hdc, isDarkTheme ? pal.uiTabInactive : RGB(255, 255, 255));

        return (LRESULT)hDarkEditBrush;

    }

    case WM_PAINT: {

        PAINTSTRUCT ps;

        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc;

        GetClientRect(hwnd, &rc);

        Pal pal = GetPalette();



        HBRUSH hbrBg = CreateSolidBrush(isDarkTheme ? pal.uiTopBar : RGB(242, 242, 247));

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

        RECT rcHdr = { 16, 12, rc.right - 40, 36 };

        DrawTextW(hdc, L"Найти и заменить (Kova Search)", -1, &rcHdr, DT_LEFT | DT_VCENTER | DT_SINGLELINE);



        RECT rcCloseX = { rc.right - 34, 8, rc.right - 8, 34 };

        SetTextColor(hdc, RGB(235, 85, 85));

        DrawTextW(hdc, L"✕", -1, &rcCloseX, DT_CENTER | DT_VCENTER | DT_SINGLELINE);



        SelectObject(hdc, hUIFont);

        SetTextColor(hdc, isDarkTheme ? RGB(190, 190, 205) : RGB(60, 60, 70));

        RECT r1 = { 16, 48, 105, 72 };

        DrawTextW(hdc, L"Что искать:", -1, &r1, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        RECT r2 = { 16, 84, 105, 108 };

        DrawTextW(hdc, L"Заменить на:", -1, &r2, DT_LEFT | DT_VCENTER | DT_SINGLELINE);



        RECT rCase = { 402, 48, 440, 72 };

        HBRUSH hbrCase = CreateSolidBrush(g_findMatchCase ? pal.uiAccent : (isDarkTheme ? pal.uiTabHover : RGB(220, 220, 230)));

        FillRect(hdc, &rCase, hbrCase);

        DeleteObject(hbrCase);

        SetTextColor(hdc, (g_findMatchCase || isDarkTheme) ? RGB(255, 255, 255) : RGB(20, 20, 20));

        DrawTextW(hdc, L"Aa", -1, &rCase, DT_CENTER | DT_VCENTER | DT_SINGLELINE);



        struct Btn { RECT r; const wchar_t* txt; bool accent; };

        Btn btns[] = {

            { { 16, 126, 114, 156 },  L"Найти пред.", false },

            { { 122, 126, 224, 156 }, L"Найти далее", true },

            { { 232, 126, 330, 156 }, L"Заменить",    false },

            { { 338, 126, 440, 156 }, L"Заменить всё",false }

        };

        for (auto& b : btns) {

            HBRUSH hb = CreateSolidBrush(b.accent ? pal.uiAccent : (isDarkTheme ? pal.uiTabHover : RGB(220, 220, 230)));

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

        RECT rc;

        GetClientRect(hwnd, &rc);

        RECT rcCloseX = { rc.right - 34, 8, rc.right - 8, 34 };

        RECT rCase = { 402, 48, 440, 72 };

        RECT bPrev = { 16, 126, 114, 156 };

        RECT bNext = { 122, 126, 224, 156 };

        RECT bRep = { 232, 126, 330, 156 };

        RECT bRepAll = { 338, 126, 440, 156 };

        if (PtInRect(&rcCloseX, pt)) { DestroyWindow(hwnd); return 0; }

        if (PtInRect(&rCase, pt)) { g_findMatchCase = !g_findMatchCase; InvalidateRect(hwnd, &rCase, FALSE); return 0; }

        if (PtInRect(&bPrev, pt)) { DoFindPrev(); return 0; }

        if (PtInRect(&bNext, pt)) { DoFindNext(); return 0; }

        if (PtInRect(&bRep, pt)) { DoReplaceOne(false); return 0; }

        if (PtInRect(&bRepAll, pt)) { DoReplaceOne(true); return 0; }

        SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);

        return 0;

    }

    case WM_DESTROY:

        hwndFindDlg = NULL;

        hEditFindBox = NULL;

        hEditReplaceBox = NULL;

        return 0;

    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);

}



void ShowCustomFindDialog() {

    if (currentTab >= 0) {

        int selS = (int)SendActive(SCI_GETSELECTIONSTART);

        int selE = (int)SendActive(SCI_GETSELECTIONEND);

        if (selE > selS && (selE - selS) < 250) {

            std::vector<char> selBuf((size_t)(selE - selS) + 1, 0);

            SendActive(SCI_GETSELTEXT, 0, (LPARAM)selBuf.data());

            if (strchr(selBuf.data(), '\n') == NULL && strchr(selBuf.data(), '\r') == NULL) {

                std::wstring wSel = Utf8ToWide(selBuf.data());

                if (!wSel.empty() && wSel.size() < 255) {

                    wcscpy_s(szFindWhat, wSel.c_str());

                }

            }

        }

    }



    if (hwndFindDlg) {

        if (hEditFindBox) {

            SetWindowTextW(hEditFindBox, szFindWhat);

            SendMessageW(hEditFindBox, EM_SETSEL, 0, -1);

        }

        SetForegroundWindow(hwndFindDlg);

        SetFocus(hEditFindBox);

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

    int w = 456, h = 172;

    int x = rcMain.right - w - 30;

    int y = rcMain.top + 85;



    hwndFindDlg = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, L"KovaCustomFindCls", L"",

        WS_POPUP | WS_VISIBLE, x, y, w, h, hwndMain, NULL, hInst, NULL);

}



// ============================================================

//  ЛОГ ОБНОВЛЕНИЙ И ОКНО "О ПРОГРАММЕ"

// ============================================================

bool g_hoverAboutCloseBtn = false;

bool g_hoverAboutLogBtn = false;

bool g_hoverChangelogCloseBtn = false;



LRESULT CALLBACK ChangelogWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    switch (uMsg) {

    case WM_CREATE:

        ApplyDwmDarkTitleBar(hwnd, isDarkTheme);

        return 0;

    case WM_PAINT: {

        PAINTSTRUCT ps;

        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc;

        GetClientRect(hwnd, &rc);

        Pal pal = GetPalette();



        HBRUSH hbrBg = CreateSolidBrush(isDarkTheme ? pal.uiTopBar : RGB(242, 242, 247));

        FillRect(hdc, &rc, hbrBg);

        DeleteObject(hbrBg);



        RECT rcTopBar = { 0, 0, rc.right, 4 };

        HBRUSH hbrAcc = CreateSolidBrush(pal.uiAccent);

        FillRect(hdc, &rcTopBar, hbrAcc);

        DeleteObject(hbrAcc);



        HPEN hPen = CreatePen(PS_SOLID, 1, pal.uiAccent);

        HGDIOBJ oldPen = SelectObject(hdc, hPen);

        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

        Rectangle(hdc, 0, 0, rc.right, rc.bottom);

        SelectObject(hdc, oldBr);

        SelectObject(hdc, oldPen);

        DeleteObject(hPen);



        SetBkMode(hdc, TRANSPARENT);

        SelectObject(hdc, hUIBoldFont);

        SetTextColor(hdc, isDarkTheme ? RGB(255, 255, 255) : RGB(20, 20, 25));

        RECT rHdr = { 22, 14, rc.right - 22, 42 };

        DrawTextW(hdc, L"📋 Лог обновлений Kova Code Studio", -1, &rHdr, DT_LEFT | DT_VCENTER | DT_SINGLELINE);



        SelectObject(hdc, hUIFont);

        SetTextColor(hdc, isDarkTheme ? RGB(205, 205, 218) : RGB(45, 45, 55));

        RECT rTxt = { 22, 50, rc.right - 22, rc.bottom - 62 };

        const wchar_t* logText =

            L"Версия v1.3.0 (Текущая):\n"

            L" • Встроенный FTP-менеджер (WinINet): подключение, навигация по папкам,\n"

            L"   скачивание и открытие файлов прямо в редакторе, отправка обратно,\n"

            L"   создание/удаление папок, автовыгрузка при Ctrl+S и сборка + .smx на FTP (F6)\n"

            L" • Подсветка всех вхождений слова под курсором и мультикурсор (Ctrl+Shift+L)\n"

            L" • Быстрое сворачивание/разворачивание всего кода (Ctrl+Shift+[ / ])\n"

            L" • Обновлен шаблон SourcePawn (Ctrl+J) и убран лишний текст с панели\n\n"

            L"Версия v1.2.0:\n"

            L" • Автопоиск компилятора spcomp64/spcomp и папок include + маркеры ошибок\n"

            L" • Боковая панель «Структура кода» (Ctrl+B) и автодополнение (Ctrl+Space)\n"

            L" • Автоформатирование кода (Shift+Alt+F), 5 тем оформления, поддержка .cfg/.vdf\n"

            L" • Защита несохраненных вкладок, поиск с учетом регистра (Aa), перенос строк (Alt+Z)\n\n"

            L"Версия v1.0.0:\n"

            L" • Нативный редактор на базе Scintilla, лексер SP/Python/HTML, Smart Run (F5)\n"

            L" • Палитра цветов MoreColors + генератор кода (Ctrl+M) и сохранение сессии";

        DrawTextW(hdc, logText, -1, &rTxt, DT_LEFT | DT_WORDBREAK);



        RECT rcBtn = { (rc.right - 160) / 2, rc.bottom - 48, (rc.right + 160) / 2, rc.bottom - 16 };

        HBRUSH hbrBtn = CreateSolidBrush(g_hoverChangelogCloseBtn ? pal.uiTabHover : pal.uiAccent);

        FillRect(hdc, &rcBtn, hbrBtn);

        DeleteObject(hbrBtn);

        SelectObject(hdc, hUIBoldFont);

        SetTextColor(hdc, RGB(255, 255, 255));

        DrawTextW(hdc, L"Закрыть", -1, &rcBtn, DT_CENTER | DT_VCENTER | DT_SINGLELINE);



        EndPaint(hwnd, &ps);

        return 0;

    }

    case WM_MOUSEMOVE: {

        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

        RECT rc; GetClientRect(hwnd, &rc);

        RECT rcBtn = { (rc.right - 160) / 2, rc.bottom - 48, (rc.right + 160) / 2, rc.bottom - 16 };

        bool hov = PtInRect(&rcBtn, pt) != 0;

        if (hov != g_hoverChangelogCloseBtn) {

            g_hoverChangelogCloseBtn = hov;

            InvalidateRect(hwnd, &rcBtn, FALSE);

        }

        return 0;

    }

    case WM_LBUTTONDOWN: {

        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

        RECT rc; GetClientRect(hwnd, &rc);

        RECT rcBtn = { (rc.right - 160) / 2, rc.bottom - 48, (rc.right + 160) / 2, rc.bottom - 16 };

        if (PtInRect(&rcBtn, pt)) { DestroyWindow(hwnd); return 0; }

        SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);

        return 0;

    }

    case WM_KEYDOWN:

        if (wParam == VK_ESCAPE || wParam == VK_RETURN) DestroyWindow(hwnd);

        return 0;

    case WM_DESTROY:

        hwndChangelogDlg = NULL;

        return 0;

    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);

}



void ShowChangelogWindow() {

    if (hwndChangelogDlg) {

        SetForegroundWindow(hwndChangelogDlg);

        return;

    }

    WNDCLASSW wc = { 0 };

    wc.lpfnWndProc = ChangelogWndProc;

    wc.hInstance = hInst;

    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    wc.lpszClassName = L"KovaChangelogCls";

    RegisterClassW(&wc);



    int w = 580, h = 450;

    RECT rcMain;

    GetWindowRect(hwndMain, &rcMain);

    int x = rcMain.left + (rcMain.right - rcMain.left - w) / 2;

    int y = rcMain.top + (rcMain.bottom - rcMain.top - h) / 2;

    hwndChangelogDlg = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, L"KovaChangelogCls", L"",

        WS_POPUP | WS_VISIBLE, x, y, w, h, hwndMain, NULL, hInst, NULL);

}



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

        DrawTextW(hdc, L"Kova Code Studio v1.3.0", -1, &rtTitle, DT_LEFT | DT_SINGLELINE);



        SelectObject(hdc, hUIBoldFont);

        SetTextColor(hdc, RGB(255, 155, 45));

        RECT rtSub = { 86, 52, rc.right - 28, 78 };

        DrawTextW(hdc, L"Release Build 1.3.0  |  Разработчик: ANR1S", -1, &rtSub, DT_LEFT | DT_SINGLELINE);



        SelectObject(hdc, hUIFont);

        SetTextColor(hdc, isDarkTheme ? RGB(205, 205, 218) : RGB(50, 50, 60));

        RECT rtDesc = { 28, 90, rc.right - 28, rc.bottom - 62 };

        const wchar_t* aboutText =

            L"Молниеносная нативная среда разработки (C++ Win32 API / Scintilla).\n"

            L"Создана специально для профессионального скриптинга SourcePawn (CS:S / CS:GO),\n"

            L"разработки Python-утилит, конфигов KeyValues (.cfg/.vdf) и Web-модулей.\n\n"

            L"Ключевые возможности v1.3.0:\n"

            L" • Встроенный FTP-менеджер: прямое открытие/редактирование файлов и выгрузка .smx (F6)\n"

            L" • Автопоиск spcomp64/spcomp + автоподключение include и маркеры ошибок\n"

            L" • Автодополнение SourceMod API (Ctrl+Space), автоформатирование (Shift+Alt+F)\n"

            L" • Боковая панель «Структура кода» (Ctrl+B), подсветка вхождений и мультикурсор (Ctrl+Shift+L)\n"

            L" • 5 тем оформления, палитра MoreColors + 9 шаблонов (Ctrl+M), защита вкладок\n"

            L" • Поиск/замена (Aa), перенос строк (Alt+Z), сворачивание кода (Ctrl+Shift+[/])";

        DrawTextW(hdc, aboutText, -1, &rtDesc, DT_LEFT | DT_WORDBREAK);



        RECT rcLogBtn = { rc.right / 2 - 170, rc.bottom - 52, rc.right / 2 - 10, rc.bottom - 18 };

        RECT rcBtn = { rc.right / 2 + 10, rc.bottom - 52, rc.right / 2 + 170, rc.bottom - 18 };



        HBRUSH hbrLog = CreateSolidBrush(g_hoverAboutLogBtn ? pal.uiTabHover : (isDarkTheme ? pal.uiTabInactive : RGB(215, 215, 226)));

        FillRect(hdc, &rcLogBtn, hbrLog);

        DeleteObject(hbrLog);

        SelectObject(hdc, hUIBoldFont);

        SetTextColor(hdc, isDarkTheme ? RGB(255, 255, 255) : RGB(20, 20, 30));

        DrawTextW(hdc, L"Лог обновлений", -1, &rcLogBtn, DT_CENTER | DT_VCENTER | DT_SINGLELINE);



        HBRUSH hbrBtn = CreateSolidBrush(g_hoverAboutCloseBtn ? RGB(145, 80, 255) : pal.uiAccent);

        FillRect(hdc, &rcBtn, hbrBtn);

        DeleteObject(hbrBtn);

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

        RECT rcLogBtn = { rc.right / 2 - 170, rc.bottom - 52, rc.right / 2 - 10, rc.bottom - 18 };

        RECT rcBtn = { rc.right / 2 + 10, rc.bottom - 52, rc.right / 2 + 170, rc.bottom - 18 };

        bool hovClose = PtInRect(&rcBtn, pt) != 0;

        bool hovLog = PtInRect(&rcLogBtn, pt) != 0;

        if (hovClose != g_hoverAboutCloseBtn || hovLog != g_hoverAboutLogBtn) {

            g_hoverAboutCloseBtn = hovClose;

            g_hoverAboutLogBtn = hovLog;

            RECT rBtns = { 0, rc.bottom - 56, rc.right, rc.bottom };

            InvalidateRect(hwnd, &rBtns, FALSE);

        }

        return 0;

    }

    case WM_LBUTTONDOWN: {

        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

        RECT rc;

        GetClientRect(hwnd, &rc);

        RECT rcLogBtn = { rc.right / 2 - 170, rc.bottom - 52, rc.right / 2 - 10, rc.bottom - 18 };

        RECT rcBtn = { rc.right / 2 + 10, rc.bottom - 52, rc.right / 2 + 170, rc.bottom - 18 };

        if (PtInRect(&rcLogBtn, pt)) {

            ShowChangelogWindow();

            return 0;

        }

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



    int w = 590, h = 445;

    RECT rcMain;

    GetWindowRect(hwndMain, &rcMain);

    int x = rcMain.left + (rcMain.right - rcMain.left - w) / 2;

    int y = rcMain.top + (rcMain.bottom - rcMain.top - h) / 2;



    hwndAboutDlg = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, L"KovaAboutClassV1", L"",

        WS_POPUP | WS_VISIBLE, x, y, w, h, hwndMain, NULL, hInst, NULL);

}



// ============================================================

//  ВСТРОЕННЫЙ FTP-МЕНЕДЖЕР (WININET)

// ============================================================

void OpenPath(const wchar_t* path);

void SaveCurrentFile(bool forceSaveAs);

void RunSmartDispatcher();



struct FtpEntry {

    std::wstring name;

    bool isDir;

    ULONGLONG size;

    FILETIME mtime;

};



std::vector<FtpEntry> g_ftpEntries;

HWND hComboFtpProfiles = NULL;

HWND hEditFtpHost = NULL, hEditFtpPort = NULL, hEditFtpUser = NULL, hEditFtpPass = NULL, hEditFtpDir = NULL, hEditFtpNewDir = NULL;

HWND hListFtp = NULL;

HFONT hFtpListFont = NULL;

std::wstring g_ftpStatusText = L"Статус: Отключен";



static std::wstring SanitizeFtpFileName(const std::wstring& raw) {

    size_t pos = raw.find_last_of(L"\\/");

    std::wstring name = (pos == std::wstring::npos) ? raw : raw.substr(pos + 1);

    std::wstring clean;

    for (wchar_t c : name) {

        if (c == L'<' || c == L'>' || c == L':' || c == L'"' || c == L'|' || c == L'?' || c == L'*' || c < 32) continue;

        clean.push_back(c);

    }

    if (clean == L"." || clean == L"..") return L"";

    return clean;

}



static void NormalizeFtpHost(wchar_t* hostBuf, size_t maxLen) {

    std::wstring s = hostBuf;

    while (!s.empty() && (s.front() == L' ' || s.front() == L'\t')) s.erase(s.begin());

    while (!s.empty() && (s.back() == L' ' || s.back() == L'\t' || s.back() == L'/')) s.pop_back();

    if (_wcsnicmp(s.c_str(), L"ftp://", 6) == 0) s = s.substr(6);

    size_t slash = s.find(L'/');

    if (slash != std::wstring::npos) s = s.substr(0, slash);

    wcsncpy_s(hostBuf, maxLen, s.c_str(), _TRUNCATE);

}



static void FtpReportError(const wchar_t* prefix) {

    DWORD err = GetLastError();

    wchar_t extBuf[512] = { 0 };

    DWORD extLen = 511;

    DWORD extErr = 0;

    std::wstring msg = std::wstring(L"[FTP ОШИБКА] ") + prefix + L" (код " + std::to_wstring(err) + L")";

    if (err == ERROR_INTERNET_EXTENDED_ERROR && InternetGetLastResponseInfoW(&extErr, extBuf, &extLen) && extLen > 0) {

        msg += L": ";

        msg += extBuf;

    }

    g_ftpStatusText = msg;

    LogToConsoleW(msg.c_str(), LOG_ERROR);

    if (hwndFtpDlg) InvalidateRect(hwndFtpDlg, NULL, FALSE);

}



void FtpDisconnect() {

    if (g_hFtp) { InternetCloseHandle(g_hFtp); g_hFtp = NULL; }

    if (g_hInet) { InternetCloseHandle(g_hInet); g_hInet = NULL; }

    g_ftpEntries.clear();

    if (hListFtp) ListView_DeleteAllItems(hListFtp);

    g_ftpStatusText = L"Статус: Отключен";

    if (hwndFtpDlg) InvalidateRect(hwndFtpDlg, NULL, FALSE);

}



static int GetSystemFileIconIndex(const std::wstring& name, bool isDir) {

    SHFILEINFOW sfi = { 0 };

    DWORD attrs = isDir ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;

    LPCWSTR probe = isDir ? L"folder" : name.c_str();

    if (SHGetFileInfoW(probe, attrs, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES)) {

        return sfi.iIcon;

    }

    return -1;

}



void FtpRefreshProfilesCombo(int selectIdx = -1) {

    if (!hComboFtpProfiles) return;

    SendMessageW(hComboFtpProfiles, CB_RESETCONTENT, 0, 0);

    for (size_t i = 0; i < g_ftpProfiles.size(); i++) {

        std::wstring label = std::to_wstring(i + 1) + L". " + g_ftpProfiles[i].name;

        SendMessageW(hComboFtpProfiles, CB_ADDSTRING, 0, (LPARAM)label.c_str());

    }

    if (selectIdx >= 0 && selectIdx < (int)g_ftpProfiles.size()) {

        SendMessageW(hComboFtpProfiles, CB_SETCURSEL, (WPARAM)selectIdx, 0);

    }

    else if (!g_ftpProfiles.empty()) {

        SendMessageW(hComboFtpProfiles, CB_SETCURSEL, 0, 0);

    }

}



void FtpApplySelectedProfile() {

    if (!hComboFtpProfiles) return;

    int sel = (int)SendMessageW(hComboFtpProfiles, CB_GETCURSEL, 0, 0);

    if (sel < 0 || sel >= (int)g_ftpProfiles.size()) return;

    const FtpProfile& p = g_ftpProfiles[sel];

    wcscpy_s(szFtpHost, p.host.c_str());

    g_ftpPort = p.port;

    wcscpy_s(szFtpUser, p.user.c_str());

    wcscpy_s(szFtpPass, p.pass.c_str());

    wcscpy_s(szFtpRemoteDir, p.remoteDir.c_str());

    g_ftpPassive = p.passive;



    wchar_t portStr[16];

    swprintf(portStr, 16, L"%d", g_ftpPort);

    if (hEditFtpHost) SetWindowTextW(hEditFtpHost, szFtpHost);

    if (hEditFtpPort) SetWindowTextW(hEditFtpPort, portStr);

    if (hEditFtpUser) SetWindowTextW(hEditFtpUser, szFtpUser);

    if (hEditFtpPass) SetWindowTextW(hEditFtpPass, szFtpPass);

    if (hEditFtpDir) SetWindowTextW(hEditFtpDir, szFtpRemoteDir);

    if (hwndFtpDlg) InvalidateRect(hwndFtpDlg, NULL, FALSE);

}



void FtpSaveCurrentProfile() {

    if (!hwndFtpDlg) return;

    if (hEditFtpHost) GetWindowTextW(hEditFtpHost, szFtpHost, 128);

    NormalizeFtpHost(szFtpHost, 128);

    if (hEditFtpHost) SetWindowTextW(hEditFtpHost, szFtpHost);

    if (wcslen(szFtpHost) == 0) {

        g_ftpStatusText = L"Введите хост перед сохранением профиля!";

        InvalidateRect(hwndFtpDlg, NULL, FALSE);

        return;

    }

    if (hEditFtpPort) {

        wchar_t pb[16] = { 0 };

        GetWindowTextW(hEditFtpPort, pb, 16);

        g_ftpPort = _wtoi(pb);

        if (g_ftpPort <= 0 || g_ftpPort > 65535) g_ftpPort = 21;

    }

    if (hEditFtpUser) GetWindowTextW(hEditFtpUser, szFtpUser, 128);

    if (hEditFtpPass) GetWindowTextW(hEditFtpPass, szFtpPass, 128);

    if (hEditFtpDir) GetWindowTextW(hEditFtpDir, szFtpRemoteDir, 512);



    FtpProfile prof;

    prof.host = szFtpHost;

    prof.port = g_ftpPort;

    prof.user = szFtpUser;

    prof.pass = szFtpPass;

    prof.remoteDir = (wcslen(szFtpRemoteDir) > 0) ? szFtpRemoteDir : L"/";

    prof.passive = g_ftpPassive;

    prof.name = (prof.user.empty() ? L"" : prof.user + L"@") + prof.host + L":" + std::to_wstring(prof.port);



    int targetIdx = -1;

    for (int i = 0; i < (int)g_ftpProfiles.size(); i++) {

        if (_wcsicmp(g_ftpProfiles[i].host.c_str(), prof.host.c_str()) == 0 &&

            g_ftpProfiles[i].port == prof.port &&

            _wcsicmp(g_ftpProfiles[i].user.c_str(), prof.user.c_str()) == 0) {

            g_ftpProfiles[i] = prof;

            targetIdx = i;

            break;

        }

    }

    if (targetIdx < 0) {

        if ((int)g_ftpProfiles.size() >= MAX_FTP_PROFILES) {

            MessageBoxW(hwndFtpDlg,

                L"Достигнут лимит сохраненных подключений (максимум 8).\nУдалите один из старых профилей.",

                L"FTP-менеджер", MB_OK | MB_ICONINFORMATION);

            return;

        }

        g_ftpProfiles.push_back(prof);

        targetIdx = (int)g_ftpProfiles.size() - 1;

    }



    SaveConfig();

    FtpRefreshProfilesCombo(targetIdx);

    g_ftpStatusText = std::wstring(L"Профиль сохранен (") + std::to_wstring(g_ftpProfiles.size()) + L"/8): " + prof.name;

    LogToConsoleW((std::wstring(L"> [FTP] Сохранен профиль: ") + prof.name).c_str(), LOG_SUCCESS);

    InvalidateRect(hwndFtpDlg, NULL, FALSE);

}



void FtpDeleteSelectedProfile() {

    if (!hComboFtpProfiles) return;

    int sel = (int)SendMessageW(hComboFtpProfiles, CB_GETCURSEL, 0, 0);

    if (sel < 0 || sel >= (int)g_ftpProfiles.size()) return;

    std::wstring removed = g_ftpProfiles[sel].name;

    g_ftpProfiles.erase(g_ftpProfiles.begin() + sel);

    SaveConfig();

    FtpRefreshProfilesCombo(sel > 0 ? sel - 1 : 0);

    g_ftpStatusText = L"Удален профиль: " + removed;

    InvalidateRect(hwndFtpDlg, NULL, FALSE);

}



void FtpRefreshList() {

    if (!g_hFtp || !hListFtp) return;

    wchar_t curDir[512] = L"/";

    DWORD curLen = 511;

    if (FtpGetCurrentDirectoryW(g_hFtp, curDir, &curLen)) {

        wcscpy_s(szFtpRemoteDir, curDir);

        if (hEditFtpDir) SetWindowTextW(hEditFtpDir, szFtpRemoteDir);

    }



    g_ftpEntries.clear();

    ListView_DeleteAllItems(hListFtp);



    if (wcscmp(szFtpRemoteDir, L"/") != 0 && wcslen(szFtpRemoteDir) > 0) {

        FtpEntry up = { L"..", true, 0, {0,0} };

        g_ftpEntries.push_back(up);

    }



    WIN32_FIND_DATAW fd = { 0 };

    HINTERNET hFind = FtpFindFirstFileW(g_hFtp, L"*", &fd, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);

    if (hFind) {

        std::vector<FtpEntry> dirs, files;

        do {

            if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0) continue;

            FtpEntry e;

            e.name = fd.cFileName;

            e.isDir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

            e.size = ((ULONGLONG)fd.nFileSizeHigh << 32) | fd.nFileSizeLow;

            e.mtime = fd.ftLastWriteTime;

            if (e.isDir) dirs.push_back(e);

            else files.push_back(e);

        } while (InternetFindNextFileW(hFind, &fd));

        InternetCloseHandle(hFind);



        auto cmpName = [](const FtpEntry& a, const FtpEntry& b) {

            return _wcsicmp(a.name.c_str(), b.name.c_str()) < 0;

        };

        std::sort(dirs.begin(), dirs.end(), cmpName);

        std::sort(files.begin(), files.end(), cmpName);



        for (const auto& d : dirs) g_ftpEntries.push_back(d);

        for (const auto& f : files) g_ftpEntries.push_back(f);

    }



    for (int i = 0; i < (int)g_ftpEntries.size(); i++) {

        const FtpEntry& e = g_ftpEntries[i];

        LVITEMW lvi = { 0 };

        lvi.mask = LVIF_TEXT | LVIF_IMAGE;

        lvi.iItem = i;

        lvi.iSubItem = 0;

        std::wstring dispName = (e.name == L"..") ? L".. (Наверх)" : e.name;

        lvi.pszText = (LPWSTR)dispName.c_str();

        lvi.iImage = GetSystemFileIconIndex(e.name, e.isDir);

        ListView_InsertItem(hListFtp, &lvi);



        std::wstring typeStr = L"Файл";

        if (e.name == L"..") typeStr = L"Переход вверх";

        else if (e.isDir) typeStr = L"Папка";

        else {

            size_t dot = e.name.find_last_of(L'.');

            if (dot != std::wstring::npos && dot + 1 < e.name.size()) {

                std::wstring ext = e.name.substr(dot + 1);

                std::transform(ext.begin(), ext.end(), ext.begin(), [](wchar_t c) { return (wchar_t)towupper(c); });

                typeStr = L"Файл ." + ext;

            }

        }

        ListView_SetItemText(hListFtp, i, 1, (LPWSTR)typeStr.c_str());



        std::wstring sizeStr = L"<DIR>";

        if (!e.isDir) {

            wchar_t szBuf[64];

            if (e.size < 1024) swprintf(szBuf, 64, L"%llu Б", e.size);

            else if (e.size < 1024 * 1024) swprintf(szBuf, 64, L"%.1f КБ", e.size / 1024.0);

            else swprintf(szBuf, 64, L"%.2f МБ", e.size / (1024.0 * 1024.0));

            sizeStr = szBuf;

        }

        ListView_SetItemText(hListFtp, i, 2, (LPWSTR)sizeStr.c_str());



        std::wstring dateStr = L"—";

        if (e.mtime.dwHighDateTime != 0 || e.mtime.dwLowDateTime != 0) {

            FILETIME lft = { 0 };

            SYSTEMTIME st = { 0 };

            if (FileTimeToLocalFileTime(&e.mtime, &lft) && FileTimeToSystemTime(&lft, &st)) {

                wchar_t dtBuf[64];

                swprintf(dtBuf, 64, L"%02d.%02d.%04d %02d:%02d", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute);

                dateStr = dtBuf;

            }

        }

        ListView_SetItemText(hListFtp, i, 3, (LPWSTR)dateStr.c_str());

    }



    g_ftpStatusText = std::wstring(L"Подключен: ") + szFtpHost + L"  |  " + szFtpRemoteDir + L"  (" + std::to_wstring(g_ftpEntries.size()) + L" эл.)";

    if (hwndFtpDlg) InvalidateRect(hwndFtpDlg, NULL, FALSE);

}



bool FtpConnect(bool readFromInputs) {

    if (readFromInputs && hwndFtpDlg) {

        if (hEditFtpHost) GetWindowTextW(hEditFtpHost, szFtpHost, 128);

        NormalizeFtpHost(szFtpHost, 128);

        if (hEditFtpHost) SetWindowTextW(hEditFtpHost, szFtpHost);

        if (hEditFtpPort) {

            wchar_t pBuf[16] = { 0 };

            GetWindowTextW(hEditFtpPort, pBuf, 16);

            g_ftpPort = _wtoi(pBuf);

            if (g_ftpPort <= 0 || g_ftpPort > 65535) g_ftpPort = 21;

        }

        if (hEditFtpUser) GetWindowTextW(hEditFtpUser, szFtpUser, 128);

        if (hEditFtpPass) GetWindowTextW(hEditFtpPass, szFtpPass, 128);

        if (hEditFtpDir) GetWindowTextW(hEditFtpDir, szFtpRemoteDir, 512);

    }

    if (wcslen(szFtpHost) == 0) {

        g_ftpStatusText = L"Укажите адрес хоста FTP!";

        LogToConsoleW(L"[FTP] Укажите хост сервера.", LOG_WARNING);

        if (hwndFtpDlg) InvalidateRect(hwndFtpDlg, NULL, FALSE);

        return false;

    }



    FtpDisconnect();

    SaveConfig();



    g_hInet = InternetOpenW(L"KovaCodeFTP/1.3", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

    if (!g_hInet) {

        FtpReportError(L"Не удалось инициализировать WinINet");

        return false;

    }

    DWORD timeout = 8000;

    InternetSetOptionW(g_hInet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));



    const wchar_t* userPtr = (wcslen(szFtpUser) > 0) ? szFtpUser : NULL;

    const wchar_t* passPtr = (wcslen(szFtpPass) > 0) ? szFtpPass : NULL;

    g_hFtp = InternetConnectW(g_hInet, szFtpHost, (INTERNET_PORT)g_ftpPort, userPtr, passPtr,

        INTERNET_SERVICE_FTP, g_ftpPassive ? INTERNET_FLAG_PASSIVE : 0, 0);

    if (!g_hFtp) {

        FtpReportError(L"Не удалось подключиться к FTP");

        FtpDisconnect();

        return false;

    }



    if (wcslen(szFtpRemoteDir) > 0 && wcscmp(szFtpRemoteDir, L"/") != 0) {

        FtpSetCurrentDirectoryW(g_hFtp, szFtpRemoteDir);

    }

    LogToConsoleW((std::wstring(L"> [FTP] Подключено к ") + szFtpHost + L":" + std::to_wstring(g_ftpPort)).c_str(), LOG_SUCCESS);

    if (hListFtp) FtpRefreshList();

    return true;

}



bool FtpUploadLocalFile(const std::wstring& localPath, const std::wstring& remotePathOrName) {

    if (!g_hFtp && !FtpConnect(false)) return false;

    if (FtpPutFileW(g_hFtp, localPath.c_str(), remotePathOrName.c_str(), FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_RELOAD, 0)) {

        LogToConsoleW((std::wstring(L"> [FTP] Выгружен на сервер: ") + remotePathOrName).c_str(), LOG_SUCCESS);

        if (hwndFtpDlg && hListFtp) FtpRefreshList();

        return true;

    }

    FtpReportError(L"Ошибка выгрузки файла на FTP");

    return false;

}



void FtpUploadActiveTab() {

    if (currentTab < 0) return;

    if (tabs[currentTab].filePath.find(L"Новый файл") != std::wstring::npos || SendActive(SCI_GETMODIFY)) {

        SaveCurrentFile(false);

        if (tabs[currentTab].filePath.find(L"Новый файл") != std::wstring::npos) return;

    }

    if (!g_hFtp && !FtpConnect(true)) return;



    const std::wstring& localPath = tabs[currentTab].filePath;

    const wchar_t* fn = wcsrchr(localPath.c_str(), L'\\');

    fn = fn ? fn + 1 : localPath.c_str();



    std::wstring targetRemote = tabs[currentTab].ftpRemotePath.empty() ? std::wstring(fn) : tabs[currentTab].ftpRemotePath;

    FtpUploadLocalFile(localPath, targetRemote);

}



void FtpDownloadAndOpenSelected() {

    if (!g_hFtp || !hListFtp) return;

    int sel = ListView_GetNextItem(hListFtp, -1, LVNI_SELECTED);

    if (sel < 0 || sel >= (int)g_ftpEntries.size()) return;

    const FtpEntry& e = g_ftpEntries[sel];

    if (e.isDir) {

        if (FtpSetCurrentDirectoryW(g_hFtp, e.name.c_str())) {

            FtpRefreshList();

        }

        else {

            FtpReportError(L"Не удалось открыть папку");

        }

        return;

    }



    std::wstring safeName = SanitizeFtpFileName(e.name);

    if (safeName.empty()) {

        LogToConsoleW(L"[FTP ОШИБКА] Небезопасное имя файла отклонено.", LOG_ERROR);

        return;

    }



    std::wstring cacheDir = std::wstring(szExeDir) + L"ftp_cache\\";

    CreateDirectoryW(cacheDir.c_str(), NULL);

    std::wstring localPath = cacheDir + safeName;



    if (FtpGetFileW(g_hFtp, e.name.c_str(), localPath.c_str(), FALSE, FILE_ATTRIBUTE_NORMAL,

        FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_RELOAD, 0)) {

        OpenPath(localPath.c_str());

        if (currentTab >= 0) {

            std::wstring rem = szFtpRemoteDir;

            while (!rem.empty() && rem.back() == L'/') rem.pop_back();

            rem += L"/" + e.name;

            tabs[currentTab].ftpRemotePath = rem;

        }

        LogToConsoleW((std::wstring(L"> [FTP] Скачан и открыт файл: ") + e.name).c_str(), LOG_SUCCESS);

    }

    else {

        FtpReportError(L"Не удалось скачать файл с FTP");

    }

}



void FtpDeleteSelectedItem(HWND owner) {

    if (!g_hFtp || !hListFtp) return;

    int sel = ListView_GetNextItem(hListFtp, -1, LVNI_SELECTED);

    if (sel < 0 || sel >= (int)g_ftpEntries.size() || g_ftpEntries[sel].name == L"..") return;

    const FtpEntry& e = g_ftpEntries[sel];

    std::wstring q = std::wstring(L"Удалить на FTP: «") + e.name + L"»?";

    if (MessageBoxW(owner, q.c_str(), L"FTP-менеджер", MB_YESNO | MB_ICONWARNING) == IDYES) {

        BOOL ok = e.isDir ? FtpRemoveDirectoryW(g_hFtp, e.name.c_str()) : FtpDeleteFileW(g_hFtp, e.name.c_str());

        if (ok) {

            LogToConsoleW((std::wstring(L"> [FTP] Удалено: ") + e.name).c_str(), LOG_WARNING);

            FtpRefreshList();

        }

        else FtpReportError(L"Не удалось удалить на FTP");

    }

}



void RunSmartAndUploadSmx() {

    if (currentTab < 0) return;

    RunSmartDispatcher();

    std::wstring file = tabs[currentTab].filePath;

    if (GetLang(file) != LANG_SP) return;

    std::wstring outSmx = file;

    size_t dot = outSmx.find_last_of(L'.');

    if (dot == std::wstring::npos) return;

    outSmx = outSmx.substr(0, dot) + L".smx";

    if (GetFileAttributesW(outSmx.c_str()) == INVALID_FILE_ATTRIBUTES) return;



    const wchar_t* fn = wcsrchr(outSmx.c_str(), L'\\');

    fn = fn ? fn + 1 : outSmx.c_str();

    if (!g_hFtp && wcslen(szFtpHost) == 0) {

        LogToConsoleW(L"[FTP] Настройте подключение в FTP-менеджере для выгрузки .smx.", LOG_WARNING);

        return;

    }

    FtpUploadLocalFile(outSmx, fn);

}



LRESULT CALLBACK FtpDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    switch (uMsg) {

    case WM_CREATE: {

        ApplyDwmDarkTitleBar(hwnd, isDarkTheme);

        Pal pal = GetPalette();

        if (hDarkEditBrush) DeleteObject(hDarkEditBrush);

        hDarkEditBrush = CreateSolidBrush(isDarkTheme ? pal.uiTabInactive : RGB(255, 255, 255));



        INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES };

        InitCommonControlsEx(&icc);



        hComboFtpProfiles = CreateWindowExW(0, L"COMBOBOX", L"",

            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,

            84, 38, 304, 220, hwnd, (HMENU)308, hInst, NULL);



        wchar_t portStr[16];

        swprintf(portStr, 16, L"%d", g_ftpPort);



        hEditFtpHost = CreateWindowExW(0, L"EDIT", szFtpHost, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,

            62, 72, 185, 24, hwnd, (HMENU)301, hInst, NULL);

        hEditFtpPort = CreateWindowExW(0, L"EDIT", portStr, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,

            295, 72, 54, 24, hwnd, (HMENU)302, hInst, NULL);

        hEditFtpUser = CreateWindowExW(0, L"EDIT", szFtpUser, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,

            405, 72, 110, 24, hwnd, (HMENU)303, hInst, NULL);

        hEditFtpPass = CreateWindowExW(0, L"EDIT", szFtpPass, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_PASSWORD | ES_AUTOHSCROLL,

            574, 72, 108, 24, hwnd, (HMENU)304, hInst, NULL);



        hEditFtpDir = CreateWindowExW(0, L"EDIT", szFtpRemoteDir, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,

            62, 104, 290, 24, hwnd, (HMENU)306, hInst, NULL);

        hEditFtpNewDir = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,

            510, 138, 108, 24, hwnd, (HMENU)307, hInst, NULL);



        hListFtp = CreateWindowExW(0, WC_LISTVIEWW, L"",

            WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS,

            16, 172, 666, 304, hwnd, (HMENU)305, hInst, NULL);



        ListView_SetExtendedListViewStyle(hListFtp, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

        ListView_SetBkColor(hListFtp, isDarkTheme ? pal.bg : RGB(255, 255, 255));

        ListView_SetTextBkColor(hListFtp, isDarkTheme ? pal.bg : RGB(255, 255, 255));

        ListView_SetTextColor(hListFtp, isDarkTheme ? pal.fg : RGB(20, 20, 25));



        SHFILEINFOW sfi = { 0 };

        HIMAGELIST hSysImg = (HIMAGELIST)SHGetFileInfoW(L"C:\\", 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

        if (hSysImg) ListView_SetImageList(hListFtp, hSysImg, LVSIL_SMALL);



        LVCOLUMNW col = { 0 };

        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;

        col.fmt = LVCFMT_LEFT;

        col.cx = 290; col.pszText = (LPWSTR)L"Имя"; ListView_InsertColumn(hListFtp, 0, &col);

        col.cx = 115; col.pszText = (LPWSTR)L"Тип"; ListView_InsertColumn(hListFtp, 1, &col);

        col.fmt = LVCFMT_RIGHT;

        col.cx = 95;  col.pszText = (LPWSTR)L"Размер"; ListView_InsertColumn(hListFtp, 2, &col);

        col.fmt = LVCFMT_LEFT;

        col.cx = 142; col.pszText = (LPWSTR)L"Изменён"; ListView_InsertColumn(hListFtp, 3, &col);



        if (hFtpListFont) DeleteObject(hFtpListFont);

        hFtpListFont = CreateFontW(15, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");

        SendMessageW(hComboFtpProfiles, WM_SETFONT, (WPARAM)hUIFont, TRUE);

        SendMessageW(hEditFtpHost, WM_SETFONT, (WPARAM)hUIFont, TRUE);

        SendMessageW(hEditFtpPort, WM_SETFONT, (WPARAM)hUIFont, TRUE);

        SendMessageW(hEditFtpUser, WM_SETFONT, (WPARAM)hUIFont, TRUE);

        SendMessageW(hEditFtpPass, WM_SETFONT, (WPARAM)hUIFont, TRUE);

        SendMessageW(hEditFtpDir, WM_SETFONT, (WPARAM)hUIFont, TRUE);

        SendMessageW(hEditFtpNewDir, WM_SETFONT, (WPARAM)hUIFont, TRUE);

        SendMessageW(hListFtp, WM_SETFONT, (WPARAM)hFtpListFont, TRUE);



        FtpRefreshProfilesCombo();

        if (g_hFtp) FtpRefreshList();

        return 0;

    }

    case WM_CTLCOLOREDIT:

    case WM_CTLCOLORLISTBOX: {

        HDC hdc = (HDC)wParam;

        Pal pal = GetPalette();

        SetTextColor(hdc, isDarkTheme ? pal.fg : RGB(15, 15, 20));

        SetBkColor(hdc, isDarkTheme ? pal.uiTabInactive : RGB(255, 255, 255));

        return (LRESULT)hDarkEditBrush;

    }

    case WM_COMMAND: {

        if (LOWORD(wParam) == 308 && HIWORD(wParam) == CBN_SELCHANGE) {

            FtpApplySelectedProfile();

            return 0;

        }

        break;

    }

    case WM_NOTIFY: {

        LPNMHDR pnm = (LPNMHDR)lParam;

        if (pnm->hwndFrom == hListFtp) {

            if (pnm->code == NM_DBLCLK) {

                FtpDownloadAndOpenSelected();

                return 0;

            }

            if (pnm->code == LVN_KEYDOWN) {

                LPNMLVKEYDOWN pk = (LPNMLVKEYDOWN)lParam;

                if (pk->wVKey == VK_RETURN) { FtpDownloadAndOpenSelected(); return 0; }

                if (pk->wVKey == VK_BACK) {

                    if (g_hFtp && FtpSetCurrentDirectoryW(g_hFtp, L"..")) FtpRefreshList();

                    return 0;

                }

                if (pk->wVKey == VK_DELETE) { FtpDeleteSelectedItem(hwnd); return 0; }

            }

        }

        break;

    }

    case WM_PAINT: {

        PAINTSTRUCT ps;

        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc; GetClientRect(hwnd, &rc);

        Pal pal = GetPalette();



        HBRUSH hbrBg = CreateSolidBrush(isDarkTheme ? pal.uiTopBar : RGB(242, 242, 247));

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

        SelectObject(hdc, oldBr);

        SelectObject(hdc, oldPen);

        DeleteObject(hPen);



        SetBkMode(hdc, TRANSPARENT);

        SelectObject(hdc, hUIBoldFont);

        SetTextColor(hdc, isDarkTheme ? RGB(255, 255, 255) : RGB(20, 20, 25));

        RECT rHdr = { 16, 8, rc.right - 40, 32 };

        DrawTextW(hdc, L"🌐 FTP-менеджер (до 8 сохраненных подключений)", -1, &rHdr, DT_LEFT | DT_VCENTER | DT_SINGLELINE);



        RECT rcCloseX = { rc.right - 34, 8, rc.right - 8, 32 };

        SetTextColor(hdc, RGB(235, 85, 85));

        DrawTextW(hdc, L"✕", -1, &rcCloseX, DT_CENTER | DT_VCENTER | DT_SINGLELINE);



        SelectObject(hdc, hUIFont);

        SetTextColor(hdc, isDarkTheme ? RGB(190, 190, 205) : RGB(55, 55, 65));

        RECT lProf = { 16, 38, 82, 62 };  DrawTextW(hdc, L"Профиль:", -1, &lProf, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        RECT lHost = { 16, 72, 60, 96 };  DrawTextW(hdc, L"Хост:", -1, &lHost, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        RECT lPort = { 252, 72, 293, 96 }; DrawTextW(hdc, L"Порт:", -1, &lPort, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        RECT lUser = { 354, 72, 403, 96 }; DrawTextW(hdc, L"Логин:", -1, &lUser, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        RECT lPass = { 520, 72, 572, 96 }; DrawTextW(hdc, L"Пароль:", -1, &lPass, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        RECT lDir = { 16, 104, 60, 128 }; DrawTextW(hdc, L"Путь:", -1, &lDir, DT_LEFT | DT_VCENTER | DT_SINGLELINE);



        struct FBtn { RECT r; const wchar_t* txt; bool accent; bool active; };

        FBtn btns[] = {

            { { 396, 38, 540, 62 },   L"Сохранить профиль", false, false },

            { { 548, 38, 682, 62 },   L"Удалить профиль",   false, false },

            { { 358, 104, 434, 128 }, L"Перейти",      false, false },

            { { 442, 104, 514, 128 }, g_ftpPassive ? L"PASV ✓" : L"PASV", false, g_ftpPassive },

            { { 522, 104, 682, 128 }, g_ftpAutoUploadOnSave ? L"Автовыгрузка Ctrl+S ✓" : L"Автовыгрузка Ctrl+S", false, g_ftpAutoUploadOnSave },

            { { 16, 138, 134, 164 },  g_hFtp ? L"Отключиться" : L"Подключиться", true, false },

            { { 142, 138, 224, 164 }, L"Обновить",     false, false },

            { { 232, 138, 314, 164 }, L"Вверх (..)",   false, false },

            { { 322, 138, 412, 164 }, L"Удалить",      false, false },

            { { 420, 138, 504, 164 }, L"Создать ▸",    false, false },

            { { 624, 138, 682, 164 }, L"Папка",        false, false },

            { { 16, 486, 178, 516 },  L"Скачать и открыть", false, false },

            { { 188, 486, 404, 516 }, L"Отправить текущий файл ↑", true, false },

            { { 414, 486, 682, 516 }, L"Сборка + .smx на FTP (F6)", false, false }

        };

        for (const auto& b : btns) {

            COLORREF bg = b.accent ? pal.uiAccent : (b.active ? RGB(46, 125, 85) : (isDarkTheme ? pal.uiTabHover : RGB(220, 220, 232)));

            HBRUSH hb = CreateSolidBrush(bg);

            FillRect(hdc, &b.r, hb);

            DeleteObject(hb);

            SetTextColor(hdc, (b.accent || b.active || isDarkTheme) ? RGB(255, 255, 255) : RGB(20, 20, 25));

            DrawTextW(hdc, b.txt, -1, (LPRECT)&b.r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        }



        SelectObject(hdc, hSmallFont);

        SetTextColor(hdc, g_hFtp ? RGB(80, 220, 120) : (isDarkTheme ? RGB(170, 170, 185) : RGB(70, 70, 85)));

        RECT rStat = { 16, 522, rc.right - 16, 546 };

        DrawTextW(hdc, g_ftpStatusText.c_str(), -1, &rStat, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);



        EndPaint(hwnd, &ps);

        return 0;

    }

    case WM_LBUTTONDOWN: {

        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

        RECT rc; GetClientRect(hwnd, &rc);

        RECT rcCloseX = { rc.right - 34, 8, rc.right - 8, 32 };

        if (PtInRect(&rcCloseX, pt)) { DestroyWindow(hwnd); return 0; }



        RECT rSaveProf = { 396, 38, 540, 62 };

        RECT rDelProf = { 548, 38, 682, 62 };

        RECT rGoDir = { 358, 104, 434, 128 };

        RECT rPasv = { 442, 104, 514, 128 };

        RECT rAuto = { 522, 104, 682, 128 };

        RECT rConn = { 16, 138, 134, 164 };

        RECT rRefr = { 142, 138, 224, 164 };

        RECT rUp = { 232, 138, 314, 164 };

        RECT rDel = { 322, 138, 412, 164 };

        RECT rMkDir1 = { 420, 138, 504, 164 };

        RECT rMkDir2 = { 624, 138, 682, 164 };

        RECT rDown = { 16, 486, 178, 516 };

        RECT rUpAct = { 188, 486, 404, 516 };

        RECT rBuildUp = { 414, 486, 682, 516 };



        if (PtInRect(&rSaveProf, pt)) { FtpSaveCurrentProfile(); return 0; }

        if (PtInRect(&rDelProf, pt)) { FtpDeleteSelectedProfile(); return 0; }

        if (PtInRect(&rPasv, pt)) { g_ftpPassive = !g_ftpPassive; SaveConfig(); InvalidateRect(hwnd, NULL, FALSE); return 0; }

        if (PtInRect(&rAuto, pt)) { g_ftpAutoUploadOnSave = !g_ftpAutoUploadOnSave; SaveConfig(); InvalidateRect(hwnd, NULL, FALSE); return 0; }

        if (PtInRect(&rConn, pt)) {

            if (g_hFtp) FtpDisconnect();

            else FtpConnect(true);

            return 0;

        }

        if (PtInRect(&rGoDir, pt)) {

            if (g_hFtp && hEditFtpDir) {

                wchar_t d[512] = { 0 };

                GetWindowTextW(hEditFtpDir, d, 512);

                if (FtpSetCurrentDirectoryW(g_hFtp, d)) FtpRefreshList();

                else FtpReportError(L"Не удалось перейти в папку");

            }

            return 0;

        }

        if (PtInRect(&rRefr, pt)) { FtpRefreshList(); return 0; }

        if (PtInRect(&rUp, pt)) {

            if (g_hFtp && FtpSetCurrentDirectoryW(g_hFtp, L"..")) FtpRefreshList();

            return 0;

        }

        if (PtInRect(&rMkDir1, pt) || PtInRect(&rMkDir2, pt)) {

            if (g_hFtp && hEditFtpNewDir) {

                wchar_t nd[128] = { 0 };

                GetWindowTextW(hEditFtpNewDir, nd, 128);

                std::wstring safeDir = SanitizeFtpFileName(nd);

                if (!safeDir.empty()) {

                    if (FtpCreateDirectoryW(g_hFtp, safeDir.c_str())) {

                        SetWindowTextW(hEditFtpNewDir, L"");

                        LogToConsoleW((std::wstring(L"> [FTP] Создана папка: ") + safeDir).c_str(), LOG_SUCCESS);

                        FtpRefreshList();

                    }

                    else FtpReportError(L"Не удалось создать папку");

                }

            }

            return 0;

        }

        if (PtInRect(&rDel, pt)) { FtpDeleteSelectedItem(hwnd); return 0; }

        if (PtInRect(&rDown, pt)) { FtpDownloadAndOpenSelected(); return 0; }

        if (PtInRect(&rUpAct, pt)) { FtpUploadActiveTab(); return 0; }

        if (PtInRect(&rBuildUp, pt)) { RunSmartAndUploadSmx(); return 0; }



        SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);

        return 0;

    }

    case WM_KEYDOWN:

        if (wParam == VK_ESCAPE) DestroyWindow(hwnd);

        return 0;

    case WM_DESTROY:

        if (hFtpListFont) { DeleteObject(hFtpListFont); hFtpListFont = NULL; }

        hwndFtpDlg = NULL;

        hComboFtpProfiles = hEditFtpHost = hEditFtpPort = hEditFtpUser = hEditFtpPass = hEditFtpDir = hEditFtpNewDir = hListFtp = NULL;

        return 0;

    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);

}



void ShowFtpManagerDialog() {

    if (hwndFtpDlg) {

        SetForegroundWindow(hwndFtpDlg);

        return;

    }

    WNDCLASSW wc = { 0 };

    wc.lpfnWndProc = FtpDlgProc;

    wc.hInstance = hInst;

    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    wc.lpszClassName = L"KovaFtpMgrCls";

    RegisterClassW(&wc);



    int w = 698, h = 554;

    RECT rcMain;

    GetWindowRect(hwndMain, &rcMain);

    int x = rcMain.left + (rcMain.right - rcMain.left - w) / 2;

    int y = rcMain.top + (rcMain.bottom - rcMain.top - h) / 2;

    hwndFtpDlg = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, L"KovaFtpMgrCls", L"",

        WS_POPUP | WS_VISIBLE, x, y, w, h, hwndMain, NULL, hInst, NULL);

}



// ============================================================

//  МЕНЮ И ТУЛБАР БЫСТРЫХ ДЕЙСТВИЙ

// ============================================================

void InitToolbarButtons() {

    g_toolbarButtons.clear();

    g_toolbarButtons.push_back({ IDM_FILE_NEW,          L"+ Новый файл",       { 10,  5, 112, 32 }, false });

    g_toolbarButtons.push_back({ IDM_FILE_OPEN,         L"Открыть",            { 118, 5, 194, 32 }, false });

    g_toolbarButtons.push_back({ IDM_FILE_SAVE,         L"Сохранить",          { 200, 5, 286, 32 }, false });

    g_toolbarButtons.push_back({ IDM_RUN_SMART,         L"▶ Запуск / Сборка",  { 294, 5, 434, 32 }, true });

    g_toolbarButtons.push_back({ IDM_SET_COMPILER,      L"⚙ spcomp",           { 442, 5, 530, 32 }, false });

    g_toolbarButtons.push_back({ IDM_EDIT_SNIPPET_SP,   L"★ Шаблон .SP",       { 536, 5, 640, 32 }, false });

    g_toolbarButtons.push_back({ IDM_EDIT_CHAT_COLORS,  L"🎨 Цвета",           { 646, 5, 730, 32 }, false });

    g_toolbarButtons.push_back({ IDM_EDIT_FIND_REPLACE, L"🔍 Поиск",           { 736, 5, 810, 32 }, false });

    g_toolbarButtons.push_back({ IDM_FTP_MANAGER,       L"🌐 FTP",             { 816, 5, 886, 32 }, false });

    g_toolbarButtons.push_back({ IDM_VIEW_OUTLINE,      L"📑 Структура",       { 892, 5, 988, 32 }, false });

    g_toolbarButtons.push_back({ IDM_THEME_TOGGLE,      L"◐ Тема",             { 994, 5, 1066, 32 }, false });

}



void RebuildMenu() {

    if (!hwndMain) return;

    HMENU hOld = GetMenu(hwndMain);

    HMENU hMenu = CreateMenu();



    HMENU hFile = CreatePopupMenu();

    AppendMenuW(hFile, MF_STRING, IDM_FILE_NEW, loc[currentLang][1]);

    AppendMenuW(hFile, MF_STRING, IDM_FILE_OPEN, loc[currentLang][2]);

    AppendMenuW(hFile, MF_STRING, IDM_FILE_SAVE, loc[currentLang][3]);

    AppendMenuW(hFile, MF_STRING, IDM_FILE_SAVEALL, loc[currentLang][16]);

    AppendMenuW(hFile, MF_STRING, IDM_FILE_SAVEAS, loc[currentLang][4]);

    AppendMenuW(hFile, MF_SEPARATOR, 0, NULL);

    AppendMenuW(hFile, MF_STRING, IDM_FTP_MANAGER, L"FTP-менеджер...\tCtrl+Alt+F");

    AppendMenuW(hFile, MF_STRING, IDM_FTP_UPLOAD_ACTIVE, L"Отправить текущий файл на FTP");

    AppendMenuW(hFile, MF_SEPARATOR, 0, NULL);

    AppendMenuW(hFile, MF_STRING, IDM_TAB_CLOSE_CURRENT, L"Закрыть текущую вкладку\tCtrl+W");

    AppendMenuW(hFile, MF_STRING, IDM_FILE_OPEN_FOLDER, loc[currentLang][5]);

    AppendMenuW(hFile, MF_SEPARATOR, 0, NULL);

    AppendMenuW(hFile, MF_STRING, IDM_FILE_EXIT, loc[currentLang][6]);



    HMENU hEdit = CreatePopupMenu();

    AppendMenuW(hEdit, MF_STRING, IDM_EDIT_FIND_REPLACE, loc[currentLang][8]);

    AppendMenuW(hEdit, MF_STRING, IDM_EDIT_GOTO_LINE, loc[currentLang][9]);

    AppendMenuW(hEdit, MF_STRING, IDM_EDIT_SELECT_ALL_OCC, L"Мультикурсор: выделить все совпадения\tCtrl+Shift+L");

    AppendMenuW(hEdit, MF_STRING, IDM_EDIT_DUPLICATE, loc[currentLang][10]);

    AppendMenuW(hEdit, MF_STRING, IDM_EDIT_COMMENT, L"Закомментировать строки\tCtrl+/");

    AppendMenuW(hEdit, MF_STRING, IDM_EDIT_LINE_UP, L"Переместить строку вверх\tAlt+Up");

    AppendMenuW(hEdit, MF_STRING, IDM_EDIT_LINE_DOWN, L"Переместить строку вниз\tAlt+Down");

    AppendMenuW(hEdit, MF_SEPARATOR, 0, NULL);

    AppendMenuW(hEdit, MF_STRING, IDM_EDIT_AUTOCOMPLETE, loc[currentLang][17]);

    AppendMenuW(hEdit, MF_STRING, IDM_EDIT_FORMAT_CODE, loc[currentLang][18]);

    AppendMenuW(hEdit, MF_STRING, IDM_EDIT_FOLD_ALL, L"Свернуть все блоки кода\tCtrl+Shift+[");

    AppendMenuW(hEdit, MF_STRING, IDM_EDIT_UNFOLD_ALL, L"Развернуть все блоки кода\tCtrl+Shift+]");

    AppendMenuW(hEdit, MF_SEPARATOR, 0, NULL);

    AppendMenuW(hEdit, MF_STRING, IDM_EDIT_SNIPPET_SP, loc[currentLang][11]);

    AppendMenuW(hEdit, MF_STRING, IDM_EDIT_CHAT_COLORS, L"Палитра цветов чата SourceMod\tCtrl+M");



    HMENU hView = CreatePopupMenu();

    AppendMenuW(hView, MF_STRING | (g_showOutline ? MF_CHECKED : MF_UNCHECKED), IDM_VIEW_OUTLINE, loc[currentLang][20]);

    AppendMenuW(hView, MF_STRING | (g_wordWrap ? MF_CHECKED : MF_UNCHECKED), IDM_VIEW_WORDWRAP, loc[currentLang][21]);

    AppendMenuW(hView, MF_SEPARATOR, 0, NULL);

    AppendMenuW(hView, MF_STRING, IDM_VIEW_ZOOM_IN, loc[currentLang][22]);

    AppendMenuW(hView, MF_STRING, IDM_VIEW_ZOOM_OUT, loc[currentLang][23]);

    AppendMenuW(hView, MF_STRING, IDM_VIEW_ZOOM_RESET, L"Сбросить масштаб\tCtrl+0");



    HMENU hBuild = CreatePopupMenu();

    AppendMenuW(hBuild, MF_STRING, IDM_RUN_SMART, loc[currentLang][13]);

    AppendMenuW(hBuild, MF_STRING, IDM_RUN_AND_UPLOAD, L"Собрать и выгрузить .smx на FTP\tF6");

    AppendMenuW(hBuild, MF_SEPARATOR, 0, NULL);

    AppendMenuW(hBuild, MF_STRING, IDM_SET_COMPILER, loc[currentLang][14]);

    AppendMenuW(hBuild, MF_STRING, IDM_CLEAR_CONSOLE, L"Очистить консоль");



    HMENU hSettings = CreatePopupMenu();

    HMENU hTheme = CreatePopupMenu();

    AppendMenuW(hTheme, MF_STRING | (currentTheme == 0 ? MF_CHECKED : MF_UNCHECKED), IDM_THEME_DARK, L"VS Code Dark+");

    AppendMenuW(hTheme, MF_STRING | (currentTheme == 1 ? MF_CHECKED : MF_UNCHECKED), IDM_THEME_LIGHT, L"VS Code Light+");

    AppendMenuW(hTheme, MF_STRING | (currentTheme == 2 ? MF_CHECKED : MF_UNCHECKED), IDM_THEME_CYBER, L"Cyber Neon / Dracula");

    AppendMenuW(hTheme, MF_STRING | (currentTheme == 3 ? MF_CHECKED : MF_UNCHECKED), IDM_THEME_MONOKAI, L"Monokai Pro");

    AppendMenuW(hTheme, MF_STRING | (currentTheme == 4 ? MF_CHECKED : MF_UNCHECKED), IDM_THEME_ONEDARK, L"One Dark Pro");



    HMENU hLang = CreatePopupMenu();

    AppendMenuW(hLang, MF_STRING | (currentLang == 0 ? MF_CHECKED : MF_UNCHECKED), IDM_LANG_RU, L"Русский");

    AppendMenuW(hLang, MF_STRING | (currentLang == 1 ? MF_CHECKED : MF_UNCHECKED), IDM_LANG_EN, L"English");

    AppendMenuW(hLang, MF_STRING | (currentLang == 2 ? MF_CHECKED : MF_UNCHECKED), IDM_LANG_DE, L"Deutsch");

    AppendMenuW(hLang, MF_STRING | (currentLang == 3 ? MF_CHECKED : MF_UNCHECKED), IDM_LANG_FR, L"Français");

    AppendMenuW(hLang, MF_STRING | (currentLang == 4 ? MF_CHECKED : MF_UNCHECKED), IDM_LANG_IT, L"Italiano");



    AppendMenuW(hSettings, MF_POPUP, (UINT_PTR)hTheme, L"Тема оформления");

    AppendMenuW(hSettings, MF_POPUP, (UINT_PTR)hLang, L"Language / Язык");

    AppendMenuW(hSettings, MF_SEPARATOR, 0, NULL);

    AppendMenuW(hSettings, MF_STRING, IDM_CHANGELOG, L"Лог обновлений");

    AppendMenuW(hSettings, MF_STRING, IDM_ABOUT, L"О программе Kova Code (v1.3.0)");



    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hFile, loc[currentLang][0]);

    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hEdit, loc[currentLang][7]);

    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hView, loc[currentLang][19]);

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



    int outlineW = (g_showOutline && rc.right >= 520) ? g_outlineWidth : 0;

    int editorWidth = rc.right - outlineW;

    if (editorWidth < 200) editorWidth = rc.right;



    // Синхронное изменение размеров окон за один проход без разрывов при растягивании

    HDWP hdwp = BeginDeferWindowPos(2);

    if (hdwp) {

        hdwp = DeferWindowPos(hdwp, hwndConsole, NULL, 0, consoleTop, rc.right, g_consoleHeight,

            SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);

        if (currentTab >= 0 && currentTab < (int)tabs.size()) {

            hdwp = DeferWindowPos(hdwp, tabs[currentTab].hwndEditor, NULL, 0, editorTop, editorWidth, editorHeight,

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

        g_outlineScroll = 0;

        g_hoverOutlineIdx = -1;

        UpdateOutline(tabs[currentTab]);

        ResizeLayout();

        SetFocus(tabs[currentTab].hwndEditor);

    }

}



void CreateOrReplaceTab(const wchar_t* path = L"Новый файл.txt");



bool ConfirmCloseTab(int index) {

    if (index < 0 || index >= (int)tabs.size()) return true;

    if (!tabs[index].isModified) return true;



    const wchar_t* fn = wcsrchr(tabs[index].filePath.c_str(), L'\\');

    fn = fn ? fn + 1 : tabs[index].filePath.c_str();

    std::wstring msg = std::wstring(L"Сохранить изменения в файле «") + fn + L"» перед закрытием?";

    int res = MessageBoxW(hwndMain, msg.c_str(), L"Kova Code Studio", MB_YESNOCANCEL | MB_ICONQUESTION);

    if (res == IDCANCEL) return false;

    if (res == IDYES) {

        SwitchTab(index);

        SaveCurrentFile(false);

        if (tabs[index].isModified) return false;

    }

    return true;

}



void CloseTab(int index) {

    if (index < 0 || index >= (int)tabs.size()) return;

    if (!ConfirmCloseTab(index)) return;

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

    for (int i = (int)tabs.size() - 1; i >= 0; i--) {

        if (i == keepIndex) continue;

        if (!ConfirmCloseTab(i)) {

            SwitchTab(currentTab);

            return;

        }

        DestroyWindow(tabs[i].hwndEditor);

        tabs.erase(tabs.begin() + i);

        if (i < keepIndex) keepIndex--;

    }

    currentTab = 0;

    SwitchTab(0);

}



void SaveAllFiles() {

    if (tabs.empty()) return;

    int orig = currentTab;

    int saved = 0;

    for (int i = 0; i < (int)tabs.size(); i++) {

        if (tabs[i].isModified) {

            SwitchTab(i);

            SaveCurrentFile(false);

            if (!tabs[i].isModified) saved++;

        }

    }

    if (orig >= 0 && orig < (int)tabs.size()) SwitchTab(orig);

    wchar_t msg[128];

    swprintf(msg, 128, L"> Сохранено измененных вкладок: %d.", saved);

    LogToConsoleW(msg, LOG_SUCCESS);

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

    TabData td;

    td.hwndEditor = hEd;

    td.filePath = path;

    td.lang = GetLang(path);

    td.isModified = false;

    td.rcTab = { 0,0,0,0 };

    td.rcClose = { 0,0,0,0 };

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

    if (size < 0 || size > 64 * 1024 * 1024) {

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

        UpdateOutline(tabs[currentTab]);

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

        L"Все поддерживаемые (*.sp;*.inc;*.cfg;*.ini;*.py;*.html;*.cpp;*.txt)\0*.txt;*.sp;*.inc;*.cfg;*.ini;*.vdf;*.kv;*.json;*.bat;*.cmd;*.py;*.html;*.php;*.css;*.js;*.cpp;*.h\0"

        L"SourcePawn (*.sp;*.inc)\0*.sp;*.inc\0"

        L"Конфиги / KeyValues (*.cfg;*.ini;*.vdf;*.kv)\0*.cfg;*.ini;*.vdf;*.kv;*.json\0"

        L"Текстовые файлы (*.txt)\0*.txt\0"

        L"Python (*.py)\0*.py\0"

        L"Web (*.html;*.php;*.css;*.js)\0*.html;*.php;*.css;*.js\0"

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

            L"Конфиг / KeyValues (*.cfg;*.ini)\0*.cfg;*.ini;*.vdf\0"

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

            case 4: file += L".cfg";  break;

            case 5: file += L".py";   break;

            case 6: file += L".html"; break;

            case 7: file += L".cpp";  break;

            default: break;

            }

        }



        tabs[currentTab].filePath = file;

        tabs[currentTab].lang = GetLang(file);

        RelexTab(tabs[currentTab].hwndEditor);

        UpdateOutline(tabs[currentTab]);

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

        UpdateOutline(tabs[currentTab]);

        SaveConfig();

        InvalidateRect(hwndMain, NULL, FALSE);

        if (g_ftpAutoUploadOnSave && !tabs[currentTab].ftpRemotePath.empty() && g_hFtp) {

            FtpUploadLocalFile(file, tabs[currentTab].ftpRemotePath);

        }

    }

    else {

        LogToConsoleW(L"[ОШИБКА] Не удалось сохранить файл на диск.", LOG_ERROR);

    }

}



// ============================================================

//  ОТРИСОВКА ТУЛБАРА, КАСТОМНОЙ ПОЛОСЫ ВКЛАДОК, OUTLINE И СТАТУС-БАРА

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



    // 1. ВЕРХНИЙ ТУЛБАР БЫСТРЫХ ДЕЙСТВИЙ

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

            int r = (int)GetRValue(pal.uiAccent) + 22; if (r > 255) r = 255;

            int g = (int)GetGValue(pal.uiAccent) + 22; if (g > 255) g = 255;

            int bl = (int)GetBValue(pal.uiAccent) + 22; if (bl > 255) bl = 255;

            btnBg = hov ? RGB(r, g, bl) : pal.uiAccent;

        }

        else {

            btnBg = hov ? pal.uiTabHover : (isDarkTheme ? pal.uiTabInactive : RGB(225, 225, 234));

        }

        HBRUSH hbrB = CreateSolidBrush(btnBg);

        FillRect(hdc, &b.rc, hbrB);

        DeleteObject(hbrB);



        if (b.id == IDM_VIEW_OUTLINE && g_showOutline) {

            RECT rInd = { b.rc.left, b.rc.bottom - 2, b.rc.right, b.rc.bottom };

            HBRUSH hInd = CreateSolidBrush(pal.uiAccent);

            FillRect(hdc, &rInd, hInd);

            DeleteObject(hInd);

        }



        SetTextColor(hdc, (b.isAccent || isDarkTheme) ? RGB(255, 255, 255) : RGB(25, 25, 30));

        DrawTextW(hdc, b.label, -1, (LPRECT)&b.rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

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



    int editorTop = g_toolbarHeight + g_tabHeight;

    int consoleBottom = rc.bottom - g_statusHeight;

    int consoleTop = consoleBottom - g_consoleHeight;

    int hdrTop = consoleTop - g_consoleHeaderHeight;



    // 2.5 БОКОВАЯ ПАНЕЛЬ «СТРУКТУРА КОДА / OUTLINE» (СПРАВА)

    if (g_showOutline && rc.right >= 520) {

        int oLeft = rc.right - g_outlineWidth;

        RECT rcOut = { oLeft, editorTop, rc.right, hdrTop };

        HBRUSH hbrOut = CreateSolidBrush(pal.lineNumBg);

        FillRect(hdc, &rcOut, hbrOut);

        DeleteObject(hbrOut);



        MoveToEx(hdc, oLeft, editorTop, NULL);

        LineTo(hdc, oLeft, hdrTop);



        int oHdrBottom = editorTop + 28;

        RECT rcOutHdr = { oLeft + 1, editorTop, rc.right, oHdrBottom };

        HBRUSH hbrOutHdr = CreateSolidBrush(pal.uiTopBar);

        FillRect(hdc, &rcOutHdr, hbrOutHdr);

        DeleteObject(hbrOutHdr);



        MoveToEx(hdc, oLeft + 1, oHdrBottom - 1, NULL);

        LineTo(hdc, rc.right, oHdrBottom - 1);



        int symCount = (currentTab >= 0) ? (int)tabs[currentTab].outline.size() : 0;

        wchar_t oHdrTxt[64];

        swprintf(oHdrTxt, 64, L"СТРУКТУРА (%d)", symCount);

        SelectObject(hdc, hSmallFont);

        SetTextColor(hdc, isDarkTheme ? RGB(195, 195, 215) : RGB(40, 40, 55));

        RECT rcOutHdrTxt = { oLeft + 10, editorTop, rc.right - 30, oHdrBottom };

        DrawTextW(hdc, oHdrTxt, -1, &rcOutHdrTxt, DT_LEFT | DT_VCENTER | DT_SINGLELINE);



        g_rcOutlineClose = { rc.right - 26, editorTop + 4, rc.right - 6, oHdrBottom - 4 };

        if (g_hoverOutlineClose) {

            HBRUSH hbrOC = CreateSolidBrush(RGB(215, 65, 65));

            FillRect(hdc, &g_rcOutlineClose, hbrOC);

            DeleteObject(hbrOC);

        }

        SetTextColor(hdc, g_hoverOutlineClose ? RGB(255, 255, 255) : (isDarkTheme ? RGB(170, 170, 185) : RGB(80, 80, 95)));

        DrawTextW(hdc, L"✕", -1, &g_rcOutlineClose, DT_CENTER | DT_VCENTER | DT_SINGLELINE);



        if (currentTab >= 0) {

            auto& items = tabs[currentTab].outline;

            int listTop = oHdrBottom + 2;

            int listH = hdrTop - listTop;

            int rowH = 24;

            int totalRows = (int)items.size();

            int visibleRows = (listH > 0) ? (listH / rowH) : 0;

            int maxScroll = totalRows - visibleRows;

            if (maxScroll < 0) maxScroll = 0;

            if (g_outlineScroll > maxScroll) g_outlineScroll = maxScroll;

            if (g_outlineScroll < 0) g_outlineScroll = 0;



            int curEdLine = (int)SendActive(SCI_LINEFROMPOSITION, SendActive(SCI_GETCURRENTPOS));

            int activeIdx = -1;

            for (int i = 0; i < totalRows; i++) {

                if (items[i].line <= curEdLine) activeIdx = i;

            }



            int savedDC = SaveDC(hdc);

            IntersectClipRect(hdc, oLeft + 1, listTop, rc.right, hdrTop);



            if (items.empty()) {

                SelectObject(hdc, hSmallFont);

                SetTextColor(hdc, pal.lineNumFg);

                RECT rcEmpty = { oLeft + 10, listTop + 12, rc.right - 10, listTop + 40 };

                DrawTextW(hdc, L"Символы не найдены", -1, &rcEmpty, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            }

            else {

                for (int i = 0; i < totalRows; i++) {

                    int y = listTop + (i - g_outlineScroll) * rowH;

                    if (y + rowH <= listTop || y >= hdrTop) {

                        items[i].rc = { 0, 0, 0, 0 };

                        continue;

                    }

                    RECT rr = { oLeft + 1, y, rc.right, y + rowH };

                    items[i].rc = rr;



                    bool isAct = (i == activeIdx);

                    bool isHov = (i == g_hoverOutlineIdx);

                    if (isHov || isAct) {

                        HBRUSH hbrRow = CreateSolidBrush(isHov ? pal.uiTabHover : pal.caretLine);

                        FillRect(hdc, &rr, hbrRow);

                        DeleteObject(hbrRow);

                    }

                    if (isAct) {

                        RECT rBar = { rr.left, rr.top + 2, rr.left + 3, rr.bottom - 2 };

                        HBRUSH hbrBar = CreateSolidBrush(pal.uiAccent);

                        FillRect(hdc, &rBar, hbrBar);

                        DeleteObject(hbrBar);

                    }



                    const wchar_t* tag = L"fn";

                    COLORREF tagClr = pal.func;

                    if (items[i].kind == 1) { tag = L"ev"; tagClr = pal.pre; }

                    else if (items[i].kind == 2) { tag = L"#"; tagClr = pal.type; }

                    else if (items[i].kind == 3) { tag = L"§"; tagClr = pal.tag; }



                    SelectObject(hdc, hSmallFont);

                    SetTextColor(hdc, tagClr);

                    RECT rTag = { rr.left + 8, rr.top, rr.left + 28, rr.bottom };

                    DrawTextW(hdc, tag, -1, &rTag, DT_CENTER | DT_VCENTER | DT_SINGLELINE);



                    wchar_t lnBuf[16];

                    swprintf(lnBuf, 16, L":%d", items[i].line + 1);

                    SetTextColor(hdc, pal.lineNumFg);

                    RECT rLn = { rr.right - 44, rr.top, rr.right - 6, rr.bottom };

                    DrawTextW(hdc, lnBuf, -1, &rLn, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);



                    SelectObject(hdc, isAct ? hUIBoldFont : hUIFont);

                    SetTextColor(hdc, isAct ? (isDarkTheme ? RGB(255, 255, 255) : RGB(15, 15, 20)) : pal.fg);

                    RECT rLbl = { rr.left + 30, rr.top, rr.right - 46, rr.bottom };

                    DrawTextW(hdc, items[i].label.c_str(), -1, &rLbl, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

                }

            }

            RestoreDC(hdc, savedDC);

        }

    }

    else {

        g_rcOutlineClose = { 0, 0, 0, 0 };

    }



    // 3. ЗАГОЛОВОК КОНСОЛИ (SPLITTER BAR)

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

    const wchar_t* wrapState = g_wordWrap ? L"  |  Wrap: ON" : L"";



    wchar_t statBuf[512];

    swprintf(statBuf, 512, L"Стр %d, Стлб %d   |   Символов: %d   |   Масштаб: %d%%   |   %s   |   %s   |   %s%s",

        line, col, totalLen, zoom, langName, GetThemeName(currentTheme), compState, wrapState);



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

        LogToConsoleW(L"> Kova Code Studio v1.3.0 Release (Разработчик: ANR1S) готов к работе!", LOG_SUCCESS);

        LogToConsoleW(L"> Новое: FTP-менеджер (Ctrl+Alt+F), Сборка + .smx на FTP (F6), подсветка слов, мультикурсор (Ctrl+Shift+L).", LOG_NORMAL);

        if (wcslen(szCompilerPath) > 0) {

            LogToConsoleW((std::wstring(L"> Подключен компилятор SourcePawn: ") + szCompilerPath).c_str(), LOG_NORMAL);

        }

        RestoreSavedSession();

        return 0;

    }



    case WM_TIMER: {

        if (wParam == IDT_OUTLINE_UPDATE) {

            KillTimer(hwnd, IDT_OUTLINE_UPDATE);

            if (currentTab >= 0 && currentTab < (int)tabs.size()) {

                UpdateOutline(tabs[currentTab]);

                if (g_showOutline) {

                    RECT rc;

                    GetClientRect(hwnd, &rc);

                    RECT rcOut = { rc.right - g_outlineWidth, g_toolbarHeight + g_tabHeight, rc.right, rc.bottom - g_statusHeight - g_consoleHeight - g_consoleHeaderHeight };

                    InvalidateRect(hwnd, &rcOut, FALSE);

                }

            }

            return 0;

        }

        break;

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



        int hovOut = -1;

        bool hovOutClose = false;

        if (g_showOutline && rc.right >= 520 && pt.x >= rc.right - g_outlineWidth && pt.y >= g_toolbarHeight + g_tabHeight && pt.y < hdrTop) {

            if (PtInRect(&g_rcOutlineClose, pt)) hovOutClose = true;

            if (currentTab >= 0) {

                for (int i = 0; i < (int)tabs[currentTab].outline.size(); i++) {

                    if (PtInRect(&tabs[currentTab].outline[i].rc, pt)) {

                        hovOut = i;

                        break;

                    }

                }

            }

        }

        if (hovOut != g_hoverOutlineIdx || hovOutClose != g_hoverOutlineClose) {

            g_hoverOutlineIdx = hovOut;

            g_hoverOutlineClose = hovOutClose;

            RECT rcOut = { rc.right - g_outlineWidth, g_toolbarHeight + g_tabHeight, rc.right, hdrTop };

            InvalidateRect(hwnd, &rcOut, FALSE);

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



        if (g_showOutline && rc.right >= 520 && pt.x >= rc.right - g_outlineWidth && pt.y >= g_toolbarHeight + g_tabHeight && pt.y < hdrTop) {

            if (PtInRect(&g_rcOutlineClose, pt)) {

                g_showOutline = false;

                SaveConfig();

                ResizeLayout();

                RebuildMenu();

                return 0;

            }

            if (currentTab >= 0) {

                for (const auto& item : tabs[currentTab].outline) {

                    if (PtInRect(&item.rc, pt)) {

                        HWND ed = tabs[currentTab].hwndEditor;

                        SendEd(ed, SCI_GOTOLINE, item.line);

                        SendEd(ed, SCI_SCROLLCARET);

                        SetFocus(ed);

                        InvalidateRect(hwnd, NULL, FALSE);

                        return 0;

                    }

                }

            }

            return 0;

        }



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

            HighlightWordOccurrences(ed);

            RECT rc;

            GetClientRect(hwnd, &rc);

            RECT rcStat = { 0, rc.bottom - g_statusHeight, rc.right, rc.bottom };

            InvalidateRect(hwnd, &rcStat, FALSE);

            if (g_showOutline && rc.right >= 520) {

                int hdrTop = rc.bottom - g_statusHeight - g_consoleHeight - g_consoleHeaderHeight;

                RECT rcOut = { rc.right - g_outlineWidth, g_toolbarHeight + g_tabHeight, rc.right, hdrTop };

                InvalidateRect(hwnd, &rcOut, FALSE);

            }

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

                SetTimer(hwnd, IDT_OUTLINE_UPDATE, 220, NULL);

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

        case IDM_FILE_SAVEALL:      SaveAllFiles(); break;

        case IDM_FILE_SAVEAS:       SaveCurrentFile(true); break;

        case IDM_FILE_OPEN_FOLDER:  OpenContainingFolder(currentTab); break;

        case IDM_FILE_EXIT:         SendMessageW(hwnd, WM_CLOSE, 0, 0); break;

        case IDM_FTP_MANAGER:       ShowFtpManagerDialog(); break;

        case IDM_FTP_UPLOAD_ACTIVE: FtpUploadActiveTab(); break;

        case IDM_RUN_SMART:         RunSmartDispatcher(); break;

        case IDM_RUN_AND_UPLOAD:    RunSmartAndUploadSmx(); break;

        case IDM_SET_COMPILER:      SelectCompilerDlg(); break;

        case IDM_CLEAR_CONSOLE:     ClearConsole(); break;

        case IDM_EDIT_FIND_REPLACE: ShowCustomFindDialog(); break;

        case IDM_EDIT_GOTO_LINE:    ShowGotoLineDialog(); break;

        case IDM_EDIT_SELECT_ALL_OCC: SelectAllWordOccurrences(); break;

        case IDM_EDIT_DUPLICATE:    DuplicateCurrentLine(); break;

        case IDM_EDIT_COMMENT:      ToggleLineComment(); break;

        case IDM_EDIT_LINE_UP:      MoveSelectedLinesUp(); break;

        case IDM_EDIT_LINE_DOWN:    MoveSelectedLinesDown(); break;

        case IDM_EDIT_AUTOCOMPLETE: if (currentTab >= 0) TriggerAutoComplete(tabs[currentTab], true); break;

        case IDM_EDIT_FORMAT_CODE:  FormatActiveCode(); break;

        case IDM_EDIT_FOLD_ALL:     FoldAllCode(true); break;

        case IDM_EDIT_UNFOLD_ALL:   FoldAllCode(false); break;

        case IDM_EDIT_SNIPPET_SP:   InsertSourcePawnTemplate(); break;

        case IDM_EDIT_CHAT_COLORS:  ShowChatColorsDialog(); break;

        case IDM_VIEW_OUTLINE:

            g_showOutline = !g_showOutline;

            SaveConfig();

            ResizeLayout();

            RebuildMenu();

            break;

        case IDM_VIEW_WORDWRAP:

            g_wordWrap = !g_wordWrap;

            for (auto& tb : tabs) {

                SendEd(tb.hwndEditor, SCI_SETWRAPMODE, g_wordWrap ? SC_WRAP_WORD : SC_WRAP_NONE, 0);

            }

            SaveConfig();

            RebuildMenu();

            InvalidateRect(hwnd, NULL, FALSE);

            LogToConsoleW(g_wordWrap ? L"> Перенос строк: ВКЛ (Alt+Z)." : L"> Перенос строк: ВЫКЛ.", LOG_ACCENT);

            break;

        case IDM_VIEW_ZOOM_IN:

            SendActive(SCI_ZOOMIN);

            InvalidateRect(hwnd, NULL, FALSE);

            break;

        case IDM_VIEW_ZOOM_OUT:

            SendActive(SCI_ZOOMOUT);

            InvalidateRect(hwnd, NULL, FALSE);

            break;

        case IDM_VIEW_ZOOM_RESET:

            SendActive(SCI_SETZOOM, 0);

            InvalidateRect(hwnd, NULL, FALSE);

            break;

        case IDM_TAB_CLOSE:         CloseTab(g_contextMenuTabIdx); break;

        case IDM_TAB_CLOSE_CURRENT: CloseTab(currentTab); break;

        case IDM_TAB_CLOSE_OTHERS:  CloseOtherTabs(g_contextMenuTabIdx); break;

        case IDM_TAB_OPEN_DIR:      OpenContainingFolder(g_contextMenuTabIdx); break;

        case IDM_TAB_COPY_PATH:     CopyFilePathToClipboard(g_contextMenuTabIdx); break;

        case IDM_TAB_NEXT:          if (!tabs.empty()) SwitchTab((currentTab + 1) % (int)tabs.size()); break;

        case IDM_TAB_PREV:          if (!tabs.empty()) SwitchTab((currentTab + (int)tabs.size() - 1) % (int)tabs.size()); break;

        case IDM_THEME_DARK:        currentTheme = 0; SaveConfig(); ApplyThemeToAll(); RebuildMenu(); break;

        case IDM_THEME_LIGHT:       currentTheme = 1; SaveConfig(); ApplyThemeToAll(); RebuildMenu(); break;

        case IDM_THEME_CYBER:       currentTheme = 2; SaveConfig(); ApplyThemeToAll(); RebuildMenu(); break;

        case IDM_THEME_MONOKAI:     currentTheme = 3; SaveConfig(); ApplyThemeToAll(); RebuildMenu(); break;

        case IDM_THEME_ONEDARK:     currentTheme = 4; SaveConfig(); ApplyThemeToAll(); RebuildMenu(); break;

        case IDM_THEME_TOGGLE:

            currentTheme = (currentTheme + 1) % 5;

            SaveConfig();

            ApplyThemeToAll();

            RebuildMenu();

            LogToConsoleW((std::wstring(L"> Активирована тема: ") + GetThemeName(currentTheme)).c_str(), LOG_ACCENT);

            break;

        case IDM_CHANGELOG:         ShowChangelogWindow(); break;

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



    case WM_CLOSE: {

        for (int i = 0; i < (int)tabs.size(); i++) {

            if (!ConfirmCloseTab(i)) return 0;

        }

        DestroyWindow(hwnd);

        return 0;

    }



    case WM_DESTROY:

        KillTimer(hwnd, IDT_OUTLINE_UPDATE);

        FtpDisconnect();

        SaveConfig();

        if (hUIFont) { DeleteObject(hUIFont); hUIFont = NULL; }

        if (hUIBoldFont) { DeleteObject(hUIBoldFont); hUIBoldFont = NULL; }

        if (hTitleFont) { DeleteObject(hTitleFont); hTitleFont = NULL; }

        if (hSmallFont) { DeleteObject(hSmallFont); hSmallFont = NULL; }

        if (hDarkEditBrush) { DeleteObject(hDarkEditBrush); hDarkEditBrush = NULL; }

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

            L"Kova Code Studio v1.2.0", MB_OK | MB_ICONERROR);

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



    hwndMain = CreateWindowExW(0, L"KovaCodeClass", L"Kova Code Studio v1.3.0",

        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 1240, 800, NULL, NULL, hInstance, NULL);



    SendMessageW(hwndMain, WM_SETICON, ICON_BIG, (LPARAM)hGlobalAppIconBig);

    SendMessageW(hwndMain, WM_SETICON, ICON_SMALL, (LPARAM)hGlobalAppIconSmall);



    ShowWindow(hwndMain, nCmdShow);

    UpdateWindow(hwndMain);



    ACCEL accels[] = {

        { FCONTROL | FVIRTKEY, 'N', IDM_FILE_NEW },

        { FCONTROL | FVIRTKEY, 'O', IDM_FILE_OPEN },

        { FCONTROL | FVIRTKEY, 'S', IDM_FILE_SAVE },

        { FCONTROL | FSHIFT | FVIRTKEY, 'S', IDM_FILE_SAVEALL },

        { FCONTROL | FVIRTKEY, 'W', IDM_TAB_CLOSE_CURRENT },

        { FCONTROL | FVIRTKEY, VK_TAB, IDM_TAB_NEXT },

        { FCONTROL | FSHIFT | FVIRTKEY, VK_TAB, IDM_TAB_PREV },

        { FCONTROL | FVIRTKEY, 'F', IDM_EDIT_FIND_REPLACE },

        { FCONTROL | FVIRTKEY, 'H', IDM_EDIT_FIND_REPLACE },

        { FCONTROL | FVIRTKEY, 'G', IDM_EDIT_GOTO_LINE },

        { FCONTROL | FVIRTKEY, 'D', IDM_EDIT_DUPLICATE },

        { FCONTROL | FVIRTKEY, 'J', IDM_EDIT_SNIPPET_SP },

        { FCONTROL | FVIRTKEY, 'M', IDM_EDIT_CHAT_COLORS },

        { FCONTROL | FVIRTKEY, 'B', IDM_VIEW_OUTLINE },

        { FCONTROL | FVIRTKEY, VK_SPACE, IDM_EDIT_AUTOCOMPLETE },

        { FSHIFT | FALT | FVIRTKEY, 'F', IDM_EDIT_FORMAT_CODE },

        { FALT | FVIRTKEY, 'Z', IDM_VIEW_WORDWRAP },

        { FCONTROL | FVIRTKEY, VK_OEM_PLUS, IDM_VIEW_ZOOM_IN },

        { FCONTROL | FVIRTKEY, VK_ADD, IDM_VIEW_ZOOM_IN },

        { FCONTROL | FVIRTKEY, VK_OEM_MINUS, IDM_VIEW_ZOOM_OUT },

        { FCONTROL | FVIRTKEY, VK_SUBTRACT, IDM_VIEW_ZOOM_OUT },

        { FCONTROL | FVIRTKEY, '0', IDM_VIEW_ZOOM_RESET },

        { FCONTROL | FVIRTKEY, VK_NUMPAD0, IDM_VIEW_ZOOM_RESET },

        { FCONTROL | FVIRTKEY, VK_OEM_2, IDM_EDIT_COMMENT },

        { FALT | FVIRTKEY, VK_UP, IDM_EDIT_LINE_UP },

        { FALT | FVIRTKEY, VK_DOWN, IDM_EDIT_LINE_DOWN },

        { FCONTROL | FALT | FVIRTKEY, 'F', IDM_FTP_MANAGER },

        { FCONTROL | FSHIFT | FVIRTKEY, 'L', IDM_EDIT_SELECT_ALL_OCC },

        { FCONTROL | FSHIFT | FVIRTKEY, VK_OEM_4, IDM_EDIT_FOLD_ALL },

        { FCONTROL | FSHIFT | FVIRTKEY, VK_OEM_6, IDM_EDIT_UNFOLD_ALL },

        { FVIRTKEY, VK_F5, IDM_RUN_SMART },

        { FVIRTKEY, VK_F6, IDM_RUN_AND_UPLOAD }

    };

    HACCEL hAccel = CreateAcceleratorTableW(accels, sizeof(accels) / sizeof(ACCEL));



    MSG msg = { 0 };

    while (GetMessageW(&msg, NULL, 0, 0)) {

        if (msg.message == WM_KEYDOWN) {

            if (hwndFindDlg && IsWindow(hwndFindDlg) && (msg.hwnd == hwndFindDlg || IsChild(hwndFindDlg, msg.hwnd))) {

                if (msg.wParam == VK_ESCAPE) {

                    DestroyWindow(hwndFindDlg);

                    if (currentTab >= 0) SetFocus(tabs[currentTab].hwndEditor);

                    continue;

                }

                if (msg.wParam == VK_RETURN) {

                    if (GetKeyState(VK_SHIFT) & 0x8000) DoFindPrev();

                    else DoFindNext();

                    continue;

                }

                if (msg.wParam == VK_TAB) {

                    SetFocus(GetFocus() == hEditFindBox ? hEditReplaceBox : hEditFindBox);

                    continue;

                }

            }

            if (hwndGotoDlg && IsWindow(hwndGotoDlg) && (msg.hwnd == hwndGotoDlg || IsChild(hwndGotoDlg, msg.hwnd))) {

                if (msg.wParam == VK_ESCAPE) {

                    DestroyWindow(hwndGotoDlg);

                    if (currentTab >= 0) SetFocus(tabs[currentTab].hwndEditor);

                    continue;

                }

                if (msg.wParam == VK_RETURN) {

                    ExecuteGotoLine();

                    continue;

                }

            }

        }



        if (msg.message == WM_MOUSEWHEEL && g_showOutline && currentTab >= 0) {

            POINT pt;

            GetCursorPos(&pt);

            ScreenToClient(hwndMain, &pt);

            RECT rc;

            GetClientRect(hwndMain, &rc);

            int hdrTop = rc.bottom - g_statusHeight - g_consoleHeight - g_consoleHeaderHeight;

            if (rc.right >= 520 && pt.x >= rc.right - g_outlineWidth && pt.y >= g_toolbarHeight + g_tabHeight && pt.y < hdrTop) {

                int delta = GET_WHEEL_DELTA_WPARAM(msg.wParam);

                g_outlineScroll -= (delta > 0 ? 2 : -2);

                if (g_outlineScroll < 0) g_outlineScroll = 0;

                RECT rcOut = { rc.right - g_outlineWidth, g_toolbarHeight + g_tabHeight, rc.right, hdrTop };

                InvalidateRect(hwndMain, &rcOut, FALSE);

                continue;

            }

        }



        if (!TranslateAcceleratorW(hwndMain, hAccel, &msg)) {

            TranslateMessage(&msg);

            DispatchMessageW(&msg);

        }

    }



    DestroyAcceleratorTable(hAccel);

    FreeLibrary(hModScintilla);

    return 0;

}