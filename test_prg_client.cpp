
/*
* @Author: Ganesh Joshi
* CSCI 611 Project 4 GoldRush Enhancement
* test_prg_client : Client exe
* Date : 05/12/2015
*/

#include "goldchase.h"
#include "Map.h"
#include<iostream>
#include<sys/mman.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include <fstream>
#include <semaphore.h>
#include<signal.h>
#include <mqueue.h>
using namespace std;

//Global pointer which will point to the Map object
Map* mapper=NULL;


/* Function to refresh the map*/
void screen_refresh(int x){
  //cerr << "refreshed" <<endl;
  if(mapper!=NULL)
    mapper->drawMap();

  }

  void giveSignal(pid_t x[],int player){
    /* Give signal to refresh */
    for(int i=0;i<5;i++){
      if(x[i]!=0){

        kill(x[i],SIGINT);
        //kill(daemonID,SIGUSR1);
      }


    }
  }

  void giveSignalTOdaemon(pid_t x){
    if(x!=0)
      kill(x,SIGUSR1);

    }



    mqd_t readqueue_fd; //message queue file descriptor
    mqd_t writequeue_fd; //message queue file descriptor
    //string mq_name="/GJ_C_player2_mq";
    //string mq_name1="/GJ_C_player2_mq";
    string mq_name="/GJ_player";



    void read_message(int)
  {
    //set up message queue to receive signal whenever
    //message comes in
    struct sigevent mq_notification_event;
    mq_notification_event.sigev_notify=SIGEV_SIGNAL;
    mq_notification_event.sigev_signo=SIGUSR2;
    mq_notify(readqueue_fd, &mq_notification_event);

    //read a message
    int err;
    char msg[121];
    memset(msg, 0, 121);//set all characters to '\0'
    while((err=mq_receive(readqueue_fd, msg, 120, NULL))!=-1)
    {
      //cout << "Message received: " << msg << endl;
      mapper->postNotice(msg);
      memset(msg, 0, 121);//set all characters to '\0'
    }
    //we exit while-loop when mq_receive returns -1
    //if errno==EAGAIN that is normal: there is no message waiting
    if(errno!=EAGAIN)
    {
      perror("mq_receive");
      exit(1);
    }
  }


  void clean_up(int)
{
  //cerr << "Cleaning up message queue" << endl;
  mq_close(readqueue_fd);
  mq_unlink(mq_name.c_str());
  //exit(1);
}


