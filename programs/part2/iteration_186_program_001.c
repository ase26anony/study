/* mcf_test.c - Test program to trigger GCC's Minimum Cost Flow debug output */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with high register pressure and control flow */
static int __attribute__((noinline)) complex_mcf_function(int seed) {
    /* Declare many local variables to create register pressure */
    volatile int v0 = seed, v1 = seed + 1, v2 = seed + 2, v3 = seed + 3;
    volatile int v4 = seed + 4, v5 = seed + 5, v6 = seed + 6, v7 = seed + 7;
    volatile int v8 = seed + 8, v9 = seed + 9, v10 = seed + 10, v11 = seed + 11;
    volatile int v12 = seed + 12, v13 = seed + 13, v14 = seed + 14, v15 = seed + 15;
    volatile int v16 = seed + 16, v17 = seed + 17, v18 = seed + 18, v19 = seed + 19;
    volatile int v20 = seed + 20, v21 = seed + 21, v22 = seed + 22, v23 = seed + 23;
    volatile int v24 = seed + 24, v25 = seed + 25, v26 = seed + 26, v27 = seed + 27;
    volatile int v28 = seed + 28, v29 = seed + 29;
    
    /* Mixed types to increase pressure on different register classes */
    volatile float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f;
    volatile double d0 = seed * 0.01, d1 = seed * 0.02;
    volatile void* p0 = &v0, *p1 = &v1, *p2 = &v2;
    
    int result = 0;
    int i, j;
    
    /* Complex loop with data dependencies across variables */
    for (i = 0; i < 100; i++) {
        /* Chain of dependent arithmetic operations */
        v0 = v1 + v2;
        v1 = v3 * v4;
        v2 = v5 - v6;
        v3 = v7 / (v8 + 1);
        v4 = v9 ^ v10;
        v5 = v11 | v12;
        v6 = v13 & v14;
        v7 = v15 << 2;
        v8 = v16 >> 1;
        v9 = v17 + v18;
        v10 = v19 * v20;
        
        /* Floating point operations mixed in */
        f0 = f1 * 1.1f;
        f1 = f2 + 0.5f;
        f2 = f0 - 0.2f;
        d0 = d1 * 1.01;
        d1 = d0 / 2.0;
        
        /* Pointer arithmetic */
        p0 = (char*)p1 + 1;
        p1 = (char*)p2 - 1;
        p2 = p0;
        
        /* Complex conditional structure creating many basic blocks */
        if (i % 3 == 0) {
            v11 = v21 + v22;
            v12 = v23 - v24;
            
            /* Inline assembly with register clobbering */
            asm volatile (
                "# Force register pressure\n\t"
                : 
                : "r"(v11), "r"(v12)
                : "eax", "ebx", "ecx", "edx", "memory"
            );
        } else if (i % 3 == 1) {
            v13 = v25 * v26;
            v14 = v27 / (v28 + 1);
            
            /* Another inline assembly with different clobbers */
            asm volatile (
                "# More register pressure\n\t"
                : 
                : "r"(v13), "r"(v14)
                : "esi", "edi", "ebp", "memory"
            );
        } else {
            v15 = v29 ^ v0;
            v16 = v1 | v2;
            
            /* Force spill/fill code */
            asm volatile (
                "# Complex constraint\n\t"
                : "+r"(v15), "+r"(v16)
                :
                : "cc", "memory"
            );
        }
        
        /* Nested switch with many cases */
        switch (i % 7) {
            case 0:
                v17 = v3 + v4;
                v18 = v5 - v6;
                break;
            case 1:
                v19 = v7 * v8;
                v20 = v9 / (v10 + 1);
                break;
            case 2:
                v21 = v11 ^ v12;
                v22 = v13 | v14;
                break;
            case 3:
                v23 = v15 & v16;
                v24 = v17 << 1;
                break;
            case 4:
                v25 = v18 >> 2;
                v26 = v19 + v20;
                break;
            case 5:
                v27 = v21 * v22;
                v28 = v23 - v24;
                break;
            case 6:
                v29 = v25 ^ v26;
                v0 = v27 | v28;
                /* Fall through to create merge point */
            default:
                v1 = v29 + v0;
                break;
        }
        
        /* Inner loop with break/continue creating more control flow */
        for (j = 0; j < 5; j++) {
            if (j == 2) {
                v2 = v1 * 3;
                continue;
            }
            if (j == 4) {
                v3 = v2 / 2;
                break;
            }
            v4 = v3 + j;
        }
        
        /* Goto labels to create additional basic blocks */
        if (i % 11 == 0) {
            goto special_case;
        }
        
        v5 = v4 * 2;
        goto after_special;
        
    special_case:
        v5 = v4 / 2;
        
    after_special:
        /* Compute running result with all variables */
        result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        result += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
        result += v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
        result += (int)f0 + (int)f1 + (int)f2 + (int)d0 + (int)d1;
        result += (int)((long)p0 % 256) + (int)((long)p1 % 256) + (int)((long)p2 % 256);
        
        /* Prevent loop unrolling */
        asm volatile ("# barrier" ::: "memory");
    }
    
    return result;
}

/* Another complex function to create interprocedural effects */
static int __attribute__((noinline)) helper_function(int x) {
    volatile int a = x, b = x * 2, c = x * 3, d = x * 4;
    volatile int e = x * 5, f = x * 6, g = x * 7, h = x * 8;
    
    /* Complex conditional chain */
    if (x % 2 == 0) {
        a = b + c;
        if (x % 4 == 0) {
            d = e * f;
            if (x % 8 == 0) {
                g = h / a;
            } else {
                g = h % a;
            }
        } else {
            d = e - f;
        }
    } else {
        a = b - c;
        if (x % 3 == 0) {
            d = e / f;
        } else {
            d = e | f;
        }
    }
    
    /* More inline assembly */
    asm volatile (
        "# Helper clobber\n\t"
        : "+r"(a), "+r"(d)
        :
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    return a + b + c + d + e + f + g + h;
}

int main(void) {
    int total = 0;
    int i;
    
    printf("Starting MCF test...\n");
    
    /* Call complex function multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        int result = complex_mcf_function(i * 100);
        total += result;
        
        /* Also call helper to create more compilation context */
        total += helper_function(i * 50);
        
        printf("Iteration %d: partial total = %d\n", i, total);
    }
    
    printf("Final checksum: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}
