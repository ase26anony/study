/* test_scheduler_context.c
 * Complex program to trigger scheduler context allocation and cleanup
 * Compile with: gcc -O3 -fschedule-insns2 -fdump-rtl-sched2 test_scheduler_context.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Volatile input to prevent constant propagation */
volatile int g_input = 42;
volatile float g_float_input = 3.14159f;

/* Function 1: Integer-heavy computation with many serial dependencies */
int integer_heavy_computation(int base) {
    /* Declare many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af;
    
    /* Initial values with dependencies */
    a = base + 1;
    b = a * 2;
    c = b - base;
    d = c / 3;
    e = d << 2;
    f = e | 0xFF;
    g = f & 0x0F;
    h = g ^ base;
    i = h % 17;
    j = i + a;
    
    /* Create anti-dependencies and output dependencies */
    k = j;
    k = k + b;  /* output dependency */
    l = k - c;
    m = l * d;
    n = m / e;
    o = n << f;
    p = o | g;
    q = p & h;
    r = q ^ i;
    s = r % j;
    t = s + k;
    
    /* More operations to extend basic block */
    u = t * l;
    v = u / m;
    w = v << n;
    x = w | o;
    y = x & p;
    z = y ^ q;
    aa = z % r;
    ab = aa + s;
    ac = ab * t;
    ad = ac / u;
    ae = ad << v;
    af = ae | w;
    
    /* Inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(af) : : "memory");
    
    /* Control flow to split into multiple basic blocks */
    if (af > 1000) {
        /* True branch with more dependencies */
        int tmp1 = af * 2;
        int tmp2 = tmp1 - base;
        int tmp3 = tmp2 / 3;
        af = tmp3 + af;
    } else {
        /* False branch with different dependencies */
        int tmp1 = af + base;
        int tmp2 = tmp1 * 3;
        int tmp3 = tmp2 >> 1;
        af = tmp3 - af;
    }
    
    /* Final computation with all variables */
    return (a + b + c + d + e + f + g + h + i + j + 
            k + l + m + n + o + p + q + r + s + t +
            u + v + w + x + y + z + aa + ab + ac + ad + ae + af) % 10000;
}

