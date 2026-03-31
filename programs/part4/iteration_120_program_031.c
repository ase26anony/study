/* hw-doloop-test.c
 * Test program to trigger partial loop block overlap conditions in hw-doloop.cc
 * Compile with: gcc -O2 -march=rv32imc -fdump-rtl-doloop -o test hw-doloop-test.c
 */

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

/* Shared prologue block used by multiple loops */
static inline void shared_prologue(volatile int *counter) {
    (*counter)++;
    asm volatile("" : : : "memory");
}

int main() {
    volatile int N = 1000;
    volatile int M1 = 100;
    volatile int M2 = 200;
    volatile int M3 = 300;
    volatile int cond = 1;
    volatile int checksum = 0;
    
    /* COMMON_PROLOGUE: Shared block used by inner loops */
    volatile int common_counter = 0;
    
    /* OUTER_LOOP: Contains complex body with if-else */
    for (volatile int i = 0; i < N; i++) {
        checksum += i;
        
        /* Call dummy to create unique block in outer loop */
        dummy1(i);
        
        /* Use shared prologue - this block will be in outer loop's bitmap */
        shared_prologue(&common_counter);
        
        if (cond) {
            /* Branch 1: Contains INNER_LOOP_A */
            
            /* Pre-inner loop block (unique to this branch) */
            dummy2(i * 2);
            
            /* INNER_LOOP_A: Starts inside if branch */
            for (volatile int j = 0; j < M1; j++) {
                checksum += j * 3;
                dummy3(j);
                
                /* CRITICAL: Jump to shared block outside if-else */
                if (j == M1 - 1) {
                    goto shared_block;
                }
            }
            
            /* Block only reached if no goto taken (shouldn't happen) */
            dummy4(-1);
        } else {
            /* Branch 2: Contains INNER_LOOP_B */
            
            /* Different pre-inner loop block */
            dummy2(i * 3);
            
            /* INNER_LOOP_B: Different iteration count, shares prologue */
            for (volatile int k = 0; k < M2; k++) {
                checksum += k * 5;
                dummy4(k);
            }
        }
        
        /* Shared block that both inner loops can reach */
        shared_block:
        asm volatile("" : : : "memory");
        checksum += 7;
    }
    
    /* SIBLING_LOOP_C: Shares common_prologue with inner loops 
     * but has different body and iteration count.
     * This creates partial overlap with outer loop's bitmap.
     */
    shared_prologue(&common_counter);  // Shared block
    
    for (volatile int l = 0; l < M3; l++) {
        /* Different body from inner loops */
        checksum += l * 11;
        dummy1(l * 2);
        dummy2(l * 3);
        
        /* Additional unique block not in other loops */
        if (l % 2 == 0) {
            dummy3(l);
        } else {
            dummy4(l);
        }
    }
    
    /* Another loop that shares some blocks with SIBLING_LOOP_C */
    volatile int M4 = 150;
    shared_prologue(&common_counter);  // Shared block again
    
    for (volatile int m = 0; m < M4; m++) {
        /* Shares some but not all blocks with SIBLING_LOOP_C */
        checksum += m * 13;
        dummy1(m * 2);  // Shared with SIBLING_LOOP_C
        dummy3(m * 4);  // Different from SIBLING_LOOP_C
    }
    
    /* Prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    printf("Common counter: %d\n", common_counter);
    
    return checksum > 0 ? 0 : 1;
}
