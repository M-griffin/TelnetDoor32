
/****************************************************************************
 * Door32 Telnet Client (c) 2003 Michael Griffin
 ****************************************************************************
 * By: Mercyful Fate (Michael Griffin)
 ****************************************************************************

 03/08/03 - Finished Working Door32 Input Output Telnet Client Pre-Beta 1
 		    Need to Fix Hardcoding of Outbound Connections
 		    Need to Update Door32.sys PATH to Command line Argument
 		    -- Keeps Crashing, maybe overflow ????

 03/09/03 - Changing from WinMain to Main no more Event Handler
 		    Using just plane threads for both sockets
 		    Seemed to have fixed the outbound connections by rewritting the send for local socket.
		    Beta 1 Release Completed! Still come char gooffs on the display.
		  - Fixing Comments up, and cleaning code
          - start of beta 2, changed buffer from 1024 to 20000 and no more ansi fuckups

 03/10/03 - Released beta 2 with display fixes. Starting on Beta 3 with Local Ansi Parser
 		    For Local Sysop display.
   			and ansi from the socket...
          - Not connecting to Linux BBSes due to missing RFC Protocol Implimentation

 03/11/03 - Working on Implimenting Telnet RFC Protocol for connections
 			TelentProtcol()  gets called after each RECV from Server!!
 			This will Parse for RFC Codes, then Send to both Client_Socket & Local Parser
          - RFC is working, small bug where first and 3rd char not getting recv. :(
            then flushed back to the user on second conenctions and more out...

 03/12/03 - Finished new SysOp Thread for receiving input from the console, also
 			finished Beta 3 with early local ansi display, most only works telnetting out
 			still quite a few artifacts that i need to filter.  Also missing
 			inital character input from 1 and 3rd pressed key. gota figure that bitch out.
 		  - Started on Beta 4, Looking for fix for crappy char input in first and 3rd key input
 		  	Note, only fucks up 1st,3rd, client Input after 1st telnet out!!
		  - Bug when Users logsoff Manual from Telnet, error!! ?? Exception error.. Doh!!
    	  - Fixed Hangup Bug! Removed CheckLine, if INVALID SOCKET will exit
          - Release Beta 4

 03/13/03 - Seperate Outbound Telnet code into tnmain.cpp/h to organize a bit more.
 		    Starting work on Beta 5, And local sysop display
          - Updated Sysop Display with Node and Connect TO Info.

 03/15/03 - Restarted from Pre-Beta 5, beta 5 has messed up echo input and it was messing
 		    with linux on the second call out bug.  Going to work on an internal Telnet
 		    Menu so user doesn't have to enter and exit program.. might fix all the bugs!
 		  - Release Beta 6
 		  
 03/16/03 - Reformatted and Cleaned Code up a bit.  Also test program with Mystic 2.0 and
 			Doesn't have the 2nd outconnection bugs that 1.07.03 has.  Not sure what to 
 			do about that.



 */

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>		// atoi() - system()
#include <process.h>	// Threads
#include <winsock.h>
#include <fstream>		// Log stream

#include "ansi.h" 		// Local Side Ansi Parser
#include "wincon.h"		// Windows console i/o
#include "tnmain.h"		// Telnet Functions

/****************************************************************************/
/* Function Globals
/****************************************************************************/
char        	DOOR32P[255];						// CMDLine d32 path
char 			HOST[255];							// CMDLine hostname
int				PORT = 23;							// CMDLine port to connect to
int 			ECHO = 0;                           // CMDLine echo input

char 			sendbuff[20000];					// Main Buffer for Recv on Telnet Socket
char 			codebuf[256];
unsigned char 	codeptr;
ofstream 		log;		     			    	// Setup Log file for logging errors
SOCKET			client_socket = INVALID_SOCKET; 	// Handle to Socket from door32.sys
bool 			DISPLAYON = false;  				// Toggle for Local Display on/off
DOOR32 			door32;			 					// Handle to Door32.sys data



/****************************************************************************/
/* Truncates white-space chars off end of 'str' and terminates at first tab */
/****************************************************************************/
void truncsp(char *str) {

	#define SP      0x20
	char c;

	str[strcspn(str,"\t")]=0;
	c=strlen(str);
	while(c && (char)str[c-1] <= SP) c--;
	str[c]=0;
	return;
}

