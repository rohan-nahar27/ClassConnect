/*-----------------------------------------------------------------------------------------------------------------*/ 
/*-----------------------------------------------------------------------------------------------------------------*/ 
/*-----------------------------------------------------------------------------------------------------------------*/ 
/*------------------------------------------CLASS CONNECT----------------------------------------------------------*/ 
/*-----------------------------------------------------------------------------------------------------------------*/ 
/*-----------------------------------------------------------------------------------------------------------------*/ 
/*-----------------------------------------------------------------------------------------------------------------*/ 



#include<sys/socket.h>
#include<sys/types.h>
#include<stdio.h>
#include<netinet/in.h>
#include"errexit.c"
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdbool.h>
#include<dirent.h>
#include<string.h>
#include<stdarg.h>
#include<errno.h>
#include<pthread.h>

#define BLEN 1024

struct professorCredential
{
	char uName[1024];
	char pwd[1024];
}pCredential;

int  connectTCP(const char *host,const char *service);
int  errexit(const char *format, ...);
void upload_File(int sock);
void upload_grades(int sock);
void log_in(int sock,char resBuffer[]);
void upload_Test(int sock);
void upload_Questions(int sock);
void upload_Answers(int sock);
void insert_Student(int sock);
void updateOfficeHours(int sock);


int main(int argc, char * argv[])
{
	char resBuffer[BLEN],reqBuffer[BLEN];
	char uInput;
	char *host;
	struct sockaddr_in main_server, backup_server;
	int sock,y;
	unsigned short service;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1)
	{
		printf("Failed at socket creation\n");
		exit(1);
	}
	    
	switch (argc)
	{
		case 3:
			host = argv[1];              /* server IP address (dotted quad) */
			service =atoi(argv[2]);
			break;

		default:
			errexit("Usage: %s <Server IP> <Server Port>\n", argv[0]);
	}
    
	main_server.sin_addr.s_addr = inet_addr("127.0.0.1");
	main_server.sin_family = AF_INET;
	main_server.sin_port = htons(service);
    
	backup_server.sin_addr.s_addr = inet_addr("127.0.0.1");
	backup_server.sin_family = AF_INET;
	backup_server.sin_port = htons( 6666 );
    

	int s =connect(sock , (struct sockaddr *)&main_server , sizeof(main_server)) ;
    
	if(s<0)
	{
		printf("Server to Client Connection failed\n");
		int y =connect(sock , (struct sockaddr *)&backup_server , sizeof(backup_server)) ;
		if(y>=0) 
		{ 
			puts("\n\nConnected to Backup server");
		}
	}
	else if(s>=0)
	{
		puts("Connected to server\n");
	}
    
	if(s>=0 || y>=0 )
	{
		memset(&reqBuffer[0],0,sizeof(reqBuffer));
		strcpy(reqBuffer,"Professor");
		send(sock,reqBuffer,sizeof(reqBuffer),0);
		memset(&reqBuffer[0],0,sizeof(reqBuffer));
    
		memset(&resBuffer[0],0,sizeof(resBuffer));
		recv(sock,resBuffer,sizeof(resBuffer),0);
		log_in(sock,resBuffer);
    
		if(strcmp(resBuffer, "1")==0)
		{
			memset(&resBuffer[0],0,BLEN);
			while(1)
			{
				printf("\n\nMenu\na. Insert student data\nb. Update office hours\nc. Add new Exam\nd. Upload new document\ne.Upload Grades");
				printf("\nSelect: ");
				scanf(" %c%*c",&uInput);
        	
				switch(uInput)
				{
					case 'a':
						insert_Student(sock);
						break;
		
					case 'b':
						updateOfficeHours(sock);
						break;
		
					case 'c':
						upload_Test(sock);
						break;
		
					case 'd':
						upload_File(sock);
						break;

					case 'e':
						upload_grades(sock);
						break;
        		        
					default:
						printf("Invalid user Input");
						break;
				}
			}
		}
		else if(strcmp(resBuffer, "2")==0)
		{
			errexit("Invalid Password: %s\n",strerror(errno));
		}
		else if(strcmp(resBuffer, "3")==0)
		{
			errexit("Username Invalid: %s\n",strerror(errno));
		}
	}
	else
	{
		printf("Server is down");
	}
	close(sock);
	return 0;
}

/*-----------------------------------------------------------------------------------------------------------------*/ 
/*------------------------------------------LOGIN INFORMATION------------------------------------------------------*/ 
/*-----------------------------------------------------------------------------------------------------------------*/

