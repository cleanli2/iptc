#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include <cstdio>
#include <sys/stat.h>

//#pragma comment(lib, "ws2_32.lib")

HINSTANCE hg_app;
HWND editHd;
HWND sHd;
#define MY_ID_EDIT 0x3501
#define MY_ID_BT 0x3502
#define TEXT_W 500
#define TEXT_H 100
#define HINT_SIZE 10

int csa[HINT_SIZE];
int csap=0;
char tc[3];
int bc;
int g_correct=0;
#define COLOR_CORRECT RGB(0, 0, 255)
#define COLOR_NOT_CORRECT RGB(255, 0, 0)

#define BUFSIZE 192
#define EMPTYLEFT 6
#define OBJ_EMPTYLEFT 4

FILE* g_fp=NULL;
char strbuf[BUFSIZE+1];
char objbuf[BUFSIZE+1]={0};
char stext_buf[50];
int cur_size=0, g_filesize=0;

void csa_init()
{
    for (int i=0;i<HINT_SIZE;i++){
        csa[i]=-1;
    }
}
void print_csa()
{
    printf("csa:%d\r\n", csap);
    for(int i=0;i<HINT_SIZE;i++){
        printf("%d ", csa[i]);
    }
    printf("csa:end\r\n");
}

void put_csa(int cz)
{
    int cur;
    if(csap==0){
        cur=csa[HINT_SIZE-1];
    }
    else{
        cur=csa[csap-1];
    }
    if(cur==cz){
        return;
    }
    if(csap>(HINT_SIZE-1)){
        csap=0;
    }
    csa[csap++]=cz;
    printf("put_csa\r\n");
    print_csa();
}

int get_csa()
{
    printf("get_csa\r\n");
    print_csa();
    if(csap==HINT_SIZE)return csa[0];
    else return csa[csap];
}

void dumpstr(void*is)
{
    char*s=(char*)is;
    int n=0;
    while(*s||n<16){
        if((n%16)==0)printf("\r\n%04x: ", n);
        printf("%02x ", 0xff&(*s));
        s++;
        n++;
    }
    printf("\r\n");
}

void str_leftmove(char*s, int n)
{
    int l=strlen(s);
    if(l==0)return;
    if(l<=n){
        memset(s, 0, l);
        return;
    }
    for(int i=0;i<l-n;i++){
        s[i]=s[i+n];
    }
    memset(s+l-n, 0, n);
}

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
    struct stat fstat;

    setbuf(stdout, NULL);
    printf("started\r\n");
    
    //open save
    g_fp=fopen("save", "r");
    if(!g_fp){
        printf("open save failed\r\n");
    }
    else{
        fscanf(g_fp, "%d", &cur_size);
        printf("open save ok=%d\r\n", cur_size);
        fclose(g_fp);
    }

    g_fp=fopen("book.txt", "rb");

    if(!g_fp){
        printf("open book.txt failed\r\n");
        MessageBox(NULL, _T("打开book.txt失败，退出！"), _T("提示"),MB_OK);
        return -1;
    }
    else{
        printf("open ok\r\n");
    }

    if(stat("book.txt", &fstat)<0){
        printf("get book.txt filesize failed\r\n");
        MessageBox(NULL, _T("获取book.txt大小失败，退出！"), _T("提示"),MB_OK);
        return -1;
    }

    g_filesize=fstat.st_size;
    printf("filesize=%d\r\n", g_filesize);

    memset(strbuf, 0, BUFSIZE);
    memset(objbuf, 0, BUFSIZE);

    csa_init();

    if(cur_size>g_filesize)cur_size=g_filesize;

    if(cur_size!=0){
        fseek(g_fp, cur_size, SEEK_SET);
    }
    printf("cur %d ftell:%d\r\n", cur_size, ftell(g_fp));

    fread(objbuf, BUFSIZE-OBJ_EMPTYLEFT, 1, g_fp);

    int tmpj=0, tmpv;
    put_csa(cur_size);
    for (int i=0;i<HINT_SIZE-1;i++){
        tmpv=strbuf[tmpj]=objbuf[tmpj];
        cur_size++;
        tmpj++;
        printf("tmpv=%x\r\n", tmpv);
        if((unsigned char)tmpv>0x80 || ((unsigned char)tmpv&0xff)==0xd){
            strbuf[tmpj]=objbuf[tmpj];
            cur_size++;
            tmpj++;
        }
        put_csa(cur_size);
    }
    printf("cursize=%d after hint\r\n", cur_size);

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
           _T("修"),       /* Title Text */
           WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX& ~WS_THICKFRAME,
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           TEXT_W+40,                 /* The programs width */
           TEXT_H*2,                 /* and height in pixels */
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

    fclose(g_fp);
    printf("end\r\n");

    //save
    printf("cur=%d before exit\r\n", cur_size);
    cur_size = get_csa();
    g_fp=fopen("save", "w");
    if(!g_fp){
        printf("save failed\r\n");
        MessageBox(NULL, _T("保存失败，退出！"), _T("提示"),MB_OK);
    }
    else{
        printf("open save ok=%d\r\n", cur_size);
    }
    fprintf(g_fp, "%d", cur_size);
    fclose(g_fp);

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}

