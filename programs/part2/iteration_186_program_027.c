/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow debug output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to generate complex flow graph with register pressure */
__attribute__((noinline))
static unsigned long test_mcf_pass(int iterations, int seed) {
    /* Declare many variables to create register pressure */
    volatile int v0 = seed, v1 = seed + 1, v2 = seed + 2, v3 = seed + 3;
    volatile int v4 = seed + 4, v5 = seed + 5, v6 = seed + 6, v7 = seed + 7;
    volatile int v8 = seed + 8, v9 = seed + 9, v10 = seed + 10, v11 = seed + 11;
    volatile int v12 = seed + 12, v13 = seed + 13, v14 = seed + 14, v15 = seed + 15;
    volatile int v16 = seed + 16, v17 = seed + 17, v18 = seed + 18, v19 = seed + 19;
    volatile int v20 = seed + 20, v21 = seed + 21, v22 = seed + 22, v23 = seed + 23;
    volatile int v24 = seed + 24, v25 = seed + 25, v26 = seed + 26, v27 = seed + 27;
    volatile int v28 = seed + 28, v29 = seed + 29;
    
    /* Mix different types to increase register class pressure */
    volatile float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f;
    volatile double d0 = seed * 0.01, d1 = seed * 0.02;
    volatile void* p0 = &v0, *p1 = &v1, *p2 = &v2;
    
    unsigned long checksum = 0;
    int i;
    
    /* Complex loop with many basic blocks */
    for (i = 0; i < iterations; i++) {
        /* Create data dependencies across variables */
        v0 = v1 + v2;
        v1 = v3 ^ v4;
        v2 = v5 * v6;
        v3 = v7 - v8;
        v4 = v9 / (v10 + 1);
        v5 = v11 | v12;
        v6 = v13 & v14;
        v7 = v15 << 2;
        v8 = v16 >> 1;
        v9 = v17 % (v18 + 1);
        v10 = v19 * v20;
        
        /* Complex conditional creating multiple basic blocks */
        if (v0 & 1) {
            v11 = v21 + v22;
            if (v1 > v2) {
                v12 = v23 * v24;
                /* Inline assembly with register clobbering */
                asm volatile (
                    "# Force register pressure\n"
                    : 
                    : "r"(v0), "r"(v1), "r"(v2)
                    : "eax", "ebx", "ecx", "edx", "memory"
                );
            } else {
                v12 = v25 - v26;
                /* Another inline assembly with different clobbers */
                asm volatile (
                    "# More register pressure\n"
                    : 
                    : "r"(v3), "r"(v4)
                    : "esi", "edi", "memory"
                );
            }
            v13 = v27 ^ v28;
        } else {
            v11 = v29 * v0;
            if (v1 < v2) {
                v12 = v0 / (v1 + 1);
            } else {
                v12 = v1 % (v2 + 1);
                /* Force spill/reload */
                asm volatile (
                    "# Complex constraint\n"
                    : "+r"(v12), "+r"(v13)
                    : "r"(v14), "r"(v15)
                    : "cc", "memory"
                );
            }
            v13 = v2 | v3;
        }
        
        /* Switch statement creating many basic blocks */
        switch (i & 0xF) {
            case 0: v14 = v4 + v5; break;
            case 1: v14 = v6 - v7; break;
            case 2: v14 = v8 * v9; break;
            case 3: v14 = v10 ^ v11; break;
            case 4: v14 = v12 | v13; break;
            case 5: v14 = v14 & v15; break;
            case 6: v14 = v16 << 1; break;
            case 7: v14 = v17 >> 2; break;
            case 8: v14 = v18 + v19; break;
            case 9: v14 = v20 - v21; break;
            case 10: v14 = v22 * v23; break;
            case 11: v14 = v24 ^ v25; break;
            case 12: v14 = v26 | v27; break;
            case 13: v14 = v28 & v29; break;
            case 14: v14 = v0 << 3; break;
            case 15: v14 = v1 >> 4; break;
        }
        
        /* More arithmetic with floating point mixing */
        f0 = f1 * 1.1f + v0;
        f1 = f2 / 2.0f - v1;
        d0 = d1 * 1.01 + v2;
        d1 = d0 / 1.5 - v3;
        
        /* Pointer arithmetic */
        p0 = (char*)p1 + v4;
        p1 = (char*)p2 - v5;
        p2 = (char*)p0 + v6;
        
        /* Update checksum */
        checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        checksum += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
        checksum += v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
        checksum += (unsigned long)f0 + (unsigned long)f1 + (unsigned long)d0 + (unsigned long)d1;
        checksum += (unsigned long)p0 + (unsigned long)p1 + (unsigned long)p2;
        
        /* Nested loop for more complexity */
        for (int j = 0; j < 3; j++) {
            v15 = v16 + j;
            v16 = v17 - j;
            v17 = v18 * (j + 1);
            
            /* Conditional goto creating irregular control flow */
            if (j == 1 && (checksum & 0x100)) {
                v18 = v19 ^ v20;
                goto special_label;
            }
            
            v19 = v20 | v21;
            continue;
            
        special_label:
            v20 = v21 & v22;
            v21 = v23 + v24;
        }
        
        /* Loop with break/continue creating exit-like regions */
        for (int k = 0; k < 5; k++) {
            if (k == 2 && (v0 & 0x10)) {
                v22 = v23 * v24;
                break;  /* Creates early exit */
            }
            if (k == 3 && (v1 & 0x20)) {
                v23 = v24 - v25;
                continue; /* Creates loop back edge */
            }
            v24 = v25 + v26;
            v25 = v26 ^ v27;
        }
    }
    
    /* Final computation mixing all variables */
    v26 = v27 + v28 + v29;
    v27 = v0 * v1 * v2;
    v28 = v3 ^ v4 ^ v5;
    v29 = v6 | v7 | v8;
    
    checksum += v26 + v27 + v28 + v29;
    checksum += (unsigned long)(f0 * 100.0f);
    checksum += (unsigned long)(d0 * 1000.0);
    
    return checksum;
}

