server:./Server.h ./Server.h ./ImageRpc.cpp ./ImageRpc.h 
	g++ -I . -c ./ImageRpc.cpp ./ImageRpc.h ./Server.cpp ./Server.h -std=c++11
	g++ -o server ./ImageRpc.o ./Server.o -lIce -lIceUtil -lpthread -llog4cplus


client:./Client.cpp ./ImageRpc.cpp ./ImageRpc.h  
	g++ -I . -c Client.cpp ImageRpc.cpp ImageRpc.h -std=c++11
	g++ -o client ./Client.o ./ImageRpc.o -lIce -lIceUtil -lpthread -llog4cplus

clean:
	rm *.gch
	rm *.o
