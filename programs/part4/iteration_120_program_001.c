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
    
    /* Common prologue block - will be shared by multiple loops */
    volatile int common = 0;
    dummy1(common);
    
    /* OUTER LOOP - contains complex control flow */
    for (volatile int i = 0; i < N; ++i) {
        /* This block is part of outer loop only */
        dummy2(i);
        
        if (cond) {
            /* INNER LOOP A - starts inside if branch */
            /* This loop will partially overlap with outer loop */
            volatile int j = 0;
            
            /* Shared block with inner loop B */
            volatile int setup = i * 2;
            dummy3(setup);
            
            /* Loop A proper */
            for (j = 0; j < M; ++j) {
                dummy1(j);
                checksum += j;
                
                /* CRITICAL: Jump to block outside if branch but still in outer loop */
                if (j == M/2) {
                    goto shared_block;  /* Creates partial overlap */
                }
            }
            
            /* Block only reached if no goto taken */
            dummy4(j);
        } else {
            /* INNER LOOP B - alternative inner loop */
            /* Shares setup block with loop A but has different body */
            volatile int k = 0;
            
            /* Shared block with inner loop A */
            volatile int setup = i * 3;  /* Different computation */
            dummy3(setup);
            
            /* Loop B proper */
            for (k = 0; k < K; ++k) {
                dummy2(k);
                checksum -= k;
            }
        }
        
        /* This label creates the partial overlap scenario */
        shared_block:
        /* Block that is in outer loop and reachable from inner loop A via goto */
        volatile int post = i * 5;
        dummy4(post);
        checksum += post;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
        
        /* Toggle condition to ensure both branches are taken */
        cond = !cond;
    }
    
    /* SIBLING LOOP C - shares common prologue but different body */
    /* This creates bitmap intersection but not subset relationship */
    {
        /* Re-use common prologue */
        volatile int setup = common + 1;
        dummy1(setup);
        
        /* Loop C proper - different iteration count and body */
        for (volatile int c = 0; c < N/2; ++c) {
            dummy3(c);
            checksum += c * 3;
            
            /* Different control flow pattern */
            if (c % 2) {
                dummy4(c);
            }
        }
    }
    
    /* Another sibling-like structure with partial block sharing */
    {
        /* Block shared with outer loop's prologue */
        volatile int setup2 = common + 2;
        dummy1(setup2);
        
        /* Loop D - shares some setup but has unique body */
        for (volatile int d = 0; d < M; ++d) {
            dummy2(d * 2);
            checksum -= d;
            
            /* Complex body to create multiple basic blocks */
            if (d % 3 == 0) {
                dummy3(d);
            } else if (d % 3 == 1) {
                dummy4(d);
            }
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
