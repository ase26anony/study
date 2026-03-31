/* test_scheduler_context.c
 * Complex program to trigger GCC scheduler context allocation and cleanup
 * Compile with: gcc -O3 -fschedule-insns2 -fdump-rtl-sched2 test_scheduler_context.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Volatile input to prevent constant propagation */
volatile int g_input = 42;
volatile float g_float_input = 3.14159f;

/* Function 1: Integer-heavy computation with many serial dependencies */
int complex_int_chain(int a, int b, int c, int d, int e) {
    /* Create register pressure with many variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Initial assignments with dependencies */
    v1 = a + b;
    v2 = b * c;
    v3 = c - d;
    v4 = d ^ e;
    v5 = e | a;
    
    /* Create anti-dependencies and output dependencies */
    v6 = v1 + v2;
    v7 = v2 * v3;
    v8 = v3 - v4;
    v9 = v4 ^ v5;
    v10 = v5 | v1;
    
    /* More complex dependency chain */
    v11 = v6 * v7;
    v12 = v7 - v8;
    v13 = v8 ^ v9;
    v14 = v9 | v10;
    v15 = v10 + v6;
    
    /* Use inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v11), "+r"(v12));
    
    v16 = v11 * v12;
    v17 = v12 - v13;
    v18 = v13 ^ v14;
    v19 = v14 | v15;
    v20 = v15 + v11;
    
    /* Control flow to create multiple basic blocks */
    if (v16 > 1000) {
        v21 = v16 / 2;
        v22 = v17 * 3;
        v23 = v18 + 100;
    } else {
        v21 = v16 * 2;
        v22 = v17 / 3;
        v23 = v18 - 100;
    }
    
    /* More operations in both branches converge */
    v24 = v21 + v22;
    v25 = v22 * v23;
    v26 = v23 - v21;
    
    /* Final chain with all variables to prevent elimination */
    v27 = v24 + v25 + v26;
    v28 = v25 * v26 * v24;
    v29 = v26 ^ v24 ^ v25;
    v30 = v27 | v28 | v29;
    
    return v30;
}

/* Function 2: Floating-point array processing with loops */
float float_array_processing(float base, int iterations) {
    float arr[32];
    float sum = 0.0f;
    
    /* Initialize array with dependencies */
    arr[0] = base;
    arr[1] = base * 1.1f;
    for (int i = 2; i < 32; i++) {
        /* True data dependency: uses previous two elements */
        arr[i] = arr[i-1] + arr[i-2] * 0.5f;
        
        /* Anti-dependency: modify previous element */
        arr[i-1] = arr[i-1] * 0.9f;
        
        /* Output dependency: multiple writes to sum */
        sum = sum + arr[i];
    }
    
    /* Complex floating-point operations */
    float temp = arr[0];
    for (int i = 0; i < iterations; i++) {
        temp = sinf(temp) * cosf(arr[i % 32]);
        temp = temp + sqrtf(fabsf(arr[(i+1) % 32]));
        
        /* Inline assembly to prevent reordering */
        asm volatile("" : "+f"(temp));
    }
    
    return sum + temp;
}

/* Function 3: Mixed operations with control flow and many locals */
double mixed_operations(int mode, double x, double y) {
    /* Many local variables for register pressure */
    double a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    
    /* Initialize with dependencies */
    a1 = x + y;
    a2 = x - y;
    a3 = x * y;
    a4 = x / (y + 1.0);
    
    i1 = (int)x;
    i2 = (int)y;
    i3 = i1 * i2;
    i4 = i1 + i2;
    
    /* Switch statement for control flow */
    switch (mode % 4) {
        case 0:
            a5 = sin(a1) * cos(a2);
            a6 = exp(a3) / log(fabs(a4) + 1.0);
            i5 = i3 << 2;
            i6 = i4 >> 1;
            break;
        case 1:
            a5 = tan(a1) + atan(a2);
            a6 = pow(a3, 2.0) - sqrt(a4);
            i5 = i3 ^ 0xFF;
            i6 = i4 | 0x0F;
            break;
        case 2:
            a5 = asin(fmod(a1, 1.0)) + acos(fmod(a2, 1.0));
            a6 = hypot(a3, a4);
            i5 = i3 & i4;
            i6 = i3 % (i4 + 1);
            break;
        default:
            a5 = sinh(a1) + cosh(a2);
            a6 = tanh(a3) * erf(a4);
            i5 = ~i3;
            i6 = i4 * 3;
            break;
    }
    
    /* Converge all paths */
    a7 = a5 + a6;
    a8 = a5 * a6;
    a9 = a7 - a8;
    a10 = a7 / (a8 + 0.001);
    
    i7 = i5 + i6;
    i8 = i5 * i6;
    i9 = i7 - i8;
    i10 = i7 ^ i8;
    
    /* Mix types with conversions */
    b1 = a9 + (double)i9;
    b2 = a10 * (double)i10;
    b3 = b1 / b2;
    b4 = b1 * b2 - b3;
    
    /* More operations to increase block size */
    b5 = b3 + b4;
    b6 = b3 * b4;
    b7 = b5 - b6;
    b8 = b5 / b6;
    b9 = b7 * b8;
    b10 = b7 + b8 - b9;
    
    return b10;
}

