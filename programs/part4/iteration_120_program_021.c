/* Test program for hw-doloop.cc partial overlap bitmap analysis */
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
    /* Volatile variables to prevent optimization */
    volatile int N = 1000;
    volatile int M1 = 100;
    volatile int M2 = 200;
    volatile int M3 = 300;
    volatile int cond = 1;
    volatile int checksum = 0;
    
    /* Common prologue block - shared by multiple loops */
    volatile int shared = 0;
    dummy1(shared);
    
    /* OUTER LOOP - contains complex control flow */
    for (volatile int i = 0; i < N; ++i) {
        /* This block is part of outer loop only */
        dummy2(i);
        
        /* Complex if-else structure to create multiple basic blocks */
        if (cond) {
            /* Branch 1 - will contain INNER LOOP A */
            
            /* INNER LOOP A - starts inside if branch */
            for (volatile int j = 0; j < M1; ++j) {
                /* Unique body for inner loop A */
                dummy3(j);
                checksum += j;
                
                /* GOTO to create partial overlap */
                /* This jumps to a block that's in outer loop but outside if branch */
                if (j == M1/2) {
                    goto shared_block;
                }
            }
            
            /* Block only reached if goto not taken */
            dummy4(999);
        } else {
            /* Branch 2 - contains INNER LOOP B */
            
            /* INNER LOOP B - shares prologue but different body */
            for (volatile int k = 0; k < M2; ++k) {
                dummy1(k * 2);
                checksum -= k;
            }
        }
        
        /* Shared block - part of outer loop, reached by goto from inner loop A */
        shared_block:
        dummy2(checksum);
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Reset shared variable for sibling loop */
    shared = 1;
    dummy1(shared);
    
    /* SIBLING LOOP C - shares prologue with inner loops but different body */
    /* This creates partial overlap with outer loop's blocks */
    for (volatile int l = 0; l < M3; ++l) {
        /* Uses same dummy1 call as prologue, creating bitmap intersection */
        dummy1(l);
        checksum += l * 3;
        
        /* Different body from inner loops */
        if (l % 2 == 0) {
            dummy3(l);
        } else {
            dummy4(l);
        }
    }
    
    /* Prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
