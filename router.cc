#include "general.h"


// some routine code snippets from beej

extern string  portNoOfProxy;
typedef unsigned short WORD;
typedef unsigned char BYTE;

unsigned short int portNoOfProxy_;

// from http://www.tenouk.com/download/pdf/Module43.pdf


typedef struct
{
	struct sockaddr_in circuitRequesterAddr;
	uint32_t prevHopRealIP;
	unsigned short givenCicuitID;
	unsigned short myCircuitID;
	struct sockaddr_in nextHopRtrForCircuit;
	MyKEY *ptrToKEYForCCt;
}CircuitInfoEntry;



CircuitInfoEntry CircuitInfoEntryTable[CircuitInfoEntryTableSize] ;

UDPSocket *sockChild=NULL;
RawSockets *myRawSock=NULL;

MyFile *RouterFile;
extern int stage;

uint32_t myRawIP;

void ProcessIPPacket(Message &messsage)
{

	assert(messsage.actualLength == 84);
	struct iphdr &IPHeader = *(struct iphdr*)messsage.msg;

	char ipstrD[INET6_ADDRSTRLEN];
	memset(ipstrD,0,sizeof ipstrD);
	inet_ntop(AF_INET, &IPHeader.daddr, ipstrD, sizeof ipstrD);

	char ipstrS[INET6_ADDRSTRLEN];
	memset(ipstrS,0,sizeof ipstrS);
	inet_ntop(AF_INET, &IPHeader.daddr, ipstrS, sizeof ipstrS);

	struct icmphdr &icmp=*(struct icmphdr*)(messsage.msg + sizeof IPHeader);


	if ( stage == 2 && stage ==3 ) {
		RouterFile->logIT("ICMP from port: %hu, src: %s, dst: %s, type: %hu",
				ntohs(sockChild->lastSender.sin_port), IP_to_String(IPHeader.saddr).c_str(),IP_to_String(IPHeader.daddr).c_str(),icmp.type)  ;
	}

	icmp.type=0;
	//__u16     checksum=icmp.checksum;
	icmp.checksum=0;
	icmp.checksum = csum((unsigned short *)&icmp,(messsage.actualLength - sizeof IPHeader)/2);

	__u32 addr=IPHeader.saddr;
	IPHeader.saddr = IPHeader.daddr;
	IPHeader.daddr = addr;

	IPHeader.check = 0;
	IPHeader.check = csum((WORD *)&IPHeader,(sizeof IPHeader)/2);



	/*if (stage ==2) {
		char ICMPfromtunnel[245];
	    snprintf(ICMPfromtunnel, sizeof ICMPfromtunnel - 1, "ICMP from port %d, src:%s, dst:%s, type: %d",ntohs((sockChild->lastSender).sin_port),ipstrS,ipstrD,type );
		RouterFile->writeLine( string(ICMPfromtunnel) );
	}*/

}

int myRouterID;

bool isThisICMPEchoRequestForTunnel(Message &messsage)
{
	struct iphdr &IPHeader = *(struct iphdr*)messsage.msg;

	unsigned short int FourthThOctet,ThirdOctet,SecndOctet,FirstOctet;

	//sscanf(TunnelIPNetwork,"%hu.%hu.%hu.%hu",&FourthThOctet,&ThirdOctet,&SecndOctet,&FirstOctet);

	 char ipstrD[INET6_ADDRSTRLEN];
	 memset(ipstrD,0,sizeof ipstrD);
	 inet_ntop(AF_INET, &IPHeader.daddr, ipstrD, sizeof ipstrD);

	sscanf(  ipstrD,"%hu.%hu.%hu.%hu",&FourthThOctet,&ThirdOctet,&SecndOctet,&FirstOctet);

	if (FourthThOctet == FOURTHOctet && ThirdOctet == THIRDOctet && SecndOctet == SECONDOctet)
		return true;
	else
		return false;


}

bool SendToWorld(Message &messsage)
{
	                unsigned char * ICMPReq = messsage.msg + sizeof(struct iphdr);

	                // HARD_CODE
	                char buffer[64];
	                memcpy(buffer,ICMPReq,64);

	                struct iovec iov[3];
	                iov[0] .iov_base =(void *)buffer;
	                iov[0] .iov_len = sizeof buffer;

	                struct iphdr &IPHeader = *(struct iphdr*)messsage.msg;

					struct sockaddr_in destIPForRaw;
					memset(&destIPForRaw,0,sizeof(destIPForRaw));
					destIPForRaw.sin_family = AF_INET;
					destIPForRaw.sin_addr.s_addr = IPHeader.daddr;

	                struct msghdr myMsgHdr;
	                memset(&myMsgHdr, 0, sizeof(myMsgHdr));

	                myMsgHdr.msg_name = (caddr_t) &destIPForRaw;
	                myMsgHdr.msg_namelen = sizeof(destIPForRaw);
	                myMsgHdr.msg_iov = iov;
	                myMsgHdr.msg_iovlen = 1;

	                // SHAQ check return value!!
	                   sendmsg(myRawSock->sockFD, &myMsgHdr, 0);
	                  MyDEBUG(" AFTER sendmsg :: I am router with ID %u, just  forwarded ICMP request to internet",myRouterID);

	                  return true;
}

