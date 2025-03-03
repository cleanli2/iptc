#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include <WindowsX.h>
#include <cstdio>
#include <time.h>
#include <sys/stat.h>
#include "iptc_rc.h"

//#pragma comment(lib, "ws2_32.lib")

#define TWOC2 18
#define FONTSIZE 30
HINSTANCE hg_app;
HWND editHd;
HWND sHd;
HWND cmc_sHd;
#define MY_ID_EDIT 0x3501
#define MY_ID_BT 0x3502
#define MY_ID_BTNH 0x3503
#define TEXT_W 580
#define TEXT_H 288
#define HINT_SIZE 10
#define HINT_MAX 9
#define CIXS (TEXT_W+10)
#define CIYS (40)
#define CXSP 1
#define CYSP 2
#define WY (FONTSIZE+CYSP)
#define WX (FONTSIZE+CXSP+1)
#define CIXS1 (CIXS-WX*TWOC2-2)
#define CIYS1 (CIYS+WY*9)
#define CIXE2 (CIXS)
#define CIYE2 (CIYE1)
#define CIYS (40)
#define CIW 480
#define CSPLINE2 (TWOC2)
#define CIW2 (WX*CSPLINE2)
#define CMCC_SIZE 480
#define HIS_SIZE (TWOC2*4)
#define CSPLINE (CIW/WX)
#define CIXE1 (CIXS+WX*CSPLINE)
#define CIYE1 (CIYS+WY*15)
char common_cc[CMCC_SIZE+1]={"的是不人一这了你有个就在他我能功么来修炼也那都到们大法上中去要出它为可看讲说什以心时会多样种体还好高常想气所现家下没很身自西过事得东次层生真道些间给把正里着当佛子做己天因病后往性之开成发物用情候师学本呢和起化作只其问空许够实理别对而动题怎定质点意教叫觉然宇宙从经象吗神行目但外形小干传求同知根坏特门地年命越走吃方于如变练老最存面难长量认谁轮者打相才带力识全度德业提态思头果前治掉念哪话社元转悟回边各无比等已儿受再世类界眼直信状代又部通式执感另让手白明关管完少整苦程著两放太达国主利"};
char his_buf[HIS_SIZE+1]={"的的的的的的的的的的的的的的的的的的的的的的的的的的的的的的的的的的的的"};

#define TWOC1  ((CMCC_SIZE-120)/2)
#define SIG(x, xs, xe) ((x)>=(xs) && (xe)>=(x))
#define IN_RANGE(x, y, xs, ys, xe, ye) (SIG(x, xs, xe))&&(SIG(y, ys, ye))

char hint_bufp[HINT_MAX][128]={0};
char hint_rec[HINT_MAX*128+256];
int hint_idxp=0;
int laststrlen=0;
int g_sel_cn = -1;

int end_of_file=0;
int csa[HINT_SIZE];
int csap=0;
char tc[3];
int bc;
int g_correct=0;
int bytes_ct=0;
#define COLOR_CORRECT RGB(0, 0, 255)
#define COLOR_NOT_CORRECT RGB(255, 0, 0)

#define BUFSIZE 192
#define EMPTYLEFT 6
#define OBJ_EMPTYLEFT 4

HFONT hFont = NULL, oft=NULL;
FILE* g_fp=NULL;
FILE* log_fp=NULL;
char strbuf[BUFSIZE+1];
char objbuf[BUFSIZE+1]={0};
char stext_buf[50];
char stext_buf2[50];
int cur_size=0, g_filesize=0;
void save_hint();
void datelog();

#define TIMER_TXTX 978
#define TIMER_TXTY 5
#define ICT_TXTX 15
#define ICT_TXTY CIYS1+2*WY+5
#define SPD_TXTX 195
#define SPD_TXTY CIYS1+2*WY+5
int timer_count=0;
char timer_sbf[32]={0};
char ict_sbf[32]={0};
char spd_sbf[32]={0};
HANDLE hTimer = NULL;
HANDLE hTimerQueue = NULL;
void update_showbuf();

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

