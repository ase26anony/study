/* haifa-sched-test.c
 * Test program to trigger scheduler context allocation and cleanup
 * Compile with: gcc -O3 -fschedule-insns2 -fno-guess-branch-probability -fno-inline haifa-sched-test.c -o haifa-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to create aliasing and data dependencies */
volatile int g_counter = 0;
int g_array[256];
int g_result1, g_result2, g_result3;
int* volatile g_ptr1 = &g_result1;
int* volatile g_ptr2 = &g_result2;

/* Prevent inlining to create scheduling region boundaries */
__attribute__((noinline, optimize("O3")))
int complex_chain(int start, int iterations) {
    int a = start, b = start * 2, c = start * 3;
    int d = start + 5, e = start + 7, f = start + 11;
    int g = start * 13, h = start * 17, i = start * 19;
    int j = start + 23, k = start + 29, l = start + 31;
    int m = start * 37, n = start * 41, o = start * 43;
    int p = start + 47, q = start + 53, r = start + 59;
    int s = start * 61, t = start * 67, u = start * 71;
    
    /* Data-dependent loop with unpredictable exit */
    int counter = 0;
    while (__builtin_expect_with_probability(counter < iterations, 1, 0.7)) {
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
        o += p ^ q;
        p += q ^ r;
        q += r ^ s;
        r += s ^ t;
        s += t ^ u;
        t += u ^ a;
        u += a ^ b;
        
        /* Memory barrier to split scheduling regions */
        asm volatile("" : : : "memory");
        
        /* Unpredictable conditional break */
        if (__builtin_expect_with_probability((a & 0xFF) == 0, 0, 0.1)) {
            /* This creates a speculative path that may need state saving */
            volatile int temp = g_counter;
            a ^= temp;
            break;
        }
        
        counter++;
    }
    
    /* Mix results to prevent elimination */
    return a + b + c + d + e + f + g + h + i + j + 
           k + l + m + n + o + p + q + r + s + t + u;
}

__attribute__((noinline, optimize("O3")))
int switch_computation(int selector, int base) {
    int v1 = base, v2 = base * 2, v3 = base * 3;
    int v4 = base + 4, v5 = base + 5, v6 = base + 6;
    int v7 = base * 7, v8 = base * 8, v9 = base * 9;
    int v10 = base + 10, v11 = base + 11, v12 = base + 12;
    int v13 = base * 13, v14 = base * 14, v15 = base * 15;
    int v16 = base + 16, v17 = base + 17, v18 = base + 18;
    
    /* Large switch creates complex control flow */
    switch (selector & 0xF) {
        case 0:
            v1 = (v1 ^ v2) + (v3 & v4);
            v5 = v6 * v7 - v8;
            break;
        case 1:
            v2 = (v2 | v3) * (v4 ^ v5);
            v6 = v7 + v8 - v9;
            break;
        case 2:
            v3 = (v3 & v4) | (v5 ^ v6);
            v7 = v8 * v9 + v10;
            break;
        case 3:
            v4 = (v4 ^ v5) + (v6 & v7);
            v8 = v9 - v10 * v11;
            break;
        case 4:
            v5 = (v5 | v6) * (v7 ^ v8);
            v9 = v10 + v11 - v12;
            break;
        case 5:
            v6 = (v6 & v7) | (v8 ^ v9);
            v10 = v11 * v12 + v13;
            break;
        case 6:
            v7 = (v7 ^ v8) + (v9 & v10);
            v11 = v12 - v13 * v14;
            break;
        case 7:
            v8 = (v8 | v9) * (v10 ^ v11);
            v12 = v13 + v14 - v15;
            break;
        case 8:
            v9 = (v9 & v10) | (v11 ^ v12);
            v13 = v14 * v15 + v16;
            break;
        case 9:
            v10 = (v10 ^ v11) + (v12 & v13);
            v14 = v15 - v16 * v17;
            break;
        case 10:
            v11 = (v11 | v12) * (v13 ^ v14);
            v15 = v16 + v17 - v18;
            break;
        case 11:
            v12 = (v12 & v13) | (v14 ^ v15);
            v16 = v17 * v18 + v1;
            break;
        case 12:
            v13 = (v13 ^ v14) + (v15 & v16);
            v17 = v18 - v1 * v2;
            break;
        case 13:
            v14 = (v14 | v15) * (v16 ^ v17);
            v18 = v1 + v2 - v3;
            break;
        case 14:
            v15 = (v15 & v16) | (v17 ^ v18);
            v1 = v2 * v3 + v4;
            break;
        default: /* case 15 */
            v16 = (v16 ^ v17) + (v18 & v1);
            v2 = v3 - v4 * v5;
            break;
    }
    
    /* Merge point with many live variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18;
}

/* Recursive function to create call/return scheduling boundaries */
__attribute__((noinline))
int recursive_compute(int depth, int value) {
    if (__builtin_expect_with_probability(depth <= 0, 0, 0.3)) {
        return value;
    }
    
    int local1 = value * 2;
    int local2 = value + depth;
    int local3 = value ^ depth;
    int local4 = value | (depth << 3);
    int local5 = value & (depth * 7);
    
    /* Memory operation with side effect */
    g_array[depth & 0xFF] = value;
    
    int result = recursive_compute(depth - 1, local1 + local2);
    
    /* Complex computation after recursion */
    result = (result ^ local3) + (local4 & local5);
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result * 31;
}

/* Function with irregular control flow using goto */
__attribute__((noinline, optimize("O3")))
int irregular_control_flow(int limit) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    int x6 = 6, x7 = 7, x8 = 8, x9 = 9, x10 = 10;
    int i = 0;
    
restart_point:
    while (__builtin_expect_with_probability(i < limit, 1, 0.8)) {
        x1 = (x1 * x2) + x3;
        x2 = (x2 ^ x3) | x4;
        x3 = (x3 + x4) & x5;
        x4 = (x4 * x5) ^ x6;
        x5 = (x5 | x6) + x7;
        
        /* Unpredictable goto back */
        if (__builtin_expect_with_probability((x1 & 0x7) == 0, 0, 0.15)) {
            /* This creates a loop with irregular structure */
            x6 = (x6 & x7) * x8;
            goto restart_point;
        }
        
        x6 = (x6 + x7) | x8;
        x7 = (x7 ^ x8) & x9;
        x8 = (x8 * x9) + x10;
        x9 = (x9 | x10) ^ x1;
        x10 = (x10 + x1) & x2;
        
        i++;
        
        /* Do-while with break inside */
        do {
            x1 = x1 ^ x10;
            if (__builtin_expect_with_probability(x1 > 1000000, 0, 0.05)) {
                x2 = x2 * 3;
                break;
            }
            x2 = x2 + x9;
        } while (0);
    }
    
    return x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10;
}

