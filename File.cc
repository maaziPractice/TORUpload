
#include "general.h"

MyFile::MyFile(string filename,string mode)
{
		if(mode.compare("r") == 0){
			fp=fopen(filename.c_str(), "r");
		}
		else if(mode.compare( "w") == 0)
		{
			fp=fopen(filename.c_str(), "w");
		}else if(mode.compare("rw")==0)
		{
			fp=fopen(filename.c_str(), "rw");
		}
}


MyFile::~MyFile()
{
	fclose(fp);
}

string MyFile::readNextLine()
{
	char singleLine[100];
	for (;;) {
		char *lineRead = fgets(singleLine, sizeof singleLine, fp);
		if (lineRead != NULL) {
			if(*lineRead == '#' ) continue;
			return string(lineRead);
		} else
			return string("");
	}
}

bool MyFile::writeLine(string singleLine)
{
	int rv=fputs( ( singleLine+string("\n") ).c_str() ,fp);
	fflush(fp);

	if(rv==EOF)
		return false;
	return true;

}

void MyFile::logIT(const char *format, ...)
{	            va_list ap;
	    		// You will get an unused variable message here -- ignore it.
	    		va_start(ap, format);

	    		vfprintf(fp,format, ap);
	    		fputs("\n",fp);
	    		va_end(ap);
	    		fflush(fp);

}

//stays on same line
void MyFile::logIT_(const char *format, ...)
{
    			va_list ap;
	    		// You will get an unused variable message here -- ignore it.
	    		va_start(ap, format);

	    		vfprintf(fp,format, ap);
	    	//	fputs("\n",fp);
	    		va_end(ap);
	    		fflush(fp);



}

void MyFile::HexDump(const unsigned char *format, unsigned int len)
{
	fprintf(fp,"0x%02x",format[0]);
	    		for (unsigned int var = 1; var < len; ++var)
	    				{
	    					fprintf(fp,"%02x",format[var]);
	    				}
	    		fflush(fp);

}
string MyFile::getValueofThisKey(string key)
{
	return string("");
}
