/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2010-2014 Intel Corporation. All rights reserved.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/queue.h>

#include <rte_memory.h>
#include <rte_memzone.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>
#include <rte_hash.h>
#include <rte_jhash.h>
#include <rte_malloc.h>




#include "kvs_main.h"


#define DEFAULT_HASH_FUNC  rte_jhash


int kvs_hash_init( char* name )
{
	//char name[RTE_HASH_NAMESIZE] = {0};
	struct rte_hash *h = NULL;
    struct rte_hash_parameters hash_params = {
            .entries = HASH_MAX_NUM_ENTRY, /* table load = 50% */
            .key_len = 4, /*sizeof(uint32_t),*/ /* Store IPv4 dest IP address */
            .socket_id = rte_socket_id(),
            .hash_func_init_val = 0,
			.name = name,
			.extra_flag = 0,
			.hash_func = rte_myhash,
    };

    h = rte_hash_create(&hash_params);

    if( h == NULL) {
    	printf("Unable to create hashmap\n");
    	return -1;
    }
    rte_hash_set_cmp_func(h,kvs_key_cmp);
    rte_hash_init_lock(h);
    return 0;

}

int kvs_hash_init_64( char* name )
{
	//char name[RTE_HASH_NAMESIZE] = {0};
	struct rte_hash *h = NULL;
    struct rte_hash_parameters hash_params = {
            .entries = HASH_MAX_NUM_ENTRY, /* table load = 50% */
            .key_len = 8, /*sizeof(uint32_t),*/ /* Store IPv4 dest IP address */
            .socket_id = rte_socket_id(),
            .hash_func_init_val = 0,
			.name = name,
			.extra_flag = 0,
			.hash_func = rte_myhash,
    };

    h = rte_hash_create(&hash_params);

    if( h == NULL) {
    	printf("Unable to create hashmap\n");
    	return -1;
    }
    rte_hash_set_cmp_func(h,kvs_key_cmp);
    rte_hash_init_lock(h);
    return 0;
}


int kvs_hash_delete(char * name)
{
	struct rte_hash *h = NULL;

	if(name == NULL) {
    	printf("No name specified\n");
        return -1;
	}

	h = rte_hash_find_existing(name);

    if(h == NULL ) {
    	printf("Unable to find hash function %s\n", name);
        return 0;
    }
	rte_hash_free (h);
	return 0;
}

int kvs_hash_delete_h(struct rte_hash *h)
{
    if(h == NULL ) {
    	printf("Unable to find hash function \n");
        return 0;
    }
	rte_hash_free (h);
	return 0;
}

int kvs_key_cmp (const void *key1, const void *key2, size_t key_len) {

	if ((key1 == NULL) || ( key2 == NULL) || (key_len == 0)) {
		printf("something is wrong\n");
		return 1;
	}
	//printf("key1 = %d  key2 = %d\n",*((const int *)(key1)), *((const int *)(key2)));

	for(unsigned int i =0;i<key_len;i++){
		if(((const uint8_t *)key1)[i] != ((const uint8_t *)key2)[i]) {
			return 1;
		}

	}
/*
	if ( *((const int *)(key1)) ==  *((const int *)(key2))){
		//printf("found key\n");
		return 0;
	}*/
	return 0;
}

void* kvs_get(char * name, int key) {
	struct rte_hash *h = NULL;

	if (name == NULL) {
		return NULL;
	}
	h = rte_hash_find_existing(name);

	return kvs_get_h( h,  &key, 4);

}


void* kvs_get_64(char * name, int64_t key) {
	struct rte_hash *h = NULL;

	if (name == NULL) {
		return NULL;
	}
	h = rte_hash_find_existing(name);

	return kvs_get_h( h,  &key, 8);

}


void* kvs_get_h(struct rte_hash *h, void *key, uint32_t key_len) {

	void *data = NULL,*res=NULL;
	int retval;

    if((h == NULL )||(key == NULL)) {
    	printf("ERROR key:%p h:%p\n", key, h);
        return NULL;
    }

    if(rte_hash_len(h) != key_len ) {
    	printf("ERROR key_len mismatch key_len:%u\n", key_len);
        return NULL;
    }

    rte_hash_lock(h);
    rte_hash_set_cmp_func(h,kvs_key_cmp);
    rte_hash_update_hash_fun(h,rte_myhash);
    if((retval = rte_hash_lookup_data(h,(void *)key,(void *)&data ))<0){
    	//printf("ERROR found no key:%d retval:%d\n", key, retval);
    	rte_hash_unlock(h);
    	return NULL;
    }
#if 0
    if(pthread_mutex_lock(&((( struct kvs_hash_struct *)data)->lock))) {
    	//printf("ERROR cannot get a lock :%d", key);
    	return NULL;
    }
#endif
    res = ((struct kvs_hash_struct *)(data))->data;
#if 0
    pthread_mutex_unlock(&((( struct kvs_hash_struct *)data)->lock));
#endif
    rte_hash_unlock(h);
    return res;

}