void ChangeDestAddressToEth0(Message &message)
{
	    struct iphdr &IPHeader = *(struct iphdr*)(message.msg);

	    IPHeader.daddr = getIPforThisNIC(string("eth0")).s_addr ;

	    IPHeader.check = 0;
	    IPHeader.check = csum((WORD *)&IPHeader,(sizeof IPHeader)/2);

}

void RelayThisStuff(Message &message)
{
	switch(message.msg[20]) //what is the TOR command ??
							{
								case RELAY_THIS_DATA_FWD:
												        	MyDEBUG("Router # %u for Stage 5 doing RELAY_THIS_DATA_FWD",myRouterID);
														//after we Fetch Circuit Info by looking up for givenCicuitID
														if( CircuitInfoEntryTable[0].nextHopRtrForCircuit.sin_port != LAST_HOP_PortNumber)
														{
																sockChild->SetReciverSockAddress(CircuitInfoEntryTable[0].nextHopRtrForCircuit);

																//changing circuit ID to mapped ID
																*( unsigned short*)((char*) (message.msg) + 21) = htons(CircuitInfoEntryTable[0].myCircuitID);

																struct iphdr *IPHeader = (struct iphdr*)(message.msg + 20 + 3 );//IP header of ICMP
																CircuitInfoEntryTable[0].prevHopRealIP=IPHeader->saddr;

																//  If the circuit exists and has a non-exit next hop, the router should change the source IP
																//  message[of the encapsulated packet] to its own, map the circuit ID from incoming to outgoing, and
																//  forward the packet to its neighbor over UDP with another relay-data control message.

																char myRouterIDString[10];
																sprintf(myRouterIDString,"%hu",myRouterID);
																IPHeader->saddr = getIPforThisNIC( string("eth") + string(myRouterIDString) ).s_addr ;

																//for logging only
																{
																	RouterFile->logIT("relay packet, circuit incoming: 0x%02x, outgoing: 0x%02x, incoming src:%s, outgoing src: %s, dst: %s" \
																			,CircuitInfoEntryTable[0].givenCicuitID, CircuitInfoEntryTable[0].myCircuitID   \
																			, IP_to_String(CircuitInfoEntryTable[0].prevHopRealIP).c_str() ,IP_to_String(IPHeader->saddr).c_str() \
																				,IP_to_String(IPHeader->daddr).c_str()
																			);
																}


																sockChild->SendMessage(message);
														}
														else // I am Last Hop!!
														{
																Message PlainTextMsg;
																assert(message.actualLength - 23 == 84);
																memcpy(PlainTextMsg.msg,message.msg+23,message.actualLength - 23);
																PlainTextMsg.actualLength = 84;
																SendToWorld(PlainTextMsg);

																//for logging only
																{
																	struct iphdr *IPHeader = (struct iphdr*)(message.msg + 20 + 3 );
																	/*RouterFile->logIT("relay packet, circuit incoming: 0x%02x, outgoing: 0x%02x, incoming src:%s, outgoing src: %s, dst: %s" \
																			,CircuitInfoEntryTable[0].givenCicuitID, CircuitInfoEntryTable[0].myCircuitID   \
																			, IP_to_String(IPHeader->saddr).c_str() , IP_to_String(myRawIP).c_str() \
																				,IP_to_String(IPHeader->daddr).c_str()
																			);*/

																	RouterFile->logIT("outgoing packet, circuit incoming: 0x%02x, incoming src: %s, outgoing src: %s, dst: %s",CircuitInfoEntryTable[0].myCircuitID, IP_to_String(IPHeader->saddr).c_str(), IP_to_String(myRawIP).c_str(), IP_to_String(IPHeader->daddr).c_str());

																}



														}

														break;

								case RELAY_THIS_DATA_BACKWD :

														assert(message.actualLength == 23 + 84);
														//after we Fetch Circuit Info by myCircuitID
														MyDEBUG("Router # %u for Stage 5 doing RELAY_THIS_DATA_BACKWD",myRouterID);

														if( CircuitInfoEntryTable[0].circuitRequesterAddr.sin_port != htons(portNoOfProxy_))
														{		//I should send stuff to
																*( unsigned short*)((char*) (message.msg) + 21) = htons(CircuitInfoEntryTable[0].givenCicuitID);
																// change desti IP
																struct iphdr &IPHeader = *(struct iphdr*)(message.msg+ 23);

																{
																	RouterFile->logIT
																	("relay reply packet, circuit incoming: 0x%02x, outgoing: 0x%02x, src: %s, incoming dst: %s, outgoing dest: %s"   \
																		,CircuitInfoEntryTable[0].myCircuitID,CircuitInfoEntryTable[0].givenCicuitID,
																		IP_to_String(IPHeader.saddr).c_str()   ,
																		IP_to_String(IPHeader.daddr).c_str() ,IP_to_String(CircuitInfoEntryTable[0].prevHopRealIP).c_str()
																	);

																}


																IPHeader.daddr = CircuitInfoEntryTable[0].prevHopRealIP;
																IPHeader.check = 0; 	IPHeader.check = csum((WORD *)&IPHeader,(sizeof IPHeader)/2);

																sockChild->SetReciverSockAddress(CircuitInfoEntryTable[0].circuitRequesterAddr);
																sockChild->SendMessage(message);

														}
														else // I am the first hop !!
														{
															    MyDEBUG("\nRouter # %u for Stage 5 doing RELAY_THIS_DATA_BACKWD decapsulating\n",myRouterID);
												                struct iphdr &IPHeader = *(struct iphdr*)(message.msg+ 20 + 3);

												                {
																	RouterFile->logIT
																	("relay reply packet, circuit incoming: %hu, outgoing: %hu, src: %s, incoming dst: %s, outgoing dest: %s"   \
																		,CircuitInfoEntryTable[0].myCircuitID,CircuitInfoEntryTable[0].givenCicuitID,
																		IP_to_String(IPHeader.saddr).c_str()   ,
																		IP_to_String(IPHeader.daddr).c_str() ,IP_to_String( getIPforThisNIC(string("eth0")).s_addr ).c_str()
																	);

																 }

												                IPHeader.daddr = getIPforThisNIC(string("eth0")).s_addr ;

																IPHeader.check = 0;
																IPHeader.check = csum((WORD *)&IPHeader,(sizeof(struct iphdr))/2);

																sockChild->SetReciverSockAddress(CircuitInfoEntryTable[0].circuitRequesterAddr);
																sockChild->SendMessage(message);

																MyDEBUG("\nRouter # %u for Stage 5 doing RELAY_THIS_DATA_BACKWD DONE with Send\n",myRouterID);

														}
														break;
								default:
																			assert(false);
																			break;


							}


}


