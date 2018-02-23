#ifndef __TNMAIN_CPP    /*  An extra safeguard to prevent this header from  */
#define __TNMAIN_CPP    /*  being included twice in the same source file    */

#include "tnmain.h"
#include "ansi.h"


/****************************************************************************/
/* Begining of RFC Telnet Interface
/****************************************************************************/
HANDLE stdin1;
HANDLE stdout1;
HANDLE stderr1;

/****************************************************************************/
/*
/****************************************************************************/
void ddww_error(SOCKET server,_verb verb,_option option) {

	switch(verb) {

  		case verb_will:
  			log << "verb_will (ddww_error) v/" << verb << " o/" << option << endl;
    		noreply(server,verb,option);
    		break;
		case verb_wont:
  			log << "verb_wont (ddww_error) v/" << verb << " o/" << option << endl;
    		return;
  		case verb_do:
  			log << "verb_do (ddww_error) v/" << verb << " o/" << option << endl;
    		noreply(server,verb,option);
    		break;
  		case verb_dont:
  			log << "verb_dont (ddww_error) v/" << verb << " o/" << option << endl;
    		return;
  	}
}

/****************************************************************************/
/*
/****************************************************************************/
void ddww_echo(SOCKET server,_verb verb, _option option) {

	DWORD mode;
	GetConsoleMode(stdin1,&mode); // ENABLE_ECHO_INPUT

	int set = !(mode & ENABLE_ECHO_INPUT);
	log << "set int " << set << endl;

	switch(verb) {
		case verb_will: // server wants to echo stuff
			//don't confirm - already set.
    		if(set) { log << "verb_dont set (ddww_echo) v/" << verb << " o/" << option << endl; return; }
    		log << "verb_dont (ddww_echo) v/" << verb << " o/" << option << endl;
    		SetConsoleMode(stdin1,mode & (~ENABLE_ECHO_INPUT));
    		break;
		case verb_wont: // server don't want to echo
			//don't confirm - already unset.
			if(!set) { log << "verb_wont !set (ddww_echo) v/" << verb << " o/" << option << endl; return; }
			log << "verb_wont (ddww_echo) v/" << verb << " o/" << option << endl;
			SetConsoleMode(stdin1,mode | ENABLE_ECHO_INPUT);
			break;
		case verb_do:   // server wants me to loopback
			log << "verb_do (ddww_echo) v/" << verb << " o/" << option << endl;
			noreply(server,verb,option);
			return;
		case verb_dont: // server doesn't want me to echo
			log << "verb_dont (ddww_echo) v/" << verb << " o/" << option << endl;
			break;        // don't bother to reply - I don't
	}
	log << "yesreply (ddww_echo) v/" << verb << " o/" << option << endl;
	yesreply(server,verb,option);
}


/****************************************************************************/
/*
/****************************************************************************/
void ddww_supp(SOCKET server,_verb verb,_option option)  { //Suppress GA

	DWORD mode;
	GetConsoleMode(stdin1,&mode); // ENABLE_LINE_INPUT

	int set = !(mode & ENABLE_LINE_INPUT);
	log << "set int " << set << endl;

	switch(verb) {

		case verb_will: // server wants to suppress GA's
			//don't confirm - already set.
			if(set) { log << "verb_will set (ddww_supp) v/" << verb << " o/" << option << endl; break; }
			SetConsoleMode(stdin1,mode & (~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT)));
			askfor(server,verb_do,TOPT_SUPP);
			askfor(server,verb_will,TOPT_SUPP);
			askfor(server,verb_do,TOPT_ECHO);
			log << "verb_dont !set (ddww_supp) v/" << verb << " o/" << option << endl;
			break;
  		case verb_wont: // server wants to send GA's
			//don't confirm - already unset.
			if(!set) { log << "verb_wont !set (ddww_supp) v/" << verb << " o/" << option << endl; break; }
			SetConsoleMode(stdin1,mode | ENABLE_LINE_INPUT);
			askfor(server,verb_dont,TOPT_SUPP);
			askfor(server,verb_wont,TOPT_SUPP);
			log << "verb_wont set (ddww_supp) v/" << verb << " o/" << option << endl;
			break;
  		case verb_do:   // server wants me to suppress GA's
			if(set) { log << "verb_do set (ddww_supp) v/" << verb << " o/" << option << endl; break; }
			askfor(server,verb_do,TOPT_SUPP);
			log << "verb_do !set (ddww_supp) v/" << verb << " o/" << option << endl;
			break;
		case verb_dont: // server wants me to send GA's
			if(!set) { log << "verb_dont !set (ddww_supp) v/" << verb << " o/" << option << endl; break; }
			askfor(server,verb_dont,TOPT_SUPP);
			log << "verb_dont set (ddww_supp) v/" << verb << " o/" << option << endl;
			break;
	}
}


/****************************************************************************/
/* Option TERMINAL-TYPE
/****************************************************************************/
void ddww_term(SOCKET server,_verb verb,_option option) { //Subnegotiate terminal type

	switch(verb) {

		case verb_will:
			log << "verb_will (ddww_term) v/" << verb << " o/" << option << endl;
			noreply(server,verb,option); // I don't want terminal info
			break;
        case verb_wont:
			log << "verb_wont (ddww_term) v/" << verb << " o/" << option << endl;
			//dat be cool - its not going to send. no need to confirm
			break;
  		case verb_do:
			log << "verb_do (ddww_term) v/" << verb << " o/" << option << endl;
			yesreply(server,verb,option); //I'll send it when asked
			break;
  		case verb_dont://Ok - I won't
			log << "verb_dont (ddww_term) v/" << verb << " o/" << option << endl;
			break;
  	}
}


