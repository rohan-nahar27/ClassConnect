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
#include"passivesock.c"
#include"passiveTCP.c"
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
#define QLEN 32

int f_Count;

struct {
       pthread_mutex_t st_mutex;
       unsigned int st_concount;
       unsigned int st_contotal;
} statistics;

struct sockaddr_in server_addr;
 
struct studentLogin{
 
	pthread_mutex_t sL_mutex;    
	int * sId;
	char **pwd;
}sLogin;

struct studentGrades{
 
	int * sId;
	char **grade;
}sGrades;
 
struct professorLogin{
 
    char uName[1024];
    char uPwd[1024];
}pLogin;
 
struct studentCredential{
 
	int sId;
	char pwd[1024];
}sCredential;
 
struct professorCredential{
 
    char uName[1024];
    char pwd[1024];
}pCredential;

//Defining all functions
int  passiveTCP(const char *service, int qlen);
int  errexit(const char *format, ...);
void load_studentinfo();
void load_Professorinfo();
void load_gradesinfo();
void upload_file(int sockfd);
void upload_grades(int sockfd);
void upload_test(int sockfd);
void upload_answers(int sockfd);
void download_file(int sockfd);
void student_login(int sockfd);
void professor_login(int sockfd);
void insert_studentinfo(int sockfd);
void office_hours(int sockfd);
void evaluate_test(int sockfd);
void view_officehours(int sockfd);
void view_grades(int sockfd);
void prstats(void);
void *client_handler(void *);


int main(int argc, char * argv[])
{
	int master_sockfd,sockfd;
	struct sockaddr_in client_addr;
	unsigned int client_addrlen;
	char reqBuffer[BLEN],resBuffer[BLEN];
	char *service;
	int pId;
	pthread_t th, new_client;
	      
	pthread_attr_t ta;
	statistics.st_contotal = 0;
	       //Check for the input parameters
	switch(argc)
	{
		case 2:
			service = argv[1];
			break;

		default:
			errexit("Usage: %s <Server Port>\n", argv[0]);
	}
       //Initialize Threads and Mutex
	(void) pthread_attr_init(&ta);
	(void) pthread_attr_setdetachstate(&ta, PTHREAD_CREATE_DETACHED);
	(void) pthread_mutex_init(&statistics.st_mutex, 0);
	(void) pthread_mutex_init(&sLogin.sL_mutex, 0);
	
	load_Professorinfo();
	
	master_sockfd = passiveTCP(service,QLEN);
	   
	while(1)
	{
		//creating new socket
	while((sockfd = accept(master_sockfd,(struct sockaddr *)&client_addr, &client_addrlen)))
	{	
		puts("Connection accepted");
		if(pthread_create( &new_client , NULL ,  client_handler , (void*) &sockfd) < 0)
			errexit("pthread_create: %s\n", strerror(errno));
	
		(void) pthread_mutex_lock(&statistics.st_mutex);
		statistics.st_contotal++;
		(void) pthread_mutex_unlock(&statistics.st_mutex);
	          
		if (pthread_create(&th, &ta, (void * (*)(void *))prstats, 0) < 0)
			errexit("pthread_create(prstats): %s\n", strerror(errno));
	}
	        
	if(sockfd<0)
	{
		errexit("accept failed: %s\n", strerror(errno));
	}
	close(sockfd);
	}
	    
	return 0;

}

/*-----------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------CLIENT HANDLER---------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------*/

