/* mcf_coverage.c - Program to trigger GCC's MCF pass debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all computations */
#define NO_OPTIMIZE(x) asm volatile("" : "+r" (x))

/* Complex test function with maximum register pressure */
static int __attribute__((noinline)) 
test_mcf_pressure(int seed, int iterations) {
    /* Declare many local variables to create register pressure */
    volatile int v0 = seed, v1 = seed + 1, v2 = seed + 2, v3 = seed + 3;
    volatile int v4 = seed + 4, v5 = seed + 5, v6 = seed + 6, v7 = seed + 7;
    volatile int v8 = seed + 8, v9 = seed + 9, v10 = seed + 10, v11 = seed + 11;
    volatile int v12 = seed + 12, v13 = seed + 13, v14 = seed + 14, v15 = seed + 15;
    volatile int v16 = seed + 16, v17 = seed + 17, v18 = seed + 18, v19 = seed + 19;
    volatile int v20 = seed + 20, v21 = seed + 21, v22 = seed + 22, v23 = seed + 23;
    volatile int v24 = seed + 24, v25 = seed + 25, v26 = seed + 26, v27 = seed + 27;
    volatile int v28 = seed + 28, v29 = seed + 29;
    
    /* Mix different types to pressure different register classes */
    volatile float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f;
    volatile double d0 = seed * 0.01, d1 = seed * 0.02;
    volatile void* p0 = &v0, *p1 = &v1, *p2 = &v2;
    
    int result = 0;
    
    /* Complex loop with overlapping live ranges */
    for (int i = 0; i < iterations; i++) {
        /* Chain of dependent arithmetic operations */
        v0 = v1 + v2; NO_OPTIMIZE(v0);
        v1 = v0 ^ v3; NO_OPTIMIZE(v1);
        v2 = v1 * v4; NO_OPTIMIZE(v2);
        v3 = v2 - v5; NO_OPTIMIZE(v3);
        v4 = v3 | v6; NO_OPTIMIZE(v4);
        v5 = v4 & v7; NO_OPTIMIZE(v5);
        v6 = v5 << 2; NO_OPTIMIZE(v6);
        v7 = v6 >> 1; NO_OPTIMIZE(v7);
        v8 = v7 + v9; NO_OPTIMIZE(v8);
        v9 = v8 * v10; NO_OPTIMIZE(v9);
        
        /* Floating point operations intermixed */
        f0 = f1 * 1.1f; NO_OPTIMIZE(f0);
        f1 = f0 + f2; NO_OPTIMIZE(f1);
        f2 = f1 - 0.5f; NO_OPTIMIZE(f2);
        
        d0 = d1 * 1.01; NO_OPTIMIZE(d0);
        d1 = d0 + 0.001; NO_OPTIMIZE(d1);
        
        /* Pointer arithmetic */
        p0 = (char*)p1 + v0; NO_OPTIMIZE(p0);
        p1 = (char*)p2 - v1; NO_OPTIMIZE(p1);
        p2 = (char*)p0 + v2; NO_OPTIMIZE(p2);
        
        /* Complex conditional structure creating many basic blocks */
        switch (i % 12) {
            case 0:
                v10 = v9 + v11; v11 = v10 * 3; goto label_a;
            case 1:
                v12 = v11 - v13; v13 = v12 / 2; goto label_b;
            case 2:
                v14 = v13 ^ v15; v15 = v14 | 0xFF; goto label_c;
            case 3:
                v16 = v15 & v17; v17 = v16 << 3; goto label_d;
            case 4:
                v18 = v17 >> 2; v19 = v18 + 1; goto label_e;
            case 5:
                v20 = v19 * v21; v21 = v20 % 7; goto label_f;
            case 6:
                v22 = v21 | v23; v23 = v22 ^ 0xAA; goto label_g;
            case 7:
                v24 = v23 & v25; v25 = v24 + 100; goto label_h;
            case 8:
                v26 = v25 - v27; v27 = v26 * 2; goto label_i;
            case 9:
                v28 = v27 / 3; v29 = v28 + 50; goto label_j;
            case 10:
                v0 = v29 ^ v1; v1 = v0 | 0x55; goto label_k;
            case 11:
                v2 = v1 & v3; v3 = v2 << 1; goto label_l;
        }
        
        /* Multiple labels creating control flow merge points */
        label_a: v10 = v10 * 2; if (v10 > 1000) goto early_exit;
        label_b: v11 = v11 + v12; if (v11 < 0) v11 = -v11;
        label_c: v12 = v12 ^ v13; NO_OPTIMIZE(v12);
        label_d: v13 = v13 | v14; NO_OPTIMIZE(v13);
        label_e: v14 = v14 & v15; NO_OPTIMIZE(v14);
        label_f: v15 = v15 + v16; NO_OPTIMIZE(v15);
        label_g: v16 = v16 - v17; NO_OPTIMIZE(v16);
        label_h: v17 = v17 * v18; NO_OPTIMIZE(v17);
        label_i: v18 = v18 / (v19 + 1); NO_OPTIMIZE(v18);
        label_j: v19 = v19 % (v20 + 1); NO_OPTIMIZE(v19);
        label_k: v20 = v20 << (v21 & 3); NO_OPTIMIZE(v20);
        label_l: v21 = v21 >> (v22 & 3); NO_OPTIMIZE(v21);
        
        /* Inline assembly with register clobbers to force graph transformations */
        asm volatile (
            "# Force register pressure\n"
            "mov %0, %0\n"
            :
            : "r" (v0)
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
        
        /* Another asm with different clobbers */
        asm volatile (
            "# More pressure\n"
            "add %0, %1\n"
            : "+r" (v1)
            : "r" (v2)
            : "cc", "memory"
        );
        
        continue;
        
        early_exit:
        /* Early exit path creating additional control flow */
        v29 = v0 + v1 + v2;
        if (v29 > 10000) {
            result += v29;
            break;
        }
    }
    
    /* Final computation using all variables */
    result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    result += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    result += v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    result += (int)f0 + (int)f1 + (int)f2;
    result += (int)d0 + (int)d1;
    result += (int)(long)p0 + (int)(long)p1 + (int)(long)p2;
    
    return result;
}

/* Wrapper with even more complexity */
static int __attribute__((noinline))
complex_wrapper(int base) {
    int total = 0;
    
    /* Multiple calls with different parameters */
    total += test_mcf_pressure(base, 50);
    total += test_mcf_pressure(base + 1, 25);
    total += test_mcf_pressure(base + 2, 75);
    total += test_mcf_pressure(base + 3, 10);
    
    /* Additional complex control flow */
    for (int j = 0; j < 20; j++) {
        if (j % 3 == 0) {
            total += test_mcf_pressure(base + j, 5);
        } else if (j % 3 == 1) {
            total -= test_mcf_pressure(base - j, 3);
        } else {
            total ^= test_mcf_pressure(base ^ j, 7);
        }
        
        /* Nested switch */
        switch (j % 5) {
            case 0: total <<= 1; break;
            case 1: total >>= 1; break;
            case 2: total |= 0x1234; break;
            case 3: total &= 0xFFFF; break;
            case 4: total ^= 0xABCD; break;
        }
    }
    
    return total;
}

int main(void) {
    int final_result = 0;
    
    printf("Starting MCF coverage test...\n");
    
    /* Call multiple times to prevent interprocedural optimization */
    for (int run = 0; run < 10; run++) {
        int r = complex_wrapper(run * 100);
        final_result += r;
        printf("Run %d: result = %d\n", run, r);
    }
    
    printf("Final checksum: %d\n", final_result);
    printf("Compile with: gcc -O2 -fdump-rtl-mcf -fdump-rtl-mcf-details -fno-schedule-insns mcf_coverage.c -o mcf_test\n");
    printf("Also try: gcc -O3 -fdump-rtl-all -fno-peephole2 mcf_coverage.c -o mcf_test\n");
    
    return final_result != 0 ? 0 : 1;
}
