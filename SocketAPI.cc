
// some routine code snippets from beej
#include "SocketAPI.h"

/*class SimpleSocket{
 int sockfd;

 int receiverAddress;
 int myAddress;

 int CreateSocket(String sockType);		   //

 int SendData(String dataToBeSent);   //
 String ReceiveData(); //for TCP

 int GetMySockAddressIP(String &IPofRec,String &PortofRec);
 int SetMySockAddressIP(String IPofRec,int PortofRec);

 int SetRecSockAddress(String IPofRec,int PortofRec);

 int GetRecSockAddress(String &IPofRec,String &PortofRec);

 ~SimpleSocket(); 						//close all sockets, free memory if used

 Listen(); + Accept () ;  ???
 bool isThisListerSock;


 };*/

// default goodies
// code snippets from beej tutorial


//change AF_INET to AF_UNSPEC
UDPSocket::UDPSocket()
{
	if ((		sockFD = socket(AF_INET, SOCK_DGRAM, 0)		) == -1)
			{
		       perror("\nUDPSocket constructor Error : ");
		       exit(1);
			}

	cout<<"\nIn Socket constructor\n";
	srand ( time(NULL) );
	senderCount = 0 ;



	for (int var = 0; var < MAX_SENDERS; ++var) {
		whoHaveSendMe[var] = NULL ;
	}

	/*struct sockaddr_in Sockaddr;
	socklen_t len = sizeof Sockaddr;
	memset((char *)&Sockaddr,0,sizeof(Sockaddr));

	int rv = getsockname(sockFD ,(struct sockaddr *) &Sockaddr ,&len);

	if( len < sizeof Sockaddr || rv == -1)
		{
			perror("\nUDPSocket");
			exit(1);
		}
	((struct sockaddr_in *)&sad)->sin_port
	((struct sockaddr_in *)&sad)->sin_addr

	cout<<"Dynamic Port is "<<htons(Sockaddr.sin_port);*/



/*
	setsockopt(sock_fd,SOL_SOCKET,SO_RCVBUF,&n,sizeof(n));
	setsockopt(sock_fd,SOL_SOCKET,SO_REUSEADDR,&socket_reuse_on,sizeof(socket_reuse_on));
*/
}

bool UDPSocket::ReceiveMessage(Message &message)
{

	//struct sockaddr_storage from;

	//socklen_t fromlen = sizeof(from);
	socklen_t fromlen = sizeof(lastSender);
	//message.actualLength = recvfrom(sockFD, message.msg, maxMessageLength ,0, (struct sockaddr *) &from, &fromlen);

	message.actualLength = recvfrom(sockFD, message.msg, maxMessageLength ,0, (struct sockaddr *) &lastSender, &fromlen);
	if(message.actualLength == -1)
	{
		perror("\nReceiveMessage");
		exit(1);
	}

	/* char s[INET6_ADDRSTRLEN];
	 printf("listener: got packet from %s\n",  inet_ntop(from.ss_family, get_in_addr((struct sockaddr *)&from),s, sizeof s));*/

	return true;
}




void UDPSocket::SaveLastSenderInfo(unsigned short int routerID)
{
	MyDEBUG("sender count is %u",senderCount);
    assert(senderCount++ < MAX_SENDERS);
    assert(whoHaveSendMe[routerID-1] ==  NULL);

    whoHaveSendMe[routerID-1] = new struct sockaddr_in;
    // assert((routerID-1) != 1);
	* whoHaveSendMe[routerID-1] =  lastSender;
}

void UDPSocket::SetRandomReceiver()
{

	assert(senderCount > 0);
	while( 1 )
	    {
	    	int temp;
	    	if(whoHaveSendMe[temp = (rand() % MAX_SENDERS) ] ==  NULL)
	    		          continue;
	    	else
	    	               {
	    					//assert(whoHaveSendMe[1] != NULL);
	    		            sendToAddress = *whoHaveSendMe[temp];
	    	                break;
	    	               }

	    }

}



bool UDPSocket::SetThisRouterAsSender(unsigned short int routerID)
{
	sendToAddress = *whoHaveSendMe[routerID-1];
	return true;
}


bool UDPSocket::SetReceiverAsLastSender()
{

	sendToAddress=lastSender;
	return true;
}

