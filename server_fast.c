#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <sys/time.h>

#define MAXBUFLEN 1400

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) 
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) 
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) 
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) 
        {
            perror("server: socket");
            continue;
        }
        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }
    if (p == NULL) 
    {
        fprintf(stderr, "server: failed to bind socket\n");
        return 2;
    }
    freeaddrinfo(servinfo);
    char *payload,*pkt;
    while(1)
    {
        printf("server: waiting to recvfrom...\n");
        addr_len = sizeof their_addr;
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,(struct sockaddr *)&their_addr, &addr_len)) == -1) 
        {
            perror("recvfrom");
            exit(1);
        }
        buf[6]--;
        printf("server: got packet from %s\n",inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),s, sizeof s));
        printf("server: packet is %d bytes long\n", numbytes);
        if ((numbytes = sendto(sockfd, buf, numbytes, 0,(struct sockaddr *)&their_addr, sizeof(their_addr)))==-1) 
        {
            perror("server: sendto");
            exit(1);
        }
        printf("server: sent %d bytes to %s\n\n", numbytes, inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),s, sizeof s));
    }
    
    close(sockfd);
    return 0;
}

/*


int main(int argc, char *argv[])
{
    int sockfd;
    int yes=1;
    int numbytes;
    struct sockaddr_in their_addr;
    char buf[MAXBUFLEN];
    struct sockaddr_in servaddr;
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    if ((sockfd = socket(AF_INET,SOCK_DGRAM,0)) == -1) 
    {
            perror("server: socket");
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) 
    {
            perror("setsockopt");
            exit(3);
    }
    memset(&servaddr,'\0',sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons((int)argv[1]);
	inet_pton(AF_INET,"192.168.43.231",&(servaddr.sin_addr));
    if (bind(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) == -1) 
    {
            close(sockfd);
            perror("server: bind");
            exit(2);
    }
    while(1)
    {
        printf("server: waiting to recvfrom...\n");
        addr_len = sizeof their_addr;
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,(struct sockaddr *)&their_addr, &addr_len)) == -1) 
        {
            perror("recvfrom");
            exit(1);
        }
        if(numbytes==0)
        {
            printf("Client has left");
            break;
        }
        printf("server: got packet from %s\n",inet_ntop(AF_INET,&(their_addr.sin_addr),s, sizeof s));
        printf("server: packet is %d bytes long\n", numbytes);
        buf[numbytes] = '\0';
        printf("server: packet contains \"%s\"\n", buf);

        if ((numbytes = sendto(sockfd, buf, strlen(buf), 0,(struct sockaddr *)&their_addr, &addr_len))==-1) 
        {
            perror("server: sendto");
            exit(4);
        }
    }
    close(sockfd);
    return 0;
}

*/