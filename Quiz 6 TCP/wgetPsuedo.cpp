#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    const char* userSelection = argv[1];

    struct sockaddr_in servaddr;
    struct hostent *hp;
    int sock_id;
    char message[1024*1024] = {0};
    int msglen;
    //char request[] = "GET /index.html HTTP/1.0\n"
    char* r1 = "GET /index.html HTTP/1.1\r\nHost: ";// + userSelectionC + "\r\nConnection: close\r\n\r\n";
    char* r3 = "\r\nConnection: Keep-Alive\r\n\r\n";
    "From: UbuntuUser\nUser-Agent: psuedoWget \n\n";

    char * r4 = (char *) malloc(1 + strlen(r1)+ strlen(userSelection) +strlen(r3) );
    strcpy(r4, r1);
    strcat(r4, userSelection);
    strcat(r4, r3);
    printf("%s", r4);
    char* request = r4;

    //socket
    if((sock_id = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr,"Socket Failed\n"); exit(EXIT_FAILURE);
    }
    else {
        fprintf(stderr,"Socket Successful\n");
    }

    memset(&servaddr,0,sizeof(servaddr));

    //gets address from user input from command line
    if((hp = gethostbyname(userSelection)) == NULL) {
        fprintf(stderr,"Host not obtained\n"); exit(EXIT_FAILURE);
    }
    else {
        fprintf(stderr,"Host obtained from address\n");
    }
    memcpy((char *)&servaddr.sin_addr.s_addr, (char *)hp->h_addr, hp->h_length);
    //int port number and type
    servaddr.sin_port = htons(80);
    servaddr.sin_family = AF_INET;
    //make the connection
    if(connect(sock_id, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        fprintf(stderr, "Connection Failed\n");
    }
    else {
        fprintf(stderr,"Connected Successfully\n");
    }
    //HTTP
    write(sock_id,request,strlen(request)); //request
    msglen = read(sock_id,message,1024*1024); //response

    //write response from server to html file
    FILE *fp;
    fp = fopen("contentDownload.html", "w+");
    fputs(message, fp);
    fclose(fp);

    fprintf(stderr,"HTML data written to file.\n");

    return 0;
}
