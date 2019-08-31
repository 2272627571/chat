#include <stdio.h>
#include<queue>
#include<string.h>
#include <Winsock2.h>
using namespace std;
DWORD WINAPI oneThread(LPVOID lpParameter);//���߳�
DWORD WINAPI acceptThread(LPVOID lpParameter);//�����Ƿ����µĿͻ���
DWORD WINAPI ListenThread(LPVOID);//����ÿһ����Ϣ��������
DWORD WINAPI recvThread();//����ÿ���ͻ���
DWORD WINAPI SendThread();//������Ϣ�����е�ÿһ����Ϣ

typedef struct Client
{
    SOCKET socket;
    char name[10];//�Ի���������
    char aim[10];//�Ի����շ�����
    int flag;//�Ƿ񴴽������߳�
} Client;
typedef struct Message
{
    char news[128];//��Ϣ
    char aim[10];//��Ϣ���շ�
    char name[10];//��Ϣ����
} Message;

Client AClient[10];//�ͻ��˽ṹ������
queue <Message> message;//��Ϣ����

int t=0;//ListenThread����Ϣ���
int cnt=0;//acceptThread�ͻ��˼���
int change = 0;//acceptThread��־�Ƿ����µĿͻ������ӽ���,��������1

//���ؽ��շ��׽���
SOCKET getsocket(char tem[])//�������Ϊ��Ϣ���н��շ���������ɵ�����
{
    int i;
    char BClient[10];//�ͻ�����
    for (i = 0; i < cnt; i++)//��ÿһ���ͻ���
    {
        if (strcmp(tem, AClient[i].name)==0)//��tem��������ַ���ͻ���������ͬ�������Ƹ���BClient���飬��Ӧ�±�һ��
        {
            memset(BClient, 0, sizeof(BClient));//��BClient�е�ǰλ�ú����sizeof(BClient)���ֽ���0�滻������BClient,�������������0������
            strcpy(BClient, AClient[i].name);
            return AClient[i].socket;
        }
    }
    printf("no named client---------------\n");
    return 0;
}

int main()
{
    WORD wVersionRequested;//Winsock�汾
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD( 1, 1 );//��һ������Ϊ��λ�ֽڣ��ڶ�������Ϊ��λ�ֽ�
    err = WSAStartup( wVersionRequested, &wsaData );//��ʼ��Winsock
    if ( err != 0 )
    {
        return 0;
    }
    if ( LOBYTE( wsaData.wVersion ) != 1 || HIBYTE( wsaData.wVersion ) != 1 )
    {
        WSACleanup( );
        return 0;
    }
    SOCKET sockSrv=socket(AF_INET,SOCK_STREAM,0);//�����׽���(AF_INETΪIPV4Э�飻�׽ӿ�����ΪSOCK_STREAM,protocolһ��Ϊ0)
    SOCKADDR_IN addrSrv;
    memset((void *)&addrSrv, 0, sizeof(addrSrv));
    addrSrv.sin_addr.S_un.S_addr=htonl(INADDR_ANY);//��������ַ��Ϣ(�����ֽ�ת���������ֽڣ�ͨ���ַ��ֵΪ0)
    addrSrv.sin_family=AF_INET;//IP��ַ��
    addrSrv.sin_port=htons(6000);//�˿ں�(�����ֽ�ת���������ֽ�)
    int result=bind(sockSrv,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));//���̰��ض�IP��ַ�������׽ӿ���
    if (result>= 0)
    {
        printf("�󶨶˿ڳɹ������ڿ�ʼ�������߳�..........\n");
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)oneThread,
                     (LPVOID)&sockSrv, 0, NULL);
    }
    else
    {
        printf("��������ʼ��ʧ��.........................\n");
    }
    int i;
    for (i = 0; i < 1000; i++)
        Sleep(100000);
    closesocket(sockSrv);//�ر��׽���sockSrv
    WSACleanup();
    system("pause");//�����ж�
    return 0;
}
//���߳�
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

