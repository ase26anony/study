/* test_sched_context.c - Complex program to trigger scheduler context allocation */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Integer-heavy computation with many serial dependencies */
int integer_heavy_computation(int seed) {
    /* Create many local variables to increase register pressure */
    int a = seed + 1, b = seed * 2, c = seed / 3, d = seed - 4;
    int e = seed % 5, f = seed | 0xFF, g = seed & 0xF0, h = seed ^ 0x55;
    int i = seed << 2, j = seed >> 1, k = ~seed, l = seed + 100;
    int m = seed * 3, n = seed / 2, o = seed - 10, p = seed % 7;
    int q = seed | 0xAA, r = seed & 0x0F, s = seed ^ 0x33, t = seed << 1;
    
    /* Create true data dependencies (RAW) */
    a = b + c;          /* 1 */
    d = a * e;          /* 2 depends on 1 */
    f = d - g;          /* 3 depends on 2 */
    h = f / i;          /* 4 depends on 3 */
    j = h * k;          /* 5 depends on 4 */
    l = j + m;          /* 6 depends on 5 */
    n = l - o;          /* 7 depends on 6 */
    p = n * q;          /* 8 depends on 7 */
    r = p / s;          /* 9 depends on 8 */
    t = r + a;          /* 10 depends on 9 and 1 */
    
    /* Anti-dependencies (WAR) */
    a = t + b;          /* 11 reuses a after t */
    c = a - d;          /* 12 reuses c after a */
    e = c * f;          /* 13 reuses e after c */
    
    /* Output dependencies (WAW) */
    b = e + g;          /* 14 redefines b */
    b = h - i;          /* 15 redefines b again */
    
    /* Use inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(b), "+r"(c));
    
    /* More complex dependencies */
    int u = b * c;
    int v = u / d;
    int w = v + e;
    int x = w - f;
    int y = x * g;
    int z = y / h;
    
    /* Control flow to create multiple basic blocks */
    if (z > 1000) {
        /* Branch with its own dependencies */
        int aa = z + i;
        int bb = aa * j;
        int cc = bb / k;
        z = cc + l;
    } else {
        /* Else branch with different dependencies */
        int dd = z - m;
        int ee = dd * n;
        int ff = ee / o;
        z = ff + p;
    }
    
    /* Final computation using many variables */
    return a + b + c + d + e + f + g + h + i + j + 
           k + l + m + n + o + p + q + r + s + t + 
           u + v + w + x + y + z;
}

/* Function 2: Floating-point array processing with loops */
float floating_point_processing(float seed) {
    /* Many local float variables for register pressure */
    float f1 = seed * 1.1f, f2 = seed / 2.2f, f3 = seed + 3.3f, f4 = seed - 4.4f;
    float f5 = seed * 5.5f, f6 = seed / 6.6f, f7 = seed + 7.7f, f8 = seed - 8.8f;
    float f9 = seed * 9.9f, f10 = seed / 10.10f, f11 = seed + 11.11f, f12 = seed - 12.12f;
    
    /* Array with memory access dependencies */
    float arr[20];
    for (int i = 0; i < 20; i++) {
        arr[i] = seed * i;
    }
    
    /* Loop with data dependencies across iterations */
    for (int i = 2; i < 18; i++) {
        /* RAW dependencies: arr[i] depends on arr[i-1] and arr[i-2] */
        arr[i] = arr[i-1] + arr[i-2] * f1;
        
        /* Mix with other float operations */
        f1 = f1 * 1.01f;
        f2 = f2 / 1.02f;
        f3 = f3 + arr[i];
        f4 = f4 - f1;
        
        /* Inline assembly to prevent optimization */
        asm volatile("" : "+r"(i));
    }
    
    /* Nested loop with more complex dependencies */
    float sum = 0.0f;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 5; j++) {
            int idx = i * 2 + j;
            sum += arr[idx] * f5 + f6;
            f5 = f5 * 0.99f;
            f6 = f6 / 0.98f;
            
            /* Conditional inside loop */
            if (sum > 100.0f) {
                f7 = f7 + f8;
                f8 = f8 - f9;
            } else {
                f9 = f9 * f10;
                f10 = f10 / f11;
            }
        }
    }
    
    return f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10 + f11 + f12 + sum;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_operations(int int_seed, float float_seed) {
    /* Declare many variables of different types */
    int i1 = int_seed, i2 = int_seed * 2, i3 = int_seed / 3, i4 = int_seed + 4;
    int i5 = int_seed - 5, i6 = int_seed % 6, i7 = int_seed | 0x0F, i8 = int_seed & 0xF0;
    float f1 = float_seed, f2 = float_seed * 2.0f, f3 = float_seed / 3.0f;
    double d1 = (double)int_seed, d2 = d1 * 1.5, d3 = d1 / 2.5;
    long l1 = (long)int_seed, l2 = l1 << 2, l3 = l1 >> 1;
    
    /* Complex control flow with multiple basic blocks */
    double result = 0.0;
    for (int counter = 0; counter < 8; counter++) {
        if (counter % 3 == 0) {
            /* Block A: Integer-heavy */
            i1 = i2 + i3;
            i4 = i1 * i5;
            i6 = i4 - i7;
            i8 = i6 / i2;
            result += (double)i8;
            
            /* Inline assembly barrier */
            asm volatile("" : "+r"(i1), "+r"(i2));
        } 
        else if (counter % 3 == 1) {
            /* Block B: Float-heavy */
            f1 = f2 * f3;
            f2 = f1 + float_seed;
            f3 = f2 / (counter + 1.0f);
            result += (double)f3;
        } 
        else {
            /* Block C: Mixed */
            d1 = d2 + d3;
            d2 = d1 * (double)counter;
            d3 = d2 / (d1 + 1.0);
            l1 = l2 + l3;
            l2 = l1 << 1;
            l3 = l2 >> 2;
            result += d3 + (double)l3;
        }
        
        /* Common code with dependencies on all paths */
        i2 = i2 + counter;
        f2 = f2 * 1.1f;
        d2 = d2 - 0.5;
        l2 = l2 ^ counter;
    }
    
    /* Switch statement creating multiple basic blocks */
    switch (i1 % 4) {
        case 0:
            result = result * 2.0 + (double)(i2 * i3);
            break;
        case 1:
            result = result / 2.0 - (double)(i4 / i5);
            break;
        case 2:
            result = result + (double)f1 * (double)f2;
            break;
        case 3:
            result = result - d1 * d2;
            break;
    }
    
    return result;
}

