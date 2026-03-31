/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow pass debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all variables alive */
#define USE(x) asm volatile("" : : "r"(x))

/* Complex test function with high register pressure */
static int __attribute__((noinline)) complex_mcf_test(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    volatile int v0 = seed + 1;
    volatile int v1 = seed * 2;
    volatile int v2 = seed / 3;
    volatile int v3 = seed ^ 0x1234;
    volatile int v4 = seed | 0x5678;
    volatile int v5 = seed & 0x9ABC;
    volatile int v6 = ~seed;
    volatile int v7 = seed << 2;
    volatile int v8 = seed >> 1;
    volatile int v9 = seed % 17;
    volatile int v10 = seed + 11;
    volatile int v11 = seed * 3;
    volatile int v12 = seed / 5;
    volatile int v13 = seed ^ 0xDEAD;
    volatile int v14 = seed | 0xBEEF;
    volatile int v15 = seed & 0xCAFE;
    volatile int v16 = ~seed + 1;
    volatile int v17 = seed << 3;
    volatile int v18 = seed >> 2;
    volatile int v19 = seed % 23;
    volatile int v20 = seed + 19;
    volatile int v21 = seed * 5;
    volatile int v22 = seed / 7;
    volatile int v23 = seed ^ 0xF00D;
    volatile int v24 = seed | 0xBAAD;
    volatile int v25 = seed & 0xFACE;
    volatile int v26 = ~seed * 2;
    volatile int v27 = seed << 1;
    volatile int v28 = seed >> 3;
    volatile int v29 = seed % 29;
    
    /* Additional variables with different types to increase pressure */
    volatile float f0 = seed * 0.1f;
    volatile float f1 = seed * 0.2f;
    volatile float f2 = seed * 0.3f;
    volatile void* p0 = &v0;
    volatile void* p1 = &v1;
    volatile void* p2 = &v2;
    
    int result = 0;
    
    /* Complex loop with many basic blocks */
    for (int i = 0; i < iterations; i++) {
        /* Create data dependencies between variables */
        v0 = v1 + v2;
        v1 = v3 - v4;
        v2 = v5 * v6;
        v3 = v7 / (v8 + 1);
        v4 = v9 ^ v10;
        v5 = v11 | v12;
        v6 = v13 & v14;
        v7 = ~v15;
        v8 = v16 << (i & 3);
        v9 = v17 >> (i & 1);
        
        /* Inline assembly with register clobbers to force graph transformations */
        asm volatile (
            "# Force register pressure\n"
            : "=r"(v10), "=r"(v11), "=r"(v12)
            : "0"(v0), "1"(v1), "2"(v2)
            : "eax", "ebx", "ecx", "edx", "memory"
        );
        
        /* Complex conditional chain creating many basic blocks */
        switch (i % 13) {
            case 0:
                v13 = v10 + v11;
                v14 = v12 * v13;
                /* Force spill/reload */
                asm volatile("# case 0" : : : "esi", "edi");
                break;
            case 1:
                v15 = v11 - v12;
                v16 = v13 / (v14 + 1);
                break;
            case 2:
                v17 = v12 ^ v13;
                v18 = v14 | v15;
                /* More register pressure */
                asm volatile("# case 2" : : : "r8", "r9", "r10");
                break;
            case 3:
                v19 = v13 & v14;
                v20 = ~v15;
                break;
            case 4:
                v21 = v14 << 2;
                v22 = v15 >> 1;
                break;
            case 5:
                v23 = v16 + v17;
                v24 = v18 * v19;
                break;
            case 6:
                v25 = v17 - v18;
                v26 = v19 / (v20 + 1);
                /* Force additional clobbers */
                asm volatile("# case 6" : : : "xmm0", "xmm1", "xmm2");
                break;
            case 7:
                v27 = v18 ^ v19;
                v28 = v20 | v21;
                break;
            case 8:
                v29 = v19 & v20;
                v0 = ~v21;
                break;
            case 9:
                v1 = v20 << 3;
                v2 = v21 >> 2;
                break;
            case 10:
                v3 = v21 + v22;
                v4 = v23 * v24;
                /* Mix float operations */
                f0 = f1 * f2;
                f1 = f0 + (float)v3;
                break;
            case 11:
                v5 = v22 - v23;
                v6 = v24 / (v25 + 1);
                break;
            case 12:
                v7 = v23 ^ v24;
                v8 = v25 | v26;
                /* Pointer arithmetic */
                p0 = (char*)p1 + v7;
                p1 = (char*)p2 - v8;
                break;
        }
        
        /* Nested conditionals creating more control flow edges */
        if (i & 1) {
            if (v0 > v1) {
                v9 = v2 + v3;
                /* goto creating irregular control flow */
                if (v9 > 1000) goto special_label;
            } else {
                v10 = v4 - v5;
            }
            
            if (v6 < v7) {
                v11 = v8 * v9;
            } else if (v10 == v11) {
                v12 = v13 / 2;
            } else {
                v14 = v15 ^ v16;
            }
        } else {
            switch (i % 7) {
                case 0: v17 = v18 + 1; break;
                case 1: v18 = v19 - 1; break;
                case 2: v19 = v20 * 2; break;
                case 3: v20 = v21 / 2; break;
                case 4: v21 = v22 ^ 0xFF; break;
                case 5: v22 = v23 | 0xAA; break;
                case 6: v23 = v24 & 0x55; break;
            }
        }
        
        continue; /* Skip special_label for most iterations */
        
    special_label:
        /* Special basic block reached via goto */
        v24 = v25 << 1;
        v25 = v26 >> 1;
        /* Force graph fixup by creating value used in multiple paths */
        volatile int temp = v24 + v25;
        v26 = temp;
        v27 = temp * 2;
        
        /* Artificial use of all variables to keep them alive */
        USE(v0); USE(v1); USE(v2); USE(v3); USE(v4);
        USE(v5); USE(v6); USE(v7); USE(v8); USE(v9);
        USE(v10); USE(v11); USE(v12); USE(v13); USE(v14);
        USE(v15); USE(v16); USE(v17); USE(v18); USE(v19);
        USE(v20); USE(v21); USE(v22); USE(v23); USE(v24);
        USE(v25); USE(v26); USE(v27); USE(v28); USE(v29);
        USE(f0); USE(f1); USE(f2);
        USE(p0); USE(p1); USE(p2);
    }
    
    /* Combine all variables into final result */
    result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
             v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
             v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 +
             (int)f0 + (int)f1 + (int)f2 +
             (int)(long)p0 + (int)(long)p1 + (int)(long)p2;
    
    return result;
}

/* Wrapper to prevent inlining and create more call context */
static int __attribute__((noinline)) test_wrapper(int base) {
    int sum = 0;
    
    /* Call test function with different parameters */
    sum += complex_mcf_test(50, base);
    sum += complex_mcf_test(25, base * 2);
    sum += complex_mcf_test(75, base * 3);
    sum += complex_mcf_test(10, base * 4);
    
    return sum;
}

int main(void) {
    int total = 0;
    
    printf("Starting MCF coverage test...\n");
    
    /* Multiple calls with different seeds */
    for (int i = 0; i < 5; i++) {
        int result = test_wrapper(100 + i * 50);
        total += result;
        printf("Iteration %d: result = %d\n", i, result);
    }
    
    printf("Total checksum: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total == 0) {
        printf("Unexpected zero result\n");
    }
    
    return 0;
}