/* Function 4: Switch statement with different operation blocks */
long switch_based_computation(int selector, long seed) {
    long result = seed;
    
    /* Large switch with different computation patterns */
    switch (selector & 7) {
        case 0: {
            /* Integer arithmetic chain */
            long t1 = result * 1103515245 + 12345;
            long t2 = t1 * 1103515245 + 12345;
            long t3 = t2 * 1103515245 + 12345;
            result = (t1 ^ t2 ^ t3) & 0x7FFFFFFF;
            break;
        }
        case 1: {
            /* Bit manipulation */
            result = (result << 13) ^ result;
            result = (result >> 17) ^ result;
            result = (result << 5) ^ result;
            result = result & 0x7FFFFFFF;
            break;
        }
        case 2: {
            /* Memory-style operations */
            long arr[8];
            for (int i = 0; i < 8; i++) {
                arr[i] = result + i;
                result = result ^ arr[i];
            }
            break;
        }
        case 3: {
            /* Mixed operations */
            result = (result * 3) / 2;
            result = result + (result << 2);
            result = result - (result >> 3);
            break;
        }
        case 4: {
            /* Dependency chain */
            long a = result;
            long b = a * 2;
            long c = b + a;
            long d = c ^ b;
            long e = d * 3;
            long f = e - c;
            result = f ^ d ^ e;
            break;
        }
        case 5: {
            /* Loop with dependencies */
            for (int i = 0; i < 10; i++) {
                result = result * 1664525 + 1013904223;
                if (result & 1) {
                    result = result ^ 0x5A5A5A5A;
                }
            }
            break;
        }
        case 6: {
            /* Multiple variables */
            long x1 = result, x2 = result + 1, x3 = result + 2;
            for (int i = 0; i < 5; i++) {
                x1 = x1 * x2 + x3;
                x2 = x2 * x3 + x1;
                x3 = x3 * x1 + x2;
            }
            result = x1 + x2 + x3;
            break;
        }
        default: {
            /* Complex chain */
            long y = result;
            y = y * y + y;
            y = (y >> 16) | (y << 16);
            y = y ^ 0xDEADBEEF;
            result = y & 0x7FFFFFFF;
            break;
        }
    }
    
    return result;
}

/* Main function that calls all complex functions */
int main(int argc, char *argv[]) {
    /* Use command line arguments or stdin for dynamic inputs */
    int base_int = g_input;
    if (argc > 1) {
        base_int = atoi(argv[1]);
    }
    
    float base_float = g_float_input;
    if (argc > 2) {
        base_float = atof(argv[2]);
    }
    
    volatile long total = 0;
    
    /* Call Function 1: Integer chain */
    int res1 = complex_int_chain(base_int, base_int + 1, base_int + 2, 
                                 base_int + 3, base_int + 4);
    total += res1;
    
    /* Call Function 2: Float array processing */
    float res2 = float_array_processing(base_float, 50);
    total += (long)res2;
    
    /* Call Function 3: Mixed operations */
    double res3 = mixed_operations(base_int % 4, (double)base_float, 
                                   (double)base_float * 2.0);
    total += (long)res3;
    
    /* Call Function 4: Switch-based computation */
    long res4 = switch_based_computation(base_int, total);
    total += res4;
    
    /* Additional calls with different parameters to increase coverage */
    for (int i = 0; i < 3; i++) {
        res1 = complex_int_chain(base_int + i, base_int + i + 1, 
                                 base_int + i + 2, base_int + i + 3, 
                                 base_int + i + 4);
        total += res1;
        
        res3 = mixed_operations((base_int + i) % 4, 
                                (double)base_float * i, 
                                (double)base_float * (i + 1));
        total += (long)res3;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result checksum: %ld\n", total);
    
    /* Also use volatile sink */
    volatile long sink = total;
    (void)sink;
    
    return 0;
}
