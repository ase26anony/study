#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_var1 = 1;
volatile int g_var2 = 2;
int g_array[256];

/* Helper functions with noinline to prevent inlining */
__attribute__((noinline, optimize("O3")))
int compute_chain(int seed) {
    volatile int barrier = 0;
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    int i = seed + 8, j = seed + 9, k = seed + 10, l = seed + 11;
    int m = seed + 12, n = seed + 13, o = seed + 14, p = seed + 15;
    int q = seed + 16, r = seed + 17, s = seed + 18, t = seed + 19;
    
    /* Long chain of dependent operations */
    a += b * c;
    b ^= d << 2;
    c = (c + e) | f;
    d *= g + h;
    e = (e ^ i) + j;
    f = f * k - l;
    g += m * n;
    h = h ^ o ^ p;
    i = i * q + r;
    j = j - s * t;
    
    /* Memory barrier to split scheduling region */
    asm volatile("" : : : "memory");
    
    /* More dependent operations */
    k = (a + b) * (c - d);
    l = (e ^ f) + (g & h);
    m = i * j - k;
    n = l + m + n;
    o = o * p / (q + 1);
    p = r ^ s ^ t;
    
    /* Data-dependent loop with unpredictable exit */
    int counter = 0;
    while (__builtin_expect_with_probability(counter < barrier, 0, 0.3)) {
        counter++;
        a += counter;
    }
    
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p + q + r + s + t;
}

__attribute__((noinline, optimize("O3")))
int recursive_compute(int depth, int val) {
    if (__builtin_expect(depth <= 0, 0)) {
        return val;
    }
    
    int local1 = val * 2;
    int local2 = val + g_var1;  /* Volatile read creates uncertainty */
    int local3 = local1 ^ local2;
    int local4 = local3 * g_var2;  /* Another volatile read */
    
    /* Create register pressure */
    int t1 = local1 + local2;
    int t2 = local3 - local4;
    int t3 = t1 * t2;
    int t4 = t1 ^ t2 ^ t3;
    int t5 = t4 << (depth & 3);
    
    /* Recursive call */
    int result = recursive_compute(depth - 1, t5);
    
    /* Complex computation after recursion */
    result = (result * 1103515245 + 12345) & 0x7fffffff;
    
    return result;
}

__attribute__((noinline, optimize("O3")))
int switch_computation(int selector) {
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    /* Switch with many cases - creates complex control flow */
    switch (selector & 0xF) {
        case 0:
            v1 += v2 * v3;
            v4 = v5 ^ v6;
            v7 = v8 - v9;
            break;
        case 1:
            v2 = v3 * v4 + v5;
            v6 ^= v7;
            v8 = v9 << 2;
            break;
        case 2:
            v3 = v4 + v5 * v6;
            v7 = v8 ^ v9;
            v10 = v11 - v12;
            break;
        case 3:
            v4 *= v5 + v6;
            v7 = v8 | v9;
            v10 = v11 ^ v12;
            break;
        case 4:
            v5 = v6 - v7 * v8;
            v9 = v10 + v11;
            v12 = v13 ^ v14;
            break;
        case 5:
            v6 = v7 * v8 ^ v9;
            v10 = v11 + v12;
            v13 = v14 - v15;
            break;
        case 6:
            v7 += v8 * v9;
            v10 = v11 ^ v12;
            v13 = v14 | v15;
            break;
        case 7:
            v8 = v9 - v10 * v11;
            v12 = v13 + v14;
            v15 = v16 ^ v17;
            break;
        case 8:
            v9 *= v10 + v11;
            v12 = v13 | v14;
            v15 = v16 - v17;
            break;
        case 9:
            v10 = v11 ^ v12 * v13;
            v14 = v15 + v16;
            v17 = v18 ^ v19;
            break;
        case 10:
            v11 += v12 - v13;
            v14 = v15 * v16;
            v17 = v18 | v19;
            break;
        case 11:
            v12 = v13 ^ v14 * v15;
            v16 = v17 + v18;
            v19 = v20 - v1;
            break;
        case 12:
            v13 *= v14 + v15;
            v16 = v17 | v18;
            v19 = v20 ^ v1;
            break;
        case 13:
            v14 = v15 - v16 * v17;
            v18 = v19 + v20;
            v1 = v2 ^ v3;
            break;
        case 14:
            v15 += v16 * v17;
            v18 = v19 ^ v20;
            v1 = v2 | v3;
            break;
        default:  /* case 15 */
            v16 = v17 ^ v18 * v19;
            v20 = v1 + v2;
            v3 = v4 - v5;
            break;
    }
    
    /* Merge point with many live variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
}

__attribute__((noinline, optimize("O3")))
int pipelined_loop(int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Inner computation with data-dependent break */
        int inner_sum = 0;
        int j = 0;
        
        do {
            if (__builtin_expect_with_probability((i ^ j) == 0xFF, 0, 0.1)) {
                /* Unlikely break - scheduler may save state */
                asm volatile("" : : : "memory");
                break;
            }
            
            inner_sum += g_array[(i + j) & 0xFF] * j;
            j++;
            
            /* Small inner loop with register pressure */
            int t1 = inner_sum * i;
            int t2 = t1 ^ j;
            int t3 = t2 + g_var1;  /* Volatile read */
            inner_sum = t3 & 0xFFFF;
            
        } while (j < 8);
        
        sum += inner_sum;
        
        /* Irregular control flow with goto */
        if (__builtin_expect((i & 0x3F) == 0, 0)) {
            /* Jump back creates non-trivial CFG */
            i += 2;
            if (i < iterations) {
                goto skip_adjust;
            }
        }
        
        i -= 1;
    skip_adjust:
        asm volatile("" : : : "memory");
    }
    
    return sum;
}

int main() {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 1103515245 + 12345;
    }
    
    int result = 0;
    
    /* Kernel 1: Long chain with data-dependent exit */
    result ^= compute_chain(42);
    
    /* Kernel 2: Switch with many cases */
    for (int i = 0; i < 100; i++) {
        result += switch_computation(result + i);
    }
    
    /* Kernel 3: Recursive computation */
    result ^= recursive_compute(4, result);
    
    /* Kernel 4: Pipelined-style loop */
    result += pipelined_loop(1000);
    
    /* Additional complex loop with pointer aliasing */
    int *ptr1 = &g_var1;
    int *ptr2 = (int*)((char*)&g_var2);
    
    for (int i = 0; i < 500; i++) {
        /* Access through different pointers creates aliasing uncertainty */
        int val1 = *ptr1;
        int val2 = *ptr2;
        
        /* Dependent chain */
        val1 = val1 * 3 + val2;
        val2 = val1 ^ (val2 << 1);
        
        /* Store back - volatile ensures side-effect */
        g_var1 = val1;
        g_var2 = val2;
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
        
        result += val1 + val2;
    }
    
    /* Final checksum */
    printf("Result: %d\n", result);
    
    return 0;
}