void writeMessage(std::string message,int player){
  //mqd_t writequeue_fd; //message queue file descriptor
  //cerr<<mq_name;
  // cerr<<"player is "<<player;
  if(player == G_PLR0)
    mq_name="/GJ_C_player0_mq";
    else if(player == G_PLR1)
    mq_name="/GJ_C_player1_mq";
    else if(player == G_PLR2)
    mq_name="/GJ_C_player2_mq";
    else if(player == G_PLR3)
    mq_name="/GJ_C_player3_mq";
    else if(player == G_PLR4)
    mq_name="/GJ_player4_mq";

    const char *ptr=message.c_str();

    if((writequeue_fd=mq_open(mq_name.c_str(), O_WRONLY|O_NONBLOCK))==-1)
    {
      perror("mq_open");
      exit(1);
    }
    //cerr << "fd=" << writequeue_fd << endl;
    char message_text[121];
    memset(message_text, 0, 121);
    strncpy(message_text, ptr, 120);
    if(mq_send(writequeue_fd, message_text, strlen(message_text), 0)==-1)
    {
      perror("mq_send");
      exit(1);
    }
    mq_close(writequeue_fd);
  }

  void sendmessage(string queue,string message){

    const char *ptr=message.c_str();
    if((writequeue_fd=mq_open(queue.c_str(), O_WRONLY|O_NONBLOCK))==-1)
    {
      perror("mq_open");
      return;
    }

    char message_text[121];
    memset(message_text, 0, 121);
    strncpy(message_text, ptr, 120);
    if(mq_send(writequeue_fd, message_text, strlen(message_text), 0)==-1)
    {
      perror("mq_send");
      exit(1);
    }
    mq_close(writequeue_fd);
  }

  void broadCast(std::string message,int player){

    if(player & G_PLR0) sendmessage("/GJ_C_player0_mq",message);
    if(player & G_PLR1) sendmessage("/GJ_C_player1_mq",message);
    if(player & G_PLR2) sendmessage("/GJ_C_player2_mq",message);
    if(player & G_PLR3) sendmessage("/GJ_C_player3_mq",message);
  }




  void initializeQ(int player){

    if(player == G_PLR0)
      mq_name="/GJ_C_player0_mq";
      else if(player == G_PLR1)
      mq_name="/GJ_C_player1_mq";
      else if(player == G_PLR2)
      mq_name="/GJ_C_player2_mq";
      else if(player == G_PLR3)
      mq_name="/GJ_C_player3_mq";
      else if(player == G_PLR4)
      mq_name="/GJ_player4_mq";

      /* Initialize the Q readeing stuff here*/
      struct sigaction action_to_take;
      //handle with this function
      action_to_take.sa_handler=read_message;
      //zero out the mask (allow any signal to interrupt)
      sigemptyset(&action_to_take.sa_mask);
      action_to_take.sa_flags=0;
      //tell how to handle SIGINT
      sigaction(SIGUSR2, &action_to_take, NULL);

      struct mq_attr mq_attributes;
      mq_attributes.mq_flags=0;
      mq_attributes.mq_maxmsg=10;
      mq_attributes.mq_msgsize=120;

      if((readqueue_fd=mq_open(mq_name.c_str(), O_RDONLY|O_CREAT|O_EXCL|O_NONBLOCK,
        S_IRUSR|S_IWUSR, &mq_attributes))==-1)
      {
        perror("mq_open");
        exit(1);
      }
      //set up message queue to receive signal whenever message comes in
      struct sigevent mq_notification_event;
      mq_notification_event.sigev_notify=SIGEV_SIGNAL;
      mq_notification_event.sigev_signo=SIGUSR2;
      mq_notify(readqueue_fd, &mq_notification_event);

      ////////////End of Q reading

    }



    //Structure to hold GameBoard
    struct mapstruct
  {
    int rows;
    int columns;
    unsigned char list;
    int daemonID;
    pid_t pids[5];
    unsigned char data[0];


  };

  //Function to move players on the Board
  int movePlayer(int player_position,struct mapstruct *map,int steps,int &foundgold,Map &goldMine,int player);
  //Function to init the game
  mapstruct * gameinit(sem_t* lock,bool isNewGame, int &loc,int shared_mem,int &current_player);

  int main(int argc,char *argv[])
{

  /*=======================================Client test_prg===============================*/
  int shared_mem;
  bool first, second;
  int foundgold;
  sem_t* lock;
  bool isNewGame;
  struct mapstruct* map;
  int P1;
  int currentPlayer;

  if(argv[1]==NULL)
    exit(0);

    /* Changes for setting up signal handler*/
    struct sigaction my_sig_handler;
    my_sig_handler.sa_handler=screen_refresh; //handle with this function
    sigemptyset(&my_sig_handler.sa_mask);
    my_sig_handler.sa_flags=0;
    sigaction(SIGINT, &my_sig_handler, NULL); //tell how to handle SIGINT



    /* initialize random seed: */
    srand (time(0));

    //=========================================================
    //Open semaphore

    lock=sem_open("/goldchaselock",O_RDWR|O_CREAT,S_IRUSR|S_IWUSR,1);
    shared_mem=shm_open("/ClientsharedMemoryGJ",O_RDWR,S_IRUSR|S_IWUSR);

    if(shared_mem == -1 && argv[1]!=NULL){
      //no command line arguments, hence start the server daemon.
      //This is new Game, i.e. Player No 1
      isNewGame=true;
      //initializeQ();


      //TODO
      /*if(fork()>0){
      if(fork()>0)
      exit(0);
      else if(execlp("./client","./client",NULL) == -1)
      perror("Unable to start the server\n");

      sem_wait(lock);
      map=gameinit(lock,isNewGame,P1,shared_mem,currentPlayer);
      sem_post(lock);



    }
    */
  }else{
    //This is not a new Game, i.e. Player no 2..3..4..5
    //Hostname is povided,call client daemon
    isNewGame=false;
    sem_wait(lock);
    map=gameinit(lock,isNewGame,P1,shared_mem,currentPlayer);
    sem_post(lock);

    //Send SIGHUP Signal to daemon.

    kill(map->daemonID,SIGHUP);


  }

  //initialize the map
  Map goldMine(map->data,map->rows,map->columns);

  //Initialize globla pointer
  mapper=&goldMine;

  //Hardcoded
  //string mq_name="/GJ_C_player1_mq";

  int a=0;
  std::string message;
  bool flag=true;
  int curr_pos;
  int send_to;
  int to;
  int who;
  while(flag)
  {

    a=goldMine.getKey();
    switch(a){

      case 104: //goldMine.postNotice("H key pressed!! //Moving Left");
      //Move the pointer left -1

      sem_wait(lock);
      curr_pos=P1;
      P1=movePlayer(curr_pos,map,-1,foundgold,goldMine,currentPlayer);
      sem_post(lock);
      goldMine.drawMap();
      break;

      case 106: //goldMine.postNotice("J key pressed!! //Moving down");
      // when we move down, it will be -80
      sem_wait(lock);
      curr_pos=P1;
      P1=movePlayer(curr_pos,map,-map->columns,foundgold,goldMine,currentPlayer);
      sem_post(lock);
      goldMine.drawMap();
      break;
      case 107: //goldMine.postNotice("K key pressed!! //Moving up");
      //when we move up it will be +80
      sem_wait(lock);
      curr_pos=P1;
      P1=movePlayer(curr_pos,map,map->columns,foundgold,goldMine,currentPlayer);
      sem_post(lock);
      goldMine.drawMap();
      break;

      case 108: //goldMine.postNotice("l key pressed!! //Moving right");
      //when we move right it will be +1
      sem_wait(lock);
      curr_pos=P1;
      P1=movePlayer(curr_pos,map,1,foundgold,goldMine,currentPlayer);
      sem_post(lock);
      goldMine.drawMap();
      break;


      case 113:  flag=false; //Q key Press
      map->data[P1]=0;
      map->list &= ~currentPlayer;
      kill(map->daemonID,SIGHUP);
      giveSignal(map->pids,1);
      giveSignalTOdaemon(map->daemonID);
      if(currentPlayer == G_PLR0) map->pids[0]=0;
      else if(currentPlayer== G_PLR1) map->pids[1]=0;
      else if(currentPlayer == G_PLR2) map->pids[2]=0;
      else if(currentPlayer == G_PLR3) map->pids[3]=0;
      else if(currentPlayer == G_PLR4) map->pids[4]=0;
      clean_up(1);
      break;

      case 81:   flag=false; //Q key Press
      map->data[P1]=0;
      map->list &= ~currentPlayer;
      kill(map->daemonID,SIGHUP);
      giveSignal(map->pids,1);
      giveSignalTOdaemon(map->daemonID);
      if(currentPlayer == G_PLR0) map->pids[0]=0;
      else if(currentPlayer== G_PLR1) map->pids[1]=0;
      else if(currentPlayer == G_PLR2) map->pids[2]=0;
      else if(currentPlayer == G_PLR3) map->pids[3]=0;
      else if(currentPlayer == G_PLR4) map->pids[4]=0;
      clean_up(1);
      break;

      case 109: //send message
      sem_wait(lock);
      send_to=map->list;
      send_to &=~currentPlayer;
      to=goldMine.getPlayer(send_to);
      who=currentPlayer;
      if(currentPlayer ==4) who=3;
      if(currentPlayer >=8) who=4;
      message="Player"+ std::to_string(who)+" Says ";
      message=message+goldMine.getMessage();
      writeMessage(message,to);
      sem_post(lock);
      break;


      case 98:  sem_wait(lock);
      send_to=map->list;
      send_to &=~currentPlayer;
      message="Player"+ std::to_string(currentPlayer)+" Says ";
      message=message+goldMine.getMessage();
      broadCast(message,send_to);
      sem_post(lock);
      break;

      default:  //goldMine.postNotice("Wrong Key !!");
      break;

    }


  }

  //Unlink shared memory only if the no player present
  if(map->list==0){
    mq_close(readqueue_fd);
    mq_unlink(mq_name.c_str());
    shm_unlink("/ClientsharedMemoryGJ");
    sem_unlink("goldchaselock");
  }
}


