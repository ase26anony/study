/* mcf_trigger.c - Program to trigger GCC's Minimum Cost Flow debug output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to generate complex control flow and register pressure */
__attribute__((noinline))
unsigned long test_mcf(int iterations) {
    /* Declare many local variables to create register pressure */
    volatile int v0 = 1, v1 = 2, v2 = 3, v3 = 4, v4 = 5;
    volatile int v5 = 6, v6 = 7, v7 = 8, v8 = 9, v9 = 10;
    volatile int v10 = 11, v11 = 12, v12 = 13, v13 = 14, v14 = 15;
    volatile int v15 = 16, v16 = 17, v17 = 18, v18 = 19, v19 = 20;
    volatile int v20 = 21, v21 = 22, v22 = 23, v23 = 24, v24 = 25;
    volatile int v25 = 26, v26 = 27, v27 = 28, v28 = 29, v29 = 30;
    
    /* Mix different types to increase pressure across register classes */
    volatile float f0 = 1.0f, f1 = 2.0f, f2 = 3.0f;
    volatile double d0 = 1.0, d1 = 2.0;
    volatile void* p0 = &v0, *p1 = &v1;
    
    unsigned long checksum = 0;
    int i;
    
    /* Complex loop with many basic blocks */
    for (i = 0; i < iterations; i++) {
        /* Create data dependencies between variables */
        v0 = v1 + v2;
        v1 = v3 - v4;
        v2 = v5 * v6;
        v3 = v7 / (v8 + 1);
        v4 = v9 ^ v10;
        v5 = v11 | v12;
        v6 = v13 & v14;
        v7 = v15 << 2;
        v8 = v16 >> 1;
        v9 = v17 + v18;
        v10 = v19 - v20;
        
        /* Floating point operations to use FP registers */
        f0 = f1 * 1.5f;
        f1 = f2 + 0.5f;
        f2 = f0 - f1;
        d0 = d1 * 1.25;
        d1 = d0 / 2.0;
        
        /* Pointer arithmetic */
        p0 = (char*)p1 + v0;
        p1 = (char*)p0 - v1;
        
        /* Complex conditional chain creating many basic blocks */
        if (v0 % 3 == 0) {
            v11 = v12 + v13;
            /* Inline asm with register clobbers to force graph transformations */
            asm volatile (
                "# Force register pressure\n"
                : "=r"(v12), "=r"(v13)
                : "0"(v11), "1"(v14)
                : "eax", "ebx", "ecx", "edx", "memory"
            );
        } else if (v0 % 3 == 1) {
            v12 = v13 * v14;
            /* Another asm with different clobbers */
            asm volatile (
                "# More register pressure\n"
                : "=r"(v13), "=r"(v14)
                : "0"(v12), "1"(v15)
                : "esi", "edi", "ebp", "memory"
            );
        } else {
            v13 = v14 ^ v15;
            /* Third asm statement */
            asm volatile (
                "# Even more pressure\n"
                : "=r"(v14), "=r"(v15)
                : "0"(v13), "1"(v16)
                : "r8", "r9", "r10", "r11", "memory"
            );
        }
        
        /* Nested switch with many cases */
        switch (v1 % 7) {
            case 0:
                v16 = v17 + 1;
                checksum += v16;
                break;
            case 1:
                v17 = v18 - 1;
                checksum += v17;
                /* Fall through to create more complex CFG */
            case 2:
                v18 = v19 * 2;
                checksum += v18;
                break;
            case 3:
                v19 = v20 / 2;
                checksum += v19;
                /* goto to create irregular control flow */
                if (v19 > 100) goto special_case;
                break;
            case 4:
                v20 = v21 | 0xFF;
                checksum += v20;
                break;
            case 5:
                v21 = v22 & 0x0F;
                checksum += v21;
                break;
            case 6:
                v22 = v23 << 1;
                checksum += v22;
                break;
            default:
                v23 = v24 ^ 0x55;
                checksum += v23;
        }
        
        /* Inner loop with break/continue */
        int j;
        for (j = 0; j < 5; j++) {
            if (v2 % 2 == 0) {
                v24 = v25 + j;
                if (v24 > 50) break;
            } else {
                v25 = v26 - j;
                if (v25 < 0) continue;
            }
            v26 = v27 * (j + 1);
            checksum += v26;
        }
        
        /* Label for goto */
        special_case:
        v27 = v28 + v29;
        checksum += v27;
        
        /* More arithmetic to ensure all variables are used */
        v28 = v29 * v0;
        v29 = v0 + i;
        
        checksum += v28 + v29 + (unsigned long)f0 + (unsigned long)d0;
    }
    
    /* Final computation using all variables */
    checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    checksum += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    checksum += v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    checksum += (unsigned long)f0 + (unsigned long)f1 + (unsigned long)f2;
    checksum += (unsigned long)d0 + (unsigned long)d1;
    checksum += (unsigned long)p0 + (unsigned long)p1;
    
    return checksum;
}

/* Another complex function to prevent interprocedural optimization */
__attribute__((noinline))
unsigned long test_mcf2(int seed) {
    volatile int a = seed, b = seed + 1, c = seed + 2;
    volatile int d = seed + 3, e = seed + 4, f = seed + 5;
    unsigned long sum = 0;
    
    /* Different pattern of control flow */
    if (seed % 2 == 0) {
        for (int i = 0; i < 10; i++) {
            a = b + c;
            b = c + d;
            c = d + e;
            d = e + f;
            e = f + a;
            f = a + b;
            
            /* Another asm with clobbers */
            asm volatile (
                "# Additional pressure\n"
                : "+r"(a), "+r"(b)
                :
                : "xmm0", "xmm1", "xmm2", "memory"
            );
            
            sum += a + b + c + d + e + f;
            
            if (sum > 1000000) {
                /* Early exit path */
                return sum;
            }
        }
    } else {
        int counter = 0;
        while (counter++ < 8) {
            a = b * c;
            b = c * d;
            c = d * e;
            sum += a | b | c;
            
            if (counter == 4) {
                /* Complex conditional with goto */
                if (a > b) goto loop_exit;
            }
        }
        loop_exit:
        sum += d + e + f;
    }
    
    return sum;
}

int main() {
    unsigned long total = 0;
    
    /* Call test functions multiple times with different inputs */
    for (int i = 0; i < 5; i++) {
        total += test_mcf(10 + i);
        total += test_mcf2(i * 7);
    }
    
    printf("Result: %lu\n", total);
    return 0;
}
