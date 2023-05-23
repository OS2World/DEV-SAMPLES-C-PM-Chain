/*---------------------------------------------
   CHAIN.C -- Cellular Automata Sample Program
  --------------------------------------------*/
#define INCL_WIN
#define INCL_DOS
#define INCL_GPI
#define INCL_DEV
#define INCL_ERRORS

#include <os2.h>
#include <stdlib.h>
#include <memory.h>
#include <process.h>
#include <malloc.h>
#include <time.h>
#include "chain.h"

MRESULT EXPENTRY ClientWndProc (HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY SetupDlgProc (HWND, ULONG, MPARAM, MPARAM );

VOID runThread(VOID *);
VOID randarr(char *, char * );
VOID copy_arr (char *, char * );
/* my declarations */
VOID set_arr( unsigned int, unsigned int, int, char * );
VOID process_arr4(char *, char *, unsigned int *, char *);
VOID process_arr8(char *, char *, unsigned int *, char *);
VOID ClkShowFrameControls ( HWND ) ;
VOID ClkHideFrameControls ( HWND ) ;
VOID ReDrawWin ( HWND , HPS );
VOID SetupChain(VOID);


/* my global variables */
HAB hab = NULLHANDLE;
BOOL fControlsHidden = FALSE ;
HWND hwndTitleBar = NULLHANDLE , hwndSysMenu = NULLHANDLE , hwndMinMax = NULLHANDLE , hwndMenu = NULLHANDLE ;
HWND hwndFrame = NULLHANDLE, hwndClient = NULLHANDLE;
HDC          hdc = NULLHANDLE;
CALCPARM     cp = {0};
HBITMAP      hbmprev = NULLHANDLE;
SIZEL       sizel = {0};
BITMAPINFOHEADER2 bmp = {0};
TID          tid = 0;

unsigned long unicount;

unsigned int bxsize;
unsigned int bysize;

typedef struct {
    ULONG axsize;
    ULONG aysize;
    ULONG states;
    ULONG sb8ob4;
    ULONG autors;
} MMPARMS;

typedef MMPARMS * MMPARMSP;

MMPARMS mmparms = { 41, 41, 16, 0, 0 };

/* standard windows - pm main routine here */
int main (void)
{
    static CHAR szClientClass[] = "chain";
    static ULONG flFrameFlags = FCF_TITLEBAR | FCF_SYSMENU |
        FCF_SIZEBORDER | FCF_MINMAX | FCF_SHELLPOSITION |
        FCF_TASKLIST | FCF_MENU;
    HMQ hmq;
    QMSG qmsg;

    hab = WinInitialize(0);
    hmq = WinCreateMsgQueue (hab, 0 );

    WinRegisterClass (hab, (PCSZ) szClientClass, (PFNWP) ClientWndProc, CS_SIZEREDRAW, 0 );

    hwndFrame = WinCreateStdWindow ( HWND_DESKTOP, WS_VISIBLE,
        &flFrameFlags, (PCSZ) szClientClass, (PCSZ) szClientClass, 0L, NULLHANDLE, IDM_RESOURCE, &hwndClient );

    WinSendMsg (hwndFrame, WM_SETICON,
        MPFROMLONG(WinQuerySysPointer( HWND_DESKTOP, SPTR_APPICON, FALSE )),
        MPVOID );

    while (WinGetMsg (hab, &qmsg, NULLHANDLE, 0, 0))
        WinDispatchMsg (hab, &qmsg );

    WinDestroyWindow (hwndFrame );
    WinDestroyMsgQueue (hmq);
    WinTerminate (hab);
    return 0;

}
/* client window proceedure here */
MRESULT EXPENTRY ClientWndProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    HPS                 hps;
    int i, count;
    POINTL      apl[4];
    RECTL        rcl;
    LONG     gcolor;
    RGB      *rcolor;

    switch (msg)
    {
        /* tell pm to track the frame on 1 button down */
    	case WM_BUTTON1DOWN :
	        return WinSendMsg ( WinQueryWindow(hwnd, QW_PARENT) , WM_TRACKFRAME ,
		        MPFROMSHORT(TF_MOVE) , MPVOID ) ;
        /* tell pm to hide controls on double click */
    	case WM_BUTTON1DBLCLK :
	        if ( fControlsHidden )
               ClkShowFrameControls ( hwndFrame ) ;
    	    else
               ClkHideFrameControls ( hwndFrame ) ;
    	    return MRFROMLONG(0);
        /* start up and create first bitmap here */
     case WM_CREATE:
            /* set menu items to desired state here */
            EnableMenuItem ( hwnd, IDM_START, TRUE );
            EnableMenuItem ( hwnd, IDM_STOP, FALSE );
            EnableMenuItem ( hwnd, IDM_SETUP, TRUE );
            /* fill out bit map header info */
            bmp.cbFix = FIELDOFFSET(BITMAPINFOHEADER2,ulCompression);
            bmp.cx = mmparms.axsize;
            bmp.cy = mmparms.aysize;
            bmp.cPlanes = 1;
            bmp.cBitCount = 4;
            /* malloc array for colors */
            cp.pbmi = malloc( sizeof(BITMAPINFO2) + (sizeof(RGB2) * 15) );
            /* fill out bit map header info here, must be duplicate */
            cp.pbmi->cbFix = FIELDOFFSET(BITMAPINFO2,ulCompression);
            cp.pbmi->cx = mmparms.axsize;
            cp.pbmi->cy = mmparms.aysize;
            cp.pbmi->cPlanes = 1;
            cp.pbmi->cBitCount = 4;

            /* get a handle to current presentation space */
            hps = WinGetScreenPS ( HWND_DESKTOP);
            /* get the current colors used by the Presentation space */
            for ( i = 0 ; i < 16; ++i )
            {
                gcolor = GpiQueryRGBColor( hps, (ULONG)NULL, (LONG)i );
                rcolor = (RGB *)&gcolor;
                cp.pbmi->argbColor[i].bBlue = rcolor->bBlue;
                cp.pbmi->argbColor[i].bGreen = rcolor->bGreen;
                cp.pbmi->argbColor[i].bRed = rcolor->bRed;
            }
            /* calculate first level bitmap size in bytes */
            bxsize = (((((mmparms.axsize*4)/8)/4)+1 ) *4);

            bysize = mmparms.aysize;
            /* allocate space for first level bitmap */
            cp.abitmap = (char *) malloc ( bxsize * bysize );
            /* check alloc was good */
            if ( NULL == cp.abitmap )
            {
                WinAlarm ( HWND_DESKTOP, WA_ERROR );
                return MRFROMLONG(0);
            }
            /* allocate space for my byte maps */
            cp.old = (char *) malloc ( mmparms.axsize * mmparms.aysize );
            /* need two of them */
            cp.new = (char *) malloc ( mmparms.axsize * mmparms.aysize );
            /* check allocate was ok */
            if ( cp.old == NULL )
            {
                WinAlarm ( HWND_DESKTOP, WA_ERROR );
                return MRFROMLONG(0);
            }
            if ( cp.new == NULL )
            {
                if ( cp.old != NULL )
                {
                    free(cp.old);
                }
                WinAlarm ( HWND_DESKTOP, WA_ERROR );
                return MRFROMLONG(0);
            }
            /* randomize my byte map */
            randarr( cp.old, cp.new );
            /* copy byte map to first level bitmap */
            copy_arr ( cp.old, cp.abitmap );
            /* setup size variables for bitmap generation */
            sizel.cx = mmparms.axsize;
            sizel.cy = mmparms.aysize;
            /* get a memory device context */
            hdc = DevOpenDC
                ( hab, OD_MEMORY, (PCSZ) "*", 0L, NULL, NULLHANDLE );
            /* Create my own presentaion space */
            /* associate it with my memory device context */
            cp.hps = GpiCreatePS
                ( hab, hdc, &sizel, PU_PELS | GPIT_MICRO | GPIA_ASSOC );
            /* create a bitmap for the presentation space */
            /* this is the second level bitmap */
            cp.hbm = GpiCreateBitmap
                ( cp.hps, &bmp, CBM_INIT, (PBYTE) cp.abitmap, cp.pbmi );
            /* assign the second level bitmap to the presentation space */
            hbmprev = GpiSetBitmap ( cp.hps, cp.hbm );
            /* and were done with create */
            return MRFROMLONG(0);
        case WM_SIZE:
            /* do nothing for size command */
            return MRFROMLONG(0);
        case WM_PAINT:
            /* do standard begin paint command */
            hps = WinBeginPaint ( hwnd, NULLHANDLE, &rcl );
            /* get current rectangle size */
            WinQueryWindowRect ( hwnd, &rcl );
            /* fill out two rectangle sizes */
            apl[0].x = rcl.xLeft;
            apl[0].y = rcl.yBottom;
            apl[1].x = rcl.xRight;
            apl[1].y = rcl.yTop;
            apl[2].x = 0L;
            apl[2].y = 0L;
            apl[3].x = (LONG)mmparms.axsize;
            apl[3].y = (LONG)mmparms.aysize;
            /* copy first level bitmap to second level bitmap */
            GpiSetBitmapBits ( cp.hps, 0L, (LONG)bysize, (const BYTE *)
								cp.abitmap, (const BITMAPINFO2 *) cp.pbmi );
            /* bitblt the bitmap presentation space to the screen PS */
            GpiBitBlt ( hps, cp.hps, 4L, apl, ROP_SRCCOPY, BBO_IGNORE );
            /* flag we have drawn to thread */
            cp.drawn = 0;
            /* end the paint session */
            WinEndPaint ( hps );
            /* and were outta here */
            return MRFROMLONG(0);

        case WM_DESTROY:
            /* destroy command from the close box */
            if (cp.startf)
            {
                cp.startf = 0;
                DosWaitThread(&tid,DCWW_WAIT);
            }
            /* free things up */
            free ( cp.pbmi );
            free ( cp.old );
            free ( cp.new );
            /* delete things */
            GpiDeleteBitmap ( cp.hbm );
            /* delete my PS */
            GpiDestroyPS ( cp.hps );
            /* close out my memory device context */
            DevCloseDC ( hdc );
            /* were done */
            return MRFROMLONG(0);
        case WM_CALC_DONE:
            /* set menu items the way we want'em */
            EnableMenuItem ( hwnd, IDM_START, TRUE );
            EnableMenuItem ( hwnd, IDM_STOP, FALSE );
            EnableMenuItem ( hwnd, IDM_SETUP, TRUE );
            EnableMenuItem ( hwnd, IDM_STEP, TRUE );
            EnableMenuItem ( hwnd, IDM_CONTINUE, TRUE );
            return MRFROMLONG(0);
        case WM_COMMAND:
            /* proccess menu commands here, first find out which one */
            switch (COMMANDMSG(&msg)->cmd)
            {
            /* do continue command here */
            case IDM_CONTINUE:
                /* set variable for thread, skip initialize */
                cp.startf = 0;
                goto skip;
            case IDM_START:
                /* redo all bitmap arrays, in case parameters have changed */
                SetupChain();
skip:
                /* set initial state of thread variables */
                cp.drawn = 0;
                cp.sb8ob4 = mmparms.sb8ob4;
                cp.autors = mmparms.autors;
                cp.hwnd = hwnd;

                /* some no longer used variable */
                unicount = 0L;
                /* startup thread */
                if ( -1 == (tid = _beginthread ( runThread,
                    NULL, STACKSIZE, &cp)) )
                {
                    WinAlarm ( HWND_DESKTOP, WA_ERROR );
                    return MRFROMLONG(0);
                }
                /* set priority of thread to very low */
                DosSetPriority (PRTYS_THREAD, PRTYC_IDLETIME, PRTYD_MINIMUM, tid );
                /* set menu items to proper state */
                EnableMenuItem ( hwnd, IDM_START, FALSE );
                EnableMenuItem ( hwnd, IDM_STOP, TRUE );
                EnableMenuItem ( hwnd, IDM_SETUP, FALSE );
                EnableMenuItem ( hwnd, IDM_STEP, FALSE );
                EnableMenuItem ( hwnd, IDM_CONTINUE, FALSE );
                EnableMenuItem ( hwnd, IDM_CLEAR, TRUE );
                return MRFROMLONG(0);
            case IDM_STOP:
                /* to stop thread proccessing hit this variable */
                cp.startf = 0;
                return MRFROMLONG(0);
            case IDM_SETUP:
                /* call dialog box handler */
                i = WinDlgBox ( HWND_DESKTOP, hwnd, SetupDlgProc,
                    NULLHANDLE, IDD_DIALOG, &mmparms );
                if ( !i )
                {
                    SetupChain();
                    /* copy parameters/variables */
                    cp.drawn = 0;
                    cp.sb8ob4 = mmparms.sb8ob4;
                    cp.autors = mmparms.autors;
                    /* re randomize array */
                    randarr( cp.old, cp.new );
                    /* move my byte map into first level bitmap */
                    copy_arr ( cp.old, cp.abitmap );
                    /* invalidate rectangle to force redraw */
                    WinInvalidateRect( hwnd, NULL, FALSE );
                    /* set menu items to desired state */
                    EnableMenuItem ( hwnd, IDM_CONTINUE, i );
                    EnableMenuItem ( hwnd, IDM_STEP, i );
                    EnableMenuItem ( hwnd, IDM_CLEAR, i );
                }
                /* were outta here */
                return MRFROMLONG(0);
            case IDM_STEP:
                /* check which processing routine to call */
                /* call life chain process my byte map */
                if ( cp.sb8ob4)
                    process_arr4( cp.old, cp.new, (unsigned int *) &count, cp.abitmap );
                else
                    process_arr8( cp.old, cp.new, (unsigned int *) &count, cp.abitmap );
                /* move my byte map into first level bitmap */
                copy_arr( cp.old, (char *) cp.abitmap );
                /* invalidate rectangle to force redraw */
                WinInvalidateRect( hwnd, NULL, FALSE );
                return MRFROMLONG(0);
            /* reinitialize my chain array */
            case IDM_CLEAR:
                /* re randomize it */
                randarr( cp.old, cp.new );
                /* copy my byte map into first level bitmap */
                copy_arr( cp.old, (char *) cp.abitmap );
                /* set first level bitmap into second level bitmap */
                GpiSetBitmapBits
                    ( cp.hps, 0L, (LONG)bysize, (const BYTE *) cp.abitmap, (const BITMAPINFO2 *) cp.pbmi );
                /* invalidate the rectangle to force redraw */
                WinInvalidateRect( hwnd, NULL, FALSE );
                return MRFROMLONG(0);
            /* currently unused */
            case IDM_INC:
                DosSetPriority( PRTYS_THREAD, PRTYC_NOCHANGE, 10, tid );
                return MRFROMLONG(0);
            /* currently unused */
            case IDM_DEC:
                DosSetPriority( PRTYS_THREAD, PRTYC_NOCHANGE, -10, tid );
                return MRFROMLONG(0);
            }
    }
    /* standard default pm return */
    return WinDefWindowProc (hwnd, msg, mp1, mp2 );
}
/* generic delete old bitmaps and setup new ones routines */
void SetupChain(void)
{
    if (hbmprev) {
       /* set my PS space back to original bitmap */
       GpiSetBitmap ( cp.hps, hbmprev );
       /* delete my second level bitmap */
       GpiDeleteBitmap ( cp.hbm );
       /* delete my PS */
       GpiDestroyPS ( cp.hps );
       /* close out my memory device context */
       DevCloseDC ( hdc );
    } /* endif */
    /* set up start flag for thread */
    cp.startf = 1;
    /* delete my byte maps */
    if ( cp.old != NULL )
    {
        free ( cp.old );
    }
    if ( cp.new != NULL )
    {
        free ( cp.new );
    }
    /* delete the first level bitmap storage */
    if ( NULL != cp.abitmap )
    {
        free( cp.abitmap );
    }
    /* recalculate my first level bitmap sizes */

    bxsize = (((((mmparms.axsize*4)/8)/4)+1 ) *4);

    bysize = mmparms.aysize;
    /* re allocate my first leve bitmap space */
    cp.abitmap = (char *) malloc ( bxsize * bysize );
    if ( NULL == cp.abitmap )
    {
        WinAlarm ( HWND_DESKTOP, WA_ERROR );
        return;
    }
    /* realloc my byte map sizes */
    cp.old = (char *) malloc ( mmparms.axsize * mmparms.aysize );
    cp.new = (char *) malloc ( mmparms.axsize * mmparms.aysize );
    /* test the allocations */
    if ( cp.old == NULL )
    {
        WinAlarm ( HWND_DESKTOP, WA_ERROR );
        return;
    }
    if ( cp.new == NULL )
    {
        if ( cp.old != NULL )
        {
            free(cp.old);
        }
        WinAlarm ( HWND_DESKTOP, WA_ERROR );
        return;
    }
    /* fill out all of my internal variables */
    /* so theres a lot of duplication, lets just say i never */
    /* finished the program really */
    sizel.cx = mmparms.axsize;
    sizel.cy = mmparms.aysize;
    bmp.cx = mmparms.axsize;
    bmp.cy = mmparms.aysize;
    cp.pbmi->cx = mmparms.axsize;
    cp.pbmi->cy = mmparms.aysize;

    /* open a memory device context */
    hdc = DevOpenDC
        ( hab, OD_MEMORY, (PCSZ) "*", 0L, NULL, NULLHANDLE );
    /* create myself a presentation space */
    cp.hps = GpiCreatePS
        ( hab, hdc, &sizel, PU_PELS | GPIT_MICRO | GPIA_ASSOC );
    /* create myself a bitmap out of my first level bitmap */
    cp.hbm = GpiCreateBitmap
        ( cp.hps, &bmp, CBM_INIT,
					(PBYTE) cp.abitmap, (const BITMAPINFO2 *) cp.pbmi );
    /* assign my second level bitmap to my presentation space */
    hbmprev = GpiSetBitmap ( cp.hps, cp.hbm );

    return;
}

