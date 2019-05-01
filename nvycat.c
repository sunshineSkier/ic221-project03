/* IC221 Fall 2019, Lab 12: Sockets
 * mywget.c: Program to perform an HTTP GET request and save contents to a file.
 * Chloe Bryan m210738
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUF_SIZE 4096

const char USAGE[]="mywget domain path [port]\n"
  "\n"
  "connect to the web server at domain and port, if provided, and request\n"
  "the file at path. If the file exist, save the file based on the\n"
  "filename of value in the path\n"
  "\n"
  "If domain is not reachable, report error\n";

int main(int argc, char *argv[]) {
  short port=80;                 //the port we are connecting on

  struct addrinfo *result;       //to store results
  struct addrinfo hints;         //to indicate information we want

  struct sockaddr_in *saddr_in;  //socket interent address
  struct sockaddr_in addr = {};  //not a pointer but is a specific internet address

  int s;                       //for error checking
  int n;                       //for error checking

  int sock;                      //socket file descriptor

  // char* hostname = argv[1]; //stores user input of the address we seek to lookup
  char request[1096]; //the base string that we will build the request off of, 1096 is arbitrary
  int msglen; //message length
  char response[BUF_SIZE];
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

  //double checking everything
  // printf("socket connected\n");
  // printf("%s has address %s\n", hostname, inet_ntoa(saddr_in->sin_addr));
  // printf("current addr has address %s\n", inet_ntoa(addr.sin_addr)); //these are printing out the same stuff and I'm not sure how addr got the ip address but okay cool
  // printf("port we are using is called %d\n", port);
  // printf("current addr has port %d\n", ntohs(addr.sin_port)); //network = the way things are stored in the struct and host is the way things are on my local machine

  //populate request from whatever is in stdin
  if((n = read(0, request, BUF_SIZE)) < 0){
    perror("stdin error");
  }

  printf("if there is no given stdin, the request looks like: %s", request);

  // printf("checkpoint #1, stdin said: %s\n", request);

  //send request from stdin
  if(write(sock, request,strlen(request)) < 0){
    perror("send");
  }

  //send the get request and host header for the path to the port socket we have established
  msglen = strlen(request); // we need to specify how many bytes to send
  if (write(sock, request, msglen) != msglen) {
    fprintf(stderr, "ERROR writing the message to the socket\n");
  }

  //read the response, check the code (use strcmp on just the numeric part)
  if( (n = read(sock, response, BUF_SIZE)) > 0){
    //print the response string or error to standard out
    if(write(1, response, n) < 0){
      perror("write");
      exit(1);
    }
  }
  if (n<0){ perror("read error"); }

  //cleanup by closing files
  close(sock);

  return 0; //success
}
