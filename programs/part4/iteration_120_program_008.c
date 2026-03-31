/* Test program for hw-doloop.cc partial overlap bitmap analysis */
/* Compile with: -O2 -march=rv32imc -fdump-rtl-doloop -fdump-rtl-all */

#include <stdio.h>

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
        volatile int temp = i * 2;
        dummy2(temp);
        
        if (cond) {
            /* INNER LOOP A - starts inside if branch */
            /* This loop will partially overlap with outer loop */
            volatile int j = 0;
            
            /* Label for goto to create partial overlap */
            loop_a_start:
            for (; j < M; ++j) {
                /* Unique body for loop A */
                volatile int val = i + j;
                dummy3(val);
                checksum += val;
                
                /* Jump to shared block - creates partial overlap */
                if (j == M/2) {
                    goto shared_block;
                }
            }
            
            /* End of if branch */
            dummy4(i);
        } else {
            /* INNER LOOP B - different body, same prologue */
            for (volatile int k = 0; k < K; ++k) {
                volatile int val = i - k;
                dummy1(val);
                checksum -= val;
            }
        }
        
        /* Shared block - part of outer loop, entered from loop A */
        shared_block:
        volatile int shared = i * 3;
        dummy2(shared);
        checksum += shared;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
        
        /* Toggle condition to ensure both branches are taken */
        cond = !cond;
    }
    
    /* SIBLING LOOP C - shares prologue but different body */
    /* Uses same shared_counter from prologue */
    dummy1(shared_counter);
    
    for (volatile int c = 0; c < N/2; ++c) {
        /* Partially overlaps with outer loop's prologue block */
        volatile int val = c + shared_counter;
        dummy4(val);
        checksum += val * 2;
        
        /* Different basic block pattern from other loops */
        if (c % 3 == 0) {
            dummy3(c);
        } else {
            dummy2(c);
        }
    }
    
    /* Additional complexity to create more partial overlaps */
    volatile int extra = 0;
    for (volatile int x = 0; x < 10; ++x) {
        /* This loop shares the dummy1 call with the prologue */
        dummy1(x);
        extra += x;
        
        /* Nested mini-loop that overlaps with parent */
        for (volatile int y = 0; y < 5; ++y) {
            /* Jump back to parent loop body */
            if (y == 3) {
                goto mini_loop_escape;
            }
            dummy2(x + y);
        }
        mini_loop_escape:
        dummy3(x);
    }
    
    printf("Checksum: %d\n", checksum + extra);
    return 0;
}