/* dialog box handler */
MRESULT EXPENTRY SetupDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    /* everything static in pm, not what I call effecient!!! */
    static MMPARMS mmparmsl;
    static MMPARMSP mmparmsp;
    switch ( msg )
    {
    case WM_INITDLG:
        /* init the dialog box */
        /* get a pointer to passed user structure */
        mmparmsp = MPFROMP(mp2);
        /* copy the contents of the structure to our local */
        mmparmsl = *mmparmsp;
        /* put values into the dialog box items */
        WinSetDlgItemShort ( hwnd, IDM_ARRAYS, mmparmsl.axsize, FALSE );
        WinSetDlgItemShort ( hwnd, IDM_NUMBER, mmparmsl.states, FALSE );
        /* setup radio buttons this way or */
        if (mmparmsl.sb8ob4 )
        {
            WinSendDlgItemMsg( hwnd, IDM_8CELL, BM_SETCHECK,
                MPFROM2SHORT (FALSE, 0 ), NULL );
            WinSendDlgItemMsg( hwnd, IDM_4CELL, BM_SETCHECK,
                MPFROM2SHORT (TRUE, 0 ), NULL );
        }
        /* that way */
        else
        {
            WinSendDlgItemMsg( hwnd, IDM_4CELL, BM_SETCHECK,
                MPFROM2SHORT (FALSE, 0 ), NULL );
            WinSendDlgItemMsg( hwnd, IDM_8CELL, BM_SETCHECK,
                MPFROM2SHORT (TRUE, 0 ), NULL );
        }
        /* setup check boxes */
        WinSendDlgItemMsg ( hwnd, IDM_AUTOR, BM_SETCHECK,
            MPFROM2SHORT ( mmparmsl.autors, 0 ), NULL );
        /* bring dialog box into focus */
        WinSetFocus( HWND_DESKTOP,
            WinWindowFromID ( hwnd, IDM_ARRAYS ));
        return MRFROMLONG(1);
     case WM_CONTROL:
        /* control functions */
        /* get message id of user action */
        switch ( SHORT1FROMMP ( mp1 ) )
        {
        /* switch the radio buttons */
        case IDM_4CELL:
            WinSendDlgItemMsg( hwnd, IDM_8CELL, BM_SETCHECK,
                MPFROM2SHORT (FALSE, 0 ), NULL );
            WinSendDlgItemMsg( hwnd, IDM_4CELL, BM_SETCHECK,
                MPFROM2SHORT (TRUE, 0 ), NULL );
            mmparmsl.sb8ob4 = 1;
            break;
        /* switch the radio buttons */
        case IDM_8CELL:
            WinSendDlgItemMsg( hwnd, IDM_4CELL, BM_SETCHECK,
                MPFROM2SHORT (FALSE, 0 ), NULL );
            WinSendDlgItemMsg( hwnd, IDM_8CELL, BM_SETCHECK,
                MPFROM2SHORT (TRUE, 0 ), NULL );
            mmparmsl.sb8ob4 = 0;
            break;
        /* do the check box */
        case IDM_AUTOR:
            if ( mmparmsl.autors )
            {
                WinSendDlgItemMsg ( hwnd, IDM_AUTOR, BM_SETCHECK,
                    MPFROM2SHORT ( FALSE, 0 ), NULL );
                mmparmsl.autors = 0;
            }
            else
            {
                WinSendDlgItemMsg ( hwnd, IDM_AUTOR, BM_SETCHECK,
                    MPFROM2SHORT ( TRUE, 0 ), NULL );
                mmparmsl.autors = 1;
            }

        }
        return MRFROMLONG(0);
     case WM_COMMAND:
        switch ( COMMANDMSG(&msg)->cmd)
        {
        case IDM_OK:
            WinQueryDlgItemShort(hwnd, IDM_ARRAYS, (PSHORT) &mmparmsl.axsize, FALSE );
            /* do some range checking */
            /* for some reason i never understood 200 does not work right */
            /* but 1 - 199 and 201 - BIG  works fine, hummmmmm */
            /* well, I now know that multiples of 40 do not work. */
            /* but i cannot find out why. Must be bug in PM */
            if ( mmparmsl.axsize > 199 )
            {
                mmparmsl.axsize = 199;
            }
            mmparmsl.aysize = mmparmsl.axsize;
            WinQueryDlgItemShort(hwnd, IDM_NUMBER, (PSHORT) &mmparmsl.states, FALSE );
            if ( mmparmsl.states >= 33 )
            {
                mmparmsl.states = 32;
            }
            mmparmsl.autors = SHORT1FROMMR ( WinSendDlgItemMsg
                ( hwnd, IDM_AUTOR, BM_QUERYCHECK, NULL, NULL ) );
            *mmparmsp = mmparmsl;
            WinDismissDlg( hwnd, FALSE );
            return MRFROMLONG(0);
        case IDM_CANCEL:
            WinDismissDlg( hwnd, TRUE );
            return MRFROMLONG(0);
        }
        break;
    }
    return WinDefDlgProc ( hwnd, msg, mp1, mp2 );
}

