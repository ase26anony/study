/* 
 * Test program specifically designed to trigger the uncovered delay slot 
 * filling logic in GCC's reorg.cc (lines 2135-2149).
 * Targets MIPS architecture with delay slots.
 * Compile with: gcc -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all -o reorg_test reorg_test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* 
 * Force MIPS target if not compiling natively on MIPS.
 * This attribute ensures delay slot handling is activated.
 */
#ifdef __GNUC__
#define MIPS_TARGET __attribute__((target("arch=mips")))
#else
#define MIPS_TARGET
#endif

/* Volatile variables to prevent unwanted optimizations */
static volatile int guard = 0;

/* 
 * Simple non-trapping integer operations that are safe for delay slot candidates.
 * These use distinct register sets to avoid resource conflicts.
 */
MIPS_TARGET
static int safe_op1(int a, int b) {
    return a + b;  /* Non-trapping addition */
}

MIPS_TARGET
static int safe_op2(int a, int b) {
    return a ^ b;  /* Non-trapping bitwise XOR */
}

MIPS_TARGET
static int safe_op3(int a, int b) {
    return a & ~b; /* Non-trapping bitwise AND with complement */
}

/*
 * Main computational kernel with label-oriented jump patterns.
 * Structured to create the specific RTL sequences needed:
 * 1. Unconditional jumps to labels (simplejump_p)
 * 2. Labels immediately followed by simple, splittable operations
 * 3. Non-overlapping resource usage between jump context and target operations
 * 4. No trapping operations after labels
 */
MIPS_TARGET
void compute_kernel(int* restrict arr1, int* restrict arr2, int size) {
    int i;
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    /* Use distinct register sets for different operations */
    int r1, r2, r3, r4, r5, r6;
    
    /* Initialize working registers with non-zero values */
    r1 = arr1[0];
    r2 = arr2[0];
    r3 = 1;
    r4 = 2;
    r5 = 3;
    r6 = 4;
    
    for (i = 0; i < size; i++) {
        /* 
         * Create pressure for delay slot filling by using __builtin_expect
         * and multiple conditional branches
         */
        if (__builtin_expect((arr1[i] & 1) != 0, 0)) {
            /* 
             * Pattern 1: Jump to label L1 with simple operation after it
             * This should generate: jump L1; nop
             * Where the nop could be filled by the operation at L1
             */
            if (guard) {
                /* This condition rarely true, but creates the jump pattern */
                goto L1;
            }
            
            /* Normal path continues here */
            r1 = safe_op1(r1, arr1[i]);
            continue;
        }
        
        /* Another branch to create scheduling pressure */
        if (__builtin_expect((arr2[i] & 2) != 0, 1)) {
            /* Pattern 2: Another jump to label L2 */
            if (guard) {
                goto L2;
            }
            
            r2 = safe_op2(r2, arr2[i]);
            continue;
        }
        
        /* 
         * Pattern 3: Jump to label L3 inside nested condition
         * Creates complex control flow for scheduler
         */
        if (i % 3 == 0) {
            if (guard) {
                goto L3;
            }
            
            r3 = safe_op3(r3, arr1[i]);
        }
        
        /* Mix in some memory barriers to constrain scheduling */
        if (i % 7 == 0) {
            __sync_synchronize();
        }
        
        /* Continue loop */
        continue;
        
        /* 
         * LABEL DEFINITIONS
         * Each label is immediately followed by a simple, non-trapping operation
         * that uses distinct resources from the jump context
         */
        
        L1:
            /* 
             * Candidate for delay slot filling:
             * - Simple integer operation (addition)
             * - Uses registers r4 and r5 (distinct from r1, r2, r3 used before jump)
             * - Non-trapping
             * - Not a jump or sequence
             */
            r4 = r4 + r5;  /* Simple add operation */
            continue;      /* Jump back to loop */
        
        L2:
            /* Another candidate with different resources */
            r5 = r5 ^ r6;  /* Bitwise XOR operation */
            continue;
        
        L3:
            /* Third candidate with yet another resource set */
            r6 = r6 & ~r4; /* Bitwise AND with complement */
            continue;
    }
    
    /* Use results to prevent dead code elimination */
    arr1[0] = r1 + r2 + r3 + r4 + r5 + r6;
}

/*
 * Secondary function with different jump pattern to increase coverage
 */
MIPS_TARGET
void secondary_kernel(int* restrict arr, int size) {
    int i, j;
    int temp = 0;
    
    /* Nested loops create complex control flow graph */
    for (i = 0; i < size; i++) {
        for (j = 0; j < 4; j++) {
            /* Create jump to label based on complex condition */
            if ((arr[i] + j) % 5 == 0) {
                if (guard) {
                    /* Jump to label L4 */
                    goto L4;
                }
                
                temp += arr[i] * j;
                continue;
            }
            
            /* Different operation to diversify resource usage */
            if ((arr[i] - j) % 3 == 0) {
                temp -= arr[i] / (j + 1);  /* Safe division, j+1 never 0 */
                continue;
            }
            
            continue;
            
            L4:
                /* 
                 * Another delay slot candidate:
                 * - Simple subtraction
                 * - Uses local variable 'temp' and constant
                 * - Non-trapping
                 */
                temp = temp - 1;
                continue;
        }
        
        /* Insert floating point operations in alternate path */
        if (i % 2 == 0) {
            float f = (float)arr[i];
            f = f * 1.5f;
            arr[i] = (int)f;
        }
    }
    
    /* Prevent elimination */
    arr[size-1] = temp;
}

MIPS_TARGET
int main() {
    const int SIZE = 256;
    int* arr1 = (int*)malloc(SIZE * sizeof(int));
    int* arr2 = (int*)malloc(SIZE * sizeof(int));
    
    if (!arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 13) % 97;
        arr2[i] = (i * 17) % 101;
    }
    
    /* 
     * Execute kernels multiple times to ensure scheduler sees
     * various execution patterns
     */
    for (int iter = 0; iter < 10; iter++) {
        compute_kernel(arr1, arr2, SIZE);
        secondary_kernel(arr1, SIZE);
        
        /* Modify guard occasionally to explore different paths */
        guard = iter % 3;
    }
    
    /* Compute and print checksum to verify execution */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= arr1[i];
        checksum += arr2[i];
    }
    
    printf("Result checksum: %d\n", checksum);
    
    free(arr1);
    free(arr2);
    
    return 0;
}
