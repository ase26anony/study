#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_var1 = 42;
volatile int g_var2 = 73;
int g_array[256];

/* Helper functions marked noinline to prevent inlining */
__attribute__((noinline, optimize("O3")))
int complex_chain(int seed) {
    volatile int barrier;
    int a = seed, b = seed * 2, c = seed + 1;
    int d = 0, e = 0, f = 0, g = 0, h = 0;
    
    /* Long chain of dependent operations */
    for (int i = 0; i < 32; i++) {
        a += b * c;
        b ^= a + i;
        c = (c << 3) | (c >> 29);
        
        /* Memory barrier to split scheduling regions */
        if (__builtin_expect_with_probability(i == 16, 0, 0.3)) {
            asm volatile("" : : : "memory");
            barrier = g_var1;
        }
        
        d += a * b;
        e ^= c + d;
        f = (f + e) * 3;
        g = g * 5 + f;
        h = h ^ (g << i);
    }
    
    /* Data-dependent exit condition */
    int i = 0;
    while (__builtin_expect_with_probability(g_array[i] != 0, 0, 0.2)) {
        h += g_array[i];
        i = (i + 1) & 255;
        if (i == 0) break;
    }
    
    return a + b + c + d + e + f + g + h;
}

__attribute__((noinline, optimize("O3")))
int nested_loops(int start) {
    int x1 = start, x2 = start + 1, x3 = start + 2;
    int x4 = start + 3, x5 = start + 4, x6 = start + 5;
    int x7 = start + 6, x8 = start + 7, x9 = start + 8;
    int x10 = start + 9, x11 = start + 10, x12 = start + 11;
    
    for (int outer = 0; outer < 8; outer++) {
        /* Software pipelining style computation */
        for (int inner = 0; inner < 4; inner++) {
            x1 = x1 * 3 + inner;
            x2 = x2 ^ (x1 >> 2);
            x3 = x3 + x2 * 7;
            x4 = x4 - x3;
            x5 = x5 * 11 + x4;
            x6 = x6 ^ x5;
            
            /* Memory operation with uncertain latency */
            if (__builtin_expect_with_probability((inner & 1) == 0, 1, 0.7)) {
                volatile int* ptr = (volatile int*)&g_var2;
                x7 += *ptr;
            }
            
            x8 = x8 + x7 * 13;
            x9 = x9 ^ (x8 << 1);
            x10 = x10 * 17 + x9;
            x11 = x11 - x10;
            x12 = x12 ^ x11;
        }
        
        /* Irregular control flow with goto */
        if (__builtin_expect_with_probability(outer == 3, 0, 0.1)) {
            goto restart_point;
        }
        
        continue;
        
        restart_point:
        x1 = (x1 + 1) & 0xFFF;
    }
    
    return x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10 + x11 + x12;
}

__attribute__((noinline, optimize("O3")))
int switch_computation(int val) {
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
    int r6 = 0, r7 = 0, r8 = 0, r9 = 0, r10 = 0;
    
    /* Large switch statement with different computation patterns */
    switch (val & 0xF) {
        case 0:
            r1 = val * 2;
            r2 = val + 1;
            for (int i = 0; i < 5; i++) r1 += r2 * i;
            break;
        case 1:
            r3 = val ^ 0xAAAA;
            r4 = val | 0x5555;
            r3 = r3 * r4;
            break;
        case 2:
            r5 = val << 3;
            r6 = val >> 2;
            r5 = r5 | r6;
            break;
        case 3:
            r7 = val * 7;
            r8 = val / 3;
            r7 = r7 - r8;
            break;
        case 4:
            r9 = val + 100;
            r10 = val - 50;
            r9 = r9 * r10;
            break;
        case 5:
            r1 = val * val;
            r2 = val + val;
            r1 = r1 % 997;
            break;
        case 6:
            r3 = ~val;
            r4 = val & 0xFF;
            r3 = r3 + r4;
            break;
        case 7:
            r5 = val * 13;
            r6 = val * 17;
            r5 = r5 ^ r6;
            break;
        case 8:
            r7 = val << 1;
            r8 = val << 2;
            r7 = r7 | r8;
            break;
        case 9:
            r9 = val + 255;
            r10 = val - 128;
            r9 = r9 & r10;
            break;
        case 10:
            r1 = val * 3;
            r2 = val * 5;
            r1 = r1 + r2;
            break;
        case 11:
            r3 = val ^ 0x1234;
            r4 = val ^ 0x5678;
            r3 = r3 * r4;
            break;
        case 12:
            r5 = val + 1000;
            r6 = val - 500;
            r5 = r5 / (r6 ? r6 : 1);
            break;
        case 13:
            r7 = val << 4;
            r8 = val >> 4;
            r7 = r7 | r8;
            break;
        case 14:
            r9 = val * 19;
            r10 = val * 23;
            r9 = r9 ^ r10;
            break;
        case 15:
            r1 = val + 4096;
            r2 = val - 2048;
            r1 = r1 * r2;
            break;
    }
    
    /* Merge point with many live variables */
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
}

