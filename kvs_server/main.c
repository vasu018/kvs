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

#include "main.h"

int
main(int argc, char **argv)
{
	int ret;
	char name[RTE_HASH_NAMESIZE] = {HASH_NAME};
	struct rte_hash *h = NULL;
	int32_t iter = 0;
	void * key =NULL;
	void * data= NULL;

	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_panic("Cannot init EAL\n");

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
    	goto cleanup;
    }
    rte_hash_set_cmp_func(h,kvs_key_cmp);
    printf("get(%d) returned %d\n",5,get(5));
        printf("set(%d,%d) returned %d\n",5,11,set(5,11));

        while (rte_hash_iterate(h,(void *)&key,(void *)&data,(uint32_t *)&iter) != -ENOENT){
        	if(key){
        		printf("%d key %p %d\n", iter, key, *((int *)(key)));
        	}
        	if(data){
        		printf("%d data %p %d\n", iter, data,*((int *)(data)));
        	}

        }
        printf("get(%d) returned %d\n",5,get(5));
        printf("del(%d) returned %d\n",5,del(5));
        printf("get(%d) returned %d\n",5,get(5));

    cleanup:
	rte_hash_free (h);
	rte_eal_mp_wait_lcore();
	return 0;
}