void *client_handler(void *socket_desc)
{
	int sockfd = *(int*)socket_desc;
	unsigned int client_addrlen;
	char reqBuffer[BLEN],resBuffer[BLEN];
	int pId;
	load_studentinfo();	
    
	memset(&reqBuffer[0],0,sizeof(reqBuffer));
	recv(sockfd,reqBuffer,sizeof(reqBuffer),0);
	perror("recv(client Info):");
	if(strcmp(reqBuffer,"Student")==0)
	{
		memset(&resBuffer[0],0,sizeof(resBuffer));
		strcpy(resBuffer,"\n\t\t\t\t\tCLASS CONNECT\n\t\t\t\t=============================\n\n\t\rSTUDENT ID :");
		send(sockfd,resBuffer,sizeof(resBuffer),0);
		memset(&resBuffer[0],0,sizeof(resBuffer));
		recv(sockfd,&sCredential,sizeof(sCredential),0);
		student_login(sockfd); // call student login function
          	memset(&reqBuffer[0], 0, sizeof(reqBuffer));
		while(recv(sockfd,reqBuffer,BLEN,0))
		{
		char uInput = reqBuffer[0];

        	switch(uInput)
        	{
                    
			case 'a':
				view_officehours(sockfd);
				break;
			
			case 'b':
				evaluate_test(sockfd);
				break;
	
			case 'c':
				download_file(sockfd);
				break;

			case 'd':
				view_grades(sockfd);
				break;
			
			default:
				printf("Invalid user Input\n");
				break;
		}
        	memset(&reqBuffer[0], 0, sizeof(reqBuffer));}
	}
        	
	else if(strcmp(reqBuffer,"Professor")==0)
	{
		memset(&resBuffer[0],0,sizeof(resBuffer));
		strcpy(resBuffer,"\n\t\t\t\tCLASS CONNECT\n\t\t\t\t=============================\n\n\t\rUSER NAME : ");
		send(sockfd,resBuffer,sizeof(resBuffer),0);
		memset(&resBuffer[0],0,sizeof(resBuffer));
		recv(sockfd,&pCredential,sizeof(pCredential),0);
		professor_login(sockfd); // call Professor login function
		memset(&reqBuffer[0], 0, sizeof(reqBuffer));
		while(recv(sockfd,reqBuffer,BLEN,0))
		{
		char uInput = reqBuffer[0];
            
		switch(uInput)
		{
			case 'a':
				insert_studentinfo(sockfd);
                		break;

			case 'b':
				office_hours(sockfd);
				break;

			case 'c':
				upload_test(sockfd);
				break;

			case 'd':
				upload_file(sockfd);
				break;

			case 'e':
				upload_grades(sockfd);
				break;
			
			default:
				printf("Invalid user Input\n");
				break;
		}
		
		memset(&reqBuffer[0], 0, sizeof(reqBuffer));}
	}
}


/*-----------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------STUDENT INFO-----------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------*/

void load_studentinfo()
{
    int size=0;
    char password[5120];
   

    FILE *fp = fopen("./Files/Login/students.txt", "r");
    
    if(fp == NULL)
    {
        printf("File not found\n");
    }
    
    while (!feof(fp))
    {
        memset(&password[0],0,sizeof(password));
	(void) pthread_mutex_lock(&sLogin.sL_mutex);        
	sLogin.sId=(int *)realloc(sLogin.sId,(size + 1) * sizeof(int));
        sLogin.pwd = (char **)realloc(sLogin.pwd, (size + 1) * sizeof(char *));
        
        fscanf(fp, "%d,%s", &sLogin.sId[size],password);
//printf("%d \t %s", sLogin.sId[size],password);        
        sLogin.pwd[size]=strdup(password);
	printf("Id: %d \t Password: %s", sLogin.sId[size],sLogin.pwd[size]);
        size++;
	(void) pthread_mutex_unlock(&sLogin.sL_mutex);
    }
	fclose(fp);
	printf("Count: %d",size);	
	
    f_Count=size;
	
}

/*-----------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------Grades INFO-----------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------*/

void load_gradesinfo()
{
    int sz=0;
    char grades[1024];
   

    FILE *fr = fopen("./Files/Grades/grades.txt", "r");
    
    if(fr == NULL)
    {
        printf("File not found\n");
    }
    
    while (!feof(fr))
    {
	perror("in loop");        
	memset(&grades[0],0,sizeof(grades));
	sGrades.sId=(int *)realloc(sGrades.sId,(sz + 1) * sizeof(int));
        sGrades.grade = (char **)realloc(sGrades.grade, (sz + 1) * sizeof(char *));
        
        fscanf(fr, "%d,%s", &sGrades.sId[sz],grades);
	sGrades.grade[sz]=strdup(grades);
	sz++;
    }
fclose(fr);
}
        


/*-----------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------PROFESSOR INFO-----------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------*/

void load_Professorinfo()
{
	char *full_file_string;
	FILE *fp = fopen("./Files/Login/professor.txt", "r");
	if(fp == NULL)
	{
		printf("File not found\n");
	}
	full_file_string = (char *)malloc(100);
	fgets(full_file_string, BLEN, fp);
	strcpy(pLogin.uName ,strtok(full_file_string, ","));
	strcpy(pLogin.uPwd ,strtok(NULL, ","));
	fclose(fp);
	free(full_file_string);
}

/*-----------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------ADD STUDENT------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------*/

