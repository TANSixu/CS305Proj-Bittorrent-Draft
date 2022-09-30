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
#include <assert.h>
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
#define BUFLEN PACKETLEN
#define HASHASCIILEN 40
#define HASHBTYELEN 20
#define HEADERLEN 16
#define MAGIC 15441
#define PADLEN 4

bt_config_t config;
int sock;

typedef struct header_s {
  short magicnum;
  uint8_t version;
  uint8_t packet_type;
  uint16_t header_len;
  uint16_t packet_len; 
  uint32_t seq_num;
  uint32_t ack_num;
} header_t;  

header_t* make_header(uint8_t type, uint16_t pkt_len, uint32_t seq, uint32_t ack){
  // need to converto to network order!!
  header_t* my_header = malloc(sizeof(header_t));
  my_header->magicnum = htons(MAGIC);
  my_header->version = 1;
  my_header->packet_type = type;
  my_header->header_len = htons(HEADERLEN);
  my_header->seq_num = htonl(seq);
  my_header->ack_num = htonl(ack);
  return my_header;
}


typedef struct data_packet {
  header_t header;
  char data[PACKETLEN-HEADERLEN];
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
  list->len += 1;
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

  printf("PROCESS_INBOUND_UDP SKELETON -- replace I've been doing!!\n"
	 "Incoming message from %s:%d\n\n", 
	 inet_ntoa(from.sin_addr),
	 ntohs(from.sin_port));
  
  // Step1. parse udp pkt
  // TODO: check the integrity of pkt
  // TODO: If vary pkt length, need to read header first
  data_packet_t* recv_pkt = (data_packet_t*)buf;
  switch (recv_pkt->header.packet_type)
  {
  case 0:{
    // received who has pkt
    // Step1. get the numebr of chunks
    uint8_t chunknums;
    memcpy(&chunknums, (recv_pkt->data), sizeof(uint8_t));
    printf("The chunk in received pkt is %d\n", chunknums);

    // Step2. check all hashes
    char hash_ascii[HASHASCIILEN];
    char hash_byte[HASHBTYELEN];
    uint8_t haschunk_cnt = 0;
    char data[chunknums*HASHBTYELEN];
    for(int i=0; i < chunknums; ++i){
      memcpy(hash_byte, recv_pkt->data+PADLEN+i*HASHBTYELEN, sizeof(uint8_t)*HASHBTYELEN);
      hex2ascii(hash_byte, HASHBTYELEN, hash_ascii);
      for(bt_haschunks_t* hc = config.haschunks; hc != NULL; hc = hc->next){
        printf("I have chunk: %s, pkt: %s\n", hc->chunk_hash, hash_ascii);
        if(strncmp(hc->chunk_hash, hash_ascii, HASHASCIILEN) == 0){
          memcpy(data+haschunk_cnt*HASHBTYELEN, hash_byte, HASHBTYELEN);
          haschunk_cnt++;
        }
      }
    }
    // Step3. send Ihave if cnt if not zero
    if(haschunk_cnt != 0){
      data_packet_t *ihave_pkt = malloc(sizeof(data_packet_t));
      header_t* header = make_header(1, haschunk_cnt*HASHBTYELEN+HASHBTYELEN+HEADERLEN, 1, 1);
      ihave_pkt->header = *header;
      memcpy(ihave_pkt->data, &haschunk_cnt, sizeof(uint8_t));
      memset(ihave_pkt->data+1, 0, 3*sizeof(uint8_t));
      memcpy(ihave_pkt->data+PADLEN, data, haschunk_cnt*HASHBTYELEN);
      sendto(sock, ihave_pkt, PACKETLEN, 0, &from, fromlen);
    }
  } break;

  case 1:{
    // received IHAVE pkt
    // Step1. get the chunknums
    uint8_t chunknums;
    memcpy(&chunknums, (recv_pkt->data), sizeof(uint8_t));

    // Step2. get hashes and send GET pkt
    char hash_byte[HASHBTYELEN];
    char hash_ascii[HASHASCIILEN];
    char data[HASHBTYELEN];
    for(int i=0; i < chunknums; ++i){
      memcpy(hash_byte, recv_pkt->data+PADLEN+i*HASHBTYELEN, sizeof(uint8_t)*HASHBTYELEN);
      header_t* header_get = make_header(2, HEADERLEN+HASHBTYELEN, 0, 0);
      data_packet_t* pkt_get = malloc(sizeof(data_packet_t));
      pkt_get->header = *header_get;
      memcpy(pkt_get->data, hash_byte, HASHBTYELEN);

      binary2hex(hash_byte, HASHBTYELEN, hash_ascii);
      printf("sending GET: %s\n", hash_ascii);
      sendto(sock, pkt_get, PACKETLEN, 0, &from, fromlen);
    }
  }break;

  case 2:{
    // received GET pkt
    // Step1. get hash_btye from pkt
    char hash_byte[HASHBTYELEN];
    char hash_ascii[HASHASCIILEN];
    memcpy(hash_byte, recv_pkt->data, sizeof(uint8_t)*HASHBTYELEN);
    hex2ascii(hash_byte, HASHBTYELEN, hash_ascii);
    printf("receiving GET %s\n", hash_ascii);
  }break;
  
  default:
    break;
  }
}