void log_in(int sock,char resBuffer[])
{
    
	printf("%s",resBuffer);
	scanf("%s",pCredential.uName);
	strncpy(pCredential.pwd, getpass("Password: "), 1024);
	send(sock,(void *)&pCredential,sizeof(pCredential),0);
	memset(&resBuffer[0],0,BLEN);
	recv(sock,resBuffer,BLEN,0);

}

/*-----------------------------------------------------------------------------------------------------------------*/ 
/*------------------------------------------INSERT STUDENT DATA----------------------------------------------------*/ 
/*-----------------------------------------------------------------------------------------------------------------*/

void insert_Student(int sock)
{
	char reqBuffer[BLEN],resBuffer[BLEN];
	memset(&reqBuffer[0], 0, BLEN);
	strcpy(reqBuffer,"a");
	send(sock,reqBuffer,strlen(reqBuffer),0);
	memset(&reqBuffer[0], 0, BLEN);

	memset(&resBuffer[0], 0, BLEN);
	recv(sock,resBuffer,BLEN,0);
    
	printf("%s",resBuffer);
	scanf("%s",reqBuffer);
  

	send(sock,reqBuffer,sizeof(reqBuffer),0);
    
	memset(&resBuffer[0], 0, BLEN);
	recv(sock,resBuffer,BLEN,0);
	    
	printf("%s",resBuffer);
}

/*-----------------------------------------------------------------------------------------------------------------*/ 
/*------------------------------------------UPDATE OFFICE HOURS----------------------------------------------------*/ 
/*-----------------------------------------------------------------------------------------------------------------*/

void updateOfficeHours(int sock)
{
	char reqBuffer[BLEN],resBuffer[BLEN];
	char *day,*time;
	 
	day = (char *)malloc(100);
	time = (char *)malloc(100);
	memset(&reqBuffer[0], 0, BLEN);
	strcpy(reqBuffer,"b");
	send(sock,reqBuffer,strlen(reqBuffer),0);
	memset(&reqBuffer[0], 0, BLEN);
	 
	memset(&resBuffer[0], 0, BLEN);
	recv(sock,resBuffer,sizeof(resBuffer),0);
	  
	printf("%s",resBuffer);
	scanf("%s %s",day,time);
	    
	strcpy(reqBuffer,day);
	strcat(reqBuffer," ");
	strcat(reqBuffer,time);
	       
	send(sock,reqBuffer,strlen(reqBuffer),0);
	memset(&reqBuffer[0], 0, BLEN);
	memset(&resBuffer[0], 0, BLEN);
	    
	recv(sock,resBuffer,BLEN,0);
	    
	printf("%s",resBuffer);
}

/*-----------------------------------------------------------------------------------------------------------------*/ 
/*------------------------------------------UPLOAD Grades----------------------------------------------------------*/ 
/*-----------------------------------------------------------------------------------------------------------------*/

void upload_grades(int sock)
{
	char reqBuffer1[BLEN],resBuffer1[BLEN],count1[BLEN];
	char grades[1];
	int count;
	int blen=BLEN;
	
	memset(&reqBuffer1[0], 0, BLEN);
	strcpy(reqBuffer1,"e");
	send(sock,reqBuffer1,strlen(reqBuffer1),0);
	memset(&reqBuffer1[0], 0, BLEN);

	while(1)
	{
		perror("Inside for loop");
		memset(&resBuffer1[0], 0, BLEN);
		recv(sock,resBuffer1,BLEN,0);
		if(strcmp(resBuffer1, "0")!=0)
		{
			printf("\nStudent Id: ");		
			printf("%s",resBuffer1);
			memset(&resBuffer1[0], 0, BLEN);
			printf("\nEnter Grades: ");
			scanf("%s",grades);
			send(sock,grades,strlen(grades),0);
			bzero(&grades, 1);
			
		}
		else
		{
			break;
		}
	}
	printf("\nGrades sent successfully\n");

}
/*-----------------------------------------------------------------------------------------------------------------*/ 
/*------------------------------------------UPLOAD FILE------------------------------------------------------------*/ 
/*-----------------------------------------------------------------------------------------------------------------*/

