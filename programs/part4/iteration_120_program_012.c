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
int __attribute__((noinline, noclone)) shared_prologue(int base) {
    volatile int shared = base * 2;
    asm volatile("" : : : "memory");
    return shared + 1;
}

int main() {
    volatile int N = 1000;      /* Prevent constant propagation */
    volatile int M1 = 100;
    volatile int M2 = 200;
    volatile int M3 = 300;
    volatile int cond = 1;      /* Control which branch is taken */
    
    volatile int checksum = 0;
    volatile int counter = 0;
    
    /* COMMON_PROLOGUE: This block will be shared by multiple loops */
    int shared_val = shared_prologue(42);
    
    /* OUTER_LOOP: Contains complex control flow */
    for (volatile int outer = 0; outer < N; ++outer) {
        asm volatile("" : : : "memory");  /* Barrier to prevent optimization */
        
        /* This if-else creates two distinct paths in the outer loop */
        if (cond) {
            /* Branch 1: Contains INNER_LOOP_A */
            
            /* Setup specific to inner loop A */
            volatile int setup_a = shared_val + outer;
            dummy1(setup_a);
            
            /* INNER_LOOP_A: Starts inside the if branch */
            /* This loop will partially overlap with outer loop */
            for (volatile int inner_a = 0; inner_a < M1; ++inner_a) {
                /* Body of inner loop A */
                dummy2(inner_a + outer);
                checksum += inner_a;
                
                /* CRITICAL: Jump to a block outside the if branch 
                   but still within the outer loop */
                if (inner_a == M1 - 1) {
                    goto shared_block;  /* Creates partial overlap */
                }
            }
            
            /* This block is only reached if goto is not taken */
            dummy3(999);
            continue;  /* Skip the shared block */
            
        } else {
            /* Branch 2: Contains INNER_LOOP_B */
            
            /* Setup specific to inner loop B */
            volatile int setup_b = shared_val - outer;
            dummy1(setup_b);
            
            /* INNER_LOOP_B: Completely inside the else branch */
            /* Shares common prologue but has distinct body */
            for (volatile int inner_b = 0; inner_b < M2; ++inner_b) {
                dummy4(inner_b * outer);
                checksum -= inner_b;
            }
            
            dummy2(888);
            continue;  /* Skip the shared block */
        }
        
        /* SHARED_BLOCK: This block is part of outer loop and 
           also reached by INNER_LOOP_A via goto */
        shared_block:
        counter++;
        dummy3(counter);
        
        /* This asm prevents the compiler from optimizing away the goto */
        asm volatile("" : : : "memory");
    }
    
    /* SIBLING_LOOP_C: Shares common prologue with outer loop 
       but has different body - creates partial overlap */
    /* Reset shared value to ensure prologue is used again */
    shared_val = shared_prologue(24);
    
    /* SIBLING_LOOP_C: Sequential loop that shares some blocks */
    for (volatile int sibling = 0; sibling < M3; ++sibling) {
        /* Uses same dummy1 as outer loop branches */
        dummy1(sibling + shared_val);
        
        /* Different body from inner loops */
        for (volatile int inner_c = 0; inner_c < 50; ++inner_c) {
            dummy4(inner_c * sibling);
            checksum += inner_c * 2;
        }
        
        asm volatile("" : : : "memory");
    }
    
    /* Prevent dead code elimination */
    printf("Checksum: %d, Counter: %d\n", checksum, counter);
    
    return 0;
}
