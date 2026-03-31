/* haifa-sched-trigger.c
 * Designed to trigger haifa scheduler state save/restore cleanup paths
 * Compile with: gcc -O3 -fschedule-insns2 -fno-guess-branch-probability -fno-inline -o haifa-test haifa-sched-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_counter = 0;
int g_array[256];
int g_results[4] = {0};

/* ========== Helper Functions with Complex Control Flow ========== */

/* Long chain of dependent operations with data-dependent exit */
__attribute__((noinline)) 
static int complex_chain(int seed) {
    volatile int barrier = seed;
    int a = barrier;
    int b = a + 1;
    int c = b * 2;
    int d = c ^ 0x55AA55AA;
    int e = d + a;
    int f = e * 3;
    int g = f ^ d;
    int h = g + c;
    int i = h * 5;
    int j = i ^ f;
    int k = j + e;
    int l = k * 7;
    int m = l ^ h;
    int n = m + i;
    int o = n * 11;
    int p = o ^ j;
    int q = p + k;
    int r = q * 13;
    int s = r ^ l;
    int t = s + m;
    
    /* Memory barrier to split scheduling region */
    asm volatile("" : : : "memory");
    
    int u = t * 17;
    int v = u ^ n;
    int w = v + o;
    int x = w * 19;
    int y = x ^ p;
    int z = y + q;
    
    return z;
}

/* Function with switch and many cases */
__attribute__((noinline, optimize("O3")))
static int switch_complex(int val, int mode) {
    int result = val;
    
    switch (mode % 12) {
        case 0:
            result += complex_chain(val);
            result ^= 0x11111111;
            break;
        case 1:
            result *= 3;
            result -= complex_chain(val + 1);
            break;
        case 2:
            result ^= complex_chain(val + 2);
            result += 0x22222222;
            break;
        case 3:
            result = complex_chain(result);
            result *= 7;
            break;
        case 4:
            result += result * 2;
            result ^= 0x33333333;
            break;
        case 5:
            result = complex_chain(result * 3);
            result -= val;
            break;
        case 6:
            result ^= 0x44444444;
            result += complex_chain(val * 2);
            break;
        case 7:
            result *= 11;
            result = complex_chain(result);
            break;
        case 8:
            result += 0x55555555;
            result ^= complex_chain(val + 3);
            break;
        case 9:
            result = complex_chain(result ^ 0x66666666);
            break;
        case 10:
            result *= 13;
            result -= complex_chain(val * 3);
            break;
        case 11:
            result ^= complex_chain(result + 1);
            result += 0x77777777;
            break;
        default:
            result = 0;
    }
    
    return result;
}

/* Recursive function with arithmetic */
__attribute__((noinline))
static int recursive_compute(int depth, int val) {
    if (depth <= 0) {
        return val;
    }
    
    int new_val = val;
    
    /* Create data dependency chain */
    for (int i = 0; i < 3; i++) {
        new_val += complex_chain(new_val + i);
        new_val ^= 0x12345678;
        
        /* Branch with probability hint */
        if (__builtin_expect_with_probability(
            (new_val & 0xF) == 0, 0, 0.7)) {
            asm volatile("" : : : "memory");
            new_val *= 3;
        }
    }
    
    /* Recursive call */
    int result = recursive_compute(depth - 1, new_val);
    
    /* More operations after recursion */
    result ^= 0x87654321;
    result += complex_chain(result);
    
    return result;
}

/* Function with irregular control flow using goto */
__attribute__((noinline))
static int irregular_flow(int start) {
    int a = start;
    int b = a + 1;
    int c = b * 2;
    
    int i = 0;
    
restart_point:
    while (i < 10) {
        c += complex_chain(a + i);
        
        /* Data-dependent break */
        if (__builtin_expect_with_probability(
            (c & 0xFF) > 200, 0, 0.3)) {
            a ^= c;
            goto early_exit;
        }
        
        /* Nested do-while with break */
        do {
            b += a;
            if (__builtin_expect_with_probability(
                (b % 7) == 0, 1, 0.4)) {
                break;
            }
            c ^= b;
        } while (0);
        
        i++;
        
        /* Jump back occasionally */
        if (__builtin_expect_with_probability(
            (i == 5), 0, 0.2)) {
            a = complex_chain(c);
            goto restart_point;
        }
    }
    
early_exit:
    return a + b + c;
}

