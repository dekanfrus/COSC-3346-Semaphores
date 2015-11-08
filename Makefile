.SUFFIXES: .h .c .cpp .l .o

osproj3: osproj3.cpp
	g++ -pthread -ggdb osproj3.cpp -o osproj3
clean:
	/bin/rm -f *.o *~ core osproj3
