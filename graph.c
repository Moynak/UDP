#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <netdb.h>

char t_stamp[4];
unsigned long global_tim;
char seq_no[2];
char ttl;
int pay_len;

char * creat_pkt(char *pay)
{
    char *temp;
    temp= (char *)malloc((7+pay_len)*sizeof(char));
    int i;
    for(i=0;i<2;i++){temp[i]=seq_no[i];}
    for(i=0;i<4;i++){temp[i+2]=t_stamp[i];}
    for(i=0;i<1;i++){temp[i+6]=ttl;}
    for(i=0;i<pay_len;i++){temp[i+7]=pay[i];}
    return temp;
}

char * get_payload(char *pkt)
{
    int i;
    char *temp;
    temp= (char *)malloc((pay_len)*sizeof(char));
    for(i=0;i<2;i++){seq_no[i]=pkt[i];}
    for(i=0;i<4;i++){t_stamp[i]=pkt[i+2];}
    for(i=0;i<1;i++){ttl=pkt[i+6];}
    for(i=0;i<pay_len;i++){temp[i]=pkt[i+7];}
    return temp;

}


unsigned long current_timestamp() 
{
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    unsigned long milliseconds = te.tv_sec*1000000LL + te.tv_usec; // calculate milliseconds
    //milliseconds -= global_tim;
    printf("microseconds: %lu\n", milliseconds);
    int i=0;
    unsigned long temp=milliseconds;
    for(i=0;i<4;i++)
    {
        t_stamp[3-i]='\0'+temp%256;
        printf("%lu (%llx)\t",temp%256,temp%256);
        //printf("%lu (%llx) == %c (%x)\n",temp%256,temp%256,t_stamp[7-i],t_stamp[7-i]);
        temp/=256;
    }
    return milliseconds;
}

unsigned long different_timestamp() 
{
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    unsigned long milliseconds = te.tv_sec*1000000LL + te.tv_usec; // calculate milliseconds
    //milliseconds -= global_tim;
    unsigned long ii=powl(2,32);
    milliseconds %= ii;
    printf("different microseconds: %lu\n", milliseconds);
    int i=0;
    unsigned long temp=0;
    for(i=0;i<4;i++)
    {
        temp*=256;
        temp+=(int)(t_stamp[i]%0xff);
        if((short)t_stamp[i]<0){temp+=256;}
        printf("%lu (%llx)\t",temp,temp);
        //printf("%lu (%llx) == %c (%x) (%d) (%lld) (unsig= %lu) sizeof char = %d\n",temp,temp,t_stamp[i]%256,t_stamp[i]%256,t_stamp[i]%256,t_stamp[i]%256,(int)t_stamp[i],sizeof(t_stamp[i]));
    }
    //printf("prev=%u\n",temp);
    return milliseconds- temp;
}

int set_ttl(int n)
{
    if(n%2!=0){printf("ttl must be even\n");exit(1);}
    if((n<2)||(n>20)){printf("ttl must be within 2 and 20\n");exit(2);}
    ttl=n%256;
    return 0;
}

int edit_ttl()
{
    int n=(int)ttl;
    n--;
    if(n==0){return 1;}
    ttl=n%256;
    return 0;
}





int main(int argc, char *argv[])
{
    //argv[1]= port, argv[2]= ip, argv[3]=ttl;
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    global_tim = te.tv_sec*1000000LL + te.tv_usec; // calculate microseconds


    int num_pkt=50;
    unsigned long time_spent[num_pkt][atoi(argv[3])/2];
    int sockfd;
    int PORT = atoi(argv[1]);
    socklen_t addr_len;
    int numbytes;
    
    char *recv_buf;
    char *pkt;
    char *recv_pkt;
	struct sockaddr_in servaddr;
    FILE* fptr;
    if(atoi(argv[3])==2){fptr=fopen("foo2.csv","a");}
    else if(atoi(argv[3])==8){fptr=fopen("foo8.csv","a");}
    else if(atoi(argv[3])==16){fptr=fopen("foo16.csv","a");}
    else{fptr=fopen("foo.csv","a");}
    
    if (fptr == NULL)
    {
        printf("\nUnable to open file.\n");exit(3);
    }
	if ((sockfd = socket(AF_INET,SOCK_DGRAM,0)) == -1) 
    {
            perror("client: socket");
			exit(1);
    }
	printf("client socket created\n");
	memset(&servaddr,'\0',sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(PORT);
	inet_pton(AF_INET,argv[2],&(servaddr.sin_addr));
    addr_len = sizeof servaddr;
    for(pay_len=100;pay_len<=1000;pay_len+=100)
    {
        int pktlen=7+pay_len;
        char *payload;
        payload = (char *)malloc(pay_len*sizeof(char));
        recv_pkt= (char *)malloc(pktlen*sizeof(char));
        int count=0;
        int flag=0;
        memset(payload,'\0',sizeof(payload));
        *(payload+0)='u';*(payload+1)='n';*(payload+2)='n';*(payload+3)='d';
        //printf("here 1\n");
        for(count=0;count<num_pkt;count++)
        {
            seq_no[1]=(count+1)/256;
            seq_no[0]=(count+1)%256;
            
            while(1)
            {
                if(flag==0)
                {
                    set_ttl(atoi(argv[3]));
                    current_timestamp();
                    flag++;
                }
                else
                {
                    if(edit_ttl()==1)
                    {
                        printf("end of packet #%d\n\n",count+1);
                        if(1)
                        {
                            fprintf(fptr,"%d,%lu\n",pay_len,time_spent[count][flag-1]);
                        }
                        break;
                    }
                    flag++;
                }
                pkt=creat_pkt(payload);
                if ((numbytes = sendto(sockfd, pkt, pktlen, 0,(struct sockaddr *)&servaddr, sizeof servaddr))==-1) 
                {
                    perror("client: sendto");
                    exit(1);
                }
                printf("client: sent %d bytes to %s\n", numbytes, argv[2]);
                if ((numbytes = recvfrom(sockfd, recv_pkt, pktlen , 0,(struct sockaddr*)&servaddr, &addr_len) )== -1) 
                {
                    perror("recvfrom");
                    exit(1);
                }
                recv_buf=get_payload(recv_pkt);
                time_spent[count][flag-1]=different_timestamp();
                printf("MICRO = ----------------------------------------%lu\n",time_spent[count][flag-1]);
                // if(numbytes==0)
                // {
                //     printf("Server has left");
                //     break;
                // }
                printf("Client: packet from server is %d bytes long\n", numbytes);
                //printf("Client: packet contains \"%s\"\n", recv_buf);
                printf("client: ttl=0x%x\n",ttl);
                free(pkt);
                free(recv_buf);
            }
            flag=0;
        }
        free(payload);
        free(recv_pkt);
    }
    close(sockfd);
    close(fptr);
    return 0;
}