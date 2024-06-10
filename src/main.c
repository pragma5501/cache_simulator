#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "simul.h"
#include "type.h"
general_stat_t* cache_stat;
FILE* trace_fp;

u8 flag_run = FLAG_ON;

void init_env ()
{
        cache_stat = init_general_stat();
}

void destroy_env ()
{
        destroy_general_stat(cache_stat);
}

void read_trace_file (FILE* fp)
{
        char buffer[256];
        
        if (fgets(buffer, 256, fp) == NULL) flag_run = FLAG_OFF;
        cache_stat->flag_RW    = (buffer[0] == 'R') ? FLAG_R : FLAG_W;
        cache_stat->cur_p_addr = (u64)strtoul(buffer + 2, NULL, 16);
        //printf("RW flag : %c\n", buffer[0]);
        //printf("addr    : 0x%x\n", cache_stat->cur_p_addr);
        
}

void parse_opt(int argc, char** argv)
{
        for (int i = 1; i < argc; i++) {
                //printf("argv : %s\n", argv[i]);
                // opt : Capacity
                if (strcmp("-c", argv[i]) == 0) {
                        i += 1;
                        cache_stat->L1_cache_stat_s->capacity = atoi(argv[i]) / 4;
                        cache_stat->L2_cache_stat_s->capacity = atoi(argv[i]);
                }
                // opt : Associativity
                else if (strcmp("-a", argv[i]) == 0) {
                        i += 1;
                        u8 way = atoi(argv[i]);
                        cache_stat->L1_cache_stat_s->way = (way <= 2) ? way : way / 4;
                        cache_stat->L2_cache_stat_s->way = way;
                } 
                // opt : Block size
                else if (strcmp("-b", argv[i]) == 0) {
                        i += 1;
                        cache_stat->block_size = atoi(argv[i]);
                        cache_stat->L1_cache_stat_s->block_size = cache_stat->block_size;
                        cache_stat->L2_cache_stat_s->block_size = cache_stat->block_size;

                }
                // opt : LRU replacement policy setting
                else if (strcmp("-lru", argv[i]) == 0) {
                        cache_stat->policy_replacement = POLICY_LRU;
                } 
                // opt : RANDOM replacement policy setting
                else if (strcmp("-random", argv[i]) == 0) {
                        cache_stat->policy_replacement = POLICY_RANDOM;

                }
                // opt : Trace file name
                else {
                        
                        strcpy(cache_stat->trace_file_name, argv[i]);
                }
        }

        set_cache_init_config(cache_stat->L1_cache_stat_s);
        set_cache_init_config(cache_stat->L2_cache_stat_s);

        cache_stat->L1_cache = init_cache_t(cache_stat->L1_cache_stat_s);
        cache_stat->L2_cache = init_cache_t(cache_stat->L2_cache_stat_s);
}

int run (FILE* fp) 
{
        while(flag_run == FLAG_ON) {
                read_trace_file(trace_fp);
                simul_cache(cache_stat);
                //if ((cache_stat->total_access_traff + 1000) % 10000000 == 0)
                //        print_general_stat(cache_stat);
        }
        print_general_stat(fp, cache_stat);
        
}

int main (int argc, char** argv)
{
        init_env();

        parse_opt(argc, argv);
        trace_fp = fopen(cache_stat->trace_file_name, "r");
        if (trace_fp == NULL) {
                printf("Cannot open file : %s\n", cache_stat->trace_file_name);
                exit(1);
        }

        char out_file_name[256] = "";
        strcpy(out_file_name, cache_stat->trace_file_name);
        out_file_name[strlen(out_file_name) - 4] = '\0';
        strcpy(out_file_name + 11, "/log/");
        strcpy(out_file_name + 16, cache_stat->trace_file_name + 11);

        char buf[10];
        sprintf(buf, "_%d", cache_stat->L2_cache_stat_s->capacity);
        strcpy(out_file_name + strlen(out_file_name), buf);

        sprintf(buf, "_%d",  cache_stat->L2_cache_stat_s->way);
        strcpy(out_file_name + strlen(out_file_name), buf);

        sprintf(buf, "_%d",  cache_stat->block_size);
        strcpy(out_file_name + strlen(out_file_name), buf);

        strcpy(out_file_name + strlen(out_file_name), ".out");

        FILE* fp = fopen(out_file_name, "w");
        run(fp);
        fclose(fp);

        destroy_env();


        //fclose(obj_fp);
}