mapstruct * gameinit(sem_t* lock,bool isNewGame, int &loc,int shared_mem,int &current_player){

  string tempbuffer;
  string data="";
  char *map_ptr;
  int numberofgolds;
  int rows=0;
  int columns=0;
  mapstruct *map;
  ifstream fp;
  /* Added changes to keep the pid */
  pid_t pid= getpid();

  if(isNewGame){

    fp.open("mymap.txt", std::ifstream::in);
    int num_test;
    getline(fp,tempbuffer);
    numberofgolds=atoi(tempbuffer.c_str());

    shared_mem=shm_open("/ClientsharedMemoryGJ",O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
    ftruncate(shared_mem,sizeof(mapstruct)+(rows*columns));
    map=(mapstruct*) mmap(0,sizeof(mapstruct)+(rows*columns),PROT_WRITE,MAP_SHARED,shared_mem,0);

    char c;
    unsigned char *dataptr=map->data;
    map->pids[0]=pid;
    //if this is new game

    if(fp.is_open()){
      while(!fp.get(c).eof()){

        if(c=='\000')
          break;
          if(c ==' '){ *dataptr=0;}
            if(c=='*') {*dataptr=G_WALL;}
              if(c!='\n' ){
                columns++;
                ++dataptr;
              }else{
                rows++;
              }

            }


          }


          map->rows=rows;
          map->columns=columns/rows;
          map->list=G_PLR0;

          // Now at this position call initializeQ
          initializeQ(G_PLR0);

          fp.close();

          int random;
          bool chk=true;
          //if this is new Game, then only drop Gold on Map
          fp.close();
          bool goldPloted=false;
          int cnt=numberofgolds-1;
          if(numberofgolds==0)
            chk=false;
            while(chk) {
              random = rand() % (map->rows*map->columns);
              if(map->data[random] == 0) {
                if(!goldPloted)
                {
                  map->data[random]=G_GOLD;
                  goldPloted=true;
                  continue;
                }
                if(cnt!=0)
                {
                  map->data[random]=G_FOOL;
                  cnt--;
                }else if(cnt==0)
                chk=false;

              }


            }

            chk=true;
            while(chk) {
              random = rand() % (map->rows*map->columns);

              if(map->data[random] == 0) {
                map->data[random]=G_PLR0;
                current_player=G_PLR0;
                loc=random;
                chk=false;
              }


            }



          }

          if(!isNewGame){

            shared_mem=shm_open("/ClientsharedMemoryGJ",O_RDWR,S_IRUSR|S_IWUSR);
            read(shared_mem, &rows, sizeof(int));
            read(shared_mem, &columns, sizeof(int));
            map=(mapstruct*) mmap(0,sizeof(mapstruct)+(rows*columns),PROT_WRITE,MAP_SHARED,shared_mem,0);

            //If not a new Game, find the next player
            if(!(map->list & G_PLR0)){
              current_player=G_PLR0;
              map->pids[0]=pid;
              map->list|=G_PLR0;
              initializeQ(G_PLR0);
            }
            else if(!(map->list & G_PLR1)){
              current_player=G_PLR1;
              map->pids[1]=pid;
              map->list|=G_PLR1;
              initializeQ(G_PLR1);
            }else  if(!(map->list & G_PLR2)){
              current_player=G_PLR2;
              map->pids[2]=pid;
              map->list|=G_PLR2;
              initializeQ(G_PLR2);
            }else if(!(map->list & G_PLR3)){
              current_player=G_PLR3;
              map->pids[3]=pid;
              map->list|=G_PLR3;
              initializeQ(G_PLR3);
            }else  if(!(map->list & G_PLR4)){
              current_player=G_PLR4;
              map->pids[4]=pid;
              map->list|=G_PLR4;
              initializeQ(G_PLR4);
            }

            int random;
            bool chk=true;


            while(chk) {
              random = rand() % (map->rows*map->columns);

              if(map->data[random] == 0) {
                map->data[random]=current_player;
                loc=random;
                chk=false;
              }


            }


          }

          giveSignalTOdaemon(map->daemonID);
          return map;

        }


        /*
        This function will be used to move the player position
        arguments: current position of the player, pointer to the shared
        memory
        */
        int movePlayer(int player_position,struct mapstruct *map,int steps,int &foundgold,Map &goldMine,int player){


          int x_position,y_position;

          // Get the player position
          y_position=player_position % map->columns;
          x_position=player_position / map->columns;

          //Moving Left
          if(steps == -1){
            if(map->data[player_position+steps]!=G_WALL
              && map->data[player_position+steps]!=G_PLR0
              && map->data[player_position+steps]!=G_PLR1
              && map->data[player_position+steps]!=G_PLR2
              && map->data[player_position+steps]!=G_PLR3
              && map->data[player_position+steps]!=G_PLR4
              && y_position+steps !=0){

                //check for the Gold
                if(map->data[player_position+steps] & G_GOLD){
                  foundgold=1;
                  goldMine.postNotice("You Found the Real Gold !!!");
                  //map->data[player_position]=0;
                  map->data[player_position]&=~player;
                  map->data[player_position+steps]|=player;
                  giveSignal(map->pids,player);
                  giveSignalTOdaemon(map->daemonID);
                  return player_position+steps;

                }

                if(map->data[player_position+steps] & G_FOOL){
                  //foundgold=0;
                  goldMine.postNotice("You Found the FOOL Gold !!!");
                  //map->data[player_position]=0;
                  map->data[player_position]&=~player;
                  map->data[player_position+steps]|=player;
                  giveSignal(map->pids,player);
                  giveSignalTOdaemon(map->daemonID);
                  return player_position+steps;


                }else{



                  //map->data[player_position]=0;
                  map->data[player_position]&=~player;
                  map->data[player_position+steps]|=player;
                  giveSignal(map->pids,player);
                  giveSignalTOdaemon(map->daemonID);
                  return player_position+steps;

                }
              }else if(foundgold==1 && map->data[player_position+steps]!=G_WALL){
                map->data[player_position]&=~player;
                giveSignal(map->pids,player);
                giveSignalTOdaemon(map->daemonID);
                goldMine.postNotice("You Win !!!");
              }
            }


            //Moving Right
            if(steps == 1){
              if(map->data[player_position+steps]!=G_WALL
                && map->data[player_position+steps]!=G_PLR0
                && map->data[player_position+steps]!=G_PLR1
                && map->data[player_position+steps]!=G_PLR2
                && map->data[player_position+steps]!=G_PLR3
                && map->data[player_position+steps]!=G_PLR4
                && y_position+1 < map->columns){


                  if(map->data[player_position+steps] & G_GOLD){
                    foundgold=1;
                    goldMine.postNotice("You Found the Real Gold !!!");

                    map->data[player_position]&=~player;
                    map->data[player_position+steps]|=player;
                    giveSignal(map->pids,player);
                    giveSignalTOdaemon(map->daemonID);
                    return player_position+steps;

                  } if(map->data[player_position+steps] & G_FOOL){
                    //foundgold=0;
                    goldMine.postNotice("You Found the FOOL Gold !!!");

                    map->data[player_position]&=~player;
                    map->data[player_position+steps]|=player;
                    giveSignal(map->pids,player);
                    giveSignalTOdaemon(map->daemonID);
                    return player_position+steps;


                  }




                  else{
                    //map->data[player_position]=0;
                    map->data[player_position]&=~player;
                    map->data[player_position+steps]|=player;
                    giveSignal(map->pids,player);
                    giveSignalTOdaemon(map->daemonID);
                    return player_position+steps;
                  }
                }  else if(foundgold==1 && map->data[player_position+steps]!=G_WALL){
                  map->data[player_position]&=~player;
                  giveSignal(map->pids,player);
                  giveSignalTOdaemon(map->daemonID);
                  goldMine.postNotice("You Win !!!");
                }
              }


              //Moving up
              if(steps == 80){
                x_position=(player_position+steps) / map->columns;
                //cout<<x_position;
                if(map->data[player_position+steps]!=G_WALL
                  && map->data[player_position+steps]!=G_PLR0
                  && map->data[player_position+steps]!=G_PLR1
                  && map->data[player_position+steps]!=G_PLR2
                  && map->data[player_position+steps]!=G_PLR3
                  && map->data[player_position+steps]!=G_PLR4
                  && x_position != map->rows){


                    if(map->data[player_position+steps] & G_GOLD){
                      foundgold=1;
                      goldMine.postNotice("You Found the Real Gold !!!");
                      //map->data[player_position]=0;
                      map->data[player_position]&=~player;
                      map->data[player_position+steps]|=player;
                      giveSignal(map->pids,player);
                      giveSignalTOdaemon(map->daemonID);
                      return player_position+steps;

                    } if(map->data[player_position+steps] & G_FOOL){
                      //foundgold=0;
                      goldMine.postNotice("You Found the FOOL Gold !!!");
                      //map->data[player_position]=0;
                      map->data[player_position]&=~player;
                      map->data[player_position+steps]|=player;
                      giveSignal(map->pids,player);
                      giveSignalTOdaemon(map->daemonID);
                      return player_position+steps;


                    }else{
                      //map->data[player_position]=0;
                      map->data[player_position]&=~player;
                      map->data[player_position+steps]|=player;
                      giveSignal(map->pids,player);
                      giveSignalTOdaemon(map->daemonID);
                      return player_position+steps;


                    }





                  }else if(foundgold==1 && map->data[player_position+steps]!=G_WALL){
                    map->data[player_position]&=~player;
                    giveSignal(map->pids,player);
                    giveSignalTOdaemon(map->daemonID);
                    goldMine.postNotice("You Win !!!");
                  }
                }

                //Moving Down
                if(steps == -80){
                  x_position=(player_position+steps) / map->columns;
                  if(map->data[player_position+steps]!=G_WALL
                    && map->data[player_position+steps]!=G_PLR0
                    && map->data[player_position+steps]!=G_PLR1
                    && map->data[player_position+steps]!=G_PLR2
                    && map->data[player_position+steps]!=G_PLR3
                    && map->data[player_position+steps]!=G_PLR4
                    && x_position != 0){

                      if(map->data[player_position+steps] & G_GOLD){
                        foundgold=1;
                        giveSignal(map->pids,player);
                        giveSignalTOdaemon(map->daemonID);
                        return player_position+steps;

                      } if(map->data[player_position+steps] & G_FOOL){
                        //foundgold=0;
                        goldMine.postNotice("You Found the FOOL Gold !!!");
                        //map->data[player_position]=0;
                        map->data[player_position]&=~player;
                        map->data[player_position+steps]|=player;
                        giveSignal(map->pids,player);
                        giveSignalTOdaemon(map->daemonID);
                        return player_position+steps;


                      }else{


                        //map->data[player_position]=0;
                        map->data[player_position]&=~player;
                        map->data[player_position+steps]|=player;
                        giveSignal(map->pids,player);
                        giveSignalTOdaemon(map->daemonID);
                        return player_position+steps;


                      }

                    }else if(foundgold==1 && map->data[player_position+steps]!=G_WALL){
                      map->data[player_position]&=~player;
                      giveSignal(map->pids,player);
                      giveSignalTOdaemon(map->daemonID);
                      goldMine.postNotice("You Win !!!");
                    }
                  }





                  return player_position;




                }
