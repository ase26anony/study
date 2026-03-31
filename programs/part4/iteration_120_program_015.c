/* Test program for hw-doloop.cc partial overlap bitmap analysis */
/* Compile with: -O2 -march=rv32imc -fdump-rtl-doloop */

#include <stdio.h>
#include <stdint.h>

/* Dummy functions to create unique basic blocks */
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

int main(void) {
    /* Volatile variables to prevent optimization */
    volatile int N = 1000;
    volatile int M1 = 100;
    volatile int M2 = 200;
    volatile int M3 = 300;
    volatile int cond = 1;
    volatile int checksum = 0;
    
    /* Common prologue block - will be shared by multiple loops */
    volatile int common_counter = 0;
    dummy_func1(common_counter);
    
    /* OUTER LOOP - contains complex control flow */
    for (volatile int i = 0; i < N; ++i) {
        /* This block is part of outer loop only */
        dummy_func2(i);
        
        /* Complex if-else structure creates multiple basic blocks */
        if (cond) {
            /* Branch 1 - contains INNER LOOP A */
            
            /* Prologue block for inner loop A */
            volatile int setup_a = i * 2;
            dummy_func3(setup_a);
            
            /* INNER LOOP A - starts inside if branch */
            for (volatile int j = 0; j < M1; ++j) {
                /* Body of inner loop A */
                dummy_func1(j);
                checksum += j;
                
                /* Jump to shared block - creates partial overlap */
                if (j == M1 - 1) {
                    goto shared_block;  /* Extends into outer loop block */
                }
            }
            
            /* This block is only executed if no goto taken */
            dummy_func4(i);
            continue;
            
        shared_block:
            /* This block is part of outer loop and reached by inner loop A */
            /* Creates partial overlap: inner loop A contains this block,
               but outer loop also contains it */
            volatile int shared = i + 100;
            dummy_func2(shared);
            checksum += shared;
            
        } else {
            /* Branch 2 - contains INNER LOOP B */
            
            /* Prologue block for inner loop B (different from A) */
            volatile int setup_b = i * 3;
            dummy_func4(setup_b);
            
            /* INNER LOOP B - shares common prologue but different body */
            for (volatile int k = 0; k < M2; ++k) {
                /* Different body from loop A */
                dummy_func3(k);
                checksum -= k;
                asm volatile("" : : : "memory");  /* Prevent fusion */
            }
        }
        
        /* Outer loop continuation block */
        volatile int outer_cont = i * 5;
        dummy_func1(outer_cont);
        checksum += outer_cont;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Reset common prologue for sibling loop */
    common_counter = 1;
    dummy_func1(common_counter);
    
    /* SIBLING LOOP C - shares prologue with inner loops but different body */
    /* This creates another partial overlap scenario */
    for (volatile int l = 0; l < M3; ++l) {
        /* Different body from both A and B */
        dummy_func4(l);
        checksum += l * 2;
        
        /* Additional basic block inside sibling loop */
        if (l % 2 == 0) {
            dummy_func2(l);
        } else {
            dummy_func3(l);
        }
        
        asm volatile("" : : : "memory");
    }
    
    /* Final calculation to prevent dead code elimination */
    volatile int result = checksum % 1000;
    printf("Result: %d\n", result);
    
    return 0;
}
