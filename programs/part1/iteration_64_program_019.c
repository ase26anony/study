#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_var1 = 0;
volatile int g_var2 = 0;
int * volatile g_ptr1 = &g_var1;
int * volatile g_ptr2 = &g_var2;

/* Helper functions to prevent inlining */
__attribute__((noinline)) 
__attribute__((optimize("O3")))
int compute_chain(int seed) {
    volatile int barrier = 0;
    int a = seed, b = seed * 2, c = seed * 3, d = seed * 4;
    int e = seed * 5, f = seed * 6, g = seed * 7, h = seed * 8;
    int i = seed * 9, j = seed * 10, k = seed * 11, l = seed * 12;
    int m = seed * 13, n = seed * 14, o = seed * 15, p = seed * 16;
    
    /* Long chain of dependent operations */
    a += b ^ c;
    b += c ^ d;
    c += d ^ e;
    d += e ^ f;
    e += f ^ g;
    f += g ^ h;
    g += h ^ i;
    h += i ^ j;
    i += j ^ k;
    j += k ^ l;
    k += l ^ m;
    l += m ^ n;
    m += n ^ o;
    n += o ^ p;
    o += p ^ a;
    p += a ^ b;
    
    /* Memory barrier to split scheduling region */
    asm volatile("" : : : "memory");
    
    /* More dependent operations */
    a *= b + 1;
    b *= c + 2;
    c *= d + 3;
    d *= e + 4;
    e *= f + 5;
    f *= g + 6;
    
    /* Data-dependent loop with unpredictable exit */
    int counter = 0;
    while (__builtin_expect_with_probability(counter < barrier, 0, 0.3)) {
        a ^= b;
        b ^= c;
        c ^= d;
        counter++;
    }
    
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
}

