/*---------------------------------------------
   MINISUBS.C -- Cellular Automata Sample Program
  --------------------------------------------*/
#define INCL_WIN
#define INCL_GPI
#define INCL_DOS

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include "chain.h"
extern BOOL fControlsHidden;
extern HWND hwndTitleBar , hwndSysMenu , hwndMinMax , hwndMenu ;

VOID EnableMenuItem( HWND hwnd, SHORT sMenuItem, BOOL fEnable )
{
    HWND hwndParent = WinQueryWindow ( hwnd, QW_PARENT );
    HWND hwndMenu = WinWindowFromID ( hwndParent, FID_MENU );
    WinSendMsg ( hwndMenu, MM_SETITEMATTR,
        MPFROM2SHORT ( sMenuItem, TRUE ),
        MPFROM2SHORT ( MIA_DISABLED, fEnable ? 0 : MIA_DISABLED) );
}


/*
    ClkHideFrameControls() -- Hide the title bar and associated controls
*/
VOID ClkHideFrameControls ( HWND hwndFrame )
{

    hwndTitleBar = WinWindowFromID ( hwndFrame , FID_TITLEBAR ) ;
    hwndSysMenu = WinWindowFromID ( hwndFrame , FID_SYSMENU ) ;
    hwndMinMax = WinWindowFromID ( hwndFrame , FID_MINMAX ) ;
    hwndMenu = WinWindowFromID ( hwndFrame , FID_MENU ) ;

    WinSetParent ( hwndTitleBar , HWND_OBJECT , FALSE ) ;
    WinSetParent ( hwndSysMenu , HWND_OBJECT , FALSE ) ;
    WinSetParent ( hwndMinMax , HWND_OBJECT , FALSE ) ;
    WinSetParent ( hwndMenu , HWND_OBJECT , FALSE ) ;

    WinSendMsg ( hwndFrame , WM_UPDATEFRAME ,
	( MPARAM ) ( FCF_TITLEBAR | FCF_SYSMENU | FCF_MINMAX | FCF_MENU ) ,
	NULL ) ;

    fControlsHidden = TRUE ;
}


/*
    ClkShowFrameControls() -- Show the title bar and associated controls
*/
VOID ClkShowFrameControls ( HWND hwndFrame )
{

    WinSetParent ( hwndTitleBar , hwndFrame , FALSE ) ;
    WinSetParent ( hwndSysMenu , hwndFrame , FALSE ) ;
    WinSetParent ( hwndMinMax , hwndFrame , FALSE ) ;
    WinSetParent ( hwndMenu , hwndFrame , FALSE ) ;

    WinSendMsg ( hwndFrame , WM_UPDATEFRAME ,
	( MPARAM ) ( FCF_TITLEBAR | FCF_SYSMENU | FCF_MINMAX | FCF_MENU ) ,
	NULL ) ;
    WinInvalidateRect ( hwndFrame , NULL , TRUE ) ;

    fControlsHidden = FALSE ;
}
