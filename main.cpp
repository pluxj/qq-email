#include <iostream>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <cstring>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdint.h>
using namespace std;


int onconnect(char* host,int port)
{
    int sockfd;
    int len;
    struct sockaddr_in address;
    struct hostent* h;
    sockfd = socket(AF_INET,SOCK_STREAM,NULL);

    int length = strlen(host);
    h = gethostbyname(host);

    address.sin_addr.s_addr = *((unsigned long*)h->h_addr_list[0]);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    len = sizeof(address);
    connect(sockfd,(struct sockaddr*)&address,len);
    return sockfd;
}

void recv_msg(int sockfd)
{
    char text[BUFSIZ];
    memset(text,0,sizeof(text));
    recv(sockfd,text,BUFSIZ,0);
    cout << "\n" << "recv: " << text << "\n";
}

void send_msg(int sockfd,const char *msg)
{
    char text[BUFSIZ];
    memset(text,0,sizeof(text));
    cout << "\n" << "senc:" << msg << "\n";
    send(sockfd,msg,strlen(msg),0);
}


void base64_encode(const  char*in,  int inlen,char* out,  int &outlen)
{
    #define BASE64_PAD '='
    static const char base64en[] = {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
            'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
            'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
            'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
            'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
            'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
            'w', 'x', 'y', 'z', '0', '1', '2', '3',
            '4', '5', '6', '7', '8', '9', '+', '/',
    };
    int s;
    unsigned int i;
    unsigned int j;
    unsigned char c;
    unsigned char l;

    s = 0;
    l = 0;
    for (i = j = 0; i < inlen; i++) {
        c = in[i];

        switch (s) {
            case 0:
                s = 1;
                out[j++] = base64en[(c >> 2) & 0x3F];
                break;
            case 1:
                s = 2;
                out[j++] = base64en[((l & 0x3) << 4) | ((c >> 4) & 0xF)];
                break;
            case 2:
                s = 0;
                out[j++] = base64en[((l & 0xF) << 2) | ((c >> 6) & 0x3)];
                out[j++] = base64en[c & 0x3F];
                break;
        }
        l = c;
    }

    switch (s) {
        case 1:
            out[j++] = base64en[(l & 0x3) << 4];
            out[j++] = BASE64_PAD;
            out[j++] = BASE64_PAD;
            break;
        case 2:
            out[j++] = base64en[(l & 0xF) << 2];
            out[j++] = BASE64_PAD;
            break;
    }

    out[j] = 0;
    outlen = j;
}
void disconnect(int sockfd)
{
    close(sockfd);
    cout << "close socket" << endl;
}
int main() {
    char host[] = "smtp.qq.com";   //qq smtp 地址
    int port = 25;
    int sockfd =  onconnect(host,port);    //连接qq邮箱
    char out_ch[BUFSIZ];
    int outlen= 0;
    memset(out_ch,0, sizeof(out_ch));
    recv_msg(sockfd);

    send_msg(sockfd,"helo qq\r\n");    //连接确认

    recv_msg(sockfd);

    send_msg(sockfd,"auth login\r\n");   //账号登陆

    recv_msg(sockfd);
    //输入账号
    char qqlogin[48] ;
    cout << "print QQ_Email:" << '\n';
    cin >> qqlogin;
    base64_encode(qqlogin,strlen(qqlogin),out_ch,outlen);
    strcat(out_ch,"\r\n\0");
    send_msg(sockfd,out_ch);
    recv_msg(sockfd);
    //输入密码
    char qqpwd[48];
    cout << "print QQ_PASSWORD" << '\n';
    cin >> qqpwd;
    memset(out_ch,0,sizeof(out_ch));
    base64_encode(qqpwd,strlen(qqpwd),out_ch,outlen);
    strcat(out_ch,"\r\n\0");
    send_msg(sockfd,out_ch);
    recv_msg(sockfd);

    string FromMail = "MAIL FROM:";
    FromMail += qqlogin ;
    FromMail += "\r\n";
    send_msg(sockfd,FromMail.c_str());

    recv_msg(sockfd);
    //邮件接收人
    string ToMail = "RCPT TO:";
    string str;
    cout << "send mail to:" << '\n';
    cin >> str;
    ToMail += str + "\r\n";
    send_msg(sockfd,ToMail.c_str());

    recv_msg(sockfd);

    //邮件正文
    send_msg(sockfd,"DATA\r\n");
    FromMail = "From:" ;
    FromMail += qqlogin;
    FromMail += "\r\n";
    send_msg(sockfd,FromMail.c_str());

    send_msg(sockfd,"Subject:hello world\r\n\n");

    send_msg(sockfd,"hello world\r\n");

    send_msg(sockfd,"\r\n.\r\n");

    recv_msg(sockfd);

    disconnect(sockfd);

    return 0;
}