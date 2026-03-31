/* Test program for hw-doloop.cc partial overlap bitmap analysis */
#include <stdio.h>
#include <stdlib.h>

/* Dummy functions to create unique basic blocks */
void __attribute__((noinline, noclone)) dummy1(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

void __attribute__((noinline, noclone)) dummy2(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

void __attribute__((noinline, noclone)) dummy3(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

void __attribute__((noinline, noclone)) dummy4(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

int main() {
    volatile int N = 1000;
    volatile int M = 100;
    volatile int K = 50;
    volatile int cond = 1;
    volatile int checksum = 0;
    
    /* Common prologue block shared by multiple loops */
    volatile int shared_counter = 0;
    dummy1(shared_counter);
    
    /* OUTER LOOP - contains complex control flow */
    for (volatile int i = 0; i < N; ++i) {
        /* This block is part of outer loop but will be shared */
        volatile int outer_temp = i * 2;
        dummy2(outer_temp);
        
        if (cond) {
            /* INNER LOOP A - starts inside if branch */
            volatile int j = 0;
            
            /* Loop header block */
            dummy3(j);
            
            /* This label creates partial overlap */
            shared_block:
            
            for (; j < M; ++j) {
                /* Body of inner loop A */
                checksum += j * 3;
                dummy1(j);
                
                /* Jump to shared block creates partial overlap */
                if (j == M/2) {
                    goto shared_block;
                }
            }
            
            /* This block is in outer loop but outside if branch */
            volatile int after_inner = checksum;
            dummy4(after_inner);
        } else {
            /* INNER LOOP B - shares prologue with A but different body */
            volatile int k = 0;
            dummy3(k);  /* Same dummy call as loop A header */
            
            for (; k < K; ++k) {
                /* Different body from loop A */
                checksum -= k * 5;
                dummy2(k);
            }
        }
        
        /* Outer loop continuation */
        checksum += i;
        asm volatile("" : : : "memory");
    }
    
    /* SIBLING LOOP C - shares prologue with inner loops */
    /* Reset shared counter to reuse prologue */
    shared_counter = 0;
    dummy1(shared_counter);  /* Same as initial prologue */
    
    for (volatile int l = 0; l < N/2; ++l) {
        /* Completely different body from other loops */
        checksum += l * l;
        dummy3(l);
        asm volatile("" : : : "memory");
    }
    
    /* Another overlapping structure */
    {
        volatile int x = 0;
        volatile int y = 0;
        
        /* LOOP D and LOOP E with partial overlap */
        for (x = 0; x < 200; ++x) {
            /* Block D1 */
            dummy1(x);
            
            if (x % 3 == 0) {
                /* LOOP E - partially inside D */
                for (y = 0; y < 75; ++y) {
                    /* Block E1 - inside both D and E */
                    dummy2(y);
                    
                    if (y == 50) {
                        /* Jump to block that's in D but not in E */
                        goto block_in_D_not_E;
                    }
                }
            }
            
            /* Block D2 - in D but not in E when jumped to early */
            block_in_D_not_E:
            dummy3(x);
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
