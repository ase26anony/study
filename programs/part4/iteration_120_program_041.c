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

/* Shared prologue block - will be part of multiple loops */
int shared_prologue(volatile int init) {
    int val = init * 2;
    asm volatile("" : "+r"(val) : : "memory");
    return val;
}

int main() {
    volatile int N = 1000;
    volatile int M = 100;
    volatile int K = 50;
    volatile int condition = 1;
    volatile int checksum = 0;
    
    /* Outer Loop - contains complex control flow */
    for (volatile int outer = 0; outer < N; ++outer) {
        /* Common setup block - part of outer loop */
        int common_val = shared_prologue(outer);
        
        /* This creates partial overlap scenario */
        if (condition) {
            /* Inner Loop A - starts inside if branch */
            volatile int inner_a;
            for (inner_a = 0; inner_a < M; ++inner_a) {
                dummy1(inner_a + common_val);
                checksum += inner_a;
                
                /* Jump to shared block outside if-else */
                if (inner_a == M/2) {
                    goto shared_block;
                }
            }
            /* End of Inner Loop A normal path */
            dummy2(common_val);
        } else {
            /* Inner Loop B - different body, same prologue */
            volatile int inner_b;
            for (inner_b = 0; inner_b < K; ++inner_b) {
                dummy3(inner_b - common_val);
                checksum -= inner_b;
            }
            dummy4(common_val);
        }
        
shared_block:
        /* This block is part of outer loop and reached by:
           - Normal flow from if/else branches
           - goto from Inner Loop A
           This creates partial overlap between Inner Loop A and Outer Loop */
        checksum += common_val;
        asm volatile("" : : : "memory");
    }
    
    /* Sibling Loop C - shares prologue with inner loops but different body */
    /* This creates bitmap intersection but not subset relationship */
    volatile int setup = shared_prologue(42);
    for (volatile int sibling = 0; sibling < N/2; ++sibling) {
        /* Different dummy function to create unique blocks */
        dummy1(sibling + setup);
        dummy3(sibling - setup);
        checksum += sibling * 2;
        asm volatile("" : : : "memory");
    }
    
    /* Another loop that shares some blocks with outer loop */
    volatile int temp = 0;
    for (volatile int i = 0; i < M; ++i) {
        /* This block is similar to outer loop's shared_block */
        temp += i;
        asm volatile("" : : : "memory");
        
        if (i % 3 == 0) {
            /* Different path to create partial overlap */
            dummy2(i);
        } else {
            /* Same dummy as outer loop's else branch */
            dummy4(i);
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
