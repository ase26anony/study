/* Test program to trigger partial loop overlap analysis in hw-doloop.cc */
#include <stdio.h>
#include <stdlib.h>

/* Dummy functions to create unique basic blocks and prevent optimization */
__attribute__((noinline, noclone)) void dummy_func1(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

__attribute__((noinline, noclone)) void dummy_func2(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

__attribute__((noinline, noclone)) void dummy_func3(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

__attribute__((noinline, noclone)) void dummy_func4(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

__attribute__((noinline, noclone)) void dummy_func5(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

int main(void) {
    /* Use volatile variables to prevent constant propagation and optimization */
    volatile int N = 1000;
    volatile int M1 = 100;
    volatile int M2 = 200;
    volatile int M3 = 300;
    volatile int cond = 0;
    volatile int checksum = 0;
    
    /* Common prologue block - shared by multiple loops */
    volatile int shared_counter = 0;
    dummy_func1(shared_counter);
    
    /* OUTER LOOP - contains complex control flow */
    for (volatile int i = 0; i < N; ++i) {
        /* This block is part of outer loop but also shared with inner loops */
        shared_counter = i % 10;
        dummy_func2(shared_counter);
        
        /* Complex if-else structure to create multiple basic blocks in outer loop */
        if (cond) {
            /* Branch 1: Contains INNER LOOP A */
            
            /* Prologue block for inner loop A (also part of outer loop) */
            volatile int inner_a_setup = shared_counter * 2;
            dummy_func3(inner_a_setup);
            
            /* INNER LOOP A - starts inside if branch but jumps outside */
            volatile int j = 0;
            
            /* Label for goto - creates partial overlap */
            inner_loop_a_start:
            
            for (; j < M1; ++j) {
                /* Simple body for hardware loop candidate */
                checksum += (i * j) & 0xFF;
                dummy_func4(checksum);
                
                /* Conditional goto to create partial block overlap */
                if (j == M1/2) {
                    /* Jump to a block that's in outer loop but outside if branch */
                    goto shared_outer_block;
                }
            }
            
            /* This block is only in inner loop A's false path */
            dummy_func5(j);
            continue;  /* Skip the shared block in normal execution */
            
        } else {
            /* Branch 2: Contains INNER LOOP B */
            
            /* Different setup for inner loop B */
            volatile int inner_b_setup = shared_counter * 3;
            dummy_func3(inner_b_setup);
            
            /* INNER LOOP B - shares prologue but has different body */
            for (volatile int k = 0; k < M2; ++k) {
                checksum -= (i * k) & 0x7F;
                dummy_func4(checksum + k);
            }
            
            /* Different epilogue for inner loop B */
            dummy_func5(M2);
        }
        
        /* Shared block in outer loop - reachable from inner loop A via goto */
        shared_outer_block:
        checksum ^= (i << 3);
        dummy_func2(checksum);
        
        /* Toggle condition for next iteration */
        cond = !cond;
        
        /* Memory barrier to prevent loop fusion */
        asm volatile("" : : : "memory");
    }
    
    /* Reset shared counter for sibling loop */
    shared_counter = 0;
    dummy_func1(shared_counter);
    
    /* SIBLING LOOP C - shares prologue with inner loops but different body */
    /* This creates partial overlap with outer loop's prologue block */
    for (volatile int l = 0; l < M3; ++l) {
        /* Different computation to create unique basic blocks */
        checksum += (l * l) & 0x3F;
        dummy_func3(checksum);
        
        /* Different dummy function call pattern */
        if (l % 2) {
            dummy_func4(l);
        } else {
            dummy_func5(l);
        }
    }
    
    /* Final computation to prevent dead code elimination */
    volatile int result = checksum & 0xFFFF;
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
