#include "framework.h"
#include "WindowsProject1.h"
#include <commdlg.h>
#include <objidl.h>
#include <gdiplus.h>
#include <windowsx.h> // Pour DialogBoxParam
#include <string>
#include <vector>
#include <stdexcept> // Pour gérer les exceptions
#pragma comment(lib, "Gdiplus.lib")

using namespace Gdiplus;

#define MAX_LOADSTRING 100

// Variables globales :
HINSTANCE hInst;                                // instance actuelle
WCHAR szTitle[MAX_LOADSTRING];                  // Texte de la barre de titre
WCHAR szWindowClass[MAX_LOADSTRING];            // nom de la classe de fenêtre principale
ULONG_PTR gdiplusToken;
Gdiplus::Image* loadedImage = nullptr;



// Prototypes
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
//INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void InitGDIPlus();
void ShutdownGDIPlus();
void OpenImageFile(HWND hWnd);
void DisplayImage(HWND hWnd, HDC hdc);
void ApplySteganography(HWND hWnd);
void ShowErrorAndExit(const WCHAR* message);

// Point d'entrée principal
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    // Initialiser GDI+
    InitGDIPlus(); // <-- Assure-toi que ceci est appelé ici

    // Autres initialisations...
    if (!LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING))
        ShowErrorAndExit(L"Échec de chargement du titre de la fenêtre.");

    // Continuer avec les autres initialisations
    if (!LoadStringW(hInstance, IDC_WINDOWSPROJECT1, szWindowClass, MAX_LOADSTRING))
        ShowErrorAndExit(L"Échec de chargement du nom de la classe de fenêtre.");

    // Enregistrement de la classe de fenêtre
    if (!MyRegisterClass(hInstance))
        ShowErrorAndExit(L"Échec d'enregistrement de la classe de fenêtre.");

    // Initialisation de l'instance de la fenêtre
    if (!InitInstance(hInstance, nCmdShow))
        ShowErrorAndExit(L"Échec d'initialisation de l'instance de la fenêtre.");

    // Reste du code
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT1));
    if (!hAccelTable)
        ShowErrorAndExit(L"Échec de chargement des accélérateurs.");

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // N'oublie pas de fermer GDI+ à la fin
    ShutdownGDIPlus(); // <-- Fermeture de GDI+ ici
    return static_cast<int>(msg.wParam);
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Stocke le handle d'instance dans la variable globale

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 800, 600, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    //SetWindowText(hWnd, L"Encodage Image");

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}

void InitGDIPlus()
{
    GdiplusStartupInput gdiplusStartupInput;
    if (GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr) != Ok)
    {
        ShowErrorAndExit(L"Échec de l'initialisation de GDI+.");
    }
}

void ShutdownGDIPlus()
{
    GdiplusShutdown(gdiplusToken);

    if (loadedImage)
    {
        delete loadedImage;
        loadedImage = nullptr;
    }
}

const WCHAR* GetGDIPlusErrorMessage(Status status)
{
    switch (status)
    {
    case Ok: return L"Opération réussie.";
    case FileNotFound: return L"Fichier non trouvé.";
    case OutOfMemory: return L"Pas assez de mémoire.";
    case InvalidParameter: return L"Paramètre invalide.";
    case AccessDenied: return L"Accès refusé.";
    case UnknownImageFormat: return L"Format d'image inconnu.";
    default: return L"Erreur inconnue.";
    }
}

void OpenImageFile(HWND hWnd)
{
    OPENFILENAME ofn = {};
    WCHAR szFile[260] = {};
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"Images PNG\0*.PNG\0";
    ofn.Flags = OFN_FILEMUSTEXIST;

    // Ouvre une boîte de dialogue pour choisir un fichier
    if (GetOpenFileName(&ofn))
    {
        // Journalisation du chemin du fichier pour débogage
        OutputDebugString(L"Chemin de fichier sélectionné : ");
        OutputDebugString(szFile);
        OutputDebugString(L"\n");

        // Vérifier si le fichier existe
        if (GetFileAttributes(szFile) == INVALID_FILE_ATTRIBUTES)
        {
            OutputDebugString(L"Le fichier n'existe pas ou est invalide.\n");
            MessageBox(hWnd, L"Erreur : Le fichier n'existe pas.", L"Erreur", MB_ICONERROR);
            return;
        }

        // Supprime l'image précédemment chargée (si existante)
        if (loadedImage)
        {
            delete loadedImage;
            loadedImage = nullptr;
        }

        // Charge une nouvelle image depuis le fichier
        loadedImage = Gdiplus::Image::FromFile(szFile);

        // Vérifie si le chargement de l'image a réussi
        Status status = loadedImage->GetLastStatus();
        if (status != Ok)
        {
            const WCHAR* errorMessage = GetGDIPlusErrorMessage(status);
            MessageBox(hWnd, errorMessage, L"Erreur de GDI+", MB_ICONERROR);

            // Nettoie l'objet `loadedImage` en cas d'échec
            delete loadedImage;
            loadedImage = nullptr;
            return;
        }

        // Affiche un message de succès
        MessageBox(hWnd, L"Image PNG chargée avec succès.", L"Succès", MB_ICONINFORMATION);

        // Rafraîchit la fenêtre pour afficher l'image
        InvalidateRect(hWnd, nullptr, TRUE);
    }
}

