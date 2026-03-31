#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_var1 = 42;
volatile int g_var2 = 73;
int g_array[256];

/* Helper functions marked noinline to prevent inlining */
__attribute__((noinline, optimize("O3")))
int compute_chain(int seed) {
    volatile int barrier;
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    int i = seed + 8, j = seed + 9, k = seed + 10, l = seed + 11;
    int m = seed + 12, n = seed + 13, o = seed + 14, p = seed + 15;
    
    /* Long chain of dependent operations */
    a += b * c;
    d ^= e | f;
    g *= h + i;
    j -= k ^ l;
    m &= n | o;
    p += a * d;
    
    /* Memory barrier to split scheduling region */
    asm volatile("" : : : "memory");
    
    /* More dependent operations */
    b += c * g;
    e ^= f | j;
    h *= i + m;
    k -= l ^ p;
    n &= o | a;
    
    /* Data-dependent loop with unpredictable exit */
    int counter = 0;
    while (__builtin_expect_with_probability(counter < 8, 1, 0.7)) {
        a += b;
        b ^= c;
        c *= d;
        d -= e;
        counter += g_array[counter & 0xF]; /* Data-dependent */
    }
    
    /* Final computation */
    barrier = g_var1; /* Volatile read creates scheduling uncertainty */
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p + barrier;
}

__attribute__((noinline, optimize("O3")))
int switch_computation(int selector, int input) {
    int r1 = input, r2 = input + 1, r3 = input + 2, r4 = input + 3;
    int r5 = input + 4, r6 = input + 5, r7 = input + 6, r8 = input + 7;
    int r9 = input + 8, r10 = input + 9, r11 = input + 10, r12 = input + 11;
    int r13 = input + 12, r14 = input + 13, r15 = input + 14, r16 = input + 15;
    
    /* Complex switch with many cases - creates merge point challenges */
    switch (selector & 0xF) {
        case 0:
            r1 += r2 * r3;
            r4 ^= r5;
            /* Fall through */
        case 1:
            r6 *= r7 + r8;
            r9 -= r10;
            break;
        case 2:
            r11 &= r12 | r13;
            r14 += r15;
            break;
        case 3:
            r16 ^= r1 + r2;
            r3 *= r4;
            break;
        case 4:
            r5 -= r6 ^ r7;
            r8 &= r9;
            break;
        case 5:
            r10 += r11 * r12;
            r13 ^= r14;
            break;
        case 6:
            r15 *= r16 + r1;
            r2 -= r3;
            break;
        case 7:
            r4 &= r5 | r6;
            r7 += r8;
            break;
        case 8:
            r9 ^= r10 + r11;
            r12 *= r13;
            break;
        case 9:
            r14 -= r15 ^ r16;
            r1 &= r2;
            break;
        case 10:
            r3 += r4 * r5;
            r6 ^= r7;
            break;
        case 11:
            r8 *= r9 + r10;
            r11 -= r12;
            break;
        case 12:
            r13 &= r14 | r15;
            r16 += r1;
            break;
        case 13:
            r2 ^= r3 + r4;
            r5 *= r6;
            break;
        case 14:
            r7 -= r8 ^ r9;
            r10 &= r11;
            break;
        default: /* case 15 */
            r12 += r13 * r14;
            r15 ^= r16;
            break;
    }
    
    /* Memory clobber as scheduling barrier */
    asm volatile("" : : : "memory");
    
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + 
           r9 + r10 + r11 + r12 + r13 + r14 + r15 + r16;
}

__attribute__((noinline, optimize("O3")))
int recursive_compute(int depth, int val) {
    int local1 = val, local2 = val * 2, local3 = val * 3;
    int local4 = val * 4, local5 = val * 5, local6 = val * 6;
    
    if (__builtin_expect_with_probability(depth > 0, 1, 0.6)) {
        /* Recursive call creates call/return boundaries */
        int rec_result = recursive_compute(depth - 1, val + 1);
        
        /* Complex computation after recursion */
        local1 += rec_result * local2;
        local3 ^= local4 | local5;
        local6 *= local1 + local3;
        
        /* Volatile write creates scheduling uncertainty */
        g_var2 = local6;
        
        return local1 + local3 + local6 + rec_result;
    }
    
    /* Base case with its own computation */
    local1 *= local2 + local3;
    local4 ^= local5 & local6;
    return local1 + local4;
}

__attribute__((noinline, optimize("O3")))
int software_pipelined_loop(int iterations) {
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int tmp1, tmp2, tmp3, tmp4;
    
    /* Manual software pipelining attempt */
    for (int i = 0; i < iterations; i++) {
        /* Phase 1: load/compute */
        tmp1 = g_array[i & 0xFF];
        tmp2 = tmp1 * i;
        
        /* Phase 2: compute with memory barrier */
        asm volatile("" : : : "memory");
        tmp3 = tmp2 + acc1;
        
        /* Phase 3: accumulate with volatile read */
        tmp4 = g_var1;
        acc1 = tmp3 ^ tmp4;
        acc2 += tmp1;
        acc3 *= tmp2;
        acc4 ^= tmp3;
        
        /* Irregular control flow with goto */
        if (__builtin_expect_with_probability((i & 0x7) == 0, 0, 0.3)) {
            /* Small backward jump creates control complexity */
            if (acc1 > 1000) {
                i--; /* Rare re-execution */
                continue;
            }
        }
    }
    
    return acc1 + acc2 + acc3 + acc4;
}

__attribute__((optimize("O3")))
int main() {
    /* Initialize global array with pseudo-random data */
    for (int i = 0; i < 256; i++) {
        g_array[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    
    int result = 0;
    
    /* Kernel 1: Long chain with data-dependent loop */
    result += compute_chain(1);
    
    /* Kernel 2: Complex switch statement */
    for (int i = 0; i < 32; i++) {
        result ^= switch_computation(i, result);
    }
    
    /* Kernel 3: Recursive computation */
    result += recursive_compute(4, result & 0xFF);
    
    /* Kernel 4: Software pipelined loop */
    result += software_pipelined_loop(1000);
    
    /* Additional complex loop with irregular control flow */
    int x = 0, y = 0, z = 0;
    for (int i = 0; i < 100; ) {
        /* do-while with break inside conditional */
        do {
            x += y * z;
            y ^= z + i;
            z *= x + g_var1;
            
            if (__builtin_expect_with_probability(z > 1000000, 0, 0.1)) {
                break; /* Creates internal control edge */
            }
            
            /* More computation */
            x += g_array[i & 0xFF];
            y ^= g_array[(i + 1) & 0xFF];
        } while (0);
        
        i += (z & 0x1) + 1; /* Variable increment */
    }
    result += x + y + z;
    
    /* Final checksum */
    printf("Result: %d\n", result);
    return 0;
}
