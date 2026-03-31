/* mcf_trigger.c - Program to trigger GCC's Minimum Cost Flow debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to generate complex flow graph with many basic blocks */
__attribute__((noinline))
static unsigned long long test_mcf_pressure(int seed) {
    /* Declare many local variables to create register pressure */
    volatile int v0 = seed, v1 = seed + 1, v2 = seed + 2, v3 = seed + 3;
    volatile int v4 = seed + 4, v5 = seed + 5, v6 = seed + 6, v7 = seed + 7;
    volatile int v8 = seed + 8, v9 = seed + 9, v10 = seed + 10, v11 = seed + 11;
    volatile int v12 = seed + 12, v13 = seed + 13, v14 = seed + 14, v15 = seed + 15;
    volatile int v16 = seed + 16, v17 = seed + 17, v18 = seed + 18, v19 = seed + 19;
    volatile int v20 = seed + 20, v21 = seed + 21, v22 = seed + 22, v23 = seed + 23;
    volatile int v24 = seed + 24, v25 = seed + 25, v26 = seed + 26, v27 = seed + 27;
    volatile int v28 = seed + 28, v29 = seed + 29;
    
    volatile float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f;
    volatile float f3 = seed * 0.4f, f4 = seed * 0.5f;
    
    volatile void* p0 = &v0, *p1 = &v1, *p2 = &v2, *p3 = &v3;
    
    unsigned long long checksum = 0;
    int i, j;
    
    /* Complex loop with multiple exit points to create NEW_EXIT scenarios */
    for (i = 0; i < 100; i++) {
        /* Create data dependencies between variables */
        v0 = v1 + v2;
        v1 = v3 ^ v4;
        v2 = v5 * v6;
        v3 = v7 | v8;
        v4 = v9 & v10;
        
        /* Mix integer and float operations */
        f0 = (float)v0 * 0.5f;
        f1 = (float)v1 * 0.25f;
        v5 = (int)(f0 + f1);
        
        /* Pointer arithmetic */
        p0 = (char*)p1 + v2;
        p1 = (char*)p2 - v3;
        
        /* Inline assembly with register clobbering to force graph fixups */
        /* For x86/x86-64 */
        asm volatile (
            "# Force register pressure\n"
            : "=r"(v6), "=r"(v7)
            : "0"(v6), "1"(v7)
            : "eax", "ebx", "ecx", "edx", "memory"
        );
        
        /* Complex conditional with multiple branches */
        switch (i % 13) {
            case 0: v8 = v9 + v10; break;
            case 1: v8 = v11 - v12; break;
            case 2: v8 = v13 * v14; break;
            case 3: v8 = v15 / (v16 ? v16 : 1); break;
            case 4: v8 = v17 % (v18 ? v18 : 1); break;
            case 5: v8 = v19 & v20; break;
            case 6: v8 = v21 | v22; break;
            case 7: v8 = v23 ^ v24; break;
            case 8: v8 = ~v25; break;
            case 9: v8 = v26 << 2; break;
            case 10: v8 = v27 >> 3; break;
            case 11: v8 = -v28; break;
            case 12: v8 = abs(v29); break;
            default: v8 = 0; /* Unreachable but creates control flow */
        }
        
        /* Nested conditionals creating many basic blocks */
        if (i & 1) {
            if (v0 > v1) {
                v9 = v2 + v3;
                if (v9 < 1000) {
                    v10 = v4 * v5;
                    /* Early continue creates control flow merge point */
                    if (v10 == 0) continue;
                } else {
                    v10 = v6 / v7;
                }
            } else {
                v9 = v8 - v0;
                /* goto label to create additional control flow edges */
                if (v9 < 0) goto negative_path;
            }
            v11 = v9 + v10;
        } else {
            if (v1 < v2) {
                v11 = v3 * v4;
            } else {
                v11 = v5 ^ v6;
            }
        }
        
        /* Another inline asm with different clobbers */
        asm volatile (
            "# More register pressure\n"
            : "+r"(v12), "+r"(v13)
            :
            : "r8", "r9", "r10", "r11", "memory"
        );
        
        /* Loop with break/continue creating exit-like regions */
        for (j = 0; j < 5; j++) {
            v12 += v13;
            if (v12 > 10000) {
                /* Early break creates another exit region */
                break;
            }
            if (j == 3) {
                /* Continue creates merge point */
                v13 *= 2;
                continue;
            }
            v14 = v12 - v13;
        }
        
        checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                   v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                   v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
        
        /* Skip to end of iteration */
        if (checksum & 0x1000) goto loop_end;
        
        /* More computations */
        v15 = v16 ^ v17;
        v16 = v18 + v19;
        v17 = v20 * v21;
        
        /* Another switch for more control flow */
        switch (checksum % 7) {
            case 0: v18 = v22 + v23; break;
            case 1: v18 = v24 - v25; break;
            case 2: v18 = v26 * v27; break;
            case 3: v18 = v28 / (v29 ? v29 : 1); break;
            case 4: v18 = v0 & v1; break;
            case 5: v18 = v2 | v3; break;
            case 6: v18 = v4 ^ v5; break;
        }
        
        negative_path:
        v19 = v6 + v7;
        
        loop_end:
        /* Empty statement to create a basic block */
        ;
    }
    
    /* Final mixing */
    checksum ^= (unsigned long long)v0 << 32;
    checksum ^= (unsigned long long)v1 << 16;
    checksum ^= (unsigned long long)v2 << 8;
    checksum ^= (unsigned long long)v3;
    
    return checksum;
}

/* Wrapper to prevent inlining and create more call context */
__attribute__((noinline))
static unsigned long long run_test_suite(int iterations) {
    unsigned long long total = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        total += test_mcf_pressure(i);
        
        /* Alternate between different computation patterns */
        if (i & 1) {
            total = (total << 3) | (total >> 61); /* Rotate */
        } else {
            total ^= 0x5A5A5A5A5A5A5A5AULL;
        }
    }
    
    return total;
}

int main(int argc, char **argv) {
    int iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    unsigned long long result = run_test_suite(iterations);
    
    /* Print result to prevent dead code elimination */
    printf("MCF test result: 0x%016llx\n", result);
    
    return (result != 0) ? 0 : 1;
}
