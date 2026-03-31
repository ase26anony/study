/* test_scheduler_context.c
 * Compile with: gcc -O2 -fschedule-insns -fdump-rtl-sched test_scheduler_context.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Integer-heavy computation with many serial dependencies */
int func1_intensive(int a, int b, int c, int d, int e) {
    /* Create register pressure with many local variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* True data dependencies (RAW) */
    v1 = a + b;
    v2 = v1 * c;
    v3 = v2 - d;
    v4 = v3 / e;
    v5 = v4 << 2;
    v6 = v5 | 0xFF;
    v7 = v6 & 0x0F;
    v8 = v7 ^ v1;
    v9 = v8 + v2;
    v10 = v9 - v3;
    
    /* Anti-dependencies (WAR) and Output dependencies (WAW) */
    v11 = v10;
    v10 = v11 + 1;  /* WAR */
    v12 = v10 * 2;
    v12 = v12 + 5;  /* WAW */
    
    /* More complex dependency chain */
    v13 = v12 >> 1;
    v14 = v13 * v4;
    v15 = v14 % 17;
    v16 = v15 + v5;
    v17 = v16 - v6;
    v18 = v17 | v7;
    v19 = v18 ^ v8;
    v20 = v19 & v9;
    
    /* Use inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v20));
    
    v21 = v20 * 3;
    v22 = v21 / 2;
    v23 = v22 + v10;
    v24 = v23 - v11;
    v25 = v24 << 3;
    v26 = v25 >> 1;
    v27 = v26 | v12;
    v28 = v27 ^ v13;
    v29 = v28 & v14;
    v30 = v29 + v15;
    
    /* Control flow to create basic block boundaries */
    if (v30 > 1000) {
        v30 = v30 / 2;
        v29 = v29 * 3;
    } else {
        v30 = v30 * 3;
        v29 = v29 / 2;
    }
    
    /* Final computation using most variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
           v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
}

/* Function 2: Floating-point array processing with loops */
float func2_fp_loop(float base, int iterations) {
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    float f11, f12, f13, f14, f15, f16, f17, f18, f19, f20;
    
    f1 = base;
    
    /* Loop with dependencies across iterations */
    for (int i = 0; i < iterations; i++) {
        f2 = f1 * 1.1f;
        f3 = f2 + 2.5f;
        f4 = f3 / 1.7f;
        f5 = f4 - 0.3f;
        
        /* Conditional inside loop creates basic blocks */
        if (i % 3 == 0) {
            f6 = f5 * 2.0f;
            f7 = f6 + f1;
        } else if (i % 3 == 1) {
            f6 = f5 / 2.0f;
            f7 = f6 - f1;
        } else {
            f6 = f5;
            f7 = f6;
        }
        
        f8 = f7 * 0.9f;
        f9 = f8 + f2;
        f10 = f9 - f3;
        
        /* Inline assembly to prevent optimization */
        asm volatile("" : "+r"(f10));
        
        f11 = f10 * f4;
        f12 = f11 / f5;
        f13 = f12 + f6;
        f14 = f13 - f7;
        f15 = f14 * 1.5f;
        
        f1 = f15;  /* Feed back into next iteration */
    }
    
    /* Use remaining variables to prevent dead store elimination */
    f16 = f1 * 0.5f;
    f17 = f16 + 1.0f;
    f18 = f17 / 2.0f;
    f19 = f18 - 0.25f;
    f20 = f19 * 4.0f;
    
    return f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10 +
           f11 + f12 + f13 + f14 + f15 + f16 + f17 + f18 + f19 + f20;
}

/* Function 3: Mixed operations with control flow and many local variables */
long func3_mixed_control(int x, int y, float z, double w) {
    /* Mixed type variables to use different functional units */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    float f1, f2, f3, f4, f5;
    double d1, d2, d3, d4, d5;
    long l1, l2, l3, l4, l5;
    
    /* Initial computations */
    i1 = x + y;
    f1 = z * 2.0f;
    d1 = w / 3.0;
    l1 = (long)i1 * 2L;
    
    /* Complex if-else chain creating multiple basic blocks */
    if (x > y) {
        i2 = i1 * 3;
        f2 = f1 + 1.5f;
        d2 = d1 - 0.5;
        l2 = l1 + 1000L;
        
        if (z > 0) {
            i3 = i2 / 2;
            f3 = f2 * 2.0f;
            d3 = d2 * 1.1;
            l3 = l2 << 1;
        } else {
            i3 = i2 * 2;
            f3 = f2 / 2.0f;
            d3 = d2 / 1.1;
            l3 = l2 >> 1;
        }
    } else {
        i2 = i1 / 3;
        f2 = f1 - 1.5f;
        d2 = d1 + 0.5;
        l2 = l1 - 1000L;
        
        if (w < 0) {
            i3 = i2 + 100;
            f3 = f2 + 3.0f;
            d3 = d2 - 0.2;
            l3 = l2 | 0xFFL;
        } else {
            i3 = i2 - 100;
            f3 = f2 - 3.0f;
            d3 = d2 + 0.2;
            l3 = l2 & 0xF0F0L;
        }
    }
    
    /* More operations after control flow */
    i4 = i3 ^ 0xAA;
    f4 = f3 * f1;
    d4 = d3 / d1;
    l4 = l3 * l1;
    
    /* Artificial dependency barrier */
    asm volatile("" : "+r"(i4), "+r"(f4), "+r"(d4), "+r"(l4));
    
    i5 = i4 << 2;
    f5 = f4 + f2;
    d5 = d4 - d2;
    l5 = l4 + l2;
    
    i6 = i5 >> 1;
    i7 = i6 * i3;
    i8 = i7 % 17;
    i9 = i8 + i2;
    i10 = i9 - i1;
    
    /* Final mixed-type computation */
    return l5 + (long)i10 + (long)(f5 * 100.0f) + (long)(d5 * 1000.0);
}

/* Function 4: Switch statement with different operation blocks per case */
int func4_switch_complex(int mode, int a, int b, int c) {
    int r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
    int r11, r12, r13, r14, r15, r16, r17, r18, r19, r20;
    
    /* Switch creates multiple basic blocks */
    switch (mode % 5) {
        case 0:
            /* Arithmetic intensive block */
            r1 = a + b;
            r2 = r1 * c;
            r3 = r2 - a;
            r4 = r3 / b;
            r5 = r4 << c;
            r6 = r5 | 0xFF;
            r7 = r6 & 0x0F;
            r8 = r7 ^ r1;
            r9 = r8 + r2;
            r10 = r9 - r3;
            break;
            
        case 1:
            /* Bit manipulation block */
            r1 = a ^ b;
            r2 = r1 | c;
            r3 = r2 & 0xFFFF;
            r4 = r3 << 4;
            r5 = r4 >> 2;
            r6 = ~r5;
            r7 = r6 ^ 0xAA;
            r8 = r7 | 0x55;
            r9 = r8 & r1;
            r10 = r9 << 1;
            break;
            
        case 2:
            /* Mixed operations block */
            r1 = a * b;
            r2 = r1 + c;
            r3 = r2 - a;
            r4 = r3 * 2;
            r5 = r4 / 3;
            r6 = r5 | b;
            r7 = r6 ^ c;
            r8 = r7 & a;
            r9 = r8 << 2;
            r10 = r9 >> 1;
            break;
            
        case 3:
            /* Serial dependency chain */
            r1 = a + 1;
            r2 = r1 * 2;
            r3 = r2 - 3;
            r4 = r3 / 4;
            r5 = r4 + 5;
            r6 = r5 * 6;
            r7 = r6 - 7;
            r8 = r7 / 8;
            r9 = r8 + 9;
            r10 = r9 * 10;
            break;
            
        default: /* case 4 */
            /* Complex with inline assembly */
            r1 = a + b + c;
            asm volatile("" : "+r"(r1));
            r2 = r1 * 3;
            r3 = r2 - a;
            asm volatile("" : "+r"(r3));
            r4 = r3 / 2;
            r5 = r4 | b;
            r6 = r5 ^ c;
            r7 = r6 & r1;
            r8 = r7 << r2;
            r9 = r8 >> 1;
            r10 = r9 + r3;
            break;
    }
    
    /* Common post-switch operations with many variables */
    r11 = r10 * 2;
    r12 = r11 + r1;
    r13 = r12 - r2;
    r14 = r13 * r3;
    r15 = r14 / r4;
    r16 = r15 | r5;
    r17 = r16 ^ r6;
    r18 = r17 & r7;
    r19 = r18 << 1;
    r20 = r19 >> 2;
    
    /* Final dependency chain */
    int result = r20;
    for (int i = 0; i < 3; i++) {
        result = result + r11 + r12 - r13;
        result = result * 2 - 1;
    }
    
    return result + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
}

/* Main function to ensure all code is executed */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    srand(seed);
    
    /* Generate dynamic inputs */
    int a = rand() % 1000 + 1;
    int b = rand() % 1000 + 1;
    int c = rand() % 1000 + 1;
    int d = rand() % 1000 + 1;
    int e = rand() % 1000 + 1;
    int mode = rand() % 100;
    int iterations = 10 + (rand() % 20);
    float f_base = (float)(rand() % 1000) / 10.0f;
    double d_val = (double)(rand() % 1000) / 10.0;
    
    /* Call all functions to ensure they're compiled and executed */
    int res1 = func1_intensive(a, b, c, d, e);
    float res2 = func2_fp_loop(f_base, iterations);
    long res3 = func3_mixed_control(a, b, f_base, d_val);
    int res4 = func4_switch_complex(mode, a, b, c);
    
    /* Aggregate results to a volatile sink to prevent dead code elimination */
    volatile long final_result = 0;
    final_result += res1;
    final_result += (long)res2;
    final_result += res3;
    final_result += res4;
    
    /* Print to ensure code isn't optimized away */
    printf("Result: %ld\n", final_result);
    
    return 0;
}
