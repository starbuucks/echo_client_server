all : echo_server echo_client

echo_server: echo_server.o pcap_handle.o
	g++ -g -o echo_server echo_server.o pcap_handle.o -lpthread

echo_client: echo_client.o pcap_handle.o
	g++ -g -o echo_client echo_client.o pcap_handle.o -lpthread

echo_server.o: echo_server.cpp pcap_handle.h
	g++ -g -c -o echo_server.o echo_server.cpp

echo_client.o: echo_client.cpp pcap_handle.h
	g++ -g -c -o echo_client.o echo_client.cpp

pcap_handle.o: pcap_handle.cpp
	g++ -g -c -o pcap_handle.o pcap_handle.cpp

clean:
	rm -f echo_server echo_client
	rm -f *.o


