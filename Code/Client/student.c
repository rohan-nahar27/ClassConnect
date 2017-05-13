/*-----------------------------------------------------------------------------------------------------------------*/ 
/*-----------------------------------------------------------------------------------------------------------------*/ 
/*-----------------------------------------------------------------------------------------------------------------*/ 
/*------------------------------------------CLASS CONNECT----------------------------------------------------------*/ 
/*-----------------------------------------------------------------------------------------------------------------*/ 
/*-----------------------------------------------------------------------------------------------------------------*/ 
/*-----------------------------------------------------------------------------------------------------------------*/ 


#include "errexit.c"
#include "connectTCP.c"
#include "connectsock.c"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>

#define BLEN 1024

struct studentCredential{
	int sId;
    	char pwd[1024];
}sCredential;

int  connectTCP(const char *host,const char *service);
int errexit(const char *format, ...);
void downloadFile(int csock);
void loginToPortal(int csock,char resBuffer[]);
void onlinetest(int csock);
void view_officehours(int csock);
void view_grades(int csock);

int main(int argc, char * argv[])
{
    	int csock;
    	char resBuffer[BLEN],reqBuffer[BLEN],sbuffer[BLEN];
    	char sinput;
    	char *host,*service,*chatName;
    	int pId;
    
    	//Storing and Validations for arguments
    	switch (argc)
    	{
		case 3:
			host = argv[1];              /* Assigning IP address */
			service = argv[2];
			break;
     	
		default:
			errexit("Usage: %s <Server IP> <Server Port>\n", argv[0]);
     	}	
    
    	csock = connectTCP(host,service);
    
    	memset(&reqBuffer[0],0,sizeof(reqBuffer));
    	strcpy(reqBuffer,"Student");
    	send(csock,reqBuffer,sizeof(reqBuffer),0);
    	
    	memset(&reqBuffer[0],0,sizeof(reqBuffer));
    
    	memset(&resBuffer[0],0,sizeof(resBuffer));
    	recv(csock,resBuffer,sizeof(resBuffer),0);
    	loginToPortal(csock,resBuffer);
    
    	if(strcmp(resBuffer, "1")==0)
	{
        while(1)
		{
		printf("\n\nOptions available:\n\na. View Office Hours\nb. Take online exam\nc. Download the reference materials\nd. View Grades\n");
        	printf("Please select your option : ");
        	scanf(" %c%*c",&sinput);
        
        	switch(sinput)
			{
            		case 'a':
                		view_officehours(csock);
                		break;
            		case 'b':
                		onlinetest(csock);
                		break;
            		case 'c':
                		downloadFile(csock);
                		break;

			case 'd':
				view_grades(csock);
				break;
            		
            		default:
                	printf("Invalid user Input\n");
                	break;
        		}
		}
    	}	
    	else if(strcmp(resBuffer, "2")==0)
	{
        
        	errexit("Your password is invalid: %s\n",strerror(errno));
        
    	}
    	else if(strcmp(resBuffer, "3")==0)	
	{
        
        errexit("Your student id is invalid: %s\n",strerror(errno));
    	}	
    
    	close(csock);
    
    	return 0;
}

/*-----------------------------------------------------------------------------------------------------------------*/ 
/*------------------------------------------Authenticate with Server-----------------------------------------------*/ 
/*-----------------------------------------------------------------------------------------------------------------*/

void loginToPortal(int csock,char resBuffer[])
{
	printf("%s",resBuffer);
    	scanf("%d",&sCredential.sId);
    
    	strncpy(sCredential.pwd, getpass("Password: "), 1024);
    	//scanf("%s",sCredential.pwd);
    	send(csock,(void *)&sCredential,sizeof(sCredential),0);
    
    	memset(&resBuffer[0],0,BLEN);
    	recv(csock,resBuffer,BLEN,0);
}

/*-----------------------------------------------------------------------------------------------------------------*/ 
/*------------------------------------------View Office Hours------------------------------------------------------*/ 
/*-----------------------------------------------------------------------------------------------------------------*/

void  view_officehours(int csock)
{
    	char buffer[2048],reqBuffer[BLEN];
    
   	memset(&reqBuffer[0], 0, BLEN);
	strcpy(reqBuffer,"a");
    	send(csock,reqBuffer,strlen(reqBuffer),0);
    
    	memset(&reqBuffer[0], 0, BLEN);
    	memset(&buffer[0],0,sizeof(buffer));
    	recv(csock,buffer,sizeof(buffer),0);
    	printf("\n%s",buffer);
}

/*-----------------------------------------------------------------------------------------------------------------*/ 
/*------------------------------------------View Grades-----------------------------------------------------------*/ 
/*-----------------------------------------------------------------------------------------------------------------*/

void  view_grades(int csock)
{
    	char buffer[2048],reqBuffer[BLEN];
    
   	memset(&reqBuffer[0], 0, BLEN);
	strcpy(reqBuffer,"d");
    	send(csock,reqBuffer,strlen(reqBuffer),0);
   	memset(&reqBuffer[0], 0, BLEN);
    	memset(&buffer[0],0,sizeof(buffer));
    	recv(csock,buffer,sizeof(buffer),0);
    	printf("\nYour Grade: %s",buffer);
	memset(&buffer[0],0,sizeof(buffer));
}