void DisplayImage(HWND hWnd, HDC hdc)
{
    if (loadedImage)
    {
        RECT rect;
        GetClientRect(hWnd, &rect); // Obtient la taille de la fenêtre

        Graphics graphics(hdc);

        // Dessine l'image centrée dans la fenêtre
        int imgWidth = loadedImage->GetWidth();
        int imgHeight = loadedImage->GetHeight();
        int winWidth = rect.right - rect.left;
        int winHeight = rect.bottom - rect.top;

        // Calcul des positions pour centrer l'image
        int x = (winWidth - imgWidth) / 2;
        int y = (winHeight - imgHeight) / 2;

        graphics.DrawImage(loadedImage, x, y, imgWidth, imgHeight); // Dessiner l'image à la position calculée
    }
}

Bitmap* ConvertToBitmap(Image* image)
{
    if (!image)
    {
        MessageBox(nullptr, L"Aucune image fournie pour la conversion.", L"Erreur", MB_ICONERROR);
        return nullptr;
    }

    // Vérifier les dimensions de l'image
    UINT width = image->GetWidth();
    UINT height = image->GetHeight();
    if (width == 0 || height == 0)
    {
        MessageBox(nullptr, L"Image invalide : dimensions nulles.", L"Erreur", MB_ICONERROR);
        return nullptr;
    }

    // Vérifier le type de l'image
    if (image->GetType() == ImageTypeBitmap)
    {
        MessageBox(nullptr, L"L'image est déjà un Bitmap.", L"Information", MB_ICONINFORMATION);
        return dynamic_cast<Bitmap*>(image);
    }

    // Créer un Bitmap compatible
    Bitmap* bitmap = new Bitmap(width, height, PixelFormat32bppARGB);
    if (!bitmap || bitmap->GetLastStatus() != Ok)
    {
        MessageBox(nullptr, L"Impossible de créer un Bitmap.", L"Erreur", MB_ICONERROR);
        delete bitmap;
        return nullptr;
    }

    // Rendu de l'image source dans le Bitmap
    Graphics graphics(bitmap);
    Status status = graphics.DrawImage(image, 0, 0, width, height);
    if (status != Ok)
    {
        MessageBox(nullptr, L"Échec du rendu de l'image dans le Bitmap.", L"Erreur", MB_ICONERROR);
        delete bitmap;
        return nullptr;
    }

    return bitmap;
}

// Fonction pour afficher une boîte de dialogue et récupérer le texte à cacher
std::wstring GetMessageFromUser(HWND hWnd)
{
    wchar_t buffer[256] = L""; // Limité à 255 caractères pour cet exemple
    if (DialogBoxParam(
        GetModuleHandle(nullptr),
        MAKEINTRESOURCE(IDD_INPUT_TEXT_DIALOG), // Remplacez par l'ID de votre boîte de dialogue
        hWnd,
        [](HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) -> INT_PTR {
            switch (message)
            {
            case WM_INITDIALOG:
                return TRUE; // Initialisation réussie
            case WM_COMMAND:
                if (LOWORD(wParam) == IDOK)
                {
                    // Récupère le texte de l'input
                    GetDlgItemText(hwndDlg, IDC_INPUT_TEXT, (LPWSTR)lParam, 256);
                    EndDialog(hwndDlg, IDOK);
                    return TRUE;
                }
                else if (LOWORD(wParam) == IDCANCEL)
                {
                    EndDialog(hwndDlg, IDCANCEL);
                    return TRUE;
                }
                break;
            }
            return FALSE;
        },
        (LPARAM)buffer) == IDOK)
    {
        return buffer;
    }
    return L"";
}

