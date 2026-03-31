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
    volatile int M = 100;
    volatile int K = 50;
    volatile int cond = 1;
    volatile int checksum = 0;
    
    /* COMMON PROLOGUE BLOCK - shared by multiple loops */
    volatile int shared_counter = 0;
    dummy1(shared_counter);
    
    /* OUTER LOOP - contains complex control flow */
    for (volatile int outer = 0; outer < N; ++outer) {
        /* This block is part of outer loop but will be shared */
        volatile int temp = outer * 2;
        dummy2(temp);
        
        /* Complex if-else structure creates multiple basic blocks */
        if (cond) {
            /* INNER LOOP A - starts inside true branch */
            volatile int inner_a = 0;
            
            /* Label for goto to create partial overlap */
            loop_a_start:
            for (inner_a = 0; inner_a < M; ++inner_a) {
                /* Unique body for loop A */
                dummy3(inner_a + 1000);
                checksum += inner_a;
                
                /* Jump to shared block - creates partial overlap */
                if (inner_a == M/2) {
                    goto shared_block;
                }
            }
            
            /* End of loop A normal path */
            dummy4(9999);
            continue;  /* Skip else branch */
        } else {
            /* INNER LOOP B - different body, same iteration count */
            for (volatile int inner_b = 0; inner_b < M; ++inner_b) {
                dummy3(inner_b + 2000);
                checksum -= inner_b;
            }
        }
        
        /* SHARED BLOCK - part of outer loop, reached by goto from loop A */
        shared_block:
        dummy1(checksum);
        asm volatile("" : : : "memory");
        
        /* Reset condition for next iteration */
        cond = !cond;
    }
    
    /* SIBLING LOOP C - shares prologue with outer loop but different body */
    /* Re-initialize shared counter to reuse prologue block */
    shared_counter = 0;
    dummy1(shared_counter);
    
    for (volatile int sibling = 0; sibling < K; ++sibling) {
        /* Different body from outer loop */
        dummy4(sibling + 3000);
        checksum += sibling * 3;
        
        /* Complex enough to prevent merging */
        if (sibling % 2) {
            dummy2(sibling);
        } else {
            dummy3(sibling);
        }
    }
    
    /* ANOTHER partial overlap scenario */
    /* Loop D and Loop E partially overlap through shared middle block */
    volatile int iter = 20;
    
    /* Loop D */
    for (volatile int d = 0; d < iter; ++d) {
        dummy1(d);
        
        middle_block:
        checksum += d;
        
        if (d % 3 == 0) {
            dummy2(d);
            continue;  /* Skip loop E entry */
        }
        
        /* Loop E - starts inside Loop D but extends beyond */
        for (volatile int e = 0; e < 5; ++e) {
            dummy3(e);
            checksum -= e;
            
            /* Jump back to middle block - creates partial overlap */
            if (e == 2) {
                goto middle_block;
            }
        }
        
        dummy4(d);
    }
    
    /* Prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
