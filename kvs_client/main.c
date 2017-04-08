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
/*
void menu( void )
{
    int opt = 0,key,value;char name[] = {HASH_NAME};
    printf("\nEnter a value\n1. get(key) \n2. set(key,value) \n3. del(key) \n4.exit\n:");
    scanf("%d",&opt);
    switch(opt) {
    case 1:{
            printf("\nEnter key\nGET:");
            scanf("%d",&key);
            printf("result:%p\n",get(name,key));
            break;
            }
    case 2:{
            printf("\nEnter key and value\nSET:");
            scanf("%d %d",&key,&value);
            printf("result:%d\n",set(name,key,(void *)&value));
            break;
            }
    case 3:{
            printf("\nEnter key\nDEL:");
            scanf("%d",&key);
            printf("result:%d\n",del(name,key));
            break;
            }
 
    default:flag = 0;
    }
}

*/

int
main(int argc, char **argv)
{
	int ret;char name[] = {HASH_NAME};states_t *pstate =NULL;int * data = NULL;
	//int32_t iter = 0;
	//void * key =NULL;


	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_panic("Cannot init EAL\n");


    if (signal(SIGINT, sig_handler) == SIG_ERR) {
    	printf("\ncan't catch SIGINT\n");
    	return 0;
    }/*
    while(flag) {
        menu();
    	//sleep(1);

    }*/

/*
    for(int i =1 ;i<=1000000;i++){
    	data = ( int *)rte_zmalloc("S", sizeof(int), 0);
    	*(int *)data = i+1;
    	if(set(name,i,(void *)data)) {
    		printf("set failed %d\n",i);
    		break;
    	}
    }
    for(int i =1 ;i<=1000000;i++){

    	if((*(int *)get(name,i)) !=(i+1)) {
    		printf("get failed %d\n",i);
    		break;
    	}
    	rte_free(get(name,i));
    }
 */

    pstate = (states_t *)get(name,23);
    if(pstate->tt)
    	hashtable_set_hash_func(pstate->tt,NULL);
    else
    	return 0;
    hashtable_get(pstate->tt,5,(void **)(&data));
    printf("data fetched %d\n",*(int *)data);

    printf("all done\n");
	return 0;
}
