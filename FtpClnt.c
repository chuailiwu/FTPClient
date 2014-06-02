#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")  /* WinSock使用的库函数 */

/* 定义常量 */
#define FTP_DEF_PORT     21  /* 连接的缺省端口 */
#define FTP_BUF_SIZE   1024  /* 缓冲区的大小   */
#define FTP_HOST_LEN    256  /* 主机名长度 */

void ftp_get(SOCKET soc, char* file_name){
    //printf("dsffsf");
    FILE *file_ftp;
    file_ftp = fopen(file_name, "w+");
    if(file_ftp == NULL){
        printf("[FTP] The file [%s] is not existed\n", file_name);
        exit(1);
    }

    int result = 0;
    char data_buf[FTP_BUF_SIZE];
    do /* 接收响应并保存到文件中 */
    {
        result = recv(soc, data_buf, FTP_BUF_SIZE, 0);
        if (result > 0)
        {
            fwrite(data_buf, 1, result, file_ftp);

            /* 在屏幕上输出 */
            data_buf[result] = 0;
            printf("%s", data_buf);
        }
    } while(result > 0);

    fclose(file_ftp);
}

void ftp_put(SOCKET soc, char* file_name){

    FILE *file;
    file = fopen(file_name, "rb+");
    if(file == NULL){
        printf("[FTP] The file [%s] is not existed\n", file_name);
        exit(1);
    }

    int file_len;
    fseek(file, 0, SEEK_END);
    file_len = ftell(file);
    fseek(file, 0, SEEK_SET);

    int read_len;
    char read_buf[FTP_BUF_SIZE];
    do /* 发送文件文件*/
    {
        read_len = fread(read_buf, sizeof(char), FTP_BUF_SIZE, file);

        if (read_len > 0)
        {
            send(soc, read_buf, read_len, 0);
            file_len -= read_len;
        }
    } while ((read_len > 0) && (file_len > 0));

    fclose(file);
}

int main(int argc, char **argv)
{
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2,0), &wsa_data); /* 初始化 WinSock 资源 */

    if(argc != 3 ){
        printf("usage: FTPClient commod filename");
        return 0;
    }

    char host[FTP_HOST_LEN] = "127.0.0.1";
    unsigned short port = FTP_DEF_PORT;

    char file_name[FTP_HOST_LEN];
    strcpy(file_name,argv[2]);

    unsigned long addr;
    addr = inet_addr(host);
    struct hostent *host_ent;
    if (addr == INADDR_NONE)
    {
        host_ent = gethostbyname(host);
        if (!host_ent)
        {
            printf("[FTP] invalid host\n");
            return -1;
        }
        memcpy(&addr, host_ent->h_addr_list[0], host_ent->h_length);
    }

    struct sockaddr_in serv_addr;  /* 服务器地址 */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = addr;

    SOCKET  ftp_sock = 0;         /* socket 句柄 */
    ftp_sock = socket(AF_INET, SOCK_STREAM, 0); /* 创建 socket */
    int result = 0;
    result = connect(ftp_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (result == SOCKET_ERROR) /* 连接失败 */
    {
        closesocket(ftp_sock);
        printf("[FTP] fail to connect, error = %d\n", WSAGetLastError());
        return -1;
    }

    /* 发送 FTP 请求 */
    int send_len;
    char data_buf[FTP_BUF_SIZE];
    send_len = sprintf(data_buf, "%s %s", argv[1], argv[2]);
    result = send(ftp_sock, data_buf, send_len, 0);
    char temp[1];
    recv(ftp_sock, temp, 1, 0);

    if (result == SOCKET_ERROR) /* 发送失败 */
    {
        printf("[FTP] fail to send, error = %d\n", WSAGetLastError());
        return -1;
    }
    //printf("%s\n",argv[1]);
    if(!strcmp(argv[1],"get")) ftp_get(ftp_sock, file_name);
    else if(!strcmp(argv[1], "put")) ftp_put(ftp_sock, file_name);
    else printf("sdfds1111\n");

    closesocket(ftp_sock);
    WSACleanup();

    return 0;
}
