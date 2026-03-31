/* test_sched_context.c - Complex program to trigger scheduler context allocation */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Function 1: Integer-heavy computation with many serial dependencies */
int int_heavy_computation(int seed) {
    /* Create many local variables to increase register pressure */
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    int i = seed + 8, j = seed + 9, k = seed + 10, l = seed + 11;
    int m = seed + 12, n = seed + 13, o = seed + 14, p = seed + 15;
    int q = seed + 16, r = seed + 17, s = seed + 18, t = seed + 19;
    int u = seed + 20, v = seed + 21, w = seed + 22, x = seed + 23;
    int y = seed + 24, z = seed + 25;
    
    /* Create true data dependencies (RAW) */
    a = b + c;          /* 1 */
    d = a * e;          /* 2 depends on 1 */
    f = d - g;          /* 3 depends on 2 */
    h = f / (i + 1);    /* 4 depends on 3 */
    j = h << 2;         /* 5 depends on 4 */
    k = j | m;          /* 6 depends on 5 */
    l = k ^ n;          /* 7 depends on 6 */
    o = l & p;          /* 8 depends on 7 */
    q = o % (r + 1);    /* 9 depends on 8 */
    s = q * t;          /* 10 depends on 9 */
    
    /* Anti-dependencies (WAR) */
    u = v + w;          /* Uses v, w before they're overwritten */
    v = x + y;          /* Overwrites v after it's used above */
    w = z + a;          /* Overwrites w after it's used above */
    
    /* Output dependencies (WAW) */
    x = u * 2;          /* First write to x */
    x = v * 3;          /* Second write to x - WAW with above */
    
    /* Control flow to create multiple basic blocks */
    if (s > 1000) {
        /* Block A with more dependencies */
        y = s >> 3;
        z = y * y;
        a = z + b;
    } else {
        /* Block B with different dependencies */
        y = s << 3;
        z = y / 2;
        a = z - b;
    }
    
    /* Use inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(a), "+r"(b), "+r"(c));
    
    /* More operations to extend the basic block */
    b = a * c;
    c = b + d;
    d = c - e;
    e = d * f;
    
    /* Final result uses many variables to prevent optimization */
    return a + b + c + d + e + f + g + h + i + j + 
           k + l + m + n + o + p + q + r + s + t +
           u + v + w + x + y + z;
}

