all:
	g++ -O3 -s -std=c++11 cppblog.c -o cppblog.cgi
clean:
	rm -rf cppblog.cgi *.o