int
kvs_hash_iterate(char *name, int *next_key, void **next_data, uint32_t *next)
{
    void *data = NULL,*res=NULL;
    const void *key;
    int retval;
    struct rte_hash *h = NULL;

    if (name == NULL) {
            return -1;
    }
    h = rte_hash_find_existing(name);

    if(h == NULL ) {
    	printf("ERROR No HashTable named:%s\n", name);
        return -1;
    }

    if((retval = rte_hash_iterate(h, &key, &data, next))<0){
    	printf("ERROR iteration complete... retval:%d\n", retval);
    	return -1;
    }
#if 0
    if(pthread_mutex_lock(&((( struct kvs_hash_struct *)data)->lock))) {
    	printf("ERROR cannot get a lock...  \n");
    	return -1;
    }
#endif
    res = ((struct kvs_hash_struct *)(data))->data;
#if 0
    pthread_mutex_unlock(&((( struct kvs_hash_struct *)data)->lock));
#endif

    *next_data = res;
    *next_key  = *(const int*)key;

    return retval;
}


int kvs_set(char *name,int key, void * value) {
	struct rte_hash *h = NULL;
	if(name == NULL) {
		return -1;
	}
	h = rte_hash_find_existing(name);

	return kvs_set_h(h , &key,  value, 4);

}


int kvs_set_64(char *name,int64_t key, void * value) {
	struct rte_hash *h = NULL;
	if(name == NULL) {
		return -1;
	}
	h = rte_hash_find_existing(name);

	return kvs_set_h(h , &key,  value, 8);

}


int kvs_set_h(struct rte_hash *h ,void* key, void * value, uint32_t key_len) {

	struct kvs_hash_struct* p_value = NULL;int retval;

    if((h == NULL )){
    	printf("ERROR key:%p h:%p\n", key, h);
        return -1;
    }
    if(rte_hash_len(h) != key_len ) {
    	printf("ERROR key_len mismatch key_len:%u\n", key_len);
        return -1;
    }
    rte_hash_lock(h);
    rte_hash_update_hash_fun(h,rte_myhash);
    rte_hash_set_cmp_func(h,kvs_key_cmp);
    if((retval = rte_hash_lookup_data(h,(void *)key,(void *)&p_value ))<0){
    	if(rte_hash_len(h) == 8) {
#ifdef LOGS
    		printf("found no key:%ld \n", *(long int *)key);
#endif
    	}else{
#ifdef LOGS
    		printf("found no key:%d \n", *(int *)key);
#endif
    	}
    	p_value = ( struct kvs_hash_struct *)rte_zmalloc("KVS", sizeof(struct kvs_hash_struct), 0);
    	if(p_value == NULL) {
    		printf("ERROR malloc failed\n");
    		rte_hash_unlock(h);
    		return -1;
    	}
#if 0
    	pthread_mutex_init(&(p_value->lock), NULL);
#endif

    }
#if 0
    if(pthread_mutex_lock(&(p_value->lock))) {
    	printf("ERROR cannot get a lock\n");
    	return -1;
    }
#endif
	p_value->data = value;
    retval= rte_hash_add_key_data(h,(void *)key,(void *)p_value);
#if 0
    pthread_mutex_unlock(&(p_value->lock));
#endif
    rte_hash_unlock(h);
    return retval;

}


int kvs_del(char * name, int key) {

	struct rte_hash *h = NULL;
	if(name == NULL){
		return -1;
	}
	h = rte_hash_find_existing(name);
	return kvs_del_h(h, &key, 4);
}

int kvs_del_64(char *name, int64_t key) {

	struct rte_hash *h = NULL;
	if(name == NULL){
		return -1;
	}
	h = rte_hash_find_existing(name);
	return kvs_del_h(h, &key, 8);
}


int kvs_del_h(struct rte_hash *h, void *key, uint32_t key_len) {
	int retval;
	void * data = NULL;
    if((h == NULL )){
    	printf("ERROR key:%p h:%p\n", key, h);
        return -1;
    }
    if(rte_hash_len(h) != key_len ) {
    	printf("ERROR key_len mismatch key_len:%u\n", key_len);
        return -1;
    }
    rte_hash_lock(h);
    rte_hash_update_hash_fun(h,rte_myhash);
    rte_hash_set_cmp_func(h,kvs_key_cmp);
    //TODO: Memory leak need to free the struct kvs_hash_struct
    if((retval = rte_hash_lookup_data(h,(void *)key,(void *)&data ))<0){
    	//printf("ERROR found no key:%d retval:%d\n", key, retval);
    	rte_hash_unlock(h);
    	return -1;
    }

    if(data) {

#if 0
    	while (pthread_mutex_destroy(&((( struct kvs_hash_struct *)data)->lock)) != EBUSY){
    		rte_free(( struct kvs_hash_struct *)data);
    		break;
    	}
#endif
		rte_free(( struct kvs_hash_struct *)data);
    }
    rte_hash_del_key (h, (const void *)key);
    rte_hash_unlock(h);
    return 0;
}

uint32_t
rte_myhash(const void *key, uint32_t length, uint32_t initval) {

	return DEFAULT_HASH_FUNC(key,length,initval);
}

