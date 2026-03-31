/* test_scheduler_context.c
 * Designed to trigger free_sched_context() coverage in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fdump-rtl-sched test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Volatile sink to prevent optimization */
volatile int global_sink = 0;

/* Function 1: Integer-heavy computation with many serial dependencies */
int func1_intensive(int a, int b, int c, int d, int e) {
    /* Create many local variables to increase register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Long dependency chain with RAW dependencies */
    v1 = a + b;
    asm volatile("" : "+r"(v1));  /* Prevent reordering */
    v2 = v1 * c;
    v3 = v2 - d;
    v4 = v3 / (e + 1);
    v5 = v4 << 2;
    v6 = v5 ^ v1;
    v7 = v6 | v2;
    v8 = v7 & v3;
    v9 = v8 + v4;
    v10 = v9 - v5;
    
    /* Anti-dependencies (WAR) */
    v11 = v10;
    v10 = v11 * 2;  /* v10 written after being read */
    
    /* Output dependencies (WAW) */
    v12 = v11 + 1;
    v12 = v12 * 3;  /* v12 written twice */
    
    /* More operations to create scheduling complexity */
    v13 = v12 % 17;
    v14 = v13 << v11;
    v15 = v14 >> 1;
    v16 = v15 & 0xFF;
    v17 = v16 | 0x80;
    v18 = v17 ^ 0x55;
    v19 = v18 + v12;
    v20 = v19 - v13;
    
    /* Use remaining variables to prevent dead store elimination */
    v21 = v20 * 2;
    v22 = v21 / 3;
    v23 = v22 + v14;
    v24 = v23 - v15;
    v25 = v24 * v16;
    v26 = v25 / (v17 + 1);
    v27 = v26 | v18;
    v28 = v27 & v19;
    v29 = v28 ^ v20;
    v30 = v29 + v21;
    
    /* Complex control flow to create multiple basic blocks */
    if (v30 > 1000) {
        v30 = v30 * 2;
        v29 = v29 + v22;
    } else {
        v30 = v30 / 2;
        v29 = v29 - v22;
    }
    
    /* Another basic block */
    switch (v30 % 4) {
        case 0: v30 = v30 + v23; break;
        case 1: v30 = v30 - v23; break;
        case 2: v30 = v30 * v23; break;
        case 3: v30 = v30 / (v23 + 1); break;
    }
    
    return v30 + v29 + v28 + v27 + v26;
}

/* Function 2: Floating-point array processing with loops */
double func2_fp_loop(double base, int iterations) {
    double arr[20];
    double sum = 0.0;
    int i, j;
    
    /* Initialize array with dependencies */
    arr[0] = base;
    arr[1] = base * 1.1;
    for (i = 2; i < 20; i++) {
        /* RAW dependency through array */
        arr[i] = arr[i-1] + arr[i-2] * 0.5;
        asm volatile("" : "+m"(arr[i]));  /* Memory barrier */
    }
    
    /* Nested loops with mixed operations */
    for (i = 0; i < iterations; i++) {
        double temp = 0.0;
        for (j = 0; j < 10; j++) {
            /* Complex FP operations */
            temp += sin(arr[j] * i) * cos(arr[19-j] * j);
        }
        
        /* Control flow inside loop */
        if (temp > 0) {
            sum += sqrt(temp);
        } else {
            sum -= sqrt(-temp);
        }
        
        /* Update array elements creating dependencies */
        for (j = 1; j < 20; j++) {
            arr[j] = arr[j] * 0.99 + arr[j-1] * 0.01;
        }
    }
    
    return sum;
}

