/*
 * bitflood.c
 *
 * Author: Albert Sheu <albert@cmu.edu>
 * Class: 15-441 (Fall 2007)
 *
 * Description: Skeleton for 15-441 Project 2.  Start by editing the
 * TODOs I set for you in process_inbound_udp and register_client.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "bf_parse.h"
#include "debug.h"

#define DEFAULTPORT 20000 /* TODO: set this to your group's values */
#define PORTRANGE 50
#define BUFLEN 1500
#define REGISTRATION_INTERVAL 5

/* Global variables */
int nodeID = -1;
int verbosity;
int maxConn;
bf_flood *flood;;

/* 
   Set the verbosity level, set the node ID, and configure the flood
   specifications.  These values are set as global variables.
*/
int configure(int argc, char** argv)
{
    int i;
    char* floodname;
    if (argc < 2) {
	return -1;
    }
    flood = parse_flood_file(argv[1]);
    for (i = 2; i < argc; i++) {
	if (i == argc - 1) {
	    /* Ignore final parameter */
	} else if (strcmp(argv[i], "-n") == 0) {
	    nodeID = atoi(argv[++i]);
	} else if (strcmp(argv[i], "-m") == 0) {
	    maxConn = atoi(argv[++i]);
	} else if (strcmp(argv[i], "-d") == 0) {
	    verbosity = atoi(argv[++i]);    
	} else {
	    fprintf(stderr, "Invalid parameter: %s\n", argv[i]);
	}
    }
    return (nodeID > 0);
}


/* 
   Starting from DEFAULTPORT, find a suitable port to bind and listen
   on.  Once one is found, addr is filled in with the bound address.
   If no suitable port could be found, returns -1.  Otherwise returns
   the fd it is now listening on.
*/
int configure_listensock(struct sockaddr_in *addr)
{
    int portnum, fd;
    struct hostent *host;  

    char hostname[BUFLEN];
    bzero(hostname, BUFLEN);
    gethostname(hostname, BUFSIZE);
    host = gethostbyname(hostname);

    bzero(addr, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = *(in_addr_t *)host->h_addr;

    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

    for (portnum = DEFAULTPORT; portnum < DEFAULTPORT + PORTRANGE; portnum++) {
	addr->sin_port = htons(portnum);
	if (bind(fd, (struct sockaddr*)addr, sizeof(struct sockaddr_in)) >= 0) {
	    return fd;
	}
    }
    return -1;
}

/* 
   Process incoming packets by stripping them of the Spiffy headers,
   if necessary.
*/
void process_inbound_udp(int sock) 
{
    struct sockaddr_in from;
    socklen_t fromlen;
    char buf[BUFLEN];

    fromlen = sizeof(from);

#ifdef SPIFFY_ROUTING
    spiffy_recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr*) &from, &fromlen);
#else
    recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr*) &from, &fromlen);
#endif

    /* TODO: Implement this here! */
}

/*
  Register the client with the tracker.
*/
int register_client(struct sockaddr_in listenaddr)
{    
    char buffer[BUFLEN];
    int sock, n;

    bzero(buffer, BUFLEN);
    sprintf(buffer, "REGISTER %d %s %d\r\n", 
	    nodeID,
	    inet_ntoa(listenaddr.sin_addr),
	    htons(listenaddr.sin_port));

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sock, (struct sockaddr*)&flood->tracker, sizeof(flood->tracker)) < 0) {
	perror("Error connecting to tracker");
	return -1;
    }
    if (write(sock, buffer, strlen(buffer) + 1) < 0) {
	perror("Error connecting to tracker");
	return -1;
    }
    
    bzero(buffer, BUFLEN);
    n = read(sock, buffer, BUFLEN);
    
    /* TODO: Process peer list here! */
    printf ("Return value from tracker:\n%s", buffer);

    close(sock);
    return 0;
}

/* 
   Start listening to connections.
*/
void start_client()
{
    int listensock, numfds;
    time_t next_register;
    struct timeval timeout;
    struct sockaddr_in listenaddr;
    fd_set readfds;

    listensock = configure_listensock(&listenaddr);
    if (listensock < 0) {
	fprintf(stderr, "Unable to find an open port for connection.\n");
	exit(-1);
    }

#ifdef SPIFFY_ROUTING
    if (spiffy_init(nodeID, (struct sockaddr*)&listenaddr, sizeof(listenaddr)) != 0) {
	return;
    }
#endif

#ifdef DEBUG
    fprintf (stderr, "Listening for connections on %s port %d\n\n", 
	     inet_ntoa(listenaddr.sin_addr), 
	     htons(listenaddr.sin_port));
    
    fprintf (stderr, "Flood file information:\n");
    fprintf (stderr, "  Tracker location: %s:%d\n", 
	     inet_ntoa(flood->tracker.sin_addr),
	     htons(flood->tracker.sin_port));

    fprintf (stderr, "  File name: %s\n", flood->filename);
    fprintf (stderr, "  File size: %d\n", flood->filesize);
    fprintf (stderr, "  Number chunks: %d\n", flood->numchunks);
    fprintf (stderr, "  SHA-1 Chunk[0]: %s\n\n", flood->hashes[0]);
#endif

    register_client(listenaddr);
    next_register = time(NULL) + REGISTRATION_INTERVAL;

    while (1) {	
	timeout.tv_sec = next_register - time(NULL);
	timeout.tv_usec = 0;
	FD_ZERO(&readfds);
	FD_SET(listensock, &readfds);
	numfds = select(listensock+1, &readfds, NULL, NULL, &timeout); 
	if (numfds > 0 && FD_ISSET(listensock, &readfds)) {
	    process_inbound_udp(listensock);
	}
	if (time(NULL) >= next_register) {
	    register_client(listenaddr);
	    next_register = time(NULL) + REGISTRATION_INTERVAL;
	}
    }
}

/* 
   Prints program usage information.
*/
void printUsage(char** argv)
{
    fprintf(stderr, "Usage: %s <filename.flood> -n <node-id> [-m <max-conn>] [-d <debug-level>]\n", argv[0]);
}

int main(int argc, char** argv)
{
    if (configure(argc, argv) < 0) {
	printUsage(argv);
	exit(-1);
    }
    if (DEFAULTPORT < 0) {
	fprintf(stderr, "Please set up your default port values first! (bitflood.c)\n");
	exit(-1);
    }
    start_client();
    return 0;
}
