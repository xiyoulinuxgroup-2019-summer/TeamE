#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include"my_recv.h"


#define BUFFSIZE 1024
#define MAXSIZE 1024
#define IPADDRESS "127.0.0.1"
#define SERV_PORT 4507
#define FDSIZE 1024
#define SIZE 30
typedef struct node
{
	char name[SIZE];
	char number[SIZE];
	char password[SIZE];
	char phonenumber[SIZE];
	char friendname[SIZE];
	int flag;

}loginnode;

char chat[1024];
//void my_err(const char * err_string,int line);

int login_connect(int conn_fd);
int login(conn_fd);
int main(int argc,char **argv)
{
	int i;
	int ret;
	int conn_fd;
	int serv_port;
	struct sockaddr_in serv_addr;
	char recv_buf[BUFFSIZE];

	conn_fd = socket(AF_INET,SOCK_STREAM,0);
	
	memset(&serv_addr,0,sizeof(serv_addr));
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET,IPADDRESS,&serv_addr.sin_addr);

	if(connect(conn_fd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr)) < 0)   my_err("connect",__LINE__);
	if((ret = my_recv(conn_fd,recv_buf,sizeof(recv_buf))) < 0)  my_err("recv",__LINE__);
	printf( "rettt = %d\n",ret);
	for(i = 0;i < ret;i++)  printf( "%c",recv_buf[i]); 
	printf( "\n");
	while(1)
	{
		int re;
		//scanf( "%s",chat);
		login_connect(conn_fd);
		gets(chat);
		if((ret = (send(conn_fd,chat,strlen(chat) + 1,0))) < 0)
		{
			printf( "错误 \n");
		}

		printf( "ret1 = %d\n ",ret);
//		if(( re = send(conn_fd,chat,strlen(chat),0)) < 0)  my_err("client send err",__LINE__);
	//	login_connect(conn_fd);
		if(strcmp(chat,"exit") == 0)  break;
		memset(chat,0,sizeof(chat));
	}

	close(conn_fd);

	return 0;

}
int login_connect(int conn_fd)
{
	int command;

	printf( "欢迎使用chat\n");
	printf( "请输入你的选项\n");
	printf( "[1]  登录\n");
	printf( "[2]  注册\n");
	printf( "[3]  修改密码\n");
	printf( "[4]  找回密码\n");
	printf( "[5]  退出\n");

	scanf( "%d",&command);
	getchar();
	switch(command)
	{
		case 1:
			login(conn_fd)	; break;
		case 2:
	//		regist();break;
		case 3:
	//		updatapassword();break;
		case 4:
	//		foudpassword();break;
		case 5:
	//		exit(1);break;
		default :
			printf( "选项错误\n");

	}
}

int login(int conn_fd)
{
	char cha[10] = "adasd";
	int re = 0;
	loginnode log;

	char buf[BUFFSIZE];
	printf("请输入昵称:");
	gets(log.name);
	printf( "请输入账号:");
	gets(log.number);
	printf( "请输入密码:");
	gets(log.password);
	printf( "密保问题: 手机号 :");
	gets(log.phonenumber);
	printf( "密保问题: 友好朋友的名字 :");
	gets(log.friendname);
	log.flag = 1;
	//if((re = (send(conn_fd,chat,strlen(chat) + 1,0))) < 0)  printf( "错误\n");
	memset(buf,0,1024);    //初始化
	memcpy(buf,&log,sizeof(loginnode));    //将结构体的内容转为字符串
	printf( "lenbuf = %d\n",sizeof(loginnode));
	if((re = (send(conn_fd,buf,sizeof(loginnode) + 1,0))) < 0)  printf( "错误\n");

	printf( "ret2 = %d\n",re);

	printf( "发送\n");
	memset(&log,0,sizeof(loginnode));
	memcpy(&log,buf,sizeof(loginnode));
	printf( "name %s\n",log.name);
	printf( "number %s\n",log.number);
	printf( "password %s\n",log.password);
	printf( "phone %s\n",log.phonenumber);
	printf( "frine %s\n",log.friendname);
	printf( "flag = %d\n",log.flag);
	
	
	return 0;
}
