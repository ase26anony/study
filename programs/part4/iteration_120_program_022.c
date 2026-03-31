/* Test program for hw-doloop.cc partial overlap bitmap analysis */
/* Compile with: -O2 -march=rv32imc -fdump-rtl-doloop -fdump-rtl-all */

#include <stdio.h>
#include <stdint.h>

/* Dummy functions to create unique basic blocks and prevent optimization */
__attribute__((noinline, noclone)) 
void dummy_func1(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

__attribute__((noinline, noclone))
void dummy_func2(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

__attribute__((noinline, noclone))
void dummy_func3(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

__attribute__((noinline, noclone))
void dummy_func4(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

__attribute__((noinline, noclone))
void dummy_func5(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

int main(void) {
    /* Volatile variables to prevent constant propagation and optimization */
    volatile int N = 1000;
    volatile int M1 = 100;
    volatile int M2 = 200;
    volatile int M3 = 300;
    volatile int condition = 1;
    volatile int checksum = 0;
    
    /* Common prologue block shared by multiple loops */
    volatile int shared_counter = 0;
    dummy_func1(shared_counter);
    
    /* OUTER LOOP - Contains complex control flow with multiple basic blocks */
    for (volatile int outer_i = 0; outer_i < N; ++outer_i) {
        /* Basic block A1 - part of outer loop */
        dummy_func2(outer_i);
        
        /* Complex if-else structure to create multiple basic blocks in outer loop */
        if (condition) {
            /* Basic block B1 - true branch of outer loop */
            dummy_func3(outer_i);
            
            /* INNER LOOP A - Starts inside true branch but extends beyond it */
            /* This creates partial overlap: inner loop blocks are partially 
               inside and partially outside the outer loop's true branch */
            volatile int inner_a_j = 0;
            
            /* Label for goto - creates shared block between inner and outer */
shared_block:
            dummy_func4(inner_a_j);
            
            /* Inner loop A body */
            for (volatile int inner_a_i = 0; inner_a_i < M1; ++inner_a_i) {
                /* Basic block C1 - part of inner loop A */
                checksum += inner_a_i * 2;
                dummy_func1(inner_a_i);
                
                /* Jump to shared block that's outside the true branch 
                   but still within outer loop */
                if (inner_a_i == M1/2) {
                    inner_a_j = 1;
                    goto shared_block;
                }
            }
            
            /* Basic block D1 - continuation after inner loop A in true branch */
            checksum += outer_i * 3;
            
        } else {
            /* Basic block B2 - false branch of outer loop */
            dummy_func5(outer_i);
            
            /* INNER LOOP B - Different inner loop in else branch */
            /* Shares the common prologue but has distinct body */
            for (volatile int inner_b_i = 0; inner_b_i < M2; ++inner_b_i) {
                /* Basic block C2 - part of inner loop B */
                checksum -= inner_b_i;
                dummy_func2(inner_b_i);
            }
            
            /* Basic block D2 - continuation after inner loop B in false branch */
            checksum += outer_i * 5;
        }
        
        /* Basic block E - Shared continuation block after if-else 
           This block is part of outer loop and is reached by:
           1. Normal flow from true branch (after D1)
           2. Normal flow from false branch (after D2)
           3. The goto from inner loop A (from shared_block) */
        checksum += outer_i * 7;
        asm volatile("" : : : "memory");
    }
    
    /* SIBLING LOOP C - Sequential loop that shares some blocks with outer loop */
    /* Reuses the common prologue block but has different body */
    /* This creates another partial overlap scenario */
    shared_counter = 1;
    dummy_func1(shared_counter);
    
    for (volatile int sibling_i = 0; sibling_i < M3; ++sibling_i) {
        /* Basic block F - part of sibling loop C */
        checksum += sibling_i * 11;
        dummy_func3(sibling_i);
        
        /* Insert memory barrier to prevent fusion */
        asm volatile("" : : : "memory");
    }
    
    /* Additional complexity to prevent dead code elimination */
    volatile int final_checksum = checksum;
    printf("Result: %d\n", final_checksum);
    
    return final_checksum != 0 ? 0 : 1;
}
