/* Test program to trigger partial loop overlap bitmap analysis in hw-doloop.cc */
/* Compile with: -O2 -march=rv32imc -fdump-rtl-doloop -fdump-rtl-all */

#include <stdio.h>

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

int main() {
    /* Use volatile to prevent constant propagation and loop optimization */
    volatile int N = 1000;
    volatile int M1 = 100;
    volatile int M2 = 200;
    volatile int M3 = 300;
    volatile int cond = 1;
    volatile int checksum = 0;
    
    /* Common prologue block that will be shared by multiple loops */
    volatile int shared_counter = 0;
    
    /* Outer Loop - contains complex control flow */
    for (volatile int i = 0; i < N; ++i) {
        /* This block is part of outer loop but will be shared with inner loops */
        shared_counter = i * 2;
        dummy1(shared_counter);
        
        /* Complex if-else structure to create multiple basic blocks in outer loop */
        if (cond) {
            /* Branch 1 - contains Inner Loop A */
            
            /* Inner Loop A - starts inside if branch but will extend beyond it */
            /* This creates partial overlap: loop A blocks are partially inside 
               and partially outside the if branch (which is part of outer loop) */
            for (volatile int j = 0; j < M1; ++j) {
                dummy2(j);
                checksum += j;
                
                /* Jump to shared block that's outside this if branch 
                   but still within outer loop */
                if (j == M1/2) {
                    goto shared_block;  /* Creates control flow to shared block */
                }
            }
            
            /* Block only executed if no goto taken */
            dummy3(123);
        } else {
            /* Branch 2 - contains Inner Loop B */
            /* Inner Loop B - shares the common prologue but has different body */
            for (volatile int k = 0; k < M2; ++k) {
                dummy4(k);
                checksum -= k;
            }
            dummy3(456);
        }
        
        /* Shared block that's part of outer loop and will be reached by Inner Loop A */
        shared_block:
        dummy5(i);
        
        /* Memory barrier to prevent loop fusion */
        asm volatile("" : : : "memory");
        
        /* Toggle condition to ensure both branches are taken */
        cond = !cond;
    }
    
    /* Sibling Loop C - shares some blocks with outer loop but not all */
    /* This creates another partial overlap scenario */
    shared_counter = 0;  /* Shared prologue similar to outer loop */
    dummy1(shared_counter);
    
    for (volatile int l = 0; l < M3; ++l) {
        /* Different body from inner loops A and B */
        dummy2(l * 3);
        checksum += l * 2;
        
        /* Different dummy function call to create unique basic blocks */
        if (l % 2 == 0) {
            dummy4(l);
        } else {
            dummy5(l);
        }
    }
    
    /* Final dummy block to prevent tail merging */
    dummy3(checksum);
    
    /* Print to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
