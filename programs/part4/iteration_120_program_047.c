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
int __attribute__((noinline)) shared_prologue(int base) {
    volatile int x = base * 2;
    asm volatile("" : "+r"(x) : : "memory");
    return x;
}

int main(void) {
    /* Volatile variables to prevent constant propagation */
    volatile int N = 1000;
    volatile int M1 = 100;
    volatile int M2 = 200;
    volatile int M3 = 300;
    volatile int cond = 1;
    
    volatile int checksum = 0;
    
    /* COMMON_PROLOGUE: This block will be shared by multiple loops */
    int common_val = shared_prologue(42);
    
    /* OUTER_LOOP: Complex loop with if-else structure */
    for (volatile int outer = 0; outer < N; ++outer) {
        /* Force memory barrier between iterations */
        asm volatile("" : : : "memory");
        
        /* This condition creates two distinct paths */
        if (cond) {
            /* Branch 1: Contains INNER_LOOP_A */
            
            /* Setup block for inner loop A */
            volatile int setup_a = common_val + outer;
            dummy1(setup_a);
            
            /* INNER_LOOP_A: Starts in outer's true branch */
            /* This loop will jump to SHARED_EXIT block */
            volatile int inner_a = 0;
            
            /* Loop header - part of outer's true branch */
            for (inner_a = 0; inner_a < M1; ++inner_a) {
                /* Body of inner loop A */
                dummy2(inner_a + outer);
                checksum += inner_a;
                
                /* Force unique basic block pattern */
                asm volatile("" : : : "memory");
                
                /* Jump to shared exit block - creates partial overlap */
                if (inner_a == M1 - 1) {
                    goto shared_exit;
                }
            }
            
            /* Normal exit from inner loop A */
            dummy3(123);
        } else {
            /* Branch 2: Contains INNER_LOOP_B */
            
            /* Setup block for inner loop B (different from A) */
            volatile int setup_b = common_val - outer;
            dummy4(setup_b);
            
            /* INNER_LOOP_B: Starts in outer's false branch */
            /* Shares common_prologue but has different body */
            volatile int inner_b = 0;
            for (inner_b = 0; inner_b < M2; ++inner_b) {
                /* Different body pattern */
                dummy1(inner_b * 2);
                checksum -= inner_b;
                asm volatile("" : : : "memory");
            }
            
            /* Different exit block */
            dummy2(456);
        }
        
        /* SHARED_EXIT: Block that is part of outer loop 
           and also targeted by INNER_LOOP_A's goto */
        shared_exit:
        checksum += outer;
        asm volatile("" : : : "memory");
    }
    
    /* SIBLING_LOOP_C: Sequential loop that shares common_prologue 
       but not all blocks with outer loop */
    /* Reset common_val to ensure block reuse */
    common_val = shared_prologue(84);
    
    volatile int sibling = 0;
    for (sibling = 0; sibling < M3; ++sibling) {
        /* Shares common_prologue but has completely different body */
        dummy3(sibling * 3);
        checksum += sibling * 2;
        
        /* Force memory barrier */
        asm volatile("" : : : "memory");
        
        /* Different control flow pattern */
        if (sibling % 2) {
            dummy4(sibling);
        } else {
            dummy1(sibling + 1);
        }
    }
    
    /* Final checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
