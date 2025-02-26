#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include <cstdio>
#include <time.h>
#include <sys/stat.h>

//#pragma comment(lib, "ws2_32.lib")

HINSTANCE hg_app;
HWND editHd;
HWND sHd;
#define MY_ID_EDIT 0x3501
#define MY_ID_BT 0x3502
#define MY_ID_BTNH 0x3503
#define TEXT_W 720
#define TEXT_H 320
#define HINT_SIZE 10
#define HINT_MAX 9

char hint_bufp[HINT_MAX][128]={0};
char hint_rec[HINT_MAX*128+256];
int hint_idxp=0;

int end_of_file=0;
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
char stext_buf2[50];
int cur_size=0, g_filesize=0;
void save_hint();

void csa_init()
{
    csap=0;
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

void do_iptc_init()
{
    memset(strbuf, 0, BUFSIZE);
    memset(objbuf, 0, BUFSIZE);

    csa_init();

    if(cur_size>g_filesize)cur_size=g_filesize;

    if(cur_size!=0){
        fseek(g_fp, cur_size, SEEK_SET);
    }
    end_of_file=0;
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
}

int file_convert()
{
    int bc, wait=0;
    FILE*f1=fopen("book_r.txt", "rb");

    if(!f1){
        printf("open book_r.txt failed\r\n");
        MessageBox(NULL, _T("打开book_r.txt失败，退出！"), _T("提示"),MB_OK);
        return -1;
    }
    else{
        printf("open book_r.txt ok\r\n");
    }
    FILE*f2=fopen("book.txt", "wb");

    if(!f2){
        printf("open book.txt failed\r\n");
        return -1;
    }
    else{
        printf("open book.txt ok\r\n");
    }

    while(EOF!=(bc=fgetc(f1))){
        if(wait==0){
            fputc((char)(bc&0xff), f2);
            if(0x0d==(bc&0xff)){
                fputc((char)(0xa), f2);
                wait=1;
            }
        }
        else{
            if(0x0a!=(bc&0xff)){
                fputc((char)(bc&0xff), f2);
                wait=0;
            }
        }
    }
    fclose(f1);
    fclose(f2);
    return 0;
}

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

    if(0>file_convert()){
        return -1;
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

    do_iptc_init();

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
           TEXT_H+140,                 /* and height in pixels */
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

    save_hint();
    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}

void makeup_obj()
{
    printf("end=%d objlen %d vs %d\r\n", end_of_file, strlen(objbuf), BUFSIZE-OBJ_EMPTYLEFT);
    while(!end_of_file && strlen(objbuf)<(BUFSIZE-OBJ_EMPTYLEFT)){
        bc=fgetc(g_fp);
        printf("bc=0x%x\r\n", bc);
        if(EOF==bc){
            printf("endof file\r\n");
            end_of_file=1;
            break;
        }
        tc[0]=bc&0xff;
        tc[1]=0;
        strcat(objbuf, tc);
    }
    printf("end=%d objlen %d vs %d\r\n", end_of_file, strlen(objbuf), BUFSIZE-OBJ_EMPTYLEFT);
}

void if_end()
{
    if(ftell(g_fp)==g_filesize){
        end_of_file=1;
    }
    printf("end of file %d cursize=%d gfsz %d.\r\n",
            end_of_file, cur_size, g_filesize);
    if(end_of_file && cur_size==g_filesize){
        printf("end of file.\r\n");
        MessageBox(NULL, _T("恭喜！已完成。"), _T("提示"),MB_OK);
        EnableWindow(editHd, false);
    }
}

unsigned char autofill[]={"，。；↓、！？：…《》　（）“”—"};
int need_autofill(char*s)
{
    dumpstr(s);
    dumpstr(autofill);
    if(s[0]==0)return 0;
    if(s[0]==0xd && s[1]==0xa){
        printf("autofill 0d0a\r\n");
        return 1;
    }
    for(int i=0;i<sizeof(autofill);i+=2){
        if(((unsigned char)s[0]==autofill[i])&&((unsigned char)s[1]==autofill[i+1])){
            printf("autofill i=%d\r\n", i);
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
        while(need_autofill(&objbuf[sbl])){
            tc[0]=objbuf[sbl];
            tc[1]=objbuf[sbl+1];
            tc[2]=0;
            printf("len %d vs %d\r\n", strlen(strbuf), BUFSIZE-EMPTYLEFT);
            while(strlen(strbuf)>=(BUFSIZE-EMPTYLEFT)){
                str_leftmove(strbuf, 2);
                str_leftmove(objbuf, 2);
            }
            strcat(strbuf, tc);
            sbl=strlen(strbuf);
            makeup_obj();
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

void set_font()
{
    LOGFONT font;
    font.lfHeight = 36;
    font.lfWidth = 0;
    font.lfEscapement = 0;
    font.lfOrientation = 0;
    font.lfWeight = FW_BOLD;
    font.lfItalic = false;
    font.lfUnderline = false;
    font.lfStrikeOut = false;
    font.lfEscapement = 0;
    font.lfOrientation = 0;
    font.lfOutPrecision = OUT_DEFAULT_PRECIS;
    font.lfClipPrecision = CLIP_STROKE_PRECIS | CLIP_MASK | CLIP_TT_ALWAYS | CLIP_LH_ANGLES;
    font.lfQuality = ANTIALIASED_QUALITY;
    font.lfPitchAndFamily = VARIABLE_PITCH | FF_DONTCARE;

    HFONT hFont = ::CreateFontIndirect(&font);
    SendMessage(editHd, WM_SETFONT, (WPARAM)hFont, TRUE);
}

int get_pre_posi(unsigned char*s, int ps, int n)
{
    int j=ps;
    while(n--){
        j--;
        if(s[j-1]>0x80){
            j--;
        }
    }
    return j;
}

void get_hint()
{
    int i = 0, j, strp;
    char prec[11]={0}, postc[3];
    if(hint_idxp>=HINT_MAX){
        sprintf(hint_rec, "最多%d次提示，已经用完！", HINT_MAX);
        return;
    }
    while(strbuf[i]==objbuf[i])i++;
    j=get_pre_posi((unsigned char*)strbuf, i, 5);
    postc[0]=objbuf[i];
    postc[1]=objbuf[i+1];
    postc[2]=0;
    memcpy(prec, &strbuf[j], i-j);
    sprintf(hint_bufp[hint_idxp], "第%d次: 『%s』之后是：『%s』，文件第%d字节处。", hint_idxp+1, prec, postc, cur_size);
    memset(hint_rec, 0, sizeof(hint_rec));
    sprintf(hint_rec, "%s\r\n\r\n共有%d次提示机会，已用%d次。\r\n以下为历史提示记录：\r\n",
            hint_bufp[hint_idxp], HINT_MAX, hint_idxp+1);
    for(int k=0;k<hint_idxp;k++){
        strp=strlen(hint_rec);
        sprintf(&hint_rec[strp], "%s\r\n", hint_bufp[k]);
    }
    hint_idxp++;
}

void save_hint()
{
    time_t nowt;
    FILE* tmp=fopen("hint_rec.txt", "a");
    if(!tmp){
        printf("open hint rec failed\r\n");
    }
    else{
        time(&nowt);
        fprintf(tmp, "------------------------------\r\n");
        fprintf(tmp, "%s共使用%d次提示，以下为提示记录：\r\n", ctime(&nowt), hint_idxp);
        for(int k=0;k<hint_idxp;k++){
            fprintf(tmp, "%s\r\n", hint_bufp[k]);
        }
        fclose(tmp);
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
                        20, 80, TEXT_W, TEXT_H, hwnd,(HMENU)MY_ID_EDIT, hg_app,NULL);
                CreateWindow("Button", "重来", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        TEXT_W-100, 5, 100, 30, hwnd, (HMENU)MY_ID_BT, hg_app, NULL);
                CreateWindow("Button", "提示", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        10, 5, 100, 30, hwnd, (HMENU)MY_ID_BTNH, hg_app, NULL);
                sprintf(stext_buf, "当前%d字节，总长%d字节，完成%d%%",
                        cur_size, g_filesize, cur_size*100/g_filesize);
                sHd = CreateWindow("Static",stext_buf, SS_SIMPLE | WS_CHILD | WS_VISIBLE,
                        120, 40, 450,30, hwnd, NULL, hg_app, NULL);
                sprintf(stext_buf2, "Version:%s, built @ %s%s", GIT_SHA1, __DATE__, __TIME__);
                CreateWindow("Static",stext_buf2, SS_SIMPLE | WS_CHILD | WS_VISIBLE,
                        120, 10, 450,30, hwnd, NULL, hg_app, NULL);
                set_font();
                if_end();
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
                        //printf("%s\r\n", strbuf);
                        printf("len %d, max %d\r\n", strlen(strbuf), BUFSIZE-EMPTYLEFT);
                        if(strlen(strbuf)>(BUFSIZE-EMPTYLEFT)){
                            printf("need left move\r\n");
                            while(!end_of_file && strlen(strbuf)>(BUFSIZE-EMPTYLEFT)){
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
                        if_end();
                    }
                    else if(EN_MAXTEXT==HIWORD(wParam)){
                        printf("max!!!\r\n");
                    }

                    break;
                case MY_ID_BT:
                    printf("button clicked\r\n");
                    if(IDOK==MessageBox(hwnd, _T("确认要重新开始吗？"), _T("确认"),MB_OKCANCEL)){
                        cur_size=0;
                        fseek(g_fp, cur_size, SEEK_SET);
                        do_iptc_init();
                        do_compare();
                        EnableWindow(editHd, true);
                        SetWindowText(sHd, stext_buf);
                        SetWindowText(editHd, strbuf);
                    }
                    break;
                case MY_ID_BTNH:
                    {
                        int hsti = get_csa();
                        get_hint();
                        printf("button2 clicked\r\n");
                        MessageBox(NULL, _T(hint_rec), _T("提示"),MB_OK);
                        break;
                    }
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
