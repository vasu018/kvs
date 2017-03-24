#ifndef __KVS_MAIN__
#define __KVS_MAIN__

#define HASH_NAME "kvs_tbl"
#define HASH_MAX_NUM_ENTRY 8

uint32_t rte_myhash(const void *key, uint32_t length, uint32_t initval);

int kvs_hash_init( char* name );

int kvs_hash_delete(char * name);

int kvs_key_cmp (const void *key1, const void *key2, size_t key_len) ;

int get(int key);

int set(int key, int value);

int del(int key);

#endif
