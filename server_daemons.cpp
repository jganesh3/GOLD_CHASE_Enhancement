/*
@author - Ganesh Joshi
CSCI 611 Project 4
Server daemon

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

#define MAP_REFRESH 0
struct mapstruct
{
  int rows;
  int columns;
  unsigned char list;
  int daemonID;
  pid_t pids[5];
  unsigned char data[0];

};


struct mapstruct* map;
int rows;
int columns;
// Initialize the local copy
unsigned char *local_copy;
unsigned char *org;
struct pollfd fds[5];

template<typename T>
int READ(int fd, T* obj_ptr, int count)
{

  int number_of_reads=count;
  int data_reads=0;

  char ch;

  while(number_of_reads > 0){

    data_reads+=read(fd,obj_ptr+data_reads,count);
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
      perror("This is shit\n");
      exit(1);
    }
    bytes_sent=bytes_sent-written;
  }


  return written;

}




/* Signal Handler for SIGHUP*/
void SIGHUP_HANDLER(int z){

  int list=map->list;
  int cnt=0;
  unsigned char * shared_map;
  //int op=0;
  unsigned char change;
  shared_map=map->data;
  for(int i=0;i<(map->rows*map->columns);i++)
  {
    if( local_copy[i] == shared_map[i] )
      continue;
      else{
        local_copy[i]=shared_map[i];
      }

    }

    int opx=0x80;
    for(int j=1;j<5;j++)
    {
      if(fds[j].fd!=-1){
        write(fds[j].fd,&opx,sizeof(int));
        cnt=write(fds[j].fd,&list,sizeof(list));
      }
    }

  }

  /* Signal Handler for SIGUSR1*/
  void SIGUSR1_HANDLER(int y){

    //printf("SIGUSR1 \n");
    unsigned char * shared_map;
    //Map change Operation
    int op=0;
    unsigned char change;
    shared_map=map->data;
    for(int i=0;i<(map->rows*map->columns);i++)
    {
      if( local_copy[i] == shared_map[i] )
        continue;
        else{

          for(int j=1;j<5;j++)
          {

            if(fds[j].fd!=-1){
              change=shared_map[i];
              WRITE(fds[j].fd,&op,sizeof(op));
              //Write the index
              WRITE(fds[j].fd,&i,sizeof(int));
              //Write the change data
              WRITE(fds[j].fd,&change,sizeof(char));
              //change the local map
              local_copy[i]=shared_map[i];
            }

          }
        }

      }


    }

    /* Signal Handler for SIGUSR2*/
    void SIGUSR2_HANDLER(int x){

      //printf("SIGUSR2 \n");


    }


    void refresh(){
      //Give Signal for refresh
      int *x=map->pids;
      for(int i=0;i<5;i++){
        if(x[i]!=0){
          kill(x[i],SIGINT);

        }

      }


    }

    int main()
  {

    int count=1;
    int sid;
    //Start trapping SIGHUP, SIGUSR1, and SIGUSR2
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





    if(fork()>0) //if I'm the parent
    exit(0);   //then exit. This leaves only the child
    //TODO check !
    if(setsid()==-1)
    exit(1);
    for(int i=0; i< sysconf(_SC_OPEN_MAX); ++i)
    close(i);
    open("/dev/null", O_RDWR); //fd 0
    open("/dev/null", O_RDWR); //fd 1
    open("/dev/null", O_RDWR); //fd 2

    umask(0);
    chdir("/");
    sleep(10);

  


    //We now have a daemon
    int sockfd; //file descriptor for the socket
    int status; //for error checking

    /*
    Changes to use poll

    */

    int rc;
    struct sockaddr_in client_addr;
    socklen_t clientSize=sizeof(client_addr);
    int cnt=1;

    //change this # between 2000-65k before using
    const char* portno="45000";
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints)); //zero out everything in structure
    hints.ai_family = AF_UNSPEC; //don't care. Either IPv4 or IPv6
    hints.ai_socktype=SOCK_STREAM; // TCP stream sockets
    hints.ai_flags=AI_PASSIVE; //file in the Ilist|=0x80;P of the server for me

    struct addrinfo *servinfo;
    if((status=getaddrinfo(NULL, portno, &hints, &servinfo))==-1)
    {
      //fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
      exit(1);
    }
    sockfd=socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

    /*avoid "Address already in use" error*/
    int yes=1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))==-1)
    {
      //perror("setsockopt");
      exit(1);
    }

    //We need to "bind" the socket to the port number so that the kernel
    //can match an incoming packet on a port to the proper process
    if((status=bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen))==-1)
    {
      //perror("bind");
      exit(1);
    }
    //when done, release dynamically allocated memory
    freeaddrinfo(servinfo);

    if(listen(sockfd,1)==-1)
    {
      //  perror("listen");
      exit(1);
    }


    int new_sockfd;
    //Initialize the poll
    memset(fds, 0 , sizeof(fds));

    for(int i=0;i<5;i++){
      fds[i].fd=-1;
      fds[i].events=POLLIN;
    }


    fds[0].fd = sockfd;
    fds[0].events = POLLIN;
    int current_size;


    // Read the shared memory and initialize the local copy here
    int shared_mem=shm_open("/sharedMemoryGJ",O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
    read(shared_mem, &rows, sizeof(int));
    read(shared_mem, &columns, sizeof(int));
    map=(mapstruct*) mmap(0,sizeof(mapstruct)+(rows*columns),PROT_WRITE,MAP_SHARED,shared_mem,0);

    local_copy=(unsigned char*)malloc(sizeof(char)*rows*columns);
    //Assig the  daemonID
    map->daemonID=getpid();
    org=map->data;

    int index;
    unsigned char change;

    //Initialize local copy of the map which exist in shared memory
    for(int i=0;i<(rows*columns);i++)
    {

      local_copy[i]=org[i];
    }

    //Here we are done with init. local memory


    bool initflag=false;

    while(1)
    {

      rc = poll(fds, 5, -1);

      if(fds[0].revents!=0){
        for(int i=1;i<5;i++){

          if(fds[i].fd==-1)
          {

            if((fds[i].fd=accept(sockfd, (struct sockaddr*) &client_addr, &clientSize))==-1)
            {
              //perror("accept");

            }

            fds[i].events = POLLIN;
            current_size = i;

            WRITE(fds[i].fd,&rows,sizeof(rows));
            WRITE(fds[i].fd,&columns,sizeof(columns));

            //Send the local copy of server when client connects
            unsigned char *dataptr=local_copy;
            for(int j=0;j<(rows*columns);j++)
            {

              write(fds[i].fd,&dataptr[j],sizeof(dataptr[j]));
            }

            break;



          }

        }

        if(current_size==4)//this means all slot filled up
        {
          close(accept(sockfd, (struct sockaddr*) &client_addr, &clientSize));
        }
      }


      char buffer[100];
      memset(buffer,0,100);
      int n;
      int operation;
      for(int i=1;i<5;++i){



        if(fds[i].revents!=0)
        {

          n=READ(fds[i].fd, &operation, sizeof(int));
          //printf("Received form the client %d",operation);

          if(n==0){
            close(fds[i].fd);
            fds[i].fd=-1;
          }else{

            if(operation==0){
              //This is Map refresh request;
              unsigned char value;
              READ(fds[i].fd,&index, sizeof(index));
              READ(fds[i].fd,&change,sizeof(char));

              local_copy[index]=change;
              org[index]=change;

              refresh();

              continue;
            }

            if(operation == 0x80)
            {
              int l;
              READ(fds[i].fd,&l, sizeof(l));
              map->list|=l;
            }


          }

        }
      }


    }

    //releasing the memory
    delete(local_copy);

  }
