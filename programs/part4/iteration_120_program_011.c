/* hw-doloop-test.c
 * Test program to trigger partial loop block overlap conditions in GCC's hw-doloop pass.
 * Compile with: -O2 -march=rv32imc -fdump-rtl-doloop -fdump-rtl-all
 */

#include <stdio.h>

/* Dummy functions to create unique basic blocks and prevent optimization */
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

int main(void) {
    volatile int N = 1000;      /* Prevent constant propagation */
    volatile int M1 = 100;
    volatile int M2 = 200;
    volatile int M3 = 300;
    volatile int cond = 1;      /* Control which branch is taken */
    volatile int checksum = 0;
    
    /* Outer Loop - contains complex control flow */
    for (volatile int i = 0; i < N; ++i) {
        checksum += i;
        
        /* Common setup block - will be shared with inner loops */
        volatile int common = i * 2;
        shared_prologue(&common);
        
        /* Branch that creates partial overlap */
        if (cond) {
            /* Inner Loop A - starts inside true branch */
            /* This loop will partially overlap with outer loop */
            volatile int j = 0;
            
            /* Loop prologue - part of outer loop's true branch */
            dummy1(common);
            
shared_block_label:
            /* This label creates a shared block between inner and outer loops */
            dummy2(j);
            
            /* Inner Loop A body */
            for (j = 0; j < M1; ++j) {
                checksum += j * 3;
                dummy3(j);
                
                /* Jump to shared block that's outside the if branch
                 * but still within outer loop */
                if (j == M1/2) {
                    goto shared_block_label;
                }
            }
            
            /* Continuation after inner loop in true branch */
            dummy4(common);
        } else {
            /* Inner Loop B - different from Loop A but shares common prologue */
            volatile int k = 0;
            dummy1(common + 1);  /* Different argument to prevent block merging */
            
            for (k = 0; k < M2; ++k) {
                checksum += k * 5;
                dummy2(k + 1000);  /* Different from Loop A's dummy2 calls */
            }
        }
        
        /* Shared block that Inner Loop A can jump to */
        /* This creates partial overlap: Inner Loop A contains this block,
         * but Outer Loop also contains it, and it's outside the if branch */
        checksum += 7;
        asm volatile("" : : : "memory");
    }
    
    /* Sibling Loop C - shares common prologue with inner loops 
     * but has different body and iteration count */
    {
        volatile int common = 0;
        shared_prologue(&common);
        
        /* Loop C uses the same prologue pattern but different body */
        for (volatile int l = 0; l < M3; ++l) {
            checksum += l * 11;
            dummy1(l * 2);      /* Different pattern from inner loops */
            dummy3(l + 2000);   /* Different from other dummy3 calls */
        }
    }
    
    /* Additional complexity to prevent loop fusion */
    asm volatile("" : : : "memory");
    
    /* Another loop that shares some blocks with outer loop */
    {
        volatile int setup = checksum;
        shared_prologue(&setup);
        
        for (volatile int m = 0; m < 150; ++m) {
            checksum += m * 13;
            /* Use some of the same dummy functions but with different args */
            if (m % 2) {
                dummy2(m * 3);
            } else {
                dummy4(m * 5);
            }
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