void insert_studentinfo(int sockfd)
{
	char resBuffer[BLEN], reqBuffer[BLEN];
    
	memset(&resBuffer[0], 0, BLEN);
	strcpy(resBuffer,"Please enter the studentID : ");
	send(sockfd, resBuffer,strlen(resBuffer),0);
	memset(&resBuffer[0], 0, BLEN);
    
	memset(&reqBuffer[0], 0, BLEN);
	recv(sockfd, reqBuffer,sizeof(reqBuffer),0);
    
	FILE *fp = fopen("./Files/Login/students.txt", "a");
    
	if(fp == NULL)
	{
		printf("File not found\n");
	}
    
	fprintf(fp, " %s,%s\n",reqBuffer,reqBuffer );
	fclose(fp);
    
	memset(&reqBuffer[0], 0, BLEN);
	memset(&resBuffer[0], 0, BLEN);
	strcpy(resBuffer,"Data updated successfully\n");
	send(sockfd, resBuffer,strlen(resBuffer),0);
	memset(&resBuffer[0], 0, BLEN);
}


/*-----------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------UPDATE OFFCIE HOURS--------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------*/

void office_hours(int sockfd)
{
	char resBuffer[BLEN],reqBuffer[BLEN];
    
	memset(&resBuffer[0], 0, BLEN);
	strcpy(resBuffer,"Please enter the office hours: (Ex: Friday 10:00AM)\n");
	send(sockfd, resBuffer,strlen(resBuffer),0);
	memset(&resBuffer[0], 0, BLEN);
	    
	memset(&reqBuffer[0], 0, BLEN);
	recv(sockfd,reqBuffer,sizeof(reqBuffer),0);
    
	FILE *fp = fopen("./Files/officehours/office.txt", "w");
    
	if(fp == NULL)
	{
		printf("File not found\n");
	}
    
	fprintf(fp, " %s\n",reqBuffer);
	fclose(fp);
  
	memset(&reqBuffer[0], 0, BLEN);
	memset(&resBuffer[0], 0, BLEN);
	strcpy(resBuffer,"Data updated successfully\n");
	send(sockfd, resBuffer,strlen(resBuffer),0);
	memset(&resBuffer[0], 0, BLEN);
}

/*-----------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------VALIDATE STUDENT-------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------*/

void student_login(int sockfd)
{

	int flag=0,i;
	char resBuffer[BLEN];
	memset(&resBuffer[0],0,sizeof(resBuffer));
	for (i=0;i<f_Count;i++)
	{
		printf("Client: %d \t %s",sCredential.sId, sCredential.pwd);
		if(sCredential.sId==sLogin.sId[i])
		{
			flag=1;
			if(strcmp(sCredential.pwd,sLogin.pwd[i])==0)
			{
				flag=2;	
				memset(&resBuffer[0], 0, BLEN);
				strcpy(resBuffer,"1");
				break;
			}
			else
			{
				memset(&resBuffer[0], 0, BLEN);
				strcpy(resBuffer,"2");
				break;
			}
		}
	}
    
	if(flag==0)
	{
		memset(&resBuffer[0], 0, BLEN);
		strcpy(resBuffer,"3");
	}
        
	send(sockfd, resBuffer,strlen(resBuffer),0);
	memset(&resBuffer[0],0,sizeof(resBuffer));
}

/*-----------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------VALIDATE PROFESSOR-----------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------*/

void professor_login(int sockfd)
{
	char resBuffer[BLEN];
	memset(&resBuffer[0],0,sizeof(resBuffer));
	if(strcmp(pLogin.uName,pCredential.uName)==0)
	{
		if(strcmp(pLogin.uPwd,pCredential.pwd)==0)
		{
			strcpy(resBuffer,"1");
		}
		else
		{
			strcpy(resBuffer,"2");
		}
	}
	else
	{
		strcpy(resBuffer,"3");
	}
	send(sockfd, resBuffer,strlen(resBuffer),0);
	memset(&resBuffer[0],0,sizeof(resBuffer));
}


/*-----------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------UPLOAD FILE---------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------*/

