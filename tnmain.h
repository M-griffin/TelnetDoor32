#ifndef __TNMAIN_H    /*  An extra safeguard to prevent this header from  */
#define __TNMAIN_H    /*  being included twice in the same source file    */

#include <winsock.h>
#include <fstream>

/****************************************************************************/
/* Function Globals
/****************************************************************************/
extern char        		DOOR32P[255];
extern char 			HOST[255];
extern int				PORT;
extern int              ECHO;
extern ofstream 		log;
extern SOCKET			client_socket;
extern bool 			DISPLAYON;


/****************************************************************************/
/* Structure for holding DOOR32.SYS Data for the Program
/****************************************************************************/
typedef struct {

	unsigned short int 	com_type;
	unsigned       int 	com_port;
	unsigned long  int 	com_baud;
	char 				bbs_name[30];
	unsigned long  int 	user_idx;
	char 				user_name[30];
	char 				user_handle[30];
	unsigned short int 	user_sl;
	unsigned short int 	user_ml;
	unsigned short int 	user_emu;
	unsigned short int 	user_node;
} DOOR32;

extern DOOR32 door32;


/****************************************************************************/
/*
/****************************************************************************/
enum _verb {

	verb_sb   = 250,
	verb_will = 251,
	verb_wont = 252,
	verb_do   = 253,
	verb_dont = 254
};

/****************************************************************************/
/*
/****************************************************************************/
enum _state {

	state_data,   //we expect a data byte
	state_code,   //we expect a code
  	state_option  //we expect an option
};

/****************************************************************************/
/*
/****************************************************************************/
enum _option {

	TOPT_BIN	= 0,   // Binary Transmission
	TOPT_ECHO = 1,  // Echo
	TOPT_RECN = 2,  // Reconnection
	TOPT_SUPP = 3,  // Suppress Go Ahead
	TOPT_APRX = 4,  // Approx Message Size Negotiation
	TOPT_STAT = 5,  // Status
	TOPT_TIM	= 6,   // Timing Mark
	TOPT_REM 	= 7,   // Remote Controlled Trans and Echo
	TOPT_OLW 	= 8,   // Output Line Width
	TOPT_OPS 	= 9,   // Output Page Size
	TOPT_OCRD = 10, // Output Carriage-Return Disposition
	TOPT_OHT 	= 11,  // Output Horizontal Tabstops
	TOPT_OHTD = 12, // Output Horizontal Tab Disposition
	TOPT_OFD 	= 13,  // Output Formfeed Disposition
	TOPT_OVT 	= 14,  // Output Vertical Tabstops
	TOPT_OVTD = 15, // Output Vertical Tab Disposition
	TOPT_OLD 	= 16,  // Output Linefeed Disposition
	TOPT_EXT 	= 17,  // Extended ASCII
	TOPT_LOGO = 18, // Logout
	TOPT_BYTE = 19, // Byte Macro
	TOPT_DATA = 20, // Data Entry Terminal
	TOPT_SUP 	= 21,  // SUPDUP
	TOPT_SUPO = 22, // SUPDUP Output
	TOPT_SNDL = 23, // Send Location
	TOPT_TERM = 24, // Terminal Type
	TOPT_EOR 	= 25,  // End of Record
	TOPT_TACACS = 26, // TACACS User Identification
	TOPT_OM 	= 27,   // Output Marking
	TOPT_TLN 	= 28,  // Terminal Location Number
	TOPT_3270 = 29, // Telnet 3270 Regime
	TOPT_X3 	= 30,  // X.3 PAD
	TOPT_NAWS = 31, // Negotiate About Window Size
	TOPT_TS 	= 32,   // Terminal Speed
	TOPT_RFC 	= 33,  // Remote Flow Control
	TOPT_LINE = 34, // Linemode
	TOPT_XDL 	= 35,  // X Display Location
	TOPT_ENVIR = 36,// Telnet Environment Option
	TOPT_AUTH = 37, // Telnet Authentication Option
	TOPT_NENVIR = 39,// Telnet Environment Option
	TOPT_EXTOP = 255, // Extended-Options-List
	TOPT_ERROR = 256  // Magic number
};


/****************************************************************************/
/*
/****************************************************************************/
#define NO_FLAGS_SET         0

#define NUL     0
#define BEL     7
#define BS      8
#define HT      9
#define LF     10
#define VT     11
#define FF     12
#define CR     13
#define SE    240
#define NOP   241
#define DM    242
#define BRK   243
#define IP    244
#define AO    245
#define AYT   246
#define EC    247
#define EL    248
#define GA    249
#define SB    250
#define WILL  251
#define WONT  252
#define DO    253
#define DONT  254
#define IAC   255

extern HANDLE stdin1;
extern HANDLE stdout1;
extern HANDLE stderr1;

void nvt(SOCKET server,unsigned char data);

extern char sendbuff[20000];
extern char codebuf[256];
extern unsigned char codeptr;

#define NUM_CODEC 6
typedef void (*LPCODEPROC)(char*);

void ansi_set_screen_attribute(char* buffer);
void ansi_set_position(char* buffer);
void ansi_erase_screen(char* buffer);
void ansi_move_up(char* buffer);
void ansi_erase_line(char* buffer);

void ansi(SOCKET server,unsigned char data);


void ddww_error(SOCKET server,_verb verb,_option option);
void ddww_echo(SOCKET server,_verb verb, _option option);
void ddww_supp(SOCKET server,_verb verb,_option option); //Suppress GA
void ddww_term(SOCKET server,_verb verb,_option option); //Subnegotiate terminal type
void sbproc_term(SOCKET server,unsigned char data);

int option_error(_verb,_option,int,SOCKET);

typedef void(*LPOPTIONPROC)(SOCKET,_verb,_option);
typedef void(*LPDATAPROC)(SOCKET,unsigned char data);

void receiveTelnet( void *p );

/****************************************************************************/
/*
/****************************************************************************/
inline void yesreply(SOCKET server, _verb verb,_option option) {

	unsigned char buf[3];
	buf[0] = IAC;
	buf[1] = (verb==verb_do)?WILL:(verb==verb_dont)?WONT:(verb==verb_will)?DO:DONT;
	buf[2] = (unsigned char)option;
	send(server,(char*)buf,3,0);
}

/****************************************************************************/
/*
/****************************************************************************/
inline void noreply(SOCKET server, _verb verb,_option option) {

	unsigned char buf[3];
	buf[0] = IAC;
	buf[1] = (verb==verb_do)?WONT:(verb==verb_dont)?WILL:(verb==verb_will)?DONT:DO;
	buf[2] = (unsigned char)option;
	send(server,(char*)buf,3,0);
}

/****************************************************************************/
/*
/****************************************************************************/
inline void askfor(SOCKET server, _verb verb,_option option) {

	unsigned char buf[3];
	buf[0] = IAC;
	buf[1] = (unsigned char)verb;
	buf[2] = (unsigned char)option;
	send(server,(char*)buf,3,0);
}


#endif
