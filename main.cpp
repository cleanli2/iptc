#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include <cstdio>

//#pragma comment(lib, "ws2_32.lib")

HINSTANCE hg_app;
HWND editHd;
HWND sHd;
#define MY_ID_EDIT 0x3501
#define MY_ID_BT 0x3502
char strbuf[128];

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
TCHAR szClassName[ ] = _T("CodeBlocksWindowsApp");

int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    setbuf(stdout, NULL);
    printf("started\r\n");

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;
    hg_app=hThisInstance;

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           _T("ÐÞ"),       /* Title Text */
           WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX& ~WS_THICKFRAME,
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           544,                 /* The programs width */
           375,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
    ShowWindow (hwnd, nCmdShow);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    printf("end\r\n");

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}


/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //printf("-%x %x\r\n", wParam, lParam);
    switch (message)                  /* handle the messages */
    {
        case WM_CREATE:
            printf("created\r\n");
            {
                editHd = CreateWindow(TEXT("edit"),TEXT("99"),WS_CHILD|WS_VISIBLE|WS_BORDER|ES_LEFT|ES_MULTILINE,
                        80, 80, 260, 200, hwnd,(HMENU)MY_ID_EDIT, hg_app,NULL);
                CreateWindow("Button", "b1", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        10, 10, 60, 20, hwnd, (HMENU)MY_ID_BT, hg_app, NULL);
                sHd = CreateWindow("Static","server ip", SS_SIMPLE | WS_CHILD | WS_VISIBLE,
                        10, 40, 150,20, hwnd, NULL, hg_app, NULL);
                break;
            }
        case WM_CTLCOLOREDIT:
            printf("WM_CTLCOLOREDIT\r\n");
            if(editHd==(HWND)lParam){
                printf("my edit\r\n");
                ::SetTextColor((HDC)wParam, RGB(0,0,255));
            }
            break;
        case WM_COMMAND:
            printf("wmcmd\r\n");
            switch(LOWORD(wParam))
            {
                case MY_ID_EDIT:
                    if(HIWORD(wParam)==EN_CHANGE){
                        printf("EN_CHANGE\r\n");
                        GetWindowText(editHd, strbuf, 128);
                        printf("%s\r\n", strbuf);
                        if(strlen(strbuf)>20){
                            printf("exceed 20\r\n");
                            SetWindowText(editHd, "99");
                        }
                        if(!strcmp(strbuf, "hello")){
                            printf("hello\r\n");
                        }
                    }

                    break;
                case MY_ID_BT:
                    printf("button clicked\r\n");
                    break;
                default:
                    printf("unhandled\r\n");
            }

            break;
        case WM_DESTROY:
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}
