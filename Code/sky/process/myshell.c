#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <readline/readline.h>

#define normal 0 //一般命令
#define out_redirect 1 //输出重定向
#define in_redirect 2  //输入重定向
#define have_pipe   3  //命令中有管道

void print_promopt();
void get_input(char *buf);
void explain_input(char *buf,int *argcount,char arglist[100][256]);
void do_cmd(int argcount,char arglist[100][256]);
int find_command(char *command);
int main()
{
    int i;
    int  argcount = 0;
    char arglist[100][256];
    char **arg = NULL;
    char *buf = NULL;
    system("clear");
    buf = (char *)malloc(256);
    if(buf == NULL)
    {
        perror("malloc失败!\n");
        exit(-1);  
    }

    while(1)
    {
        //将buf所指向的空间清零
        memset(buf,0,256);
        print_promopt();
        /* get_input(buf); */
        char *a = readline(NULL);
        strcpy(buf,a);
        printf("buf = %s\n",buf);
        //若输入命令为exit或者logout则退出本程序
        if(!strcmp(buf,"exit\n") || !strcmp(buf,"logout\n"))    break;
        for(int i = 0;i<100;i++)
        {
            arglist[i][0] = '\n';
        }
        argcount = 0;
        explain_input(buf,&argcount,arglist);
        do_cmd(argcount,arglist);//命令个数 命令列表
    }

    if(buf != NULL)
    {
        free(buf);
        buf = NULL;
    }

    exit(0);
    return 0;
}

void print_promopt()
{
    struct passwd *pwd;
    pwd = getpwuid(getuid());
    char hostname[64];
    puts("");
    gethostname(hostname,sizeof(hostname));
    printf("\033[1m\033[44;33m%s@%s$ \033[0m",pwd->pw_name,hostname);
    printf("\033[1m\033[42;37m%s$ \033[0m",getcwd(NULL,0));
    /* printf("myshell$$ "); */
}

void get_input(char *buf)
{
    /* int len = 0; */
    /* int ch; */
    char *a;
    a = readline(NULL);//readline为其动态分配内存
    /* strcpy(buf,a); */
    printf("%s\n",buf);

    if(((int)strlen(buf)) >= 256)
    {
        printf("命令太长!\n");
        exit(-1);//输入命令过长而退出程序?
    }
        /* printf("%s\n",buf); */
        /* buf[len++] = '\n'; */
        /* buf[len] = '\0'; */

}

