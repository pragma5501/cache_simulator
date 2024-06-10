
#include "simul.h"

void simul_cache (general_stat_t* cache_stat) 
{
        switch (cache_stat->flag_RW) {
        case FLAG_R:
                simul_read(cache_stat);
        case FLAG_W:
                simul_write(cache_stat);
        }
        cache_stat->total_access_traff = cache_stat->read_access_traff + cache_stat->write_access_traff;
}

void simul_read (general_stat_t* cache_stat)
{
        cache_stat->read_access_traff++;
        u8 flag_hit;

        // L1 cache READ
        cache_t* L1_cache = cache_stat->L1_cache;
        flag_hit = look_up_cache(L1_cache, cache_stat->cur_p_addr);
        L1_cache->stat->read_traff++;

        if (flag_hit == FLAG_HIT) {
                update_block_hit(cache_stat, L1_cache, FLAG_R);
                return;
        }
        L1_cache->stat->read_misses++;
        
        
        // L2 cache READ                
        cache_t* L2_cache = cache_stat->L2_cache;
        flag_hit = look_up_cache(L2_cache, cache_stat->cur_p_addr);
        L2_cache->stat->read_traff++;

        if (flag_hit == FLAG_HIT) {
                handle_once_miss(cache_stat, L1_cache);
                update_block_hit(cache_stat, L2_cache, FLAG_R);
                return;
        }
        L2_cache->stat->read_misses++;
        
        handle_double_miss(cache_stat, L2_cache);
        

        update_block_hit(cache_stat, L1_cache, FLAG_R);
        update_block_hit(cache_stat, L2_cache, FLAG_R);
}

void simul_write (general_stat_t* cache_stat)
{
        cache_stat->write_access_traff++;
        u8 flag_hit;

        cache_t* L1_cache = cache_stat->L1_cache;
        flag_hit = look_up_cache(L1_cache, cache_stat->cur_p_addr);
        L1_cache->stat->write_traff++;

        if (flag_hit == FLAG_HIT) {
                update_block_hit(cache_stat, L1_cache, FLAG_W);
                return;
        }
        L1_cache->stat->write_misses++;

        cache_t* L2_cache = cache_stat->L2_cache;
        flag_hit = look_up_cache(L2_cache, cache_stat->cur_p_addr);
        L2_cache->stat->write_traff++;

        if (flag_hit == FLAG_HIT) {
                handle_once_miss(cache_stat, L1_cache);
                update_block_hit(cache_stat, L2_cache, FLAG_R);
                return;
        }
        L2_cache->stat->write_misses++;

        handle_double_miss(cache_stat, L2_cache);

        update_block_hit(cache_stat, L1_cache, FLAG_W);
        update_block_hit(cache_stat, L2_cache, FLAG_R);
}