plist_t* make_whohas_pkt(char *chunkfile){
  FILE* get_chunk_file = fopen(chunkfile, "r");
  assert(get_chunk_file != NULL);

  u_int32_t id;
  char hash_ascii[HASHASCIILEN];
  uint8_t hash_bin[HASHBTYELEN];

  /*First we try reading the get_chunk_file and fit them into pkt*/
  char line[BT_FILENAME_LEN];

  /*Firstly allocate a big enough buf, shrink if not full*/
  char* data = calloc(PACKETLEN-HEADERLEN, sizeof(char));
  u_int8_t chunknum = 0;
  char* pad = calloc(3, sizeof(uint8_t));

  //make a list to store all pkt
  plist_t* list;
  init_plist(&list);

  while (fgets(line, BT_FILENAME_LEN, get_chunk_file) != NULL) {
    if (line[0] == '#') continue;
    assert(sscanf(line, "%d %s", &id, hash_ascii) != 0);
    ascii2hex(hash_ascii, HASHASCIILEN, hash_bin);
    memcpy(data+chunknum*HASHBTYELEN+PADLEN, hash_bin, HASHBTYELEN);
    chunknum += 1;

    // Case that pkt can fit more chunk hashes
    if(chunknum*HASHBTYELEN + PADLEN + HASHBTYELEN <= PACKETLEN) continue;

    // Unfortuately, we have to add a new pkt
    //Step1. produce a pkt for last data
    // Step 1.1 make header
    header_t* pkt_header = make_header(0, chunknum*HASHBTYELEN+PADLEN, 1, 0);
    data_packet_t* pkt = malloc(sizeof(data_packet_t));
    pkt->header = *pkt_header;
    // Step 1.2 make data pkt pad
    memcpy(data, &chunknum, sizeof(uint8_t));
    memcpy(data+sizeof(uint8_t), pad, 3*sizeof(uint8_t));
    memcpy(pkt->data, data, PACKETLEN-HEADERLEN);
    // Step 1.3 add into list
    push_back_pnode(list, pkt);

    //Step 2 clean pkt and prepare for next datapkt
    free(data);
    free(pkt_header);
    chunknum = 0;
  }
  // Default add last pkt
  //Step1. produce a pkt for last data
  // Step 1.1 make header
  header_t* pkt_header = make_header(0, chunknum*HASHBTYELEN+PADLEN+HEADERLEN, 1, 0);
  data_packet_t* pkt = malloc(sizeof(data_packet_t));
  pkt->header = *pkt_header;
  // Step 1.2 make data pkt pad
  memcpy(data, &chunknum, sizeof(uint8_t));
  memcpy(data+sizeof(uint8_t), pad, 3*sizeof(uint8_t));
  memcpy(pkt->data, data, PACKETLEN-HEADERLEN);
  // Step 1.3 add into list
  push_back_pnode(list, pkt);

  //Step 2 clean pkt and prepare for next datapkt
  free(data);
  free(pkt_header);
  chunknum = 0;

  fclose(get_chunk_file);
  return list;
}

void flood_whohas(plist_t* whohas_list){
  bt_peer_t* peers = config.peers;
  printf("flooding!\n");
  for(pnode_t* pnode = whohas_list->head->next; pnode != whohas_list->tail; pnode = pnode->next){
    for(bt_peer_t* pr = peers; pr->next!=NULL; pr = pr->next){
      if(pr->id == config.identity){continue;}

      // TODO: handle error case
      sendto(sock, pnode->pkt, PACKETLEN, 0, &pr->addr, sizeof(pr->addr));
    }
  }
}


void process_get(char *chunkfile, char *outputfile) {
  printf("PROCESS GET SKELETON CODE CALLED.  Fill me in! I've been doing! (%s, %s)\n", 
	chunkfile, outputfile);
  // Step1. flood who has pkt to all
  plist_t* whohas_list = make_whohas_pkt(chunkfile);
  printf("flooding whohas! Whohas pkt count: %d\n", whohas_list->len);
  flood_whohas(whohas_list);
  printf("flooding whohas end!\n");
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