/* my thread to constantly redo my CA array */

VOID runThread(VOID *p)
{
    PCALCPARM pcp = (PCALCPARM)p;
    unsigned int count;
    unsigned int bignum;
    // HPS hps;

    /* calculate internal value for auto restart feature */
    bignum = (mmparms.axsize * mmparms.aysize) -
        ((mmparms.axsize * mmparms.aysize) / 15);
    /* check if this is a continue or start */
    if ( pcp->startf == 1 )
    {
        randarr( pcp->old, pcp->new );
        copy_arr( pcp->old, (char *) pcp->abitmap );
    }
    else
    {
        pcp->startf = 1;
    }
    /* loop until parent process cancels this process */
    while (pcp->startf)
    {
        /* set count to zero for auto restart feature */
        count = 0;
        /* check which CA routine to call */
        /* CA = Cellular Automation */
        if ( pcp->sb8ob4)
            process_arr4( pcp->old, pcp->new, &count, pcp->abitmap );
        else
            process_arr8( pcp->old, pcp->new, &count, pcp->abitmap );
        /* logic for auto restart feature */
        if ( (count == 0 || count > bignum) && pcp->autors )
        {
            randarr( pcp->old, pcp->new );
            copy_arr( pcp->old, (char *) pcp->abitmap );
        }
        /* this looks like a good idea, but never seems to go into effect */
        while ( pcp->drawn )
        {
            unicount ++;
            DosSleep(0L);
        }
        /* flag need to redraw */
        pcp->drawn = 1;
        /* invalidate window rectangle to force redraw of window */
        WinInvalidateRect( pcp->hwnd, NULL, FALSE );
    }
    /* post a done message to parent process */
    WinPostMsg ( pcp->hwnd, WM_CALC_DONE, NULL, NULL );
    /* and were outta here */
    return;
}