void upload_file(int sockfd)
{
	char reqBuffer[BLEN],resBuffer[BLEN];
	long fr_block_sz;
    
	memset(&resBuffer[0], 0, BLEN);
	strcpy(resBuffer,"Please enter the filename to be uploaded\n");
	send(sockfd, resBuffer,strlen(resBuffer),0);
	memset(&resBuffer[0], 0, BLEN);
    
	memset(&reqBuffer[0], 0, BLEN);
	recv(sockfd,reqBuffer,sizeof(reqBuffer),0);
	char path[100];
	strcpy(path,".");
	strcat(path,"/Files/Uploads/");
	strcat(path,reqBuffer);
	memset(&reqBuffer[0], 0, sizeof(reqBuffer));
    
	FILE *fr = fopen(path, "a");
	if(fr==NULL)
	{
		printf("File cant be opened \n");
	}
	else
	{
		while((fr_block_sz = recv(sockfd, reqBuffer, BLEN, 0))>1)
		{
			size_t write_sz = fwrite(reqBuffer, sizeof(char), fr_block_sz, fr);
			if(write_sz < fr_block_sz)
			{
				printf("File write failed \n");
			}
			memset(&reqBuffer[0], 0, sizeof(reqBuffer));
			if (fr_block_sz == 0 || fr_block_sz != BLEN)
			break;
		}
        	if(fr_block_sz<0)
		{
			printf("Error receiving file from client\n");
		}
		fclose(fr);
	}
	memset(&resBuffer[0], 0, BLEN);
	strcpy(resBuffer,"File uploaded successfully\n");
	send(sockfd, resBuffer,strlen(resBuffer),0);
}

/*-----------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------UPLOAD Grades-------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------*/

void upload_grades(int sockfd)
{
	load_studentinfo(sockfd);	
	char reqBuffer1[BLEN],resBuffer1[BLEN],grades[1];
	int i;
	FILE *fp = fopen("./Files/Grades/grades.txt", "w");
    
	if(fp == NULL)
	{
		printf("File not found\n");
	}
	for (i=0;i<f_Count-1;i++)
	{
		memset(&resBuffer1[0], 0, BLEN);		
		snprintf(resBuffer1, BLEN, "%d",sLogin.sId[i]);		
		send(sockfd,resBuffer1 ,sizeof(resBuffer1),0);
		memset(&reqBuffer1[0], 0, BLEN);
		recv(sockfd,reqBuffer1,sizeof(reqBuffer1),0);
		fprintf(fp, " %s,%s\n",resBuffer1,reqBuffer1);
	}
	fclose(fp);
	memset(&resBuffer1[0], 0, BLEN);
	strcpy(resBuffer1,"0");
	send(sockfd,resBuffer1 ,sizeof(resBuffer1),0);
	memset(&reqBuffer1[0], 0, BLEN);
	perror("grades(uploaded):");

}

/*-----------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------UPLOAD TEST---------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------*/

void upload_test(int sockfd)
{
	char reqBuffer[BLEN],resBuffer[BLEN];
	long fr_block_sz;
    
	memset(&resBuffer[0], 0, BLEN);
	strcpy(resBuffer,"Please enter the question file name to be uploaded\n");
	send(sockfd, resBuffer,strlen(resBuffer),0);
	memset(&resBuffer[0], 0, BLEN);
    
	memset(&reqBuffer[0], 0, BLEN);
	recv(sockfd,reqBuffer,sizeof(reqBuffer),0);
    
	printf("filename received\n");
	char path[100];
	strcpy(path,".");
	strcat(path,"/Files/Test/questions/");
	strcat(path,reqBuffer);
	memset(&reqBuffer[0], 0, sizeof(reqBuffer));
    
	FILE *fr = fopen(path, "a");
    
	if(fr==NULL)
	{
		printf("File cant be opened \n");
	}
	else
	{
		while((fr_block_sz = recv(sockfd, reqBuffer, BLEN, 0))>0)
		{
			size_t write_sz = fwrite(reqBuffer, sizeof(char), fr_block_sz, fr);
			if(write_sz < fr_block_sz)
			{
				printf("File write failed on server \n");
			}
			memset(&reqBuffer[0], 0, sizeof(reqBuffer));
			if(fr_block_sz == 0 || fr_block_sz != BLEN)
			break;

		}
		if(fr_block_sz<0)
		{
			printf("Error receiving file from client\n");
		}

		fclose(fr);
	}
	
	memset(&resBuffer[0], 0, BLEN);
	strcpy(resBuffer,"Question File uploaded successfully\nPlease enter the answer file name\n");
	send(sockfd, resBuffer,strlen(resBuffer),0);
	upload_answers(sockfd);
}

/*-----------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------UPLOAD ANSWERS------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------*/