/* Function 2: Floating-point array processing with loops */
float float_array_processing(int size, float seed) {
    float arr[64];
    float result = seed;
    int i, j;
    
    /* Initialize array with dependencies */
    arr[0] = seed;
    arr[1] = seed * 2.0f;
    for (i = 2; i < size && i < 64; i++) {
        /* True data dependency chain */
        arr[i] = arr[i-1] + arr[i-2] * 0.5f;
        
        /* Anti-dependency: read arr[i-3] then modify result */
        float temp = arr[i-3] + result;
        result = temp * 0.8f;
        
        /* Output dependency on result */
        result = result + arr[i] * 0.2f;
    }
    
    /* Nested loop with more complex dependencies */
    for (i = 0; i < size && i < 64; i += 4) {
        for (j = 0; j < 4 && (i+j) < 64; j++) {
            /* Mixed integer/float operations */
            int idx = i + j;
            float val = arr[idx];
            
            /* Complex floating point operations */
            val = val * val + sqrtf(fabsf(val));
            val = sinf(val) * cosf(val);
            
            /* Dependency chain */
            if (idx > 0) {
                val = val + arr[idx-1] * 0.1f;
            }
            
            arr[idx] = val;
            result += val;
        }
        
        /* Inline assembly to prevent reordering */
        asm volatile("" : "+r"(result) : : "memory");
    }
    
    return result;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_operations(int mode, double input) {
    /* Many local variables of different types */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    float f1, f2, f3, f4, f5;
    
    /* Initialize with dependencies */
    d1 = input;
    i1 = (int)d1;
    f1 = (float)(d1 - i1);
    
    d2 = d1 * 1.5;
    i2 = i1 << 2;
    f2 = f1 + 0.5f;
    
    d3 = d2 / f2;
    i3 = i2 | 0xAA;
    f3 = f2 * f1;
    
    d4 = sin(d3);
    i4 = i3 & 0x55;
    f4 = cosf(f3);
    
    d5 = d4 + d3;
    i5 = i4 ^ i3;
    f5 = f4 - f3;
    
    /* Complex control flow */
    switch (mode % 5) {
        case 0:
            d6 = d5 * d4;
            i6 = i5 + i4;
            f1 = f5 * 2.0f;  /* Reuse f1 */
            break;
        case 1:
            d6 = d5 / d4;
            i6 = i5 - i4;
            f1 = f5 / 2.0f;
            break;
        case 2:
            d6 = pow(d5, d4);
            i6 = i5 * i4;
            f1 = sqrtf(f5);
            break;
        case 3:
            d6 = log(d5);
            i6 = i5 >> 2;
            f1 = f5 + f4;
            break;
        default:
            d6 = exp(d5);
            i6 = i5 << 2;
            f1 = f5 - f4;
            break;
    }
    
    /* More operations in all cases */
    d7 = d6 + d5;
    i7 = i6 | i5;
    f2 = f1 * f5;
    
    d8 = d7 * d6;
    i8 = i7 & i6;
    f3 = f2 / f1;
    
    d9 = d8 - d7;
    i9 = i8 ^ i7;
    f4 = f3 + f2;
    
    d10 = d9 / d8;
    i10 = i9 + i8;
    f5 = f4 - f3;
    
    /* Artificial dependency via inline assembly */
    asm volatile("" : "+r"(i10), "+r"(d10) : : "memory");
    
    /* Final computation using all variables */
    return d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
           i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10 +
           f1 + f2 + f3 + f4 + f5;
}

/* Function 4: Switch statement with different operation blocks per case */
long switch_based_computation(long val, int selector) {
    long result = val;
    
    switch (selector % 7) {
        case 0: {
            /* Block with many integer operations */
            long a = val * 2;
            long b = a + val;
            long c = b << 3;
            long d = c | 0xFFF;
            long e = d & 0x0F0F;
            long f = e ^ val;
            long g = f % 123;
            long h = g + a;
            long i = h * b;
            long j = i / c;
            result = a + b + c + d + e + f + g + h + i + j;
            break;
        }
        case 1: {
            /* Block with floating point conversions */
            double d1 = (double)val;
            double d2 = d1 * 1.234;
            double d3 = sin(d2);
            double d4 = cos(d3);
            double d5 = d2 + d3 + d4;
            result = (long)(d5 * 1000.0);
            break;
        }
        case 2: {
            /* Block with memory access pattern */
            long arr[16];
            for (int k = 0; k < 16; k++) {
                arr[k] = val + k;
            }
            for (int k = 1; k < 16; k++) {
                arr[k] = arr[k] + arr[k-1];
            }
            result = arr[15];
            break;
        }
        case 3: {
            /* Block with bit manipulation */
            result = val;
            for (int k = 0; k < 32; k++) {
                result = (result << 1) | ((result >> 31) & 1);
                result = result ^ (0x5A5A5A5A << (k % 4));
            }
            break;
        }
        case 4: {
            /* Block with mixed operations */
            result = val;
            for (int k = 0; k < 8; k++) {
                float f = (float)result;
                f = f * 1.5f + (float)k;
                int i = (int)f;
                result = (result << 4) | (i & 0xF);
            }
            break;
        }
        case 5: {
            /* Block with conditional operations */
            result = val;
            for (int k = 0; k < 20; k++) {
                if (k % 3 == 0) {
                    result = result * 3 + 1;
                } else if (k % 3 == 1) {
                    result = result >> 1;
                } else {
                    result = result ^ (result << 2);
                }
            }
            break;
        }
        default: {
            /* Default block with arithmetic series */
            long sum = 0;
            for (int k = 0; k < 20; k++) {
                sum += val * k;
                val = val + (sum % 100);
            }
            result = sum;
            break;
        }
    }
    
    /* Inline assembly to ensure scheduler sees this */
    asm volatile("" : "+r"(result) : : "memory");
    
    return result;
}

/* Main function to drive execution */
int main(int argc, char *argv[]) {
    int result_int = 0;
    float result_float = 0.0f;
    double result_double = 0.0;
    long result_long = 0L;
    
    /* Use command line arguments or default values */
    int base_val = (argc > 1) ? atoi(argv[1]) : g_input;
    float float_seed = (argc > 2) ? atof(argv[2]) : g_float_input;
    
    /* Call all complex functions to trigger scheduler */
    result_int = integer_heavy_computation(base_val);
    printf("Integer result: %d\n", result_int);
    
    result_float = float_array_processing(base_val % 50 + 10, float_seed);
    printf("Float result: %f\n", result_float);
    
    result_double = mixed_operations(base_val, (double)float_seed);
    printf("Double result: %f\n", result_double);
    
    result_long = switch_based_computation(base_val * 100L, base_val);
    printf("Long result: %ld\n", result_long);
    
    /* Aggregate results into volatile sink to prevent optimization */
    volatile int checksum = 0;
    checksum += result_int;
    checksum += (int)result_float;
    checksum += (int)result_double;
    checksum += (int)result_long;
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