/* this routine copies my byte map array into first level bitmap array */
VOID copy_arr( char *old, char *abitmap )
{
    /* position variables */
    unsigned int x, y;
    /* char pointer  */
    char *cp;
    /* loop on y axis size */
    for (y = 0; y < mmparms.aysize; ++y )
    {
        /* and loop on X axis size */
        for ( x= 0; x < mmparms.axsize; ++x)
        {
            /* point into correct place in first level bitmap */
            cp = (abitmap + (y*bxsize) + (x/2));
            /* test to see if this bitmap byte need to be initialized */
            if ( x % 2 == 0 )
            {
                *cp = 0;
            }
            /* or in the nibble from the byte map */
            /* into contents of first level bitmap pointer */
            *cp |= ((*(old + (y*mmparms.axsize)+x))%16) << ((1-(x%2)) * 4);
        }
    }
/*
    for (i=0; i<bxsize; ++i )
    {
        set_arr ( i, i, 0, abitmap );
    }
*/
}
/* this rountine sets a specific pixel in the first level bitmap */
/* use x and y and k = value to set into bitmap */
VOID set_arr( unsigned int y, unsigned int x, int k, char *abitmap )
{
    char *cp;
    char c;
    /* point into destination bitmap */
    cp = (abitmap + (y*bxsize) + (x/2));
    c = *cp;
    /* initialize proper nibble */
    if ( x % 2 == 1 )
    {
        c &= 0xf0;
    }
    else
    {
        c &= 0x0f;
    }
    /* set the nibble in the bitmap, do not destroy other nibble */
    *cp = (c | (k << ((1-(x%2)) * 4))) ;
}

