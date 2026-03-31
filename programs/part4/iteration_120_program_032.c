/* Test program for hw-doloop.cc partial overlap bitmap analysis */
/* Compile with: -O2 -march=rv32imc -fdump-rtl-doloop -fdump-rtl-all */

#include <stdio.h>
#include <stdint.h>

/* Dummy functions to create unique basic blocks and prevent optimization */
__attribute__((noinline, noclone)) void dummy1(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

__attribute__((noinline, noclone)) void dummy2(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

__attribute__((noinline, noclone)) void dummy3(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

__attribute__((noinline, noclone)) void dummy4(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

__attribute__((noinline, noclone)) void dummy5(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

int main(void) {
    /* Volatile variables to prevent constant propagation and optimization */
    volatile int N = 1000;
    volatile int M1 = 100;
    volatile int M2 = 200;
    volatile int M3 = 300;
    volatile int cond = 1;
    volatile int checksum = 0;
    
    /* Common prologue block - will be shared by multiple loops */
    volatile int common = 0;
    dummy1(common);
    
    /* OUTER LOOP - contains complex control flow */
    for (volatile int i = 0; i < N; ++i) {
        /* This block is part of outer loop but will be shared with inner loops */
        volatile int shared = i * 2;
        dummy2(shared);
        
        /* Complex if-else structure to create multiple basic blocks */
        if (cond) {
            /* Branch 1 - will contain Inner Loop A */
            
            /* INNER LOOP A - starts inside if branch */
            for (volatile int j = 0; j < M1; ++j) {
                /* Unique body for Inner Loop A */
                volatile int temp = j + i;
                dummy3(temp);
                
                /* Jump to shared block that's outside this if branch
                   but still within outer loop */
                if (j == M1 - 1) {
                    goto shared_block;
                }
                
                checksum += temp;
            }
            
            /* Block only in if branch, not in else */
            volatile int only_in_if = i * 3;
            dummy4(only_in_if);
            checksum += only_in_if;
        } else {
            /* Branch 2 - will contain Inner Loop B */
            
            /* INNER LOOP B - shares common prologue but different body */
            for (volatile int k = 0; k < M2; ++k) {
                /* Different body for Inner Loop B */
                volatile int temp = k * i;
                dummy5(temp);
                checksum += temp;
            }
            
            /* Block only in else branch */
            volatile int only_in_else = i * 4;
            dummy1(only_in_else);
            checksum += only_in_else;
        }
        
shared_block:
        /* This block is shared:
           - Always part of outer loop
           - Reached by Inner Loop A via goto
           - Not part of Inner Loop B's direct path */
        volatile int shared_val = i * 5;
        dummy2(shared_val);
        checksum += shared_val;
        
        /* Memory barrier to prevent loop fusion */
        asm volatile("" : : : "memory");
        
        /* Toggle condition to ensure both branches are taken */
        cond = !cond;
    }
    
    /* SIBLING LOOP C - shares common prologue with outer loop
       but has partial overlap (shares some blocks but not all) */
    
    /* Re-initialize common to create shared prologue block */
    common = 1;
    dummy1(common);
    
    /* Loop C shares the common prologue but has different body */
    for (volatile int l = 0; l < M3; ++l) {
        /* Different body from outer loop */
        volatile int temp = l * 7;
        dummy3(temp);
        checksum += temp;
        
        /* Include a block that's structurally similar to one in outer loop
           but with different dummy function to prevent merging */
        volatile int similar = l * 8;
        dummy4(similar);
        checksum += similar;
    }
    
    /* Additional complexity to ensure loops aren't optimized away */
    volatile int final = checksum;
    printf("Checksum: %d\n", final);
    
    return 0;
}
