/* Test program for hw-doloop.cc partial overlap bitmap analysis */
/* Compile with: -O2 -march=rv32imc -fdump-rtl-doloop */

#include <stdio.h>
#include <stdlib.h>

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
    volatile int N = 1000;      /* Prevent constant propagation */
    volatile int M = 100;
    volatile int K = 50;
    volatile int condition = 0;
    volatile int checksum = 0;
    
    /* Outer Loop - will contain partial overlaps */
    for (volatile int outer = 0; outer < N; ++outer) {
        /* Common prologue block - shared by inner loops */
        volatile int shared = outer * 2;
        dummy1(shared);
        
        /* Branch creates separate basic blocks */
        if (condition) {
            /* Inner Loop A - starts inside true branch */
            volatile int inner_a;
            for (inner_a = 0; inner_a < M; ++inner_a) {
                dummy2(inner_a);
                checksum += inner_a;
                
                /* Jump to shared continuation block */
                if (inner_a == M/2) {
                    goto shared_continuation;
                }
            }
            /* End of Inner Loop A normal path */
            dummy3(999);
        } else {
            /* Inner Loop B - different body, same iteration count */
            volatile int inner_b;
            for (inner_b = 0; inner_b < M; ++inner_b) {
                dummy3(inner_b);
                checksum -= inner_b;
            }
            dummy4(888);
        }
        
        /* Shared continuation block - part of outer loop but outside branches */
        shared_continuation:
        asm volatile("" : : : "memory");
        checksum += shared;
        
        /* Toggle condition to exercise both paths */
        condition = !condition;
    }
    
    /* Sibling Loop C - shares prologue with inner loops but different body */
    /* This creates partial overlap with outer loop's blocks */
    volatile int sibling;
    for (sibling = 0; sibling < K; ++sibling) {
        /* Reuse similar prologue pattern */
        volatile int shared2 = sibling * 3;
        dummy1(shared2);  /* Same function as outer's prologue */
        
        /* Different body from inner loops */
        dummy4(sibling);
        checksum += sibling * 2;
        
        /* Different control flow pattern */
        if (sibling % 3 == 0) {
            dummy2(sibling * 10);
        }
    }
    
    /* Additional complexity: Loop D that partially overlaps with sibling */
    volatile int loop_d;
    for (loop_d = 0; loop_d < K/2; ++loop_d) {
        /* Share some blocks with sibling loop */
        volatile int shared3 = loop_d * 4;
        dummy1(shared3);  /* Shared with sibling's prologue */
        
        /* But also have unique blocks */
        if (loop_d % 2 == 0) {
            dummy3(loop_d * 20);
        } else {
            dummy4(loop_d * 30);
        }
        
        /* Jump back to create more complex CFG */
        if (loop_d == K/4) {
            volatile int temp = loop_d;
            while (temp > 0) {
                dummy2(temp);
                temp--;
            }
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