/*-----------------------------------------------------------------------------------------------------------------*/ 
/*------------------------------------------Download Files from Server---------------------------------------------*/ 
/*-----------------------------------------------------------------------------------------------------------------*/

void downloadFile(int csock)
{
    	char reqBuffer[BLEN],resBuffer[BLEN],path[100];
    	long fr_block_sz;
    	int j,i,flag=0;
    	char **fNames=NULL;
    	memset(&reqBuffer[0], 0, BLEN);
   	strcpy(reqBuffer,"c");
    	send(csock,reqBuffer,strlen(reqBuffer),0);
    	memset(&reqBuffer[0], 0, BLEN);
    
    	memset(&resBuffer[0], 0, BLEN);
    	recv(csock,resBuffer,sizeof(resBuffer),0);
    	int count = atoi(resBuffer);
    	memset(&resBuffer[0], 0, BLEN);
    	
    	printf("\n\nFiles on Server:\n\n");
    	for(j=0;j<count;j++)
    	{
        	recv(csock, resBuffer, sizeof(resBuffer), 0);
        	printf("%s\n",resBuffer);
        	fNames = (char **)realloc(fNames, (j + 1) * sizeof(char *));
        	fNames[j] = strdup(resBuffer);
        	memset(&resBuffer[0],0,sizeof(resBuffer));
    	}	
    	printf("\nType the filename to be dowanloaded: ");
    	scanf("%s",reqBuffer);
    
    	for(i = 0; i < j; i++)
	{
        
        	if(strcmp(reqBuffer,fNames[i])==0)
		{
            		flag=1;
            		break;
        	}	
    	}
    
    	for(i = 1; i < j; i++)
    	{
        	free(fNames[i]);
    	}
    	free(fNames);
    	if(flag==1)
	{
        //Send the filename to the server
        send(csock,reqBuffer,strlen(reqBuffer),0);
        
        memset(&path[0],0,sizeof(path));
        strcpy(path,".");
        strcat(path,"/Files/Student/Downloads/");
        strcat(path,reqBuffer);
        
        
        FILE *fr = fopen(path, "a");
        
        if(fr==NULL)
        {
        	errexit("Filename '%s' Incorrect: %s\n",strerror(errno));
        }
        else
        {
            	memset(&resBuffer[0], 0, sizeof(resBuffer));
            	while((fr_block_sz = recv(csock, resBuffer, BLEN, 0))>0)
            	{
                	size_t write_sz = fwrite(resBuffer, sizeof(char), fr_block_sz, fr);
                	if(write_sz < fr_block_sz)
                	{
                    		errexit("Failed while writing file on client: %s\n",strerror(errno));
                	}
                	memset(&resBuffer[0], 0, sizeof(resBuffer));
                	if (fr_block_sz == 0 || fr_block_sz != BLEN)
                	{
                    		break;
                	}
            	}
            
            	if(fr_block_sz<0)
		{
                	errexit("Error while receiving: %s\n",strerror(errno));
            	}	
            
            	fclose(fr);
        }
        memset(&reqBuffer[0], 0, BLEN);
        memset(&path[0],0,sizeof(path));
        printf("Download Complete\n");
    }
    else
	{
        	printf("Filename Invalid: \n");
    	}
}

/*-----------------------------------------------------------------------------------------------------------------*/ 
/*---------------------------------------------Take online Test----------------------------------------------------*/ 
/*-----------------------------------------------------------------------------------------------------------------*/

void onlinetest(int csock)
{
    	int i_dir=0,j_dir=0,marks;
    	char buffer1[2048],buffer2[2048],buffer3[2048],reqBuffer[BLEN];
    
    	memset(&reqBuffer[0], 0, BLEN);
    	strcpy(reqBuffer,"b");
    	send(csock,reqBuffer,strlen(reqBuffer),0);

    	memset(&reqBuffer[0], 0, BLEN);
    
    	memset(&buffer2[0],0,sizeof(buffer2));
    	recv(csock,buffer2,sizeof(buffer2),0);

    	printf("%s\n",buffer2);
    
    	memset(&buffer2[0],0,sizeof(buffer2));
    	recv(csock,buffer2,sizeof(buffer2),0);

    	j_dir=atoi(buffer2);
    
    	for(i_dir=0;i_dir<j_dir;i_dir++)
    	{
        	recv(csock,buffer3,sizeof(buffer3),0);
        	printf("\n%s",buffer3);

        	memset(&buffer3[0],0,sizeof(buffer3));
    	}
    
    	printf("\n\nEnter test name: (Ex: Quiz1.txt) : ");
    	scanf("%s",buffer1);
    	send(csock,buffer1,sizeof(buffer1),0);
    	//Get the questions from server
    	memset(&buffer2[0],0,sizeof(buffer2));
    	recv(csock,buffer2,sizeof(buffer2),0);
    
    	printf("\n%s",buffer2);
    
    	//Answers the respective questions
    	memset(&buffer1[0],0,sizeof(buffer1));
    	printf("\n\nEnter your answers in a continuous form :");
    	scanf("%s",buffer1);
    	send(csock,buffer1,sizeof(buffer1),0);
    
	//Evaluate the score
    	memset(&buffer2[0],0,sizeof(buffer2));
    	recv(csock,buffer2,sizeof(buffer2),0);
    	marks=atoi(buffer2);
    	printf("\nMarks: %d\n",marks);
}

