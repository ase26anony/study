#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_var1 = 42;
volatile int g_var2 = 73;
int g_array[256] = {0};

/* Helper functions marked noinline to create scheduling boundaries */
__attribute__((noinline, optimize("O3")))
int compute_chain(int seed) {
    volatile int mem_barrier;
    int a = seed, b = seed * 2, c = seed + 1;
    int d = 0, e = 0, f = 0, g = 0, h = 0;
    
    /* Long chain of dependent operations */
    for (int i = 0; i < 32; i++) {
        a = (a * 1103515245 + 12345) & 0x7fffffff;
        b ^= (b << 13) ^ (b >> 17) ^ (b << 5);
        c += (c * 3) | 1;
        d = a + b;
        e = c ^ d;
        f = e * 7;
        g = f - b;
        h = g + a;
        
        /* Memory barrier to split scheduling regions */
        if (i == 16) {
            asm volatile("" : : : "memory");
            mem_barrier = g_var1;
        }
        
        /* Data-dependent exit to create uncertainty */
        if (__builtin_expect_with_probability((h & 0xFF) == 0, 0, 0.1)) {
            break;
        }
    }
    
    /* Mix with global variables via pointer aliasing */
    int* p1 = (int*)&g_var1;
    int* p2 = (int*)&g_var2;
    *p1 = (*p1 + h) ^ *p2;
    *p2 = (*p2 * 3) ^ h;
    
    return a + b + c + d + e + f + g + h;
}

__attribute__((noinline, optimize("O3")))
int recursive_compute(int n, int acc) {
    if (n <= 0) return acc;
    
    int local_vars[8] = {acc, n, 0, 0, 0, 0, 0, 0};
    
    /* Create register pressure with many variables */
    int v1 = local_vars[0], v2 = local_vars[1];
    int v3 = v1 * v2, v4 = v1 ^ v2;
    int v5 = v3 + v4, v6 = v3 - v4;
    int v7 = v5 * v6, v8 = v5 ^ v6;
    int v9 = v7 + v8, v10 = v7 - v8;
    int v11 = v9 * v10, v12 = v9 ^ v10;
    int v13 = v11 + v12, v14 = v11 - v12;
    int v15 = v13 * v14, v16 = v13 ^ v14;
    
    /* Memory operation with uncertain latency */
    volatile int* volatile_ptr = &g_array[(v16 & 0xFF)];
    int mem_val = *volatile_ptr;
    
    /* Complex expression that benefits from scheduling */
    int result = ((v15 + mem_val) * 3) ^ ((v16 - mem_val) * 5);
    
    /* Recursive call - scheduler may save/restore state around call */
    return recursive_compute(n - 1, acc + result);
}

__attribute__((noinline, optimize("O3")))
int switch_complex(int selector) {
    /* Many local variables to create register pressure */
    int r0 = 0, r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
    int r6 = 0, r7 = 0, r8 = 0, r9 = 0, r10 = 0, r11 = 0;
    
    /* Switch with many cases - each creates different scheduling pattern */
    switch (selector & 0xF) {
        case 0:
            r0 = g_var1 * 2; r1 = g_var2 + 1;
            for (int i = 0; i < 8; i++) r0 += r1 * i;
            break;
        case 1:
            r2 = g_var2 ^ 0x55; r3 = g_var1 | 0xAA;
            r2 = (r2 << 3) | (r2 >> 5);
            break;
        case 2:
            r4 = recursive_compute(2, selector);
            r5 = compute_chain(selector);
            break;
        case 3:
            r6 = selector * 3; r7 = selector / 2;
            r6 = r6 ^ r7; r7 = r6 * 7;
            break;
        case 4:
            r8 = 1;
            for (int i = 1; i <= selector % 8; i++) r8 *= i;
            break;
        case 5:
            r9 = (selector << 4) | (selector >> 4);
            r10 = r9 * 9 + 1;
            break;
        case 6:
            r11 = 0;
            do {
                r11 += selector & 1;
                selector >>= 1;
                if (__builtin_expect_with_probability(r11 > 4, 0, 0.05))
                    break;
            } while (selector != 0);
            break;
        case 7:
            r0 = selector + g_var1;
            r1 = selector - g_var2;
            r2 = r0 * r1;
            break;
        case 8:
            r3 = 0;
            for (int i = 0; i < 4; i++) {
                r3 += g_array[i] * i;
                asm volatile("" : : : "memory");
            }
            break;
        case 9:
            r4 = selector;
            r5 = selector ^ 0xFFFFFFFF;
            r6 = r4 & r5;
            break;
        case 10:
            r7 = 1;
            while (__builtin_expect_with_probability(r7 < 100, 1, 0.9)) {
                r7 = r7 * 2 + 1;
            }
            break;
        case 11:
            r8 = compute_chain(selector);
            r9 = compute_chain(r8);
            break;
        case 12:
            r10 = selector;
            goto compute_label;
        case 13:
            r11 = 0;
            for (int i = 0; i < 16; i++) {
                r11 += (selector >> i) & 1;
            }
            break;
        case 14:
            r0 = g_var1; r1 = g_var2;
            r2 = r0 - r1; r3 = r0 + r1;
            break;
        case 15:
            r4 = 0;
            for (int i = 0; i < 8; i += 2) {
                r4 += g_array[i] - g_array[i + 1];
            }
            break;
        default:
            r5 = selector;
    }
    
compute_label:
    /* Merge point with complex expression using all variables */
    return r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11;
}