/****************************************************************************/
/* Main Sysop Screen Displaying Program Info
/****************************************************************************/

void displaySysop() {

	// Ansi Screen For Sysop Commands
	char szBuffer[] = {
		"[?7h[40m[2J[2;2H[0mú[C[1mÜ[47mÜ²[0mßß[1mß[C[0;36mÜ[C²Ý[C°[C[37mú[5C[1mT[36me[0;36mlnet[C[1;37mD[36mo[0;36mor32[C[1m[[37mB[36me[0;36mta[C[1;37m6[36m][C[37m.[36mo[0;36m3[1;37m.[36m15[37m.[36mo[0;36m3[1;37m.[5C[0mú[C[36m°[CÞ²[CÜ[C[1;37mß[0mßß[1;47m²Ü[40mÜ[C[0mú[3;3H[1mÞß[C[0;36mÜÜÛ[1;46mÜ[0;36mÛ[1;46m²²Ü²°° [0;36mÜÜ[1m°[C[0mú[5C[36m([1mc[0;36m)[C[1;37mM[36me[0;36mrcyful[C[1;37mF[36ma[0;36mte[C[1;37m.[0;36m2[1moo[0;36m3[1;37m.[4Cú[C[0mú[C[1;36m°[0;36mÜÜ[46m [1m°°²Ü²²[0;36mÛ[1;46mÜ[0;36mÛÜÜ[C[1;37mßÞ[4;4H[36mÜ[46m²²ÜÜ²Ü²[40m±[46mÛÛÛÛÜÝ[40m±±[0;36mÜ[1m°[35C°[0;36mÜ[1m±±[46mÝÜÛÛÛÛ[40m±[46m²Ü²ÜÜ²²[40mÜ[5;3HÛÛ[46mÛÛÛ[40mßßß[46mÛÛ[40mßß[46m²²Û²°  [19C[0m$[C[1mú[15C[46m  [36m°²Û²²[40mßß[46mÛÛ[40mßßß[46mÛÛÛ[40mÛÛ[6;2HÛÞ[46m²[40mßÝ[C[0mÜÝ[1mÜ[2Cú[2C[0;36m²[1m±[46mÛÛÜ[40m±[0;36mÝ[C[30;44m [1;32mDisplay Still in Early Testing! [C[0;36mÞ[1m±[46mÜÛÛ[40m±[0;36m²[2C[1;37mú[2CÜ[0mÝÜ[C[1;36mÝß[46m²[40mÞÛ[7;1H[46m²[40mÞß[0mÞÜ[1;36mß[0mÞ[1m±[47m²Û[40mÝ[0mú[C$[2C[1mú[0;36mß[1m±[0;36mßß[4C[1;37mú[32C[0;36mßß[1m±[0;36mß[1;37mú[2C[0m$[Cú[1mÝ[47mÛ²[40m±[0mÞ[1;36mß[0mÜÞ[1;36mßÞ[46m²[8;1H[0mú[CÞ[1;47m²°[40mÛ±[0mÛ[1;47mÞÛ[40mÛ[8C.[0mÄÄÄÄÄÄÄÄÄÄÄÄÄ[1m´[CS[0my[1;30msOp[C[37mP[0ma[1;30mnel[C[37mÃ[0mÄÄÄÄÄÄÄÄÄÄÄ.[8C[1mÛ[47mÛÞ[0mÛ[1m±Û[47m°²[30;40mÝ[C[0mú[9;4H[47m [1mÜÜÝ[0mÛ[1;47m°°[0mÛ[4C[1mð[0mð[1;30mð[C[37m³[39C[0m³[C[1;30mð[0mð[1mð[4C[0mÛ[1;47m°°[0mÛ[1;47mÝÜÜ [10;4HÛÝßßÜÜ[0mÛÛ[8C[1m³[2Cð[0mð[1;34;44m [0;30;44m.[1;34mx[0;30;44m.[1;34m [37;40mð[0mð[2C[1;34mToggle[CDisplay[COn/Off[5C[0m³[5Cú[2CÛÛ[1;47mÜÜßßÝÛ[11;3H[40mÜ[47m²ÛÝßßß²[40mÜ[4Cð[0mð[1;30mð[C[37m³[2Cð[0mð[1;34;44m [0;30;44m.[1;34mh[0;30;44m.[1;34m [37;40mð[0mð[2C[1;34mHangup[C/[CDisconnect[CUser[2C[0m³[C[1;30mð[0mð[1mð[4CÜ[47m²ÛÝßßß²[40mÜ[12;3HÞ[47mÛÝ[0mÛ[1;47m°°[0mÛ[47m [1mÞ[8C[40m³[39C[0m³[8C[1mÞ[47mÛÝ[0mÛ[1;47m°°[0mÛ[47m [1;40mÝ[13;3HÞ[47mßÛÜ[0mÛÛ[1;47mÜÜÛ[8C[40m`ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ[0m'[8C[1mÞ[47mßÛÜ[0mÛÛ[1;47mÜÜ[40mÝ[14;3H°[47m° Þ²ßßÝ[0mÛ[8C[1m.[0mÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ.[8C[1m°[47m° Þ²ßßÝ[40m°[15;4H[47m ÜÜÝ[0mÛ[1;47m°°[0mÛ[4C[1mð[0mð[1;30mð[C[37m³[39C[0m³[C[1;30mð[0mð[1mð[5C[0;47m [1mÜÜÝ[0mÛ[1;47m°°[16;4HÛÝßßÜÜ[0mÛÛ[7Cú[1m³[39C[0m³[2Cú[6C[1;47mÛÝßßÜÜ[0mÛ[17;3H°[47m  [1m°°[0mÛ[1;47mß²Û[4C[40mð[0mð[1;30mð[C[37m³[39C[0m³[C[1;30mð[0mð[1mð[4C[0m°[47m  [1m°°[0mÛ[1;47mß²[0m°[18;4H[1m±±[47m [0mÛ[47m [1mÞÛ[0mÛ[8C[1m³[39C[0m³[9C[1m±±[47m [0mÛ[47m [1mÞÛ[19;3H[0m°[47m  [1m°°[0mÛ[1;47mß²Û[8C[40m`ÄÄÄÄÄÄÄÄÄÄ´[CU[0ms[1;30mer[C[37mI[0mn[1;30mformation[C[37mÃÄÄÄÄÄÄÄÄÄ[0m'[8C°[47m  [1m°°[0mÛ[1;47mß²[0m°[20;1H[1;36mÜÜÜ[C[37mß[47mÝ²Û[0mÛ[1;47m°[40mÝ[30m$[3C[0;36mÜ[1;46m°° [0;36mÜ[1m±[14C[37mú[14C[0m$[C[1mú[5C[36m±[0;36mÜ[46m [1m°°[0;36mÜ[3C[1;30m$[37mÞ[47m°[0mÛ[1;47mÛ²Ý[40mß[C[36mÜÜÜ[21;2H[46m²[40m±[46m²[40mÜÝ[C[0mÜÝ[1mÜ[3Cú[0;36mÞ[1m±[46mÛÛÛÜ [4C[30;40m$[7C[0m$[C[1mú[6Cú[2Cú[9Cú[2C[46m [36mÜÛÛÛ[40m±[0;36mÝ[1;37mú[3CÜ[0mÝÜ[C[1;36mÝÜ[46m²[40m±[46m²[22;3H[40mÛ[46m²°ÛÛ[40mÜÜÜ[46mÛÛ[40mÜÜ[46m²²Û²°  [40m°[2C[37mú[7Cú[2Cú[4C[0m$[C[1mú[8Cú[5C[36m°[46m  °²Û²²[40mÜÜ[46mÛÛ[40mÜÜÜ[46mÛÛ°²[40mÛ[23;4HÞ[46m²[40mß±[46mÛ[40mßßß±[46mÛÛÛ[40m±[46mÜÝ[40m±±[0;36mß[2C[1;30m$[6C[0m$[C[1mú[12Cú[2Cú[2Cú[6C[0;36mß[1m±±[46mÝÜ[40m±[46mÛÛÛ[40m±ßßß[46mÛ[40m±ß[46m²[40mÞ[0m" };

	ansiparse(szBuffer);

	// Displays Connected User Information
	gotoxy(23,15);
	color (9);
	printf ("Node      ");
 	color (11);
  	printf ( ":" );
   	color (15);
    printf ( " %i ", door32.user_node);

	gotoxy(23,16);
	color (9);
	printf ("Name      ");
 	color (11);
  	printf ( ":" );
   	color (15);
    printf ( " %s ",door32.user_name);

	gotoxy(23,17);
	color (9);
	printf ("Handle    ");
 	color (11);
  	printf ( ":" );
   	color (15);
    printf ( " %s ",door32.user_handle);

    gotoxy(23,18);
	color (9);
	printf ("Connected ");
 	color (11);
  	printf ( ":" );
   	color (15);
    printf ( " %s : %i ",HOST, PORT);

}

