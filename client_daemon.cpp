/*
@author - Ganesh Joshi
CSCI 611 Project - 4
Client Daemon

*/

#include<fcntl.h>
#include<sys/stat.h>
#include <sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<unistd.h> //for read/write
#include<string.h> //for memset
#include<stdio.h> //for fprintf, stderr, etc.
#include<stdlib.h> //for exit
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <errno.h>
#include <fstream>
#include <semaphore.h>
#include<signal.h>
#include <mqueue.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <vector>
#include <utility>

using namespace std;

struct mapstruct
{
  int rows;
  int columns;
  unsigned char list;
  int daemonID;
  pid_t pids[5];
  unsigned char data[0];

};

//Global ptrs
mapstruct *map;
unsigned char *client_localcopy=NULL;
int sockfd; //file descriptor for the socket
int status; //for error checking

template<typename T>
int READ(int fd, T* obj_ptr, int count)
{

  int number_of_reads=count;
  int data_reads=0;

  char ch;

  while(number_of_reads > 0){

    data_reads+=read(fd,obj_ptr+data_reads,count);
    //data_reads=mq_receive(fd, ptr, 120, NULL);
    number_of_reads=number_of_reads-data_reads;

  }

  return data_reads;
}

template<typename T>
int WRITE(int fd, T* obj_ptr, int count)
{
  char* addr=(char*)obj_ptr;
  int bytes_sent=count;
  int written=0;

  while(bytes_sent > 0){
    written+=write(fd,(void*)obj_ptr,count);
    if(written==-1)
    {
      perror("Error\n");
      exit(1);
    }
    bytes_sent=bytes_sent-written;
  }


  return written;

}
/* Signal Handler for SIGHUP*/
void SIGHUP_HANDLER(int z){

  //SIGHUP received , New player added.
  //Get send the new player data to server
  //printf("Inside the SIGHUP\n");
  int list=map->list;
  int cnt=0;
  unsigned char * shared_map;
  int op=0x80;
  unsigned char change;
  shared_map=map->data;
  for(int i=0;i<(map->rows*map->columns);i++)
  {
    if( client_localcopy[i] == shared_map[i] )
      continue;
      else{
        client_localcopy[i]=shared_map[i];
      }

    }

    write(sockfd,&op,sizeof(op));
    cnt=write(sockfd,&list,sizeof(list));

  }

  /* Signal Handler for SIGUSR1*/
  void SIGUSR1_HANDLER(int y){

    //  printf("Map chaged !!\n");
    unsigned char * shared_map;
    //Map change Operation
    int op=0;
    unsigned char change;
    shared_map=map->data;
    for(int i=0;i<(map->rows*map->columns);i++)
    {
      if( client_localcopy[i] == shared_map[i] )
        continue;
        else{

          change=shared_map[i];
          WRITE(sockfd,&op,sizeof(op));
          //Write the index
          WRITE(sockfd,&i,sizeof(int));
          //Write the change data
          WRITE(sockfd,&change,sizeof(char));
          //change the local map
          client_localcopy[i]=shared_map[i];
        }

      }

    }

    /* Signal Handler for SIGUSR2*/
    void SIGUSR2_HANDLER(int x){

    }



    int main()
  {

    /* Initialize the signal handlers*/
    struct sigaction signal_SIGHUP;
    struct sigaction signal_SIGUSR1;
    struct sigaction signal_SIGUSR2;

    signal_SIGHUP.sa_handler=SIGHUP_HANDLER;
    signal_SIGUSR1.sa_handler=SIGUSR1_HANDLER;
    signal_SIGUSR2.sa_handler=SIGUSR2_HANDLER;

    sigemptyset(&signal_SIGHUP.sa_mask);
    sigemptyset(&signal_SIGUSR1.sa_mask);
    sigemptyset(&signal_SIGUSR2.sa_mask);

    signal_SIGHUP.sa_flags=0;
    signal_SIGUSR1.sa_flags=0;
    signal_SIGUSR2.sa_flags=0;

    sigaction(SIGHUP,&signal_SIGHUP,NULL);
    sigaction(SIGUSR1,&signal_SIGUSR1,NULL);
    sigaction(SIGUSR2,&signal_SIGUSR2,NULL);



    int rows=0,columns=0;



    //change this # between 2000-65k before using
    const char* portno="45000";

    
    if(fork()>0) //if I'm the parent
    exit(0);   //then exit. This leaves only the child
    if((setsid())==-1)
    exit(1);
    for(int i=0; i< sysconf(_SC_OPEN_MAX); ++i)
    close(i);
    open("/dev/null", O_RDWR); //fd 0
    open("/dev/null", O_RDWR); //fd 1
    open("/dev/null", O_RDWR); //fd 2

    umask(0);
    chdir("/");
    sleep(10);




    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints)); //zero out everything in structure
    hints.ai_family = AF_UNSPEC; //don't care. Either IPv4 or IPv6
    hints.ai_socktype=SOCK_STREAM; // TCP stream sockets

    struct addrinfo *servinfo;
    //Can use a host name or an ip address
    if((status=getaddrinfo("127.0.0.1", portno, &hints, &servinfo))==-1)
    {
      //fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
      exit(1);
    }
    if((sockfd=socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol))==-1)
    {
      perror("socket");
      exit(1);
    }
    if((status=connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen))==-1)
    {
      perror("connect");
      exit(1);
    }
    //release the information allocated by getaddrinfo()
    freeaddrinfo(servinfo);


    //Local copy of the client daemon

    READ(sockfd,&rows,sizeof(int));
    READ(sockfd,&columns,sizeof(int));

    int totalMap=rows*columns;
    unsigned char temp;


    int shared_mem=shm_open("/ClientsharedMemoryGJ",O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
    ftruncate(shared_mem,sizeof(mapstruct)+(rows*columns));
    map=(mapstruct*) mmap(0,sizeof(mapstruct)+(rows*columns),PROT_WRITE,MAP_SHARED,shared_mem,0);

    unsigned char *dataptr=map->data;

    client_localcopy=(unsigned char*)malloc(sizeof(char)*rows*columns);
    map->rows=rows;
    map->columns=columns;
    map->daemonID=getpid();
    map->list=0x01;

    // Create the local copy of the map(Server will send it's local copy)
    //Also create the shared memory
    for(int i=0;i<(rows*columns);i++)
    {
      read(sockfd,&temp,1);
      client_localcopy[i]=temp;
      dataptr[i]=temp;


    }

    int nbytes;
    int x;
    int index;
    unsigned char change;

    while(1){

      nbytes=read(sockfd,&x,sizeof(int));

      //printf("Bytes read from the server %d %d",nbytes,x);

      if(x==0){
        //TODO   Map refresh is required
        unsigned char *org=map->data;
        //printf("Map has been refreshed!!!\n");
        unsigned char value;
        READ(sockfd,&index, sizeof(index));
        READ(sockfd,&change,sizeof(char));

        client_localcopy[index]=change;
        org[index]=change;

        //Give Signal for refresh
        int *x=map->pids;
        for(int i=0;i<5;i++){
          if(x[i]!=0){
            kill(x[i],SIGINT);

          }

        }
        continue;
      }


      if(x==0x80){
        printf("New player added.\n");
        int l;
        READ(sockfd,&l, sizeof(l));
        map->list|=l;


      }


    }


    //release resources.
    close(sockfd);
    delete(client_localcopy);
    return 0;
  }