void chs_put_in_list(char*ch, char*list, int len)
{
    int i;
    for(i=0;i<TWOC1;i+=2){
        if((ch[0]==common_cc[i])&&(ch[1]==common_cc[i+1])){
            return;
        }
    }
    for(i=0;i<len;i+=2){
        if((ch[0]==list[i])&&(ch[1]==list[i+1])){
            break;
        }
    }
    if(i>=len)i=0;
    str_leftmove(list+i, 2);
    list[len-2]=ch[0];
    list[len-1]=ch[1];
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
    fprintf(log_fp, "cur %d ftell:%d\r\n", cur_size, ftell(g_fp));

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
    fprintf(log_fp, "cursize=%d after hint\r\n", cur_size);
    laststrlen=strlen(strbuf);
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
    time_t sttt, edt;
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */
    struct stat fstat;
    time(&sttt);

    HANDLE h_mutex = CreateMutex(NULL, FALSE, "iptc_m");
    DWORD err = GetLastError();
    if(err==ERROR_ALREADY_EXISTS){
        MessageBox(NULL, _T("已经打开了！请先关闭原来的窗口再打开。"), _T("提示"),MB_OK);
        return 0;
    }


    log_fp=fopen("iptc.log", "a");
    if(!log_fp){
        printf("open log failed\r\n");
        MessageBox(NULL, _T("打开log失败，退出！"), _T("提示"),MB_OK);
        return -1;
    }

    setbuf(stdout, NULL);
    printf("started\r\n");
    fprintf(log_fp, "\r\n\r\n************started*************\r\n");
    datelog();
    fprintf(log_fp, "\r\n");
    
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
        fprintf(log_fp, "file confvert fail\r\n");
        fclose(log_fp);
        return -1;
    }

    g_fp=fopen("book.txt", "rb");

    if(!g_fp){
        printf("open book.txt failed\r\n");
        MessageBox(NULL, _T("打开book.txt失败，退出！"), _T("提示"),MB_OK);
        fprintf(log_fp, "open boot.txt fail\r\n");
        fclose(log_fp);
        return -1;
    }
    else{
        printf("open ok\r\n");
    }

    if(stat("book.txt", &fstat)<0){
        printf("get book.txt filesize failed\r\n");
        MessageBox(NULL, _T("获取book.txt大小失败，退出！"), _T("提示"),MB_OK);
        fprintf(log_fp, "getfilesize boot.txt fail\r\n");
        fclose(log_fp);
        return -1;
    }

    g_filesize=fstat.st_size;
    printf("filesize=%d\r\n", g_filesize);
    fprintf(log_fp, "filesize=%d\r\n", g_filesize);

    do_iptc_init();

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (hThisInstance,(LPCTSTR)(IPTC_ICON));
    wincl.hIconSm = LoadIcon (hThisInstance,(LPCTSTR)(IPTC_ICON_SM));
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl)){
        fprintf(log_fp, "register windows class fail\r\n");
        fclose(log_fp);
        return 0;
    }
    hg_app=hThisInstance;

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           _T("修"),       /* Title Text */
           WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX& ~WS_THICKFRAME,
           0,
           0,
           TEXT_W+496,                 /* The programs width */
           TEXT_H+172,                 /* and height in pixels */
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
    fprintf(log_fp, "cur=%d before exit\r\n", cur_size);
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
    fprintf(log_fp, "save cur=%d\r\n", cur_size);
    fclose(g_fp);
    time(&edt);
    int t_secs=difftime(edt, sttt);
    sprintf(hint_rec, "总共输入%d字节（%d汉字），耗时%d分%d秒，速度%d汉字/分钟。\r\n",
            bytes_ct, bytes_ct/2, t_secs/60, t_secs%60, bytes_ct*60/2/t_secs);
    printf("%s", hint_rec);
    fprintf(log_fp, "%s", hint_rec);
    //MessageBox(NULL, _T(hint_rec), _T("提示"),MB_OK);

    save_hint();
    fprintf(log_fp, "quit\r\n\r\n");
    datelog();
    fprintf(log_fp, "***********ended*************\r\n\r\n\r\n");
    fclose(log_fp);
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
    font.lfHeight = FONTSIZE;
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
    chs_put_in_list(postc, his_buf, HIS_SIZE);
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