void RelayThisStuff6(Message &message)
{

	switch(message.msg[20]) //what is the TOR command ??
							{
								case RELAY_DATA_FWD_ENC:
													MyDEBUG("Router # %u for Stage 6 doing RELAY_THIS_DATA_FWD",myRouterID);
														//after we Fetch Circuit Info by looking up for givenCicuitID
														if( CircuitInfoEntryTable[0].nextHopRtrForCircuit.sin_port != LAST_HOP_PortNumber)
														{
															  sockChild->SetReciverSockAddress(CircuitInfoEntryTable[0].nextHopRtrForCircuit);

															//changing circuit ID to mapped ID
															*( unsigned short*)((char*) (message.msg) + 21) = htons(CircuitInfoEntryTable[0].myCircuitID);
															// change source IP TODO:  ??

															  Message keyDecrypt;      memcpy(keyDecrypt.msg,message.msg+23, keyDecrypt.actualLength=message.actualLength-23);
															  keyDecrypt = (CircuitInfoEntryTable[0].ptrToKEYForCCt)->DecryptThisMessage(keyDecrypt);
															  memcpy(message.msg+23,keyDecrypt.msg,keyDecrypt.actualLength);
															  message.actualLength=23 + keyDecrypt.actualLength;

															  sockChild->SendMessage(message);

														}
														else // I am Last Hop!!
														{

															Message PlainTextMsg;      memcpy(PlainTextMsg.msg,message.msg+23, PlainTextMsg.actualLength=message.actualLength-23);
															PlainTextMsg = (CircuitInfoEntryTable[0].ptrToKEYForCCt)->DecryptThisMessage(PlainTextMsg);
															assert(PlainTextMsg.actualLength  == 84);
														    //  memcpy(message.msg+23,keyDecrypt.msg,keyDecrypt.actualLength);
															//  message.actualLength=23 + PlainTextMsg.actualLength;

															struct iphdr IP_Hdr = *(struct iphdr *)PlainTextMsg.msg;
															assert(IP_Hdr.saddr == 0);
															SendToWorld(PlainTextMsg);
															// just for check
														}
														{
															  RouterFile->logIT("relay encrypted packet, circuit incoming: 0x%02x, outgoing: 0x%02x",CircuitInfoEntryTable[0].givenCicuitID,CircuitInfoEntryTable[0].myCircuitID);
														}
														break;
								case RELAY_DATA_BWD_ENC :

														//after we Fetch Circuit Info by myCircuitID
														MyDEBUG("Router # %u for Stage 6 doing RELAY_THIS_DATA_BACKWD",myRouterID);

														//  if( CircuitInfoEntryTable[0].circuitRequesterAddr.sin_port != htons(portNoOfProxy_))
														{//I should send stuff to prev hop
															*( unsigned short*)((char*) (message.msg) + 21) = htons(CircuitInfoEntryTable[0].givenCicuitID);

															Message keyEncryptMsg;      memcpy(keyEncryptMsg.msg,message.msg+23, keyEncryptMsg.actualLength=message.actualLength-23);
															keyEncryptMsg = (CircuitInfoEntryTable[0].ptrToKEYForCCt)->EncryptThisMessage(keyEncryptMsg);
															memcpy(message.msg+23,keyEncryptMsg.msg,keyEncryptMsg.actualLength);
															message.actualLength = 23 + keyEncryptMsg.actualLength;

															sockChild->SetReciverSockAddress(CircuitInfoEntryTable[0].circuitRequesterAddr);
															sockChild->SendMessage(message);
														}
														{
																  RouterFile->logIT("relay reply encrypted packet, circuit incoming: 0x%02x, outgoing: 0x%02x",CircuitInfoEntryTable[0].myCircuitID,CircuitInfoEntryTable[0].givenCicuitID);
														}

														break;

								default:
														assert(false);
														break;

							}


}

