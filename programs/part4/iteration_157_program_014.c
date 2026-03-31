/* test_scheduler_context.c
 * Complex program to trigger GCC scheduler context allocation and cleanup
 * Compile with: gcc -O3 -fschedule-insns2 -fdump-rtl-sched2 test_scheduler_context.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent optimization of critical variables */
#define NOOPT __attribute__((optimize("O0")))
#define KEEP __attribute__((used))

/* Inline assembly to create artificial dependencies */
#define ASM_DEPENDENCY(var) asm volatile("" : "+r"(var))

/* Function 1: Integer-heavy computation with many serial dependencies */
NOOPT int func1_int_heavy(int a, int b, int c, int d, int e) {
    /* Create register pressure with many local variables */
    int v1 = a + b;
    int v2 = v1 * c;
    ASM_DEPENDENCY(v2);
    int v3 = v2 - d;
    int v4 = v3 / e;
    int v5 = v4 << 2;
    int v6 = v5 | 0xFF;
    int v7 = v6 & 0x0F;
    int v8 = v7 ^ v1;
    int v9 = v8 * v2;
    int v10 = v9 + v3;
    int v11 = v10 - v4;
    int v12 = v11 >> 1;
    int v13 = v12 * v5;
    int v14 = v13 % 17;
    int v15 = v14 + v6;
    int v16 = v15 * v7;
    int v17 = v16 / 3;
    int v18 = v17 | v8;
    int v19 = v18 ^ v9;
    int v20 = v19 + v10;
    
    /* Control flow to create multiple basic blocks */
    if (v20 > 1000) {
        v20 = v20 * 2;
        v19 = v19 + v11;
        ASM_DEPENDENCY(v19);
    } else {
        v20 = v20 / 2;
        v19 = v19 - v12;
    }
    
    /* More operations with dependencies */
    int result = v20 + v19 + v13 + v14 + v15 + v16 + v17 + v18;
    ASM_DEPENDENCY(result);
    return result;
}

/* Function 2: Floating-point array processing with loops */
NOOPT double func2_fp_loop(double base, int iterations) {
    /* Many local variables for register pressure */
    double arr[8];
    double sum = 0.0;
    double a = base;
    double b = base * 0.5;
    double c = base * 0.25;
    double d = base * 0.125;
    double e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    
    /* Initialize array with dependencies */
    for (int idx = 0; idx < 8; idx++) {
        arr[idx] = base * idx;
        ASM_DEPENDENCY(idx);
    }
    
    /* Complex floating-point operations */
    for (int iter = 0; iter < iterations; iter++) {
        e = a + b;
        f = c - d;
        g = e * f;
        h = g / (iter + 1);
        i = h + arr[iter % 8];
        j = i * 1.414;
        k = j - 3.14159;
        l = k * k;
        m = l / 2.0;
        n = m + a;
        o = n - b;
        p = o * c;
        q = p / d;
        r = q + e;
        s = r - f;
        t = s * g;
        
        /* Conditional inside loop */
        if (iter % 3 == 0) {
            a = a * 0.9;
            b = b + 0.1;
            ASM_DEPENDENCY(a);
        } else if (iter % 3 == 1) {
            c = c * 1.1;
            d = d - 0.05;
        } else {
            e = e * 0.95;
            f = f + 0.15;
        }
        
        sum += t;
    }
    
    return sum;
}

/* Function 3: Mixed operations with control flow and many local variables */
NOOPT long func3_mixed_ops(long x, long y, long z) {
    /* Declare many variables to stress register allocation */
    long a = x, b = y, c = z;
    long d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w;
    
    /* Complex dependency chain */
    d = a * b + c;
    e = d << 3;
    f = e >> 1;
    g = f | 0xABCD;
    h = g & 0xFF00;
    i = h ^ 0x1234;
    j = i % 17;
    k = j * a;
    l = k + b;
    m = l - c;
    n = m / 2;
    o = n * 3;
    p = o | d;
    q = p & e;
    r = q ^ f;
    s = r + g;
    t = s - h;
    u = t * i;
    v = u / j;
    w = v + k;
    
    /* Nested control flow */
    if (x > y) {
        if (z > 0) {
            w = w * 2;
            ASM_DEPENDENCY(w);
            for (int cnt = 0; cnt < 5; cnt++) {
                w = w + cnt;
                if (cnt % 2) {
                    w = w - l;
                } else {
                    w = w + m;
                }
            }
        } else {
            w = w / 2;
            for (int cnt = 0; cnt < 3; cnt++) {
                w = w * cnt;
                ASM_DEPENDENCY(cnt);
            }
        }
    } else {
        w = w + 1000;
        if (z < 0) {
            w = w - 500;
        }
    }
    
    return w + l + m + n + o + p + q + r + s + t + u + v;
}