/* Software pipelining style computation */
__attribute__((noinline, optimize("O3")))
int pipelined_computation(int iterations) {
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int tmp1, tmp2, tmp3, tmp4;
    
    for (int i = 0; i < iterations; i++) {
        /* Independent computations that could be pipelined */
        tmp1 = (i * 3) ^ (i + 1);
        tmp2 = (i * 5) | (i + 2);
        tmp3 = (i * 7) & (i + 3);
        tmp4 = (i * 11) + (i + 4);
        
        /* Stagger the accumulation */
        acc1 = acc1 + tmp1;
        acc2 = acc2 ^ tmp2;
        acc3 = acc3 | tmp3;
        acc4 = acc4 & tmp4;
        
        /* Memory operation every 8 iterations */
        if (__builtin_expect_with_probability((i & 0x7) == 0, 0, 0.125)) {
            g_counter = i;
            asm volatile("" : : : "memory");
        }
    }
    
    return acc1 + acc2 + acc3 + acc4;
}

int main() {
    int total_result = 0;
    
    printf("Starting scheduler stress test...\n");
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 3;
    }
    
    /* Test 1: Long dependent chain with unpredictable exit */
    printf("Running complex chain test...\n");
    total_result ^= complex_chain(42, 1000);
    
    /* Test 2: Large switch statement */
    printf("Running switch computation test...\n");
    for (int i = 0; i < 50; i++) {
        total_result += switch_computation(i, total_result);
    }
    
    /* Test 3: Recursive computation */
    printf("Running recursive test...\n");
    total_result ^= recursive_compute(8, total_result);
    
    /* Test 4: Irregular control flow */
    printf("Running irregular control flow test...\n");
    total_result += irregular_control_flow(500);
    
    /* Test 5: Software pipelining style */
    printf("Running pipelined computation test...\n");
    total_result ^= pipelined_computation(1000);
    
    /* Additional mixed test */
    printf("Running mixed pattern test...\n");
    for (int i = 0; i < 20; i++) {
        /* Alternate between different patterns */
        if (i & 1) {
            *g_ptr1 = complex_chain(total_result, 50 + (i * 3));
            total_result += *g_ptr1;
        } else {
            *g_ptr2 = switch_computation(total_result & 0xF, i * 17);
            total_result ^= *g_ptr2;
        }
        
        /* Memory barrier between patterns */
        asm volatile("" : : : "memory");
    }
    
    printf("Final result: %d\n", total_result);
    printf("Test completed.\n");
    
    return total_result != 0 ? 0 : 1;
}
