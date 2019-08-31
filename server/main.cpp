#include <stdio.h>
#include<queue>
#include<string.h>
#include <Winsock2.h>
using namespace std;
DWORD WINAPI oneThread(LPVOID lpParameter);//总线程
DWORD WINAPI acceptThread(LPVOID lpParameter);//监听是否有新的客户端
DWORD WINAPI ListenThread(LPVOID);//接收每一个信息到队列中
DWORD WINAPI recvThread();//监听每个客户端
DWORD WINAPI SendThread();//发送消息队列中的每一个消息

typedef struct Client
{
    SOCKET socket;
    char name[10];//对话发起方名称
    char aim[10];//对话接收方名称
    int flag;//是否创建监听线程
} Client;
typedef struct Message
{
    char news[128];//消息
    char aim[10];//消息接收方
    char name[10];//消息发起方
} Message;

Client AClient[10];//客户端结构体数组
queue <Message> message;//消息队列

int t=0;//ListenThread无消息情况
int cnt=0;//acceptThread客户端计数
int change = 0;//acceptThread标志是否有新的客户端连接进来,若有则置1

//返回接收方套接字
SOCKET getsocket(char tem[])//传入参数为消息队列接收方名称所组成的数组
{
    int i;
    char BClient[10];//客户端名
    for (i = 0; i < cnt; i++)//对每一个客户端
    {
        if (strcmp(tem, AClient[i].name)==0)//若tem数组里的字符与客户端名称相同，则将名称赋给BClient数组，对应下标一致
        {
            memset(BClient, 0, sizeof(BClient));//将BClient中当前位置后面的sizeof(BClient)个字节用0替换并返回BClient,即对数组进行清0操作。
            strcpy(BClient, AClient[i].name);
            return AClient[i].socket;
        }
    }
    printf("no named client---------------\n");
    return 0;
}

int main()
{
    WORD wVersionRequested;//Winsock版本
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD( 1, 1 );//第一个参数为低位字节；第二个参数为高位字节
    err = WSAStartup( wVersionRequested, &wsaData );//初始化Winsock
    if ( err != 0 )
    {
        return 0;
    }
    if ( LOBYTE( wsaData.wVersion ) != 1 || HIBYTE( wsaData.wVersion ) != 1 )
    {
        WSACleanup( );
        return 0;
    }
    SOCKET sockSrv=socket(AF_INET,SOCK_STREAM,0);//创建套接字(AF_INET为IPV4协议；套接口类型为SOCK_STREAM,protocol一般为0)
    SOCKADDR_IN addrSrv;
    memset((void *)&addrSrv, 0, sizeof(addrSrv));
    addrSrv.sin_addr.S_un.S_addr=htonl(INADDR_ANY);//服务器地址信息(主机字节转换成网络字节，通配地址，值为0)
    addrSrv.sin_family=AF_INET;//IP地址族
    addrSrv.sin_port=htons(6000);//端口号(主机字节转换成网络字节)
    int result=bind(sockSrv,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));//进程把特定IP地址捆绑在套接口上
    if (result>= 0)
    {
        printf("绑定端口成功，正在开始服务器线程..........\n");
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)oneThread,
                     (LPVOID)&sockSrv, 0, NULL);
    }
    else
    {
        printf("服务器初始化失败.........................\n");
    }
    int i;
    for (i = 0; i < 1000; i++)
        Sleep(100000);
    closesocket(sockSrv);//关闭套接字sockSrv
    WSACleanup();
    system("pause");//程序中断
    return 0;
}
//总线程
DWORD WINAPI oneThread(LPVOID lpParameter)
{
    SOCKET s= *((SOCKET *)lpParameter);
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)acceptThread,
                 (LPVOID)&s, 0, NULL);
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendThread,
                 NULL, 0, NULL);
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)recvThread,
                 NULL, 0, NULL);
    return 0;
}

//1、监听是否有新的客户端
DWORD WINAPI acceptThread(LPVOID lpParameter)
{
    int i = 0;//客户端结构体数组下标
    int ret;//返回值
    SOCKET s= *((SOCKET *)lpParameter);//套接字
    SOCKET ss;//下一连接套接字
    char buf[10];
    while (1)
    {
        /**
        *listen(套接字，此套接口排队的最大连接数)由系统默认主动连接变为被动连接，从而可以接受其他进程请求，即成为服务器进程，
        *bcaklog参数表示已连接但服务器未受理或正在进行的连接队列,可以为SOMAXCONN
        */
        listen(s, SOMAXCONN);
        if (cnt<10)
        {
            printf("正在监听端口\n");
            if ((ss = accept(s, NULL, NULL)) == INVALID_SOCKET)//从已连接队列头返回下一连接
            {
                continue;
            }
            else
            {
                printf("有客户端请求\n");
                memset(buf, 0, sizeof(buf));
                AClient[i].socket = ss;//将下一连接套接字赋给客户端结构体数组
                ret = recv(ss, buf, sizeof(buf), 0);
                if (ret > 0)
                {
                    strcpy(AClient[i].name, buf);//会话发起者
                }
                memset(buf, 0, sizeof(buf));
                ret = recv(ss, buf, sizeof(buf), 0);
                if (ret > 0)
                {
                    strcpy(AClient[i].aim, buf);//会话接收者
                }
                printf("新客户端连接成功,用户名:%s     聊天对象: %s\n", AClient[i].name, AClient[i].aim);
                i++;
                cnt++;
                change = 1;
            }
        }
        else
        {
            printf("当前客户端已满，请稍后再访问................\n");
            return 0;
        }
    }
    return 0;
}

//3、接收每一个信息到队列中
DWORD WINAPI ListenThread(LPVOID lpParameter)
{
    Client s= *((Client *)lpParameter);//客户端结构体
    Message tem_message;//消息结构体
    char buf[128];//缓冲区
    strcpy(tem_message.aim, s.aim);//对话接收方名称
    strcpy(tem_message.name, s.name);//对话发起方名称
    while (1)
    {
        memset(buf, 0, sizeof(buf));//缓冲区初始化，清零
        int ret = recv(s.socket, buf, sizeof(buf), 0);
        if (ret > 0)
        {
            strcpy(tem_message.news, buf);
            message.push(tem_message);//消息加入队列
        }
        else//无消息情况
        {
            t++;
            printf("%d", t);
            t--;
            Sleep(1000);//隔1s打印一个1
        }
    }
    return 0;
}

//2、监听每个客户端(已连接)
DWORD WINAPI recvThread()
{
    int i;
    while (1)
    {
        if (change == 1&&cnt>1)//已连接且有两个以上客户端在线
        {
            for (i = 0; i < cnt; i++)
            {
                if (AClient[i].flag != 1)//客户端创建监听线程
                {
                    printf("开始接收信息..........\n");
                    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ListenThread,
                                 (LPVOID)&AClient[i], 0, NULL);
                    AClient[i].flag = 1;
                }
            }
            change = 0;
        }
    }
    return 0;
}
//4、发送消息队列中的每一个消息
DWORD WINAPI SendThread()
{
    SOCKET s;
    Message tem_message;
    while (1)
    {
        if (message.empty()==FALSE)//消息队列不为空
        {
            tem_message = message.front();
            s = getsocket(tem_message.aim);//消息接收方名
            send(s,tem_message.name, sizeof(tem_message.name), 0);
            send(s, tem_message.news, sizeof(tem_message.news), 0);
            message.pop();//消息出队列
        }
    }
    return 0;
}