/* here we process byte map array */
/* this is the CA routine. In this version, each cell looks around at */
/* eight neighbors */
/* both of these rountes are highly optimized, and I dodn't feel like */
/* explaining it all. I'm sure you can understand everything if you */
/* refer to the original Scientific American Computer Recreations article */
/* in the DEC 89 issue */
/* I will put some comments in here, but I am not going to make things */
/* cystal clear. There is just too much background requiered to follow */
/* all this. Of course if you find all this intuitively obvious */
/* then remember the old lowest common denominator syndrome */

VOID process_arr8( char *old, char *new, unsigned int *count, char *abitmap )
{
    // POINTL apoint;
    unsigned int i, j;
    char k;
    /* my byte pointers */
    /* home new, home old, minus 1 minus 1, minus plus 1, home plus 1 */
    /* etc */
    char *homo, *homn, *m1m1, *m1h, *m1p1, *hp1, *hm1, *p1m1, *p1h, *p1p1;

    /* outer loop on y axis size */
    for ( i = 0; i < mmparms.aysize; ++i )
    {
        /* if this is column zero */
        if ( i == 0 )
        {
            /* calc all pointer starting points */
            homo = old + (i * mmparms.axsize);
            homn = new + (i * mmparms.axsize);
            m1m1 = old + ( (mmparms.aysize-1) * mmparms.axsize) + mmparms.axsize - 1;
            m1h  = old + ( (mmparms.aysize-1) * mmparms.axsize);
            m1p1 = old + ( (mmparms.aysize-1) * mmparms.axsize) + 1;
            hp1  = old + (i * mmparms.axsize) + 1;
            hm1  = old + (i * mmparms.axsize) + mmparms.axsize-1;
            p1p1 = old + ( (i+1) * mmparms.axsize) + 1;
            p1h  = old + ( (i+1) * mmparms.axsize);
            p1m1 = old + ( (i+1) * mmparms.axsize) + mmparms.axsize-1;
        }
        /* else if this is a middle column */
        /* calc all starting points differently */
        else if ( i == mmparms.aysize-1 )
        {
            homo = old + (i * mmparms.axsize);
            homn = new + (i * mmparms.axsize);
            m1m1 = old + ( (i-1) * mmparms.axsize) + mmparms.axsize - 1;
            m1h  = old + ( (i-1) * mmparms.axsize);
            m1p1 = old + ( (i-1) * mmparms.axsize) + 1;
            hp1  = old + (i * mmparms.axsize) + 1;
            hm1  = old + (i * mmparms.axsize) + mmparms.axsize-1;
            p1p1 = old + 1;
            p1h  = old ;
            p1m1 = old + mmparms.axsize-1;
        }
        /* else this is last column */
        else
        {
            homo = old + (i * mmparms.axsize);
            homn = new + (i * mmparms.axsize);
            m1m1 = old + ( (i-1) * mmparms.axsize) + mmparms.axsize - 1;
            m1h  = old + ( (i-1) * mmparms.axsize);
            m1p1 = old + ( (i-1) * mmparms.axsize) + 1;
            hp1  = old + (i * mmparms.axsize) + 1;
            hm1  = old + (i * mmparms.axsize) + mmparms.axsize-1;
            p1p1 = old + ( (i+1) * mmparms.axsize) + 1;
            p1h  = old + ( (i+1) * mmparms.axsize);
            p1m1 = old + ( (i+1) * mmparms.axsize) + mmparms.axsize-1;
        }
        /* remember we are doing the old toroid ring logic back there */

        /* find old home cells edible cell value */
        k = ((*homo) + 1) % mmparms.states;
        /* check first neighbor to see if edible */
        if ( ( (*m1m1++) == k ) )
        {
            /* assign the new home cell its new value */
            *homn = k;
            /* increment all other neighbor pointers */
            m1h++; m1p1++; hp1++; hm1++; p1p1++; p1h++; p1m1++;
        }
        /* check next neighbor, if needed */
        else if ( ( (*m1h++) == k ) )
        {
            /* assign new value if true */
            *homn = k;
            /* increment all neighbor pointers */
            m1p1++; hp1++; hm1++; p1p1++; p1h++; p1m1++;
        }
        /* and all the rest of the neighbor */
        else if ( ( (*m1p1++) == k ) )
        {
            /* if true assign new home cell its new value */
            *homn = k;
            hp1++; hm1++; p1p1++; p1h++;  p1m1++;
        }
        else if ( ( (*hp1++) == k ) )
        {
            *homn = k;
            hm1++; p1p1++; p1h++; p1m1++;
        }
        else if ( ( (*hm1++) == k ) )
        {
            *homn = k;
            p1p1++; p1h++; p1m1++;
        }
        else if ( ( (*p1p1++) == k ) )
        {
            *homn = k;
            p1h++; p1m1++;
        }
        else if ( ( (*p1h++) == k ) )
        {
            *homn = k;
            p1m1++;
        }
        else if ( ( (*p1m1++) == k ) )
        {
            *homn = k;
        }
        if ( *homn == k)
        {
            ++(*count);
            set_arr( i, 0, k, abitmap );
        }
        if ( i == 0 )
        {
            m1m1 = old + ((mmparms.aysize-1) * mmparms.axsize);
            p1m1 = old + ((i+1) * mmparms.axsize );
        }
        else if ( i == mmparms.aysize-1 )
        {
            m1m1 = old + ((i-1) * mmparms.axsize );
            p1m1 = old;
        }
        else
        {
            m1m1 = old + ((i-1) * mmparms.axsize );
            p1m1 = old + ((i+1) * mmparms.axsize );
        }
        hm1  = old + ( i * mmparms.axsize );

        ++homo;
        ++homn;

        for ( j = 1; j < mmparms.axsize-1; ++j )
        {
            k = ((*homo) + 1) % mmparms.states;
            if ( ( (*m1m1++) == k ) )
            {
                *homn = k;
                m1h++; m1p1++; hp1++; hm1++; p1p1++; p1h++; p1m1++;
            }
            else if ( ( (*m1h++) == k ) )
            {
                *homn = k;
                m1p1++; hp1++; hm1++; p1p1++; p1h++; p1m1++;
            }
            else if ( ( (*m1p1++) == k ) )
            {
                *homn = k;
                hp1++; hm1++; p1p1++; p1h++; p1m1++;
            }
            else if ( ( (*hp1++) == k ) )
            {
                *homn = k;
                hm1++;  p1p1++; p1h++; p1m1++;
            }
            else if ( ( (*hm1++) == k ) )
            {
                *homn = k;
                p1p1++; p1h++; p1m1++;
            }
            else if ( ( (*p1p1++) == k ) )
            {
                *homn = k;
                p1h++; p1m1++;
            }
            else if ( ( (*p1h++) == k ) )
            {
                *homn = k;
                p1m1++;
            }
            else if ( ( (*p1m1++) == k ) )
            {
                *homn = k;
            }
            if ( *homn == k)
            {
                ++(*count);
                set_arr( i, j, k, abitmap );
            }
            ++homo;
            ++homn;
        }
        if ( i == 0 )
        {
            m1p1 = old + (mmparms.aysize-1) * mmparms.axsize;
            p1p1 = old + (i+1) * mmparms.axsize;
        }
        else if ( i == mmparms.aysize-1 )
        {
            m1p1 = old + (i-1) * mmparms.axsize;
            p1p1 = old;
        }
        else
        {
            m1p1 = old + (i-1) * mmparms.axsize;
            p1p1 = old + (i+1) * mmparms.axsize;
        }
        hp1  = old + i * mmparms.axsize;
        k = ((*homo) + 1) % mmparms.states;
        if ( ( (*m1m1++) == k ) )
        {
            *homn = k;
            m1h++; m1p1++; hp1++; hm1++; p1p1++; p1h++; p1m1++;
        }
        else if ( ( (*m1h++) == k ) )
        {
            *homn = k;
            m1p1++; hp1++; hm1++; p1p1++; p1h++; p1m1++;
        }
        else if ( ( (*m1p1++) == k ) )
        {
            *homn = k;
            hp1++; hm1++; p1p1++; p1h++; p1m1++;
        }
        else if ( ( (*hp1++) == k ) )
        {
            *homn = k;
            hm1++; p1p1++; p1h++; p1m1++;
        }
        else if ( ( (*hm1++) == k ) )
        {
            *homn = k;
            p1p1++; p1h++; p1m1++;
        }
        else if ( ( (*p1p1++) == k ) )
        {
            *homn = k;
            p1h++; p1m1++;
        }
        else if ( ( (*p1h++) == k ) )
        {
            *homn = k;
            p1m1++;
        }
        else if ( ( (*p1m1++) == k ) )
        {
            *homn = k;
        }
        if ( *homn == k)
        {
            ++(*count);
            set_arr( i, j, k, abitmap );
        }
        ++homo;
        ++homn;
    }
    /* old[i][j] = new[i][j]; */
    memcpy ( old, new, mmparms.aysize*mmparms.axsize );
}
VOID process_arr4( char *old, char *new, unsigned int *count, char *abitmap)
{
    unsigned int i, j;
    char k;
    char *homo, *homn, *m1h, *hp1, *hm1, *p1h;

    for ( i = 0; i < mmparms.aysize; ++i )
    {
        if ( i == 0 )
        {
            homo = old + (i * mmparms.axsize);
            homn = new + (i * mmparms.axsize);
            m1h  = old + ( (mmparms.aysize-1) * mmparms.axsize);
            hp1  = old + (i * mmparms.axsize) + 1;
            hm1  = old + (i * mmparms.axsize) + mmparms.axsize-1;
            p1h  = old + ( (i+1) * mmparms.axsize);
        }
        else if ( i == mmparms.aysize-1 )
        {
            homo = old + (i * mmparms.axsize);
            homn = new + (i * mmparms.axsize);
            m1h  = old + ( (i-1) * mmparms.axsize);
            hp1  = old + (i * mmparms.axsize) + 1;
            hm1  = old + (i * mmparms.axsize) + mmparms.axsize-1;
            p1h  = old ;
        }
        else
        {
            homo = old + (i * mmparms.axsize);
            homn = new + (i * mmparms.axsize);
            m1h  = old + ( (i-1) * mmparms.axsize);
            hp1  = old + (i * mmparms.axsize) + 1;
            hm1  = old + (i * mmparms.axsize) + mmparms.axsize-1;
            p1h  = old + ( (i+1) * mmparms.axsize);
        }
        k = ((*homo) + 1) % mmparms.states;
        if ( ( (*m1h++) == k ) )
        {
            *homn = k;
            hp1++; hm1++; p1h++;
        }
        else if ( ( (*hp1++) == k ) )
        {
            *homn = k;
            hm1++; p1h++;
        }
        else if ( ( (*hm1++) == k ) )
        {
            *homn = k;
            p1h++;
        }
        else if ( ( (*p1h++) == k ) )
        {
            *homn = k;
        }
        if ( *homn == k)
        {
            ++(*count);
            set_arr( i, 0, k, abitmap );
        }

        hm1  = old + ( i * mmparms.axsize );

        ++homo;
        ++homn;

        for ( j = 1; j < mmparms.axsize-1; ++j )
        {
            k = ((*homo) + 1) % mmparms.states;
            if ( ( (*m1h++) == k ) )
            {
                *homn = k;
                hp1++; hm1++; p1h++;
            }
            else if ( ( (*hp1++) == k ) )
            {
                *homn = k;
                hm1++; p1h++;
            }
            else if ( ( (*hm1++) == k ) )
            {
                *homn = k;
                p1h++;
            }
            else if ( ( (*p1h++) == k ) )
            {
                *homn = k;
            }
            if ( *homn == k)
            {
                ++(*count);
                set_arr( i, j, k, abitmap );
            }
            ++homo;
            ++homn;
        }
        hp1  = old + i * mmparms.axsize;
        k = ((*homo) + 1) % mmparms.states;
        if ( ( (*m1h++) == k ) )
        {
            *homn = k;
            hp1++; hm1++; p1h++;
        }
        else if ( ( (*hp1++) == k ) )
        {
            *homn = k;
            hm1++; p1h++;
        }
        else if ( ( (*hm1++) == k ) )
        {
            *homn = k;
            p1h++;
        }
        else if ( ( (*p1h++) == k ) )
        {
            *homn = k;
        }
        if ( *homn == k)
        {
            ++(*count);
            set_arr( i, j, k, abitmap );
        }

        ++homo;
        ++homn;
    }
    /* old[i][j] = new[i][j]; */
    memcpy ( old, new, mmparms.aysize*mmparms.axsize );
}
/* this is a routine to randomize the contents of my nyte arrays */
VOID randarr(char *old, char *new )
{
    int i, j;
    srand ( (unsigned int) time( (NULL) ) );
    for (i=0; i < mmparms.aysize; ++i)
    {
        for (j=0; j < mmparms.axsize; ++j)
        {
            *(old + ( i * mmparms.axsize) + j)  =
            *(new + ( i * mmparms.axsize) + j)  = (BYTE)rand()%mmparms.states;
        }
    }
}