/* Another complex function to ensure MCF runs multiple times */
__attribute__((noinline))
static unsigned long test_mcf_pass2(int iterations, int seed) {
    volatile int a0 = seed, a1 = seed * 2, a2 = seed * 3, a3 = seed * 4;
    volatile int a4 = seed * 5, a5 = seed * 6, a6 = seed * 7, a7 = seed * 8;
    volatile int a8 = seed * 9, a9 = seed * 10, a10 = seed * 11, a11 = seed * 12;
    volatile int a12 = seed * 13, a13 = seed * 14, a14 = seed * 15, a15 = seed * 16;
    
    unsigned long sum = 0;
    
    /* Different control flow pattern */
    for (int i = 0; i < iterations; i++) {
        /* Deep if-else chain */
        if (i % 2 == 0) {
            a0 = a1 + a2;
            if (i % 3 == 0) {
                a1 = a3 * a4;
                if (i % 5 == 0) {
                    a2 = a5 ^ a6;
                    if (i % 7 == 0) {
                        a3 = a7 | a8;
                    } else {
                        a3 = a9 & a10;
                    }
                } else {
                    a2 = a11 - a12;
                }
            } else {
                a1 = a13 / (a14 + 1);
            }
        } else {
            a0 = a15 << 1;
        }
        
        /* More inline assembly with constraints */
        asm volatile (
            "# Force graph transformations\n"
            : "+r"(a0), "+r"(a1), "+r"(a2)
            : "r"(a3), "r"(a4)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "cc", "memory"
        );
        
        sum += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11 + a12 + a13 + a14 + a15;
    }
    
    return sum;
}

int main(int argc, char** argv) {
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 100) iterations = 100;
        if (iterations > 10000) iterations = 10000;
    }
    
    unsigned long total = 0;
    
    /* Call test functions multiple times with different seeds */
    for (int s = 0; s < 10; s++) {
        total += test_mcf_pass(iterations, s * 12345);
        total += test_mcf_pass2(iterations / 2, s * 54321);
    }
    
    printf("MCF test checksum: %lu\n", total);
    return 0;
}