__attribute__((noinline, optimize("O3")))
int software_pipelined_kernel(int iterations) {
    int result = 0;
    
    /* Outer loop with inner function calls */
    for (int i = 0; i < iterations; i++) {
        /* Call to small function creates scheduling boundary */
        int temp = compute_chain(i);
        
        /* Manual software pipelining pattern */
        int stage1 = temp * 3;
        int stage2 = stage1 + g_var1;
        int stage3 = stage2 ^ g_var2;
        
        /* Memory operation between stages */
        g_array[i & 0xFF] = stage3;
        
        int stage4 = stage3 * 7;
        int stage5 = stage4 - temp;
        
        /* Another scheduling barrier */
        asm volatile("" : : : "memory");
        
        result += stage5;
        
        /* Irregular control flow with goto */
        if (__builtin_expect_with_probability((result & 0x1000) != 0, 0, 0.01)) {
            goto early_exit;
        }
    }
    
early_exit:
    return result;
}

int main() {
    int checksum = 0;
    
    /* Initialize global array with pattern */
    for (int i = 0; i < 256; i++) {
        g_array[i] = (i * 1103515245 + 12345) & 0xFF;
    }
    
    /* Kernel 1: Long chain with data-dependent exit */
    checksum += compute_chain(42);
    
    /* Kernel 2: Recursive computation */
    checksum += recursive_compute(4, 100);
    
    /* Kernel 3: Complex switch statement */
    for (int i = 0; i < 32; i++) {
        checksum += switch_complex(i);
    }
    
    /* Kernel 4: Software pipelined pattern */
    checksum += software_pipelined_kernel(64);
    
    /* Kernel 5: Mixed pattern with volatile accesses */
    {
        int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
        int g = 7, h = 8, i = 9, j = 10, k = 11, l = 12;
        int m = 13, n = 14, o = 15, p = 16, q = 17, r = 18;
        int s = 19, t = 20;
        
        /* Chain of dependent operations */
        for (int iter = 0; iter < 16; iter++) {
            a = b + c;
            b = c ^ d;
            c = d * e;
            d = e | f;
            e = f & g;
            f = g - h;
            g = h + i;
            h = i * j;
            i = j ^ k;
            j = k | l;
            k = l + m;
            l = m - n;
            m = n * o;
            n = o ^ p;
            o = p + q;
            p = q - r;
            q = r * s;
            r = s ^ t;
            s = t + a;
            t = a - b;
            
            /* Volatile read creates scheduling uncertainty */
            volatile int sync = g_var1;
            (void)sync;
            
            /* Branch with unpredictable probability */
            if (__builtin_expect_with_probability((iter & 3) == 0, 0, 0.25)) {
                asm volatile("" : : : "memory");
            }
        }
        
        checksum += a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p + q + r + s + t;
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
