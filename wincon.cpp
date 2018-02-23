#ifndef __WINCON_CPP    /*  An extra safeguard to prevent this header from  */
#define __WINCON_CPP    /*  being included twice in the same source file    */

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <windows.h>
#include "wincon.h"

using namespace std;

POINT screensize;

#define BLINK 0

static int __BACKGROUND = 0;
static int __FOREGROUND = 7;

/*--------------------------------------------------------------------------------*/
// WINDOWS CONSOLE TEXT FUNCTIONS
/*--------------------------------------------------------------------------------*/

/****************************************************************************/
/* Function for Printing out to the screen
/****************************************************************************/
void setcolor (int fg, int bg) {

	int _attr;
 	color(fg);   // Set Foreground

  	if (fg > 15) _attr = 1;
 	else _attr = 0;
 	SetConsoleTextAttribute (GetStdHandle(STD_OUTPUT_HANDLE), _attr);

	bgcolor(bg); // Set Background
	return;
}

/****************************************************************************/
/* Clears the screen
/****************************************************************************/
void clrscr() {

	system("cls");
}

/****************************************************************************/
/* Sets Cursor Position via x,y, coords.
/****************************************************************************/ 
void gotoxy(int x, int y) {

	COORD point;
	if((x < 0 || x > screensize.x) || (y < 0 || y > screensize.y))
		return;
	point.X = (x -1); point.Y = (y -1);  // -1 compisates for Bad Windows API
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), point);
}

/****************************************************************************/
/* Delete Line
/****************************************************************************/ 
void delline() {

    COORD coord;
    DWORD written;
    CONSOLE_SCREEN_BUFFER_INFO info;

    GetConsoleScreenBufferInfo (GetStdHandle (STD_OUTPUT_HANDLE),
      &info);
    coord.X = info.dwCursorPosition.X;
    coord.Y = info.dwCursorPosition.Y;

    FillConsoleOutputCharacter (GetStdHandle (STD_OUTPUT_HANDLE),
      ' ', info.dwSize.X * info.dwCursorPosition.Y, coord, &written);
    gotoxy (info.dwCursorPosition.X + 1,
    info.dwCursorPosition.Y + 1);
}

/****************************************************************************/
/* Clear to End of Line
/****************************************************************************/ 
void clreol () {

    COORD coord;
    DWORD written;
    CONSOLE_SCREEN_BUFFER_INFO info;

    GetConsoleScreenBufferInfo (GetStdHandle (STD_OUTPUT_HANDLE),
      &info);
    coord.X = info.dwCursorPosition.X;
    coord.Y = info.dwCursorPosition.Y;

    FillConsoleOutputCharacter (GetStdHandle (STD_OUTPUT_HANDLE),
      ' ', info.dwSize.X - info.dwCursorPosition.X, coord, &written);
    gotoxy (coord.X, coord.Y);
}

/****************************************************************************/
/* Where X
/****************************************************************************/ 
int wherex() {

    CONSOLE_SCREEN_BUFFER_INFO info;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
    return info.dwCursorPosition.X + 1;
}

/****************************************************************************/
/* Where Y
/****************************************************************************/ 
int wherey() {

    CONSOLE_SCREEN_BUFFER_INFO info;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
    return info.dwCursorPosition.Y + 1;
}

/****************************************************************************/
/* Background Color
/****************************************************************************/ 
void bgcolor (int color) {

    __BACKGROUND = color;
    SetConsoleTextAttribute (GetStdHandle (STD_OUTPUT_HANDLE),
      __FOREGROUND + (color << 4));
}

/****************************************************************************/
/* Foreground Color
/****************************************************************************/ 
void color (int color) {

    __FOREGROUND = color;
    SetConsoleTextAttribute (GetStdHandle (STD_OUTPUT_HANDLE),
      color + (__BACKGROUND << 4));
}

/****************************************************************************/
/* Text Attribute
/****************************************************************************/ 
void textattr (int _attr) {

    SetConsoleTextAttribute (GetStdHandle(STD_OUTPUT_HANDLE), _attr);
}


#endif