bool TO_FORWARD_FDH=false, TO_FORWARD_ECE=false;

void CCtCreation_6(Message &r_message)
{
		struct iphdr IP_Hdr = * (struct iphdr *)r_message.msg;
		assert(IP_Hdr.protocol == PROTO_MiniTOR );

		if (r_message.msg[20] == FAKE_DIFFIE_HELLMAN)
		{
						//FDH for me!!!
						  if (TO_FORWARD_FDH==false)
						  {
							  TO_FORWARD_FDH  =  true;
							  CircuitInfoEntryTable[0].circuitRequesterAddr = sockChild->lastSender;
							  CircuitInfoEntryTable[0].givenCicuitID = ntohs(*( unsigned short*)((char*) (r_message.msg)+21) );
							  //   CircuitInfoEntryTable[0].myCircuitID = CircuitInfoEntryTable[0].givenCicuitID + 100;
							  //   CircuitInfoEntryTable[0].nextHopRtrForCircuit.sin_port = *( unsigned short*)((char*)  (r_message.msg) + 23);

							  assert(r_message.actualLength == 23+16);
							  CircuitInfoEntryTable[0].ptrToKEYForCCt = new MyKEY(r_message.msg + 23);

							  MyDEBUG("FDH Request @ Router Number %u",myRouterID);

							  	  //this is for logging only
								  {
									   RouterFile->logIT_("fake-diffie-hellman, new circuit incoming: 0x%02x, key:",CircuitInfoEntryTable[0].givenCicuitID);
									   RouterFile->HexDump(r_message.msg+23,16);
									   RouterFile->logIT_("\n");
								  }



						  }
						  else if (TO_FORWARD_FDH  == true) //FDH for others
						  {
							  // find this Circuit details, I mean next hop
							  *( unsigned short*)((char*) (r_message.msg)+21) = htons(CircuitInfoEntryTable[0].myCircuitID );

							  MyDEBUG("Router # %u doing forward FDH for givenCCtID as %u",myRouterID,CircuitInfoEntryTable[0].givenCicuitID);

							  Message keyDecrypt;      memcpy(keyDecrypt.msg,r_message.msg+23, keyDecrypt.actualLength=r_message.actualLength-23);

							  keyDecrypt = (CircuitInfoEntryTable[0].ptrToKEYForCCt)->DecryptThisMessage(keyDecrypt);

							  memcpy(r_message.msg+23,keyDecrypt.msg,keyDecrypt.actualLength);
							  r_message.actualLength=23 + keyDecrypt.actualLength;

							  char nextHopPort[10];
							  sprintf(nextHopPort,"%hu",ntohs(CircuitInfoEntryTable[0].nextHopRtrForCircuit.sin_port));
							  sockChild->SetReciverSockAddress("127.0.0.1",nextHopPort);
							  sockChild->SendMessage(r_message);

								  {   RouterFile->logIT_("fake-diffie-hellman, forwarding,  circuit incoming: 0x%02x, key: ",CircuitInfoEntryTable[0].givenCicuitID);
									  RouterFile->HexDump(keyDecrypt.msg,keyDecrypt.actualLength);
									  RouterFile->logIT_("\n");
								  }



						  }
		}
		else if(r_message.msg[20] == ENC_CIRCUIT_EXTEND)
		{
						if(TO_FORWARD_ECE== false)//first time secure cct extend request
						{
							//TODO: do this assert
						    //	assert(CircuitInfoEntryTable[0].circuitRequesterAddr == sockChild->lastSender);
							CircuitInfoEntryTable[0].myCircuitID = CircuitInfoEntryTable[0].givenCicuitID + myRouterID * 256 +1;
							Message nextHopPortMsg;    memcpy(nextHopPortMsg.msg,r_message.msg+23,nextHopPortMsg.actualLength=r_message.actualLength-23);

							nextHopPortMsg = (CircuitInfoEntryTable[0].ptrToKEYForCCt)->DecryptThisMessage(nextHopPortMsg);
							assert(nextHopPortMsg.actualLength == 2);
							CircuitInfoEntryTable[0].nextHopRtrForCircuit.sin_port =*(unsigned short*) nextHopPortMsg.msg;

							r_message.msg[20] = ENC_CIRCUIT_EXTEND_DONE;
							assert(*( unsigned short*)((char*) (r_message.msg)+21) == ntohs(CircuitInfoEntryTable[0].givenCicuitID));
							r_message.actualLength = 23;

							TO_FORWARD_ECE = true;
							 MyDEBUG("Router # %u Seding ENC_CIRCUIT_EXTEND_DONE for givenCCtID as %u",myRouterID,CircuitInfoEntryTable[0].givenCicuitID);

							sockChild->SetReceiverAsLastSender();
							sockChild->SendMessage(r_message);

							 //this is for logging only
							  {
								   RouterFile->logIT("new extend circuit: incoming: 0x%02, outgoing: 0x%02x at %hu",CircuitInfoEntryTable[0].givenCicuitID,CircuitInfoEntryTable[0].myCircuitID,ntohs(CircuitInfoEntryTable[0].nextHopRtrForCircuit.sin_port));
							  }


						}
						else if(TO_FORWARD_ECE == true)  // secure cct extend request to be fwded
						{
							*(unsigned short*) (r_message.msg +21) = htons(CircuitInfoEntryTable[0].myCircuitID);

							Message nextHopPortMsg;    memcpy(nextHopPortMsg.msg,r_message.msg+23,nextHopPortMsg.actualLength=r_message.actualLength-23);
							nextHopPortMsg = (CircuitInfoEntryTable[0].ptrToKEYForCCt)->DecryptThisMessage(nextHopPortMsg);

							memcpy(r_message.msg+23,nextHopPortMsg.msg,nextHopPortMsg.actualLength);
							r_message.actualLength=23 + nextHopPortMsg.actualLength;
							sockChild->SetReciverSockAddress(CircuitInfoEntryTable[0].nextHopRtrForCircuit);
							sockChild->SendMessage(r_message);
						}


		}
		else if(r_message.msg[20] == ENC_CIRCUIT_EXTEND_DONE)
		{
						assert(*( unsigned short*)((char*) (r_message.msg)+21) == htons(CircuitInfoEntryTable[0].myCircuitID));
						assert(r_message.actualLength == 23);

						*( unsigned short*)((char*) (r_message.msg)+21) = htons(CircuitInfoEntryTable[0].givenCicuitID);
						sockChild->SetReciverSockAddress(CircuitInfoEntryTable[0].circuitRequesterAddr);
						sockChild->SendMessage(r_message);

						{
							RouterFile->logIT("forwarding extend-done circuit, incoming: 0x%02x, outgoing: 0x%02x at %hu",    \
									CircuitInfoEntryTable[0].myCircuitID,CircuitInfoEntryTable[0].givenCicuitID,
									ntohs(CircuitInfoEntryTable[0].circuitRequesterAddr.sin_port));

						}

		}

}

