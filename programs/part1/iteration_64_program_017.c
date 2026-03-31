#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for creating aliasing and data dependencies */
volatile int g_counter = 0;
int g_array[256];
int g_results[4];
unsigned long g_checksum = 0;

/* Helper functions marked noinline to prevent inlining */
__attribute__((noinline, optimize("O3")))
void compute_chain(int *result, int start) {
    /* Long chain of dependent operations */
    int a = start;
    int b = start * 2;
    int c = start + 1;
    int d = start - 1;
    
    /* Create data dependencies */
    for (int i = 0; i < 32; i++) {
        a = (a * 1103515245 + 12345) & 0x7fffffff;
        b ^= (a >> 16) & 0xffff;
        c += (b & 0xff) * 17;
        d = (d << 3) | (c & 7);
        
        /* Memory barrier to potentially split scheduling regions */
        if (i % 8 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Data-dependent exit condition */
    int i = 0;
    while (g_array[i] != 0 && i < 255) {
        a += g_array[i];
        b ^= g_array[i + 1];
        i += 2;
    }
    
    *result = a + b + c + d;
}

__attribute__((noinline, optimize("O3")))
int recursive_compute(int depth, int value) {
    if (depth <= 0) {
        return value;
    }
    
    /* Mix of operations */
    int new_val = value;
    new_val = (new_val * 6364136223846793005ULL + 1442695040888963407ULL) & 0x7fffffff;
    
    /* Create register pressure with many variables */
    int v1 = new_val, v2 = new_val + 1, v3 = new_val + 2, v4 = new_val + 3;
    int v5 = new_val * 2, v6 = new_val * 3, v7 = new_val * 4, v8 = new_val * 5;
    int v9 = new_val ^ 0x55, v10 = new_val ^ 0xAA, v11 = new_val ^ 0xFF;
    
    /* Complex expression with many dependencies */
    v1 = v1 + v2 - v3 * v4 / (v5 + 1);
    v6 = v6 | v7 & v8 ^ v9;
    v10 = (v10 << (v11 & 3)) | (v10 >> (8 - (v11 & 3)));
    
    /* Recursive call - scheduler may save/restore state around calls */
    int ret = recursive_compute(depth - 1, new_val);
    
    /* More operations after recursion */
    ret = ret + v1 + v6 + v10;
    
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect_with_probability(ret > 1000, 0, 0.7)) {
        ret >>= 2;
    }
    
    return ret;
}

__attribute__((noinline, optimize("O3")))
void switch_computation(int *output) {
    /* Many local variables to create register pressure */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    int q = 17, r = 18, s = 19, t = 20, u = 21, v = 22, w = 23, x = 24;
    
    /* Data-dependent switch index */
    int idx = g_counter++ % 12;
    
    /* Complex switch with many cases - scheduler may need state management */
    switch (idx) {
        case 0:
            a = b + c; d = e * f; g = h ^ i;
            /* Chain operations */
            for (int z = 0; z < 8; z++) a += z * b;
            break;
        case 1:
            j = k - l; m = n / (o + 1); p = q | r;
            asm volatile ("" : : : "memory"); /* Scheduling barrier */
            break;
        case 2:
            s = t << 2; u = v >> 1; w = x & 0xFF;
            /* Loop with data-dependent exit */
            int cnt = a & 7;
            while (cnt-- > 0) s += u;
            break;
        case 3:
            a = b * c * d; e = f + g + h;
            i = j ^ k ^ l; m = n | o | p;
            break;
        case 4:
            q = r * s; t = u - v; w = x & a;
            /* Dependent chain */
            q = (q * 3 + t) ^ w;
            break;
        case 5:
            b = c + d + e; f = g * h * i;
            j = k & l & m; n = o | p | q;
            break;
        case 6:
            r = s ^ t; u = v << (w & 3);
            x = a >> (b & 3); c = d + e + f;
            break;
        case 7:
            g = h * i * j; k = l + m + n;
            o = p ^ q ^ r; s = t | u | v;
            break;
        case 8:
            w = x & a & b; c = d | e | f;
            g = h ^ i ^ j; k = l * m * n;
            break;
        case 9:
            o = p + q; r = s * t; u = v ^ w;
            /* Small loop */
            for (int z = 0; z < 4; z++) o += z;
            break;
        case 10:
            x = a << 1; b = c >> 2; d = e & 0xAA;
            f = g | 0x55; h = i ^ 0xFF;
            break;
        case 11:
            j = k + l + m; n = o * p * q;
            r = s & t & u; v = w | x | a;
            /* Use probability hint */
            if (__builtin_expect(j > 100, 0)) {
                j >>= 2;
            }
            break;
        default:
            b = c = d = 0;
    }
    
    /* Aggregate results */
    *output = a + b + c + d + e + f + g + h + i + j + k + l + 
              m + n + o + p + q + r + s + t + u + v + w + x;
}

__attribute__((noinline, optimize("O3")))
void pipelined_computation(int *results, int size) {
    /* Manual software pipelining style */
    for (int i = 0; i < size; i++) {
        /* Prologue - load phase */
        int load1 = g_array[i];
        int load2 = g_array[(i + 1) % 256];
        
        /* Computational phase 1 */
        int comp1 = load1 * 3;
        int comp2 = load2 + 7;
        
        /* Scheduling barrier */
        asm volatile ("" : : : "memory");
        
        /* Computational phase 2 */
        comp1 = (comp1 ^ 0x55) + comp2;
        comp2 = (comp2 * 13) & 0xFF;
        
        /* Epilogue - store phase */
        results[i % 4] += comp1 + comp2;
        
        /* Data-dependent loop control */
        if (__builtin_expect_with_probability(results[i % 4] > 10000, 0, 0.3)) {
            results[i % 4] >>= 2;
        }
    }
}

__attribute__((noinline, optimize("O3")))
int complex_control_flow(int seed) {
    int result = seed;
    int i = 0;
    
    /* Irregular control flow with goto */
start_loop:
    result = (result * 1103515245 + 12345) & 0x7fffffff;
    
    /* do-while with internal break */
    do {
        if (result & 0x100) {
            result ^= 0xAA;
            if (result > 1000000) {
                break;
            }
        }
        result += i;
        i++;
    } while (0);
    
    /* Conditional goto back */
    if (__builtin_expect((result & 0xF) < 8, 1)) {
        if (i < 16) {
            goto start_loop;
        }
    }
    
    /* Another loop with data-dependent exit */
    int j = result & 0xFF;
    while (g_array[j] != 0 && j < 255) {
        result += g_array[j];
        j++;
        
        /* Memory operation in loop */
        volatile int temp = g_counter;
        result ^= temp;
    }
    
    return result;
}

int main() {
    /* Initialize global array with non-zero values */
    for (int i = 0; i < 256; i++) {
        g_array[i] = (i * 3 + 1) & 0xFF;
    }
    /* Ensure some zeros for data-dependent exits */
    g_array[100] = 0;
    g_array[200] = 0;
    
    /* Kernel 1: Long chain with data-dependent exit */
    compute_chain(&g_results[0], 42);
    
    /* Kernel 2: Recursive computation */
    g_results[1] = recursive_compute(4, 1000);
    
    /* Kernel 3: Switch-based computation */
    switch_computation(&g_results[2]);
    
    /* Kernel 4: Pipelined computation */
    pipelined_computation(g_results, 128);
    
    /* Kernel 5: Complex control flow */
    g_results[3] = complex_control_flow(123456);
    
    /* Aggregate all results into checksum */
    for (int i = 0; i < 4; i++) {
        g_checksum += g_results[i];
        /* Mix bits */
        g_checksum = (g_checksum << 13) | (g_checksum >> (64 - 13));
        g_checksum ^= 0x5A5A5A5A5A5A5A5AUL;
    }
    
    /* Final computation to ensure no elimination */
    volatile unsigned long final_result = g_checksum;
    for (int i = 0; i < 100; i++) {
        final_result = (final_result * 6364136223846793005ULL + 
                       1442695040888963407ULL);
    }
    
    printf("Result: %lu\n", (unsigned long)final_result);
    
    return (final_result & 0xFF);
}
