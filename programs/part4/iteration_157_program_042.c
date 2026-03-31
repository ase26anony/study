/* test_scheduler_context.c
 * A program designed to trigger GCC's instruction scheduler context allocation
 * and cleanup, specifically covering the free_sched_context block in haifa-sched.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Function 1: Integer-heavy computation with many serial dependencies */
int integer_computation(int seed) {
    /* Create register pressure with many local variables */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af;
    
    /* Initialize with seed to prevent constant propagation */
    a = seed;
    b = seed * 2;
    c = seed + 3;
    d = seed - 4;
    
    /* Long chain of true data dependencies (RAW) */
    e = a + b;
    f = e * c;
    g = f - d;
    h = g / (seed + 1);
    i = h << 2;
    j = i | 0xFF;
    k = j & 0x0F;
    l = k ^ 0xAA;
    m = l >> 1;
    n = m % 17;
    
    /* Anti-dependencies (WAR) and output dependencies (WAW) */
    o = n + 1;      /* WAR: n read before potential write */
    n = o * 2;      /* WAW: n written again */
    p = n - o;      /* RAW: uses both n and o */
    
    /* More operations with mixed dependencies */
    q = p + seed;
    r = q * q;
    s = r / (seed + 2);
    t = s - q;
    u = t | r;
    v = u & s;
    w = v ^ t;
    x = w << 3;
    y = x >> 1;
    z = y % 19;
    
    /* Use inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(z));
    
    aa = z + a;
    ab = aa * b;
    ac = ab - c;
    ad = ac / d;
    ae = ad | e;
    af = ae & f;
    
    /* Control flow to create multiple basic blocks */
    if (af > 1000) {
        /* Branch with its own dependency chain */
        int ba = af * 2;
        int bb = ba - seed;
        int bc = bb / 3;
        af = bc + af;
    } else {
        /* Alternative branch with different operations */
        int ca = af + 500;
        int cb = ca * ca;
        int cc = cb % 256;
        af = cc - af;
    }
    
    /* Final computation using many variables */
    return (a + b + c + d + e + f + g + h + i + j + k + l + m +
            n + o + p + q + r + s + t + u + v + w + x + y + z +
            aa + ab + ac + ad + ae + af) & 0xFFFF;
}

/* Function 2: Floating-point array processing with loops */
float floating_point_computation(float seed) {
    /* Create many local variables for register pressure */
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    float f11, f12, f13, f14, f15, f16, f17, f18, f19, f20;
    
    /* Initialize with seed-dependent values */
    f1 = seed;
    f2 = seed * 1.1f;
    f3 = seed + 2.2f;
    f4 = seed - 3.3f;
    f5 = seed / 4.4f;
    
    /* Mixed integer/float operations */
    int i1 = (int)seed;
    int i2 = i1 * 2;
    
    /* Complex floating-point dependency chain */
    f6 = f1 + f2;
    f7 = f6 * f3;
    f8 = f7 - f4;
    f9 = f8 / f5;
    f10 = sinf(f9);
    
    /* More operations with control flow */
    if (f10 > 0.5f) {
        f11 = f10 * 2.0f;
        f12 = cosf(f11);
        f13 = f12 + f1;
    } else {
        f11 = f10 / 2.0f;
        f12 = tanf(f11);
        f13 = f12 - f1;
    }
    
    /* Loop with dependencies inside */
    f14 = f13;
    for (int i = 0; i < 5; i++) {
        f14 = f14 * 1.1f + (float)i;
        f15 = f14 - f13;
        f16 = f15 * f14;
        
        /* Inline assembly to prevent optimization */
        asm volatile("" : "+r"(i));
    }
    
    /* Final chain of operations */
    f17 = f14 + f13;
    f18 = f17 * f12;
    f19 = f18 / f11;
    f20 = sqrtf(fabsf(f19));
    
    return f20 + f10 + f1;
}

