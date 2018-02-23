#ifndef __WINCON_H    /*  An extra safeguard to prevent this header from  */
#define __WINCON_H    /*  being included twice in the same source file    */

/*--------------------------------------------------------------------------------*/
// Function for Printing out to the screen
void setcolor (int fg, int bg);

/*--------------------------------------------------------------------------------*/
// Clears Screen
void clrscr();

/*--------------------------------------------------------------------------------*/
// Sets Cursor Position via x,y, coords.
void gotoxy(int x, int y);

/*--------------------------------------------------------------------------------*/
// Delete Line
void delline();

/*--------------------------------------------------------------------------------*/
// Clear to End of Line
void clreol ();

/*--------------------------------------------------------------------------------*/
// Gets X coord on screen
int wherex();

/*--------------------------------------------------------------------------------*/
// Gets Y coord on screen
int wherey();

/*--------------------------------------------------------------------------------*/
// Sets Background Color
void bgcolor (int color);

/*--------------------------------------------------------------------------------*/
// Sets Foreground Color
void color (int color);

/*--------------------------------------------------------------------------------*/
// Sets Color Attribute
void textattr (int _attr);

#ifdef __cplusplus
extern "C" {
#endif

char*	_cgets (char*);
int	_cprintf (const char*, ...);
int	_cputs (const char*);
int	_cscanf (char*, ...);

int	_getch (void);
int	_getche (void);
int	_kbhit (void);
int	_putch (int);
int	_ungetch (int);


int	getch (void);
int	getche (void);
int	kbhit (void);
int	putch (int);
int	ungetch (int);

#ifdef __cplusplus
}
#endif

#endif