/* Recursive function to create return state restoration points */
__attribute__((noinline, optimize("O3")))
int recursive_compute(int n, int depth) {
    if (depth >= 4) return n;
    
    int local1 = n * 2;
    int local2 = n + 1;
    int local3 = n ^ 0xAA;
    int local4 = n << 2;
    int local5 = n >> 1;
    
    /* Chain of operations before recursion */
    for (int i = 0; i < 3; i++) {
        local1 = local1 * 3 + i;
        local2 = local2 ^ local1;
        local3 = local3 + local2 * 5;
        local4 = local4 | local3;
        local5 = local5 - local4;
    }
    
    /* Recursive call - scheduler may save state here */
    int rec_result = recursive_compute(local5, depth + 1);
    
    /* More operations after return - scheduler may restore state */
    local1 = local1 + rec_result;
    local2 = local2 * rec_result;
    local3 = local3 ^ rec_result;
    local4 = local4 - rec_result;
    local5 = local5 | rec_result;
    
    return local1 + local2 + local3 + local4 + local5;
}

__attribute__((optimize("O3")))
int main() {
    /* Initialize array with non-zero values */
    for (int i = 0; i < 256; i++) {
        g_array[i] = (i * 73) & 0xFF;
    }
    
    int result = 0;
    
    /* Kernel 1: Long chain with data-dependent break */
    result += complex_chain(1);
    
    /* Kernel 2: Nested loops with irregular control flow */
    result += nested_loops(result & 0xFF);
    
    /* Kernel 3: Large switch statement */
    for (int i = 0; i < 20; i++) {
        result += switch_computation(result + i);
    }
    
    /* Kernel 4: Recursive computation */
    result += recursive_compute(result, 0);
    
    /* Kernel 5: Manual software pipelining attempt */
    {
        int a = 1, b = 2, c = 3, d = 4, e = 5;
        int f = 6, g = 7, h = 8, i = 9, j = 10;
        int k = 11, l = 12, m = 13, n = 14, o = 15;
        int p = 16, q = 17, r = 18, s = 19, t = 20;
        
        do {
            a = a * 3 + b;
            b = b ^ c;
            c = c + d * 7;
            d = d - e;
            e = e | f;
            
            /* Scheduling barrier */
            asm volatile("" : : : "memory");
            
            f = f * 11 + g;
            g = g ^ h;
            h = h + i * 13;
            i = i - j;
            j = j | k;
            
            if (__builtin_expect_with_probability(a > 1000, 0, 0.05)) {
                break;
            }
            
            k = k * 17 + l;
            l = l ^ m;
            m = m + n * 19;
            n = n - o;
            o = o | p;
            
            p = p * 23 + q;
            q = q ^ r;
            r = r + s * 29;
            s = s - t;
            t = t | a;
            
        } while (0);
        
        result += a + b + c + d + e + f + g + h + i + j + 
                 k + l + m + n + o + p + q + r + s + t;
    }
    
    printf("Result checksum: %d\n", result);
    return 0;
}
