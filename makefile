ds:
	g++ ./dumpswitch/*.cpp ./socket/*.cpp ./thread/*.cpp -pthread -o dumpswitch.out

n:
	g++ ./node/*.cpp ./socket/*.cpp ./thread/*.cpp -pthread -o node.out

ls:
	g++ ./listenserver/*.cpp ./socket/*.cpp ./thread/*.cpp -pthread -o listenserver.out

ls2:
	g++ ./listenserver_old/*.cpp -pthread ./socket/*.cpp ./thread/*.cpp -pthread -o listenserver_old.out
    
tb:
	g++ testbed.cpp -o testbed
    
sw:
	g++ vswitch.cpp -o switch
