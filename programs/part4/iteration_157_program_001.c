/* test_sched_context.c - Complex program to trigger scheduler context allocation and cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Function 1: Integer-heavy computation with many serial dependencies */
int integer_computation(int seed) {
    /* Declare many local variables to create register pressure */
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    int i = seed + 8, j = seed + 9, k = seed + 10, l = seed + 11;
    int m = seed + 12, n = seed + 13, o = seed + 14, p = seed + 15;
    int q = seed + 16, r = seed + 17, s = seed + 18, t = seed + 19;
    int u = seed + 20, v = seed + 21, w = seed + 22, x = seed + 23;
    int y = seed + 24, z = seed + 25;
    
    /* Create true data dependencies (RAW) */
    a = b + c;          /* 1 */
    d = a * e;          /* 2 - depends on 1 */
    f = d - g;          /* 3 - depends on 2 */
    h = f / (i + 1);    /* 4 - depends on 3 */
    j = h << 2;         /* 5 - depends on 4 */
    k = j | m;          /* 6 - depends on 5 */
    l = k ^ n;          /* 7 - depends on 6 */
    o = l & p;          /* 8 - depends on 7 */
    
    /* Create anti-dependencies (WAR) and output dependencies (WAW) */
    q = r + s;          /* 9 */
    r = q * t;          /* 10 - WAR on r, WAW on q? (actually q is different) */
    s = r - u;          /* 11 - WAR on s */
    
    /* Insert inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(o), "+r"(p));
    
    /* More complex dependencies */
    v = w * x;
    w = v + y;
    x = w - z;
    
    /* Control flow to create multiple basic blocks */
    if (seed % 2 == 0) {
        /* Block A with more dependencies */
        a = b * c + d;
        e = f - g * h;
        i = j | k & l;
        
        /* Memory access pattern */
        int arr[8];
        for (int idx = 2; idx < 8; idx++) {
            arr[idx] = arr[idx-1] + arr[idx-2] + seed;
        }
        m = arr[7];
    } else {
        /* Block B with different dependencies */
        n = o ^ p;
        q = r * s;
        t = u + v - w;
        
        /* Another memory pattern */
        int brr[6];
        for (int idx = 1; idx < 6; idx++) {
            brr[idx] = brr[idx-1] * 2 + seed;
        }
        x = brr[5];
    }
    
    /* Combine results from all variables to prevent dead code elimination */
    int result = a + b + c + d + e + f + g + h + i + j + 
                 k + l + m + n + o + p + q + r + s + t + 
                 u + v + w + x + y + z;
    
    /* Final inline assembly to ensure operations aren't reordered out */
    asm volatile("" : "+r"(result));
    
    return result;
}

/* Function 2: Floating-point array processing with loops */
float float_computation(float seed) {
    /* Many local float variables for register pressure */
    float f1 = seed, f2 = seed * 1.1f, f3 = seed * 1.2f, f4 = seed * 1.3f;
    float f5 = seed * 1.4f, f6 = seed * 1.5f, f7 = seed * 1.6f, f8 = seed * 1.7f;
    float f9 = seed * 1.8f, f10 = seed * 1.9f, f11 = seed * 2.0f, f12 = seed * 2.1f;
    float f13 = seed * 2.2f, f14 = seed * 2.3f, f15 = seed * 2.4f, f16 = seed * 2.5f;
    
    /* Mixed integer/float operations */
    int i1 = (int)seed, i2 = i1 + 1, i3 = i2 * 2, i4 = i3 - 3;
    
    /* Create dependency chains */
    f1 = f2 + f3;
    f4 = f1 * f5;
    f6 = f4 - f7;
    f8 = f6 / f9;
    f10 = sinf(f8) + cosf(f9);
    
    /* Loop with dependencies */
    float arr[20];
    arr[0] = seed;
    arr[1] = seed * 0.5f;
    
    for (int i = 2; i < 20; i++) {
        /* True dependencies within loop */
        arr[i] = arr[i-1] * arr[i-2] + (float)i;
        
        /* Anti-dependency: reuse f11 */
        f11 = f12 + arr[i];
        f12 = f11 * 0.9f;
    }
    
    /* Complex conditional */
    if (f10 > 0.0f) {
        f13 = f14 * f15 - f16;
        f14 = f13 / (f15 + 1.0f);
        
        /* Nested loop */
        for (int j = 0; j < 10; j++) {
            f15 = f16 + (float)j;
            f16 = f15 * 0.8f;
        }
    } else {
        f13 = f14 + f15 + f16;
        f14 = f13 * 0.7f;
    }
    
    /* Use all variables */
    float result = f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10 +
                   f11 + f12 + f13 + f14 + f15 + f16 + (float)i1 + 
                   (float)i2 + (float)i3 + (float)i4 + arr[19];
    
    asm volatile("" : "+r"(result));
    return result;
}

