#include "cache.h"
#include "config.h"


static cache_way_t** init_cache_way_t (cache_t* cache_s)  
{
        u8 way = cache_s->stat->way;
        cache_way_t** cache_way_s = (cache_way_t**)malloc(sizeof(cache_way_t*) * way);

        for (int i = 0; i < way; i++) {
                cache_way_s[i] = (cache_way_t*)malloc(sizeof(cache_way_t));
                cache_way_s[i]->valid_bit = BIT_INVALID;
                cache_way_s[i]->dirty_bit = BIT_CLEAN;
                cache_way_s[i]->tag = 0x0;
        }
        return cache_way_s;
}

static cache_set_t** init_cache_set_t (cache_t* cache_s)
{
        u16 set_num = cache_s->stat->set_num;
        cache_set_t** cache_set_s = (cache_set_t**)malloc(sizeof(cache_set_t*) * set_num);
        
        for (int i = 0; i < set_num; i++) {
                cache_set_s[i] = (cache_set_t*)malloc(sizeof(cache_set_t));
                cache_set_s[i]->ways = init_cache_way_t(cache_s);
        }
        
        return cache_set_s;
}

cache_t* init_cache_t (cache_stat_t* stat)
{
        cache_t* cache_s = (cache_t*)malloc(sizeof(cache_t));
        cache_s->stat = stat;
        
        cache_s->cache_set = init_cache_set_t(cache_s);

        return cache_s;
}       

static void destroy_cache_way_t (cache_way_t** tgt, cache_stat_t* stat) 
{
        for (int i = 0; i < stat->way; i++) {
                free(tgt[i]);
        }
        free(tgt);
}

static void destroy_cache_set_t (cache_set_t** tgt, cache_stat_t* stat)
{
        for (int i = 0; i < stat->set_num; i++) {
                destroy_cache_way_t(tgt[i]->ways, stat);
                free(tgt[i]);
        }
        
        free(tgt);
}

void destroy_cache_t (cache_t* tgt)
{
        destroy_cache_set_t(tgt->cache_set, tgt->stat);
        free(tgt);
}


void set_cache_init_config (cache_stat_t* s)
{
        s->block_num = (s->capacity * (KB)) / (s->block_size * (BYTE));
        s->set_num = s->block_num / s->way;
        
        s->offset_bit_num = _log2(s->block_size);
        s->index_bit_num  = _log2(s->set_num);
        s->tag_bit_num    = 64 - s->offset_bit_num - s->index_bit_num;
}



u8 look_up_cache (cache_t* cache_s, u64 addr) 
{
        u32 index  = get_index(cache_s, addr);
        u32 tag    = get_tag(cache_s, addr);

        u8 flag_hit = match_tag(cache_s, index, tag);
        return flag_hit;
}



u64 get_offset (cache_t* cache_s, u64 addr)
{
        cache_stat_t* st = cache_s->stat;
        u64 offset = addr << (st->tag_bit_num + st->index_bit_num); 
        offset >>= (st->tag_bit_num + st->index_bit_num);
        return offset;
}

u64 get_index (cache_t* cache_s, u64 addr) 
{
        cache_stat_t* st = cache_s->stat;
        u64 index = addr << st->tag_bit_num;
        index = index >> (st->offset_bit_num + st->tag_bit_num);
        return index;
}

u64 get_tag (cache_t* cache_s, u64 addr)
{
        cache_stat_t* st = cache_s->stat;
        u64 tag = addr >> (st->offset_bit_num + st->index_bit_num);
        return tag;
}

u8 match_tag (cache_t* cache_s, u32 index, u64 tag)
{
        
        cache_set_t* set = cache_s->cache_set[index];
        
        //u8 flag_invalid = 0x0;
        for (int i = 0; i < cache_s->stat->way; i++) {
                if (set->ways[i]->valid_bit == BIT_INVALID) continue;
                if (tag == set->ways[i]->tag) return FLAG_HIT;
        }
        //if (flag_invalid == FLAG_INVALID) return FLAG_INVALID;
        return FLAG_MISS;
}

cache_way_t* match_tag_ret (cache_t* cache_s, u32 index, u64 tag)
{
        cache_way_t* invalid_way = (cache_way_t*)0;
        cache_set_t* set = cache_s->cache_set[index];
        for (int i = 0; i < cache_s->stat->way; i++) {
                if (set->ways[i]->valid_bit == BIT_INVALID) {
                        invalid_way = set->ways[i];
                        continue;
                }
                if (tag == set->ways[i]->tag) return set->ways[i];
        }

        return invalid_way;
}

//#define __DEBUG__

u8 update_block_hit (general_stat_t* general_stat, cache_t* cache_s, u8 flag_rw) {
        u64 addr = general_stat->cur_p_addr;
        u32 index  = get_index(cache_s, addr);
        u32 tag    = get_tag(cache_s, addr);

        #ifdef __DEBUG__
        if (flag_rw == FLAG_R) {
                printf("READ  index %d : %d\n", index, tag);
        } else {
                printf("WRITE index %d : %d\n", index, tag);
        }
        #endif
        cache_set_t* set = cache_s->cache_set[index];
        for (int i = 0; i < cache_s->stat->way; i++) {
                //if (set->ways[i]->valid_bit == BIT_INVALID) continue;
                if (tag == set->ways[i]->tag) {
                        set->ways[i]->time = general_stat->total_access_traff;
                        set->ways[i]->dirty_bit = (flag_rw == FLAG_W) ? BIT_DIRTY : set->ways[i]->dirty_bit;
                }
        }
}