//1�������Ƿ����µĿͻ���
DWORD WINAPI acceptThread(LPVOID lpParameter)
{
    int i = 0;//�ͻ��˽ṹ�������±�
    int ret;//����ֵ
    SOCKET s= *((SOCKET *)lpParameter);//�׽���
    SOCKET ss;//��һ�����׽���
    char buf[10];
    while (1)
    {
        /**
        *listen(�׽��֣����׽ӿ��Ŷӵ����������)��ϵͳĬ���������ӱ�Ϊ�������ӣ��Ӷ����Խ��������������󣬼���Ϊ���������̣�
        *bcaklog������ʾ�����ӵ�������δ��������ڽ��е����Ӷ���,����ΪSOMAXCONN
        */
        listen(s, SOMAXCONN);
        if (cnt<10)
        {
            printf("���ڼ����˿�\n");
            if ((ss = accept(s, NULL, NULL)) == INVALID_SOCKET)//�������Ӷ���ͷ������һ����
            {
                continue;
            }
            else
            {
                printf("�пͻ�������\n");
                memset(buf, 0, sizeof(buf));
                AClient[i].socket = ss;//����һ�����׽��ָ����ͻ��˽ṹ������
                ret = recv(ss, buf, sizeof(buf), 0);
                if (ret > 0)
                {
                    strcpy(AClient[i].name, buf);//�Ự������
                }
                memset(buf, 0, sizeof(buf));
                ret = recv(ss, buf, sizeof(buf), 0);
                if (ret > 0)
                {
                    strcpy(AClient[i].aim, buf);//�Ự������
                }
                printf("�¿ͻ������ӳɹ�,�û���:%s     �������: %s\n", AClient[i].name, AClient[i].aim);
                i++;
                cnt++;
                change = 1;
            }
        }
        else
        {
            printf("��ǰ�ͻ������������Ժ��ٷ���................\n");
            return 0;
        }
    }
    return 0;
}

//3������ÿһ����Ϣ��������
DWORD WINAPI ListenThread(LPVOID lpParameter)
{
    Client s= *((Client *)lpParameter);//�ͻ��˽ṹ��
    Message tem_message;//��Ϣ�ṹ��
    char buf[128];//������
    strcpy(tem_message.aim, s.aim);//�Ի����շ�����
    strcpy(tem_message.name, s.name);//�Ի���������
    while (1)
    {
        memset(buf, 0, sizeof(buf));//��������ʼ��������
        int ret = recv(s.socket, buf, sizeof(buf), 0);
        if (ret > 0)
        {
            strcpy(tem_message.news, buf);
            message.push(tem_message);//��Ϣ�������
        }
        else//����Ϣ���
        {
            t++;
            printf("%d", t);
            t--;
            Sleep(1000);//��1s��ӡһ��1
        }
    }
    return 0;
}

//2������ÿ���ͻ���(������)
DWORD WINAPI recvThread()
{
    int i;
    while (1)
    {
        if (change == 1&&cnt>1)//�����������������Ͽͻ�������
        {
            for (i = 0; i < cnt; i++)
            {
                if (AClient[i].flag != 1)//�ͻ��˴��������߳�
                {
                    printf("��ʼ������Ϣ..........\n");
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
//4��������Ϣ�����е�ÿһ����Ϣ
DWORD WINAPI SendThread()
{
    SOCKET s;
    Message tem_message;
    while (1)
    {
        if (message.empty()==FALSE)//��Ϣ���в�Ϊ��
        {
            tem_message = message.front();
            s = getsocket(tem_message.aim);//��Ϣ���շ���
            send(s,tem_message.name, sizeof(tem_message.name), 0);
            send(s, tem_message.news, sizeof(tem_message.news), 0);
            message.pop();//��Ϣ������
        }
    }
    return 0;
}
