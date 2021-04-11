#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
 
#define A 1   //增加
#define D 2	  //删除
#define M 3   //修改
#define S 4	  //检索
#define L 5   //日志
#define Q 6   //退出
#define N 16
#define R "S_login.txt"
typedef struct {
	int type;	
	char name[16];
	char data[256];
	int com;
	int id;
	int salary;
	char address[N];
	int age;
	int flag; 
}MSG;
 
int buf;   //标志位
FILE *fp;	//数据库文件的额文件指针
FILE *fp1;  //临时缓存文件的文件流指针
FILE *fp2;  //用户登录日志的文件流指针
int id = 1;  //员工编号
MSG user,tmp,linshi;
 
void *myrutine(void *arg);
int record(MSG *msg,int symbol);
 
int main(int argc,const char *argv[])
{
	printf("*******等待用户访问*******\n");
	if((fp = fopen("mydata.db","a+")) == NULL)
	{
		perror("fail to fopen 44");
		return -1;
	}
	
	MSG root ={
		.type = 1,
		.name = "admin",
		.data = "123456",
	};
	if(fread(&tmp,sizeof(MSG),1,fp) != 1)
	{
		fwrite(&root,sizeof(MSG),1,fp);
		fclose(fp);
	}
	else
	{
		fclose(fp);
	}
	int sockfd,clientfd;
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		perror("fail to socket");
		exit(-1);
	}
	int on = 1;
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(int));
	struct sockaddr_in serveraddr = {
		.sin_family = AF_INET,
		.sin_addr.s_addr = INADDR_ANY,
		.sin_port = htons( 8888 ),
	}, clientaddr={0};
	int len = sizeof(serveraddr);
	if(-1 == bind(sockfd, (struct sockaddr*)&serveraddr, len) )
	{
		perror("bind");
		return -1;
	}
	listen(sockfd, 10);
	char buf[100];
	while(1)
	{
		if(-1 == (clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len) ) )
		{
			perror("accept");
			return -1;
		}
		printf("IP: %s connected !\n", inet_ntoa(clientaddr.sin_addr) );
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_t tid;
		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
		pthread_create(&tid,&attr, myrutine, (void*)clientfd);
		printf("thread runing...........\n");
	}
	
	
	
	close(sockfd);
	fclose(fp);
	exit(0);	
}
 
