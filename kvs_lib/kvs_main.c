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



int kvs_hash_init( char* name )
{
	//char name[RTE_HASH_NAMESIZE] = {0};
	struct rte_hash *h = NULL;
    struct rte_hash_parameters hash_params = {
            .entries = HASH_MAX_NUM_ENTRY, /* table load = 50% */
            .key_len = 32, /*sizeof(uint32_t),*/ /* Store IPv4 dest IP address */
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
	if ( *((const int *)(key1)) ==  *((const int *)(key2))){
		//printf("found key\n");
		return 0;
	}
	return 1;
}

void* get(char * name,int key) {
	struct rte_hash *h = NULL;

	if (name == NULL) {
		return NULL;
	}
	h = rte_hash_find_existing(name);

	return get_h( h,  key);

}


void* get_h(struct rte_hash *h,int key) {

	void *data = NULL,*res=NULL;
	int retval;

    if(h == NULL ) {
    	printf("ERROR key:%d h:%p\n", key, h);
        return NULL;
    }

    rte_hash_set_cmp_func(h,kvs_key_cmp);

    rte_hash_update_hash_fun(h,rte_myhash);
    if((retval = rte_hash_lookup_data(h,(void *)&key,(void *)&data ))<0){
    	printf("ERROR found no key:%d retval:%d\n", key, retval);
    	return NULL;
    }
    if(pthread_mutex_lock(&((( struct kvs_hash_struct *)data)->lock))) {
    	printf("ERROR cannot get a lock :%d", key);
    	return NULL;
    }
    res = ((struct kvs_hash_struct *)(data))->data;
    pthread_mutex_unlock(&((( struct kvs_hash_struct *)data)->lock));
    return res;

}



int set(char *name,int key, void * value) {
	struct rte_hash *h = NULL;
	if(name == NULL) {
		return -1;
	}
	h = rte_hash_find_existing(name);

	return set_h(h , key,  value);

}

int set_h(struct rte_hash *h ,int key, void * value) {

	struct kvs_hash_struct* p_value = NULL;int retval;

    if((h == NULL )){
    	printf("ERROR key:%d h:%p\n", key, h);
        return -1;
    }
    rte_hash_update_hash_fun(h,rte_myhash);
    rte_hash_set_cmp_func(h,kvs_key_cmp);
    if((retval = rte_hash_lookup_data(h,(void *)&key,(void *)&p_value ))<0){
    	printf("found no key:%d \n", key);
    	p_value = ( struct kvs_hash_struct *)rte_zmalloc("KVS", sizeof(struct kvs_hash_struct), 0);
    	pthread_mutex_init(&(p_value->lock), NULL);

    }
    if(pthread_mutex_lock(&(p_value->lock))) {
    	printf("ERROR cannot get a lock :%d", key);
    	return -1;
    }
	p_value->data = value;
    retval= rte_hash_add_key_data(h,(void *)&key,(void *)p_value);
    pthread_mutex_unlock(&(p_value->lock));
    return retval;

}


int del(char * name,int key) {

	struct rte_hash *h = NULL;
	if(name == NULL){
		return -1;
	}
	h = rte_hash_find_existing(name);
	return del_h(h, key);
}


int del_h(struct rte_hash *h,int key) {
	int retval;
	void * data = NULL;
    if((h == NULL )){
    	printf("ERROR key:%d h:%p\n", key, h);
        return -1;
    }
    rte_hash_update_hash_fun(h,rte_myhash);
    rte_hash_set_cmp_func(h,kvs_key_cmp);
    //TODO: Memory leak need to free the struct kvs_hash_struct
    if((retval = rte_hash_lookup_data(h,(void *)&key,(void *)&data ))<0){
    	printf("ERROR found no key:%d retval:%d\n", key, retval);
    	return -1;
    }

    if(data) {

    	while (pthread_mutex_destroy(&((( struct kvs_hash_struct *)data)->lock)) != EBUSY){
    	rte_free(( struct kvs_hash_struct *)data);
    	}
    }
    rte_hash_del_key (h, (const void *)&key);
    return 0;
}

uint32_t
rte_myhash(const void *key, uint32_t length, uint32_t initval) {

	if( length == 0 || initval == 0){
		length = initval;
	}
	return *(const uint32_t *)key;
}

