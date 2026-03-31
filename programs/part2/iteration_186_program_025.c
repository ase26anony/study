/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow pass debugging output */
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
    volatile int v10 = v3 ^ v4;
    volatile int v11 = v5 | v6;
    volatile int v12 = v7 & v8;
    volatile int v13 = v9 << 1;
    volatile int v14 = v10 >> 2;
    volatile int v15 = v11 + v12;
    volatile int v16 = v13 * v14;
    volatile int v17 = v15 ^ v16;
    volatile int v18 = v17 | seed;
    volatile int v19 = v18 & 0xFF;
    volatile int v20 = v19 << 3;
    volatile int v21 = v20 >> 1;
    volatile int v22 = v21 + 0x100;
    volatile int v23 = v22 * 3;
    volatile int v24 = v23 ^ 0x5555;
    volatile int v25 = v24 | 0xAAAA;
    volatile int v26 = v25 & 0x3333;
    volatile int v27 = v26 << 4;
    volatile int v28 = v27 >> 2;
    volatile int v29 = v28 + seed;
    
    /* Force register clobbering with inline assembly */
    asm volatile(
        "# Force register clobbering\n"
        "movl %%eax, %%ebx\n"
        "movl %%ecx, %%edx\n"
        : 
        : 
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    int result = 0;
    
    /* Complex control flow with many basic blocks */
    switch (seed % 13) {
        case 0:
            result = v0 + v1;
            /* Fall through */
        case 1:
            result += v2 * v3;
            if (v4 > 1000) {
                result -= v5;
                goto label_a;
            } else {
                result += v6;
                if (v7 < 500) {
                    result *= 2;
                }
            }
            break;
        case 2:
            result = v8 ^ v9;
            for (int i = 0; i < 5; i++) {
                result += v10;
                if (i == 3) break;
                result -= v11;
            }
            break;
        case 3:
            result = v12 | v13;
            while (result < 10000) {
                result += v14;
                if (result > 5000) continue;
                result -= v15;
            }
            break;
        case 4:
            result = v16 & v17;
            do {
                result += v18;
                if (result % 7 == 0) goto label_b;
                result -= v19;
            } while (result < 2000);
            break;
        case 5:
            result = v20 + v21;
            if (v22 > v23) {
                result = v24 - v25;
            } else if (v26 < v27) {
                result = v28 * v29;
            } else {
                result = seed;
            }
            break;
        case 6:
            result = v0 * v29;
            /* Nested loops create more basic blocks */
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 4; j++) {
                    result += i * j;
                    if (j == 2) break;
                }
            }
            break;
        case 7:
            result = v1 | v28;
            if (v2 & 1) {
                result += v3;
                goto label_c;
            }
            result -= v4;
            break;
        case 8:
            result = v5 ^ v27;
            /* Multiple conditions */
            if (v6 > 100 && v7 < 200 && v8 != 0) {
                result += v9;
            } else if (v10 <= 50 || v11 >= 300) {
                result -= v12;
            } else {
                result *= v13;
            }
            break;
        case 9:
            result = v14 + v26;
            /* Loop with early exit */
            for (int i = 0; i < 10; i++) {
                if (i == 7) goto label_d;
                result += v15 + i;
            }
            break;
        case 10:
            result = v16 & v25;
            /* Switch inside switch */
            switch (v17 % 3) {
                case 0: result += v18; break;
                case 1: result -= v19; break;
                case 2: result *= v20; break;
            }
            break;
        case 11:
            result = v21 | v24;
            /* Complex expression with many operands */
            result = v22 + v23 - v24 * v25 / (v26 + 1) ^ v27;
            break;
        case 12:
            result = v28 ^ v29;
            /* Multiple gotos create complex CFG */
            if (v0 > v1) goto label_e;
            if (v2 < v3) goto label_f;
            result = v4;
            break;
    }
    
    /* Labels for goto statements to create additional basic blocks */
label_a:
    result += v6 * 2;
    goto label_merge;
    
label_b:
    result -= v7 / 2;
    goto label_merge;
    
label_c:
    result |= v8;
    goto label_merge;
    
label_d:
    result &= v9;
    goto label_merge;
    
label_e:
    result ^= v10;
    goto label_merge;
    
label_f:
    result |= v11;
    /* Fall through */
    
label_merge:
    /* More arithmetic to extend live ranges */
    result = result + v12 - v13 * v14 / (v15 + 1);
    
    /* Another inline assembly to force graph fixups */
    asm volatile(
        "# More register pressure\n"
        "movl %%esi, %%edi\n"
        "movl %%ebp, %%esp\n"
        : 
        : 
        : "esi", "edi", "ebp", "esp", "memory"
    );
    
    /* Use all variables to prevent elimination */
    USE(v0); USE(v1); USE(v2); USE(v3); USE(v4); USE(v5);
    USE(v6); USE(v7); USE(v8); USE(v9); USE(v10); USE(v11);
    USE(v12); USE(v13); USE(v14); USE(v15); USE(v16); USE(v17);
    USE(v18); USE(v19); USE(v20); USE(v21); USE(v22); USE(v23);
    USE(v24); USE(v25); USE(v26); USE(v27); USE(v28); USE(v29);
    
    return result;
}

/* Second complex function to increase overall compilation complexity */
static int __attribute__((noinline)) another_complex_function(int x) {
    volatile float f1 = x * 1.5f;
    volatile float f2 = x / 2.0f;
    volatile float f3 = f1 + f2;
    volatile float f4 = f1 * f2;
    volatile float f5 = f3 - f4;
    
    volatile double d1 = x * 2.5;
    volatile double d2 = x / 3.0;
    volatile double d3 = d1 + d2;
    
    int *ptr1 = (int*)&f1;
    int *ptr2 = (int*)&d1;
    
    /* Mixed type operations */
    int result = (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    result += (int)d1 + (int)d2 + (int)d3;
    result += *ptr1 + *ptr2;
    
    /* Complex loop with mixed operations */
    for (int i = 0; i < 20; i++) {
        if (i % 2 == 0) {
            f1 += i * 0.5f;
            result += (int)f1;
        } else {
            d1 -= i * 0.25;
            result -= (int)d1;
        }
        
        switch (i % 5) {
            case 0: result |= i; break;
            case 1: result ^= i; break;
            case 2: result &= ~i; break;
            case 3: result += i * 2; break;
            case 4: result -= i / 2; break;
        }
    }
    
    return result;
}

int main(void) {
    int total = 0;
    
    /* Call test functions multiple times with different inputs */
    for (int i = 0; i < 100; i++) {
        total += test_mcf(i);
        total += another_complex_function(i);
        
        /* Prevent loop unrolling */
        if (i % 7 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