bool UDPSocket::SetReciverSockAddress(struct sockaddr_in mySendAddress)
{
	sendToAddress=mySendAddress;
	return true;
}

bool UDPSocket::SetReciverSockAddress(std::string IPofRec, std::string PortofRec)
{
	struct addrinfo hints, *servinfo;//*p;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	int rv;
	if ((rv = getaddrinfo(IPofRec.c_str(), PortofRec.c_str(), &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "\nSetReciverSockAddress  : %s  \n", gai_strerror(rv));
		return false;
	}

	memcpy( &sendToAddress , servinfo->ai_addr , sizeof sendToAddress);
	freeaddrinfo(servinfo);
	return true;
}

void UDPSocket::SendMessage(Message message)
{
	if (	sendto(sockFD, message.msg , message.actualLength, 0 ,	(sockaddr*)&sendToAddress , (socklen_t) sizeof sendToAddress)    == -1)
	{
				perror(" \nSendMessage");
				exit(1);
	}
}



string UDPSocket::PrintPort()
{
// from katti
//remove before submission

	    struct sockaddr_in Sockaddr;
		socklen_t len = sizeof Sockaddr;
		memset((char *)&Sockaddr,0,sizeof(Sockaddr));

		int rv = getsockname(sockFD ,(struct sockaddr *) &Sockaddr ,&len);

		if( len < sizeof Sockaddr || rv == -1)
			{
				perror("\nUDPSocket");
				exit(1);
			}
	/*	((struct sockaddr_in *)&sad)->sin_port
		((struct sockaddr_in *)&sad)->sin_addr		*/
	       int portNo = ntohs(Sockaddr.sin_port );

	       //printf("P : Port No : %d\n", portNo);

	       char buffer [50];
	       sprintf(buffer,"%d",portNo);
	       MyDEBUG("The value of port number is %u",portNo);
	       return string(buffer);

}


bool UDPSocket::BindSocketForListening(std::string portNoToListenOn)
{
	struct addrinfo hints, *servinfo,*p;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	int rv;
	if ((rv = getaddrinfo(NULL, portNoToListenOn.c_str(), &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if (bind(sockFD, p->ai_addr, p->ai_addrlen) == -1)
		{
			close (sockFD);
			perror("listener: bind");
			continue;
		}
		break;
	}

	if (p == NULL)
	{
		fprintf(stderr, " BindSocketForListening: failed to bind socket\n");
		return false;
	}
	freeaddrinfo(servinfo);

	//PrintPort();
	return true;
}


void * UDPSocket::get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}


RawSockets::RawSockets(unsigned short int routerID)
{


	char NICname[25];
	snprintf(NICname,sizeof NICname-1,"eth%hu",routerID);
	struct in_addr IPofMyInterface = getIPforThisNIC( string(NICname) );

	MyDEBUG("Raw Socket for interface %s has been created",NICname);

	sockFD = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)  ;

/*	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_ifrn.ifrn_name, sizeof(ifr.ifr_ifrn.ifrn_name), "eth0",12);

    if (setsockopt(sockFD, SOL_SOCKET, SO_BINDTODEVICE,
	                (void *)NICname, sizeof(4)) < 0) {
	    	perror("Error in RawSockets constructor!!");
	    	exit(4);
	    }*/

	 struct  sockaddr_in my_sockaddr;
	 my_sockaddr.sin_family  = AF_INET;
	 my_sockaddr.sin_addr = IPofMyInterface;

	 if (bind(sockFD,  (const struct sockaddr *)&my_sockaddr, sizeof (struct sockaddr_in)) == -1)
				{
					//MyDEBUG("Error in bind for raw socket ");
					close (sockFD);
					perror("listener: bind for raw socket");
					exit(0);
				}



/*	    struct addrinfo hints, *servinfo,*p;
	    memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_INET; // set to AF_INET to force IPv4
		hints.ai_socktype = SOCK_RAW;
		hints.ai_flags = AI_PASSIVE; // use my IP

		int rv;
		if ((rv = getaddrinfo("192.168.201.2", 0, &hints, &servinfo)) != 0)
		{
			MyDEBUG("Error in getaddrinfo ");
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			exit(1);
		}

		// loop through all the results and bind to the first we can
		for (p = servinfo; p != NULL; p = p->ai_next)
		{
				if (bind(sockFD, p->ai_addr, p->ai_addrlen) == -1)
				{
					MyDEBUG("Error in bind for raw socket ");
					close (sockFD);
					perror("listener: bind");
					continue;
				}
				break;
		}

		if (p == NULL)
		{
			fprintf(stderr, " BindSocketForListening: failed to bind socket\n");

		}
		freeaddrinfo(servinfo);*/

}

void RawSockets::RawSend()
{


}

bool RawSockets::ReceiveMessage(Message &message)
{

	//struct sockaddr_storage from;

	//socklen_t fromlen = sizeof(from);
	socklen_t fromlen = sizeof(lastSender);
	//message.actualLength = recvfrom(sockFD, message.msg, maxMessageLength ,0, (struct sockaddr *) &from, &fromlen);

	message.actualLength = recvfrom(sockFD, message.msg, maxMessageLength ,0, (struct sockaddr *) &lastSender, &fromlen);

	assert(message.actualLength == 84);
	if(message.actualLength == -1)
	{
		perror("\nRawSockets:: ReceiveMessage Error of ");
		exit(1);
	}

	/* char s[INET6_ADDRSTRLEN];
	 printf("listener: got packet from %s\n",  inet_ntop(from.ss_family, get_in_addr((struct sockaddr *)&from),s, sizeof s));*/

	return true;
}


//generates keys for BOTH ENCRYPTION & DECRYPTION
MyKEY::MyKEY(unsigned char routerIDinByte)
{

	//assert(16 == AES_KEY_LENGTH_IN_CHARS);

	srand(time(NULL));

	assert(sizeof(int) == 4);
    int randomKeyInt[16/4];
	for (int var = 0; var < 16/4 ;++var) {
		randomKeyInt[var] = rand();
		}

	unsigned char * randomKey = (unsigned char*)randomKeyInt;
	for (int var = 0; var < 16; ++var) {
				this->key_text[var] = randomKey[var] ^ routerIDinByte;
			/*if( routerID16Times[var1] == 0)
				MyDEBUG("Yes Exor OWrks!!\n");*/
	}

	printf("Key For RouterID %hu is ",routerIDinByte);
	for (int var = 0; var < 16; ++var)
	{
		printf("%02X ",this->key_text[var]);
	}


	class_AES_set_encrypt_key(this->key_text,&(this->AES_KEY_ENC));
	class_AES_set_decrypt_key(this->key_text,&(this->AES_KEY_DEC));

}


//generates keys for BOTH ENCRYPTION & DECRYPTION
MyKEY::MyKEY(unsigned char* key_text_fromProxy)
{
	memcpy(this->key_text,key_text_fromProxy,16);

	printf("\nKey For RouterID ? is ");
		for (int var = 0; var < 16; ++var)
		{
			printf("%02X ",this->key_text[var]);
		}

		class_AES_set_encrypt_key(key_text_fromProxy,&(this->AES_KEY_ENC));
		class_AES_set_decrypt_key(key_text_fromProxy,&(this->AES_KEY_DEC));
}



Message MyKEY::EncryptThisMessage(Message ClearTextMsg)
{
	printf("\nIn Encrypt function\n");
	unsigned char *out=NULL;
	int out_len=-1;

	class_AES_encrypt_with_padding(ClearTextMsg.msg, ClearTextMsg.actualLength,&out,&out_len,&( this->AES_KEY_ENC));

	Message EncStuff;
	memcpy(EncStuff.msg, out,out_len);
	free(out);
	EncStuff.actualLength=out_len;

	return EncStuff;
}


Message MyKEY::DecryptThisMessage(Message EncryptedTextMsg)
{

	unsigned char *out=NULL;
	int out_len=-1;

	class_AES_decrypt_with_padding(EncryptedTextMsg.msg, EncryptedTextMsg.actualLength,&out,&out_len,&(this->AES_KEY_DEC));

	Message ClearTextStuff;
	memcpy(ClearTextStuff.msg, out,out_len);
	free(out);
	ClearTextStuff.actualLength=out_len;

	return ClearTextStuff;
}