__attribute__((noinline))
__attribute__((optimize("O3")))
int nested_loop_helper(int start, int iterations) {
    int x1 = start, x2 = start + 1, x3 = start + 2, x4 = start + 3;
    int x5 = start + 4, x6 = start + 5, x7 = start + 6, x8 = start + 7;
    
    /* Inner loop with software pipelining potential */
    for (int i = 0; i < iterations; i++) {
        x1 = (x1 * 1103515245 + 12345) & 0x7fffffff;
        x2 = (x2 * 1103515245 + 12345) & 0x7fffffff;
        x3 = (x3 * 1103515245 + 12345) & 0x7fffffff;
        x4 = (x4 * 1103515245 + 12345) & 0x7fffffff;
        
        /* Conditional break creates control flow complexity */
        if (__builtin_expect((x1 & 0xFF) == 0, 0)) {
            asm volatile("" : : : "memory");
            break;
        }
        
        x5 = (x5 * 1103515245 + 12345) & 0x7fffffff;
        x6 = (x6 * 1103515245 + 12345) & 0x7fffffff;
        x7 = (x7 * 1103515245 + 12345) & 0x7fffffff;
        x8 = (x8 * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8;
}

__attribute__((noinline))
__attribute__((optimize("O3")))
int recursive_compute(int depth, int val) {
    if (depth <= 0) {
        return val;
    }
    
    int local1 = val * 3;
    int local2 = val * 5;
    int local3 = val * 7;
    int local4 = val * 11;
    
    /* Pointer aliasing creates uncertainty */
    *g_ptr1 = local1;
    local2 += *g_ptr2;
    
    /* Recursive call with state that may need saving */
    int result = recursive_compute(depth - 1, local2);
    
    /* Complex computation after recursion */
    local3 ^= result;
    local4 += result * 2;
    
    asm volatile("" : : : "memory");
    
    return local3 + local4 + recursive_compute(depth - 2, local1);
}

__attribute__((noinline))
__attribute__((optimize("O3")))
int complex_switch(int selector, int base) {
    int r1 = base, r2 = base + 1, r3 = base + 2, r4 = base + 3;
    int r5 = base + 4, r6 = base + 5, r7 = base + 6, r8 = base + 7;
    int r9 = base + 8, r10 = base + 9, r11 = base + 10, r12 = base + 11;
    
    /* Large switch with different computation patterns */
    switch (selector & 0xF) {
        case 0:
            r1 = r1 * r2 + r3;
            r2 = r2 * r3 + r4;
            r3 = r3 * r4 + r5;
            break;
        case 1:
            r4 = r4 ^ r5 ^ r6;
            r5 = r5 ^ r6 ^ r7;
            r6 = r6 ^ r7 ^ r8;
            break;
        case 2:
            r7 = (r7 << 3) | (r8 >> 5);
            r8 = (r8 << 3) | (r9 >> 5);
            r9 = (r9 << 3) | (r10 >> 5);
            break;
        case 3:
            r10 = r10 + r11 - r12;
            r11 = r11 + r12 - r1;
            r12 = r12 + r1 - r2;
            break;
        case 4:
            r1 = r1 * 3 + 7;
            r2 = r2 * 5 + 11;
            r3 = r3 * 7 + 13;
            break;
        case 5:
            r4 = r4 / 2 + r5;
            r5 = r5 / 2 + r6;
            r6 = r6 / 2 + r7;
            break;
        case 6:
            r7 = r7 & r8 | r9;
            r8 = r8 & r9 | r10;
            r9 = r9 & r10 | r11;
            break;
        case 7:
            r10 = r10 * r11 % 997;
            r11 = r11 * r12 % 997;
            r12 = r12 * r1 % 997;
            break;
        case 8:
            r1 = ~r1 + r2;
            r2 = ~r2 + r3;
            r3 = ~r3 + r4;
            break;
        case 9:
            r4 = r4 * r4 + r5;
            r5 = r5 * r5 + r6;
            r6 = r6 * r6 + r7;
            break;
        case 10:
            r7 = r7 << (r8 & 3);
            r8 = r8 << (r9 & 3);
            r9 = r9 << (r10 & 3);
            break;
        case 11:
            r10 = r10 | r11 | r12;
            r11 = r11 | r12 | r1;
            r12 = r12 | r1 | r2;
            break;
        case 12:
            r1 = r1 - r2 + r3;
            r2 = r2 - r3 + r4;
            r3 = r3 - r4 + r5;
            break;
        case 13:
            r4 = r4 * 2 + r5 * 3;
            r5 = r5 * 2 + r6 * 3;
            r6 = r6 * 2 + r7 * 3;
            break;
        case 14:
            r7 = r7 ^ 0xAAAAAAAA;
            r8 = r8 ^ 0x55555555;
            r9 = r9 ^ 0x33333333;
            break;
        default: /* case 15 */
            r10 = r10 + 1;
            r11 = r11 + 2;
            r12 = r12 + 3;
            /* goto creates irregular control flow */
            if (r10 > 1000) {
                goto merge_point;
            }
            break;
    }
    
merge_point:
    /* Merge all results */
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12;
}

__attribute__((optimize("O3")))
int main() {
    int result = 0;
    
    /* Kernel 1: Long dependent chain with data-dependent exit */
    result += compute_chain(42);
    
    /* Kernel 2: Nested loops with potential software pipelining */
    for (int i = 0; i < 100; i++) {
        result += nested_loop_helper(i, 50);
    }
    
    /* Kernel 3: Recursive computation */
    result += recursive_compute(4, result & 0xFF);
    
    /* Kernel 4: Complex switch statement */
    for (int i = 0; i < 200; i++) {
        result += complex_switch(i, result & 0xFF);
    }
    
    /* Kernel 5: Manual software pipelining pattern */
    {
        int a = 1, b = 2, c = 3, d = 4, e = 5;
        int f = 6, g = 7, h = 8, i = 9, j = 10;
        
        /* do-while with internal break */
        int iter = 0;
        do {
            if (__builtin_expect_with_probability((iter & 0x7) == 0, 0, 0.2)) {
                asm volatile("" : : : "memory");
                break;
            }
            
            a = a * b + c;
            b = b * c + d;
            c = c * d + e;
            d = d * e + f;
            e = e * f + g;
            f = f * g + h;
            g = g * h + i;
            h = h * i + j;
            i = i * j + a;
            j = j * a + b;
            
            /* Memory operation with volatile */
            volatile int* mem = &g_var1;
            *mem = a;
            b += *mem;
            
            iter++;
        } while (__builtin_expect_with_probability(iter < 100, 1, 0.8));
        
        result += a + b + c + d + e + f + g + h + i + j;
    }
    
    /* Ensure result is used */
    printf("Result checksum: %d\n", result);
    
    return 0;
}