void *myrutine(void *arg)
{
	int clientfd = (int)arg;
	while(1)
	{
		read(clientfd,&user,sizeof(user));//读取客户端发过来的数据放到user结构体中
		if((fp = fopen("mydata.db","a+")) == NULL)
		{
			perror("fail to fopen mydata.db");
			return (void *)-1;
		}
		fseek(fp,0,SEEK_SET);
#if 1
		while(fread(&tmp,sizeof(MSG),1,fp))//把文件中的结构体读出来放到tmp结构体中
		{
			if((!strcmp(tmp.name,user.name)) && (!strcmp(tmp.data,user.data)))
			{	
				if(tmp.type == 1)
				{
					buf = tmp.type;
					write(clientfd,&buf,sizeof(int));
					printf("超级用户登录成功\n");
					record(&user,buf);
				}
				if(tmp.type == 2)
				{
					buf = tmp.type;
					write(clientfd,&buf,sizeof(int));
					printf("普通用户登录成功\n");
					record(&user,buf);					
				}
				
				fclose(fp);
				goto aa;
				
			}
		}
		buf = 3;           //用户名或密码输入错误
		write(clientfd,&buf,sizeof(int));
		fclose(fp);
		
		record(&user,buf);
		continue;
	}
#endif
aa:	
	while(1)
	{
		struct stat s;
		if((fp = fopen("mydata.db","a+")) == NULL)
		{
			perror("fail to fopen");
			return (void *)-1;
		}
		read(clientfd,&user,sizeof(user));
		switch(user.com)
		{
			case A:
				fseek(fp,0,SEEK_END);
				user.id = id;
				id++;
				if(fwrite(&user,sizeof(MSG),1,fp) > 0)
				{
					fflush(fp);
					buf = 1;
					write(clientfd,&buf,sizeof(int));
					printf("添加用户成功\n");
				}
				else
				{
					buf = 2;
					write(clientfd,&buf,sizeof(int));
					printf("添加用户失败\n");
				}
				fclose(fp);
				break;
			case M:
#if 1
				//fseek(fp,0,SEEK_SET);
				while(fread(&tmp,sizeof(MSG),1,fp))
				{
					if((!strcmp(user.name,tmp.name))&&(!strcmp(user.data,tmp.data)))
					{
						write(clientfd,&tmp,sizeof(tmp));
						printf("成功发送用户信息给客户端\n");
						break;
					}
				}
				fclose(fp);
				read(clientfd,&user,sizeof(user));//等待客户端修改后的信息
				
				if((fp1 = fopen("buf.db","w+")) == NULL)
				{
					perror("fail to fopen");
					return (void *)-1;
				}
				
				if((fp = fopen("mydata.db","a+")) == NULL)
				{
					perror("fail to fopen");
					return (void *)-1;
				}
				
				while(fread(&tmp,sizeof(MSG),1,fp))
				{	
					if((!strcmp(user.name,tmp.name))&&(user.id == tmp.id))
					{
						fwrite(&user,sizeof(MSG),1,fp1);
						fflush(fp1);
						continue;
					}	
					fwrite(&tmp,sizeof(MSG),1,fp1);
					fflush(fp1);
				}
				fclose(fp);
				if((fp = fopen("mydata.db","w+")) == NULL)
				{
					perror("fail to fopen");
					return (void *)-1;
				}			
				
				fseek(fp1,0,SEEK_SET);
				
				while(fread(&tmp,sizeof(MSG),1,fp1))
				{
					fwrite(&tmp,sizeof(MSG),1,fp);
					fflush(fp);
				}
				
				fclose(fp);
				fclose(fp1);
				buf = 1;
				write(clientfd,&buf,sizeof(buf));
				printf("修改成功\n");
				break;
#endif
			case D:
#if 1			
				if((fp1 = fopen("buf.db","w+")) == NULL)
				{
					perror("fail to fopen");
					return (void *)-1;
				}
				
				while(fread(&tmp,sizeof(MSG),1,fp))
				{
					if((!strcmp(user.name,tmp.name))&&(!strcmp(user.data,tmp.data)))
					{
						continue;
					}
					fwrite(&tmp,sizeof(MSG),1,fp1);
					fflush(fp1);
				}
			
				fclose(fp);
				if((fp = fopen("mydata.db","w+")) == NULL)
				{
					perror("fail to fopen");
					return (void *)-1;
				}
				fseek(fp1,0,SEEK_SET);
				while(fread(&tmp,sizeof(MSG),1,fp1))
				{
					fwrite(&tmp,sizeof(MSG),1,fp);
					fflush(fp);
				}
				fflush(fp);
				fclose(fp);
				fclose(fp1);
				buf = 1;
				write(clientfd,&buf,sizeof(buf));			
				printf("删除成功\n");
#endif		
				break;
			case S:
				fseek(fp,0,SEEK_SET);
				while(fread(&tmp,sizeof(MSG),1,fp))
				{
					if((!strcmp(user.name,tmp.name))&&(!strcmp(user.data,tmp.data)))
					{
						write(clientfd,&tmp,sizeof(tmp));
						printf("成功检索到用户信息\n");
						fclose(fp);
						goto aa;
					}					
				}
				fclose(fp);
				linshi.flag = 2;   //2表示检索失败，重新输入用户名和密码
				write(clientfd,&linshi,sizeof(MSG));
				break;
				
			case L:				
				stat(R,&s);
				buf = s.st_size;
				write(clientfd,&buf,sizeof(buf));
				int n;
				char str[20]={0};
				int fd = open(R,O_RDONLY);
				while(n = read(fd,str,sizeof(str)))
				{
					write(clientfd,str,n);
					memset(str,0,sizeof(str));
				}
				close(fd);	
				break;
				
			case Q:
				printf("用户退出\n");
				close(clientfd);
				return (void *)0;
		}
	
	}
 
}
 
int record(MSG *msg,int symbol)
{
	if((fp2 = fopen(R,"a+")) == NULL)
	{
		perror("fail to fopen login.log");
		return -1;
	}
	time_t tv;
	tv = time(NULL);
	if(symbol == 3)
	{
		fprintf(fp2,"%s  登录失败  %s",msg->name,ctime(&tv));
	}
	else
	{	
		fprintf(fp2,"%s  登录成功  %s",msg->name,ctime(&tv));
	}
	fflush(fp2);
	fclose(fp2);
	return 0;
}
