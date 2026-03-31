/* Test program for hw-doloop.cc partial overlap bitmap analysis */
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
    volatile int N = 1000;
    volatile int M1 = 100;
    volatile int M2 = 200;
    volatile int M3 = 300;
    volatile int cond = 1;
    
    volatile int checksum = 0;
    
    /* COMMON PROLOGUE BLOCK - shared by multiple loops */
    volatile int shared_counter = 0;
    dummy1(shared_counter);
    
    /* OUTER LOOP - contains complex control flow */
    for (volatile int i = 0; i < N; ++i) {
        /* This block is part of outer loop but shared with inner loops */
        shared_counter++;
        dummy2(shared_counter);
        
        if (cond) {
            /* INNER LOOP A - starts inside if branch */
            /* This loop will partially overlap with outer loop */
            volatile int j = 0;
            
            /* Label for goto to create partial overlap */
inner_loop_a_start:
            for (; j < M1; ++j) {
                /* Unique body for inner loop A */
                dummy3(j);
                checksum += j;
                
                /* Jump to shared block outside if branch */
                /* This creates partial overlap: inner loop A's blocks are
                   both inside the if branch AND in the shared block after it */
                if (j == M1/2) {
                    goto shared_block;
                }
            }
            
            /* Block only in inner loop A's if branch */
            dummy4(i);
        } else {
            /* INNER LOOP B - alternative inner loop */
            for (volatile int k = 0; k < M2; ++k) {
                dummy4(k);
                checksum -= k;
            }
        }
        
        /* SHARED BLOCK - part of outer loop, also jumped to from inner loop A */
        /* This creates the partial overlap condition */
shared_block:
        dummy5(i);
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Reset shared counter for sibling loop */
    shared_counter = 0;
    dummy1(shared_counter);
    
    /* SIBLING LOOP C - shares prologue with outer loop but has different body */
    /* This creates another partial overlap scenario */
    for (volatile int l = 0; l < M3; ++l) {
        /* This block is in sibling loop C and also in the common prologue */
        shared_counter++;
        dummy2(shared_counter);
        
        /* Different body from outer loop */
        dummy3(l * 2);
        checksum += l * 3;
        
        /* Jump back to create more complex control flow */
        if (l < M3/2) {
            /* Another shared block */
            dummy4(l);
        }
    }
    
    /* Final shared block used by multiple loops */
    dummy5(checksum);
    
    printf("Checksum: %d\n", checksum);
    
    /* Force execution of the goto path */
    if (checksum > 0) {
        /* This ensures the compiler can't optimize away the goto */
        cond = 0;
        goto inner_loop_a_start;
    }
    
    return 0;
}
