#include <pcap.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <map>
#include <arpa/inet.h>
#include <netinet/ether.h>
#include "pcap_handle.h"

using namespace std;

void print_MAC(const char* label, MAC mac){
	printf("%s : %02X:%02X:%02X:%02X:%02X:%02X\n", label,
		mac.i[0], mac.i[1], mac.i[2], mac.i[3], mac.i[4], mac.i[5]);
}

void print_IP(const char* label, uint32_t ip){
	printf("%s : %d.%d.%d.%d\n", label,
		(ip & 0xFF000000) >> 24,
		(ip & 0x00FF0000) >> 16,
		(ip & 0x0000FF00) >> 8,
		(ip & 0x000000FF));
}

void str_to_ip(char* ip_str, uint32_t* out){
	int i, st;
	int j = -1;
	uint8_t ip_arr[4];
	for(i = 0; i < 4; i++){
		st = ++j;
		for(; ip_str[j] != '.' && ip_str[j] != '\x00'; j++);
		ip_str[j] = '\x00';
		ip_arr[3 - i] = atoi(ip_str + st);
	}
	memcpy(out, ip_arr, 4);
}

void print_packet(const char* des, const u_char* packet, int len){
	printf("\n[%s] packet", des);
	for(int i = 0; i < len; i++){
		if(i % 16 == 0) printf("\n");
		printf("%02x ", *(packet + i));
	}
	printf("\n");
}
