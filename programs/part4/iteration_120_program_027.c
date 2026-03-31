/* Test program for hw-doloop.cc partial overlap bitmap analysis */
/* Compile with: -O2 -march=rv32imc -fdump-rtl-doloop */

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
    volatile int condition = 1;
    volatile int checksum = 0;
    
    /* Common prologue block shared by multiple loops */
    volatile int shared_counter = 0;
    dummy1(shared_counter);
    
    /* Outer Loop - contains complex control flow */
    for (volatile int i = 0; i < N; ++i) {
        asm volatile("" : : : "memory");
        
        /* This if-else creates multiple basic blocks within outer loop */
        if (condition) {
            /* Branch 1: Contains Inner Loop A */
            
            /* Shared setup block - also used by other loops */
            volatile int setup = i * 2;
            dummy2(setup);
            
            /* INNER LOOP A - starts inside if branch */
            /* This loop will partially overlap with outer loop */
            for (volatile int j = 0; j < M; ++j) {
                checksum += j * 3;
                dummy3(j);
                
                /* Jump to shared block outside the if branch */
                /* This creates partial overlap: inner loop contains blocks
                   both inside and outside the if branch */
                if (j == M/2) {
                    goto shared_block;
                }
            }
            
            /* Block only in if branch, not in inner loop */
            checksum += 7;
            dummy4(checksum);
            
shared_block:
            /* This block is:
               - In outer loop
               - In Inner Loop A (via goto)
               - NOT in Inner Loop B
            */
            volatile int shared = checksum * 2;
            dummy1(shared);
            
        } else {
            /* Branch 2: Contains Inner Loop B */
            
            /* Same shared setup block as branch 1 */
            volatile int setup = i * 3;
            dummy2(setup);
            
            /* INNER LOOP B - sibling of Inner Loop A */
            /* Shares setup block but has different body */
            for (volatile int k = 0; k < K; ++k) {
                checksum -= k * 5;
                dummy4(k);
                asm volatile("" : : : "memory");
            }
            
            /* Different block than branch 1 */
            checksum -= 11;
            dummy3(checksum);
            
            /* Also jumps to shared_block */
            goto shared_block;
        }
        
        /* Continuation block in outer loop after if-else */
        volatile int cont = checksum + i;
        dummy2(cont);
        
        /* Toggle condition to exercise both branches */
        condition = !condition;
    }
    
    /* Reset shared counter for sibling loop */
    shared_counter = 0;
    dummy1(shared_counter);
    
    /* SIBLING LOOP C - shares prologue but not body with outer loop */
    /* This creates another partial overlap scenario */
    for (volatile int c = 0; c < N/2; ++c) {
        /* Shares the dummy1 call from prologue */
        checksum += c * 7;
        dummy3(c * 2);
        
        /* Unique block not in other loops */
        volatile int unique = c * 11;
        dummy4(unique);
        
        asm volatile("" : : : "memory");
    }
    
    /* Final calculation to prevent elimination */
    volatile int result = checksum;
    printf("Result: %d\n", result);
    
    return 0;
}
