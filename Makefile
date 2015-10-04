all : projb

projb : Stage1.cc proxy.cc router.cc File.cc
	gcc -o aes_jh.o aes_jh.c -g -c 
	g++ -Wall *.c* aes_jh.o -o projb -g -lcrypto
	
test: MyDEBUG.cc Test/main.cc
	g++ -Wall *.c* -o test -g
	
clean: 
	rm projb