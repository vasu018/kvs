#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/queue.h>
#include <stdbool.h>

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


#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <uv.h>

#include "bstrlib.h"
#include "hashtable.h"
#include "kvs_main.h"


void alloc_buffer(uv_handle_t *handle, size_t suggested_size, struct uv_buf_t *buf);
void echo_write(uv_write_t *req, int status) ;
void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
void on_new_connection(uv_stream_t *server, int status);





