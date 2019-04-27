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

//USEFUL for making requests
const char GET[]="GET ";             //start of GET request
const char END_GET[]=" HTTP/1.0\r\n"; //end of GET request, between is the path
const char HOST[]="Host: "; //start of HOST header
const char END_HOST[]="\r\n\r\n"; //end of HOST header

//PASS CODE
const char ACCEPT_200[]="HTTP/1.1 200 OK\n";

//ERROR CODES
const char ERROR_300[]="HTTP/1.1 300 Multiple Choices\n";
const char ERROR_301[]="HTTP/1.1 301 Moved Permanently\n";
const char ERROR_400[]="HTTP/1.1 400 Bad Request\n";
const char ERROR_403[]="HTTP/1.1 403 Forbidden\n";
const char ERROR_404[]="HTTP/1.1 404 Not Found\n";
const char ERROR_500[]="HTTP/1.1 500 Internal Server Error\n";

int main(int argc, char *argv[]) {
  short port=80;                 //the port we are connecting on

  struct addrinfo *result;       //to store results
  struct addrinfo hints;         //to indicate information we want

  struct sockaddr_in *saddr_in;  //socket interent address
  struct sockaddr_in addr = {};  //not a pointer but is a specific internet address

  int s;                       //for error checking
  int n;                       //for error checking

  int sock;                      //socket file descriptor
  // int fd;                        //output file

  // char response[BUF_SIZE];           //read in BUF_SIZE byte chunks

  char* hostname = argv[1]; //stores user input of the address we seek to lookup
  char* path = argv[2]; //the path of the file we want most likely
  char request[1096]; //the base string that we will build the request off of, 1096 is arbitrary
  int msglen; //message length
  char response[4096]; //read in 4096 byte chunks
  // char* edited_response;



  if (argc < 3) {
    fprintf(stderr, "ERROR: Require domain and path\n");
    fprintf(stderr, "%s", USAGE);
    exit(1);
  }

  //usage for generic port 80
  if (argc < 2) {
    fprintf(stderr, "ERROR: Require domain\n");
    fprintf(stderr, "%s", USAGE);
    exit(1);
  }

  //retrieve the port if provided
  if (argc == 3) {
    port = atoi(argv[2]);
    // printf("we have been given a path: %s\n", path);
  }

  //retrieve the port if provided
  if (argc > 3) {
    if ((port = atoi(argv[2])) == 0) {
      fprintf(stderr, "ERROR: invlaid port number\n");
      fprintf(stderr, "%s", USAGE);
      exit(1);
    }
    // printf("updating the port number because we see some input\n");
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
  //add the port to the inet socket address, being sure to convert to network form
  // saddr_in->sin_port = htons(port);

  //open and connect the socket
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("socket");
    return 1;
  }

  // socket is created, but not yet open
  // printf("socket created\n");

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
  printf("socket connected\n");
  printf("%s has address %s\n", hostname, inet_ntoa(saddr_in->sin_addr));
  printf("current addr has address %s\n", inet_ntoa(addr.sin_addr)); //these are printing out the same stuff and I'm not sure how addr got the ip address but okay cool
  printf("port we are using is called %d\n", port);
  printf("current addr has port %d\n", ntohs(addr.sin_port)); //network = the way things are stored in the struct and host is the way things are on my local machine

  //update the request string so that it has the proper formatting
  strcpy(request, GET);
  strcat(request, path);
  strcat(request, END_GET);
  strcat(request, HOST);
  strcat(request, hostname);
  strcat(request, END_HOST);
  // printf("request now looks like:\n%s\n", request);

  //send the get request and host header for the path to the port socket we have established
  msglen = strlen(request); // we need to specify how many bytes to send
  if (write(sock, request, msglen) != msglen) {
    fprintf(stderr, "ERROR writing the message to the socket\n");
  }
  // printf("message sent\n");

  //read the response, check the code (use strcmp on just the numeric part)
  while( (n = read(sock, response, 4096)) > 0){
    //print the response string or error to standard out
    if(write(1, response, n) < 0){
      perror("write");
      exit(1);
    }
  }
  if (n<0){ perror("read error"); }


  //skip past the response headers & CHECK FOR ERRORS?
  //*****could possibly make an error checking function that handles everything??? if I have time
  if(strncmp(response+9, "300", 3) == 0){
    fprintf(stderr, "%s", ERROR_300);
    exit(1);
  }
  else if(strncmp(response+9, "301", 3) == 0){
    fprintf(stderr, "%s", ERROR_301);
    exit(1);
  }
  else if(strncmp(response+9, "400", 3) == 0){
    fprintf(stderr, "%s", ERROR_400);
    exit(1);
  }
  else if(strncmp(response+9, "403", 3) == 0){
    fprintf(stderr, "%s", ERROR_403);
    exit(1);
  }
  else if(strncmp(response+9, "404", 3) == 0){
    fprintf(stderr, "%s", ERROR_404);
    exit(1);
  }
  else if(strncmp(response+9, "500", 3) == 0){
    fprintf(stderr, "%s", ERROR_500);
    exit(1);
  }
  else {
    //prints out HTTP/1.1 200 OK
    fprintf(stderr, "%s\n", ACCEPT_200);
  }

  // look for the first blank line
  int i;
  for(i = 0; i < strlen(response); i++){
    if((strncmp(response+i, "\r\n\r\n", 4)) == 0){//4 is the number of bytes of the thing we are checking it agasint, like above it is 3
      i+=4;
      break;
    }
  }

  //write the rest of the server response to the basename of the path
  //open the file path
  FILE* fp = fopen(basename(path), "w");
  for(int j = i; j < strlen(response); j++){
    fputc(response[j], fp);
  }

  //cleanup by closing files
  close(sock);
  fclose(fp);
  // printf("socket closed\n");

  return 0; //success
}



/**
our program is going to have to open the socket and send:
GET /~bilzor/saturn.txt //thing we want to download
Host: faculty.cs.usna.edu
<blank line>
<server sends back the file>
intermediate steps: (print something out after each step to make sure it works properly)
   -deal with command line arguments (done in starter code)
   -lookup ip address for given domain name (did this in networking lab)
   -create socket and connect (port 80 = http)
   -send the get request (first 3 lines need to be written to the socket)
   -read the response and print to stdout (for checking)
make sure each step works before you move onto the next one (this is also how the final project will work)

**/