/* Function 3: Mixed operations with control flow and many locals */
long mixed_operations(long seed) {
    /* Declare many variables of different types */
    long l1 = seed, l2 = seed + 100, l3 = seed + 200, l4 = seed + 300;
    double d1 = (double)seed, d2 = d1 * 1.1, d3 = d2 * 1.2, d4 = d3 * 1.3;
    int i1 = (int)seed, i2 = i1 * 2, i3 = i2 + 3, i4 = i3 - 4;
    float f1 = (float)seed, f2 = f1 * 2.0f, f3 = f2 / 3.0f, f4 = f3 + 4.0f;
    
    /* Complex control flow with switch */
    long result = 0;
    switch (seed % 5) {
        case 0: {
            /* Case with integer dependencies */
            l1 = l2 + l3;
            l2 = l1 * l4;
            l3 = l2 - l4;
            l4 = l3 / (l1 + 1);
            
            d1 = d2 + d3;
            d2 = d1 * d4;
            result = (long)(l1 + l2 + l3 + l4 + d1 + d2);
            break;
        }
        case 1: {
            /* Case with float dependencies */
            f1 = f2 + f3;
            f2 = f1 * f4;
            f3 = f2 - f4;
            f4 = f3 / (f1 + 1.0f);
            
            i1 = i2 + i3;
            i2 = i1 * i4;
            result = (long)(f1 + f2 + f3 + f4) + i1 + i2;
            break;
        }
        case 2: {
            /* Mixed case */
            l1 = (long)(d1 + d2);
            d1 = (double)(l2 + l3);
            f1 = (float)(i1 + i2);
            i1 = (int)(f2 + f3);
            
            /* Memory operations */
            long arr[10];
            for (int idx = 0; idx < 10; idx++) {
                arr[idx] = idx * seed;
            }
            result = arr[5] + arr[6] + arr[7];
            break;
        }
        case 3: {
            /* More complex dependencies */
            for (int i = 0; i < 8; i++) {
                l1 = l2 + i;
                l2 = l1 * (i + 1);
                l3 = l2 - l4;
                l4 = l3 ^ 0xFF;
            }
            result = l1 + l2 + l3 + l4;
            break;
        }
        default: {
            /* Default with all operations */
            l1 = l2 * l3 + l4;
            d1 = sqrt(d2 * d2 + d3 * d3);
            f1 = f2 * f3 - f4;
            i1 = i2 | i3 & i4;
            result = (long)(l1 + d1 + f1) + i1;
            break;
        }
    }
    
    /* Final dependency chain */
    long temp = result;
    for (int i = 0; i < 5; i++) {
        temp = (temp * 1103515245 + 12345) & 0x7fffffff;
        asm volatile("" : "+r"(temp));
    }
    
    return temp;
}

/* Function 4: Nested loops with complex index calculations */
unsigned long loop_intensive(unsigned long seed) {
    unsigned long a = seed, b = seed * 3, c = seed * 5, d = seed * 7;
    unsigned long e = seed * 11, f = seed * 13, g = seed * 17, h = seed * 19;
    unsigned long i = seed * 23, j = seed * 29, k = seed * 31, l = seed * 37;
    
    /* Outer loop */
    for (unsigned long x = 0; x < 50; x++) {
        /* Middle loop */
        for (unsigned long y = 0; y < 20; y++) {
            /* Inner loop with dependencies */
            for (unsigned long z = 0; z < 10; z++) {
                /* Complex address calculation with dependencies */
                a = b + c * z;
                b = c + d * y;
                c = d + e * x;
                d = e + f;
                e = f + g;
                
                /* Conditional inside innermost loop */
                if ((z + y + x) % 3 == 0) {
                    f = g * h;
                    g = h ^ i;
                    h = i | j;
                } else {
                    f = g + h;
                    g = h - i;
                    h = i & j;
                }
                
                /* Memory access pattern */
                unsigned long arr[8];
                unsigned long idx = (x + y + z) % 8;
                arr[idx] = a + b + c + d;
                i = arr[idx] + j;
                j = i * k;
                k = j / (l + 1);
            }
            
            /* Insert barrier in middle loop */
            asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d));
        }
        
        /* More operations in outer loop */
        l = (a ^ b) | (c & d);
        a = b + c + d + e;
        b = c * d - e * f;
    }
    
    /* Combine all results */
    unsigned long result = a + b + c + d + e + f + g + h + i + j + k + l;
    
    /* Final computation to use result */
    result = (result * 1103515245 + 12345) & 0x7fffffff;
    
    asm volatile("" : "+r"(result));
    return result;
}

/* Main function to drive execution */
int main(int argc, char *argv[]) {
    /* Use volatile inputs to prevent constant propagation */
    volatile int int_seed;
    volatile float float_seed;
    volatile long long_seed;
    volatile unsigned long ulong_seed;
    
    /* Read from command line or use defaults */
    if (argc > 4) {
        int_seed = atoi(argv[1]);
        float_seed = atof(argv[2]);
        long_seed = atol(argv[3]);
        ulong_seed = strtoul(argv[4], NULL, 0);
    } else {
        /* Default seeds if no arguments */
        int_seed = 12345;
        float_seed = 3.14159f;
        long_seed = 987654321L;
        ulong_seed = 0xDEADBEEF;
    }
    
    /* Call all complex functions */
    int res1 = integer_computation(int_seed);
    float res2 = float_computation(float_seed);
    long res3 = mixed_operations(long_seed);
    unsigned long res4 = loop_intensive(ulong_seed);
    
    /* Aggregate results to volatile sink to prevent optimization */
    volatile long long final_result = 0;
    final_result += res1;
    final_result += (long long)res2;
    final_result += res3;
    final_result += res4;
    
    /* Print checksum to ensure all computations are used */
    printf("Result checksum: %lld\n", (long long)final_result);
    
    return 0;
}
