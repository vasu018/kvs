#ifndef __KVS_MAIN__
#define __KVS_MAIN__

#define HASH_NAME "kvs_tbl"
#define HASH_MAX_NUM_ENTRY 1048576


struct kvs_hash_struct {
	pthread_mutex_t lock;
	void * data;
	//int data;
};

uint32_t rte_myhash(const void *key, uint32_t length, uint32_t initval);

int kvs_hash_init( char* name );

int kvs_hash_delete(char * name);

int kvs_hash_delete_h(struct rte_hash *h);

int kvs_key_cmp (const void *key1, const void *key2, size_t key_len);

void * kvs_get(char * name,int key);

void * kvs_get_h(struct rte_hash *h,int key);

int kvs_set(char *name,int key, void * value);

int kvs_set_h(struct rte_hash *h ,int key, void * value);

int kvs_del(char * name,int key);

int kvs_del_h(struct rte_hash *h,int key);

#endif
