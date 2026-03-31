/* mcf_coverage.c - Program to trigger MCF pass debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to generate complex flow graph with register pressure */
__attribute__((noinline))
static unsigned long test_mcf_graph(unsigned long seed) {
    /* Declare many variables to create register pressure */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    volatile int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    volatile float f0, f1, f2, f3, f4;
    volatile double d0, d1, d2;
    volatile void *p0, *p1, *p2, *p3;
    unsigned long result = seed;
    
    /* Initialize variables with complex dependencies */
    v0 = (seed & 0xFF) + 1;
    v1 = v0 * 2;
    v2 = v1 - v0;
    v3 = v2 ^ v1;
    v4 = v3 | v0;
    
    /* Create control flow with many basic blocks */
    for (int i = 0; i < 100; i++) {
        /* Complex conditional chain creating multiple basic blocks */
        if (i % 3 == 0) {
            v5 = v4 + i;
            v6 = v5 * 2;
            /* Inline assembly with register clobbering */
            asm volatile ("# Force register pressure" 
                         : "=r"(v7), "=r"(v8)
                         : "0"(v6), "1"(v5)
                         : "eax", "ebx", "ecx", "edx", "memory");
        } else if (i % 3 == 1) {
            v9 = v4 - i;
            v10 = v9 / 2;
            /* More assembly with different clobbers */
            asm volatile ("# More pressure" 
                         : "=r"(v11), "=r"(v12)
                         : "0"(v10), "1"(v9)
                         : "esi", "edi", "ebp", "memory");
        } else {
            v13 = v4 * i;
            v14 = v13 % 17;
            /* Mixed type operations */
            f0 = (float)v14;
            f1 = f0 * 3.14159f;
            v15 = (int)f1;
        }
        
        /* Nested conditionals */
        switch (i % 7) {
            case 0:
                v16 = v0 + v1;
                v17 = v16 << 2;
                break;
            case 1:
                v18 = v2 - v3;
                v19 = v18 >> 1;
                break;
            case 2:
                v20 = v4 * v5;
                v21 = v20 & 0xFF;
                break;
            case 3:
                v22 = v6 ^ v7;
                v23 = v22 | 0xAA;
                break;
            case 4:
                v24 = v8 + v9;
                v25 = v24 * 3;
                break;
            case 5:
                v26 = v10 - v11;
                v27 = v26 / 2;
                break;
            case 6:
                v28 = v12 * v13;
                v29 = v28 % 19;
                /* Force spill/reload with pointer operations */
                p0 = &v29;
                v0 = *(int*)p0;
                break;
        }
        
        /* Cross-basic-block value flow */
        if (i > 50) {
            v1 = v15 + v17;
            v2 = v19 - v21;
            /* More assembly to force graph fixups */
            asm volatile ("# Complex constraint" 
                         : "+r"(v1), "+r"(v2)
                         : 
                         : "xmm0", "xmm1", "xmm2", "memory");
            f2 = (float)v1;
            f3 = (float)v2;
            f4 = f2 * f3;
            d0 = (double)f4;
        } else {
            v3 = v23 | v25;
            v4 = v27 ^ v29;
            /* Different execution path with different register needs */
            asm volatile ("# Alternative path" 
                         : "+r"(v3), "+r"(v4)
                         : 
                         : "xmm3", "xmm4", "xmm5", "memory");
            d1 = (double)v3;
            d2 = (double)v4;
        }
        
        /* Loop-carried dependency */
        result += v0 + v1 + v2 + v3 + v4 + v15 + v29;
        result ^= (result << 13);
        result ^= (result >> 17);
        result ^= (result << 5);
    }
    
    /* Final computation merging all paths */
    unsigned long final = result;
    final += (unsigned long)v5 + v6 + v7 + v8 + v9;
    final += (unsigned long)v10 + v11 + v12 + v13 + v14;
    final += (unsigned long)v16 + v17 + v18 + v19 + v20;
    final += (unsigned long)v21 + v22 + v23 + v24 + v25;
    final += (unsigned long)v26 + v27 + v28 + v29;
    
    /* Use floating point results to prevent elimination */
    final += (unsigned long)f0 + (unsigned long)f1 + (unsigned long)f2;
    final += (unsigned long)f3 + (unsigned long)f4;
    final += (unsigned long)d0 + (unsigned long)d1 + (unsigned long)d2;
    
    return final;
}

/* Another complex function to increase interprocedural pressure */
__attribute__((noinline))
static unsigned long secondary_graph(unsigned long input) {
    volatile int a0 = input & 0xFF, a1, a2, a3, a4, a5;
    volatile float fa, fb, fc;
    
    /* Different control flow pattern */
    for (int j = 0; j < 50; j++) {
        if (j % 4 == 0) {
            a1 = a0 * j;
            a2 = a1 + 0x1234;
            asm volatile ("# Secondary clobber" : "+r"(a1), "+r"(a2) : : "r8", "r9", "r10", "memory");
        } else {
            a3 = a0 - j;
            a4 = a3 * 3;
            asm volatile ("# Different clobbers" : "+r"(a3), "+r"(a4) : : "r11", "r12", "r13", "memory");
        }
        
        /* Switch with many cases */
        switch (j % 11) {
            case 0: a5 = a1; break;
            case 1: a5 = a2; break;
            case 2: a5 = a3; break;
            case 3: a5 = a4; break;
            case 4: a5 = a1 + a2; break;
            case 5: a5 = a3 - a4; break;
            case 6: a5 = a1 * a3; break;
            case 7: a5 = a2 ^ a4; break;
            case 8: a5 = a1 | a3; break;
            case 9: a5 = a2 & a4; break;
            case 10: a5 = ~a1; break;
        }
        
        fa = (float)a5;
        fb = fa * 2.71828f;
        fc = fb / (float)(j + 1);
        a0 = (int)fc;
    }
    
    return (unsigned long)a0 + a1 + a2 + a3 + a4 + a5;
}

int main(void) {
    unsigned long total = 0;
    
    /* Call test functions multiple times with different inputs */
    for (unsigned long i = 0; i < 10; i++) {
        total ^= test_mcf_graph(i * 0x12345678UL);
        total += secondary_graph(i * 0x87654321UL);
        
        /* Prevent loop unrolling from simplifying too much */
        asm volatile ("# Loop barrier" : : : "memory");
    }
    
    printf("Result: %lu\n", total);
    return 0;
}
