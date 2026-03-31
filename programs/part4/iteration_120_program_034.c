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
        
        /* This creates multiple basic blocks in outer loop */
        if (condition) {
            /* Inner Loop A - starts inside true branch */
            /* But will jump to shared block outside this branch */
            for (volatile int inner_a = 0; inner_a < M; ++inner_a) {
                dummy2(inner_a);
                checksum += inner_a;
                
                /* This goto creates partial overlap:
                   Inner Loop A includes the 'shared_continuation' block
                   which is also in Outer Loop but outside the if branch */
                if (inner_a == M/2) {
                    goto shared_continuation;
                }
            }
            /* End of Inner Loop A normal path */
            dummy3(999);
        } else {
            /* Inner Loop B - different from A but shares prologue */
            for (volatile int inner_b = 0; inner_b < K; ++inner_b) {
                dummy4(inner_b);
                checksum -= inner_b;
            }
        }
        
        /* Shared continuation block - part of Outer Loop 
           and also part of Inner Loop A (via goto) */
        shared_continuation:
        asm volatile("" : : : "memory");
        checksum += outer;
        
        /* Prevent loop fusion */
        asm volatile("" : : : "memory");
    }
    
    /* Sibling Loop C - shares some blocks with outer loop structure
       but not all (partial overlap scenario) */
    {
        /* Reuse similar prologue pattern */
        volatile int setup = 42;
        dummy1(setup);
        
        /* Loop C body is different but shares dummy1 call */
        for (volatile int sibling = 0; sibling < N/2; ++sibling) {
            dummy3(sibling);
            checksum += sibling * 3;
        }
    }
    
    /* Another overlapping structure for more complex bitmap intersection */
    volatile int counter = 0;
    while (counter < 10) {
        /* Loop D - partially overlaps with the following loop E */
        for (volatile int d = 0; d < 5; ++d) {
            dummy1(d);
            if (d == 2) {
                /* Jump into Loop E's region */
                goto overlap_region;
            }
        }
        
        /* Loop E - shares overlap_region with Loop D */
        for (volatile int e = 0; e < 5; ++e) {
            overlap_region:
            dummy2(e);
            checksum += e;
            if (e == 3) break;
        }
        counter++;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
