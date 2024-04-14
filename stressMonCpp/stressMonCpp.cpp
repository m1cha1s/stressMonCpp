#include "framework.h"
#include "stressMonCpp.h"

#define MAX_LOADSTRING 100
#define SIDE_BAR_WIDTH 200

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int, HWND*);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

global_variable bool Running;

global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel = 4;

global_variable HWND Button;
global_variable HWND COMPortField;

global_variable HANDLE COMPort;
global_variable DCB dcb;

global_variable int Speed = 1;

internal void
RenderWeirdGradient(int XOffset, int YOffset)
{
    int Pitch = BitmapWidth*BytesPerPixel;
    u8 *Row = (u8*)BitmapMemory;
    for (int Y = 0; Y < BitmapHeight; Y++)
    {
        u8* Pixel = (u8*)Row;
        for (int X = 0; X < BitmapWidth; X++)
        {
            *Pixel = (u8)(X + XOffset);
            ++Pixel;
            
            *Pixel = (u8)(Y + YOffset);
            ++Pixel;
            
            *Pixel = 255;
            ++Pixel;
            
            *Pixel = 0; // Padding
            ++Pixel;
        }
        Row += Pitch;
    }
}

internal void
Win32ResizeDIBSection(int Width, int Height)
{
    if (BitmapMemory)
    {
        VirtualFree(BitmapMemory, 0, MEM_RELEASE);
    }
    
    BitmapWidth = Width-SIDE_BAR_WIDTH;
    BitmapHeight = Height;
    
    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight; // This is negative for a top down DIB buffer
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;
    
    int BitmapMemorySize = BytesPerPixel*BitmapWidth*BitmapHeight;
    // Here we are allocating whole pages, 4k or 64k, depending on the system shenanigans
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void
Win32UpdateWindow(HDC DeviceContext, RECT* WindowRect, int X, int Y, int Width, int Height)
{
    int WindowWidth = WindowRect->right - WindowRect->left;
    int WindowHeight = WindowRect->bottom - WindowRect->top;
    StretchDIBits(DeviceContext,
                  /*
                  X, Y, Width, Height,
                  X, Y, Width, Height,
*/
                  0, 0, BitmapWidth, BitmapHeight,
                  0, 0, BitmapWidth, BitmapHeight,
                  BitmapMemory,
                  &BitmapInfo,
                  DIB_RGB_COLORS, SRCCOPY);
}

// Application entry
int APIENTRY 
wWinMain(_In_ HINSTANCE hInstance,
         _In_opt_ HINSTANCE hPrevInstance,
         _In_ LPWSTR    lpCmdLine,
         _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_STRESSMONCPP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    
    HWND Window;
    if (!InitInstance (hInstance, nCmdShow, &Window))
    {
        return FALSE;
    }
    
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_STRESSMONCPP));
    
    MSG msg;
    
    int XOffset = 0;
    int YOffset = 0;
    
    Running = true;
    while (Running)
    {
        
        while(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                Running = false;
            }
            
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        
        RenderWeirdGradient(XOffset, YOffset);
        
        HDC DeviceContext = GetDC(Window);
        RECT WindowRect;
        GetClientRect(Window, &WindowRect);
        int WindowWidth = WindowRect.right - WindowRect.left;
        int WindowHeight = WindowRect.bottom - WindowRect.top;
        Win32UpdateWindow(DeviceContext, &WindowRect, 0, 0, WindowWidth, WindowHeight);
        ReleaseDC(Window, DeviceContext);
        
        XOffset += Speed;
        YOffset += Speed;
        YOffset += Speed;
    }
    
    return (int) msg.wParam;
}

// Register window class
ATOM 
MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    
    wcex.cbSize = sizeof(WNDCLASSEX);
    
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_STRESSMONCPP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_STRESSMONCPP);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    
    return RegisterClassExW(&wcex);
}

// Create main window
BOOL 
InitInstance(HINSTANCE hInstance, int nCmdShow, HWND *hWnd)
{
    hInst = hInstance;
    
    *hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
    
    if (!(*hWnd))
    {
        return FALSE;
    }
    
    ShowWindow(*hWnd, nCmdShow);
    UpdateWindow(*hWnd);
    return TRUE;
}


