/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all variables alive */
#define USE(x) asm volatile("" : : "r"(x))

/* Complex test function with high register pressure */
static int __attribute__((noinline)) test_mcf(int seed) {
    /* Declare many local variables to create register pressure */
    volatile int v0 = seed + 1;
    volatile int v1 = seed * 2;
    volatile int v2 = seed / 3;
    volatile int v3 = seed ^ 0x1234;
    volatile int v4 = seed | 0xABCD;
    volatile int v5 = seed & 0xF0F0;
    volatile int v6 = seed << 2;
    volatile int v7 = seed >> 1;
    volatile int v8 = seed + v0;
    volatile int v9 = v1 * v2;
    volatile int v10 = v3 - v4;
    volatile int v11 = v5 ^ v6;
    volatile int v12 = v7 | v8;
    volatile int v13 = v9 & v10;
    volatile int v14 = v11 << 1;
    volatile int v15 = v12 >> 2;
    volatile int v16 = v13 + v14;
    volatile int v17 = v15 * v16;
    volatile int v18 = v17 / (seed ? seed : 1);
    volatile int v19 = v18 ^ v0;
    volatile int v20 = v19 | v1;
    volatile int v21 = v20 & v2;
    volatile int v22 = v21 << 3;
    volatile int v23 = v22 >> 1;
    volatile int v24 = v23 + v3;
    volatile int v25 = v24 * v4;
    volatile int v26 = v25 - v5;
    volatile int v27 = v26 ^ v6;
    volatile int v28 = v27 | v7;
    volatile int v29 = v28 & v8;
    
    int result = 0;
    
    /* Complex control flow with many basic blocks */
    for (int i = 0; i < 100; i++) {
        /* Inline assembly to clobber registers and force graph transformations */
        asm volatile (
            "# Force register clobbering\n"
            "movl %%eax, %%ebx\n"
            "movl %%ecx, %%edx\n"
            : 
            : "a"(v0), "c"(v1)
            : "ebx", "edx", "memory"
        );
        
        /* Deep conditional chain creating many basic blocks */
        if (i % 3 == 0) {
            v0 = v1 + v2;
            if (i % 5 == 0) {
                v3 = v4 * v5;
                if (i % 7 == 0) {
                    v6 = v7 ^ v8;
                    if (i % 11 == 0) {
                        v9 = v10 | v11;
                        /* Another inline asm with different clobbers */
                        asm volatile (
                            "# More register pressure\n"
                            "addl %%esi, %%edi\n"
                            : 
                            : "S"(v12), "D"(v13)
                            : "cc", "memory"
                        );
                    } else {
                        v12 = v13 & v14;
                    }
                } else {
                    v15 = v16 << 1;
                }
            } else {
                v17 = v18 >> 2;
            }
        } else if (i % 4 == 0) {
            v19 = v20 + v21;
        } else {
            v22 = v23 * v24;
        }
        
        /* Switch statement with many cases */
        switch (i % 13) {
            case 0: v25 = v26 + 1; break;
            case 1: v27 = v28 - 2; break;
            case 2: v29 = v0 * 3; break;
            case 3: v1 = v2 / 4; break;
            case 4: v3 = v4 ^ 5; break;
            case 5: v5 = v6 | 6; break;
            case 6: v7 = v8 & 7; break;
            case 7: v9 = v10 << 8; break;
            case 8: v11 = v12 >> 9; break;
            case 9: v13 = v14 + v15; break;
            case 10: v16 = v17 * v18; break;
            case 11: v19 = v20 - v21; break;
            case 12: v22 = v23 ^ v24; break;
        }
        
        /* Loop with break/continue creating exit-like regions */
        for (int j = 0; j < 10; j++) {
            if (j == 5 && (i % 17 == 0)) {
                /* Early exit from inner loop */
                v25 += v26;
                break;
            }
            if (j == 3 && (i % 19 == 0)) {
                /* Continue creates another control flow edge */
                v27 -= v28;
                continue;
            }
            v29 = v29 + j;
        }
        
        /* Use all variables to keep them alive */
        result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(i));
    }
    
    /* Final computation mixing all variables */
    result = (v0 ^ v1) + (v2 | v3) - (v4 & v5) * (v6 << v7) / 
             ((v8 ? v8 : 1) + (v9 >> v10)) + (v11 + v12) * (v13 - v14) +
             (v15 ^ v16) | (v17 & v18) + (v19 << 2) - (v20 >> 1) +
             (v21 * v22) / ((v23 ? v23 : 1) + 1) + (v24 | v25) ^ 
             (v26 & v27) + (v28 << v29);
    
    return result;
}

/* Another complex function to ensure multiple functions are processed */
static int __attribute__((noinline)) test_mcf2(int seed) {
    volatile float f0 = seed * 1.1f;
    volatile float f1 = seed * 2.2f;
    volatile float f2 = seed * 3.3f;
    volatile float f3 = seed * 4.4f;
    volatile float f4 = seed * 5.5f;
    
    volatile double d0 = seed * 1.11;
    volatile double d1 = seed * 2.22;
    volatile double d2 = seed * 3.33;
    
    /* Mix float and integer operations */
    int vi0 = (int)f0;
    int vi1 = (int)f1;
    int vi2 = (int)f2;
    
    /* Pointer variables for additional pressure */
    volatile int* p0 = &vi0;
    volatile int* p1 = &vi1;
    volatile int* p2 = &vi2;
    
    int sum = 0;
    for (int i = 0; i < 50; i++) {
        /* Complex floating point operations */
        f0 = f1 * f2 + f3 - f4;
        f1 = f2 / (f0 ? f0 : 1.0f) + f4;
        f2 = f3 * f4 - f0;
        f3 = f4 + f1 * f2;
        f4 = f0 - f1 / f2;
        
        /* Double precision operations */
        d0 = d1 + d2;
        d1 = d2 * d0;
        d2 = d0 - d1;
        
        /* Integer operations */
        vi0 = vi1 ^ vi2;
        vi1 = vi2 | vi0;
        vi2 = vi0 & vi1;
        
        /* Pointer dereferencing */
        *p0 += i;
        *p1 -= i * 2;
        *p2 ^= i * 3;
        
        /* Inline asm with floating point clobbers */
        asm volatile (
            "# FPU register pressure\n"
            "flds %0\n"
            "flds %1\n"
            "faddp\n"
            "fstps %0\n"
            : "+m"(f0)
            : "m"(f1)
            : "st", "st(1)", "memory"
        );
        
        sum += (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4 +
               (int)d0 + (int)d1 + (int)d2 + vi0 + vi1 + vi2;
    }
    
    return sum;
}

int main(void) {
    int total = 0;
    
    /* Call test functions multiple times with different seeds */
    for (int i = 0; i < 10; i++) {
        total += test_mcf(i * 12345);
        total += test_mcf2(i * 54321);
        
        /* Prevent loop optimization */
        asm volatile("" : "+r"(i));
    }
    
    printf("Result: %d\n", total);
    return 0;
}