bool  TO_FORWARD=false;
void Handler()
{

	Message r_message;
	r_message.actualLength = 0;

    fd_set readfds;

    int maxFDplus1 = (sockChild->sockFD > myRawSock->sockFD) ? (sockChild->sockFD + 1) : (myRawSock->sockFD+1) ;


    while(1)
    {
     	    FD_ZERO(&readfds);
    	    FD_SET(sockChild->sockFD, &readfds);
    	    FD_SET(myRawSock->sockFD, &readfds);

    	    					if (	select(maxFDplus1 , &readfds, NULL, NULL,NULL ) == -1)
    	    					   {
    	    										perror("In ROUTER : SELECT is not working because of :: ");
    	    										exit(4);
    	    					   }
    	    					for (int var = 0; var < maxFDplus1; ++var)
    	    						{
    	    							 	 	  // from UDP Sockets
    	    										if( FD_ISSET(var,&readfds) && var == sockChild->sockFD )
    	    										{
    	    													sockChild->ReceiveMessage(r_message);
    	    													struct iphdr IPHeader = *(struct iphdr*)r_message.msg;

    	    													{				struct icmphdr icmp=*(struct icmphdr*)(r_message.msg + sizeof IPHeader);
																				if ( stage == 2 && stage ==3 && stage == 4) {
																					RouterFile->logIT("ICMP from port: %hu, src: %s, dst: %s, type: %hu",
																							ntohs(sockChild->lastSender.sin_port), IP_to_String(IPHeader.saddr).c_str(),IP_to_String(IPHeader.daddr).c_str(),icmp.type)  ;
																				}
    	    													}

																if (IPHeader.protocol != 253 ) //== stages less than 5
																{

																		   if(isThisICMPEchoRequestForTunnel(r_message) && r_message.actualLength == 84 ) //is_it_ICMPEchoReqForTunnel
																			{
																					ProcessIPPacket(r_message);
																					sockChild->SetReceiverAsLastSender();
																					sockChild->SendMessage(r_message);
																			}
																			else
																			{
																				if(stage == 4 || stage == 3)
																					 SendToWorld(r_message);
																			}
																}
																else if (IPHeader.protocol == 253)
																{
																		if (stage == 5)
																		{
																								if(r_message.msg[20] == RELAY_THIS_DATA_BACKWD || r_message.msg[20] == RELAY_THIS_DATA_FWD)
																								{
																									// for logging only
																									{
																										 RouterFile->logIT_("pkt from port: %hu, length: %u, contents: ",ntohs(sockChild->lastSender.sin_port),r_message.actualLength-20);
																										 RouterFile->HexDump(r_message.msg+20,r_message.actualLength-20);
																										 RouterFile->logIT_("\n");
																									}
																									RelayThisStuff(r_message);
																								}
																								else if (r_message.msg[20] == CIRCUIT_EXTEND)
																								{
																									struct iphdr IP_Hdr = * (struct iphdr *)r_message.msg;
																									assert(IP_Hdr.protocol == PROTO_MiniTOR );

																									//new circuit build request
																									  if (TO_FORWARD==false)
																									  {
																										  CircuitInfoEntryTable[0].circuitRequesterAddr = sockChild->lastSender;
																										  CircuitInfoEntryTable[0].givenCicuitID = ntohs(*( unsigned short*)((char*) (r_message.msg)+21) );
																										  CircuitInfoEntryTable[0].myCircuitID = CircuitInfoEntryTable[0].givenCicuitID + myRouterID * 256 +1;
																										  CircuitInfoEntryTable[0].nextHopRtrForCircuit.sin_port = *( unsigned short*)((char*)  (r_message.msg) + 23);

																										  {
																											  RouterFile->logIT_("pkt from port: %hu, length: %hu, contents: ",ntohs(sockChild->lastSender.sin_port),r_message.actualLength-20);
																											  RouterFile->HexDump(r_message.msg+20,r_message.actualLength-20);
																											  RouterFile->logIT_("\n");
																											  RouterFile->logIT("new extend circuit: incoming: 0x%02x, outgoing: 0x%02x at %hu",CircuitInfoEntryTable[0].givenCicuitID,CircuitInfoEntryTable[0].myCircuitID,ntohs(sockChild->lastSender.sin_port));
																										  }


																										  TO_FORWARD  =  true;
																										  MyDEBUG("Circuit Extend Request @ Router Number %u",myRouterID);

																										  r_message.msg[20] =CIRCUIT_EXTEND_DONE;
																										  sockChild->SetReceiverAsLastSender();
																										  r_message.actualLength = 23;
																										  sockChild->SendMessage(r_message);




																									  }
																									  else if (TO_FORWARD  == true)
																									  {
																										  // find this Circuit details

																										  char nextHopPort[10];
																										  sprintf(nextHopPort,"%hu",ntohs(CircuitInfoEntryTable[0].nextHopRtrForCircuit.sin_port));

																										  //shaq	  sockChild->SetReciverSockAddress("127.0.0.1",nextHopPort);

																										  sockChild->SetReciverSockAddress(CircuitInfoEntryTable[0].nextHopRtrForCircuit);
																										  *( unsigned short*)((char*) (r_message.msg)+21) = htons(CircuitInfoEntryTable[0].myCircuitID );

																										  MyDEBUG("Router # %u doing forward for cctID %u",myRouterID,CircuitInfoEntryTable[0].givenCicuitID);

																										  sockChild->SendMessage(r_message);

																										  {
																											  RouterFile->logIT_("pkt from port: %hu, length: %hu, contents: ",ntohs(sockChild->lastSender.sin_port),r_message.actualLength-20);
																											  RouterFile->HexDump(r_message.msg+20,r_message.actualLength-20);
																											  RouterFile->logIT_("\n");
																											  RouterFile->logIT("forwarding extend circuit: incoming: 0x%02x, outgoing: 0x%02x at %hu",CircuitInfoEntryTable[0].givenCicuitID,CircuitInfoEntryTable[0].myCircuitID,ntohs(CircuitInfoEntryTable[0].nextHopRtrForCircuit.sin_port));
																										  }


																									  }
																								}
																								else if (r_message.msg[20] == CIRCUIT_EXTEND_DONE)
																								{
																										assert((r_message.actualLength == 23));
																										sockChild->SetReciverSockAddress(CircuitInfoEntryTable[0].circuitRequesterAddr);
																										sockChild->SendMessage(r_message);

																										 {
																											  RouterFile->logIT_("pkt from port: %hu, length: %hu, contents: ",ntohs(sockChild->lastSender.sin_port),r_message.actualLength-20);
																											  RouterFile->HexDump(r_message.msg+20,r_message.actualLength-20);
																											  RouterFile->logIT_("\n");
																											  RouterFile->logIT("forwarding extend circuit: incoming: 0x%02x, outgoing: 0x%02x at %hu",CircuitInfoEntryTable[0].myCircuitID,CircuitInfoEntryTable[0].givenCicuitID,ntohs(CircuitInfoEntryTable[0].circuitRequesterAddr.sin_port));
																										 }
																								}

																		}
																		else if(stage == 6 )
																		{

																			//for logging only
																			{//pkt from port: 52610, length: 19, contents: 0x650001c1b00841fa9f973b97f83b951757608a
																				 RouterFile->logIT_("pkt from port: %hu, length: %hu, contents: ",ntohs(sockChild->lastSender.sin_port),r_message.actualLength-20);
																				 RouterFile->HexDump(r_message.msg+20,r_message.actualLength-20);
																				 RouterFile->logIT_("\n");
																			}

																									if (r_message.msg[20] == FAKE_DIFFIE_HELLMAN || r_message.msg[20] == ENC_CIRCUIT_EXTEND || r_message.msg[20] == ENC_CIRCUIT_EXTEND_DONE)
																									{
																										CCtCreation_6(r_message);
																									}
																									else if(r_message.msg[20] == RELAY_DATA_BWD_ENC || r_message.msg[20] == RELAY_DATA_FWD_ENC)
																									{
																										RelayThisStuff6(r_message);
																									}
																									else
																										assert(false);

																		}


																}

    	    										}
    	    										// from Raw Socket
    	    										else if ( FD_ISSET(var,&readfds) && var == myRawSock->sockFD )
    	    										{
    	    												MyDEBUG("Router # %u::Reading from Raw Socket, Reply for ping has come from the internet",myRouterID);

    	    											    myRawSock->ReceiveMessage(r_message);

															if (stage == 4 || stage == 3)
															{
																			{
																				struct iphdr &IPHeader = *(struct iphdr*)(r_message.msg);
																				struct icmphdr &icmp=*(struct icmphdr*)(r_message.msg + sizeof IPHeader);
																				RouterFile->logIT("ICMP from raw sock, src: %s, dst: %s, type: %hu",IP_to_String(IPHeader.saddr).c_str(),IP_to_String(IPHeader.daddr).c_str(),icmp.type);
																			}


																			ChangeDestAddressToEth0(r_message);
																			sockChild->SetReceiverAsLastSender();
																			sockChild->SendMessage(r_message);
																			MyDEBUG("Router # %u:: ICMP Reply is being forwarded to Proxy",myRouterID);
															}
															else if(stage == 5)
															{
																	  Message myWrappedMsg;
																		{
																			struct iphdr IP_Hdr;
																			memset(&IP_Hdr,0,sizeof(IP_Hdr));

																			IP_Hdr.protocol =  PROTO_MiniTOR ;
																			IP_Hdr.daddr = IP_Hdr.saddr = inet_addr("127.0.0.1");

																			*(struct iphdr*) myWrappedMsg.msg = IP_Hdr ;
																		}
																	 (myWrappedMsg.msg)[20] = RELAY_THIS_DATA_BACKWD;

																	 *( unsigned short*)((char*) (myWrappedMsg.msg)+21) = htons(CircuitInfoEntryTable[0].givenCicuitID );

																	 assert(r_message.actualLength == 84);

																	// ChangeDestAddressToEth0(r_message);

																	 memcpy(myWrappedMsg.msg+23, r_message.msg ,r_message.actualLength);

																	//sockChild->SetReciverSockAddress(CircuitInfoEntryTable[0].circuitRequesterAddr);

																	myWrappedMsg.actualLength = 23 + 84;

																	MyDEBUG("Sahi re Sahi");
																	RelayThisStuff(myWrappedMsg);

																	//sockChild->SendMessage(myWrappedMsg);

															}
															else if(stage == 6)
															{
																		 Message myWrappedMsg;
																			{
																				struct iphdr IP_Hdr;
																				memset(&IP_Hdr,0,sizeof(IP_Hdr));

																				IP_Hdr.protocol =  PROTO_MiniTOR ;
																				IP_Hdr.daddr = IP_Hdr.saddr = inet_addr("127.0.0.1");

																				*(struct iphdr*) myWrappedMsg.msg = IP_Hdr ;
																			}
																		 (myWrappedMsg.msg)[20] = RELAY_DATA_BWD_ENC;

																		 *( unsigned short*)((char*) (myWrappedMsg.msg)+21) = htons(CircuitInfoEntryTable[0].givenCicuitID );

																		 assert(r_message.actualLength  == 84);
																		 struct iphdr *IP_HdrPtr = (struct iphdr *)r_message.msg;

																					 {
																			 	 	 	 RouterFile->logIT("incoming packet, src: %s, dst: %s, outgoing circuit: 0x%02x",IP_to_String(IP_HdrPtr->saddr).c_str(),IP_to_String(IP_HdrPtr->daddr).c_str(),CircuitInfoEntryTable[0].givenCicuitID);
																					 }



																		 IP_HdrPtr->daddr = 0;

																		Message keyEncryptMsg;      memcpy(keyEncryptMsg.msg,r_message.msg, keyEncryptMsg.actualLength=r_message.actualLength);
																		keyEncryptMsg = (CircuitInfoEntryTable[0].ptrToKEYForCCt)->EncryptThisMessage(keyEncryptMsg);

																		memcpy(myWrappedMsg.msg+23, keyEncryptMsg.msg ,keyEncryptMsg.actualLength);

																		sockChild->SetReciverSockAddress(CircuitInfoEntryTable[0].circuitRequesterAddr);
																		myWrappedMsg.actualLength = 23 + keyEncryptMsg.actualLength;
																		sockChild->SendMessage(myWrappedMsg);
															}



    	    										}


    	    						}

    }



}



