/* A P2P client
It provides the following functions:
- Register the content file to the index server (R)
- Contact the index server to search for a content file (D)
	- Contact the peer to download the file
	- Register the content file to the index server
- De-register a content file (T)
- List the on-line registered content files (O)
*/

#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
#include <fcntl.h>//for read/write/open
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

/* Not technically required, but needed on some UNIX distributions */
#include <sys/types.h>
#include <sys/stat.h>

#define SERVER "127.0.0.1"
#define SERVER_TCP_PORT 3000   /* well-known port */
#define BUFLEN 512  //Max length of buffer
#define READ 100

void die(char *s)
{
	perror(s);
	exit(1);
}
struct pdu{
	char type;
	char data[READ];
} sendData, recData;

char list_of_local_content[50][20];
int num_of_local_entries = 0;

int main(int argc, char **argv) {

	int service2;
	int sUDP, type, i;         	// Socket descriptor and socket type
	int sTCP;             	// Socket descriptor p2p client(s)
	int client_len;        	// Length of client addr
	int selfUDP_len = sizeof(selfUDP);
	int new_p2p_c;         	// Socket to connect to new p2p client
	struct hostent *hp;   	// Pointer to host info entry
	struct sockaddr_in selfUDP, selfTCP, client;	// An Internet endpoint address
	char *host = "localhost";  // Host to use (default=localhost)
	char *service1 = "10000";   // Service name or port number
	char cmd;
	char peer[20], content[20];

    
	switch(argc){
    	case 1:
        	host = "localhost";
        	break;
    	case 2:
        	host = argv[1];
        	break;
    	case 3:
        	service1 = argv[2];
        	break;
    	default:
        	fprintf(stderr, "Usage: %s host [port]\n", argv[0]);
        	exit(1);
	}

	// create UDP socket
	if ( (sUDP=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
    	die("UDP socket\n");
	}

	/* Configure UDP socket */
	memset(&selfUDP, 0, sizeof(selfUDP));
	selfUDP.sin_family = AF_INET;                      	// Set addr family (IPv4)
	selfUDP.sin_port = htons((u_short)atoi(service1)); 	// Map service name to port number
	if (hp = gethostbyname(host)) {
    	memcpy(&selfUDP.sin_addr, hp->h_addr, hp->h_length);
	}else if ( (selfUDP.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE){
    	die("Can't get host entry\n");
	}

	// create TCP socket
	if ((sTCP = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    	die("TCP socket\n");
	}

	/* Bind an address to the socket   */
	bzero((char *)&selfTCP, sizeof(struct sockaddr_in));
	selfTCP.sin_family = AF_INET;
	selfTCP.sin_port = htons(port);
	selfTCP.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sTCP, (struct sockaddr *)&selfTCP, sizeof(selfTCP)) == -1){
    	die("TCP bind\n");
	}
	// queue up to 5 connect requests
	listen(sTCP, 5);
	(void) signal(SIGCHLD, reaper);

	// Bind an address to the socket
	bzero((char *)&selfTCP, sizeof(struct sockaddr_in));
	selfTCP.sin_family = AF_INET;
	selfTCP.sin_port = htons(service2);
	selfTCP.sin_addr.s_addr = htonl(INADDR_ANY);

	// Create child process for content server execution
	if (fork() == 0){
    	while (1){
        	//Bind client to socket
        	client_len = sizeof(client);
        	new_p2p_c = accept(p2p_c, (struct sockaddr *)&client, &client_len);
        	if (new_p2p_c < 0){
            	die("Can't accept client \n");
        	}
        	switch(fork()){
            	case 0 :
                	(void) close(p2p_c);
                	exit(echod(new_p2p_c));
            	default:
                	(void) close(new_p2p_c);
                	break;
            	case -1:
            	die("fork()' error \n");
        	}
    	}
	}

	/* Asking for username */
	printf("Peer Name?: ");
	scanf("%s", peer);

	while (1){
  	printf("Enter your command. Enter '?' to see available options:");
  	scanf(" %c", &cmd);
 	 
  	//Print error if user selects wrong command
  	if(cmd != 'R' && cmd != 'S' && cmd != 'O' && cmd != 'T' && cmd != 'Q' && cmd != '?'){
    	die("Command selected doesn't exist \n");
    	continue;
  	}
  	//Display options
  	if (cmd == '?'){
     	printf("R - Register Content\n");
     	printf("S - Download Content\n");
     	printf("O - List Online Content\n");
     	printf("T - De-register Content\n");
     	printf("Q - Quit\n");
  	}

  	// Register content on Index Server
  	if (cmd == 'R'){    	 
     	printf("Please enter the name of the file to register:");
     	scanf("%s", content);
      	 
     	sendData.type = 'R';
     	strcpy(sendData.data, peer);
     	strcpy(sendData.data + 20, content);
     	bcopy(&selfTCP, sendData.data + 40, sizeof(selfTCP));

     	sendto(s, &sendData, 101, 0, NULL, 0);
     	recvfrom(s, &recData, 101, 0, (struct sockaddr *)&selfUDP, &selfUDP_len);
      	 
     	if (recData.type == 'E'){
        	die("File already exists on index server!\n");
     	} else if (recData.type == 'A'){
        	printf("Content has been registered on the server.\n");
        	strncpy(list_of_local_content[num_of_local_entries], content, 20);
        	num_of_local_entries++;
     	}
  	}
	//Searching the Content Server
  	if (cmd == 'S'){
    	 
     	sendData.type = 'S';
     	printf("Which file would you like to download?:");
     	scanf("%s", content);
     	strcpy(sendData.data, peer);
     	strcpy(sendData.data + 20, content);
     	sendto(s, &sendData, 101, 0, NULL, 0);
     	recvfrom(s, &recData, 101, 0, (struct sockaddr *)&selfUDP, &selfUDP_len);
     	if (recData.type == 'S'){
        	// Send TCP connection request to content server
        	struct sockaddr_in cs;    
        	int p2p_cs;          	 
          	 
        	if ((p2p_cs = socket(AF_INET, SOCK_STREAM, 0)) == -1){
           	fprintf(stderr, "Can't create socket\n");
           	exit(1);
        	}
        	bzero((char*)&cs, sizeof(struct sockaddr_in));
        	bcopy(recData.data, (char *)&cs, sizeof(cs));
          	 
        	if (connect(p2p_cs, (struct sockaddr *)&cs, sizeof(cs)) == -1){
           	fprintf(stderr, "Can't connect to socket\n");
           	exit(1);
        	}
        	// Send PDU to request for file content
        	int n;       	 
        	int fd = open(content, O_CREAT|O_WRONLY, S_IRUSR, S_IWUSR);
          	 
        	sendData.type = 'D';
        	strcpy(sendData.data, content);
        	send(p2p_cs, &sendData, 101, 0);
        	while (n = recv(p2p_cs, &recData, 101, 0)){
           	if (recData.type == 'C'){
              	write(fd, recData.data, n);
              	memset(recData.data, 0,  READ);
           	} else if(recData.type = 'E'){
              	printf("%s", recData.data);
              	break;
           	}
        	}
        	close(fd);
        	close(p2p_cs);
        	printf("File Successfully Downloaded!\n");
        	// Re-register file content with current peer
        	sendData.type = 'R';
        	strcpy(sendData.data, peer);
        	strcpy(sendData.data + 20, content);
        	bcopy(&selfTCP, sendData.data + 40, sizeof(selfTCP));
        	sendto(s, &sendData, 101, 0, NULL, 0);
        	recvfrom(s, &recData, 101, 0, (struct sockaddr *)&selfUDP, &selfUDP_len);
        	if (recData.type == 'E'){
           	printf("File already registered from this peer... somehow\n");
        	}else if (recData.type == 'A'){
           	printf("Registration complete.\n");
           	strncpy(list_of_local_content[num_of_local_entries], content, 20);
           	num_of_local_entries++;
        	}
     	} else if (recData.type == 'E'){
        	printf("No such content is registered on the index server\n");
     	}
  	}

  	/* List On-line Registered contents ----------- */
  	if (cmd == 'O'){
     	sendData.type = 'O';
     	sendto(s, &sendData, 101, 0, NULL, 0);    
     	printf("Available Online file contents:\n");
        	recvfrom(s, &recData, 101, 0, (struct sockaddr *)&selfUDP, &selfUDP_len);
        	if(recData.type == 'O'){
           	printf("%s", recData.data);   
        	} else if (recData.type == 'A'){
           	printf("\n");
           	continue;
        	}
  	}

  	// Content De-Registration
  	if (cmd == 'T'){
     	printf("File to de-register:");
     	scanf("%s", content);
     	sendData.type = 'T';    	 
     	strcpy(sendData.data, peer);
     	strcpy(sendData.data + 20, content);    	 
     	sendto(s, &sendData, 101, 0, NULL, 0);
     	recvfrom(s, &recData, 101, 0, (struct sockaddr *)&selfUDP, &selfUDP_len);

     	if (recData.type == 'E'){
        	die("Data does not exist on server.\n");
     	} else if (recData.type =='A'){
        	printf("Success! The content has been de-registered.\n");
        	for(i = 0; i < 50; i++){
           	if (strcmp(list_of_local_content[i], content) == 0){
              	strcpy(list_of_local_content[i], "");
              	break;
           	}
        	}
     	}
  	}
  	//User wants to quit, de-register all content
  	if (cmd == 'Q'){
     	int i;
     	sendData.type = 'T';
     	strcpy(sendData.data, peer);
     	for (i = 0; i < 50; i++){
        	if (strcmp(list_of_local_content[i], "") != 0){
           	strcpy(sendData.data + 20, list_of_local_content[i]);
           	sendto(s, &sendData, 101, 0, NULL, 0);
           	recvfrom(s, &recData, 101, 0, (struct sockaddr *)&selfUDP, &selfUDP_len);
           	if (recData.type == 'E'){
            	die("Data does not exist on server.\n", recData.data);
           	} else if (recData.type == 'A'){
              	printf("Success! The content has been de-registered.\n");
           	}
        	}
        	memset(sendData.data + 20, 0, 20);
     	}
     	printf("Disconnecting. Goodbye!\n");
     	return(0);
  	}

  	memset(sendData.data, 0, READ);
  	memset(recData.data, 0, READ);
   }
}   

/* reaper */
void reaper (int sig){
	int status;
	while(wait3(&status, WNOHANG, (struct rusage *)0) >= 0);
}

int echod (int sd){
	char buf[READ];
	int n, bytes_to_read;

	recv(sd, &recData, READ, 0);
	int fd = open(recData.data, O_RDONLY);
	if (fd > 0){
    	while(n = read(fd, buf, READ)){
        	sendData.type = 'C';
        	strncpy(sendData.data, buf, n);
        	send(sd, &sendData, n + 1, 0);
        	memset(sendData.data, 0, READ);
    	}
	} else {
    	sendData.type = 'E';
    	strcpy(sendData.data, "File unavailable.\n");
    	send(sd, &sendData, 101, 0);
	}
	close(fd);
	close(sd);
	return (0);
}