/*
 * peer.c
 *
 * Authors: Ed Bardsley <ebardsle+441@andrew.cmu.edu>,
 *          Dave Andersen
 * Class: 15-441 (Spring 2005)
 *
 * Skeleton for 15-441 Project 2.
 *
 */

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "spiffy.h"
#include "bt_parse.h"
#include "input_buffer.h"
#include "chunk.h"

#define PACKETLEN 1500
#define BUFLEN 100
#define HASHASCIILEN 40
#define HASHBTYELEN 20
#define HEADERLEN 16

typedef struct header_s {
  short magicnum;
  char version;
  char packet_type;
  short header_len;
  short packet_len; 
  u_int seq_num;
  u_int ack_num;
} header_t;  

typedef struct data_packet {
  header_t header;
  char data[BUFLEN];
} data_packet_t;

typedef struct pkt_node
{
  /*bnode is the node in sliding window buffer*/ 
  data_packet_t* pkt;
  struct pkt_node* next;
  struct pkt_node* prev;
}pnode_t;

typedef struct pkt_list
{
  /*blist is the list of the sliding window buffer. Note that this buffer is only applied to data pkt*/
  /*this can be used to buf or other pkt container, e.g. many whohas pkt*/
  int len;
  pnode_t* head;
  pnode_t* tail;
}plist_t;

void init_plist(plist_t **list){
  *list = (plist_t*) malloc(sizeof(plist_t));
  assert(list != NULL);
  (*list)->len = 0;

  (*list)->head = (pnode_t*)malloc(sizeof(pnode_t));
  (*list)->tail = (pnode_t*)malloc(sizeof(pnode_t));
  assert((*list)->head != NULL);
  assert((*list)->tail != NULL);
  (*list)->head->next = (*list)->tail;
  (*list)->head->prev = NULL;
  (*list)->tail->prev = (*list)->head;
  (*list)->tail->next = NULL;

  (*list)->head->pkt = NULL;
}

void push_back_pnode(plist_t *list, data_packet_t* pkt){
  pnode_t* temp = (pnode_t*)malloc(sizeof(pnode_t));
  assert(temp != NULL);
  temp->pkt = pkt;
  temp->prev = list->tail->prev;
  temp->next = list -> tail;
  list->tail->prev->next = temp;
  list->tail->prev = temp;
}

void remove_pnode(plist_t *list, data_packet_t *pkt){
  pnode_t *p;
  for ( p = list->head->next; p != list->tail; p = p->next)
  {
    if(p->pkt == pkt){
      // TODO: use ptr or dereference?
      p->prev->next = p->next;
      p->next->prev = p->prev;
      free(p->pkt);
      free(p);
    }
  }
}

// TODO: add remove head




void peer_run(bt_config_t *config);

int main(int argc, char **argv) {
  bt_config_t config;

  bt_init(&config, argc, argv);

  DPRINTF(DEBUG_INIT, "peer.c main beginning\n");

#ifdef TESTING
  config.identity = 1; // your group number here
  strcpy(config.chunk_file, "chunkfile");
  strcpy(config.has_chunk_file, "haschunks");
#endif

  bt_parse_command_line(&config);

#ifdef DEBUG
  if (debug & DEBUG_INIT) {
    bt_dump_config(&config);
  }
#endif
  
  peer_run(&config);
  return 0;
}


void process_inbound_udp(int sock) {
  #define BUFLEN 1500
  struct sockaddr_in from;
  socklen_t fromlen;
  char buf[BUFLEN];

  fromlen = sizeof(from);
  spiffy_recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *) &from, &fromlen);

  printf("PROCESS_INBOUND_UDP SKELETON -- replace!\n"
	 "Incoming message from %s:%d\n%s\n\n", 
	 inet_ntoa(from.sin_addr),
	 ntohs(from.sin_port),
	 buf);
}


plist_t* make_whohas_pkt(char *chunkfile){
#define PADLEN 4
  FILE* get_chunk_file = fopen(chunkfile, "r");
  assert(get_chunk_file != NULL);

  u_int32_t id;
  char hash_ascii[HASHASCIILEN];
  uint8_t hash_bin[HASHBTYELEN];

  /*First we try reading the get_chunk_file and fit them into pkt*/
  char line[BT_FILENAME_LEN];

  /*Firstly allocate a big enough buf, shrink if not full*/
  char data = calloc(PACKETLEN-PADLEN-HEADERLEN, sizeof(char));
  u_int16_t datalen = 0;
  while (fgets(line, BT_FILENAME_LEN, get_chunk_file) != NULL) {
    if (line[0] == '#') continue;
    assert(sscanf(line, "%d %s", &id, hash_ascii) != 0);
    if(datalen + PADLEN + HASHBTYELEN <= PACKETLEN){
      // case that the chunk can still be fit into last pkt
      ascii2hex(hash_ascii, HASHASCIILEN, hash_bin);
      memcpy(data+datalen, hash_bin, HASHBTYELEN);
      datalen += HASHBTYELEN;
    }else{
      // Unfortuately, we have to add a new pkt
      
    }


  }

  fclose(get_chunk_file);
}


void process_get(char *chunkfile, char *outputfile) {
  printf("PROCESS GET SKELETON CODE CALLED.  Fill me in! I've been doing! (%s, %s)\n", 
	chunkfile, outputfile);

}

void handle_user_input(char *line, void *cbdata) {
  char chunkf[128], outf[128];

  bzero(chunkf, sizeof(chunkf));
  bzero(outf, sizeof(outf));

  if (sscanf(line, "GET %120s %120s", chunkf, outf)) {
    if (strlen(outf) > 0) {
      process_get(chunkf, outf);
    }
  }
}


void peer_run(bt_config_t *config) {
  int sock;
  struct sockaddr_in myaddr;
  fd_set readfds;
  struct user_iobuf *userbuf;
  
  if ((userbuf = create_userbuf()) == NULL) {
    perror("peer_run could not allocate userbuf");
    exit(-1);
  }
  
  if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) == -1) {
    perror("peer_run could not create socket");
    exit(-1);
  }
  
  bzero(&myaddr, sizeof(myaddr));
  myaddr.sin_family = AF_INET;
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  myaddr.sin_port = htons(config->myport);
  
  if (bind(sock, (struct sockaddr *) &myaddr, sizeof(myaddr)) == -1) {
    perror("peer_run could not bind socket");
    exit(-1);
  }
  
  spiffy_init(config->identity, (struct sockaddr *)&myaddr, sizeof(myaddr));
  
  while (1) {
    int nfds;
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(sock, &readfds);
    
    nfds = select(sock+1, &readfds, NULL, NULL, NULL);
    
    if (nfds > 0) {
      if (FD_ISSET(sock, &readfds)) {
	process_inbound_udp(sock);
      }
      
      if (FD_ISSET(STDIN_FILENO, &readfds)) {
	process_user_input(STDIN_FILENO, userbuf, handle_user_input,
			   "Currently unused");
      }
    }
  }
}
