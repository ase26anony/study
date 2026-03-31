/* This program is designed to trigger specific uncovered lines in GCC's reorg.cc
   related to delay slot filling. It uses goto-label patterns with eligible
   follower instructions, carefully manages resource dependencies, and avoids
   trapping operations. Compile with -O2 -march=mips -fdump-rtl-reorg to see
   the delay slot filling in action. */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if not already default */
#ifdef __GNUC__
#define MIPS_TARGET __attribute__((target("arch=mips")))
#else
#define MIPS_TARGET
#endif

/* Volatile to prevent optimization of critical sections */
static volatile int guard = 0;

MIPS_TARGET
void compute_hash(int *input, int *output, int size) {
    int i;
    int a = 0, b = 0, c = 0, d = 0;
    int t1, t2, t3, t4;
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4;
    int s1 = 5, s2 = 6, s3 = 7, s4 = 8;
    
    for (i = 0; i < size; i++) {
        /* Create pressure for delay slot filling */
        if (__builtin_expect((input[i] & 1) != 0, 0)) {
            /* Jump pattern 1: simple goto to label with eligible follower */
            if (a < b) {
                goto label1;
            } else {
                a = b + c;
            }
            
            /* Memory barrier to constrain scheduling */
            __sync_synchronize();
            
            continue;
            
        label1:
            /* ELIGIBLE FOLLOWER: Simple non-trapping arithmetic
               Uses registers (r1,r2,r3) distinct from (a,b,c) */
            r1 = r2 + r3;  /* This should be candidate for delay slot */
            a = input[i] ^ b;
        }
        
        /* Alternate path with different jump pattern */
        if (__builtin_expect((input[i] & 2) != 0, 1)) {
            /* Force another goto-label pattern */
            if (c > d) {
                goto label2;
            }
            d = c - a;
            
            /* Another barrier */
            asm volatile("" ::: "memory");
            
            continue;
            
        label2:
            /* Another eligible follower: different register set */
            s1 = s2 | s3;  /* Bitwise OR - non-trapping */
            c = input[i] * 3;  /* Multiplication by constant is safe */
        }
        
        /* Third pattern with nested condition */
        if (i % 3 == 0) {
            int tmp = input[i];
            if (tmp % 5 == 0) {
                goto label3;
            } else if (tmp % 7 == 0) {
                goto label4;
            }
            b = tmp >> 2;
            continue;
            
        label3:
            /* Eligible: subtraction with distinct registers */
            t1 = r4 - s4;
            a = tmp + b;
            continue;
            
        label4:
            /* Eligible: bitwise AND */
            t2 = r2 & s2;
            d = tmp ^ c;
        }
        
        /* Mix in some floating point to diversify resource usage */
        if (i % 4 == 0) {
            float f1 = (float)input[i];
            float f2 = f1 * 2.0f;
            guard = (int)f2;  /* Use volatile to prevent elimination */
        }
        
        /* Complex accumulator to create data dependencies */
        output[i] = a ^ b ^ c ^ d ^ r1 ^ s1 ^ t1 ^ t2;
        
        /* Rotate registers to create varying patterns */
        t3 = a; a = b; b = c; c = d; d = t3;
        t4 = r1; r1 = r2; r2 = r3; r3 = r4; r4 = t4;
    }
}

/* Secondary function with different pattern */
MIPS_TARGET
void process_blocks(int *data, int blocks) {
    int i, j;
    int x = 0, y = 0, z = 0;
    
    for (i = 0; i < blocks; i++) {
        /* Unrolled inner loop for more scheduling opportunities */
        for (j = 0; j < 4; j++) {
            int idx = i * 4 + j;
            
            /* Pattern with computed goto-like behavior */
            switch (data[idx] % 4) {
                case 0:
                    if (x < y) goto block0;
                    break;
                case 1:
                    if (y < z) goto block1;
                    break;
                case 2:
                    if (z < x) goto block2;
                    break;
                default:
                    x = y + z;
                    continue;
            }
            
            /* Default arithmetic */
            x = x * 2 + 1;
            continue;
            
        block0:
            /* Eligible: addition with constants */
            y = x + 5;  /* Simple, non-trapping */
            data[idx] = y;
            continue;
            
        block1:
            /* Eligible: shift operation */
            z = y << 2;  /* Shift by constant is safe */
            data[idx] = z;
            continue;
            
        block2:
            /* Eligible: subtraction */
            x = z - y;
            data[idx] = x;
        }
        
        /* Occasionally insert barrier */
        if (i % 8 == 0) {
            __sync_synchronize();
        }
    }
}

MIPS_TARGET
int main() {
    const int SIZE = 256;
    const int BLOCKS = 64;
    int *input = (int*)malloc(SIZE * sizeof(int));
    int *output = (int*)malloc(SIZE * sizeof(int));
    int *blocks = (int*)malloc(BLOCKS * 4 * sizeof(int));
    
    if (!input || !output || !blocks) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern to trigger various branches */
    for (int i = 0; i < SIZE; i++) {
        input[i] = (i * 1103515245 + 12345) & 0x7fffffff;
    }
    for (int i = 0; i < BLOCKS * 4; i++) {
        blocks[i] = (i * 1664525 + 1013904223) & 0xff;
    }
    
    /* Execute the computational kernels */
    compute_hash(input, output, SIZE);
    process_blocks(blocks, BLOCKS);
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum ^= output[i];
    }
    for (int i = 0; i < BLOCKS * 4; i++) {
        sum += blocks[i];
    }
    
    printf("Result checksum: %d\n", sum);
    printf("Guard value: %d\n", guard);
    
    free(input);
    free(output);
    free(blocks);
    
    return 0;
}