/****************************************************************************/
/* TERMINAL TYPE subnegotation
/****************************************************************************/
enum {

	SB_TERM_IS = 0,
    SB_TERM_SEND = 1
};

#define NUM_TERMINALS 2

struct {

	char* name;
	LPDATAPROC termproc;
	//pre requsites.
} terminal[NUM_TERMINALS] = {

	{ "NVT", nvt },
	{ "ANSI", ansi }
};

int term_index = 0;


/****************************************************************************/
/*
/****************************************************************************/
void sbproc_term(SOCKET server,unsigned char data) {

	log << "sbproc_term data/" << data << endl;
	if(data == SB_TERM_SEND) {
		if(term_index == NUM_TERMINALS)
			term_index = 0;
	    else
      		term_index++;
    	char buf[16]; //pls limit
    	buf[0] = IAC;
    	buf[1] = SB;
    	buf[2] = TOPT_TERM;
    	buf[3] = SB_TERM_IS;
    	lstrcpy(&buf[4],terminal[(term_index==NUM_TERMINALS)?(NUM_TERMINALS-1):term_index].name);
    	int nlen = lstrlen(&buf[4]);
    	buf[4+nlen] = IAC;
    	buf[5+nlen] = SE;
    	log << "sbproc_term buf/" << buf << endl;
    	send(server,buf,4+nlen+2,0);
	}
  	else { log << "sbproc_term != SB_TERM_SEND data/"  << data << endl; }
}


/****************************************************************************/
/*
/****************************************************************************/
struct {

	_option option;
	LPOPTIONPROC OptionProc;
	LPDATAPROC DataProc;
}  ol[] = {

	{TOPT_ECHO,   ddww_echo,  NULL},
	{TOPT_SUPP,   ddww_supp,  NULL},
	{TOPT_TERM,   ddww_term,  sbproc_term},
	{TOPT_ERROR,  ddww_error, NULL}
};


/****************************************************************************/
/* Starts Parsing of All Incoming Socket Data
/****************************************************************************/
void TelentProtcol(SOCKET server,unsigned char code) {

	//These vars are the finite state
	static int state = state_data;
	static _verb verb = verb_sb;
	static LPDATAPROC DataProc = terminal[(term_index==NUM_TERMINALS)?(NUM_TERMINALS-1):term_index].termproc;

	//Decide what to do (state based)
  	switch(state) {

  		case state_data:
  			switch(code) {
    			case IAC: state = state_code; break;
    				default: DataProc(server,code);
    		}
    		break;

  		case state_code:
    		state = state_data;
    		switch(code) {
    			// State transition back to data
    			case IAC:
					DataProc(server,code);
				 	break;
    			// Code state transitions back to data
    			case SE:
    				DataProc = terminal[(term_index==NUM_TERMINALS)?(NUM_TERMINALS-1):term_index].termproc;
      				break;
    			case NOP:
      				break;
    			case DM:
      				break;
    			case BRK:
      				break;
    			case IP:
      				break;
    			case AO:
      				break;
    			case AYT:
      				break;
    			case EC:
      				break;
    			case EL:
      				break;
    			case GA:
      				break;
    			// Transitions to option state
    			case SB:
      				verb = verb_sb;
      				state = state_option;
      				break;
    			case WILL:
      				verb = verb_will;
      				state = state_option;
      				break;
    			case WONT:
      				verb = verb_wont;
      				state = state_option;
      				break;
    			case DO:
        			verb = verb_do;
      				state = state_option;
      				break;
    			case DONT:
      				verb = verb_dont;
      				state = state_option;
      				break;
    		}
    		break;

  		case state_option:
			state = state_data;

			//Find the option entry
			int i = 0;
			for( ; ol[i].option != TOPT_ERROR && ol[i].option != code ; i++);

			//Do some verb specific stuff
			if(verb == verb_sb) DataProc = ol[i].DataProc;
			else ol[i].OptionProc(server,verb,(_option)code);
			break;
	}
}


/****************************************************************************/
/* Terminal NVT
/****************************************************************************/
void nvt(SOCKET server,unsigned char data)
{
	// Stores Parsed Incomming Telnet Data into Buffer
	int sz = strlen(sendbuff);
	sendbuff[sz] = data;
}


/****************************************************************************/
/* Terminal Ansi
/****************************************************************************/
void ansi(SOCKET server,unsigned char data)
{
	// Stores Parsed Incomming Telnet Data into Buffer
	int sz = strlen(sendbuff);
	sendbuff[sz] = data;
}

/****************************************************************************/
/* Receive Data from HOST Socket And Forward to USER / BBS Socket
/****************************************************************************/
void receiveTelnet( void *p ) {

	SOCKET sock = (SOCKET)p;
	char pBuf[20000];
	char* scan;
	int nRet;

	for (;;) {
		ZeroMemory(sendbuff,20000);
		ZeroMemory(pBuf, 20000);

	    nRet = recv(sock,pBuf,sizeof(pBuf),0);
	    if ( nRet == SOCKET_ERROR ) { log << "RECV Sock Error!" << endl; log.close(); exit(1); }
	    if ( nRet == 0) Sleep(10);
		scan = pBuf;

		while(nRet--) {	// Send for RFC Checking
			TelentProtcol(sock,*scan++);
		}

		nRet = send(client_socket,sendbuff, strlen(sendbuff) , NO_FLAGS_SET);
		if ( nRet == SOCKET_ERROR ) { log << "SEND Client_Sock Error Sending!" << endl; log.close(); exit(1); }
		// Toggle for Local Sysop Display of Telnet Data
  		if ((DISPLAYON) && (strlen(sendbuff) != 0)) ansiparse(sendbuff);
	}
}

#endif
