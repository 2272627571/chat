#include <stdio.h>
#include <Winsock2.h>
DWORD WINAPI ListenThread(LPVOID);//������Ϣ
DWORD WINAPI SendThread(LPVOID);//������Ϣ
char tem1[10];
int main()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD( 1, 1 );
    err = WSAStartup( wVersionRequested, &wsaData );//��ʼ���׽��ֽӿ�
    if ( err != 0 )
    {
        return 0;
    }
    if ( LOBYTE( wsaData.wVersion ) != 1 || HIBYTE( wsaData.wVersion ) !=1 )
    {
        WSACleanup( );
        return 0;
    }
    SOCKET sockClient=socket(AF_INET,SOCK_STREAM,0);

    SOCKADDR_IN addrSrv;
    memset((void *)&addrSrv, 0, sizeof(addrSrv));
    addrSrv.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");//������IP��ַ��
    addrSrv.sin_family=AF_INET;
    addrSrv.sin_port=htons(6000);

    int i=connect(sockClient,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));
    if (i >= 0)
    {
        memset(tem1, 0, sizeof(tem1));
        printf("�ɹ�����������:���ڿ�ʼ�Ի�----------------\n");
        printf("����������û�����  ");
        scanf("%s", tem1);
        send(sockClient, tem1,sizeof(tem1), 0);
        memset(tem1, 0, sizeof(tem1));
        printf("��������Ҫ������˵��û�����  ");
        scanf("%s", tem1);
        send(sockClient, tem1, sizeof(tem1), 0);
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ListenThread,
                     (LPVOID)&sockClient, 0, NULL);
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendThread,
                     (LPVOID)&sockClient, 0, NULL);
    }
    else
    {
        printf("����ʧ��\n");
    }
    //�ر��׽���s
    int j;
    for (j = 0; j < 1000; j++)
        Sleep(100000);
    closesocket(sockClient);
    //ע���׽��ֽӿ�
    WSACleanup();
    //�����ж�
    system("pause");
    return 0;
}

DWORD WINAPI ListenThread(LPVOID lpParameter)
{
    SOCKET s= *((SOCKET *)lpParameter);
    char buf[128] = { 0 };
    int ret;
    while (1)
    {
        memset(buf, 0, sizeof(buf));
        ret = recv(s, buf, sizeof(buf), 0);
        if (ret > 0)
        {
            printf("%s ˵: ",buf);
        }
        memset(buf, 0, sizeof(buf));
        ret = recv(s, buf, sizeof(buf), 0);
        if (ret > 0)
        {
            printf("%s\n", buf);
        }
    }
    return 0;
}
DWORD WINAPI SendThread(LPVOID lpParameter)
{
    SOCKET s= *((SOCKET *)lpParameter);
    char buf[128]={0};
    while (1)
    {
        memset(buf, 0, sizeof(buf));
        scanf("%s",buf);//�����ܿո�
        //gets(buf);//���ܿո�
        send(s, buf, sizeof(buf), 0);
    }
    return 0;
}