void Router(short unsigned int myIDfromProxy)
{


	myRouterID = myIDfromProxy;

	sockChild = new UDPSocket();
	myRawSock = new RawSockets(myRouterID);

	sscanf(portNoOfProxy.c_str(),"%hu",&portNoOfProxy_);


	char RouterFileName[45];
	sprintf(RouterFileName,"stage%d.router%d.out",stage,myRouterID);

	RouterFile = new MyFile(RouterFileName,"w");

	MyDEBUG("I am router with routerID as %u and my file as %s",myRouterID,RouterFileName);

	Message message;
	char temp[20]; sprintf(temp,"%hu",myRouterID);
	myRawIP = getIPforThisNIC(string("eth")+string(temp)).s_addr;


	if (stage >= 4) {
		sprintf((char *)message.msg,"router: %u, pid: %u, port: %s, IP: %s",myRouterID,(int)getpid(),sockChild->PrintPort().c_str(), IP_to_String(myRawIP).c_str() );
		message.actualLength=strlen((char *)message.msg)+1;
	}
	else if(stage <= 3)
	{
		sprintf((char *)message.msg,"router: %u, pid: %u, port: %s",myRouterID,(int)getpid(),sockChild->PrintPort().c_str());
		message.actualLength=strlen((char *)message.msg)+1;
	}

	RouterFile->logIT((char *)message.msg);

	sleep(2);

	MyDEBUG("Router # %hu ::Aboout to send up message to Proxy",myRouterID);

	// TODO: SHAQ!!
	sockChild->SetReciverSockAddress("localhost",portNoOfProxy);

	sockChild->SendMessage(message);

	//RouterFile->logIT("router: %u, pid: %u, port: %s",myRouterID,(int)getpid(),sockChild->PrintPort().c_str());

	if (stage == 1) {
								exit(0);
							}

	Handler();



}