/* Function 2: Floating-point array processing with loops */
float float_array_processing(int size, float seed) {
    /* Many local variables for register pressure */
    float a = seed, b = seed * 1.1f, c = seed * 1.2f, d = seed * 1.3f;
    float e = seed * 1.4f, f = seed * 1.5f, g = seed * 1.6f, h = seed * 1.7f;
    float i = seed * 1.8f, j = seed * 1.9f, k = seed * 2.0f, l = seed * 2.1f;
    float arr[32];
    float result = 0.0f;
    
    /* Initialize array with dependencies */
    for (int idx = 0; idx < 32; idx++) {
        /* Complex address calculation with dependencies */
        int idx2 = idx * 2;
        int idx3 = idx2 + 1;
        
        /* Memory accesses with true dependencies */
        if (idx > 0) {
            arr[idx] = arr[idx-1] * a + b;
            a = arr[idx] * 0.5f;  /* Update a for next iteration */
        } else {
            arr[idx] = seed;
        }
        
        /* More operations in the loop body */
        b = b * 1.01f;
        c = c + sinf(d);
        d = d * 0.99f;
        
        /* Conditional inside loop creates control flow */
        if (idx % 3 == 0) {
            e = e * f;
            f = f + g;
        } else if (idx % 3 == 1) {
            g = g / h;
            h = h - i;
        } else {
            i = i * j;
            j = j + k;
        }
        
        /* Use inline assembly to prevent reordering */
        asm volatile("" : "+r"(idx), "+r"(idx2));
    }
    
    /* Process array with more dependencies */
    for (int idx = 0; idx < 31; idx++) {
        arr[idx] = arr[idx] + arr[idx+1] * l;
        l = l * 0.95f;
        
        /* More floating point operations */
        k = cosf(arr[idx]);
        result += k * idx;
    }
    
    /* Final computation using many variables */
    return result + a + b + c + d + e + f + g + h + i + j + k + l;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_operations(long seed) {
    /* Declare many variables of different types */
    int i1 = seed, i2 = seed + 1, i3 = seed + 2, i4 = seed + 3;
    long l1 = seed * 2, l2 = seed * 3, l3 = seed * 4, l4 = seed * 5;
    float f1 = seed * 1.5f, f2 = seed * 2.5f, f3 = seed * 3.5f, f4 = seed * 4.5f;
    double d1 = seed * 1.1, d2 = seed * 1.2, d3 = seed * 1.3, d4 = seed * 1.4;
    char c1 = seed & 0xFF, c2 = (seed >> 8) & 0xFF;
    short s1 = seed & 0xFFFF, s2 = (seed >> 16) & 0xFFFF;
    
    /* Complex control flow with nested conditionals */
    if (i1 > 100) {
        /* Block A: Integer operations */
        i2 = i1 * i3;
        i3 = i2 / i4;
        i4 = i3 + i1;
        
        /* Type conversions create additional operations */
        f1 = (float)i2 * 1.5f;
        d1 = (double)i3 * 2.5;
        
        if (f1 > 50.0f) {
            /* Nested block */
            f2 = f1 * f3;
            f3 = f2 / f4;
            d2 = d1 + d3;
        }
    } else {
        /* Block B: Different operations */
        l1 = l2 * l3;
        l2 = l1 >> 2;
        l3 = l2 | l4;
        
        d3 = d4 * 3.14;
        d4 = sin(d3);
        
        /* Memory operations with pointer arithmetic */
        char buffer[64];
        for (int i = 0; i < 63; i++) {
            buffer[i] = (c1 + i) ^ c2;
            c1 = buffer[i] & 0x7F;
        }
        buffer[63] = '\0';
    }
    
    /* Switch statement for more control flow */
    switch (seed % 5) {
        case 0:
            d1 = d2 * d3;
            f1 = f2 + f3;
            i1 = i2 - i3;
            break;
        case 1:
            d2 = d3 / d4;
            f2 = f3 * f4;
            i2 = i3 | i4;
            break;
        case 2:
            d3 = d4 + d1;
            f3 = f4 - f1;
            i3 = i4 ^ i1;
            break;
        case 3:
            d4 = d1 * d2;
            f4 = f1 / f2;
            i4 = i1 & i2;
            break;
        case 4:
            d1 = sqrt(d2);
            f1 = fabs(f3);
            i1 = abs(i4);
            break;
    }
    
    /* Use all variables to prevent dead code elimination */
    asm volatile("" : "+r"(i1), "+r"(i2), "+r"(i3), "+r"(i4),
                      "+r"(l1), "+r"(l2), "+r"(l3), "+r"(l4));
    
    /* Final computation */
    double result = (double)i1 + (double)i2 + (double)i3 + (double)i4 +
                   (double)l1 + (double)l2 + (double)l3 + (double)l4 +
                   (double)f1 + (double)f2 + (double)f3 + (double)f4 +
                   d1 + d2 + d3 + d4 + (double)c1 + (double)c2 +
                   (double)s1 + (double)s2;
    
    return result;
}

/* Function 4: Complex loop with switch inside */
unsigned long complex_loop_switch(int iterations, unsigned long seed) {
    unsigned long a = seed, b = seed ^ 0xAAAAAAAA, c = seed ^ 0x55555555;
    unsigned long d = seed ^ 0xCCCCCCCC, e = seed ^ 0x33333333;
    unsigned long f = seed ^ 0xF0F0F0F0, g = seed ^ 0x0F0F0F0F;
    unsigned long result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Switch inside loop - creates complex control flow */
        switch (i % 7) {
            case 0:
                a = b + c;
                b = c * d;
                c = d ^ e;
                break;
            case 1:
                d = e << 3;
                e = f >> 2;
                f = g | a;
                break;
            case 2:
                g = a & b;
                a = b % (c + 1);
                b = c / (d + 1);
                break;
            case 3:
                c = d - e;
                d = e + f;
                e = f * g;
                break;
            case 4:
                f = g ^ a;
                g = a << 1;
                a = b >> 1;
                break;
            case 5:
                b = c | d;
                c = d & e;
                d = e + f;
                break;
            case 6:
                e = f * g;
                f = g / (a + 1);
                g = a % (b + 1);
                break;
        }
        
        /* Additional operations with dependencies */
        a = a ^ (b << (i % 16));
        b = b + (c >> (i % 16));
        c = c * (d + i);
        d = d - (e * i);
        
        /* Inline assembly to create barriers */
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c));
        
        /* Accumulate result */
        result += a + b + c + d + e + f + g;
    }
    
    return result;
}

/* Main function to drive execution */
int main(int argc, char *argv[]) {
    /* Use volatile inputs to prevent constant propagation */
    volatile int input1 = 100;
    volatile float input2 = 3.14159f;
    volatile long input3 = 123456789;
    volatile int input4 = 50;
    
    /* Read from command line if available to make values truly dynamic */
    if (argc > 1) input1 = atoi(argv[1]);
    if (argc > 2) input2 = atof(argv[2]);
    if (argc > 3) input3 = atol(argv[3]);
    if (argc > 4) input4 = atoi(argv[4]);
    
    /* Call all complex functions */
    int result1 = int_heavy_computation(input1);
    float result2 = float_array_processing(input4, input2);
    double result3 = mixed_operations(input3);
    unsigned long result4 = complex_loop_switch(input4, input3);
    
    /* Aggregate results into volatile sink to prevent optimization */
    volatile double final_result = 0.0;
    final_result += (double)result1;
    final_result += (double)result2;
    final_result += result3;
    final_result += (double)result4;
    
    /* Print checksum to ensure all computations are used */
    printf("Result checksum: %f\n", final_result);
    
    return 0;
}
