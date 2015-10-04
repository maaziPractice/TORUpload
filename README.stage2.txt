Name : 		Mayur Raleraskar
Course:		CSCI551
Assignment:	Project A
USC ID :	46 598 298 15


a) Reused Code: 

	I have used code from the following website for computing checksum. It has also been cited in the source code.
	http://www.tenouk.com/download/pdf/Module43.pdf

b) Complete:
   	
	The stage 2 is complete as per the specifications. 
	Assumptions :
		The Proxy knows which router to forward the packet to, as there is only one.
	Select has been used to watch on the tunnel interface and the router for data without busy waiting.

	The checksum is calculated for both ICMP and IP packets using code snippet 
from http://www.tenouk.com/download/pdf/Module43.pdf.
        
	ICMP Echo Reply is sent to the terminal from where ping was issued.I have used in place computation for this.

c) Portable: 
	I have used ntohs/hton to make the code portable.