void upload_answers(int sockfd)
{
char reqBuffer[BLEN],resBuffer[BLEN];
long fr_block_sz;
  
memset(&reqBuffer[0], 0, BLEN);
recv(sockfd,reqBuffer,sizeof(reqBuffer),0);
char path[100];
strcpy(path,".");
strcat(path,"/Files/Test/answers/");
strcat(path,reqBuffer);
memset(&reqBuffer[0], 0, sizeof(reqBuffer));

FILE *fr = fopen(path, "a");

if(fr==NULL)
{
printf("File cant be opened \n");
}
else
{
while((fr_block_sz = recv(sockfd, reqBuffer, BLEN, 0))>0)
{
size_t write_sz = fwrite(reqBuffer, sizeof(char), fr_block_sz, fr);
            if(write_sz < fr_block_sz)
            {
                printf("File write failed on server\n");
            }
            memset(&reqBuffer[0], 0, sizeof(reqBuffer));
            if (fr_block_sz == 0 || fr_block_sz != BLEN)
            {
                break;
            }
        }
        
        if(fr_block_sz<0)
        {
            printf("Error receiving file from client\n");
        }
        
        fclose(fr);
    }
    memset(&resBuffer[0], 0, BLEN);
    strcpy(resBuffer,"Answer File uploaded successfully\n");
    send(sockfd, resBuffer,strlen(resBuffer),0);
}

/*-----------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------DOWNLOAD FILES------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------*/

void download_file(int sockfd)
{
	int i=0,strcount=0;
	DIR  *d;
	struct dirent *dir;
	size_t fs_block_sz;
	char resBuffer[BLEN],reqBuffer[BLEN],path[1024];
	char **strarray=NULL;
	d = opendir("./Files/Uploads");
   	
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if (dir->d_type == DT_REG)
			{
				strarray = (char **)realloc(strarray, (strcount + 1) * sizeof(char *));
				strarray[strcount++] = strdup(dir->d_name);
			}
		}
		closedir(d);
	}

	memset(&resBuffer[0], 0, BLEN);
	snprintf(resBuffer,sizeof(resBuffer),"%d",strcount);
	send(sockfd,(void *)resBuffer,sizeof(resBuffer),0);
	memset(&resBuffer[0], 0, BLEN);
    
	for(i = 0; i < strcount; i++)
	{
		strcpy(resBuffer,strarray[i]);
		send(sockfd,(void *)resBuffer,sizeof(resBuffer),0);
		memset(&resBuffer[0],0,sizeof(resBuffer));
		free(strarray[i]);
	}
    
	free(strarray);
	memset(&reqBuffer[0], 0, BLEN);
	recv(sockfd,reqBuffer,sizeof(reqBuffer),0);
    
	memset(&path[0],0,sizeof(path));
	strcpy(path,".");
	strcat(path,"/Files/Uploads/");
	strcat(path,reqBuffer);
	memset(&reqBuffer[0], 0, BLEN);

	FILE *fs = fopen(path, "r");
    
	if(fs == NULL)
	{
		printf("File not found \n");
	}
    
	memset(&resBuffer[0], 0, BLEN);
    
	while((fs_block_sz = fread(resBuffer, sizeof(char), BLEN, fs))>0)
	{
		if(send(sockfd, resBuffer, fs_block_sz, 0) < 0)
		{
			printf("Failed to send file \n");
            		break;
		}
		memset(&resBuffer[0], 0, BLEN);
	}
	memset(&path[0],0,sizeof(path));
}
/*-----------------------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------VIEW OFFICE HOURS-------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------*/


void view_officehours(int sockfd)
{
	   
	FILE *fp;
	int i=0;
	char ch=NULL,file[2048],buffer[2048];
	fp=NULL;
	memset(&file[0],0,sizeof(file));
	memset(&buffer[0],0,sizeof(buffer));
	strcpy(file,"./Files/officehours/");
	strcat(file,"office.txt");
	fp=fopen(file, "r");
	while(ch!=EOF)
	{
		ch=fgetc(fp);
		buffer[i]=ch;
		i++;
	}
    
	send(sockfd,(void *)buffer,sizeof(buffer),0);
	memset(&file[0],0,sizeof(file));
	memset(&buffer[0],0,sizeof(buffer));
        perror("Sent office hours!!");
	fclose(fp);
   
}


