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

#include <uv.h>

char name[RTE_HASH_NAMESIZE] = {HASH_NAME};
#define PAGE_SIZE 512

int flag = 1;

static void *s_mem(char * data, int size ){

	void * ret = rte_zmalloc("KVS", size, 0);
	if(!ret)
		return NULL;
	memcpy(ret, data, size);
	return ret;
}

void sig_handler(int signo)
{
  if (signo == SIGINT) {
    printf("received SIGINT\n");
    flag = 0;
  }
}
static void on_close(uv_handle_t* handle);
static void on_connect(uv_connect_t* req, int status);
static void on_write(uv_write_t* req, int status);
void on_read(uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf);
int pull (char *hostname, uint32_t key);


static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, struct uv_buf_t *buf) {
	if (!handle)
		return ;
  *buf = uv_buf_init((char*) calloc(suggested_size,1), suggested_size);
}


void on_close(uv_handle_t* handle)
{
	if(handle)
		printf(" value of handle->data %p.",handle->data);
}

void on_write(uv_write_t* req, int status)
{
  if (status) {
    fprintf(stderr, "uv_write error: %s\n", uv_strerror(status));
	return;
  }
  if (req)
	printf("wrote.\n");
	//uv_close((uv_handle_t*)req->handle, on_close);
}
void on_read(uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf)
{
	if(nread >= 0) {
		//printf("read: %s\n", tcp->data);
		char *data = buf->base;
		uint32_t key = *(uint32_t *)data;
		data += sizeof(uint32_t);
		printf("read: key:%d data:%s\n",key, data);
		if(strcmp(data,"-1"))
			kvs_set(name, key, s_mem(data,strlen(data)));
		else
			fprintf(stderr, "got -1 no entry\n");
	}
	else {
		//we got an EOF
    uv_close((uv_handle_t*)tcp, on_close);
	}

	//cargo-culted
	free(buf->base);
}

void on_connect(uv_connect_t* connection, int status)
{
	if (status) {
	    fprintf(stderr, "on_connect error: %s\n", uv_strerror(status));
			return;
	}
	printf("connected with data %d.\n", *(int *)(connection->data));

	uv_stream_t* stream = connection->handle;

	//char req_str[] = "a\n";
	void * req_str = connection->data;

	uv_buf_t buffer = uv_buf_init(req_str,sizeof(uint32_t));

	uv_write_t request;

	uv_write(&request, stream, &buffer, 1, on_write);
	uv_read_start(stream, alloc_buffer, on_read);
}



int pull (char *hostname, uint32_t key){
	uv_tcp_t socket;
	uv_loop_t *loop = uv_default_loop();
	if (!loop) {
		return -1;
	}
	uv_tcp_init(loop, &socket);
	uv_tcp_keepalive(&socket, 1, 60);
	printf("key:%d\n",key);
	loop->data = malloc(sizeof(uint32_t));
	*(int *)loop->data = key;
	struct sockaddr_in dest;
	uv_ip4_addr(hostname, 7000, &dest);

	uv_connect_t connect;
	connect.data = loop->data;
	uv_tcp_connect(&connect, &socket, (const struct sockaddr *)& dest, on_connect);

	uv_run(loop, UV_RUN_DEFAULT);

	return 0;
}

int
main(int argc, char **argv)
{
	int ret;//char name[] = {HASH_NAME};
	//int32_t iter = 0;
	//void * key =NULL;


	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_panic("Cannot init EAL\n");

    if (signal(SIGINT, sig_handler) == SIG_ERR) {
    	printf("\ncan't catch SIGINT\n");
    	return 0;
    }
    char host[] = "0.0.0.0";
    pull( host, 'a');
    printf("all done\n");

	rte_eal_mp_wait_lcore();
	return 0;
}
