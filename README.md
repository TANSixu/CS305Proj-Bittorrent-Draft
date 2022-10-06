# Bittorrent
15-641 Project 2: Congestion Control with Bittorrent

# Potential troubleshooting:
1. Support multiple file? Chunk ID conflict?
2. Varied pkt length?
3. Many peers have the same chunk, which to pick up?

# Schedule
|Task|State
|----|----|
|Basic WHOHAS, IHAVE, GET and DATA|Finished on 9.29
|Arrange code|9.30 In process
|Sending buffer implementation|Done on 10.4
|Flow control|Done on 10.5
|Congestion Control|Not Start, Estimated 8 hour workload
|Test case|Not Start
|Spiffy interface to Python|Not Start

# Important Data Structures
bt_config_t in bt_parse.c: 
1. peers: linked list of bt_peer_t, stroring all peers addr, port, and id
2. haschunks: linked list of bt_haschunks_t, storing all chunks with id that this peer has

pktlist in peer.c:
linked list of pkt, used by buffer or other which needs to strore pkt

# Big change
In bt_parse.h, I changed the "chunkfile" to master_data_file, which contains all data in network, not hashes.