/* haifa-sched-trigger.c
 * Designed to trigger scheduler context allocation and cleanup
 * Compile with: gcc -O3 -fschedule-insns2 -fno-guess-branch-probability -fno-inline -o haifa-test haifa-sched-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_counter = 0;
int g_array[256];
int g_results[256];

/* ========== Helper Functions with Complex Control Flow ========== */

/* Long chain of dependent operations with data-dependent exit */
__attribute__((noinline))
static int complex_chain(int start, int* data, int len) {
    int a = start, b = start * 2, c = start + 1;
    int d = 0, e = 0, f = 0, g = 0, h = 0;
    int i = 0, j = 0, k = 0, l = 0, m = 0;
    int n = 0, o = 0, p = 0, q = 0, r = 0;
    int s = 0, t = 0, u = 0, v = 0, w = 0;
    
    /* Create artificial dependencies */
    a += data[0];
    b ^= a;
    c *= b;
    d = c >> 3;
    e = d ^ c;
    f = e + a;
    g = f * b;
    h = g - c;
    i = h ^ d;
    j = i + e;
    k = j * f;
    l = k ^ g;
    m = l - h;
    n = m + i;
    o = n ^ j;
    p = o * k;
    q = p - l;
    r = q ^ m;
    s = r + n;
    t = s * o;
    u = t ^ p;
    v = u - q;
    w = v ^ r;
    
    /* Data-dependent loop with unpredictable exit */
    int idx = 0;
    while (__builtin_expect_with_probability(data[idx] != 0, 1, 0.7)) {
        /* Mix memory operations with computation */
        volatile int mem_read = g_counter;
        w += mem_read;
        w ^= data[idx];
        w *= 0x5A827999;
        
        /* Scheduling barrier */
        asm volatile("" : : : "memory");
        
        w += idx;
        idx = (idx + 1) & 0xFF;
        
        /* Early exit with probability */
        if (__builtin_expect_with_probability(w > 0x7FFFFFFF, 0, 0.3)) {
            break;
        }
    }
    
    return w;
}

/* Recursive function with arithmetic */
__attribute__((noinline))
static int recursive_compute(int depth, int value) {
    if (depth <= 0) {
        return value;
    }
    
    int a = value * 3;
    int b = value + depth;
    int c = value ^ 0xDEADBEEF;
    
    /* Create register pressure */
    int d = a + b;
    int e = b ^ c;
    int f = c * a;
    int g = d - e;
    int h = f ^ g;
    int i = h + d;
    int j = i * e;
    int k = j ^ f;
    
    /* Memory clobber to force scheduling boundaries */
    asm volatile("" : : : "memory");
    
    int result = recursive_compute(depth - 1, k);
    
    /* More computation after recursion */
    result ^= g;
    result += h;
    result *= i;
    
    return result;
}

/* Function with switch statement and many cases */
__attribute__((noinline, optimize("O3")))
static int switch_computation(int selector, int* vars) {
    int result = 0;
    
    switch (selector & 0xF) {
        case 0:
            vars[0] += vars[1];
            vars[2] ^= vars[3];
            result = vars[0] * vars[2];
            break;
        case 1:
            vars[1] -= vars[4];
            vars[5] |= vars[6];
            result = vars[1] ^ vars[5];
            break;
        case 2:
            vars[2] *= vars[7];
            vars[8] &= vars[9];
            result = vars[2] + vars[8];
            break;
        case 3:
            vars[3] ^= vars[10];
            vars[11] += vars[12];
            result = vars[3] | vars[11];
            break;
        case 4:
            vars[4] += vars[13];
            vars[14] -= vars[15];
            result = vars[4] & vars[14];
            break;
        case 5:
            vars[5] *= vars[16];
            vars[17] ^= vars[18];
            result = vars[5] + vars[17];
            break;
        case 6:
            vars[6] -= vars[19];
            vars[20] |= vars[21];
            result = vars[6] ^ vars[20];
            break;
        case 7:
            vars[7] ^= vars[22];
            vars[23] += vars[24];
            result = vars[7] * vars[23];
            break;
        case 8:
            vars[8] += vars[25];
            vars[26] &= vars[27];
            result = vars[8] | vars[26];
            break;
        case 9:
            vars[9] *= vars[28];
            vars[29] -= vars[30];
            result = vars[9] ^ vars[29];
            break;
        case 10:
            vars[10] ^= vars[31];
            vars[0] += vars[1];
            result = vars[10] & vars[0];
            break;
        case 11:
            vars[11] -= vars[2];
            vars[3] |= vars[4];
            result = vars[11] * vars[3];
            break;
        case 12:
            vars[12] += vars[5];
            vars[6] &= vars[7];
            result = vars[12] + vars[6];
            break;
        case 13:
            vars[13] *= vars[8];
            vars[9] ^= vars[10];
            result = vars[13] | vars[9];
            break;
        case 14:
            vars[14] -= vars[11];
            vars[12] += vars[13];
            result = vars[14] ^ vars[12];
            break;
        default: /* case 15 */
            vars[15] ^= vars[14];
            vars[13] *= vars[12];
            result = vars[15] & vars[13];
            break;
    }
    
    return result;
}