void datelog()
{
    time_t nowt;

    time(&nowt);
    printf("%s", ctime(&nowt));
    fprintf(log_fp, "%s", ctime(&nowt));
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

int get_selcn(int x, int y)
{
    int ret;
    printf("CSPLINE=%d\r\n", CSPLINE);
    if(IN_RANGE(x, y, CIXS, CIYS, CIXE1, CIYE1)){
        ret=(y-CIYS)/WY*CSPLINE+(x-CIXS)/WX;
    }
    else if(IN_RANGE(x, y, CIXS1, CIYS1, CIXE2, CIYE2)){
        ret=(y-CIYS1)/WY*CSPLINE2+(x-CIXS1)/WX+TWOC1;
    }
    else{
        ret = -1;
    }
    printf("get_selcn ret=%d\r\n", ret);
    return ret;
}

/*  This function is called by the Windows function DispatchMessage()  */

void handle_input()
{
    int newstrlen;
    newstrlen=strlen(strbuf);
    printf("EN_CHANGE %d %d\r\n", laststrlen, newstrlen);
    if(newstrlen>laststrlen){
        bytes_ct+=newstrlen-laststrlen;
        for(int n=laststrlen;n<=newstrlen;n++){
            if((unsigned char)strbuf[n]>0x80){
                if(n+1<=newstrlen){
                    chs_put_in_list(&strbuf[n], his_buf, HIS_SIZE);
                    n++;
                }
            }
        }
        update_showbuf();
    }
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
    laststrlen=strlen(strbuf);
    SetWindowText(sHd, stext_buf);
    SetWindowText(editHd, strbuf);
    SetFocus(editHd);
    SendMessage(editHd, EM_SETSEL, strlen(strbuf), strlen(strbuf));
    if_end();
}

void update_showbuf()
{
    sprintf(ict_sbf, "已输入 %05d 字节", bytes_ct);
    sprintf(spd_sbf, "打字速度 %03d 汉字/分", bytes_ct*60/2/timer_count);
}

VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
    timer_count++;
    sprintf(timer_sbf, "%02d:%02d:%02d", timer_count/3600,
            (timer_count%3600)/60, timer_count%60);
    update_showbuf();
    HWND hwnd;
    if (lpParam != NULL)
    {
        RECT rct={TIMER_TXTX, TIMER_TXTY, TIMER_TXTX+WX*8, TIMER_TXTY+WY};
        RECT rctict={ICT_TXTX, ICT_TXTY, ICT_TXTX+WX*20, ICT_TXTY+WY*2};
        hwnd=(HWND)lpParam;
        InvalidateRect(hwnd,&rct,TRUE);
        InvalidateRect(hwnd,&rctict,TRUE);
    }
}

