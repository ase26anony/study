/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow debug output */
#include <stdio.h>
#include <stdlib.h>

/* Force register pressure by using many variables with overlapping live ranges */
#define DECLARE_VARS \
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9; \
    volatile int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19; \
    volatile int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29; \
    volatile float f0, f1, f2, f3, f4; \
    volatile double d0, d1, d2; \
    volatile void *p0, *p1, *p2, *p3; \
    volatile long l0, l1, l2, l3, l4;

/* Complex function to create many basic blocks and register pressure */
static int __attribute__((noinline)) 
complex_mcf_function(int iterations, int seed) {
    DECLARE_VARS;
    int result = seed;
    
    /* Initialize all variables with different values */
    v0 = seed; v1 = seed + 1; v2 = seed + 2; v3 = seed + 3; v4 = seed + 4;
    v5 = seed + 5; v6 = seed + 6; v7 = seed + 7; v8 = seed + 8; v9 = seed + 9;
    v10 = seed + 10; v11 = seed + 11; v12 = seed + 12; v13 = seed + 13; v14 = seed + 14;
    v15 = seed + 15; v16 = seed + 16; v17 = seed + 17; v18 = seed + 18; v19 = seed + 19;
    v20 = seed + 20; v21 = seed + 21; v22 = seed + 22; v23 = seed + 23; v24 = seed + 24;
    v25 = seed + 25; v26 = seed + 26; v27 = seed + 27; v28 = seed + 28; v29 = seed + 29;
    
    f0 = seed * 0.1f; f1 = seed * 0.2f; f2 = seed * 0.3f; f3 = seed * 0.4f; f4 = seed * 0.5f;
    d0 = seed * 0.01; d1 = seed * 0.02; d2 = seed * 0.03;
    p0 = &v0; p1 = &v1; p2 = &v2; p3 = &v3;
    l0 = seed * 10L; l1 = seed * 20L; l2 = seed * 30L; l3 = seed * 40L; l4 = seed * 50L;
    
    /* Complex loop with many basic blocks */
    for (int i = 0; i < iterations; i++) {
        /* Create data dependencies across variables */
        v0 = v1 + v2;
        v1 = v3 - v4;
        v2 = v5 * v6;
        v3 = v7 / (v8 + 1);
        v4 = v9 ^ v10;
        
        /* Force register pressure with inline assembly clobbers */
        /* Clobber multiple registers to force graph transformations */
        asm volatile (
            "# Force register clobbering\n"
            : 
            : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4)
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
        
        /* Complex conditional chain creating many basic blocks */
        if (i % 3 == 0) {
            v5 = v11 + v12;
            v6 = v13 - v14;
            
            /* More inline assembly with different clobbers */
            asm volatile (
                "# Another clobber point\n"
                : 
                : "r"(v5), "r"(v6)
                : "r8", "r9", "r10", "r11", "memory"
            );
            
            if (i % 5 == 0) {
                v7 = v15 * v16;
                v8 = v17 | v18;
                goto label_a;  /* Create unusual control flow */
            } else {
                v9 = v19 & v20;
                v10 = v21 ^ v22;
            }
        } else if (i % 3 == 1) {
            v11 = v23 + v24;
            v12 = v25 - v26;
            
            /* Switch statement creating many basic blocks */
            switch (i % 7) {
                case 0: v13 = v0 + v1; break;
                case 1: v13 = v2 - v3; break;
                case 2: v13 = v4 * v5; break;
                case 3: v13 = v6 / (v7 + 1); break;
                case 4: v13 = v8 ^ v9; break;
                case 5: v13 = v10 | v11; break;
                case 6: v13 = v12 & v13; break;
                default: v13 = 0; break;
            }
        } else {
            v14 = v27 + v28;
            v15 = v29 - v0;
            
            /* Nested loop with break/continue creating more blocks */
            for (int j = 0; j < 3; j++) {
                if (j == 1) continue;
                v16 = v1 + j;
                if (j == 2) break;
                v17 = v2 - j;
            }
        }
        
        /* Merge point with more computations */
        v18 = v3 + v4;
        v19 = v5 - v6;
        
        /* Use floating point to pressure different register classes */
        f0 = f1 * f2;
        f1 = f3 + f4;
        f2 = f0 - f1;
        
        /* Pointer arithmetic */
        p0 = (void*)((char*)p1 + v7);
        p1 = (void*)((char*)p2 - v8);
        
        /* Long computations */
        l0 = l1 * l2;
        l1 = l3 + l4;
        l2 = l0 - l1;
        
        /* Another inline assembly with memory clobber */
        asm volatile (
            "# Memory clobber to force spills\n"
            : 
            : "r"(v18), "r"(v19), "r"(f0), "r"(f1), "m"(v0)
            : "memory"
        );
        
        label_a:
        /* More dependent computations */
        v20 = v21 + v22;
        v21 = v23 - v24;
        v22 = v25 * v26;
        v23 = v27 / (v28 + 1);
        
        /* Update result with all computations */
        result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        result += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
        result += v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
        result += (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4;
        result += (int)d0 + (int)d1 + (int)d2;
        result += (int)(long)p0 + (int)(long)p1 + (int)(long)p2 + (int)(long)p3;
        result += (int)l0 + (int)l1 + (int)l2 + (int)l3 + (int)l4;
        
        /* Another conditional creating exit-like regions */
        if (result > 1000000) {
            result /= 2;
            goto early_exit;  /* Create early exit path */
        }
    }
    
    return result;
    
early_exit:
    /* Cleanup computations before early return */
    v29 = v0 + v1 + v2;
    return result + v29;
}

/* Wrapper function to prevent inlining and create more context */
static int __attribute__((noinline))
mcf_test_driver(int base) {
    int total = 0;
    
    /* Call complex function multiple times with different parameters */
    for (int i = 0; i < 10; i++) {
        total += complex_mcf_function(5 + (i % 3), base + i * 7);
        
        /* Additional computations between calls */
        if (i % 2 == 0) {
            volatile int temp = total * 3;
            total = temp / 2;
        }
    }
    
    return total;
}

int main(void) {
    int final_result = 0;
    
    printf("Starting MCF coverage test...\n");
    
    /* Run test multiple times to ensure MCF pass runs */
    for (int run = 0; run < 3; run++) {
        int run_result = mcf_test_driver(1000 + run * 100);
        final_result += run_result;
        printf("Run %d: result = %d\n", run, run_result);
    }
    
    printf("Final checksum: %d\n", final_result);
    printf("Test completed. Check GCC RTL dumps for MCF debug output.\n");
    
    return final_result != 0 ? 0 : 1;
}
