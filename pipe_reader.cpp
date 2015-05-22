#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include<iostream>
#include<unistd.h>
#include <signal.h>
#include <mqueue.h>
#include<cstring>
#include <cstdio>
#include <errno.h>
#include <cstdlib>
using namespace std;
#define MAX_BUF 1024



template<typename T>
int READ(int fd, T* obj_ptr, int count)
{



/*  char data[MAX_BUF];
  memset(data,0,MAX_BUF);
*/
  int number_of_reads=count;
  int data_reads=0;

  char ch;

  while(number_of_reads > 0){

    data_reads+=read(fd,obj_ptr+data_reads,count);
    //data_reads=mq_receive(fd, ptr, 120, NULL);
    number_of_reads=number_of_reads-data_reads;

  }





  //*(obj_ptr+data_reads)='\0';


  return data_reads;
}

int main()
{
    int fd;
    string myfifo = "/tmp/myfifo";
    char buf[MAX_BUF];
    memset(buf, 0, 1000);
    int data_suck=0;
    /* open, read, and display the message from the FIFO */
    fd = open(myfifo.c_str(), O_RDONLY);
    data_suck+=READ(fd, buf+data_suck, 16);
    data_suck+=READ(fd, buf+data_suck, 16);

    for(int i=0;i<data_suck;i++)
      printf("%c",buf[i]);
    close(fd);

    return 0;
}
