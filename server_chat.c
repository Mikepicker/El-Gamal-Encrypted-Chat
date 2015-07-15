/* -------------------------------------------------------------------------------------- */
/* ------------------------------------ENCRYPTED CHAT------------------------------------ */
/* -------------------------------------------------------------------------------------- */

/* Server / Client Chat based on "El Gamal" Encryption Scheme. Developed by Michele Rullo */
/* Extension of http://www.theinsanetechie.in/2014/01/a-simple-chat-program-in-c-tcp.html */

/* -------------------------------------------------------------------------------------- */
/* -------------------------------------SERVER CODE-------------------------------------- */
/* -------------------------------------------------------------------------------------- */

#include"stdio.h"  
#include"stdlib.h"  
#include"sys/types.h"  
#include"sys/socket.h"  
#include"string.h"  
#include"netinet/in.h"  
#include"pthread.h"  
  
#define PORT 4444  
#define BUF_SIZE 2000  
#define CLADDR_LEN 100  
 
/* EL GAMAL PARAMS */
#define PK 7	// Server Private Key

long P,G,server_public_key,client_public_key = 0;
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
			/*---------------------------*/
			/* RECEIVING EL GAMAL PARAMS */
			/*---------------------------*/
			if (P == 0)
			{
				P = atoi(buffer);
				printf("P:%lu\n", P);
				fflush(stdout);
			}
			else if (G == 0)
			{
				G = atoi(buffer);
				printf("G:%lu\n", G);
				fflush(stdout);

				// COMPUTING SERVER PUBLIC KEY
				server_public_key = fastExp(G, PK) % P;
			}
			else if (client_public_key == 0)
			{
				client_public_key = atoi(buffer);
				printf("Client Public Key:%lu\n", client_public_key);

				// COMPUTING SESSION KEY
				session_key = fastExp(client_public_key, PK) % P;
				printf("Session Key:%lu\n", session_key);
				fflush(stdout);
			}
			/*---------------------------*/
			/*---------------------------*/
			/*---------------------------*/
			else
			{
				encode_decode(buffer, session_key); // Decode incoming message
				printf("client: ");  
				fputs(buffer, stdout); 
				fflush(stdout); 
				//printf("\n");
			}  
		}    
	}  
}  

void main()
{  
	struct sockaddr_in addr, cl_addr;  
	int sockfd, len, ret, newsockfd;  
	char buffer[BUF_SIZE];  
	pid_t childpid;  
	char clientAddr[CLADDR_LEN];  
	pthread_t rThread;  

	sockfd = socket(AF_INET, SOCK_STREAM, 0);  
	if (sockfd < 0)
	{  
		printf("Error creating socket!\n");  
		exit(1);  
	}  
	printf("Socket created...\n");  

	memset(&addr, 0, sizeof(addr));  
	addr.sin_family = AF_INET;  
	addr.sin_addr.s_addr = INADDR_ANY;  
	addr.sin_port = PORT;  

	ret = bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));  
	if (ret < 0) 
	{  
		printf("Error binding!\n");  
		exit(1);  
	}  
	printf("Binding done...\n");  

	printf("Waiting for a connection...\n");  
	listen(sockfd, 5);  


	len = sizeof(cl_addr);  
	newsockfd = accept(sockfd, (struct sockaddr *) &cl_addr, &len);  
	if (newsockfd < 0) 
	{  
		printf("Error accepting connection!\n");  
		exit(1);  
	}   

	inet_ntop(AF_INET, &(cl_addr.sin_addr), clientAddr, CLADDR_LEN);  
	printf("Connection accepted from %s...\n", clientAddr);  

	//creating a new thread for receiving messages from the client  
	ret = pthread_create(&rThread, NULL, receiveMessage, (void *) newsockfd);  
	if (ret) 
	{  
		printf("ERROR: Return Code from pthread_create() is %d\n", ret);  
		exit(1);  
	}  

	/* EL GAMAL PARAMS EXCHANGE */
	while(!session_key) {} // Loop until session key gets computed
	
	// SENDING SERVER PUBLIC KEY TO CLIENT
	printf("Sending Public Key to Client..\n");
	memset(buffer, 0, BUF_SIZE);
	sprintf(buffer, "%lu", server_public_key);
	ret = sendto(newsockfd, buffer, BUF_SIZE, 0, (struct sockaddr *) &cl_addr, len);    
	if (ret < 0) 
		printf("Error sending data!\n\t-%s", buffer);

	/*--------------------------*/

	///memset(buffer, 0, BUF_SIZE); 
	printf("Handshaking ended! Now you can communicate in a secure way.\n");
	printf("Enter your messages one by one and press return key!\n");

	while (fgets(buffer, BUF_SIZE, stdin) != NULL) 
	{  
		encode_decode(buffer, session_key); // Encode outgoing message
		ret = sendto(newsockfd, buffer, BUF_SIZE, 0, (struct sockaddr *) &cl_addr, len);    
		if (ret < 0) 
		{    
			printf("Error sending data!\n");    
			exit(1);  
		}  
	}     

	close(newsockfd);  
	close(sockfd);  

	pthread_exit(NULL);  
	return;  
}  
