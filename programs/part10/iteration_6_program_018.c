/* delay_slot_test.c - Target GCC's delay slot filler for RISC architectures */
#include <stdio.h>
#include <stdlib.h>

/* Force register allocation for specific variables to avoid conflicts */
#define REG1 asm("$t0")
#define REG2 asm("$t1")
#define REG3 asm("$t2")
#define REG4 asm("$t3")
#define REG5 asm("$t4")
#define REG6 asm("$t5")

/* Helper to create predictable but varying branch conditions */
static volatile int global_seed = 42;

int main(void) {
    /* Use register variables to control allocation and prevent spilling */
    register int a REG1 = 0;
    register int b REG2 = 100;
    register int c REG3 = 0;
    register int d REG4 = 1;
    register int e REG5 = 2;
    register int f REG6 = 3;
    
    /* Volatile counter to prevent loop unrolling */
    volatile int iterations = 100;
    int i;
    
    /* Result accumulator to keep computations live */
    int result = 0;
    
    for (i = 0; i < iterations; i++) {
        /* Vary branch condition to create both taken and not-taken paths */
        int condition = (global_seed + i) & 1;
        
        /* 
         * PATTERN 1: Simple conditional branch with nop filler
         * This creates a trial instruction (nop) that could be replaced
         */
        if (__builtin_expect(a < b, condition)) {
            /* Insert nop that delay slot filler might try to move */
            asm volatile("nop" ::: "memory");
target_label_1:
            /* Candidate for delay slot: simple arithmetic, uses different registers */
            c = d + e;  /* REG3 = REG4 + REG5 - independent of branch condition */
        }
        
        /* Update variables to change future branch outcomes */
        a += i;
        b -= (i & 3);
        
        /* 
         * PATTERN 2: Another branch with different register usage
         * Uses different condition and target label
         */
        if (__builtin_expect(d > e, condition ^ 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");  /* Two nops for multiple slots */
target_label_2:
            /* Another independent operation */
            f = c + 2;  /* REG6 = REG3 + 2 */
        }
        
        /* 
         * PATTERN 3: Nested conditions to create complex control flow
         */
        if (__builtin_expect((a & 1) == 0, 0)) {
            if (__builtin_expect(b > 50, 1)) {
                asm volatile("nop" ::: "memory");
target_label_3:
                /* Simple move operation - good delay slot candidate */
                e = f;
            }
        }
        
        /* 
         * PATTERN 4: Loop with back edge - creates backward branch
         * which often has different delay slot filling behavior
         */
        int j;
        for (j = 0; j < 3; j++) {
            if (__builtin_expect(j == 2, 0)) {
                asm volatile("nop" ::: "memory");
                asm volatile("nop" ::: "memory");
                asm volatile("nop" ::: "memory");
target_label_4:
                /* Multiple independent operations */
                d = a + 1;
                c = b + 2;
            }
            result += j;
        }
        
        /* Accumulate results to keep variables live */
        result += a + b + c + d + e + f;
        
        /* Modify global seed to vary branch patterns */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional test with switch to create jump table */
    switch (result & 3) {
        case 0:
            if (__builtin_expect(c > d, 0)) {
                asm volatile("nop" ::: "memory");
target_label_5:
                f = e + 1;
            }
            break;
        case 1:
            if (__builtin_expect(a < c, 1)) {
                asm volatile("nop" ::: "memory");
target_label_6:
                d = f + 2;
            }
            break;
        default:
            /* Default case with its own branch */
            if (__builtin_expect(b > a, 0)) {
                asm volatile("nop" ::: "memory");
target_label_7:
                e = d + 3;
            }
    }
    
    return result & 255;  /* Return non-zero to indicate success */
}

/* Additional function to create cross-function branch opportunities */
static int helper_func(int x, int y) {
    register int r1 asm("$s0") = x;
    register int r2 asm("$s1") = y;
    register int r3 asm("$s2") = 0;
    register int r4 asm("$s3") = 0;
    
    /* Function with its own branch patterns */
    if (__builtin_expect(r1 > r2, 0)) {
        asm volatile("nop" ::: "memory");
helper_label_1:
        r3 = r4 + 5;  /* Independent operation */
    }
    
    return r1 + r2 + r3;
}
