#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_var1 = 42;
volatile int g_var2 = 73;
int g_array[256] = {0};

/* Helper functions marked noinline to prevent inlining */
__attribute__((noinline, optimize("O3")))
int complex_chain(int seed) {
    volatile int barrier;
    int a = seed, b = seed * 2, c = seed + 1, d = seed ^ 0x55;
    int e = seed >> 2, f = seed << 1, g = seed % 17, h = seed | 0xFF;
    int i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;
    
    /* Create register pressure with many variables */
    i = a + b; j = c - d; k = e * f; l = g ^ h;
    m = i & j; n = k | l; o = m + n; p = o * 2;
    q = p ^ seed; r = q << 3; s = r >> 1; t = s + 12345;
    u = t * 6789; v = u % 5432; w = v ^ 0xABCD; x = w + 9999;
    y = x * 777; z = y % 8888;
    
    /* Memory barrier to split scheduling region */
    asm volatile("" : : : "memory");
    
    /* Long dependent chain */
    for (int idx = 0; idx < 8; idx++) {
        a += b; b ^= c; c *= d; d += e;
        e ^= f; f *= g; g += h; h ^= i;
        i += j; j ^= k; k *= l; l += m;
        m ^= n; n *= o; o += p; p ^= q;
        q += r; r ^= s; s *= t; t += u;
        u ^= v; v *= w; w += x; x ^= y;
        y += z; z ^= a;
        
        /* Data-dependent break to create uncertainty */
        if (__builtin_expect_with_probability(a > 1000000, 0, 0.3)) {
            barrier = g_var1;
            break;
        }
    }
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    /* Final computation with all variables */
    return (a + b + c + d + e + f + g + h + i + j + 
            k + l + m + n + o + p + q + r + s + t +
            u + v + w + x + y + z) & 0xFFFF;
}

__attribute__((noinline, optimize("O3")))
int switch_computation(int selector) {
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0;
    int r7 = 0, r8 = 0, r9 = 0, r10 = 0, r11 = 0, r12 = 0;
    int r13 = 0, r14 = 0, r15 = 0, r16 = 0, r17 = 0, r18 = 0;
    
    /* Large switch to create complex control flow */
    switch (selector & 0xF) {
        case 0:
            r1 = g_var1 * 2; r2 = g_var2 + 1;
            for (int i = 0; i < 5; i++) r1 += r2 * i;
            break;
        case 1:
            r3 = g_var2 ^ 0xAA; r4 = g_var1 | 0x55;
            r3 = (r3 * r4) % 1000;
            break;
        case 2:
            r5 = g_var1 + g_var2; r6 = g_var1 - g_var2;
            r5 = r5 * r6;
            break;
        case 3:
            r7 = ~g_var1; r8 = ~g_var2;
            r7 = r7 & r8;
            break;
        case 4:
            r9 = g_var1 << 2; r10 = g_var2 >> 1;
            r9 = r9 | r10;
            break;
        case 5:
            r11 = g_var1 % 13; r12 = g_var2 % 17;
            r11 = r11 * r12;
            break;
        case 6:
            r13 = g_var1 ^ g_var2; r14 = g_var1 & g_var2;
            r13 = r13 - r14;
            break;
        case 7:
            r15 = g_var1 * 3; r16 = g_var2 * 5;
            r15 = r15 + r16;
            break;
        case 8:
            r17 = g_var1 | g_var2; r18 = g_var1 ^ g_var2;
            r17 = r17 & r18;
            break;
        case 9:
            r1 = g_var1 + 100; r2 = g_var2 - 50;
            r1 = r1 * r2;
            break;
        case 10:
            r3 = g_var1 << 1; r4 = g_var2 >> 2;
            r3 = r3 | r4;
            break;
        case 11:
            r5 = g_var1 % 7; r6 = g_var2 % 11;
            r5 = r5 ^ r6;
            break;
        case 12:
            r7 = ~g_var2; r8 = ~g_var1;
            r7 = r7 | r8;
            break;
        case 13:
            r9 = g_var1 * g_var2; r10 = g_var1 + g_var2;
            r9 = r9 - r10;
            break;
        case 14:
            r11 = g_var1 & 0xF0; r12 = g_var2 & 0x0F;
            r11 = r11 | r12;
            break;
        case 15:
            r13 = g_var1 ^ 0xFF; r14 = g_var2 ^ 0xAA;
            r13 = r13 + r14;
            break;
    }
    
    /* Merge point with all variables */
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 +
           r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18;
}

__attribute__((noinline, optimize("O3")))
int nested_loops(int iterations) {
    int sum = 0;
    volatile int* ptr1 = (volatile int*)&g_var1;
    volatile int* ptr2 = (volatile int*)&g_var2;
    
    /* Software pipelining style loop */
    for (int i = 0; i < iterations; i++) {
        int acc = i;
        
        /* Inner loop with memory operations */
        for (int j = 0; j < 8; j++) {
            /* Pointer aliasing creates uncertainty */
            int val1 = *ptr1;
            int val2 = *ptr2;
            
            acc += val1 * j;
            acc ^= val2 + j;
            
            /* Data-dependent continue */
            if (__builtin_expect_with_probability((acc & 1) == 0, 1, 0.7)) {
                acc += g_array[j];
            } else {
                acc -= g_array[255 - j];
            }
            
            /* Memory clobber */
            asm volatile("" : : : "memory");
        }
        
        sum += acc;
        
        /* Irregular control flow with goto */
        if (__builtin_expect_with_probability(sum > 10000, 0, 0.2)) {
            i--;  /* Create potential for scheduler state save */
            if (i < 0) break;
        }
    }
    
    return sum;
}

__attribute__((noinline, optimize("O3")))
int recursive_compute(int n, int depth) {
    if (depth >= 4 || n <= 0) {
        return n;
    }
    
    int a = recursive_compute(n - 1, depth + 1);
    int b = recursive_compute(n - 2, depth + 1);
    
    /* Complex computation at return point */
    int result = (a * 3 + b * 7) ^ 0x1234;
    
    /* Memory operation before return */
    volatile int mem = g_var1;
    result += mem;
    
    return result;
}

__attribute__((optimize("O3")))
int main() {
    int total = 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 3 + 1;
    }
    
    /* Kernel 1: Long dependent chain with branch probability hints */
    for (int i = 0; i < 50; i++) {
        total += complex_chain(i + total);
        
        /* Do-while with break to create control edges */
        int counter = 0;
        do {
            if (__builtin_expect_with_probability(counter > 5, 0, 0.4)) {
                total += g_var2;
                break;
            }
            counter++;
            total ^= 1;
        } while (counter < 10);
    }
    
    /* Kernel 2: Switch-based computation */
    for (int i = 0; i < 100; i++) {
        total += switch_computation(total + i);
        
        /* Loop with irregular exit */
        int j = 0;
        while (g_array[j] != 0) {
            total += g_array[j];
            j = (j + 1) & 0xFF;
            if (j == 0) break;
        }
    }
    
    /* Kernel 3: Nested loops with software pipelining style */
    total += nested_loops(20);
    
    /* Kernel 4: Recursive computation */
    total += recursive_compute(8, 0);
    
    /* Final aggregation with memory barrier */
    asm volatile("" : : : "memory");
    
    printf("Result checksum: %d\n", total);
    return total & 0xFF;
}