void timer_init(HWND hwnd)
{
    //timer init
    // Create the timer queue.
    hTimerQueue = CreateTimerQueue();
    if (NULL == hTimerQueue)
    {
        //MessageBox(hwnd, "CreateTimerQueue failed", "Error", MB_ICONERROR);
        printf("CreateTimerQueue failed");
        return;
    }
    // Set a timer to call the timer routine in 10 seconds.
    if (!CreateTimerQueueTimer( &hTimer, hTimerQueue,
                (WAITORTIMERCALLBACK)TimerRoutine, hwnd , 500, 1000, 0))
    {
        //MessageBox(hwnd, "CreateTimerQueueTimer failed", "Error", MB_ICONERROR);
        printf("CreateTimerQueueTimer failed");
        return;
    }

}

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
                        10, 40, TEXT_W, TEXT_H, hwnd,(HMENU)MY_ID_EDIT, hg_app,NULL);
                CreateWindow("Button", "重来", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        TEXT_W-116, 5, 100, 30, hwnd, (HMENU)MY_ID_BT, hg_app, NULL);
                CreateWindow("Button", "提示", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        10, 5, 100, 30, hwnd, (HMENU)MY_ID_BTNH, hg_app, NULL);
                sprintf(stext_buf, "当前%d字节，总长%d字节，完成%d%%",
                        cur_size, g_filesize, cur_size*100/g_filesize);
                sHd = CreateWindow("Static",stext_buf, SS_SIMPLE | WS_CHILD | WS_VISIBLE,
                         120, 10, 450,30, hwnd, NULL, hg_app, NULL);
                //cmc_sHd = CreateWindow("Static", "III", SS_SIMPLE | WS_CHILD | WS_VISIBLE ,
                //         TEXT_W+10, 40, 300,100, hwnd, NULL, hg_app, NULL);
                sprintf(stext_buf2, "Version:%s, built @ %s%s", GIT_SHA1, __DATE__, __TIME__);
                CreateWindow("Static",stext_buf2, SS_SIMPLE | WS_CHILD | WS_VISIBLE,
                        TEXT_W-10,10, 450,30, hwnd, NULL, hg_app, NULL);
                set_font();
                timer_init(hwnd);
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
        case WM_LBUTTONDOWN:
            {
                char ocnc[3]={0};
                int lmx = GET_X_LPARAM(lParam);
                int lmy = GET_Y_LPARAM(lParam);
                printf("l m %d %d\r\n", lmx, lmy);
                int sel_cn=get_selcn(lmx, lmy);
                if(sel_cn>=0){
                    if(sel_cn==g_sel_cn){
                        printf("%d sel confirmed\r\n", g_sel_cn);
                        if(g_sel_cn<TWOC1){
                            ocnc[0]=common_cc[g_sel_cn*2];
                            ocnc[1]=common_cc[g_sel_cn*2+1];
                        }
                        else{
                            ocnc[0]=his_buf[(g_sel_cn-TWOC1)*2];
                            ocnc[1]=his_buf[(g_sel_cn-TWOC1)*2+1];
                        }
                        ocnc[2]=0;
                        strcat(strbuf, ocnc);
                        SetWindowText(editHd, strbuf);
                        handle_input();
                        g_sel_cn=-1;
                    }
                    else{
                        g_sel_cn=sel_cn;
                    }
                    InvalidateRect(hwnd,NULL,TRUE);
                }
            }
            break;

        case WM_COMMAND:
            //printf("wmcmd\r\n");
            switch(LOWORD(wParam))
            {
                case MY_ID_EDIT:
                    if(HIWORD(wParam)==EN_CHANGE){
                        GetWindowText(editHd, strbuf, BUFSIZE);
                        handle_input();
                        InvalidateRect(hwnd,NULL,TRUE);
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
                        InvalidateRect(hwnd,NULL,TRUE);
                        break;
                    }
                default:
                    printf("unhandled\r\n");
            }

            break;
        case WM_PAINT:
            {
                HDC hdc;
                RECT rt;
                PAINTSTRUCT ps;
                hdc = BeginPaint(hwnd, &ps);

                TextOut(hdc, TIMER_TXTX, TIMER_TXTY, timer_sbf,strlen(timer_sbf));
                TextOut(hdc, ICT_TXTX, ICT_TXTY, ict_sbf,strlen(ict_sbf));
                TextOut(hdc, SPD_TXTX, SPD_TXTY, spd_sbf,strlen(spd_sbf));
                if(hFont==NULL){
                    LOGFONT font;

                    font.lfHeight = FONTSIZE;
                    //font.lfHeight = MulDiv(20, GetDeviceCaps(hdc, LOGPIXELSY), 72);
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
                    strcpy_s(font.lfFaceName, "楷体");

                    hFont = ::CreateFontIndirect(&font);
                }

                oft = (HFONT)SelectObject(hdc, hFont);
                SetTextColor(hdc, RGB(0, 128, 255));
                SetTextCharacterExtra(hdc, CXSP);
                for(int i=0;i<TWOC1/(CIW/WX);i++){
                    TextOut(hdc, CIXS, CIYS+i*WY, common_cc+i*2*CIW/WX,2*CIW/WX);
                }
                SetTextColor(hdc, RGB(20, 128, 155));
                SetTextCharacterExtra(hdc, CXSP);
                TextOut(hdc, CIXS1, CIYS1, his_buf,TWOC2*2);
                TextOut(hdc, CIXS1, CIYS1+WY, his_buf+TWOC2*2,TWOC2*2);
                if(g_sel_cn>=0){
                    HPEN hPen = CreatePen(PS_SOLID,4,RGB(0,0,0));;
                    HPEN orgPen = (HPEN)SelectObject(hdc, hPen);
                    if(g_sel_cn<TWOC1){
                        MoveToEx(hdc, CIXS+(g_sel_cn%CSPLINE)*WX, CIYS+(g_sel_cn/CSPLINE)*WY, NULL);
                        LineTo(hdc, CIXS+(g_sel_cn%CSPLINE)*WX, CIYS+(g_sel_cn/CSPLINE)*WY+WY);
                        LineTo(hdc, CIXS+(g_sel_cn%CSPLINE)*WX+WX, CIYS+(g_sel_cn/CSPLINE)*WY+WY);
                        LineTo(hdc, CIXS+(g_sel_cn%CSPLINE)*WX+WX, CIYS+(g_sel_cn/CSPLINE)*WY);
                        LineTo(hdc, CIXS+(g_sel_cn%CSPLINE)*WX, CIYS+(g_sel_cn/CSPLINE)*WY);
                    }
                    else{
                        int ti=g_sel_cn-TWOC1;
                        MoveToEx(hdc, CIXS1+(ti%CSPLINE2)*WX, CIYS1+(ti/CSPLINE2)*WY, NULL);
                        LineTo(hdc, CIXS1+(ti%CSPLINE2)*WX, CIYS1+(ti/CSPLINE2)*WY+WY);
                        LineTo(hdc, CIXS1+(ti%CSPLINE2)*WX+WX, CIYS1+(ti/CSPLINE2)*WY+WY);
                        LineTo(hdc, CIXS1+(ti%CSPLINE2)*WX+WX, CIYS1+(ti/CSPLINE2)*WY);
                        LineTo(hdc, CIXS1+(ti%CSPLINE2)*WX, CIYS1+(ti/CSPLINE2)*WY);
                    }
                    SelectObject(hdc, orgPen);
                    DeleteObject(hPen);
                }
                SelectObject(hdc, oft);
                EndPaint(hwnd, &ps);
            }
            break;
        case WM_DESTROY:
            DeleteObject(hFont);
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}
