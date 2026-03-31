#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for aliasing and memory effects */
volatile int g_counter = 0;
int g_array[256];
int g_results[4];
long g_state = 0;

/* Function attributes to control optimization and scheduling */
__attribute__((noinline)) 
__attribute__((optimize("O3")))
static int compute_chain(int seed) {
    volatile int barrier;
    int a = seed, b = seed * 2, c = seed + 1, d = seed ^ 0x55AA;
    int e = seed >> 2, f = seed << 1, g = seed % 17, h = seed | 0xFF;
    
    /* Long chain of dependent operations */
    a += b * c;
    b ^= d + e;
    c *= f - g;
    d += h ^ a;
    e = (e * b) >> (c & 3);
    f = (f + d) * (g - a);
    g ^= (h << (b & 7)) | e;
    h += (c * d) / (f + 1);
    
    /* Memory barrier to split scheduling region */
    asm volatile("" : : : "memory");
    
    /* More dependent operations */
    a = (a + b) ^ (c - d);
    b = (b * c) + (d ^ e);
    c = (c - d) * (e + f);
    d = (d ^ e) + (f * g);
    e = (e << 2) | (g >> 3);
    f = (f + h) * (a - b);
    g = (g ^ c) + (d * e);
    h = (h - f) ^ (g & 0xFF);
    
    /* Another memory barrier */
    barrier = a;
    (void)barrier;
    
    /* Final computation with data-dependent exit */
    int result = a + b + c + d + e + f + g + h;
    int i = 0;
    while (g_array[i] != 0 && i < 255) {
        result ^= g_array[i];
        i++;
        /* Create unpredictable branch probability */
        if (__builtin_expect_with_probability(result & 0x100, 0, 0.3)) {
            result >>= 1;
        }
    }
    
    return result;
}

__attribute__((noinline))
__attribute__((optimize("O3")))
static void complex_switch(int selector, int* results) {
    int r0 = 0, r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
    int r6 = 0, r7 = 0, r8 = 0, r9 = 0, r10 = 0, r11 = 0;
    
    /* Switch with many cases - creates complex control flow */
    switch (selector & 0xF) {
        case 0:
            r0 = compute_chain(selector);
            r1 = r0 * 2;
            r2 = r1 ^ 0x1234;
            break;
        case 1:
            r1 = compute_chain(selector + 1);
            r3 = r1 >> 1;
            r4 = r3 + selector;
            break;
        case 2:
            r2 = compute_chain(selector + 2);
            r5 = r2 & 0xFF;
            r6 = r5 * 3;
            break;
        case 3:
            r3 = compute_chain(selector + 3);
            r7 = r3 | 0xAA;
            r8 = r7 - selector;
            break;
        case 4:
            r4 = compute_chain(selector + 4);
            r9 = r4 ^ r4;
            r10 = r9 + 1;
            break;
        case 5:
            r5 = compute_chain(selector + 5);
            r11 = r5 % 17;
            r0 = r11 * 2;
            break;
        case 6:
            r6 = compute_chain(selector + 6);
            r1 = r6 << 2;
            r2 = r1 ^ 0x55;
            break;
        case 7:
            r7 = compute_chain(selector + 7);
            r3 = r7 >> 3;
            r4 = r3 + 0x100;
            break;
        case 8:
            r8 = compute_chain(selector + 8);
            r5 = r8 & 0xF0;
            r6 = r5 | 0x0F;
            break;
        case 9:
            r9 = compute_chain(selector + 9);
            r7 = r9 * r9;
            r8 = r7 % 256;
            break;
        case 10:
            r10 = compute_chain(selector + 10);
            r9 = r10 ^ selector;
            r10 = r9 + r10;
            break;
        case 11:
            r11 = compute_chain(selector + 11);
            r0 = r11 << 1;
            r1 = r0 ^ r11;
            break;
        case 12:
            r0 = compute_chain(selector + 12);
            r2 = r0 * 3;
            r3 = r2 >> 1;
            break;
        case 13:
            r1 = compute_chain(selector + 13);
            r4 = r1 & 0x7F;
            r5 = r4 | 0x80;
            break;
        case 14:
            r2 = compute_chain(selector + 14);
            r6 = r2 + 0x1000;
            r7 = r6 - selector;
            break;
        default: /* case 15 */
            r3 = compute_chain(selector + 15);
            r8 = r3 ^ 0xFFFF;
            r9 = r8 >> 4;
            break;
    }
    
    /* Merge results with irregular control flow */
    results[0] = r0 + r1 + r2;
    results[1] = r3 + r4 + r5;
    results[2] = r6 + r7 + r8;
    results[3] = r9 + r10 + r11;
    
    /* Artificial goto to create irregular CFG */
    if (__builtin_expect_with_probability(results[0] > 1000, 0, 0.2)) {
        goto recalculation;
    }
    
    return;
    
recalculation:
    results[0] >>= 1;
    results[1] <<= 1;
}

