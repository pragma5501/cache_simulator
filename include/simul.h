#ifndef __SIMUL_H__
#define __SIMUL_H__

#include "type.h"
#include "config.h"
#include "cache.h"

void simul_cache (general_stat_t* cache_stat);
void simul_read (general_stat_t* cache_stat);
void simul_write (general_stat_t* cache_stat);
#endif