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
#include <signal.h>
#include <unistd.h>
int flag = 1;

void sig_handler(int signo)
{
  if (signo == SIGINT) {
    printf("received SIGINT\n");
    flag = 0;
  }
}

typedef struct states{
	hash_table_t *tt;

}states_t;



int
main(int argc, char **argv)
{
	int ret;hash_table_t *tt = NULL;int * data = NULL;
	states_t *pstate =NULL;
	char name[RTE_HASH_NAMESIZE] = {HASH_NAME};

	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_panic("Cannot init EAL\n");

	if(kvs_hash_init(name)){
		printf("unable to create hash\n");
		goto cleanup;
	}
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
    	printf("\ncan't catch SIGINT\n");
    	goto cleanup;
    }
    pstate = (states_t *) rte_malloc(NULL,sizeof(states_t),0);

    tt = hashtable_create(100,NULL,free_wrapper,NULL);
    data = (int *) rte_malloc(NULL,sizeof(int),0);
    *data = 19;
    hashtable_insert(tt,(const hash_key_t)5,(void *)data);
    pstate->tt = tt;
    set(name,23,(void *)pstate);
    while(flag) {

    	sleep(1);

    }
    hashtable_get(tt,5,(void **)(&data));
    printf("data fetched %d\n",*(int *)data);
    hashtable_free(tt,5);
    cleanup:
	kvs_hash_delete(name);
	rte_eal_mp_wait_lcore();
	return 0;
}
