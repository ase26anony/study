/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow pass debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all computations */
#define NO_OPTIMIZE(x) asm volatile("" : "+r" (x))

/* Complex test function with high register pressure */
static int __attribute__((noinline)) test_mcf_pass(int seed) {
    /* Declare many local variables to create register pressure */
    volatile int v0 = seed + 1;
    int v1 = seed * 2;
    int v2 = seed / 3;
    int v3 = seed % 7;
    int v4 = seed ^ 0x55;
    int v5 = seed | 0xAA;
    int v6 = seed & 0xF0;
    int v7 = seed << 2;
    int v8 = seed >> 1;
    int v9 = ~seed;
    int v10 = seed + 11;
    int v11 = seed * 3;
    int v12 = seed / 5;
    int v13 = seed % 11;
    int v14 = seed ^ 0x33;
    int v15 = seed | 0xCC;
    int v16 = seed & 0x0F;
    int v17 = seed << 3;
    int v18 = seed >> 2;
    int v19 = seed + 19;
    int v20 = seed * 5;
    int v21 = seed / 7;
    int v22 = seed % 13;
    int v23 = seed ^ 0x99;
    int v24 = seed | 0x66;
    int v25 = seed & 0xF5;
    int v26 = seed << 1;
    int v27 = seed >> 3;
    int v28 = seed + 23;
    int v29 = seed * 7;
    
    /* Complex control flow with many basic blocks */
    int result = 0;
    
    /* First level of conditionals */
    if (v0 > 100) {
        v1 += v2;
        /* Inline assembly to clobber registers and force graph modifications */
        asm volatile(
            "# Force register pressure\n\t"
            : 
            : "r" (v0), "r" (v1), "r" (v2)
            : "eax", "ebx", "ecx", "edx", "esi", "edi"
        );
        
        if (v1 < 50) {
            v3 = v4 * v5;
            result += v3;
        } else {
            v6 = v7 / (v8 + 1);
            result += v6;
        }
    } else {
        v9 = v10 ^ v11;
        result += v9;
    }
    
    /* Switch statement with many cases - creates multiple basic blocks */
    switch (seed % 10) {
        case 0:
            v12 = v13 + v14;
            result += v12;
            /* More inline assembly with clobbers */
            asm volatile(
                "# Another clobber point\n\t"
                : 
                : "r" (v12), "r" (v13)
                : "eax", "ebx", "memory"
            );
            break;
        case 1:
            v15 = v16 | v17;
            result += v15;
            break;
        case 2:
            v18 = v19 & v20;
            result += v18;
            break;
        case 3:
            v21 = v22 ^ v23;
            result += v21;
            break;
        case 4:
            v24 = v25 * v26;
            result += v24;
            break;
        case 5:
            v27 = v28 / (v29 + 1);
            result += v27;
            break;
        case 6:
            v0 = v1 + v2 + v3;
            result += v0;
            break;
        case 7:
            v4 = v5 * v6 * v7;
            result += v4;
            break;
        case 8:
            v8 = v9 | v10 | v11;
            result += v8;
            break;
        case 9:
            v12 = v13 & v14 & v15;
            result += v12;
            break;
        default:
            result += 999;
    }
    
    /* Complex loop with nested conditionals */
    for (int i = 0; i < 5; i++) {
        /* Create data dependencies across loop iterations */
        v0 = v1 + i;
        v1 = v2 * (i + 1);
        v2 = v3 - i;
        v3 = v4 / (i + 2);
        v4 = v5 | i;
        v5 = v6 & (i + 3);
        
        if (i % 2 == 0) {
            v6 = v7 << (i % 4);
            result += v6;
            
            /* Conditional goto to create more complex CFG */
            if (result > 1000) {
                goto early_exit;
            }
        } else {
            v7 = v8 >> (i % 3);
            result += v7;
        }
        
        /* Nested loop */
        for (int j = 0; j < 3; j++) {
            v8 = v9 + j;
            v9 = v10 * (j + 1);
            v10 = v11 - j;
            
            if (j == 1) {
                /* Another inline assembly with different clobbers */
                asm volatile(
                    "# Nested loop clobber\n\t"
                    : 
                    : "r" (v8), "r" (v9), "r" (v10)
                    : "ecx", "edx", "memory"
                );
                continue;  /* Creates additional control flow edges */
            }
            
            result += v8 + v9 + v10;
        }
    }
    
early_exit:
    
    /* More arithmetic to ensure all variables are used */
    v11 = v12 + v13 + v14 + v15;
    v16 = v17 * v18 * v19;
    v20 = v21 | v22 | v23;
    v24 = v25 & v26 & v27;
    v28 = v29 ^ seed ^ result;
    
    /* Final computation using all variables */
    result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    result += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    result += v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    
    /* Prevent dead code elimination */
    NO_OPTIMIZE(result);
    
    return result;
}

/* Second complex function to increase interprocedural pressure */
static int __attribute__((noinline)) another_complex_function(int x) {
    volatile int a = x * 3;
    int b = x + 7;
    int c = x / 2;
    int d = x % 9;
    int e = x ^ 0xFF;
    int f = x | 0x11;
    int g = x & 0xEE;
    
    /* Complex if-else chain */
    if (a > b) {
        c = d + e;
        if (c < f) {
            g = a * b;
        } else if (c > g) {
            g = d / (e + 1);
        } else {
            g = f | e;
        }
    } else if (a < c) {
        d = e ^ f;
        if (d > g) {
            a = b << 2;
        }
    } else {
        f = g & a;
    }
    
    /* Loop with break/continue */
    for (int i = 0; i < 10; i++) {
        if (i == 5) break;
        if (i % 2 == 0) continue;
        
        a += i;
        b *= (i + 1);
        c -= i;
        
        /* Inline assembly */
        asm volatile(
            "# Function 2 clobber\n\t"
            : 
            : "r" (a), "r" (b)
            : "eax", "ebx", "ecx"
        );
    }
    
    return a + b + c + d + e + f + g;
}

int main(void) {
    int total = 0;
    
    /* Call test functions multiple times with different inputs */
    for (int i = 0; i < 100; i++) {
        total += test_mcf_pass(i);
        total += another_complex_function(i);
        
        /* Prevent loop unrolling */
        if (i % 7 == 0) {
            asm volatile("# Loop barrier" : : : "memory");
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
