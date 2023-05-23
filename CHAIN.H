
#define IDM_RESOURCE 1

#define IDM_CNTRL 1
#define IDM_SETUP 2
#define IDD_DIALOG 3
#define WM_CALC_DONE (WM_USER + 0)
#define IDM_START 10
#define IDM_STOP 11
#define IDM_STEP 12
#define IDM_CONTINUE 13
#define IDM_CLEAR 14
#define IDM_INC 15
#define IDM_DEC 16
#define IDM_PRIO 17

#define IDM_DIALOG 20
#define IDM_DINER 40
#define IDM_4CELL 42
#define IDM_8CELL 41
#define IDM_ARRAYS 21
#define IDM_ARRAYST 22
#define IDM_NUMBERT 23
#define IDM_NUMBER 24
#define IDM_OK 25
#define IDM_CANCEL 26
#define IDM_AUTOR 27

#define STACKSIZE 0x8000UL

void EnableMenuItem(HWND , SHORT , BOOL );

typedef struct {
    HWND hwnd;
    HPS hps;
    int startf;
    int drawn;
    BITMAPINFO2  *pbmi;
    HBITMAP hbm;
    char *abitmap;
    int sb8ob4;
    USHORT autors;
    char *old;
    char *new;
} CALCPARM;

typedef CALCPARM *PCALCPARM;

