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

int main() {
    volatile int N = 1000;
    volatile int M = 100;
    volatile int K = 50;
    volatile int cond = 1;
    volatile int checksum = 0;
    
    /* Common prologue block shared by multiple loops */
    volatile int shared_counter = 0;
    dummy1(shared_counter);
    
    /* OUTER LOOP - contains complex control flow */
    for (volatile int outer = 0; outer < N; ++outer) {
        asm volatile("" : : : "memory");
        
        /* This if-else creates multiple basic blocks within outer loop */
        if (cond) {
            /* Branch 1: Contains INNER LOOP A with partial overlap */
            
            /* Prologue block for inner loop A (also part of outer loop) */
            volatile int inner_a_setup = outer * 2;
            dummy2(inner_a_setup);
            
            /* INNER LOOP A - starts inside if branch */
            for (volatile int inner_a = 0; inner_a < M; ++inner_a) {
                /* Body block A1 */
                checksum += inner_a * 3;
                dummy3(inner_a);
                
                /* Jump to shared block that's outside the if branch
                   but still within outer loop */
                if (inner_a == M/2) {
                    goto shared_block;
                }
                
                /* Body block A2 */
                checksum -= inner_a;
                dummy4(inner_a);
            }
            
            /* Continuation after inner loop A (only if no goto taken) */
            checksum += 100;
        } else {
            /* Branch 2: Contains INNER LOOP B */
            volatile int inner_b_setup = outer * 3;
            dummy2(inner_b_setup);
            
            /* INNER LOOP B - completely inside else branch */
            for (volatile int inner_b = 0; inner_b < K; ++inner_b) {
                checksum += inner_b * 5;
                dummy3(inner_b + 1000);
            }
            
            checksum += 200;
        }
        
        /* Shared block that's part of outer loop but outside specific branches */
        shared_block:
        checksum += outer;
        dummy1(checksum);
        
        /* Reset condition for next iteration */
        cond = !cond;
    }
    
    /* SIBLING LOOP C - shares prologue with inner loops but has different body */
    /* Re-use the shared_counter from the common prologue */
    dummy1(shared_counter + 1);
    
    for (volatile int sibling = 0; sibling < N * 2; ++sibling) {
        /* Different body from inner loops */
        checksum += sibling * 7;
        dummy2(sibling + 2000);
        
        /* Add some internal control flow to create more basic blocks */
        if (sibling % 3 == 0) {
            checksum -= 5;
            dummy3(sibling);
        } else {
            checksum += 3;
            dummy4(sibling);
        }
    }
    
    /* Another loop that partially overlaps with sibling loop C */
    /* They share the dummy2 call but have different loop structures */
    volatile int setup = checksum;
    dummy2(setup);
    
    for (volatile int partial = 0; partial < K; ++partial) {
        /* First block: shared pattern with sibling loop C */
        checksum += partial * 7;
        dummy2(partial + 3000);
        
        /* Second block: unique to this loop */
        checksum *= 2;
        dummy3(partial * 2);
        
        /* Third block: also unique */
        if (partial % 2 == 0) {
            checksum += 1;
            dummy4(partial);
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
