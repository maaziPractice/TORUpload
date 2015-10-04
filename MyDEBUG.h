

#ifndef DEBUG_GUARD
#define DEBUG_GUARD
extern void  MyDEBUG(const char *format, ...);
extern struct in_addr getIPforThisNIC(string NICname);
extern unsigned short csum(unsigned short *buf, int nwords);
extern string IP_to_String( uint32_t IPaddr);
#endif
