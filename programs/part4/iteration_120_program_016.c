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
int __attribute__((noinline, noclone)) shared_prologue(int base) {
    volatile int x = base + 1;
    asm volatile("" : : : "memory");
    return x;
}

int main() {
    volatile int N = 1000;
    volatile int M1 = 100;
    volatile int M2 = 200;
    volatile int M3 = 300;
    volatile int selector = 1;
    
    volatile int checksum = 0;
    
    /* COMMON_PROLOGUE: This block will be shared by multiple loops */
    int common_val = shared_prologue(0);
    
    /* OUTER_LOOP: Contains complex control flow */
    for (volatile int outer = 0; outer < N; ++outer) {
        asm volatile("" : : : "memory");
        
        /* This if-else creates two distinct paths in the outer loop */
        if (selector > 0) {
            /* Branch 1: Contains INNER_LOOP_A */
            
            /* SHARED_BLOCK_A: Block shared between outer and inner loops */
            volatile int temp_a = common_val + outer;
            dummy1(temp_a);
            
            /* INNER_LOOP_A: Starts inside if branch but jumps outside */
            for (volatile int inner_a = 0; inner_a < M1; ++inner_a) {
                dummy2(inner_a + temp_a);
                checksum += inner_a;
                
                /* CRITICAL: Jump to a block outside the if branch 
                   but still within outer loop */
                if (inner_a == M1/2) {
                    goto shared_exit_block;
                }
            }
            
            /* This block is only executed if no goto taken */
            dummy3(999);
            continue;
            
        shared_exit_block:
            /* This block is part of outer loop and reached by inner loop goto */
            dummy4(888);
            checksum += 1000;
            continue;
            
        } else {
            /* Branch 2: Contains INNER_LOOP_B */
            
            /* SHARED_BLOCK_B: Different shared block for inner loop B */
            volatile int temp_b = common_val - outer;
            dummy3(temp_b);
            
            /* INNER_LOOP_B: Shares common_prologue but different body */
            for (volatile int inner_b = 0; inner_b < M2; ++inner_b) {
                dummy4(inner_b + temp_b);
                checksum -= inner_b;
            }
        }
        
        /* This block is part of outer loop after if-else */
        volatile int post_if = checksum % 100;
        dummy1(post_if);
    }
    
    /* Reset common_val for sibling loop */
    common_val = shared_prologue(100);
    
    /* SIBLING_LOOP_C: Shares common_prologue but disjoint from outer loop */
    for (volatile int sibling = 0; sibling < M3; ++sibling) {
        /* Uses same prologue but different body */
        volatile int temp_c = common_val + sibling * 2;
        dummy2(temp_c);
        checksum += temp_c;
        
        /* Different control flow pattern */
        if (sibling % 3 == 0) {
            dummy3(sibling);
        } else {
            dummy4(sibling);
        }
    }
    
    /* Additional complexity: Nested partial overlap scenario */
    {
        volatile int P = 50;
        volatile int Q = 60;
        
        /* LOOP_D and LOOP_E will partially overlap */
        int shared_counter = 0;
        
        /* LOOP_D */
        for (volatile int d = 0; d < P; ++d) {
            /* Block D1 */
            dummy1(d);
            shared_counter++;
            
            /* Block D2 (shared with LOOP_E) */
            volatile int mid = shared_counter * 2;
            dummy2(mid);
            
            /* Block D3 (not in LOOP_E) */
            if (d % 2 == 0) {
                dummy3(d);
            }
        }
        
        /* LOOP_E - partially overlaps with LOOP_D */
        for (volatile int e = 0; e < Q; ++e) {
            /* Block E1 (not in LOOP_D) */
            dummy4(e);
            
            /* Block D2 (shared with LOOP_D) */
            volatile int mid = shared_counter * 3;
            dummy2(mid);
            
            /* Block E3 (not in LOOP_D) */
            if (e % 3 == 0) {
                dummy1(e);
            }
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
