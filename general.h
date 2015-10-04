/*
#include <stdio.h>
#include <stdlib.h>
*/

#include <errno.h>

#include <netdb.h>
// #include <sys/types.h>
#include <netinet/in.h>
// #include <sys/socket.h>
// #include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <signal.h>
// #include <pthread.h>

#include "SocketAPI.h"



//from sample tunnel.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>


#include <linux/ip.h>
#include <linux/icmp.h>




#define TunnelIPNetwork "10.5.51.2"
#define FOURTHOctet 10
#define  THIRDOctet 5
#define SECONDOctet 51
#define FIRSTOctet 2

#define PROTO_MiniTOR 253
#define LAST_HOP_PortNumber 0xFFFF

#ifndef TOR_COMMANDS
#define TOR_COMMANDS

			#define							CIRCUIT_EXTEND  0x52
			#define	                        CIRCUIT_EXTEND_DONE  0x53
			#define	                        RELAY_THIS_DATA_FWD  0x51
			#define	                        RELAY_THIS_DATA_BACKWD  0x54
	        #define	                        FAKE_DIFFIE_HELLMAN  0x65
			#define						    ENC_CIRCUIT_EXTEND  0x62
			#define							ENC_CIRCUIT_EXTEND_DONE 0x63
		    #define							RELAY_DATA_FWD_ENC 0x61
			#define							RELAY_DATA_BWD_ENC 0x64
#endif



class MyFile
{
	public:
	    MyFile(string filename,string mode);
		~MyFile();
		string readNextLine();

		bool writeLine(string singleLine);
		string getValueofThisKey(string key);
		void logIT(const char *format, ...);
		void logIT_(const char *format, ...);
		void HexDump(const unsigned char *format, unsigned int len);
	private:
		//fstream &fileObj;
		FILE* fp;
};

#define CircuitInfoEntryTableSize 10

#include<stdarg.h>
