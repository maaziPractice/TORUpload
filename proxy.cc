#include "general.h"

extern string  portNoOfProxy;
extern int noOfRouters;
MyFile *proxyFile;
extern UDPSocket sockParent;
extern int stage;
extern int noOfHops;


// some routine code snippets from beej
/**************************************************************************
 * tun_alloc: allocates or reconnects to a tun/tap device.
 * copy from from simpletun.c, used with permission of the author
 * refer to http://backreference.org/2010/03/26/tuntap-interface-tutorial/ for more info
 **************************************************************************/

int tun_alloc(char *dev, int flags)
{
    struct ifreq ifr;
    int fd, err;
    char *clonedev = (char*)"/dev/net/tun";

    if( (fd = open(clonedev , O_RDWR)) < 0 )
    {
	perror("Opening /dev/net/tun");
	return fd;
    }

    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = flags;

    if (*dev) //??
    {
	strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    }

    if( (err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0 )
    {
	perror("ioctl(TUNSETIFF)");
	close(fd);
	return err;
    }

    strcpy(dev, ifr.ifr_name);
    return fd;
}

typedef unsigned short NextHopInfo;
typedef struct
{
   struct iphdr IP_Hdr;
   unsigned char TypeByte;
   unsigned short CircuitID;
   NextHopInfo nextHopRtrID;

}UDPPayLoad_ADT;

char* CreateExtendCctMessage(unsigned short int CctID,unsigned short nextHopRtrID)
{

	char * udpPayLoad = new char[25];

	struct iphdr IP_Hdr;
	memset(&IP_Hdr,0,sizeof(IP_Hdr));
	IP_Hdr.protocol =  PROTO_MiniTOR ;
	IP_Hdr.daddr = IP_Hdr.saddr = inet_addr("127.0.0.1");

	*(struct iphdr*) udpPayLoad = IP_Hdr ;
	udpPayLoad[20] = CIRCUIT_EXTEND;

	*( unsigned short*)((char*)udpPayLoad+21) = CctID;

// ASSUME
if(LAST_HOP_PortNumber != nextHopRtrID)
	*( unsigned short*)((char*)udpPayLoad+23) = ( sockParent.whoHaveSendMe[ nextHopRtrID - 1] )->sin_port;
else
	*( unsigned short*)((char*)udpPayLoad+23)= LAST_HOP_PortNumber;

return udpPayLoad;



}


int* RandomlySelectHops2()
{

	int* randomHOPS = new int[noOfHops+1];
	// TODO: select randomly increasing routers

		 srand(time(NULL));
		int i=(rand()%noOfRouters);
		 //proxyFile->logIT("hop: %d, router: %d",0+1,randomHOPS[0]);


		 int a[noOfRouters];

		for (int var = 0; var < noOfRouters; ++var) {
			a[var]=var+1;
		}


		for ( int var1 = 0; var1 < noOfHops; ++var1) {
			randomHOPS[var1]=a[(++i)%noOfRouters];

			proxyFile->logIT("hop: %hu, router: %hu",var1+1,randomHOPS[var1]);

			MyDEBUG("Rogog");
		}


		/*	for (int var = 0; var < noOfHops; ++var)
			{
				randomHOPS[var] = i;
				proxyFile->logIT("hop: %d, router: %d",var+1,randomHOPS[var]);
				i = (i%noOfRouters) + 1;
			}*/
	return randomHOPS;
}


static int s = 1 ;
int* myCctHops=NULL;
void Build_An_Circuit()
{

	myCctHops = RandomlySelectHops2();

   myCctHops[noOfHops] = LAST_HOP_PortNumber;

   int CircuitID = (0*256) + (s++)         ;
   Message messagefromRouter,messageToRouter;

   sockParent.SetThisRouterAsSender(myCctHops[0]);
	for (int var = 0; var < noOfHops; ++var)
	{
		char* temp = CreateExtendCctMessage(  CircuitID , myCctHops[var+1]  );
		memcpy(messageToRouter.msg , temp ,25);
		delete []temp;
		messageToRouter.actualLength = 25;
		sockParent.SendMessage(messageToRouter);
		sockParent.ReceiveMessage(messagefromRouter);

       if(messagefromRouter.msg[20] == CIRCUIT_EXTEND_DONE && messagefromRouter.actualLength == 23)
    	   	   	   	   {
    	   	   	   	   	   	   	   MyDEBUG("Got a CIRCUIT_Extend_DONE");

    	   	   	   	   	   	   	   proxyFile->logIT_("pkt from port: %hu, length: %hu, contents: ",ntohs(sockParent.lastSender.sin_port),messageToRouter.actualLength-20);
    	   	   	   	   	   	   	   proxyFile->HexDump(messageToRouter.msg+20,messageToRouter.actualLength-20);
    	   	   	   	   	   	   	   proxyFile->logIT_("\n");
    	   	   	   	   	   	   	   proxyFile->logIT("incoming extend-done circuit, incoming: 0x%02x from port: %hu",*(unsigned short *)(messagefromRouter.msg+21),htons(sockParent.lastSender.sin_port));

    	   	   	   	   }

	}


}



void EnterTainRouterUpMsges()
{
	Message messagefromRouter;
	for (int var = 0; var < noOfRouters; ++var)
	{

			sockParent.ReceiveMessage(messagefromRouter);
			if(strncmp((char *)messagefromRouter.msg,"router: ",strlen("router ")) == 0)
					  {

						  char singleLineChar[45];
						  sprintf(singleLineChar,"%d", ntohs(sockParent.lastSender.sin_port ));

						  proxyFile->logIT((char *)messagefromRouter.msg); //(string((char *)messagefromRouter.msg)+string(singleLineChar));

						  unsigned short routerID;
						  // SHAQ !!
						  sscanf((char *)messagefromRouter.msg,"router: %hu",&routerID);
						  assert(routerID < 7 && routerID >=1);
						  sockParent.SaveLastSenderInfo(routerID);
					  }
			else
				assert(false);
	}

	if (stage == 1)
	   						exit(0);

	/*if (stage >= 5)
			proxyFile->logIT("hop: %u, router: %u",noOfHops,noOfRouters);
*/
}

typedef struct
{
	//SHAQ
	int routerID;
	MyKEY* routerKEY;
}TOR_HOP;
TOR_HOP * TOR_CCt_HOPs=NULL;  //[MAX_CCTS_ALLOWED];









#define MAX_CCTS_ALLOWED 2


Message OnionEncrypt(Message PlainText, int nthHopOfCCt)
{

//	MyDEBUG("Value of noOfHops %d & length of PlainText is %u",nthHopOfCCt,PlainText.actualLength);

	while(nthHopOfCCt >= 0)
	{

				PlainText =  ( TOR_CCt_HOPs[nthHopOfCCt--].routerKEY ) -> EncryptThisMessage(PlainText);
	}
	//shaq
	Message OnionEncryptText = PlainText; // just for readbilitysss
	return OnionEncryptText;
}

Message OnionDecrypt(Message EncrText)
{

//	MyDEBUG("Value of noOfHops %d & length of PlainText is %u",nthHopOfCCt,PlainText.actualLength);

	unsigned short int i=0;
	while(i  <= noOfHops-1)
	{

		EncrText =  ( TOR_CCt_HOPs[i++].routerKEY ) ->DecryptThisMessage(EncrText);
	}
	//shaq
	Message PlainText = EncrText; // just for readbilitysss
	return PlainText;
}


// Following 2 are Onion functions
// nthHopOfCCt starts from zero
Message CreateOnionEncryptMessages(unsigned short int CircuitID,   int nthHopOfCCt,unsigned short TOR_cmd,Message* packetOfIP)
{

	Message TorHDRPlusOnionEncryptMsg;
	unsigned char * TorPayLoad = NULL;

	// TODO: ASK JAI, JANA
   {
		struct iphdr IP_Hdr;
		memset(&IP_Hdr,0,sizeof(IP_Hdr));
		IP_Hdr.protocol =  PROTO_MiniTOR ;
		IP_Hdr.daddr = IP_Hdr.saddr = inet_addr("127.0.0.1");
		*(struct iphdr*) TorHDRPlusOnionEncryptMsg.msg = IP_Hdr ;
		TorHDRPlusOnionEncryptMsg.actualLength = sizeof(struct iphdr);

		unsigned int BaseOffSet = sizeof(struct iphdr);
		TorPayLoad = TorHDRPlusOnionEncryptMsg.msg+BaseOffSet;
		TorPayLoad[0] = TOR_cmd;
		TorHDRPlusOnionEncryptMsg.actualLength+=1; // for TYPE to be put in switch
	}

	*(unsigned short int*)(TorPayLoad + 1) = htons(CircuitID);
	TorHDRPlusOnionEncryptMsg.actualLength+=2;
	unsigned short OnionEncryptMessagesCurrentlength = TorHDRPlusOnionEncryptMsg.actualLength;

	Message PlainText;
	Message OnionEncrypted;
	unsigned short int routerID; struct iphdr *IPHeader=NULL;
	switch (TOR_cmd)
	{
		case FAKE_DIFFIE_HELLMAN:
											memcpy(PlainText.msg, TOR_CCt_HOPs[nthHopOfCCt].routerKEY->key_text, 16);
											PlainText.actualLength = 16;
											OnionEncrypted = OnionEncrypt(PlainText,nthHopOfCCt-1);
											memcpy(TorHDRPlusOnionEncryptMsg.msg+OnionEncryptMessagesCurrentlength, OnionEncrypted.msg, OnionEncrypted.actualLength );
											TorHDRPlusOnionEncryptMsg.actualLength += OnionEncrypted.actualLength;
											//goto DONE;
											break;

		case ENC_CIRCUIT_EXTEND:
											routerID = TOR_CCt_HOPs[ nthHopOfCCt + 1].routerID ;
											(routerID == 0xFF) ? routerID=0xFFFF : routerID=sockParent.whoHaveSendMe[routerID-1]->sin_port ;

											*(unsigned short*)PlainText.msg =  routerID; PlainText.actualLength=sizeof(unsigned short);
											OnionEncrypted = OnionEncrypt(PlainText,nthHopOfCCt);
											memcpy(TorHDRPlusOnionEncryptMsg.msg+OnionEncryptMessagesCurrentlength, OnionEncrypted.msg, OnionEncrypted.actualLength );
											TorHDRPlusOnionEncryptMsg.actualLength += OnionEncrypted.actualLength;
											break;

		case RELAY_DATA_FWD_ENC:
											MyDEBUG("In Proxy for Stage 6");
											IPHeader = (struct iphdr*)(*packetOfIP).msg;
											assert(packetOfIP->actualLength == 84);
											IPHeader->saddr = 0;
											OnionEncrypted = OnionEncrypt(*packetOfIP, nthHopOfCCt);
											memcpy(TorHDRPlusOnionEncryptMsg.msg+OnionEncryptMessagesCurrentlength, OnionEncrypted.msg, OnionEncrypted.actualLength );
											TorHDRPlusOnionEncryptMsg.actualLength += OnionEncrypted.actualLength;
											break;

		default:
											assert(false);
											break;
	}


	return TorHDRPlusOnionEncryptMsg;

}


void Build_An_Circuit_6()
{

	TOR_CCt_HOPs = new TOR_HOP[noOfHops+1] ;
	// first and last entries is GOD!!

	int* myHOPRouter = RandomlySelectHops2();
	for (int var = 0; var < noOfHops; ++var)
	{
		TOR_CCt_HOPs[var].routerID = myHOPRouter[var];
		assert(TOR_CCt_HOPs[var].routerID > 0);
		TOR_CCt_HOPs[var].routerKEY = new MyKEY((unsigned char)  TOR_CCt_HOPs[var].routerID);
	}
	// shaq:
	delete []myHOPRouter;

	TOR_CCt_HOPs[noOfHops].routerID = 0xFF;

	 int CircuitID = (0*256) + (s++)         ;
    Message messagefromRouter;

	sockParent.SetThisRouterAsSender(TOR_CCt_HOPs[0].routerID);
	   int var;
		for ( var = 0; var < noOfHops; ++var)
		{
			Message fakeDiffieMsg = CreateOnionEncryptMessages(CircuitID, var, FAKE_DIFFIE_HELLMAN,NULL);

			sockParent.SendMessage(fakeDiffieMsg);

			sleep(2);

			Message EncryCCtExtendMsg = CreateOnionEncryptMessages(CircuitID,var,ENC_CIRCUIT_EXTEND,NULL);

			sockParent.SendMessage(EncryCCtExtendMsg);

			MyDEBUG("\nWaiting on recv from\n");
			sockParent.ReceiveMessage(messagefromRouter);
			MyDEBUG("\nOut of recv from\n");

	       assert( messagefromRouter.msg[20] == ENC_CIRCUIT_EXTEND_DONE && messagefromRouter.actualLength == 23
	    		             && CircuitID==ntohs( *(unsigned short int*)(messagefromRouter.msg+21) )
	    		       );
	    	   	   	   	   MyDEBUG("Got a ENC_CIRCUIT_EXTEND_DONE");

			//sleep(5);
		}

		MyDEBUG("Out of CCT %d",var);


}



// some snippet from beej of select
void tunnelPlusSock_reader()
{
    char tun_name[IFNAMSIZ];

    strcpy(tun_name, "tun1");

    //A point-to-point device (IFF_TUN). The TUN device allows for processing IP packets.
    int tun_fd = tun_alloc(tun_name, IFF_TUN | IFF_NO_PI);

    if(tun_fd < 0)
    {
		perror("Open tunnel interface");
		exit(1);
    }

    fd_set readfds;
    int maxFDplus1 = (sockParent.sockFD > tun_fd) ? (sockParent.sockFD + 1) : (tun_fd+1) ;
    while(1)
    {
     	    FD_ZERO(&readfds);
    	    FD_SET(sockParent.sockFD, &readfds);
    	    FD_SET(tun_fd, &readfds);
    	    char ipstrD[INET6_ADDRSTRLEN];
    	    char ipstrS[INET6_ADDRSTRLEN];
    	    struct icmphdr icmp;	struct iphdr IPHeader;

					   if (	select(maxFDplus1 , &readfds, NULL, NULL,NULL ) == -1)
					   {
										perror("SELECT Gandale!!");
										exit(4);
					   }
						for (int var = 0; var < maxFDplus1; ++var)
						{
										//   Messages from Router
										if( FD_ISSET(var,&readfds) && var == sockParent.sockFD )
										{
											Message messagefromRouter;
											sockParent.ReceiveMessage(messagefromRouter);
													if (stage <= 4) {


														IPHeader = *(struct iphdr*)messagefromRouter.msg;

														memset(ipstrD,0,sizeof ipstrD);
														inet_ntop(AF_INET, &IPHeader.daddr, ipstrD, sizeof ipstrD);


														memset(ipstrS,0,sizeof ipstrS);
														inet_ntop(AF_INET, &IPHeader.saddr, ipstrS, sizeof ipstrD);

														// struct icmphdr icmp;
														memset(&icmp,1,sizeof icmp);
														icmp=*(struct icmphdr*)(messagefromRouter.msg + sizeof IPHeader);

														assert(icmp.type == 0);

														proxyFile->logIT("ICMP from port: %hu, src: %s, dst: %s, type: %hu",ntohs(sockParent.lastSender.sin_port),ipstrS,ipstrD,icmp.type);

														write(tun_fd,messagefromRouter.msg,messagefromRouter.actualLength);
													}
													else if(stage == 5 )
													{
														MyDEBUG("Proxy doing decapsu!!!\n");

															// logging block
															{
																   proxyFile->logIT_("pkt from port: %hu, length: %hu, contents: ",ntohs(sockParent.lastSender.sin_port),messagefromRouter.actualLength-20);
																   proxyFile->HexDump(messagefromRouter.msg+20,messagefromRouter.actualLength-20);
																   proxyFile->logIT_("\n");
																   struct iphdr *IP_HDR = (struct iphdr *)messagefromRouter.msg+20+3;
																   proxyFile->logIT("incoming packet, circuit incoming: 0x%02x, src: %s, dst: %s",*(unsigned short *)(messagefromRouter.msg+21),IP_to_String(IP_HDR->saddr).c_str(),IP_to_String(IP_HDR->daddr).c_str());
															}



														write(tun_fd,messagefromRouter.msg+23,messagefromRouter.actualLength-23);
													}
													else
													{
														MyDEBUG("Stage 6 Proxy doing decapsu!!!\n");
														assert(messagefromRouter.msg[20] == RELAY_DATA_BWD_ENC);
														Message EncryptedIP;
														memcpy(EncryptedIP.msg,messagefromRouter.msg+23,EncryptedIP.actualLength = messagefromRouter.actualLength-23);
														Message DecryptedIP = OnionDecrypt(EncryptedIP);
														assert(DecryptedIP.actualLength == 84);

														struct iphdr *IPHDR = (struct iphdr *)DecryptedIP.msg;
														IPHDR->daddr = getIPforThisNIC("eth0").s_addr;
														IPHDR->check=0;
														IPHDR->check = csum((unsigned short *)IPHDR, sizeof(struct iphdr)/2);
														write(tun_fd,DecryptedIP.msg,DecryptedIP.actualLength);

													}


										}
										else if( FD_ISSET(var,&readfds) && var == tun_fd) // messages from tunnel
										{   		//cout<<"\nJust after second if\n";

													Message messageToRouter;
													messageToRouter.actualLength = read(tun_fd,messageToRouter.msg,sizeof(messageToRouter.msg));

													MyDEBUG(" Read packet from tunnel of length %u",messageToRouter.actualLength);
													if(messageToRouter.actualLength < 0)
													{
																perror("Reading from tunnel interface");
																close(tun_fd);
																exit(1);
													}
													else
													{
													//			printf("\nRead a packet from tunnel, packet length:%d\n", messageToRouter.actualLength);
																 IPHeader = *(struct iphdr*)messageToRouter.msg;

																//char ipstrD[INET6_ADDRSTRLEN];
																memset(ipstrD,0,sizeof ipstrD);
																inet_ntop(AF_INET, &IPHeader.daddr, ipstrD, sizeof ipstrD);

																//char ipstrS[INET6_ADDRSTRLEN];
																memset(ipstrS,0,sizeof ipstrD);
																inet_ntop(AF_INET, &IPHeader.saddr, ipstrS, sizeof ipstrD);


																memset(&icmp,1,sizeof icmp);
																icmp=*(struct icmphdr*)(messageToRouter.msg + sizeof IPHeader);
																assert(icmp.type == 8);

											//					ICMP from tunnel, src: 10.5.51.2, dst: 10.5.51.3, type: 8

																if (stage  >= 2) {
																	proxyFile->logIT("ICMP from tunnel, src: %s, dst: %s, type: %hu",ipstrS,ipstrD,icmp.type);
																}

																if (stage == 4 || stage == 3)
																{
																	sockParent.SetRandomReceiver();
																	sockParent.SendMessage(messageToRouter);
																}
																else if( stage == 5  )
																{
																	MyDEBUG("In Proxy for Stage 5");
																	sockParent.SetReciverSockAddress( *( sockParent.whoHaveSendMe[ myCctHops[0] - 1 ]) ) ;

																	memmove(messageToRouter.msg+23,messageToRouter.msg,messageToRouter.actualLength);

																	{
																		struct iphdr IP_Hdr;
																		memset(&IP_Hdr,0,sizeof(IP_Hdr));

																		IP_Hdr.protocol =  PROTO_MiniTOR ;
																		IP_Hdr.daddr = IP_Hdr.saddr = inet_addr("127.0.0.1");

																		*(struct iphdr*) messageToRouter.msg = IP_Hdr ;
																	}

																   (messageToRouter.msg)[20] = RELAY_THIS_DATA_FWD;

																   *( unsigned short*)((char*) (messageToRouter.msg)+21) = s -1 ;

																	messageToRouter.actualLength = messageToRouter.actualLength + 23;
																	sockParent.SendMessage(messageToRouter);
																}
																else if(stage == 6)
																{
																	unsigned short i = TOR_CCt_HOPs[0].routerID;
																	sockParent.SetReciverSockAddress( *( sockParent.whoHaveSendMe[ i-1 ]) ) ;
																	MyDEBUG("In Proxy for Stage 6");
																	Message EncryptedIP_Packet = CreateOnionEncryptMessages(s-1,noOfHops-1,RELAY_DATA_FWD_ENC,&messageToRouter);

																	sockParent.SendMessage(EncryptedIP_Packet);

																	MyDEBUG("In Proxy DONE sending");

																}


													}

										}
						}
    }
}






void Proxy()
{
		char proxyFileName[45];
		sprintf(proxyFileName,"stage%d.proxy.out",stage);
	//	printf("proxyFileName in proxy is %s\n",proxyFileName);
		proxyFile = new MyFile(proxyFileName,"w");
  //	proxyFile->writeLine(   string("proxy port: ") +  portNoOfProxy       );
		proxyFile->logIT("proxy port: %s",portNoOfProxy.c_str());

		EnterTainRouterUpMsges();

		if(stage > 4 )
		{
			assert(noOfHops > 0);
			assert(noOfHops <= noOfRouters);
			if(stage == 5) Build_An_Circuit();
			else if(stage == 6) Build_An_Circuit_6();
		}


		tunnelPlusSock_reader();
}














