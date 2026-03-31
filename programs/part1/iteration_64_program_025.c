/* haifa-sched-test.c
 * Test program to trigger haifa scheduler state save/restore cleanup paths
 * Compile with: gcc -O3 -fschedule-insns2 -fno-guess-branch-probability -fno-inline haifa-sched-test.c -o haifa-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_counter = 0;
int g_array[256];
int g_results[256];

/* Prevent optimization */
#define KEEP(expr) do { asm volatile("" : : "r"(expr) : "memory"); } while(0)

/* Noinline functions to create scheduling boundaries */
__attribute__((noinline, optimize("O3")))
static int compute_chain(int seed) {
    volatile int barrier;
    int a = seed, b = seed * 2, c = seed * 3, d = seed * 4;
    int e = seed * 5, f = seed * 6, g = seed * 7, h = seed * 8;
    
    /* Long chain of dependent operations */
    a += b ^ c;
    b += c ^ d;
    c += d ^ e;
    d += e ^ f;
    e += f ^ g;
    f += g ^ h;
    g += h ^ a;
    h += a ^ b;
    
    /* Memory barrier to split scheduling regions */
    asm volatile("" : : : "memory");
    
    /* More dependent operations */
    a *= b + 1;
    b *= c + 2;
    c *= d + 3;
    d *= e + 4;
    e *= f + 5;
    f *= g + 6;
    g *= h + 7;
    h *= a + 8;
    
    /* Data-dependent control flow */
    if (__builtin_expect_with_probability((a & 0xFF) > 128, 1, 0.7)) {
        barrier = a;
        a += barrier * 2;
    }
    
    return a + b + c + d + e + f + g + h;
}

__attribute__((noinline, optimize("O3")))
static int recursive_compute(int depth, int val) {
    if (depth <= 0) {
        return val;
    }
    
    int a = val * depth;
    int b = val + depth;
    int c = val ^ depth;
    
    /* Create register pressure */
    int r1 = a * b, r2 = b * c, r3 = c * a;
    int r4 = r1 ^ r2, r5 = r2 ^ r3, r6 = r3 ^ r1;
    int r7 = r4 * r5, r8 = r5 * r6, r9 = r6 * r4;
    
    /* Recursive call - scheduler may save state around call */
    int recurse_result = recursive_compute(depth - 1, r7 + r8 + r9);
    
    /* Complex merge point */
    return recurse_result + r1 + r2 + r3 + r4 + r5 + r6;
}

__attribute__((noinline, optimize("O3")))
static void switch_computation(int *results, int size) {
    for (int i = 0; i < size; i++) {
        /* Data-dependent switch with many cases */
        switch (i & 0xF) {  /* 16 cases */
            case 0: results[i] = i * 2; break;
            case 1: results[i] = i + results[i-1]; break;
            case 2: results[i] = i ^ 0x55AA; break;
            case 3: results[i] = results[i-1] * 3; break;
            case 4: results[i] = i << (i & 3); break;
            case 5: results[i] = results[i-2] + results[i-1]; break;
            case 6: results[i] = ~i; break;
            case 7: results[i] = i * i; break;
            case 8: results[i] = results[i-1] ^ results[i-2]; break;
            case 9: results[i] = i / 2; break;
            case 10: results[i] = i | 0xFF00; break;
            case 11: results[i] = results[i-3] * 4; break;
            case 12: results[i] = i - results[i-1]; break;
            case 13: results[i] = i & 0x0F0F; break;
            case 14: results[i] = results[i-1] << 2; break;
            case 15: results[i] = i % 13; break;
        }
        
        /* Irregular control flow within loop */
        if (__builtin_expect_with_probability((results[i] & 1) == 0, 0, 0.3)) {
            /* Jump back to create complex CFG */
            i -= 2;
            if (i < 0) i = 0;
        }
    }
}

__attribute__((noinline, optimize("O3")))
static int software_pipelined_loop(int iterations) {
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int tmp1, tmp2, tmp3, tmp4;
    
    /* Manual software pipelining */
    for (int i = 0; i < iterations; i++) {
        /* Phase 1: load/compute */
        tmp1 = g_array[i & 255];
        tmp2 = g_array[(i + 1) & 255];
        
        /* Phase 2: compute */
        tmp3 = tmp1 * tmp2;
        tmp4 = tmp1 + tmp2;
        
        /* Phase 3: accumulate with memory barrier */
        asm volatile("" : : : "memory");
        acc1 += tmp3;
        acc2 += tmp4;
        
        /* Phase 4: cross-accumulate */
        acc3 += acc1 ^ acc2;
        acc4 += acc1 & acc2;
        
        /* Data-dependent loop exit */
        if (__builtin_expect_with_probability(acc1 > 1000000, 0, 0.1)) {
            break;
        }
    }
    
    return acc1 + acc2 + acc3 + acc4;
}

__attribute__((noinline, optimize("O3")))
static int complex_control_flow(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    int i = seed + 8, j = seed + 9, k = seed + 10, l = seed + 11;
    int m = seed + 12, n = seed + 13, o = seed + 14, p = seed + 15;
    
    /* do-while with internal break */
    int counter = 0;
    do {
        counter++;
        
        /* Nested switch */
        switch (counter & 3) {
            case 0:
                a += b; b += c; c += d;
                /* Fall through */
            case 1:
                d += e; e += f; f += g;
                if (__builtin_expect_with_probability(a > 100, 0, 0.4)) {
                    break;  /* Break from do-while */
                }
                break;
            case 2:
                g += h; h += i; i += j;
                /* Memory operation with uncertain latency */
                g_counter = j;
                break;
            case 3:
                j += k; k += l; l += m;
                m += n; n += o; o += p;
                break;
        }
        
        /* Pointer aliasing to create dependencies */
        int *ptr1 = &g_array[counter & 255];
        int *ptr2 = (int*)((char*)ptr1 + (counter & 3));
        *ptr1 = a;
        *ptr2 = b;
        
    } while (__builtin_expect_with_probability(counter < 50, 1, 0.8));
    
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
}

int main(void) {
    /* Initialize globals */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 3 + 1;
        g_results[i] = 0;
    }
    
    int total = 0;
    
    /* Test 1: Long dependent chain with probabilistic branching */
    total += compute_chain(42);
    
    /* Test 2: Recursive computation */
    total += recursive_compute(4, total);
    
    /* Test 3: Switch-based computation with irregular control flow */
    switch_computation(g_results, 128);
    for (int i = 0; i < 128; i++) {
        total += g_results[i];
    }
    
    /* Test 4: Software pipelined loop */
    total += software_pipelined_loop(1000);
    
    /* Test 5: Complex control flow with many variables */
    total += complex_control_flow(total & 0xFF);
    
    /* Additional stress: loop with helper function calls */
    for (int i = 0; i < 100; i++) {
        /* Mix of operations that may trigger scheduler state save */
        int val = compute_chain(i);
        
        /* Data-dependent array access */
        g_results[i & 127] += val;
        
        /* Conditional with unpredictable outcome */
        if (__builtin_expect_with_probability((val & 0xF) == 0, 0, 0.2)) {
            /* Call another function - scheduling boundary */
            g_results[(i + 1) & 127] += recursive_compute(2, val);
        }
    }
    
    /* Final aggregation */
    for (int i = 0; i < 128; i++) {
        total += g_results[i];
    }
    
    /* Ensure result is used */
    KEEP(total);
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
