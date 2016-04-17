all:
	g++ Mutex.cpp -lpthread -c -o Mutex.o
	g++ Task.cpp -lpthread -c -o Task.o
	g++ ThreadPool.cpp -lpthread -c -o ThreadPool.o
	g++ Mutex.o Task.o ThreadPool.o threadpool_test.cpp -lpthread -o threadpool_test

clean:
	rm *.o
	rm ./threadpool_test
