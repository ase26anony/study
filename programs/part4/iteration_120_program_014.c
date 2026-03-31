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
    
    /* Outer Loop - contains complex control flow */
    for (volatile int outer = 0; outer < N; ++outer) {
        /* Common prologue block - shared by inner loops */
        volatile int shared = outer * 2;
        dummy1(shared);
        
        /* This creates two distinct basic blocks in outer loop */
        if (condition) {
            /* Inner Loop A - starts inside true branch */
            /* But will jump to shared_continuation block */
            for (volatile int inner_a = 0; inner_a < M; ++inner_a) {
                dummy2(inner_a + shared);
                checksum += inner_a;
                
                /* This goto creates partial overlap:
                 * Inner Loop A's body includes the goto block,
                 * and the target is in outer loop but outside if branch */
                if (inner_a == M/2) {
                    goto shared_continuation;
                }
            }
            /* End of Inner Loop A normal path */
            dummy3(999);
        } else {
            /* Inner Loop B - different from A but shares prologue */
            for (volatile int inner_b = 0; inner_b < K; ++inner_b) {
                dummy4(inner_b - shared);
                checksum -= inner_b;
            }
        }
        
        /* Target of goto - still in outer loop but outside if branches */
        shared_continuation:
        checksum += outer;
        asm volatile("" : : : "memory"); /* Prevent optimization */
    }
    
    /* Sibling Loop C - shares some blocks with outer loop structure */
    /* Uses similar prologue pattern but different body */
    {
        volatile int setup = 123;
        dummy1(setup);
        
        /* Loop C - partially overlaps with outer loop's block pattern */
        for (volatile int sibling = 0; sibling < N/2; ++sibling) {
            /* Shares dummy1 call pattern with outer loop */
            if (sibling % 3 == 0) {
                dummy2(sibling);
            } else {
                dummy3(sibling);
            }
            checksum += sibling * 2;
        }
        
        /* Different epilogue than outer loop */
        dummy4(setup * 2);
    }
    
    /* Another overlapping loop structure */
    /* This creates more complex bitmap intersections */
    volatile int counter = 0;
    while (counter < 10) {
        /* Mixed loop types to ensure distinct bitmaps */
        for (volatile int i = 0; i < 5; ++i) {
            dummy1(counter + i);
            checksum += i;
            
            /* Small inner loop that shares some blocks */
            for (volatile int j = 0; j < 3; ++j) {
                dummy2(j);
                if (j == 1) {
                    /* Early exit creates partial overlap */
                    goto partial_exit;
                }
            }
            partial_exit:
            dummy3(i);
        }
        counter++;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
