/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all variables alive */
#define USE(x) asm volatile("" : : "r"(x))

/* Complex function with high register pressure and control flow */
static int __attribute__((noinline)) test_function(int seed) {
    /* Declare many local variables to create register pressure */
    volatile int v0 = seed;
    int v1 = v0 * 2;
    int v2 = v1 + 1;
    int v3 = v2 ^ v0;
    int v4 = v3 - v1;
    int v5 = v4 * 3;
    int v6 = v5 / 2;
    int v7 = v6 | v3;
    int v8 = v7 & v4;
    int v9 = v8 << 2;
    int v10 = v9 >> 1;
    int v11 = v10 + v5;
    int v12 = v11 * v6;
    int v13 = v12 % 7;
    int v14 = v13 ^ v8;
    int v15 = v14 + v9;
    int v16 = v15 * 2;
    int v17 = v16 - v11;
    int v18 = v17 | v12;
    int v19 = v18 & v13;
    int v20 = v19 << 1;
    int v21 = v20 >> 2;
    int v22 = v21 + v14;
    int v23 = v22 * v15;
    int v24 = v23 % 11;
    int v25 = v24 ^ v18;
    int v26 = v25 + v19;
    int v27 = v26 * 3;
    int v28 = v27 - v22;
    int v29 = v28 | v23;
    int v30 = v29 & v24;
    
    /* Complex control flow with many basic blocks */
    int result = 0;
    
    /* Multiple nested conditionals */
    if (v0 > 0) {
        if (v1 < 100) {
            result += v2;
            if (v3 % 2 == 0) {
                result += v4;
                goto label1;
            } else {
                result -= v5;
                goto label2;
            }
        } else {
            result += v6;
        }
    } else {
        if (v7 != 0) {
            result += v8;
        }
    }
    
    /* Large switch statement creates many basic blocks */
    switch (v9 % 10) {
        case 0: result += v10; break;
        case 1: result += v11; break;
        case 2: result += v12; break;
        case 3: result += v13; break;
        case 4: result += v14; break;
        case 5: result += v15; break;
        case 6: result += v16; break;
        case 7: result += v17; break;
        case 8: result += v18; break;
        case 9: result += v19; break;
        default: result += v20;
    }
    
    /* Loop with complex exit conditions */
    for (int i = 0; i < v21 % 8; i++) {
        result += v22 + i;
        if (result > 1000) {
            result -= v23;
            break;
        } else if (result < 0) {
            result += v24;
            continue;
        }
        result *= 2;
    }
    
    /* Inline assembly with register clobbers to force graph transformations */
    asm volatile (
        "# Force register pressure\n"
        "mov %0, %%eax\n"
        "add %1, %%eax\n"
        "mov %%eax, %0\n"
        : "+r" (result)
        : "r" (v25)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    /* More complex control flow with goto creating multiple entry/exit regions */
    if (v26 > v27) {
        goto label3;
    }
    
    result += v28;
    goto label4;
    
label1:
    result += v29;
    if (v30 > 0) {
        goto label4;
    }
    return result;
    
label2:
    result -= v0;
    goto label1;
    
label3:
    result *= v1;
    /* Another inline asm with different clobbers */
    asm volatile (
        "# More register pressure\n"
        "imul %1, %0\n"
        : "+r" (result)
        : "r" (v2)
        : "cc", "memory"
    );
    
label4:
    /* Final computation using all variables */
    result = result + v0 - v1 + v2 - v3 + v4 - v5 + v6 - v7 + v8 - v9 +
             v10 - v11 + v12 - v13 + v14 - v15 + v16 - v17 + v18 - v19 +
             v20 - v21 + v22 - v23 + v24 - v25 + v26 - v27 + v28 - v29 + v30;
    
    return result;
}

/* Second complex function to create interprocedural effects */
static int __attribute__((noinline)) another_function(int x, int y) {
    volatile int a = x;
    volatile int b = y;
    int c = a * b;
    int d = c + a;
    int e = d - b;
    int f = e * 2;
    int g = f / 3;
    int h = g | a;
    int i = h & b;
    
    /* Complex switch with many cases */
    switch (c % 12) {
        case 0: return a + i;
        case 1: return b + h;
        case 2: return c + g;
        case 3: return d + f;
        case 4: return e + e;
        case 5: return f + d;
        case 6: return g + c;
        case 7: return h + b;
        case 8: return i + a;
        case 9: return a * i;
        case 10: return b * h;
        case 11: return c * g;
        default: return 0;
    }
}

int main(void) {
    int total = 0;
    
    /* Call functions multiple times with different inputs */
    for (int i = 0; i < 100; i++) {
        total += test_function(i);
        total += another_function(i, i * 2);
        
        /* Prevent loop unrolling */
        if (i % 7 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
