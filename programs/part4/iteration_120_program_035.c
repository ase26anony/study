/* Test program for hw-doloop.cc uncovered lines 429-436 */
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

/* Shared prologue block - will be part of multiple loops */
int shared_prologue(volatile int* counter) {
    int val = *counter;
    asm volatile("" : : : "memory");
    return val;
}

int main(void) {
    volatile int N = 1000;
    volatile int M1 = 100;
    volatile int M2 = 200;
    volatile int M3 = 300;
    volatile int cond = 1;
    
    volatile int checksum = 0;
    
    /* Outer Loop - contains complex control flow */
    for (volatile int i = 0; i < N; ++i) {
        /* Common setup block - shared by inner loops */
        volatile int setup = shared_prologue(&i);
        dummy1(setup);
        
        /* Branch creates separate basic blocks */
        if (cond) {
            /* Inner Loop A - starts inside true branch */
            volatile int j = 0;
            
            /* Loop header block */
        loop_a_start:
            if (j >= M1) goto loop_a_end;
            
            /* Loop body block A */
            dummy2(j);
            checksum += j;
            
            j++;
            goto loop_a_start;
            
        loop_a_end:
            /* Jump to shared block outside the if branch */
            goto shared_block;
        } else {
            /* Inner Loop B - different body, same iteration count */
            for (volatile int k = 0; k < M1; ++k) {
                dummy3(k);
                checksum -= k;
                asm volatile("" : : : "memory");
            }
        }
        
        /* This label is reached from Inner Loop A via goto */
        /* It's inside outer loop but outside the if branch */
        shared_block:
        dummy4(i);
        asm volatile("" : : : "memory");
        
        /* Toggle condition to ensure both branches are taken */
        cond = !cond;
    }
    
    /* Sibling Loop C - shares prologue with inner loops */
    /* but has different body and iteration count */
    volatile int setup = shared_prologue(&N);
    for (volatile int l = 0; l < M3; ++l) {
        /* Different dummy function to create unique blocks */
        dummy1(l + setup);
        dummy2(l * 2);
        checksum += l;
        asm volatile("" : : : "memory");
    }
    
    /* Additional complexity: Loop D that partially overlaps with C */
    /* by sharing some setup code but having different body */
    volatile int temp = checksum;
    for (volatile int m = 0; m < M2; ++m) {
        /* First part: shared operations with loop C */
        dummy1(m);
        asm volatile("" : : : "memory");
        
        /* Second part: unique operations */
        if (m % 2) {
            dummy3(m);
            checksum += temp;
        } else {
            dummy4(m);
            checksum -= temp;
        }
        
        /* Third part: more shared operations */
        dummy2(m);
        asm volatile("" : : : "memory");
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