u8 handle_once_miss (general_stat_t* general_stat, cache_t* L1_cache) 
{
        cache_way_t* victim_block = get_victim_block(general_stat, L1_cache);
        u64 index  = get_index(general_stat->L2_cache, general_stat->cur_p_addr);
        u64 tag = victim_block->tag;

        if (victim_block->valid_bit == BIT_INVALID) {
                victim_block->valid_bit = BIT_VALID;
                victim_block->dirty_bit = BIT_CLEAN;
                victim_block->tag = get_tag(L1_cache, general_stat->cur_p_addr);
                victim_block->time = general_stat->total_access_traff;
                return 0;
        }

        victim_block->valid_bit = BIT_VALID;
        victim_block->dirty_bit = BIT_CLEAN;
        // mem load
        victim_block->tag = get_tag(L1_cache, general_stat->cur_p_addr);
        victim_block->time = general_stat->total_access_traff;
        
        cache_way_t* L2_block = match_tag_ret(general_stat->L2_cache, index, tag);

        if (L2_block == (cache_way_t*)0) {
                return;
        }
        if (victim_block->dirty_bit == BIT_DIRTY) {
                L2_block->dirty_bit = BIT_DIRTY;
                L2_block->time = general_stat->total_access_traff;
                general_stat->L1_cache_stat_s->dirty_eviction++;
        } else {
                general_stat->L1_cache_stat_s->clean_eviction++;
        }

}
u8 handle_double_miss (general_stat_t* general_stat, cache_t* L2_cache) 
{
        cache_way_t* victim_block = get_victim_block(general_stat, L2_cache);
        // cold miss
        if (victim_block->valid_bit == BIT_INVALID) {
                victim_block->valid_bit = BIT_VALID;
                victim_block->dirty_bit = BIT_CLEAN;
                // mem load
                victim_block->tag = get_tag(L2_cache, general_stat->cur_p_addr);
                victim_block->time = general_stat->total_access_traff;
        }

        // victim이 L2 캐쉬에있다!!
        u64 index  = get_index(general_stat->L1_cache, general_stat->cur_p_addr);
        u64 tag = victim_block->tag;

        // is same block in cache 1?
        cache_way_t* L1_block = match_tag_ret(general_stat->L1_cache, index, tag);

        // No Evict
        if (L1_block == (cache_way_t*)0) {
                L1_block = get_victim_block(general_stat, general_stat->L1_cache);
        }

        if (L1_block->dirty_bit == BIT_DIRTY) {
                victim_block->dirty_bit = BIT_DIRTY;
                general_stat->L1_cache_stat_s->dirty_eviction++;
        }

        if (L1_block->valid_bit == BIT_CLEAN && L1_block->valid_bit == BIT_VALID) {
                general_stat->L1_cache_stat_s->clean_eviction++;
        }

        if (victim_block->dirty_bit == BIT_DIRTY) {
                general_stat->L2_cache_stat_s->dirty_eviction++;
        } else {
                general_stat->L2_cache_stat_s->clean_eviction++;
        }

        victim_block->valid_bit = BIT_VALID;
        victim_block->dirty_bit = BIT_CLEAN;
        // mem load
        victim_block->tag = get_tag(L2_cache, general_stat->cur_p_addr);
        victim_block->time = general_stat->total_access_traff;

        L1_block->valid_bit = BIT_VALID;
        L1_block->dirty_bit = BIT_DIRTY;
        L1_block->tag = get_tag(general_stat->L1_cache, general_stat->cur_p_addr);;
        L1_block->time = victim_block->time;
}


cache_way_t* get_victim_block (general_stat_t* general_stat, cache_t* cache_s) 
{
        u64 addr = general_stat->cur_p_addr;
        u64 index  = get_index(cache_s, addr);
        cache_set_t* set = cache_s->cache_set[index];
        // printf("index : %d\n", index);

        u8 victim_block_idx = 0;

        for (int i = 0; i < cache_s->stat->way; i++) {
                if (set->ways[i]->valid_bit == BIT_INVALID) {
                        set->ways[i]->index = index;
                        return set->ways[i];
                }
        }

        if (general_stat->policy_replacement == POLICY_LRU) {
                u32 min_time = 0xffffffff;
                for (int i = 0; i < cache_s->stat->way; i++) {
                        if (set->ways[i]->time > min_time) continue;
                        min_time = set->ways[i]->time;
                        victim_block_idx = i; 
                }
        }

        if (general_stat->policy_replacement == POLICY_RANDOM) {
                victim_block_idx = rand() % (cache_s->stat->way); 
        }

        set->ways[victim_block_idx]->index = index;
        return set->ways[victim_block_idx];
}

