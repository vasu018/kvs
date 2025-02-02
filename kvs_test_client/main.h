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

#include "bstrlib.h"
#include "hashtable.h"
#include "kvs_main.h"
void sig_handler(int signo);
//void menu( void );
