#!/bin/bash

make clean; make

capacity=()
associativity=()
block_size=()
policy=("random" "lru")


directory="./tracefile"
tracefile=()



function init_env {
    for file in "$directory"/*; do
        if [[ -f $file ]]; then
            tracefile+=("$file")
        fi
    done

    local var=2
    for ((i=0; i<9; i++)); do
        var=$((var * 2))
        capacity+=("$var")
    done
    
    var=1
    for ((i=0; i<4; i++)); do
        var=$((var * 2))
        associativity+=("$var")
        
    done

    var=8
    for ((i=0; i<4; i++)); do
        var=$((var * 2))
        block_size+=("$var")
    done
}

init_env
# ./runfile -c 1024 -a 8 -b 16 -lru ./tracefile/483_xalancbmk.out > ./tracefile/log/result6.txt
cnt=0
for filename in "${tracefile[@]}"; do
    for p in "${policy[@]}"; do
        for c in "${capacity[@]}"; do
            for a in "${associativity[@]}"; do
                for b in "${block_size[@]}"; do
                    echo "./runfile -c "$c" -a "$a" -b "$b" -"$p" "$filename""
                    ./runfile -c "$c" -a "$a" -b "$b" -"$p" "$filename" > ./tracefile/data/result"$cnt".txt &
                    cnt=$((cnt + 1))
                done
                wait
            done
        done
    done
    echo "$filename"
done