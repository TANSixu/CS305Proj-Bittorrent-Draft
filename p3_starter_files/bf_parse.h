#ifndef F07_441_BF_PARSE_H
#define F07_441_BF_PARSE_H

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define HASHSIZE 20
#define BUFSIZE 65536
#define CHUNKSIZE (512 * 1024)

typedef struct {
    struct sockaddr_in tracker;
    unsigned int filesize;
    unsigned int numchunks;
    char* filename;
    char** hashes;
} bf_flood;

bf_flood* parse_flood_file(const char* filename);
void shahash(uint8_t *str, int len, uint8_t *hash);
void binary2hex(uint8_t *buf, int len, char *hex);
void hex2binary(char *hex, int len, uint8_t*buf);

#endif