// Process main window events
LRESULT CALLBACK 
WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
        {
            RECT ClientRect;
            GetClientRect(hWnd, &ClientRect);
            int Width = ClientRect.right - ClientRect.left;
            int Height = ClientRect.bottom - ClientRect.top;
            
            Button = CreateWindow( 
                                  L"BUTTON",  // Predefined class; Unicode assumed 
                                  L"Connect",      // Button text 
                                  WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
                                  Width - 150,         // x position
                                  10,         // y position 
                                  100,        // Button width
                                  23,        // Button height
                                  hWnd,     // Parent window
                                  (HMENU)ConnectButton,       // No menu.
                                  NULL, 
                                  NULL);      // Pointer not needed.
            
            COMPortField = CreateWindow(
                                        L"EDIT",
                                        L"COM",
                                        WS_BORDER | WS_TABSTOP | WS_VISIBLE | WS_CHILD,
                                        Width - 150,
                                        10 + 23 + 10,
                                        100,
                                        23,
                                        hWnd,
                                        NULL,
                                        NULL,
                                        NULL);
        } break;
        case WM_SIZE:
        {
            RECT ClientRect;
            GetClientRect(hWnd, &ClientRect);
            int Width = ClientRect.right - ClientRect.left;
            int Height = ClientRect.bottom - ClientRect.top;
            Win32ResizeDIBSection(Width, Height);
            
            SetWindowPos(Button, NULL, Width - 150, 10, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
            SetWindowPos(COMPortField, NULL, Width - 150, 10 + 23 + 10, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
        } break;
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            
            switch (wmId)
            {
                case ConnectButton:
                {
                    Speed++; // To be removed
                    
                    WCHAR portName[maxPortStrSize];
                    
                    GetWindowTextW(COMPortField, portName, maxPortStrSize);
                    
                    OutputDebugStringW(portName);
                    OutputDebugStringW(L"\n");
                    
                    if (COMPort != INVALID_HANDLE_VALUE)
                    {
                        CloseHandle(COMPort);
                    }
                    
                    COMPort = CreateFile(portName,
                                         GENERIC_READ | GENERIC_WRITE,
                                         0,
                                         NULL,
                                         OPEN_EXISTING,
                                         0,
                                         NULL);
                    
                    if (COMPort == INVALID_HANDLE_VALUE)
                    {
                        goto FAIL;
                    }
                    
                    // Initialize the DCB struct
                    SecureZeroMemory(&dcb, sizeof(DCB));
                    dcb.DCBlength = sizeof(DCB);
                    
                    if (!GetCommState(COMPort, &dcb))
                    {
                        goto FAIL_OPEN_HANDLE;
                    }
                    
                    dcb.BaudRate = CBR_9600;
                    dcb.ByteSize = 8;
                    dcb.Parity = NOPARITY;
                    dcb.StopBits = ONESTOPBIT;
                    
                    if (!SetCommState(COMPort, &dcb))
                    {
                        goto FAIL_OPEN_HANDLE;
                    }
                    
                    /*
                    if (!GetCommState(COMPort, &dcb))
                    {
                        goto FAIL_OPEN_HANDLE;
                    }*/
                    
                    break;
                    FAIL_OPEN_HANDLE:
                    CloseHandle(COMPort);
                    FAIL:
                    MessageBoxA(NULL, "Failed to open COM port.", "Error", MB_OK | MB_ICONERROR);
                } break;
                case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
                case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
                default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            int x = ps.rcPaint.left;
            int y = ps.rcPaint.top;
            
            int width = ps.rcPaint.right - ps.rcPaint.left;
            int height = ps.rcPaint.bottom - ps.rcPaint.top;
            
            RECT ClientRect;
            GetClientRect(hWnd, &ClientRect);
            
            Win32UpdateWindow(hdc, &ClientRect, x, y, width, height);
            
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

// Process events of the about info
INT_PTR CALLBACK 
About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        case WM_INITDIALOG:
        return (INT_PTR)TRUE;
        
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