/* Function 4: Switch statement with different operation blocks per case */
long switch_based_computation(long seed) {
    long a = seed, b = seed * 2, c = seed / 3, d = seed + 4;
    long e = seed - 5, f = seed % 6, g = seed | 0xFF00, h = seed & 0x00FF;
    long i = seed << 3, j = seed >> 2, k = ~seed, l = seed ^ 0xAAAA;
    
    /* Large switch with different dependency patterns in each case */
    switch (seed % 8) {
        case 0: {
            /* Linear dependencies */
            a = b + c;
            d = a * e;
            f = d - g;
            h = f / i;
            j = h * k;
            l = j + a;
            break;
        }
        case 1: {
            /* Parallel chains */
            a = b * c;
            d = e * f;
            g = h * i;
            j = k * l;
            /* Then combine */
            a = a + d;
            g = g + j;
            l = a * g;
            break;
        }
        case 2: {
            /* Anti-dependencies */
            long temp = a;
            a = b;
            b = c;
            c = d;
            d = e;
            e = f;
            f = g;
            g = h;
            h = i;
            i = j;
            j = k;
            k = l;
            l = temp;
            break;
        }
        case 3: {
            /* Memory-like access pattern */
            long arr[8] = {a, b, c, d, e, f, g, h};
            for (int idx = 1; idx < 8; idx++) {
                arr[idx] = arr[idx-1] + arr[idx] * (idx + 1);
            }
            a = arr[0]; b = arr[1]; c = arr[2]; d = arr[3];
            e = arr[4]; f = arr[5]; g = arr[6]; h = arr[7];
            break;
        }
        case 4: {
            /* Complex mixed */
            a = (b & c) | (d ^ e);
            f = (g << 2) + (h >> 1);
            i = (j * k) / (l + 1);
            a = a + f - i;
            break;
        }
        case 5: {
            /* Nested calculations */
            for (int iter = 0; iter < 4; iter++) {
                a = a + b;
                b = b * c;
                c = c - d;
                d = d / (e + 1);
                asm volatile("" : "+r"(a), "+r"(b));
            }
            break;
        }
        case 6: {
            /* Multiple dependency chains */
            long x1 = a + b;
            long x2 = c + d;
            long x3 = e + f;
            long x4 = g + h;
            long y1 = x1 * x2;
            long y2 = x3 * x4;
            l = y1 + y2;
            break;
        }
        case 7: {
            /* All variables used */
            a = b + c + d + e + f + g + h + i + j + k + l;
            b = a * 2;
            c = b / 3;
            d = c - 4;
            l = a + b + c + d;
            break;
        }
    }
    
    /* Final aggregation */
    return a + b + c + d + e + f + g + h + i + j + k + l;
}

/* Main function to drive execution */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int int_input;
    volatile float float_input;
    volatile long long_input;
    
    /* Get dynamic inputs */
    if (argc > 3) {
        int_input = atoi(argv[1]);
        float_input = atof(argv[2]);
        long_input = atol(argv[3]);
    } else {
        /* Use time-based seeds if no arguments */
        int_input = time(NULL) % 1000;
        float_input = (float)(time(NULL) % 100) / 3.14f;
        long_input = (long)time(NULL) * 17L;
    }
    
    /* Call all complex functions */
    int result1 = integer_heavy_computation(int_input);
    float result2 = floating_point_processing(float_input);
    double result3 = mixed_operations(int_input, float_input);
    long result4 = switch_based_computation(long_input);
    
    /* Aggregate results into volatile sink to prevent dead code elimination */
    volatile double final_result = 0.0;
    final_result += (double)result1;
    final_result += (double)result2;
    final_result += result3;
    final_result += (double)result4;
    
    /* Print checksum */
    printf("Result checksum: %f\n", final_result);
    
    return 0;
}