//解析输入的命令
//将结果存入arglist中
void explain_input(char *buf,int *argcount,char arglist[100][256])
{
    char *p = buf;
    char *q = buf;
    int number = 0;


    while(1)
    {
        if(p[0] == '\n') break;
        if(p[0] == ' ')  p++;
        else
        {
            q = p;
            number = 0;
            while(q[0] != ' ' && q[0] != '\n')
            {
                number++;
                q++;
            }
            strncpy(arglist[*argcount],p,number);
            arglist[*argcount][number] = '\0';
            *argcount = *argcount+1;
            p = q;
        }
    }
    for(int i = 0;i<*argcount;i++)
    {
        printf("解析命令　%s\n",arglist[i]);
    }
    return ;
}
void do_cmd(int argcount,char arglist[100][256])
{
    int how = 0;//指示命令中是否含有< > |
    int background = 0;//表示命令中是否有后台运行表示符
    int status,i,fd,cnt = 0;
    char *arg[argcount+1],*argnext[argcount+1],*file;
    pid_t pid;

    //将命令取出来
    for(i = 0;i<argcount;i++)
    {
        arg[i] = (char*)arglist[i];
    }
    arg[argcount] = NULL;

    //查看命令中是否有后台运行符号
    for(i = 0;i < argcount;i++)
    {
        if(strncmp(arg[i],"&",1) == 0)//后台运行符号在命令的最后面
        {
            if(i == argcount-1)
            {
                background = 1;
                arg[argcount-1] = NULL;//去掉最后这个&
                break;
            }
            else
            {
                printf("命令输入有误\n");
                return ;
            }
        }
    }

    //输入检测
    for(i = 0; arg[i] != NULL; i++)
    {
        if(strcmp(arg[i],">") == 0)
        {//含有> i==0?没有处理?
            cnt++;
            how = out_redirect;
            if(arg[i+1] == NULL || i == 0) cnt++;//无输出文件名也不合法
        }
        if(strcmp(arg[i],"<") == 0)
        {//含有<
            cnt++;
            how = in_redirect;
            if(i == 0) cnt++;//不合法
        }
        if(strcmp(arg[i],"|") == 0)
        {
            cnt++;
            how = have_pipe;
            if(arg[i+1] == NULL || i == 0) cnt++;
        }

    }

    //不合法输入处理
    if(cnt > 1)
    {
        printf("输入命令有误\n");
        return ;
    }


    //合法输入处理
    /* printf("输入合法\n"); */
    if(how == out_redirect)
    {
        for(i = 0;arg[i] != NULL ; i++)
        {
            if(strcmp(arg[i],">") == 0)
            {
                file = arg[i+1];//file指向要输出的文件
                /* printf("%s\n",file); */
                arg[i] = NULL;//去掉>这个命令
            }
        }
    }

    if(how == in_redirect)
    {
        for(i = 0;arg[i] != NULL ;i++)
        {
            if(strcmp(arg[i],"<") == 0)
            {
                file = arg[i+1];
                arg[i] = NULL;
            }
        }
    }

    if(how == have_pipe)//管道符
    {
        for(i = 0;arg[i] != NULL ; i++)
        {
            if(strcmp(arg[i],"|") == 0)
            {
                arg[i] = NULL;
                int j = 0;
                for(j = i+1;arg[j] != NULL; j++)
                {
                    argnext[j-i-1] = arg[j];//从0开始存放i+1之后的命令
                }
                argnext[j-i-1] = arg[j];
                break;
            }
        }
    }

    if((pid = fork()) < 0)
    {
        printf("进程创建失败\n");
        return ;
    }
    int temp;//父进程到此阻塞
    if(pid != 0) wait(&temp);
    switch(how)
    {
        case 0://normal命令
            if(!pid)//子进程中
                if( !find_command(arg[0]) )
                {
                    printf("未找到该命令\n");
                    exit(0);
                }
                else
                {
                    execvp(arg[0],arg);
                    exit(0);
                }
            break;
        case 1://输出重定向
            if(!pid)
                if( !find_command(arg[0]) )
                {
                    printf("未找到该命令:%s\n",arg[0]);
                    exit(0);
                }
                else
                {
                    fd = open(file,O_RDWR | O_CREAT | O_TRUNC,0644);//0644与umask做&运算后得到创建文件的权限
                    dup2(fd,1); //复制一个文件描述符
                    execvp(arg[0],arg);
                    exit(0);
                }
            break;
        case 2://输入重定向if(!pid)
                if(!pid)
                if( !find_command(arg[0]) )
                {
                    printf("未找到该命令:%s\n",arg[0]);
                    exit(0);
                }
                else
                {
                    fd = open(file,O_RDONLY);//0644与umask做&运算后得到创建文件的权限
                    dup2(fd,0); //复制一个文件描述符,第二各参数为新文件描述符的数值
                    execvp(arg[0],arg);
                    exit(0);
                }
                break;
        case 3://含有管道符号
                {int pid2,status2,fd2;
                if(!pid)
                {

                    if((pid2 = fork()) < 0)//由子进程创建一个子子进程
                    {
                        printf("进程创建失败\n");
                        exit(0);
                    }
                    else if(pid2 == 0)//进程创建成功 
                    {
                        if(!(find_command(arg[0])))
                        {
                            printf("未找到命令:%s",arg[0]);
                            exit(0);
                        }
                    
                    fd2  = open("./tmpfile",O_WRONLY | O_CREAT | O_TRUNC,0644);
                    dup2(fd2,1);
                    execvp(arg[0],arg);
                    exit(0);//子子进程结束 子子进程的作用就是执行 |前面的命令写入一个文件中
                    }
                

                //子进程所要执行的操作
                if(waitpid(pid2,&status2,0) == -1)
                {
                    printf("等待子进程时候出错:%d",__LINE__);
                }
                if(!find_command(argnext[0]))
                {
                    printf("未找到命令:%s",argnext[0]);
                }

                fd2 = open("./tmpfile",O_RDONLY);
                dup2(fd2,0);//复制标准输出
                execvp(argnext[0],argnext);
                if(remove("./tmpfile"))
                    printf("删除tempfile出错\n");
                exit(0);
                }}

                default:
                    break;
            



    if(background == 1)
    {//有后台运行符号父进程直接返回，不等待子进程
        printf("[进程ID:%d]\n",pid);
        return ;
    }
        if(waitpid(pid,&status,0) == -1)
        {
            printf("父进程等待子进程出错\n");
        }
        /* printf("父进程等待子进程\n"); */
    }
}
int find_command(char *command)
{
    DIR* dp;
    struct dirent* dirp;
    char *path[] = {"./","/bin",NULL}; //进程映象所在的目录

    //使上层目录下的ls可以执行
    if(strncmp(command,"./",2) == 0)
    {
        command += 2;//向后偏移3个单位
        return 1;
    }

    //在上层目录，/bin目录查找要执行的程序
    int i = 0;
    while(path[i] != NULL)
    {
        if((dp = opendir(path[i])) == NULL)
        {
            printf("命令存储目录打开失败\n");
        }
        while((dirp = readdir(dp) ) != NULL)
        {
            if(strcmp(dirp->d_name,command) == 0)//找到要执行的命令
            {
                closedir(dp);
                return 1;
            }
        }
        closedir(dp);
        i++;
    }
    return 0;
}
