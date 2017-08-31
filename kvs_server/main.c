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
#include <rte_malloc.h>

#define TRUE   1
#define FALSE  0
#define PORT 8888
#define PAGE_SIZE 512

char name[RTE_HASH_NAMESIZE] = {HASH_NAME};
//char name[RTE_HASH_NAMESIZE] = {"mytable"};


uv_loop_t *loop;


static void *s_mem(char * data, int size ){

	void * ret = rte_zmalloc("KVS", size, 0);
	if(!ret)
		return NULL;
	memcpy(ret, data, size);
	return ret;
}

static char *create_buf(uint32_t key, char * temp){
	char * res = (char *) calloc(PAGE_SIZE , sizeof(char));
	memcpy(res,&key,sizeof(uint32_t));
	char def[] = "-1";
	if(!temp)
		temp = def;
	memcpy(res + sizeof(uint32_t), temp, strlen(temp));
	return res;
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, struct uv_buf_t *buf) {
	if (!handle)
		return ;
  *buf = uv_buf_init((char*) calloc(suggested_size,1), suggested_size);
}

void echo_write(uv_write_t *req, int status) {
  if (status == -1) {
    fprintf(stderr, "Write error!\n");
  }

  char *base = (char*) req->data;
  fprintf(stdout, "printing base %p\n",base);
  if(base){
	  uint32_t key = *(uint32_t *)req->data;
	  base += sizeof(uint32_t);
	  fprintf(stdout, "printing key:%d data:%s\n", key, base);
	  kvs_del(name, key);
  }

  free(req->data);
  free(req);
}
//#pragma GCC diagnostic push  // require GCC 4.6
//#pragma GCC diagnostic ignored "-Wcast-qual"
void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
  if (nread == -1) {
    fprintf(stderr, "Read error!\n");
    uv_close((uv_handle_t*)client, NULL);
    return;
  }
  uv_write_t *write_req = (uv_write_t*)malloc(sizeof(uv_write_t));
  uv_buf_t ubuf;
  uv_buf_t *wbuf = NULL;
  if(nread == sizeof(uint32_t)){

	  //TODO:need to add support for htons
	  fprintf(stdout, "received %d\n",*(uint32_t *)buf->base);

	  //buf->base = NULL;
	  char * temp = (char *)kvs_get(name, *(uint32_t *)buf->base);
	  char * tmp = create_buf(*(uint32_t *)buf->base, temp);

	  free(buf->base);
	  //free(buf);
	  fprintf(stdout, "fetched %s\n",tmp);
	  write_req->data = (void *)(tmp);
	  //ubuf = uv_buf_init(tmp,strlen(tmp));

	  ubuf = uv_buf_init(tmp,PAGE_SIZE);
	  wbuf = &ubuf;
	  uv_write(write_req, client, wbuf, 1, echo_write);

  }else {
	  write_req->data = (void*)(buf->base);
	  uv_write(write_req, client, buf, 1, echo_write);
  }

  //write_req->data = (void*)(buf->base);
  //uv_write(write_req, client, wbuf, 1, echo_write);
}
//#pragma GCC diagnostic pop   // require GCC 4.6

void on_new_connection(uv_stream_t *server, int status) {
  if (status == -1) {
    return;
  }

  uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, client);
  if (uv_accept(server, (uv_stream_t*) client) == 0) {
    uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);
  }
  else {
    uv_close((uv_handle_t*) client, NULL);
  }
}


int
main(int argc, char **argv)
{
	int ret;

	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_panic("Cannot init EAL\n");

	if(kvs_hash_init(name)){
		printf("unable to create hash\n");
		goto cleanup;
	}

// making few entries in the table
	char abc[] = "100";
	kvs_set(name,'a',s_mem(abc, strlen(abc)));
	loop = uv_default_loop();

	uv_tcp_t server;
	uv_tcp_init(loop, &server);

	struct sockaddr_in bind_addr;
	uv_ip4_addr("0.0.0.0", 7000, &bind_addr);
	uv_tcp_bind(&server, (const struct sockaddr *)&bind_addr, 0);
	int r = uv_listen((uv_stream_t*) &server, 128, on_new_connection);
	if (r) {
		fprintf(stderr, "Listen error!\n");
		return 1;
	}
	return uv_run(loop, UV_RUN_DEFAULT);

	cleanup:
	rte_eal_mp_wait_lcore();
	return 0;
}
