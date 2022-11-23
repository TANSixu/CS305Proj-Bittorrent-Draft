/*
 * bitflood.c
 *
 * Author: Albert Sheu <albert@cmu.edu>
 * Class: 15-441 (Fall 2007)
 *
 * Description: Code for parsing a flood file, and for converting
 * chunk hashes between binary and hex format.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <assert.h>
#include "bf_parse.h"
#include "sha.h"

/*
  Extract relevant information from the .flood file at filename.
  Hashes will be stored in hex format.
*/
bf_flood* parse_flood_file(const char* filename)
{
    int i, chunknum, port;
    FILE *fp;
    char *line, *addr_string, *hostname;
    char buf[BUFSIZE], hashstr[BUFSIZE];    
    bf_flood* flood_config;
    struct hostent *host;

    fp = fopen(filename, "r");
    bzero(buf, BUFSIZE);
    fread(buf, BUFSIZE, 1, fp);
    
    flood_config = malloc(sizeof(bf_flood));

    line = strtok(buf, "\n");
    for (i = 1; line != NULL; i++) {
	if (i == 1) {
	    addr_string = line;
	} else if (i == 2) {
	    flood_config->filename = malloc(sizeof(char) * (strlen(line)+1));
	    strcpy(flood_config->filename, line);
	} else if (i == 3) {
	    flood_config->filesize = atoi(line);
	    flood_config->numchunks = ((int)(flood_config->filesize + CHUNKSIZE - 1) / CHUNKSIZE);
	    flood_config->hashes = malloc(sizeof(char*) * flood_config->numchunks);	    
	} else {
	    bzero(hashstr, BUFSIZE);
	    sscanf(line, "%d %s", &chunknum, hashstr);
	    flood_config->hashes[chunknum] = malloc(sizeof(char) * (strlen(hashstr)+1));
	    strcpy(flood_config->hashes[chunknum], hashstr);
	}
	line = strtok(NULL, "\n");
    }
    assert(i > 3);

    hostname = strtok(addr_string, ":");
    port = atoi(strtok(NULL, ":"));

    host = gethostbyname(hostname);
    assert(host != NULL);
    flood_config->tracker.sin_addr.s_addr = *(in_addr_t *)host->h_addr;
    flood_config->tracker.sin_family = AF_INET;
    flood_config->tracker.sin_port = htons(port);

    return flood_config;
}

/*
  str -- is the data you want to hash
  len -- the length of the data you want to hash.
  hash -- the target where the function stores the hash. The length of the
  array should be SHA1_HASH_SIZE.
*/
void shahash(uint8_t *str, int len, uint8_t *hash) {
	SHA1Context sha;
	SHA1Init(&sha);
	SHA1Update(&sha, str, len);
	SHA1Final(&sha, hash);
	memset(&sha, 0, sizeof(sha));
}

/*
  Converts the binary char string str to ASCII format.  The length of
  ascii should be 2 times that of str.
*/
void binary2hex(uint8_t *buf, int len, char *hex) {
	int i=0;
	for(i=0;i<len;i++) {
		sprintf(hex+(i*2), "%.2x", buf[i]);
	}
	hex[len*2] = 0;
}
  
/*
  Ascii to hex conversion routine
*/
static uint8_t _hex2binary(char hex)
{
     hex = toupper(hex);
     uint8_t c = ((hex <= '9') ? (hex - '0') : (hex - ('A' - 0x0A)));
     return c;
}

/*
 Converts the ascii character string in "ascii" to binary string in "buf"
 the length of buf should be atleast len / 2
*/
void hex2binary(char *hex, int len, uint8_t*buf) {
	int i;
	for(i = 0; i < len; i+=2) {
	    buf[i/2] = _hex2binary(hex[i]) << 4 
	      | _hex2binary(hex[i+1]);
	}
}

