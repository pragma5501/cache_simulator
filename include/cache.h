#ifndef __CACHE_H__
#define __CACHE_H__

#include <stdio.h>
#include "type.h"

#define POLICY_FIFO 0x00
#define POLICY_LRU 0x01
#define POLICY_RANDOM 0x02

#define FLAG_R 0x0
#define FLAG_W 0x1


#define BIT_VALID       0x1
#define BIT_INVALID     0x0

#define BIT_DIRTY       0x1
#define BIT_CLEAN       0X0

#define FLAG_HIT        0x1
#define FLAG_MISS       0x0
//#define FLAG_INVALID    0x2

typedef struct _cache_status_t {
        u16 capacity;
        u8 way;
        u8 block_size;

        u32 block_num;
        u32 set_num;
        
        u16 offset_bit_num;
        u16 index_bit_num;
        u16 tag_bit_num;

        u32 read_traff;
        u32 write_traff;

        u32 read_misses;
        u32 write_misses;
        
        u8 read_miss_rate;
        u8 write_miss_rate;
        
        u32 clean_eviction;
        u32 dirty_eviction;
} cache_stat_t;

typedef struct _cache_way_t {
        u8 valid_bit : 1;
        u8 dirty_bit : 1;
        u64 index;
        u64 tag;
        u32 time;
} cache_way_t;

typedef struct _cache_set_t {
        cache_way_t** ways;
} cache_set_t;


typedef struct _cache_t {
        cache_stat_t* stat;
        cache_set_t**  cache_set;
} cache_t;

typedef struct _general_status_t {
        char* trace_file_name;
        
        cache_t* L1_cache;
        cache_t* L2_cache;
        
        cache_stat_t* L1_cache_stat_s;
        cache_stat_t* L2_cache_stat_s;

        u8 block_size;
        u8 policy_replacement;

        u64 total_access_traff;
        u64 read_access_traff;
        u64 write_access_traff;
        
        u8 flag_RW;
        u64 cur_p_addr;
} general_stat_t;

cache_t* init_cache_t (cache_stat_t* stat);
void destroy_cache_t (cache_t* tgt);

void set_cache_init_config (cache_stat_t* s);


u8 look_up_cache (cache_t* cache_s, u64 addr) ;
u64 get_offset (cache_t* cache_s, u64 addr);
u64 get_index (cache_t* cache_s, u64 addr) ;
u64 get_tag (cache_t* cache_s, u64 addr);
u8 match_tag (cache_t* cache_s, u32 index, u64 tag);
cache_way_t* match_tag_ret (cache_t* cache_s, u32 index, u64 tag);
u8 update_block_hit (general_stat_t* general_stat, cache_t* cache_s, u8 flag_rw);
u8 handle_double_miss (general_stat_t* general_stat, cache_t* L2_cache) ;
u8 handle_once_miss (general_stat_t* general_stat, cache_t* L1_cache) ;
cache_way_t* get_victim_block (general_stat_t* general_stat, cache_t* cache_s);

general_stat_t* init_general_stat ();
void destroy_general_stat (general_stat_t* stat);
void print_general_stat (FILE* fp, general_stat_t* stat) ;



#endif