/* Test program for hw-doloop.cc partial overlap bitmap analysis */
/* Compile with: -O2 -march=rv32imc -fdump-rtl-doloop -fdump-rtl-all */

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
int __attribute__((noinline, noclone)) shared_prologue(int base) {
    volatile int x = base * 2;
    asm volatile("" : : : "memory");
    return x + 1;
}

int main() {
    volatile int N = 1000;
    volatile int M1 = 100;
    volatile int M2 = 200;
    volatile int M3 = 300;
    volatile int selector = 1;
    volatile int checksum = 0;
    
    /* Outer Loop - contains complex control flow */
    for (volatile int outer = 0; outer < N; ++outer) {
        /* Common setup block - part of outer loop */
        int common_val = shared_prologue(outer);
        checksum += common_val;
        
        /* Branch creates multiple basic blocks in outer loop */
        if (selector > 0) {
            /* Inner Loop A - starts inside true branch */
            /* This loop will partially overlap with outer loop */
            volatile int inner_a = 0;
            
            /* Label for goto - creates partial overlap */
            loop_a_start:
            for (inner_a = 0; inner_a < M1; ++inner_a) {
                dummy1(inner_a + common_val);
                checksum += inner_a;
                
                /* Conditional goto to create partial overlap */
                /* Jumps to a block outside the if branch but still in outer loop */
                if (inner_a == M1/2) {
                    goto shared_continuation;
                }
            }
            
            /* This block is part of Inner Loop A's exit but also in outer loop */
            dummy2(common_val);
            checksum += 777;
            
            /* Jump to avoid else branch when goto wasn't taken */
            goto outer_loop_end;
        } else {
            /* Inner Loop B - different from A but shares common prologue */
            for (volatile int inner_b = 0; inner_b < M2; ++inner_b) {
                dummy3(inner_b + common_val);
                checksum -= inner_b;
            }
            dummy4(common_val);
            checksum += 888;
        }
        
        /* Shared continuation block - part of outer loop, 
           can be reached from Inner Loop A via goto */
        shared_continuation:
        checksum += 999;
        
        outer_loop_end:
        /* Prevent loop optimizations */
        asm volatile("" : : : "memory");
        
        /* Toggle selector to exercise both branches */
        selector = -selector;
    }
    
    /* Sibling Loop C - shares some blocks with outer loop structure */
    /* Uses similar prologue pattern but different body */
    volatile int sibling_base = 42;
    int common_val2 = shared_prologue(sibling_base);
    
    for (volatile int sibling = 0; sibling < M3; ++sibling) {
        /* This prologue call creates block intersection with outer loop */
        if (sibling % 2 == 0) {
            dummy1(common_val2 + sibling);
        } else {
            dummy2(common_val2 - sibling);
        }
        checksum += sibling * 2;
        
        /* Different control flow than inner loops */
        if (sibling == M3/3) {
            dummy3(checksum);
        }
    }
    
    /* Another loop that shares the goto label block */
    /* This creates additional partial overlap scenarios */
    volatile int extra_loop = 0;
    shared_label:
    for (extra_loop = 0; extra_loop < 50; ++extra_loop) {
        dummy4(extra_loop);
        checksum += extra_loop * 3;
        
        /* Jump back to label - creates loop with shared entry/exit */
        if (extra_loop == 25) {
            goto loop_a_start;  /* Creates overlap with Inner Loop A */
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
