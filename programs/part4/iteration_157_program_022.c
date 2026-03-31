/* test_scheduler_context.c
 * Compile with: gcc -O2 -fschedule-insns -fdump-rtl-sched test_scheduler_context.c -o test
 * Or with: gcc -O3 -fschedule-insns2 -fdump-rtl-sched2 test_scheduler_context.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Volatile variables to prevent constant propagation */
volatile int g_input1 = 7;
volatile int g_input2 = 13;
volatile float g_input3 = 3.14159f;
volatile float g_input4 = 2.71828f;

/* Function 1: Integer-heavy computation with many serial dependencies */
int integer_heavy_computation(int a, int b, int c) {
    /* Create many local variables to increase register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Long dependency chain with RAW dependencies */
    v1 = a + b;
    v2 = v1 * c;
    v3 = v2 - a;
    v4 = v3 / (b + 1);
    v5 = v4 << 2;
    v6 = v5 | 0xFF;
    v7 = v6 & 0x3F;
    v8 = v7 ^ v1;
    v9 = v8 % (c + 1);
    v10 = v9 + v2;
    
    /* Anti-dependencies (WAR) */
    v11 = v10 * 2;
    v10 = v11 - 5;  /* WAR: v10 is reused */
    
    /* Output dependencies (WAW) */
    v12 = v11 + v3;
    v12 = v12 * 4;  /* WAW: v12 is overwritten */
    
    /* More operations to create scheduling complexity */
    v13 = (v12 >> 3) + v4;
    v14 = v13 * v5;
    v15 = v14 - v6;
    v16 = v15 / (v7 + 1);
    v17 = v16 << 1;
    v18 = v17 | 0x7F;
    v19 = v18 & 0x1F;
    v20 = v19 ^ v8;
    
    /* Use inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v20));
    
    v21 = v20 % (v9 + 2);
    v22 = v21 + v10;
    v23 = v22 * 3;
    v24 = v23 - v11;
    v25 = v24 / (v12 + 1);
    v26 = v25 << 4;
    v27 = v26 | 0x3F;
    v28 = v27 & 0x0F;
    v29 = v28 ^ v13;
    v30 = v29 % (v14 + 3);
    
    /* Final result uses many variables to prevent dead store elimination */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
           v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
}

