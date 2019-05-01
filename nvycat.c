/* IC221 Fall 2019, Project03
 * Chloe Bryan m210738
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>
#include <signal.h> //added for signal stuff
#include <sys/signal.h> //addded for my signaling
#include <pthread.h> //added for my threading

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

//global variables I guess
#define BUF_SIZE 4096
int sock;                      //socket file descriptor
char response[BUF_SIZE];
int n;

void exithandler(int signum){
  //cleanup by closing files (sock is a file descriptor)
  close(sock);

  exit(1);
}

void * sock_to_term(void * args){
  // printf("writing from sock to terminal\n");
  if( (n = read(sock, response, BUF_SIZE)) > 0){ //reading from socket, writing to stdout
    //print the response string or error to standard out
    if(write(1, response, n) < 0){
      perror("write");
      exit(1);
    }
  }
  //SPACESHIP IS GETTING HUNG UP HERE
  printf("hang line 42\n");
  printf("what is n? n = %d\n", n);


  if (n<0){ perror("read error"); }
  return NULL;
}

void * term_to_sock(void * args){
  // printf("writing from terminal to socket\n");
  if( (n = read(0, response, BUF_SIZE)) > 0){ //reading from socket, writing to stdout
    //print the response string or error to standard out
    if(write(sock, response, n) < 0){
      perror("write");
      exit(1);
    }
    // printf("hang line 54\n");
  }
  if (n<0){ perror("read error"); }
  return NULL;
}

int main(int argc, char *argv[]) {
  short port=80;                 //the port we are connecting on

  struct addrinfo *result;       //to store results
  struct addrinfo hints;         //to indicate information we want

  struct sockaddr_in *saddr_in;  //socket interent address
  struct sockaddr_in addr = {};  //not a pointer but is a specific internet address

  int s;                       //for error checking
  int n;                       //for error checking


  // char* hostname = argv[1]; //stores user input of the address we seek to lookup
  char request[1096] = {}; //the base string that we will build the request off of, 1096 is arbitrary
  // int server_sock, client_sock;        // Socket file descriptor
  int listen_option = 0;  //flag that lets us know if we are going to listen also, init to 0 bc FALSE



  if (argc < 3) {
    fprintf(stderr, "ERROR: require arguments\n");
    exit(1);
  }

  // //case that we are given -l
  // if(argc > 3 && (strcmp(argv[1],"-l") == 0)){
  //   printf("-l option was given!\n");
  //   listen_option = 1; //true
  // } else {
  //   printf("-l option was NOT given!\n");
  // }


  //retrieve the port if provided
  if (argc > 2 && (listen_option != 1)) {
    if ((port = atoi(argv[2])) == 0) {
      fprintf(stderr, "ERROR: invlaid port number\n");
      exit(1);
    }
  }



  // lookup domain to get socket address
  /**
  we need to get the stuff for this function
  int connect(int sockfd, const struct sockaddr *addr,
                   socklen_t addrlen);
  */
  //set up the hints
  memset(&hints,0,sizeof(struct addrinfo));  //zero out hints
  hints.ai_family = AF_INET; //we only want IPv4 addresses

  //Convert the hostname to an address
  if ((s = getaddrinfo(argv[1], NULL, &hints, &result)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n",gai_strerror(s));
    exit(1);
  }

  //convert generic socket address to inet socket address
  saddr_in = (struct sockaddr_in *) result->ai_addr;

  //open and connect the socket
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("socket");
    return 1;
  }

  // socket is created, but not yet open
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port); // port number, it can be anything you want
  /**
  inet_aton() converts the Internet host address cp from the IPv4 numbers-and-dots notation  into
       binary  form  (in  network  byte  order)  and  stores  it  in the structure that inp points to.
  **/
  inet_aton(inet_ntoa(saddr_in->sin_addr), &addr.sin_addr); // inet_ntoa(saddr_in->sin_addr) == ip address in saddr_in converted to a string

  //make that connection
  if (connect(sock, (struct sockaddr*) &addr, sizeof(struct sockaddr_in)) != 0) {
    perror("connect");
    return 1;
  }

  // //populate request from whatever is in stdin
  // if((n = read(0, request, BUF_SIZE)) < 0){
  //   perror("stdin error");
  // }

  // printf("the request here says: %s\n", request);

  //send request from stdin
  if(write(sock, request,strlen(request)) < 0){
    perror("send");
  }

  /*****************
  threaded section
  *****************/
  signal(SIGINT, exithandler); //if the user presses ^C here I want to make sure everything gets closed out
  while(1){
    pthread_t sock_term, term_sock;
    sleep(0.5);

    //create a new thread have it run the function hello_fun
    pthread_create(&sock_term, NULL, sock_to_term, NULL); //last NULL is for the arguments to the function hello_fun
    pthread_create(&term_sock, NULL, term_to_sock, NULL); //last NULL is for the arguments to the function hello_fun
    //wait until the thread completes
    pthread_join(sock_term, NULL);
    pthread_join(term_sock, NULL);
  }
  /**********
  one thread should read from the socket and write to the terminal (standard out),
  other thread should read from the terminal (standard in) and write to the socket,
  both doing this over and over again in a loop
  ***********/

  return 0; //success
}