/* Function 4: Switch statement with different operation blocks */
NOOPT int func4_switch_complex(int mode, int val1, int val2, int val3) {
    int result = 0;
    
    switch (mode % 5) {
        case 0: {
            /* Integer arithmetic block */
            int t1 = val1 * val2;
            int t2 = val3 << 4;
            int t3 = t1 + t2;
            int t4 = t3 / 7;
            int t5 = t4 | 0xF0;
            int t6 = t5 & 0x3F;
            int t7 = t6 ^ val1;
            int t8 = t7 * 3;
            int t9 = t8 - val2;
            int t10 = t9 + val3;
            ASM_DEPENDENCY(t10);
            result = t10;
            break;
        }
        case 1: {
            /* Bit manipulation block */
            int b1 = val1 ^ val2;
            int b2 = b1 | val3;
            int b3 = b2 << 1;
            int b4 = b3 >> 2;
            int b5 = b4 & 0x7F;
            int b6 = b5 ^ 0x55;
            int b7 = b6 | 0xAA;
            int b8 = b7 << 3;
            int b9 = b8 >> 1;
            int b10 = b9 & 0xFF;
            ASM_DEPENDENCY(b10);
            result = b10;
            break;
        }
        case 2: {
            /* Mixed operations with loop */
            int accum = 0;
            for (int i = 0; i < 8; i++) {
                accum += val1 * i;
                accum -= val2 * (i + 1);
                accum ^= val3 * (i + 2);
                if (i % 2) {
                    accum = accum >> 1;
                } else {
                    accum = accum << 1;
                }
                ASM_DEPENDENCY(i);
            }
            result = accum;
            break;
        }
        case 3: {
            /* Complex dependency chain */
            int chain[6];
            chain[0] = val1;
            for (int i = 1; i < 6; i++) {
                chain[i] = chain[i-1] * val2 + val3;
                if (chain[i] > 1000) {
                    chain[i] = chain[i] % 1000;
                }
            }
            result = chain[0] + chain[1] + chain[2] + chain[3] + chain[4] + chain[5];
            ASM_DEPENDENCY(result);
            break;
        }
        default: {
            /* Default: simple but with many operations */
            result = val1;
            for (int i = 0; i < 10; i++) {
                result = result * 2 + val2;
                result = result - val3;
                result = result ^ (result >> 4);
                ASM_DEPENDENCY(result);
            }
            break;
        }
    }
    
    return result;
}

/* Main function to ensure all code is executed */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int input1, input2, input3;
    volatile double input4;
    volatile long input5, input6, input7;
    volatile int mode_input;
    
    /* Initialize with non-constant values */
    if (argc > 1) {
        input1 = atoi(argv[1]);
        input2 = atoi(argv[argc > 2 ? 2 : 1]);
        input3 = atoi(argv[argc > 3 ? 3 : 1]);
        input4 = atof(argv[argc > 4 ? 4 : "1.5"]);
        input5 = atol(argv[argc > 5 ? 5 : "100"]);
        input6 = atol(argv[argc > 6 ? 6 : "200"]);
        input7 = atol(argv[argc > 7 ? 7 : "300"]);
        mode_input = atoi(argv[argc > 8 ? 8 : "2"]);
    } else {
        /* Use time-based values if no arguments */
        time_t t = time(NULL);
        input1 = (t % 100) + 1;
        input2 = ((t / 100) % 100) + 1;
        input3 = ((t / 10000) % 100) + 1;
        input4 = 1.5 + ((double)(t % 100) / 100.0);
        input5 = 100 + (t % 1000);
        input6 = 200 + ((t / 10) % 1000);
        input7 = 300 + ((t / 100) % 1000);
        mode_input = t % 10;
    }
    
    /* Call all complex functions */
    int result1 = func1_int_heavy(input1, input2, input3, input1 + input2, input2 + input3);
    double result2 = func2_fp_loop(input4, 5);
    long result3 = func3_mixed_ops(input5, input6, input7);
    int result4 = func4_switch_complex(mode_input, input1, input2, input3);
    
    /* Aggregate results to prevent dead code elimination */
    volatile long final_result = 0;
    final_result += result1;
    final_result += (long)result2;
    final_result += result3;
    final_result += result4;
    
    /* Print to ensure code isn't optimized away */
    printf("Results: %d, %.2f, %ld, %d\n", result1, result2, result3, result4);
    printf("Final aggregated result: %ld\n", final_result);
    
    return 0;
}