/*-----------------------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------VIEW GRADES-------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------*/
void view_grades(int sockfd)
{
	load_gradesinfo(sockfd);	
	char gBuffer[1024];
	int i=0;
	
	memset(&gBuffer[0],0,sizeof(gBuffer));
	perror("View Grades");
	for(i=0;i<f_Count;i++)
	{	

		if(sCredential.sId==sGrades.sId[i])
		{
			strcpy(gBuffer,sGrades.grade[i]);
			send(sockfd, (void *)gBuffer,sizeof(gBuffer),0);
			memset(&gBuffer[0],0,sizeof(gBuffer));
		}
	}
}

/*-----------------------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------TAKE TEST---------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------*/


void evaluate_test(int sockfd)
{
	char buffer1[2048];
	char buffer2[2048];
	char buffer3[2048];
	char *hey[2048];
	char copy[2048];
    
	/** Declration for directories**/
	DIR *d;
	struct dirent *dir;
	int i_dir=0,j_dir=0;
    
	/** Declaration for displaying file contents and calculating the d **/
	FILE *fp,*fp_1;
	int i_file=0,j_file=0,score=0;
	char ch,ch_1,filename[2048],file[2048],buffer_ans[2048],buffer_ques[2048],ans[2048];
    
	printf("\nstudent wants to take an online exam\n");
	strcpy(buffer2,"\n\nTest series available -");
	send(sockfd,(void *)buffer2,sizeof(buffer2),0);
    	perror("String sent:");
	d=opendir("./Files/Test/questions/");
	if(d)
	{
		while((dir =readdir(d))!= NULL)
		{
			if (dir->d_type == DT_REG)
			{
				hey[i_dir]=dir->d_name;
				i_dir++;
			}
		}
		closedir (d);
	}	
 
	memset(&buffer2[0],0,sizeof(buffer2));
	snprintf(buffer2,sizeof(buffer2),"%d",i_dir);
	send(sockfd,(void *)buffer2,sizeof(buffer2),0);
	perror("Sent the file names:");
   
	for(j_dir=0;j_dir<i_dir;j_dir++)
	{
		strcpy(copy,hey[j_dir]);
		send(sockfd,(void *)copy,sizeof(copy),0);
		memset(&copy[0],0,sizeof(copy));
	}
    
	recv(sockfd,buffer1,sizeof(buffer1),0);
        perror("The file name recived:");
	strcpy(filename,buffer1);

	memset(&file[0],0,sizeof(file));
	memset(&buffer_ques[0],0,sizeof(buffer_ques));
	strcpy(file,"./Files/Test/questions/");
	strcat(file,filename);
	fp=fopen(file, "r");
	while(ch!=EOF)
	{
		ch=fgetc(fp);
		buffer_ques[i_file]=ch;
		i_file++;
	}
    
	send(sockfd,(void *)buffer_ques,sizeof(buffer_ques),0);
        perror("Sent the questions:");
	fclose(fp);
   
	memset(&file[0],0,sizeof(file));
	strcpy(file,"./Files/Test/answers/");
	strcat(file,filename);
	fp_1=fopen(file, "r");
	memset(&buffer_ans[0],0,sizeof(buffer_ans));
	while(ch_1!=EOF)
	{
		ch_1=fgetc(fp_1);
		buffer_ans[j_file]=ch_1;
		j_file++;
	}
  
	fclose(fp_1);

	j_file=0;
	memset(&buffer1[0],0,sizeof(buffer1));
	recv(sockfd,buffer1,sizeof(buffer1),0);
	strcpy(ans,buffer1);
 
	for(i_file=0;i_file<strlen(ans);i_file++)
	{
		if(ans[i_file]==buffer_ans[j_file])
		{
			score=score+1;
		}
 
		j_file++;
	}
	memset(&buffer2[0],0,sizeof(buffer2));
	snprintf(buffer2,sizeof(buffer2),"%d",score);
	send(sockfd,(void *)buffer2,sizeof(buffer2),0);
   
	memset(&buffer1[0],0,sizeof(buffer1));
	memset(&buffer2[0],0,sizeof(buffer2));
	memset(&buffer3[0],0,sizeof(buffer3));
	memset(&hey[0],0,sizeof(hey));
}

/*-----------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------TOTAL CLIENTS-------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------*/


void prstats(void)
{
    (void) pthread_mutex_lock(&statistics.st_mutex);
    (void) printf("%-32s: %u\n", "Total number of client connections",statistics.st_contotal);
    (void) pthread_mutex_unlock(&statistics.st_mutex);
}