/****************************************************************************/
/* Handles all Keyboard Input from Sysop Console
/****************************************************************************/
void sysopThread(void *p ) {

	char c;
	system("cls");
	displaySysop();

	for (;;) {
		c = getche();
		// Erase all chars from screen inputted
		printf( "\b" );
		printf( " "  );
		printf( "\b" );
		switch (c) {

			case 'x': // Toggle Display
				if (DISPLAYON == false) {
    				DISPLAYON = true;
    				system("cls");
				}
				else {
    				DISPLAYON = false;
					system("cls");
					displaySysop();
				}
				break;

			case 'X': // Toggle Display
				if (DISPLAYON == false) {
    				DISPLAYON = true;
    				system("cls");
				}
				else {
    				DISPLAYON = false;
					system("cls");
					displaySysop();
				}
				break;

			case 'h': // Hangup User
				log.close();
				log << "Exit From Sysop at Console!";
				exit(0);
				break;

			case 'H': // Hangup User
				log.close();
				log << "Exit From Sysop at Console!";
				exit(0);
				break;

			default:
				break;
		}
	}
}

/****************************************************************************/
/* Gets all data from door32.sys and returns the filled structure			*/
/****************************************************************************/
void door() {

  	int cnt = 0;
	// Setup Door32 Path from Command Line
 	cnt = strlen(DOOR32P);
	DOOR32P[cnt] = 'd'; ++cnt;
	DOOR32P[cnt] = 'o'; ++cnt;
	DOOR32P[cnt] = 'o'; ++cnt;
	DOOR32P[cnt] = 'r'; ++cnt;
	DOOR32P[cnt] = '3'; ++cnt;
	DOOR32P[cnt] = '2'; ++cnt;
	DOOR32P[cnt] = '.'; ++cnt;
	DOOR32P[cnt] = 's'; ++cnt;
	DOOR32P[cnt] = 'y'; ++cnt;
	DOOR32P[cnt] = 's'; ++cnt;

 	// Open door32.sys file in bbs Directory
 	FILE *stream;
	if(( stream = fopen( DOOR32P, "rt" )) == NULL ) {
		printf( "Can't open %s : \r\n\n", DOOR32P );
		Sleep(4000);
		return; // exit
	}

	// Load each Type into Structure from File
	char * str = new char [256];

	memset(&door32,0,sizeof(DOOR32));
	ZeroMemory(str, 256);
	fgets(str,81,stream);
	truncsp(str);
	door32.com_type = atoi(str);

	ZeroMemory(str, 256);
	fgets(str,81,stream);
	door32.com_port = atoi(str);
	client_socket   = atoi(str);

	ZeroMemory(str, 256);
	fgets(str,81,stream);
	door32.com_baud = atoi(str);

	ZeroMemory(str, 256);
	fgets(str,81,stream);
	truncsp(str);
	strcpy(door32.bbs_name,(str));

	ZeroMemory(str, 256);
	fgets(str,81,stream);
	door32.user_idx = atoi(str);

	ZeroMemory(str, 256);
	fgets(str,81,stream);
	truncsp(str);
	strcpy(door32.user_name,(str));

	ZeroMemory(str, 256);
	fgets(str,81,stream);
	truncsp(str);
	strcpy(door32.user_handle,(str));

	ZeroMemory(str, 256);
	fgets(str,81,stream);
	door32.user_sl = atoi(str);

	ZeroMemory(str, 256);
	fgets(str,81,stream);
	door32.user_ml = atoi(str);

	ZeroMemory(str, 256);
	fgets(str,81,stream);
	door32.user_emu = atoi(str);

	ZeroMemory(str, 256);
	fgets(str,81,stream);
	door32.user_node = atoi(str);

	delete [] str;
	fclose(stream);

	return;
}