void upload_File(int sock)
{
	char reqBuffer[BLEN],resBuffer[BLEN],path[100];
	size_t fs_block_sz;
	int blen=BLEN;
	
	memset(&reqBuffer[0], 0, BLEN);
	strcpy(reqBuffer,"d");
	send(sock,reqBuffer,strlen(reqBuffer),0);
	memset(&reqBuffer[0], 0, BLEN);
	  
	memset(&resBuffer[0], 0, BLEN);
	recv(sock,resBuffer,BLEN,0);
	   
	printf("%s",resBuffer);
	scanf("%s",reqBuffer);
	 

	send(sock,reqBuffer,sizeof(reqBuffer),0);
	    
	strcpy(path,".");
	strcat(path,"/Files/Professor/");
	strcat(path,reqBuffer);
	memset(&reqBuffer[0], 0, BLEN);
	memset(&resBuffer[0], 0, BLEN);
	    
	FILE *fs = fopen(path, "r");
	    
	if(fs == NULL)
	{
		errexit("File not found :%s\n",strerror(errno));
	}
	   
	while((fs_block_sz = fread(reqBuffer, sizeof(char), BLEN, fs))>1)
	{
		if(send(sock, reqBuffer, fs_block_sz, 0) < 0)
		{
			errexit("Failed to send file :%s\n",strerror(errno));
			break;
		}
		perror("Sending Document:");
		memset(&reqBuffer[0], 0, BLEN);
        
	}
    
	recv(sock, resBuffer, blen, 0);
	printf("%s\n",resBuffer);
	memset(&resBuffer[0], 0, BLEN);
}

/*-----------------------------------------------------------------------------------------------------------------*/ 
/*---------------------------------------------UPLOAD TEST---------------------------------------------------------*/ 
/*-----------------------------------------------------------------------------------------------------------------*/

void upload_Test(int sock)
{
	char reqBuffer[BLEN];
    
	memset(&reqBuffer[0], 0, BLEN);
	strcpy(reqBuffer,"c");
	send(sock,reqBuffer,strlen(reqBuffer),0);
	memset(&reqBuffer[0], 0, BLEN);
	    
	upload_Questions(sock);
}
	
/*-----------------------------------------------------------------------------------------------------------------*/ 
/*------------------------------------------UPLOAD QUESTIONS-------------------------------------------------------*/ 
/*-----------------------------------------------------------------------------------------------------------------*/

void upload_Questions(int sock)
{
    
	size_t fs_block_sz;
	char resBuffer[BLEN],reqBuffer[BLEN],path[100];
	int blen=BLEN;
	    
	    
	memset(&resBuffer[0], 0, BLEN);
	recv(sock,resBuffer,BLEN,0);
	    
	printf("%s",resBuffer);
	scanf("%s",reqBuffer);
	    
	send(sock,reqBuffer,sizeof(reqBuffer),0);
	    
	strcpy(path,".");
	strcat(path,"/Files/Professor/Test/Question/");
	strcat(path,reqBuffer);
	memset(&reqBuffer[0], 0, BLEN);
	memset(&resBuffer[0], 0, BLEN);
	printf("Open File\n");
	FILE *fs = fopen(path, "r");
    	
	if(fs == NULL)
	{
		errexit("File not found :%s\n",strerror(errno));
	}
    
	while((fs_block_sz = fread(reqBuffer, sizeof(char), BLEN, fs))>0)
	{
		if(send(sock, reqBuffer, fs_block_sz, 0) < 0)
		{
			errexit("Failed to send file :%s\n",strerror(errno));
			break;
		}
		perror("Sending Exam:");
		memset(&reqBuffer[0], 0, BLEN);
	}
    
	recv(sock, resBuffer, blen, 0);
	printf("%s",resBuffer);
	memset(&resBuffer[0], 0, BLEN);
    
	upload_Answers(sock);
}

/*-----------------------------------------------------------------------------------------------------------------*/ 
/*------------------------------------------UPLOAD ANSWERS---------------------------------------------------------*/ 
/*-----------------------------------------------------------------------------------------------------------------*/

void upload_Answers(int sock)
{

	size_t fs_block_sz;
	char resBuffer[BLEN],reqBuffer[BLEN],path[100];
	int blen=BLEN;
	scanf("%s",reqBuffer);
	send(sock,reqBuffer,sizeof(reqBuffer),0);

	strcpy(path,".");
	strcat(path,"/Files/Professor/Test/Answer/");
	strcat(path,reqBuffer);
	memset(&reqBuffer[0], 0, BLEN);
	memset(&resBuffer[0], 0, BLEN);
	    
	FILE *fs = fopen(path, "r");
	    
	if(fs == NULL)
	{
		errexit("File not found :%s\n",strerror(errno));
	}
	    
	while((fs_block_sz = fread(reqBuffer, sizeof(char), BLEN, fs))>0)
	{
		if(send(sock, reqBuffer, fs_block_sz, 0) < 0)
		{
			errexit("Failed to send file :%s\n",strerror(errno));
			break;
		}
		memset(&reqBuffer[0], 0, BLEN);
	}
    
	recv(sock, resBuffer, blen, 0);
	printf("%s",resBuffer);
	memset(&resBuffer[0], 0, BLEN);
}	


