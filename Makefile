tar: log.o main.o
	g++ -o a log.o main.o -g -Wall -pthread -Wl,-rpath,/usr/local/lib -L /usr/local/lib -lboost_log -lboost_system -lboost_thread -lboost_filesystem

log.o:log.cc log.h
	g++ -c log.cc -g -Wall -DBOOST_LOG_DYN_LINK -I /usr/local/include

main.o : main.cc
	g++ -c main.cc -g -Wall

clean:
	rm -rf log.o main.o a
