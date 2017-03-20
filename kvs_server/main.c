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

#include "main.h"

#define HASH_NAME "kvs_tbl"
#define HASH_MAX_NUM_ENTRY 8

int gkey=5;
int arr[100] = {-1};
//struct rte_hash *h = NULL;
int kvs_key_cmp (const void *key1, const void *key2, size_t key_len) {

	if ((key1 == NULL) || ( key2 == NULL) || (key_len == 0)) {
		printf("something is wrong\n");
		return 1;
	}
	printf("key1 = %d  key2 = %d\n",*((const int *)(key1)), *((const int *)(key2)));
	if ( *((const int *)(key1)) ==  *((const int *)(key2))){
		printf("found key\n");
		return 0;
	}
	return 1;
}

int get(int key) {
	struct rte_hash *h = NULL;
	void *data = NULL;
	int retval;
	h = rte_hash_find_existing(HASH_NAME);
    if(h == NULL ) {
    	printf("ERROR key:%d h:%p\n", key, h);
        return -1;
    }
    if((retval = rte_hash_lookup_data(h,(void *)&key,(void *)&data ))){
    	printf("ERROR found no key:%d retval:%d\n", key, retval);
    	return -1;
    }
    retval = *((int *)(data));
    return retval;

}


int set(int key, int value) {
	struct rte_hash *h = NULL;
	h = rte_hash_find_existing(HASH_NAME);
    if((h == NULL )){
    	printf("ERROR key:%d h:%p\n", key, h);
        return -1;
    }
    return rte_hash_add_key_data(h,(void *)&key,(void *)&value);

}

int del(int key) {

	struct rte_hash *h = NULL;
	h = rte_hash_find_existing(HASH_NAME);
    if((h == NULL )){
    	printf("ERROR key:%d h:%p\n", key, h);
        return -1;
    }
    rte_hash_del_key (h, (const void *)&key);

    return 0;
}

static int
lcore_hello(__attribute__((unused)) void *arg)
{
	unsigned lcore_id;
	lcore_id = rte_lcore_id();
	printf("hello from core %u\n", lcore_id);
	return 0;
}

int
main(int argc, char **argv)
{
	int ret;
	unsigned lcore_id;
	char name[RTE_HASH_NAMESIZE] = {HASH_NAME};
	struct rte_hash *h = NULL;
	int32_t iter = 0;
	//int32_t *p_iter= &iter;
	void * key =NULL;
	void * data= NULL;

	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_panic("Cannot init EAL\n");

	/* call lcore_hello() on every slave lcore */
	RTE_LCORE_FOREACH_SLAVE(lcore_id) {
		rte_eal_remote_launch(lcore_hello, NULL, lcore_id);
	}

    struct rte_hash_parameters hash_params = {
            .entries = HASH_MAX_NUM_ENTRY, /* table load = 50% */
            .key_len = 32, /*sizeof(uint32_t),*/ /* Store IPv4 dest IP address */
            .socket_id = rte_socket_id(),
            .hash_func_init_val = 0,
			.name = name,
			.extra_flag = 0,
			//.hash_func = rte_jhash,
    };

    h = rte_hash_create(&hash_params);

    if( h == NULL) {
    	printf("Unable to create hashmap\n");
    	goto cleanup;
    }
    rte_hash_set_cmp_func(h,kvs_key_cmp);
	/* call it on master lcore too */
        printf("get(%d) returned %d\n",5,get(5));
        printf("set(%d,%d) returned %d\n",5,11,set(5,11));

        while (rte_hash_iterate(h,(void *)&key,(void *)&data,(uint32_t *)&iter) != -ENOENT){
        	if(key){
        		printf("%d key %p %d\n", iter, key, *((int *)(key)));
        	}
        	if(data){
        		printf("%d data %p %d\n", iter, data,*((int *)(data)));
        	}
        	key = NULL;
        	data = NULL;

        }
        printf("get(%d) returned %d\n",5,get(5));
        printf("del(%d) returned %d\n",5,del(5));
        printf("get(%d) returned %d\n",5,get(5));

    cleanup:
	rte_hash_free (h);
	rte_eal_mp_wait_lcore();
	return 0;
}
