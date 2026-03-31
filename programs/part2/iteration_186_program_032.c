/* mcf_test.c - Test program to trigger GCC's Minimum Cost Flow pass */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with high register pressure and control flow */
static int __attribute__((noinline)) 
complex_mcf_function(int seed, int iterations) 
{
    /* Declare many local variables to create register pressure */
    volatile int v0 = seed, v1 = seed + 1, v2 = seed + 2, v3 = seed + 3;
    volatile int v4 = seed + 4, v5 = seed + 5, v6 = seed + 6, v7 = seed + 7;
    volatile int v8 = seed + 8, v9 = seed + 9, v10 = seed + 10, v11 = seed + 11;
    volatile int v12 = seed + 12, v13 = seed + 13, v14 = seed + 14, v15 = seed + 15;
    volatile int v16 = seed + 16, v17 = seed + 17, v18 = seed + 18, v19 = seed + 19;
    volatile int v20 = seed + 20, v21 = seed + 21, v22 = seed + 22, v23 = seed + 23;
    volatile int v24 = seed + 24, v25 = seed + 25, v26 = seed + 26, v27 = seed + 27;
    volatile int v28 = seed + 28, v29 = seed + 29;
    
    /* Mix different types to increase pressure across register classes */
    volatile float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f;
    volatile void* p0 = &v0, *p1 = &v1, *p2 = &v2;
    
    int result = 0;
    
    /* Complex loop with many iterations and data dependencies */
    for (int i = 0; i < iterations; i++) {
        /* Chain of dependent arithmetic operations */
        v0 = v1 + v2;
        v1 = v3 * v4 - v5;
        v2 = v6 / (v7 + 1) + v8;
        v3 = v9 ^ v10 | v11;
        v4 = v12 & v13 << v14;
        v5 = v15 - v16 * v17;
        v6 = v18 + v19 - v20;
        v7 = v21 * v22 / (v23 + 1);
        v8 = v24 | v25 & v26;
        v9 = v27 ^ v28 + v29;
        
        /* Complex conditional chain creating many basic blocks */
        if (v0 & 0x1) {
            v10 = v1 * 2;
            if (v1 > 100) {
                v11 = v2 / 3;
                /* Inline assembly with register clobbers to force graph modifications */
                asm volatile (
                    "# Force register pressure\n"
                    : "=r"(v12), "=r"(v13)
                    : "0"(v10), "1"(v11)
                    : "eax", "ebx", "ecx", "edx", "memory"
                );
            } else {
                v11 = v2 * 3;
                goto label_a;  /* Create control flow complexity */
            }
        } else if (v0 & 0x2) {
            v10 = v1 / 2;
            if (v1 < 50) {
                v11 = v2 + 100;
            } else {
                v11 = v2 - 100;
            }
        } else {
            v10 = v1 + v2;
            v11 = v2 - v1;
        }
        
        /* Another level of conditionals */
        if (v3 > v4) {
            v12 = v5 * v6;
            /* More inline assembly with different clobbers */
            asm volatile (
                "# More register pressure\n"
                : "+r"(v12), "+r"(v13)
                :
                : "esi", "edi", "ebp", "memory"
            );
        } else {
            v12 = v5 / (v6 + 1);
        }
        
        /* Switch statement with many cases creating multiple basic blocks */
        switch (v7 & 0xF) {
            case 0: v13 = v8 + 1; break;
            case 1: v13 = v8 - 1; break;
            case 2: v13 = v8 * 2; break;
            case 3: v13 = v8 / 2; break;
            case 4: v13 = v8 ^ 0xFF; break;
            case 5: v13 = v8 | 0xAA; break;
            case 6: v13 = v8 & 0x55; break;
            case 7: v13 = v8 << 2; break;
            case 8: v13 = v8 >> 2; break;
            case 9: v13 = ~v8; break;
            case 10: v13 = v8 + v9; break;
            case 11: v13 = v8 - v9; break;
            case 12: v13 = v8 * v9; break;
            case 13: v13 = v8 ^ v9; break;
            case 14: v13 = v8 | v9; break;
            case 15: v13 = v8 & v9; break;
            default: v13 = 0; break;
        }
        
        /* Nested loops with breaks/continues */
        for (int j = 0; j < 3; j++) {
            if (v13 > 1000) {
                v14 += j;
                continue;
            }
            for (int k = 0; k < 2; k++) {
                v15 += k;
                if (v15 > 500) {
                    break;
                }
            }
            if (j == 1) {
                goto label_b;
            }
        }
        
        /* Float operations to use FP registers */
        f0 = f1 * 1.1f + f2;
        f1 = f0 / 2.0f - f2;
        f2 = f0 + f1 * 0.5f;
        
        /* Pointer arithmetic */
        p0 = (char*)p1 + v16;
        p1 = (char*)p2 - v17;
        p2 = p0;
        
        /* More complex conditionals */
        if (v18++ > v19--) {
            v20 = v21 * v22;
            if (v20 < 0) {
                v21 = -v20;
                goto label_c;
            }
        }
        
        /* Continue the computation chain */
        v22 = v23 + v24 - v25;
        v23 = v26 * v27 / (v28 + 1);
        v24 = v29 | v0 & v1;
        v25 = v2 ^ v3 + v4;
        v26 = v5 - v6 * v7;
        v27 = v8 + v9 - v10;
        v28 = v11 * v12 / (v13 + 1);
        v29 = v14 | v15 & v16;
        
        /* Accumulate result with data dependency */
        result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                  v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                  v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
        
        /* Labels for goto statements creating complex control flow */
        label_a:
        v16 = v17 * 2;
        continue;
        
        label_b:
        v17 = v18 / 2;
        if (i % 2 == 0) {
            continue;
        }
        
        label_c:
        v18 = v19 + 1;
    }
    
    /* Final computation mixing all variables */
    result = (result + (int)f0 + (int)f1 + (int)f2) ^ 
             ((int)(long)p0 + (int)(long)p1 + (int)(long)p2);
    
    return result;
}

/* Wrapper function to prevent inlining and create more call context */
static int __attribute__((noinline))
test_mcf_wrapper(int base) 
{
    int sum = 0;
    
    /* Call the complex function multiple times with different parameters */
    sum += complex_mcf_function(base, 10);
    sum += complex_mcf_function(base + 1, 8);
    sum += complex_mcf_function(base + 2, 6);
    sum += complex_mcf_function(base + 3, 4);
    
    /* Additional complex computation in wrapper */
    volatile int x = base * 2;
    volatile int y = base / 2;
    
    for (int i = 0; i < 5; i++) {
        x = x * 3 + y;
        y = y / 2 + x;
        
        /* More inline assembly */
        asm volatile (
            "# Final register pressure\n"
            : "+r"(x), "+r"(y)
            :
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
        );
    }
    
    return sum + x + y;
}

int main(void) 
{
    int total = 0;
    
    printf("Starting MCF test...\n");
    
    /* Call the test multiple times to ensure MCF runs */
    for (int i = 0; i < 3; i++) {
        int result = test_mcf_wrapper(i * 100);
        total += result;
        printf("Iteration %d: result = %d\n", i, result);
    }
    
    printf("Total checksum: %d\n", total);
    
    /* Use the result to prevent dead code elimination */
    if (total == 0) {
        printf("Unexpected zero result\n");
    }
    
    return 0;
}