__attribute__((noinline))
static int recursive_scheduler(int depth, int value) {
    if (depth <= 0) {
        return value;
    }
    
    int local_vars[8];
    for (int i = 0; i < 8; i++) {
        local_vars[i] = value + i;
    }
    
    /* Create register pressure with many variables */
    int a = local_vars[0], b = local_vars[1], c = local_vars[2];
    int d = local_vars[3], e = local_vars[4], f = local_vars[5];
    int g = local_vars[6], h = local_vars[7];
    
    /* Dependent operations that benefit from scheduling */
    for (int i = 0; i < 4; i++) {
        a = (a + b) ^ c;
        b = (b * d) + e;
        c = (c ^ f) - g;
        d = (d + h) * a;
        e = (e ^ b) >> (c & 3);
        f = (f * g) + d;
        g = (g ^ a) | b;
        h = (h + c) * (d & 0xFF);
        
        /* Memory operation with uncertain latency */
        g_counter = i;
    }
    
    /* Recursive call - creates call/return boundaries */
    int result = recursive_scheduler(depth - 1, a + b + c + d);
    
    /* More operations after recursion */
    result = (result ^ e) + (f * g) - h;
    
    /* Software pipelining style manual unrolling */
    do {
        result += (a * b);
        if (result & 0x100) {
            result >>= 2;
            break;
        }
        result ^= (c ^ d);
    } while (0);
    
    return result;
}

__attribute__((noinline))
__attribute__((optimize("O3")))
static void software_pipelined_loop(int iterations, int* output) {
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int tmp1, tmp2, tmp3, tmp4;
    
    /* Manual software pipelining */
    for (int i = 0; i < iterations; i++) {
        /* Stage 1: Load/compute */
        tmp1 = g_array[i & 255] + i;
        
        /* Stage 2: Compute with previous value */
        tmp2 = acc1 * tmp1;
        
        /* Stage 3: More computation */
        tmp3 = tmp2 ^ acc2;
        
        /* Stage 4: Final accumulation */
        acc4 = acc3 + tmp3;
        
        /* Rotate registers for next iteration */
        acc3 = acc2;
        acc2 = tmp2;
        acc1 = tmp1;
        
        /* Data-dependent break to challenge scheduler */
        if (__builtin_expect_with_probability(acc4 > 0x7FFFFFFF, 0, 0.1)) {
            acc4 >>= 1;
        }
    }
    
    *output = acc4;
}

int main(void) {
    /* Initialize global array with non-zero values */
    for (int i = 0; i < 256; i++) {
        g_array[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    
    int final_result = 0;
    
    /* Kernel 1: Long chain with data-dependent exit */
    for (int i = 0; i < 100; i++) {
        int r = compute_chain(i);
        final_result ^= r;
        
        /* Create aliasing effects */
        int* alias1 = &g_results[i & 3];
        int* alias2 = (int*)((char*)&g_results[0] + ((i * 4) & 15));
        *alias1 = r;
        *alias2 ^= r;
    }
    
    /* Kernel 2: Complex switch statement */
    for (int i = 0; i < 50; i++) {
        complex_switch(i + final_result, g_results);
        final_result += g_results[0] + g_results[1] + g_results[2] + g_results[3];
    }
    
    /* Kernel 3: Recursive function with register pressure */
    final_result ^= recursive_scheduler(4, final_result);
    
    /* Kernel 4: Software pipelined computation */
    int pipeline_result;
    software_pipelined_loop(1000, &pipeline_result);
    final_result += pipeline_result;
    
    /* Kernel 5: Irregular loop with goto */
    int loop_var = 0;
    int sum = 0;
    
irregular_loop:
    for (int j = 0; j < 10; j++) {
        sum += compute_chain(loop_var + j);
        
        /* Nested loop with break */
        for (int k = 0; k < 5; k++) {
            sum ^= k;
            if (sum & 0x800) {
                sum >>= 1;
                break;
            }
        }
    }
    
    loop_var++;
    if (loop_var < 5) {
        /* Jump back creating irregular CFG */
        goto irregular_loop;
    }
    
    final_result ^= sum;
    
    /* Ensure computations aren't eliminated */
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
