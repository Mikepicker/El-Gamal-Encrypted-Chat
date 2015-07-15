/* -------------------------------------------------------------------------------------- */
/* ------------------------------------ENCRYPTED CHAT------------------------------------ */
/* -------------------------------------------------------------------------------------- */

/* Server / Client Chat based on "El Gamal" Encryption Scheme. Developed by Michele Rullo */
/* Extension of http://www.theinsanetechie.in/2014/01/a-simple-chat-program-in-c-tcp.html */

/* -------------------------------------------------------------------------------------- */
/* -------------------------------------CLIENT CODE-------------------------------------- */
/* -------------------------------------------------------------------------------------- */

#include"stdio.h"    
#include"stdlib.h"    
#include"sys/types.h"    
#include"sys/socket.h"    
#include"string.h"    
#include"netinet/in.h"    
#include"netdb.h"  
#include"pthread.h"  
    
#define PORT 4444   
#define BUF_SIZE 2000   
    
/* EL GAMAL PARAMS */
#define P 73	// Order
#define G 41	// Generator
#define PK 5	// Client Private Key

long server_public_key, client_public_key = 0;
long session_key = 0;
/*-----------------*/

// XOR based message encoder/decoder
static encode_decode(char* msg, long key)
{
	char *i = msg;
	while(*i)
	{
		*i ^= key;
		i += 1;
	}
}

static long fastExp(int base, int exp)
{
	long f = 1;
	long b = base;
	while(exp > 0)
	{
		int lsb = 0x1 & exp;
		exp >>= 1;
		if (lsb)
			f *= b;
		b *= b;
	}
	return f;
}

void exchangeParams(int sockfd, void * addr)
{
	int ret;
	char buffer[BUF_SIZE];
	memset(buffer, 0, BUF_SIZE); 

	// SEND P
	sprintf(buffer, "%d", P);
	ret = sendto(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *) &addr, sizeof(addr));    
	if (ret < 0) 
	{    
		printf("Error sending data!\n\t-%s", buffer);    
	}  

	// SEND G
	sprintf(buffer, "%d", G);
	ret = sendto(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *) &addr, sizeof(addr));    
	if (ret < 0) 
	{    
		printf("Error sending data!\n\t-%s", buffer);    
	}

	// SEND CLIENT PUBLIC KEY
	client_public_key = fastExp(G, PK) % P;
	sprintf(buffer, "%lu", client_public_key);
	ret = sendto(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *) &addr, sizeof(addr));    
	if (ret < 0) 
	{    
		printf("Error sending data!\n\t-%s", buffer);    
	}
}

void * receiveMessage(void * socket)
{  
	int sockfd, ret;  
	char buffer[BUF_SIZE];   
	sockfd = (int) socket;  
	memset(buffer, 0, BUF_SIZE);
	   
	for (;;)
	{  
		ret = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);    
		if (ret < 0)
		{    
			printf("Error receiving data!\n");      
		} 
		else 
		{  
			// RECEIVING SERVER PUBLIC KEY
			if (server_public_key == 0)
			{
				server_public_key = atoi(buffer);
				printf("Server Public Key:%lu\n", server_public_key);

				// COMPUTING SESSION KEY
				session_key = fastExp(server_public_key, PK) % P;
				printf("Session Key:%lu\n", session_key);
				fflush(stdout);
			}
			else
			{
				encode_decode(buffer, session_key); // Decode incoming message
				printf("server: ");  
				fputs(buffer, stdout);  
				//printf("\n");
			}  
		}    
	}  
}  

int main(int argc, char**argv) 
{    
	struct sockaddr_in addr, cl_addr;    
	int sockfd, ret;    
	char buffer[BUF_SIZE];   
	char * serverAddr;  
	pthread_t rThread;  

	if (argc < 2) 
	{  
		printf("usage: client < ip address >\n");  
		exit(1);    
	}  

	serverAddr = argv[1];   

	sockfd = socket(AF_INET, SOCK_STREAM, 0);    
	if (sockfd < 0) 
	{    
		printf("Error creating socket!\n");    
		exit(1);    
	}    
	printf("Socket created...\n");     

	memset(&addr, 0, sizeof(addr));    
	addr.sin_family = AF_INET;    
	addr.sin_addr.s_addr = inet_addr(serverAddr);  
	addr.sin_port = PORT;       

	ret = connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));    
	if (ret < 0) 
	{    
		printf("Error connecting to the server!\n");    
		exit(1);    
	}    
	printf("Connected to the server...\n");     

	//creating a new thread for receiving messages from the server  
	ret = pthread_create(&rThread, NULL, receiveMessage, (void *) sockfd);  
	if (ret) 
	{  
		printf("ERROR: Return Code from pthread_create() is %d\n", ret);  
		exit(1);  
	}  

	/* EL GAMAL PARAMS EXCHANGE */
	exchangeParams(sockfd, &addr);
	while(!session_key) {}
	/*--------------------------*/

	memset(buffer, 0, BUF_SIZE);  
	printf("Enter your messages one by one and press return key!\n"); 

	while (fgets(buffer, BUF_SIZE, stdin) != NULL) 
	{  
		encode_decode(buffer, session_key); // Encode outgoing message
		ret = sendto(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *) &addr, sizeof(addr));    
		if (ret < 0) 
		{    
			printf("Error sending data!\n\t-%s", buffer);    
		}  
	}  

	close(sockfd);  
	pthread_exit(NULL);  

	return 0;      
}    
