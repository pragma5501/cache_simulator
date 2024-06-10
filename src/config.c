#include "cache.h"
#include <stdio.h>

static cache_stat_t* init_cache_stat () 
{
        cache_stat_t* stat = (cache_stat_t*)malloc(sizeof(cache_stat_t));
        memset(stat, 0, sizeof(cache_stat_t));
        return stat;
}

static void destroy_cache_stat (cache_stat_t* stat)
{
        free(stat);
}

general_stat_t* init_general_stat ()
{
        general_stat_t* stat = (general_stat_t*)malloc(sizeof(general_stat_t));
        memset(stat, 0, sizeof(general_stat_t));

        stat->trace_file_name = (char*)malloc(sizeof(char) * 256);

        stat->L1_cache_stat_s = init_cache_stat();
        stat->L2_cache_stat_s = init_cache_stat();

        return stat;
}

void destroy_general_stat (general_stat_t* stat)
{
        destroy_cache_t(stat->L1_cache);
        destroy_cache_t(stat->L2_cache);
        destroy_cache_stat(stat->L1_cache_stat_s);
        destroy_cache_stat(stat->L2_cache_stat_s);

        free(stat->trace_file_name);
        free(stat);

}

void print_general_stat (FILE* fp, general_stat_t* stat) 
{
        cache_stat_t* s1 = stat->L1_cache_stat_s;
        cache_stat_t* s2 = stat->L2_cache_stat_s;
        s1->read_miss_rate = (100 * s1->read_misses) / s1->read_traff;
        s2->read_miss_rate = (100 * s2->read_misses) / s2->read_traff;
        s1->write_miss_rate = (100 * s1->write_misses) / s1->write_traff;
        s2->write_miss_rate = (100 * s2->write_misses) / s2->write_traff;

        fprintf(fp, "L1 Capacity       : %d\n" , s1->capacity);
        fprintf(fp, "L1 way            : %d\n" , s1->way);
        fprintf(fp, "L2 Capacity       : %d\n" , s2->capacity);
        fprintf(fp, "L2 way            : %d\n" , s2->way);
        fprintf(fp, "Block Size        : %d\n" , stat->block_size);
        fprintf(fp, "Total accesses    : %llu\n" , stat->total_access_traff);
        fprintf(fp, "Read accesses     : %llu\n" , stat->read_access_traff);
        fprintf(fp, "Write accesses    : %llu\n" , stat->write_access_traff);
        fprintf(fp, "L1 Read misses    : %lu\n" , s1->read_misses);
        fprintf(fp, "L2 Read misses    : %lu\n" , s2->read_misses);
        fprintf(fp, "L1 Write misses   : %lu\n" , s1->write_misses);
        fprintf(fp, "L2 Write misses   : %lu\n" , s2->write_misses);
        fprintf(fp, "L1 Read miss rate : %d\n" , s1->read_miss_rate);
        fprintf(fp, "L2 Read miss rate : %d\n" , s2->read_miss_rate);
        fprintf(fp, "L1 Write miss rate: %d\n" , s1->write_miss_rate);
        fprintf(fp, "L2 write miss rate: %d\n" , s2->write_miss_rate);
        fprintf(fp, "L1 Clean eviction : %d\n" , s1->clean_eviction);
        fprintf(fp, "L2 clean eviction : %d\n" , s2->clean_eviction);
        fprintf(fp, "L1 dirty eviction : %d\n" , s1->dirty_eviction);
        fprintf(fp, "L2 dirty eviction : %d\n" , s2->dirty_eviction);

}