/* Function 2: Floating-point array processing with loops */
float float_array_processing(float a, float b, int n) {
    float arr[32];
    float sum = 0.0f;
    int i;
    
    /* Initialize array with dependencies */
    arr[0] = a;
    arr[1] = b;
    for (i = 2; i < 32; i++) {
        /* RAW dependency through array elements */
        arr[i] = arr[i-1] + arr[i-2] * 0.5f;
    }
    
    /* Process array with mixed operations */
    for (i = 0; i < n && i < 32; i++) {
        float temp;
        if (i % 4 == 0) {
            temp = arr[i] * arr[i];
        } else if (i % 4 == 1) {
            temp = sqrtf(fabsf(arr[i]));
        } else if (i % 4 == 2) {
            temp = sinf(arr[i]) + cosf(arr[i]);
        } else {
            temp = arr[i] / (arr[(i+1)%32] + 1.0f);
        }
        
        /* Create anti-dependency */
        arr[i] = temp * 0.9f;  /* WAR: arr[i] is reused */
        sum += arr[i];
    }
    
    /* Complex floating-point operations */
    float x = sum;
    for (i = 0; i < 5; i++) {
        x = x * 1.1f - 0.3f;
        x = 1.0f / (x + 2.0f);
        x = x * x - 0.5f;
    }
    
    return sum + x;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_operations(int mode, double x, double y) {
    /* Many local variables for register pressure */
    double a, b, c, d, e, f, g, h, i, j;
    double k, l, m, n, o, p, q, r, s, t;
    
    /* Initial computations */
    a = x + y;
    b = x - y;
    c = x * y;
    d = x / (y + 1.0);
    
    /* Control flow creates multiple basic blocks */
    if (mode == 0) {
        /* Block A with dependencies */
        e = sin(a) + cos(b);
        f = log(fabs(c) + 1.0);
        g = exp(d) - 1.0;
        h = sqrt(a*a + b*b);
        
        /* Inline assembly barrier */
        asm volatile("" : "+r"(h));
        
        i = h * e;
        j = f / g;
    } else if (mode == 1) {
        /* Block B with different dependencies */
        e = tan(a) * atan(b);
        f = pow(c, 2.0);
        g = fmod(d, 2.5);
        h = hypot(a, b);
        
        i = e + f;
        j = g - h;
    } else {
        /* Default block */
        e = a * b;
        f = c + d;
        g = a / (b + 0.001);
        h = c - d;
        
        i = e * f;
        j = g / h;
    }
    
    /* Common code with output dependencies */
    k = i + j;
    k = k * 2.0;  /* WAW */
    
    l = k - a;
    m = l * b;
    n = m / c;
    o = n + d;
    
    /* More operations */
    p = sin(o) * cos(k);
    q = exp(l) / (m + 1.0);
    r = sqrt(n*n + o*o);
    s = p + q - r;
    t = s * 0.5;
    
    return a + b + c + d + e + f + g + h + i + j + 
           k + l + m + n + o + p + q + r + s + t;
}

/* Function 4: Switch statement with different operation blocks per case */
long switch_based_computation(int selector, long base) {
    long result = base;
    
    switch (selector % 5) {
        case 0: {
            /* Integer operations with dependencies */
            long a = base * 3;
            long b = a + 17;
            long c = b >> 2;
            long d = c | 0xABCD;
            long e = d & 0x7F;
            result = a + b + c + d + e;
            break;
        }
        case 1: {
            /* Different integer pattern */
            long a = base ^ 0x1234;
            long b = a * 7;
            long c = b % 31;
            long d = c << 3;
            long e = d - 19;
            result = a * b + c * d - e;
            break;
        }
        case 2: {
            /* Mixed operations */
            double a = (double)base * 1.5;
            double b = a * a;
            double c = sqrt(b);
            result = (long)(a + b + c);
            break;
        }
        case 3: {
            /* Bit manipulation chain */
            long a = ~base;
            long b = a | (base << 8);
            long c = b & 0xF0F0F0F0;
            long d = c ^ 0x0F0F0F0F;
            long e = d >> 4;
            result = e * 11;
            break;
        }
        case 4: {
            /* Complex chain with inline assembly */
            long a = base + 100;
            asm volatile("" : "+r"(a));
            long b = a * 3;
            long c = b - 50;
            asm volatile("" : "+r"(c));
            long d = c / 7;
            long e = d | 0xFF;
            result = a + b + c + d + e;
            break;
        }
    }
    
    /* Post-switch computations */
    if (result > 1000) {
        result = result % 1000;
    } else {
        result = result * 2 + 1;
    }
    
    return result;
}

/* Main function that calls all test functions */
int main(int argc, char *argv[]) {
    int i;
    volatile int result = 0;
    volatile float fresult = 0.0f;
    volatile double dresult = 0.0;
    volatile long lresult = 0L;
    
    /* Use command line arguments or defaults to prevent constant folding */
    int int_arg1 = (argc > 1) ? atoi(argv[1]) : g_input1;
    int int_arg2 = (argc > 2) ? atoi(argv[2]) : g_input2;
    float float_arg1 = (argc > 3) ? atof(argv[3]) : g_input3;
    float float_arg2 = (argc > 4) ? atof(argv[4]) : g_input4;
    
    /* Call each function multiple times with varying inputs */
    for (i = 0; i < 3; i++) {
        /* Function 1 - Integer heavy */
        result += integer_heavy_computation(
            int_arg1 + i, 
            int_arg2 - i, 
            (int_arg1 * int_arg2) % 17
        );
        
        /* Function 2 - Floating point array */
        fresult += float_array_processing(
            float_arg1 + i * 0.1f,
            float_arg2 - i * 0.05f,
            8 + i * 2
        );
        
        /* Function 3 - Mixed operations */
        dresult += mixed_operations(
            i % 3,
            (double)int_arg1 / (i + 1),
            (double)int_arg2 * 0.5
        );
        
        /* Function 4 - Switch based */
        lresult += switch_based_computation(
            int_arg1 + i * 7,
            int_arg2 * 100 + i * 50
        );
    }
    
    /* Aggregate results to a volatile sink to prevent optimization */
    volatile long long final_sink = 
        (long long)result + 
        (long long)fresult + 
        (long long)dresult + 
        (long long)lresult;
    
    /* Print something to ensure execution */
    printf("Results: %d, %.2f, %.2f, %ld\n", 
           result, fresult, dresult, lresult);
    
    return (final_sink > 0) ? 0 : 1;
}
