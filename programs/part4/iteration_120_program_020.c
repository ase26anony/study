/* Test program for hw-doloop.cc partial overlap bitmap analysis */
/* Compile with: -O2 -march=rv32imc -fdump-rtl-doloop -fdump-rtl-all */

#include <stdio.h>
#include <stdint.h>

/* Dummy functions to create unique basic blocks */
__attribute__((noinline, noclone)) 
void dummy_func1(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

__attribute__((noinline, noclone))
void dummy_func2(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

__attribute__((noinline, noclone))
void dummy_func3(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

__attribute__((noinline, noclone))
void dummy_func4(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

/* Shared prologue block - will be part of multiple loops */
__attribute__((noinline, noclone))
int shared_prologue(int base) {
    volatile int val = base * 2;
    asm volatile("" : : : "memory");
    return val;
}

int main(void) {
    volatile int N = 1000;      /* Prevent constant propagation */
    volatile int M1 = 100;
    volatile int M2 = 200;
    volatile int M3 = 300;
    volatile int selector = 1;  /* Control which branch is taken */
    
    volatile int checksum = 0;
    
    /* COMMON_PROLOGUE: This block will be shared by multiple loops */
    int common_val = shared_prologue(42);
    
    /* OUTER_LOOP: Contains complex control flow */
    for (volatile int outer = 0; outer < N; ++outer) {
        asm volatile("" : : : "memory"); /* Prevent optimizations */
        
        /* This if-else creates two distinct paths in outer loop */
        if (selector > 0) {
            /* Branch 1: Contains INNER_LOOP_A */
            
            /* INNER_LOOP_A: Starts inside if branch but jumps outside */
            volatile int inner_a = 0;
            
            /* Loop prologue - part of outer loop's if branch */
            int loop_a_setup = common_val + 1;
            dummy_func1(loop_a_setup);
            
            /* The actual loop */
            for (inner_a = 0; inner_a < M1; ++inner_a) {
                dummy_func2(inner_a);
                checksum += inner_a;
                
                /* This goto creates partial overlap:
                 * Jumps to a block that's in outer loop but outside if branch */
                if (inner_a == M1/2) {
                    goto shared_block;
                }
            }
            
            /* Continuation after inner loop (still in if branch) */
            dummy_func3(checksum);
            continue; /* Skip else branch */
            
        shared_block:
            /* This block is:
             * 1. Inside INNER_LOOP_A (via goto)
             * 2. Inside OUTER_LOOP 
             * 3. Outside the if branch's main body
             * Creates partial overlap! */
            dummy_func4(inner_a);
            checksum += 999;
            
        } else {
            /* Branch 2: Contains INNER_LOOP_B */
            
            /* INNER_LOOP_B: Shares common_prologue but different body */
            int loop_b_setup = common_val + 2;
            dummy_func2(loop_b_setup);
            
            for (volatile int inner_b = 0; inner_b < M2; ++inner_b) {
                dummy_func3(inner_b);
                checksum -= inner_b;
                asm volatile("" : : : "memory");
            }
            
            dummy_func4(checksum);
        }
        
        /* Block after if-else, still in outer loop */
        checksum += outer;
    }
    
    /* SIBLING_LOOP_C: Shares common_prologue with inner loops 
     * but has different body and iteration count.
     * This creates another partial overlap scenario. */
    int sibling_setup = shared_prologue(84); /* Same function, different arg */
    dummy_func1(sibling_setup);
    
    for (volatile int sibling = 0; sibling < M3; ++sibling) {
        /* Different body from inner loops */
        dummy_func4(sibling);
        checksum *= 2;
        asm volatile("" : : : "memory");
        
        /* Small conditional to create extra basic blocks */
        if (sibling % 2 == 0) {
            dummy_func2(sibling);
        } else {
            dummy_func3(sibling);
        }
    }
    
    /* Final checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