/****************************************************************************/
/* Receive Key Input from User / BBS Socket, and Send Out to HOST Socket
/****************************************************************************/
void receiveTerm( void *p ) {

 	SOCKET sock = (SOCKET)p;
	char szBuffer[256];

	int i = 0;

	for (;;) {
		ZeroMemory(szBuffer, 256);
		i = recv(client_socket, szBuffer, 256, NO_FLAGS_SET);
		if ( i == SOCKET_ERROR ) { log << "Recv Client_Socket Error!" << endl; log.close(); exit(1); }

		if (ECHO == 1) { // If Server Doesn't Echo's Input - Not Enabled now!
			i = send(client_socket, szBuffer, strlen(szBuffer), NO_FLAGS_SET);
			if ( i == SOCKET_ERROR ) { log << "Echo Input Error Sending!" << endl; log.close(); exit(1); }
		}

		i = send(sock, szBuffer, strlen(szBuffer), NO_FLAGS_SET);
		if ( i == SOCKET_ERROR ) { log << "Send Sock Error Sending!" << endl; log.close(); exit(1); }
	}
}



/****************************************************************************/
/* Outbound Telnet Socket Container
/****************************************************************************/

struct MAIN
{
	HINSTANCE hInst;
	HWND hWnd;

	SOCKET Sock;
	sockaddr_in LocalSin, RemoteSin;
} Main;