/* Function 3: Mixed operations with control flow and many locals */
long func3_mixed_ops(long x, long y, long z) {
    /* Many local variables for register pressure */
    long a = x, b = y, c = z;
    long d, e, f, g, h, i, j, k, l, m;
    long n, o, p, q, r, s, t, u, v, w;
    
    /* Initial computation block */
    d = a * b + c;
    e = b * c - a;
    f = c * a + b;
    
    /* Branch with different dependency patterns */
    if (d > e) {
        g = d << 2;
        h = e >> 1;
        i = f & 0xFF;
        asm volatile("" : "+r"(g), "+r"(h), "+r"(i));
    } else {
        g = d >> 2;
        h = e << 1;
        i = f | 0xFF;
        asm volatile("" : "+r"(g), "+r"(h), "+r"(i));
    }
    
    /* Another basic block */
    j = g * h;
    k = h + i;
    l = i - g;
    
    /* Switch statement creating multiple basic blocks */
    switch (j % 5) {
        case 0:
            m = j + k;
            n = k * l;
            break;
        case 1:
            m = j - k;
            n = k / (l + 1);
            break;
        case 2:
            m = j * k;
            n = k - l;
            break;
        case 3:
            m = j / (k + 1);
            n = k + l;
            break;
        default:
            m = j ^ k;
            n = k | l;
            break;
    }
    
    /* More operations using all variables */
    o = m + n;
    p = n - m;
    q = o * p;
    r = p / (o + 1);
    s = q ^ r;
    t = r | s;
    u = s & t;
    v = t + u;
    w = u - v;
    
    /* Final computation with all values */
    return a + b + c + d + e + f + g + h + i + j + 
           k + l + m + n + o + p + q + r + s + t + 
           u + v + w;
}

/* Function 4: Switch statement with different operation blocks */
int func4_switch_complex(int mode, int val) {
    int result = val;
    
    switch (mode % 6) {
        case 0: {
            /* Block with integer arithmetic chain */
            int t1 = val * 3;
            int t2 = t1 + 17;
            int t3 = t2 << 1;
            int t4 = t3 ^ 0xAA;
            int t5 = t4 / 7;
            result = t5;
            asm volatile("" : "+r"(result));
            break;
        }
        case 1: {
            /* Block with memory-like operations */
            int arr[8];
            for (int i = 0; i < 8; i++) {
                arr[i] = val * i;
            }
            for (int i = 1; i < 8; i++) {
                arr[i] += arr[i-1];
            }
            result = arr[7];
            break;
        }
        case 2: {
            /* Block with many temporary variables */
            int a = val, b = val+1, c = val+2, d = val+3;
            int e = a*b, f = b*c, g = c*d, h = d*a;
            int i = e+f, j = f+g, k = g+h, l = h+e;
            result = i+j+k+l;
            break;
        }
        case 3: {
            /* Block with conditional operations */
            int x = val;
            for (int i = 0; i < 10; i++) {
                if (x % 2 == 0) {
                    x = x * 3 + 1;
                } else {
                    x = x / 2;
                }
                asm volatile("" : "+r"(x));
            }
            result = x;
            break;
        }
        case 4: {
            /* Block with bit manipulation chain */
            unsigned int u = (unsigned int)val;
            u = (u >> 16) | (u << 16);
            u = u ^ 0xFFFFFFFF;
            u = (u & 0x55555555) << 1 | (u & 0xAAAAAAAA) >> 1;
            u = u * 0x9E3779B9;
            result = (int)u;
            break;
        }
        default: {
            /* Default block with mixed operations */
            double dval = (double)val;
            dval = sin(dval) * cos(dval);
            dval = sqrt(fabs(dval));
            dval = dval * 100.0;
            result = (int)dval;
            break;
        }
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Use command line arguments or stdin to get dynamic values */
    int input1, input2, input3;
    double input4;
    long input5, input6;
    
    if (argc > 5) {
        input1 = atoi(argv[1]);
        input2 = atoi(argv[2]);
        input3 = atoi(argv[3]);
        input4 = atof(argv[4]);
        input5 = atol(argv[5]);
        input6 = atol(argv[6]);
    } else {
        /* Default values if no arguments provided */
        input1 = 12345;
        input2 = 67890;
        input3 = 24680;
        input4 = 3.14159;
        input5 = 1000000;
        input6 = 2000000;
    }
    
    /* Call all functions to trigger scheduler in different contexts */
    int res1 = func1_intensive(input1, input2, input3, input1 ^ input2, input2 | input3);
    double res2 = func2_fp_loop(input4, 5);
    long res3 = func3_mixed_ops(input5, input6, input5 + input6);
    int res4 = func4_switch_complex(input1, input2);
    
    /* Aggregate results into volatile sink to prevent optimization */
    global_sink = res1 + (int)res2 + (int)res3 + res4;
    
    /* Print checksum to ensure all computations are performed */
    printf("Result checksum: %d\n", global_sink);
    
    return 0;
}
