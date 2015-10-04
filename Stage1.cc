#include "general.h"

#include "common.h"

// some routine code snippets from beej

void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}


MyFile *configFile;
extern int stage;
extern int noOfRouters;
extern string  portNoOfProxy;
extern UDPSocket sockParent;


int main(int argc, char  *argv[])
{

	//test();
	sockParent.BindSocketForListening("0");
	portNoOfProxy = sockParent.PrintPort();

	if(argc < 2) {cout<<"\nPlz specify config file name which is present in the current directory\n";exit(1);}




	// portNoOfProxy = sockParent.PrintPort();

	//   Kill all zombie process-------------
	   struct sigaction sa;
		sa.sa_handler = sigchld_handler;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = SA_RESTART;
		if (sigaction(SIGCHLD, &sa, NULL) == -1)
		{
			perror("sigaction");
			exit(1);
		}

		configFile = new MyFile(argv[1],"r");
		string singleLine = configFile->readNextLine();
		char wordStage[10];
		sscanf(singleLine.c_str(),"%s %d",wordStage,&stage);
		printf("\nStage from file is %d\n",stage);
		singleLine = configFile->readNextLine();
		char routerWord[10];
		sscanf(singleLine.c_str(),"%s %d",routerWord,&noOfRouters);
		printf("\nnoOfRouters from file is %d\n",noOfRouters);
		MyDEBUG("Port number of proxy is %s",portNoOfProxy.c_str());

		if(stage > 4)
		{
			singleLine = configFile->readNextLine();
			char miniTORHopsWord[20];
			sscanf(singleLine.c_str(),"%s %d",miniTORHopsWord,&noOfHops);
		}


	for(int i=0;i<noOfRouters;i++)
		{
					pid_t pID = fork();
					if (pID == 0)
				   {
						Router(i+1);
				   }

		}

	Proxy();


//		MyFile* tf= new MyFile("Stage.txt","w");




exit(0);

}