/****************************************************************************/
/* Initalize Door32 & Outbound Telnet Socket.  The Connect Out to Host
/****************************************************************************/
int InitSocket(int nPort) {

	char * szBuffer = new char [1024];
	ZeroMemory(szBuffer, 1024);

	WSAData wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);  // Startup Winsock

	Main.LocalSin.sin_family = AF_INET;
	Main.LocalSin.sin_port = NO_FLAGS_SET;

	Main.RemoteSin.sin_family = AF_INET;
	Main.RemoteSin.sin_port = htons(nPort);

	hostent* pHostEnt = gethostbyname(HOST);
	if (pHostEnt) { // First lookup by name, then host if that fails
		if ((Main.RemoteSin.sin_addr.S_un.S_addr = *(DWORD*)pHostEnt->h_addr) == INADDR_NONE) {
			log << "BY NAME INADDR_NONE" << endl;
		}
	}
	else if((Main.RemoteSin.sin_addr.S_un.S_addr = inet_addr(HOST)) == INADDR_NONE) {
		log << "BY ADDRESS INADDR_NONE" << endl;
		return false;		// Error setting IP
	}

	if ((Main.Sock = socket(PF_INET, SOCK_STREAM, NO_FLAGS_SET)) == SOCKET_ERROR) {
		log << "SET SOCKET_ERROR" << endl;
		return false;		// Error creating socket
	}

	if (bind(Main.Sock, (sockaddr *)&Main.LocalSin, sizeof(sockaddr_in)) == SOCKET_ERROR) {
		log << "BIND SOCKET_ERROR" << endl;
		return false;		// Error binding socket
	}

	// Display Connecting Message to User
  	char* szBuffer2 = new char[2048];
 	szBuffer2 = "[?7h[40m[2J[11;20H[0;1m.[0mÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ.[12;16H[1mð[0mð[1;30mð[C[37m³[39C[0m³[C[1;30mð[0mð[1mð[13;19H[0mú[1m³[39C[0m³[2Cú[14;16H[1mð[0mð[1;30mð[C[37m³[6CC[35mon[0;35mnecting[C[1;37mT[35mo[C[37mS[35mys[0;35mtem[C[1;37mN[35mo[0;35mw[C[1;37m.[C.[C.[3C[0m³[C[1;30mð[0mð[1mð[15;20H³[39C[0m³[16;20H[1m`ÄÄÄÄÄÄÄÄÄÄ´[CC[34monnection[C[37mS[34mtats[C[37mÃÄÄÄÄÄÄÄÄÄ[0m'[0m";
	ansiparse(szBuffer2);
 	send(client_socket, szBuffer2, strlen(szBuffer2), NO_FLAGS_SET);

	if (connect(Main.Sock, (sockaddr *)&Main.RemoteSin, sizeof(sockaddr_in)) == SOCKET_ERROR) {
		log << "CONNECT SOCKET_ERROR" << endl;
		// Display Connection Failed! Message to User
		ZeroMemory(szBuffer, 1024);
		char* szBuffer4 = new char [2048];
  		szBuffer4 = "[?7h[40m[2J[11;20H[0;1m.[0mÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ.[12;16H[1mð[0mð[1;30mð[C[37m³[39C[0m³[C[1;30mð[0mð[1mð[13;19H[0mú[1m³[39C[0m³[2Cú[14;16H[1mð[0mð[1;30mð[C[37m³[10C[30m[[37mC[31mon[0;31mnection[C[1;37mF[31mai[0;31mled[1;37m![30m][9C[0m³[C[1;30mð[0mð[1mð[15;20H³[39C[0m³[16;20H[1m`ÄÄÄÄÄÄÄÄÄÄ´[CC[34monnection[C[37mS[34mtats[C[37mÃÄÄÄÄÄÄÄÄÄ[0m'[0m";
		send(client_socket, szBuffer4, strlen(szBuffer4), NO_FLAGS_SET);
		ansiparse(szBuffer4);
		delete [] szBuffer4;
		Sleep(3000);
		return false;
	}

	// Display Connection Successfull Message
	char* szBuffer3 = new char [2048];
 	szBuffer3 = "[?7h[40m[2J[11;20H[0;1m.[0mÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ.[12;16H[1mð[0mð[1;30mð[C[37m³[39C[0m³[C[1;30mð[0mð[1mð[13;19H[0mú[1m³[39C[0m³[2Cú[14;16H[1mð[0mð[1;30mð[C[37m³[8C[30m[[37mC[32mon[0;32mnection[C[1;37mE[32mst[0;32mablished[1;30m][7C[0m³[C[1;30mð[0mð[1mð[15;20H³[39C[0m³[16;20H[1m`ÄÄÄÄÄÄÄÄÄÄ´[CC[34monnection[C[37mS[34mtats[C[37mÃÄÄÄÄÄÄÄÄÄ[0m'[19;80H[1m [0m";
	ansiparse(szBuffer3);
 	send(client_socket, szBuffer3, strlen(szBuffer3), NO_FLAGS_SET);

	Sleep(3000);

	delete [] szBuffer;
	delete [] szBuffer2;
	delete [] szBuffer3;

    WSAAsyncSelect(Main.Sock, Main.hWnd, WM_USER + 2, FD_READ | FD_CLOSE | FD_WRITE | FD_CONNECT | FD_OOB);

    // Start Main Program Threads here
 	HANDLE ahThread[3];
    ahThread[0] = (HANDLE)_beginthread( receiveTerm,   NO_FLAGS_SET, (void *)Main.Sock ); // Waits for Input from User
    ahThread[1] = (HANDLE)_beginthread( receiveTelnet, NO_FLAGS_SET, (void *)Main.Sock ); // Polls for Incomming Telnet Data
    ahThread[2] = (HANDLE)_beginthread( sysopThread,   NO_FLAGS_SET, (void *)Main.Sock ); // Sysop Input at Console
    WaitForMultipleObjects( 3, ahThread, FALSE, INFINITE );
    return TRUE;
}