void makeup_obj()
{
    while(strlen(objbuf)<(BUFSIZE-OBJ_EMPTYLEFT)){
        bc=fgetc(g_fp);
        printf("bc=0x%x\r\n", bc);
        if(EOF==bc){
            printf("endof file\r\n");
            break;
        }
        tc[0]=bc&0xff;
        tc[1]=0;
        strcat(objbuf, tc);
    }
}

unsigned char autofill[]={"，。；↓、！？：…《》　（）“”—"};
int need_autofill(char*s)
{
    dumpstr(s);
    dumpstr(autofill);
    if(s[0]==0xd && s[1]==0xa){
        printf("autofill 0d0a\r\n");
        return 1;
    }
    for(int i=0;i<sizeof(autofill);i+=2){
        if(((unsigned char)s[0]==autofill[i])&&((unsigned char)s[1]==autofill[i+1])){
            printf("autofill\r\n");
            return 1;
        }
    }
    printf("no need autofill\r\n");
    return 0;

}

void do_compare()
{
    if(!strncmp(strbuf, objbuf, strlen(strbuf))){
        int sbl=strlen(strbuf);
        printf("strcmp correct\r\n");
        g_correct=1;
        if(strlen(objbuf)==(BUFSIZE-OBJ_EMPTYLEFT)){
            while(need_autofill(&objbuf[sbl])){
                tc[0]=objbuf[sbl];
                tc[1]=objbuf[sbl+1];
                tc[2]=0;
                while(strlen(strbuf)>(BUFSIZE-EMPTYLEFT)){
                    str_leftmove(strbuf, 2);
                    str_leftmove(objbuf, 2);
                }
                strcat(strbuf, tc);
                sbl=strlen(strbuf);
                makeup_obj();
            }
        }
        cur_size=ftell(g_fp)-(strlen(objbuf)-strlen(strbuf));
        put_csa(cur_size);
        sprintf(stext_buf, "当前%d字节，总长%d字节，完成%d%%",
                cur_size, g_filesize, cur_size*100/g_filesize);
    }
    else{
        g_correct=0;
    }
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
                do_compare();
                editHd = CreateWindow(TEXT("edit"),TEXT(strbuf),WS_CHILD|WS_VISIBLE|WS_BORDER|ES_LEFT|ES_MULTILINE,
                        20, 50, TEXT_W, TEXT_H, hwnd,(HMENU)MY_ID_EDIT, hg_app,NULL);
                CreateWindow("Button", "重来", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        10, 10, 100, 30, hwnd, (HMENU)MY_ID_BT, hg_app, NULL);
                sprintf(stext_buf, "当前%d字节，总长%d字节，完成%d%%",
                        cur_size, g_filesize, cur_size*100/g_filesize);
                sHd = CreateWindow("Static",stext_buf, SS_SIMPLE | WS_CHILD | WS_VISIBLE,
                        120, 10, 450,30, hwnd, NULL, hg_app, NULL);
                break;
            }
        case WM_CTLCOLOREDIT:
            //printf("WM_CTLCOLOREDIT\r\n");
            if(editHd==(HWND)lParam){
                //printf("my edit\r\n");
                if(g_correct){
                    ::SetTextColor((HDC)wParam, COLOR_CORRECT);
                }
                else{
                    ::SetTextColor((HDC)wParam, COLOR_NOT_CORRECT);
                }
            }
            break;
        case WM_COMMAND:
            //printf("wmcmd\r\n");
            switch(LOWORD(wParam))
            {
                case MY_ID_EDIT:
                    if(HIWORD(wParam)==EN_CHANGE){
                        printf("EN_CHANGE\r\n");
                        GetWindowText(editHd, strbuf, BUFSIZE);
                        printf("%s\r\n", strbuf);
                        printf("len %d, max %d\r\n", strlen(strbuf), BUFSIZE-EMPTYLEFT);
                        if(strlen(strbuf)>(BUFSIZE-EMPTYLEFT)){
                            printf("need left move\r\n");
                            while(strlen(strbuf)>(BUFSIZE-EMPTYLEFT)){
                                if((unsigned char)(0xff&strbuf[0])>(unsigned char)0x80){
                                    str_leftmove(strbuf, 2);
                                    str_leftmove(objbuf, 2);
                                }
                                else{
                                    str_leftmove(strbuf, 1);
                                    str_leftmove(objbuf, 1);
                                }
                            }
                            makeup_obj();
                            printf("strlen strbuf=%d, max %d\r\n", strlen(strbuf), (BUFSIZE-EMPTYLEFT));
                        }
                        printf("strshow:\r\n");
                        dumpstr(strbuf);
                        printf("objshow:\r\n");
                        dumpstr(objbuf);
                        do_compare();
                        SetWindowText(sHd, stext_buf);
                        SetWindowText(editHd, strbuf);
                        SendMessage(editHd, EM_SETSEL, strlen(strbuf), strlen(strbuf));
                        InvalidateRect(hwnd,NULL,TRUE);
                    }
                    else if(EN_MAXTEXT==HIWORD(wParam)){
                        printf("max!!!\r\n");
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
