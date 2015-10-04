
#include "general.h"
//#include<MyDEBUG.h>




typedef enum  {NO_DEBUG=0, STDOUT_DUMP=1,FILE_DUMP=2,BOTH_DUMP=3} DEBUG_LEVEL;

DEBUG_LEVEL GlobalDebugFlag = STDOUT_DUMP;

void  MyDEBUG(const char *format, ...)
{
	/*char finalformat[200];
	finalformat[0] = '\n';
	finalformat[1] =NULL;
	strcat(finalformat,format);*/
	/*format[strlen(format)]='\n';
	fomat[strlen(format)] = '\0';*/
    if (GlobalDebugFlag == STDOUT_DUMP)
    {
			va_list ap;
			// You will get an unused variable message here -- ignore it.
			va_start(ap, format);
			printf("\n");
			vfprintf(stdout, format, ap);
			printf("\n");
			va_end(ap);
			fflush(stdout);
    }
    else if(GlobalDebugFlag  == FILE_DUMP)
    {
    	   va_list ap;
    		// You will get an unused variable message here -- ignore it.
    		va_start(ap, format);
      		 FILE * errFile;
    		 errFile = fopen ("errFile.txt","w");
    		vfprintf(errFile, format, ap);
    		va_end(ap);
    		fflush(errFile);
    }

}

#include <stdio.h>

#include <string.h> /* for strncpy */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>



struct in_addr getIPforThisNIC(string NICname)
{
 int fd;
 struct ifreq ifr;

 fd = socket(AF_INET, SOCK_DGRAM, 0);

 /* I want to get an IPv4 IP address */
 ifr.ifr_addr.sa_family = AF_INET;

 /* I want IP address attached to "eth0" */
 strncpy(ifr.ifr_name, NICname.c_str(), IFNAMSIZ-1);

 ioctl(fd, SIOCGIFADDR, &ifr);

 close(fd);

 /* display result */
 printf("In function getIPforThisNIC %s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

 return (((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);

}

string IP_to_String( uint32_t IPaddr)
{
	 char ipstrD[INET6_ADDRSTRLEN];

	memset(ipstrD,0,sizeof ipstrD);
	inet_ntop(AF_INET, &IPaddr, ipstrD, sizeof ipstrD);
	return string(ipstrD);
}


unsigned short csum(unsigned short *buf, int nwords)
{
        unsigned long sum;
        for(sum=0; nwords>0; nwords--)
                sum += *buf++;
        sum = (sum >> 16) + (sum &0xffff);
        sum += (sum >> 16);
        return (unsigned short)(~sum);
}