void ApplySteganography(HWND hWnd)
{
    if (!loadedImage)
    {
        ShowErrorAndExit(L"Aucune image chargée pour appliquer la stéganographie.");
        return;
    }

    // Demander le texte à l'utilisateur
    std::wstring userMessage = GetMessageFromUser(hWnd);
    if (userMessage.empty())
    {
        MessageBox(hWnd, L"Aucun message fourni. Opération annulée.", L"Information", MB_ICONINFORMATION);
        return;
    }

    // Convertir le message en une chaîne de caractères
    std::string message = std::string(userMessage.begin(), userMessage.end());
    size_t messageLength = message.length();

    // Convertir le message en une séquence de bits
    std::vector<bool> bitsToHide;
    for (size_t i = 0; i < messageLength; ++i)
    {
        unsigned char byte = message[i];
        for (int j = 7; j >= 0; --j) // Chaque caractère en binaire
        {
            bitsToHide.push_back((byte >> j) & 1);
        }
    }

    // Ajouter un terminator (fin de message) pour marquer la fin de la séquence de bits
    unsigned char terminator = 0; // 0x00 pour marquer la fin
    for (int j = 7; j >= 0; --j)
    {
        bitsToHide.push_back((terminator >> j) & 1);
    }

    // Conversion en Bitmap
    Bitmap* bitmap = ConvertToBitmap(loadedImage);
    if (!bitmap)
    {
        ShowErrorAndExit(L"Impossible de convertir l'image en Bitmap. Vérifiez le format de l'image ou sa compatibilité.");
        return;
    }

    size_t bitIndex = 0;
    UINT width = bitmap->GetWidth();
    UINT height = bitmap->GetHeight();

    // Remplacer les bits de chaque pixel
    for (UINT y = 0; y < height; ++y)
    {
        for (UINT x = 0; x < width; ++x)
        {
            Color pixel;
            bitmap->GetPixel(x, y, &pixel);

            // Modifier chaque composant de couleur (R, G, B)
            for (int colorComponent = 0; colorComponent < 3; ++colorComponent)
            {
                if (bitIndex < bitsToHide.size())
                {
                    unsigned char colorValue = 0;
                    switch (colorComponent)
                    {
                    case 0: colorValue = pixel.GetR(); break;
                    case 1: colorValue = pixel.GetG(); break;
                    case 2: colorValue = pixel.GetB(); break;
                    }

                    // Modifier le LSB (bit de poids faible) du composant de couleur
                    colorValue = (colorValue & 0xFE) | bitsToHide[bitIndex];
                    bitIndex++;

                    // Créer une nouvelle couleur avec le composant modifié
                    if (colorComponent == 0)
                        pixel = Color(pixel.GetA(), colorValue, pixel.GetG(), pixel.GetB());
                    else if (colorComponent == 1)
                        pixel = Color(pixel.GetA(), pixel.GetR(), colorValue, pixel.GetB());
                    else if (colorComponent == 2)
                        pixel = Color(pixel.GetA(), pixel.GetR(), pixel.GetG(), colorValue);
                }
            }

            // Modifier le pixel avec la nouvelle couleur
            bitmap->SetPixel(x, y, pixel);

            // Si tous les bits ont été cachés, on peut sortir
            if (bitIndex >= bitsToHide.size())
            {
                break;
            }
        }

        if (bitIndex >= bitsToHide.size())
        {
            break;
        }
    }

    InvalidateRect(hWnd, nullptr, TRUE); // Rafraîchir la fenêtre
    MessageBox(hWnd, L"Stéganographie appliquée avec succès!", L"Succès", MB_ICONINFORMATION);
}

// Fonction pour afficher un message d'erreur et quitter l'application.
void ShowErrorAndExit(const WCHAR* errorMsg)
{
    MessageBox(nullptr, errorMsg, L"Erreur Critique", MB_ICONERROR | MB_OK);
    ExitProcess(EXIT_FAILURE);
}

// Fonction qui gère les messages de la fenêtre
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        CreateWindow(L"BUTTON", L"Charger Image", WS_VISIBLE | WS_CHILD,
            10, 10, 150, 30, hWnd, (HMENU)1, hInst, nullptr);
        CreateWindow(L"BUTTON", L"Appliquer Stéganographie", WS_VISIBLE | WS_CHILD,
            170, 10, 200, 30, hWnd, (HMENU)2, hInst, nullptr);
    }
    break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case 1: // Bouton "Charger Image"
            OpenImageFile(hWnd);
            break;
        case 2: // Bouton "Appliquer Stéganographie"
            ApplySteganography(hWnd);
            break;
        }
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        DisplayImage(hWnd, hdc); // Appelle l'affichage de l'image
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

//INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
//{
//    UNREFERENCED_PARAMETER(lParam);
//    switch (message)
//    {
//    case WM_INITDIALOG:
//        return (INT_PTR)TRUE;
//
//    case WM_COMMAND:
//        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
//        {
//            EndDialog(hDlg, LOWORD(wParam));
//            return (INT_PTR)TRUE;
//        }
//        break;
//    }
//    return (INT_PTR)FALSE;
//}