/* Software pipelining style computation */
__attribute__((noinline, optimize("O3")))
static int pipelined_computation(int iterations) {
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int temp1, temp2, temp3, temp4;
    
    for (int i = 0; i < iterations; i++) {
        /* Independent computations that could be pipelined */
        temp1 = complex_chain(i);
        temp2 = complex_chain(i + 1);
        temp3 = complex_chain(i + 2);
        temp4 = complex_chain(i + 3);
        
        /* Mix results */
        acc1 += temp1 ^ temp2;
        acc2 += temp2 * temp3;
        acc3 += temp3 ^ temp4;
        acc4 += temp4 * temp1;
        
        /* Memory barrier every 8 iterations */
        if (__builtin_expect_with_probability(
            (i & 0x7) == 0, 0, 0.25)) {
            asm volatile("" : : : "memory");
        }
    }
    
    return acc1 ^ acc2 ^ acc3 ^ acc4;
}

/* ========== Main Orchestration Function ========== */

int main(void) {
    int result1 = 0, result2 = 0, result3 = 0, result4 = 0;
    int checksum = 0;
    
    /* Initialize global array with pattern */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 3 + 1;
    }
    
    /* Kernel 1: Long chain with data-dependent if break */
    printf("Running kernel 1...\n");
    for (int i = 0; i < 100; i++) {
        int val = g_array[i & 0xFF];
        
        /* Force many local variables for register pressure */
        int v1 = val, v2 = val + 1, v3 = val + 2, v4 = val + 3;
        int v5 = val * 2, v6 = val * 3, v7 = val * 4, v8 = val * 5;
        int v9 = val ^ 0xAA, v10 = val ^ 0xBB, v11 = val ^ 0xCC, v12 = val ^ 0xDD;
        int v13, v14, v15, v16, v17, v18, v19, v20;
        
        /* Complex computation chain */
        v13 = complex_chain(v1);
        v14 = complex_chain(v2);
        v15 = complex_chain(v3);
        v16 = complex_chain(v4);
        
        v17 = v13 + v14;
        v18 = v15 ^ v16;
        v19 = v17 * v18;
        v20 = v19 - v13;
        
        /* Data-dependent exit condition */
        while (v20 > 0) {
            v20 = complex_chain(v20);
            if (__builtin_expect_with_probability(
                (v20 & 0xFFF) == 0, 0, 0.1)) {
                break;
            }
        }
        
        result1 += v20;
    }
    
    /* Kernel 2: Switch with many cases */
    printf("Running kernel 2...\n");
    for (int i = 0; i < 50; i++) {
        result2 += switch_complex(i, i);
        
        /* Pointer aliasing to create uncertainty */
        int *ptr1 = &g_array[i & 0xFF];
        int *ptr2 = (int*)((char*)&g_array[0] + ((i * 4) & 0x3FC));
        
        if (ptr1 != ptr2) {
            *ptr1 += complex_chain(*ptr2);
            *ptr2 ^= complex_chain(*ptr1);
        }
    }
    
    /* Kernel 3: Recursive computation */
    printf("Running kernel 3...\n");
    for (int i = 0; i < 20; i++) {
        result3 += recursive_compute(3, i * 7 + 3);
    }
    
    /* Kernel 4: Irregular control flow */
    printf("Running kernel 4...\n");
    for (int i = 0; i < 30; i++) {
        result4 += irregular_flow(i * 11 + 5);
    }
    
    /* Kernel 5: Software pipelining style */
    printf("Running kernel 5...\n");
    int result5 = pipelined_computation(40);
    
    /* Final checksum computation */
    checksum = result1 ^ result2 ^ result3 ^ result4 ^ result5;
    
    /* Use results to prevent elimination */
    g_results[0] = result1;
    g_results[1] = result2;
    g_results[2] = result3;
    g_results[3] = result4;
    
    printf("Final checksum: %d\n", checksum);
    printf("Results: %d %d %d %d\n", 
           g_results[0], g_results[1], g_results[2], g_results[3]);
    
    return checksum != 0 ? 0 : 1;
}