/* Software pipelined style computation */
__attribute__((noinline))
static int pipelined_compute(int iterations) {
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int tmp1, tmp2, tmp3;
    
    for (int i = 0; i < iterations; i++) {
        /* Phase 1: Load and initial computation */
        tmp1 = g_array[i & 0xFF];
        tmp1 ^= i;
        tmp1 *= 0x9E3779B9;
        
        /* Scheduling barrier between phases */
        asm volatile("" : : : "memory");
        
        /* Phase 2: More computation */
        tmp2 = tmp1 + acc1;
        tmp2 ^= 0xAAAAAAAA;
        tmp2 = (tmp2 >> 3) | (tmp2 << 29);
        
        /* Phase 3: Final computation and store */
        tmp3 = tmp2 - acc2;
        tmp3 *= 0x6ED9EBA1;
        acc3 += tmp3;
        
        /* Rotate accumulators */
        acc1 = acc2;
        acc2 = acc3;
        acc3 = tmp3 & 0x7FFFFFFF;
        
        /* Irregular control flow with goto */
        if (__builtin_expect_with_probability((i & 0x3F) == 0, 0, 0.1)) {
            g_counter++;
            goto pipeline_continue;
        }
        
        if (__builtin_expect_with_probability((i & 0x1F) == 0, 0, 0.2)) {
            acc1 ^= 0x55555555;
        }
        
    pipeline_continue:
        /* Empty label for goto target */
        ;
    }
    
    return acc1 + acc2 + acc3;
}

/* Function with do-while and break statements */
__attribute__((noinline))
static int loop_with_breaks(int limit) {
    int x = 1, y = 2, z = 3;
    int a = 4, b = 5, c = 6;
    int d = 7, e = 8, f = 9;
    int result = 0;
    int i = 0;
    
    do {
        /* Complex computation chain */
        x += y;
        y ^= z;
        z *= x;
        a -= b;
        b |= c;
        c &= a;
        d ^= e;
        e += f;
        f *= d;
        
        /* Conditional break inside do-while */
        if (__builtin_expect_with_probability(i > limit, 0, 0.4)) {
            break;
        }
        
        /* Another computation chain */
        x ^= a;
        y += b;
        z *= c;
        a -= d;
        b ^= e;
        c |= f;
        
        /* Another conditional break */
        if (__builtin_expect_with_probability((x & 0xFF) == 0, 0, 0.2)) {
            result = x + y + z;
            break;
        }
        
        i++;
        
        /* Memory clobber to create scheduling region boundary */
        asm volatile("" : : : "memory");
        
    } while (1);
    
    return result + a + b + c + d + e + f;
}

/* ========== Main Function ========== */

int main(void) {
    /* Initialize global arrays */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 0x01010101;
        g_results[i] = 0;
    }
    
    uint64_t final_checksum = 0;
    
    /* Kernel 1: Complex chain with data-dependent exit */
    printf("Running kernel 1...\n");
    for (int i = 0; i < 100; i++) {
        int result = complex_chain(i, g_array, 256);
        g_results[i & 0xFF] ^= result;
        final_checksum += result;
    }
    
    /* Kernel 2: Recursive computation */
    printf("Running kernel 2...\n");
    for (int i = 0; i < 50; i++) {
        int result = recursive_compute(3, i);
        g_results[(i * 3) & 0xFF] += result;
        final_checksum ^= result;
    }
    
    /* Kernel 3: Switch-based computation */
    printf("Running kernel 3...\n");
    int vars[32];
    for (int i = 0; i < 32; i++) {
        vars[i] = i * 0x11111111;
    }
    
    for (int i = 0; i < 200; i++) {
        int result = switch_computation(i, vars);
        g_results[i & 0xFF] *= (result | 1);
        final_checksum += result;
    }
    
    /* Kernel 4: Pipelined computation */
    printf("Running kernel 4...\n");
    int pipe_result = pipelined_compute(1000);
    g_results[pipe_result & 0xFF] ^= pipe_result;
    final_checksum += pipe_result;
    
    /* Kernel 5: Loop with breaks */
    printf("Running kernel 5...\n");
    int break_result = loop_with_breaks(100);
    g_results[break_result & 0xFF] += break_result;
    final_checksum ^= break_result;
    
    /* Aggregate all results */
    for (int i = 0; i < 256; i++) {
        final_checksum += g_results[i];
    }
    
    /* Use volatile to prevent elimination */
    volatile uint64_t output = final_checksum;
    printf("Final checksum: 0x%016llX\n", (unsigned long long)output);
    
    return 0;
}
