#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
// #define NDEBUG
#include<assert.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include<iostream>

using namespace std;

#define maxMessageLength 500
#define MAX_SENDERS 10

#include "MyDEBUG.h"

typedef struct
{
	unsigned char msg[maxMessageLength];
	unsigned short int actualLength;
}Message;


class UDPSocket{
public:

	int sockFD;
	struct sockaddr_in lastSender;
    UDPSocket();
    bool ReceiveMessage(Message &message);
    bool SetReciverSockAddress(std::string IPofRec, std::string PortofRec);
    bool SetReciverSockAddress(struct sockaddr_in);
    bool SetReceiverAsLastSender();
    void SendMessage(Message message);
    bool BindSocketForListening(std::string portNoToListenOn);
    void SaveLastSenderInfo(unsigned short int);
    string PrintPort();

    void SetRandomReceiver();


    bool SetThisRouterAsSender(unsigned short int routerID);
//private:
    void * get_in_addr(struct sockaddr *sa);

    	// int maxMessageLength=1000;
    struct sockaddr_in sendToAddress; // address of recevier

    struct sockaddr_in* whoHaveSendMe[MAX_SENDERS];
    unsigned short int senderCount;



};



class RawSockets
{
public:
 int sockFD;
 struct sockaddr_in lastSender;

RawSockets(unsigned short int);

void RawSend();
bool ReceiveMessage(Message &message);

};


#include "aes_jh.h"


class MyKEY
{
public:

	unsigned char key_text[16]; // to be marshalled
	AES_KEY AES_KEY_ENC;
	AES_KEY AES_KEY_DEC;


	MyKEY(unsigned char routerIDinByte); // to be used by proxy
	MyKEY(unsigned char* key_text_fromProxy); // to b used by routers


	Message EncryptThisMessage(Message);
	Message DecryptThisMessage(Message);
};