/* Function 3: Mixed operations with complex control flow */
long mixed_operations(long seed) {
    /* Many local variables of different types */
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    double d1, d2, d3, d4, d5;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    
    /* Initialize with seed */
    l1 = seed;
    l2 = seed * 3;
    l3 = seed + 5;
    d1 = (double)seed / 7.0;
    
    /* Switch statement to create multiple basic blocks */
    switch (seed % 4) {
        case 0:
            /* Integer-heavy block */
            i1 = (int)l1 * 2;
            i2 = i1 + 10;
            i3 = i2 - (int)l2;
            l4 = i3 * l3;
            d2 = d1 * 2.5;
            break;
        case 1:
            /* Float-heavy block */
            d2 = d1 * 3.14;
            i1 = (int)(d2 * 100);
            l4 = l1 << 2;
            i3 = i1 & 0xFF;
            break;
        case 2:
            /* Mixed block */
            l4 = l1 | l2;
            d2 = d1 + (double)l4;
            i1 = (int)d2;
            i3 = i1 ^ 0xAA;
            break;
        default:
            /* Complex block */
            l4 = l1 + l2 + l3;
            d2 = sin(d1) * cos(d1);
            i1 = (int)(d2 * 1000);
            i3 = i1 % 256;
            break;
    }
    
    /* Common code with dependencies on switch results */
    l5 = l4 * 2;
    d3 = d2 + 1.0;
    i4 = i1 + i3;
    
    /* Nested if-else for more control flow */
    if (l5 > 1000) {
        if (d3 > 2.0) {
            l6 = l5 / 2;
            d4 = d3 * d3;
            i5 = i4 << 1;
        } else {
            l6 = l5 * 3;
            d4 = sqrt(d3);
            i5 = i4 >> 1;
        }
    } else {
        l6 = l5 + 500;
        d4 = log(fabs(d3) + 1.0);
        i5 = i4 | 0xF0;
    }
    
    /* Final computation using all variables */
    l7 = l6 + (long)i5;
    d5 = d4 * (double)l7;
    i6 = (int)d5;
    
    /* Inline assembly to create artificial dependency */
    asm volatile("" : "+r"(i6));
    
    l8 = l7 * (long)i6;
    i7 = i6 + (int)l8;
    l9 = l8 - (long)i7;
    i8 = i7 * 2;
    l10 = l9 | (long)i8;
    
    return l10;
}

/* Function 4: Memory access pattern with address calculations */
int memory_access_pattern(int seed, int* arr, int size) {
    /* Many local variables */
    int sum = 0;
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    /* Initialize array if provided */
    if (arr && size > 10) {
        /* Complex memory access pattern with dependencies */
        for (int i = 2; i < size - 2; i++) {
            /* RAW dependencies through memory */
            arr[i] = arr[i-1] + arr[i-2] + seed;
            
            /* Address calculation with dependencies */
            t1 = i * 2;
            t2 = t1 + seed;
            t3 = arr[i] * t2;
            
            /* More operations */
            t4 = t3 % 256;
            t5 = t4 ^ 0x55;
            t6 = t5 << 1;
            
            /* Anti-dependency (WAR) */
            t7 = t6 + 1;
            t6 = t7 * 2;  /* WAW on t6 */
            
            /* Use inline assembly */
            asm volatile("" : "+r"(i));
        }
        
        /* Process array with complex index calculations */
        for (int i = 0; i < size; i += 3) {
            int idx1 = i;
            int idx2 = (i * 3) % size;
            int idx3 = (i + 5) % size;
            
            t8 = arr[idx1];
            t9 = arr[idx2];
            t10 = arr[idx3];
            
            t11 = t8 + t9;
            t12 = t10 - t11;
            t13 = t12 * seed;
            t14 = t13 / (i + 1);
            
            sum += t14;
        }
    }
    
    /* Additional computation independent of array */
    t15 = seed * 7;
    t16 = t15 + 11;
    t17 = t16 % 19;
    t18 = t17 << 3;
    t19 = t18 >> 1;
    t20 = t19 ^ 0xFF;
    
    return sum + t20;
}

/* Main function to ensure all code is executed */
int main(int argc, char** argv) {
    /* Use volatile inputs to prevent constant propagation */
    volatile int input1;
    volatile float input2;
    volatile long input3;
    
    /* Get inputs from command line or use defaults */
    if (argc > 3) {
        input1 = atoi(argv[1]);
        input2 = atof(argv[2]);
        input3 = atol(argv[3]);
    } else {
        /* Use "random" but deterministic values */
        input1 = 12345;
        input2 = 3.14159f;
        input3 = 987654321L;
    }
    
    /* Prepare array for memory access function */
    int array[50];
    for (int i = 0; i < 50; i++) {
        array[i] = i * 3 + input1;
    }
    
    /* Call all complex functions in sequence */
    int result1 = integer_computation(input1);
    float result2 = floating_point_computation(input2);
    long result3 = mixed_operations(input3);
    int result4 = memory_access_pattern(input1, array, 50);
    
    /* Aggregate results to prevent dead code elimination */
    volatile int final_result = 0;
    final_result += result1;
    final_result += (int)result2;
    final_result += (int)(result3 & 0xFFFFFFFF);
    final_result += result4;
    
    /* Print to ensure code isn't optimized away */
    printf("Final checksum: %d\n", final_result);
    
    return 0;
}
