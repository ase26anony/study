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

/* Shared prologue block - will be part of multiple loop bitmaps */
int __attribute__((noinline)) shared_prologue(int val) {
    asm volatile("" : "+r"(val) : : "memory");
    return val * 2;
}

int main() {
    volatile int N = 1000;
    volatile int M = 100;
    volatile int K = 50;
    volatile int condition = 1;
    volatile int checksum = 0;
    
    /* COMMON_PROLOGUE: This block will be shared by multiple loops */
    int shared_val = shared_prologue(42);
    
    /* OUTER_LOOP: Contains complex control flow with multiple basic blocks */
    for (volatile int outer = 0; outer < N; ++outer) {
        asm volatile("" : : : "memory");  /* Prevent optimization */
        
        /* Branch creates separate basic blocks within outer loop */
        if (condition) {
            /* Branch 1 - will contain Inner Loop A */
            
            /* Setup block for Inner Loop A (unique to this branch) */
            int setup_a = shared_val + outer;
            dummy1(setup_a);
            
            /* INNER_LOOP_A: Starts inside if branch but jumps outside */
            for (volatile int inner_a = 0; inner_a < M; ++inner_a) {
                /* Body block A1 */
                dummy2(inner_a + outer);
                checksum += inner_a;
                
                /* This goto creates partial overlap:
                   Jumps to a block that's in outer loop but outside if branch */
                if (inner_a == M/2) {
                    goto shared_block;  /* Jump to block shared with outer loop */
                }
                
                /* Body block A2 (not always executed due to goto) */
                dummy3(inner_a * 2);
            }
            
            /* Continuation after inner loop (only if no goto taken) */
            dummy4(outer * 3);
            continue;
            
        shared_block:
            /* This block is:
               1. Inside Outer Loop
               2. Outside the if branch (after the if-else)
               3. Inside Inner Loop A (via goto)
               
               Creates partial overlap between Outer and Inner A */
            checksum += 777;
            asm volatile("" : : : "memory");
        }
        else {
            /* Branch 2 - contains Inner Loop B */
            
            /* Setup block for Inner Loop B (different from A's setup) */
            int setup_b = shared_val - outer;
            dummy2(setup_b);
            
            /* INNER_LOOP_B: Completely inside else branch */
            for (volatile int inner_b = 0; inner_b < K; ++inner_b) {
                /* Different body pattern from Loop A */
                dummy3(inner_b * 3);
                checksum -= inner_b;
                asm volatile("" : : : "memory");
            }
            
            dummy4(outer * 7);
        }
        
        /* This block is in Outer Loop but outside both if/else branches */
        checksum += outer % 7;
    }
    
    /* Reset shared prologue for sibling loop */
    asm volatile("" : : : "memory");
    shared_val = shared_prologue(24);
    
    /* SIBLING_LOOP_C: Shares prologue with outer/inner loops but has 
       different body and iteration count - creates partial overlap */
    for (volatile int sibling = 0; sibling < N/2; ++sibling) {
        /* Uses same prologue but different body blocks */
        int val = shared_val + sibling;
        dummy1(val);
        checksum += val % 11;
        
        /* Additional unique block in sibling */
        if (sibling % 3 == 0) {
            dummy2(sibling);
            checksum += 5;
        }
        
        asm volatile("" : : : "memory");
    }
    
    /* Another loop that shares exit block with sibling */
    for (volatile int extra = 0; extra < 10; ++extra) {
        dummy3(extra);
        checksum += extra * 2;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
