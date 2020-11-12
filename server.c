/*Index Server 

Message types:
R - used for registration
A - used by the server to acknowledge the success of registration
T - used by chat users for de-registration
S - Search content
O - List content
L - Location of the content server peer
E - Error messages from the Server
*/

#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include <fcntl.h>//for read/write/open
#include <unistd.h>
/* Not technically required, but needed on some UNIX distributions */
#include <sys/types.h>
#include <sys/stat.h>
#include<sys/socket.h>

#define BUFLEN 512  //Max length of buffer
#define READ 100
#define PASS 1
#define FAIL 0

void die(char *s){
	perror(s);
	exit(1);
}

struct pdu{
	char type;
	char data[READ];
}sendData, recData;

struct entry{
	char usr_name[20];
	char content_name[20];
	struct sockaddr addr;
};

int main (int argc, char *argv[]){
	struct sockaddr_in sin;   // An Internet endpoint address
	struct entry list[200];
	char *service = "10000";   // Service name or port number
	char name[20], content[20], address[60];
	int alen, i, n, check;       	// Length of address and counters
	int num_of_entries = 0;
	char buf[READ];        	// "Input" buffer for data handle
	struct sockaddr_in fsin;  // The 'from' address of a client

	//create a UDP socket
	if ((socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    	die("socket");
	}

	// zero out the structure
	memset((char *) &sin, 0, sizeof(sin));
	//contains a code for the address family
	sin.sin_family = AF_INET;
	//instead of simply copying the port number to this field, it is necessary to convert this to network byte order using the function htons()
	sin.sin_port = htons(PORT);
	//IP address of the machine on which the server is running
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	//bind socket to port
	if( bind(s , (struct sockaddr*)&sin, sizeof(sin) ) == -1)  {
    	die("bind");
	}
	printf("Socket connected.\n");

	for(i = 0; i < 200; i++){
    	memset(list[i].usr_name, 0, sizeof(name));
    	memset(list[i].content_name, 0, sizeof(content));
    	memset(&list[i].addr, 0, sizeof(struct sockaddr));
	}

	while (1) {
    	check = FALSE;
    	alen = sizeof(fsin);
    	recvfrom(s, &recData, 101, 0, (struct sockaddr *)&fsin, &alen);
    	strncpy(name, recData.data, 20);
    	strncpy(content, recData.data + 20, 20);
    	strncpy(address, recData.data + 40, 60);

	//If the index server receives an error
	if (recData.type == 'E')
	{
    	strcpy(sendData.data,"ERROR recieving data");
    	sendto(s, &sendData, strlen(sendData.data)+1, 0, (struct sockaddr*) &si_other, slen);
    	die("recvfrom()");
    	return -1;
	}
	//Server receives request for content registration
 	else if (recData.type == 'R') {
    	for(int i = 0; i< sizeof(list);i++) {
        	//check if content name already exist
        	if (strcmp(list[i].usr_name, name) == 0 &&
            	strcmp(list[i].content_name, content) == 0) {
            	check = TRUE;
            	break;
        	}
        	//if content name doesn't exist, register content
        	if (check == FALSE) {
            	struct entry new_entry;
            	strcpy(new_entry.usr_name, name);
            	strcpy(new_entry.content_name, content);
            	bcopy(recData.data + 40, &new_entry.addr, sizeof(sin));
            	list[num_of_entries] = new_entry;
            	num_of_entries++;
            	sendData.type = 'A';
            	printf("Registration Request Received. The following has been added...\n");
            	printf("Peer Name: %s\n", name);
            	printf("Associated Content: %s\n", content);
        	} else {
            	sendData.type = 'E';
        	}
        	sendto(s, &sendData, 101, 0, (struct sockaddr *) &fsin, alen);
    	}
    	//Search request
 	} else if (recData.type == 'S') {
    	struct entry latest_entry;
    	for(int i = 0; i< sizeof(list);i++) {
        	//Searching list for content name
        	if (strcmp(list[i].content_name, content) == 0){
            	check = TRUE;
            	latest_entry = list[i];
        	}
        	//If content exist on server, send the address back to client
        	if (check == TRUE){
            	sendData.type = 'S';
            	bcopy(&latest_entry.addr, sendData.data, sizeof(sin));
            	printf("Content Download Request Received. The server address for:\n");
            	printf("%s : %s has been sent\n", name, content);
        	} else {
            	sendData.type = 'E';
            	strcpy(sendData.data, "Search content not found. Please try again.\n");
        	}
        	sendto(s, &sendData, 101, 0, (struct sockaddr *)&fsin, alen);
    	}
	//Request to terminate content from client
 	} else if (recData.type == 'T') {
    	for (int i = 0; i < sizeof(list); i++) {
    	//searches the stored content to de-register the requested file
        	if (strcmp(list[i].usr_name, name) == 0 && strcmp(list[i].content_name, content) == 0){
            	memset(list[i].usr_name, 0, sizeof(name));
            	memset(list[i].content_name, 0, sizeof(content));
            	memset(&list[i].addr, 0, sizeof(struct sockaddr));
            	check = TRUE;
            	break;
        	}
    	}
    //If the server has the content, send an acknowledge to the client
    	if (check == TRUE){
        	printf("De-registring following files:\n");
        	printf("Content %s\n", content);
        	printf("Has been removed from Peer %s\n", name);
        	sendData.type = 'A';
    	}else{
    //else send error if content is not on the server
        	sendData.type = 'E';
    	}
    	sendto(s, &sendData, 101, 0, (struct sockaddr*)&fsin, alen);

	//user wants to list the content on the index server
	} else if (recData.type == 'O') {
    	sendData.type = 'O';
    	for(int i = 0; i< sizeof(list);i++) {
        	if (strcmp(list[i].usr_name, "") != 0){
            	memset (sendData.data, 0, READ);
            	strcat(sendData.data, list[i].content_name);
            	strcat(sendData.data, " is available on peer: ");
            	strcat(sendData.data, list[i].usr_name);
            	strcat(sendData.data, "\n");
            	sendto(s, &sendData, 101, 0, (struct sockaddr*)&fsin, alen);
        	}
    	}
    	sendData.type = 'A';
    	sendto(s, &sendData, 101, 0, (struct sockaddr*)&fsin, alen);

	}

 	memset (recData.data, 0, READ);
 	memset (sendData.data, 0, READ);

}
