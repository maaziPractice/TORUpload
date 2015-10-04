Name : 		Mayur Raleraskar
Course:		CSCI551
Assignment:	Project A
USC ID :	46 598 298 15



a) Reused Code: 
	I have referred to 2 books namely, "Beej's Guide to Network Programming - Using Internet Sockets" and UNIX Network Programming, Volume 1 by Richard Stevens
	I mainly used the above books for getting to know the different data structures defined in the include files and the various system calls and how they can be used.


b) Complete:
    	The stage 1 works as per the specifications. 
	Configuration file is parsed successfully if it is exactly as shown in the assignment file.
	I have assumed there will only be 1 router.
	If configuration file mentions the stage to be 1, the program will create the folowing log files :
		stage1.proxy.out
		stage1.router1.out

c) Portable: 
    	The issue here is Byte Ordering.I have used portable functions like hton before sending(numerals in headers of UDP and IP headers) on wire & ntoh before reading into machine data type like int. Hence code should be pretty portable especially on *nux systems.
	