/****************************************************************************/
/* Main Program Entrance
/****************************************************************************/
int main (int argc, char *argv[]) {

	SetConsoleTitle("Telnet Door32 .Beta 6. (c) 2003 Mercyful Fate");

	// Not Ready to support Local Echo Input
 	//if (argc != 5) {
 	if (argc != 4) {
 		printf ("Invalid CMD Line Parameters - Registering %i out of 3! \n\n", argc-1);
 		log.open("Td32-Error.log");
 		log << "Invalid CMD Line Parameters - Registering " << argc-1 << " out of 3! " << endl;
 		log << "Make sure to Read the Doc's!" << endl;
 		log.close();
 		Sleep(4000);
 		exit(1);
 	}

	// Set Global Parameters
	strcpy (DOOR32P,argv[1]);
	strcpy (HOST,argv[2]);
	PORT = atoi(argv[3]);
	//ECHO = atoi(argv[4]);
	ECHO = 0;

	door(); // Call Function to Read in door32.sys values

	char logp[20];
	sprintf(logp,"Td32-Node%i.log",door32.user_node); // Start Logging
	log.open(logp);
	log << "cmd line parameters : " << DOOR32P << " " << HOST << " " << PORT << " " << ECHO << endl;

	// Start Door32 And Telnet Socket Startup, and Telnet Outbound Connection here
	if (!InitSocket(PORT))	{

		char * szBuffer = new char [100];
		sprintf(szBuffer, "Error connecting [%d]", WSAGetLastError());
		log << szBuffer << endl;
		delete [] szBuffer;
	}
    log << "Finished" << endl;
    log.close();

}




