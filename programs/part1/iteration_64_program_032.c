/* haifa_sched_test.c
 * Test program to trigger haifa-sched.cc uncovered lines 4681-4691
 * Compile with: gcc -O3 -fschedule-insns2 -fno-guess-branch-probability -fno-inline haifa_sched_test.c -o haifa_sched_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing to create scheduling complexity */
volatile int global_a = 42;
volatile int global_b = 73;
int global_array[256];

/* ========== Helper Functions with Specific Scheduling Patterns ========== */

/* Long chain of dependent operations with memory barriers */
__attribute__((noinline)) 
unsigned long chain_computation(unsigned long seed, int iterations) {
    unsigned long a = seed;
    unsigned long b = seed * 3;
    unsigned long c = seed + 7;
    unsigned long d = seed ^ 0xDEADBEEF;
    
    /* Create data-dependent loop exit condition */
    int i = 0;
    while (i < iterations) {
        /* Long dependency chain */
        a = (a * 1103515245 + 12345) & 0x7FFFFFFF;
        b = b ^ (a >> 16);
        c = c + (b * 3);
        d = d ^ (c + global_a);  /* Volatile read creates uncertainty */
        
        /* Scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* More operations with dependencies */
        a = a + (d >> 8);
        b = b * 1664525 + 1013904223;
        c = c ^ b;
        d = d + (c * global_b);  /* Another volatile read */
        
        /* Complex conditional that might cause speculative scheduling */
        if (__builtin_expect_with_probability((a & 0xFF) > 200, 0, 0.3)) {
            /* This path is less likely but has different operations */
            a = a >> 4;
            b = b << 2;
            asm volatile("" : : : "memory");
        }
        
        i++;
    }
    
    return a ^ b ^ c ^ d;
}

/* Function with switch statement creating multiple merge points */
__attribute__((noinline, optimize("O3")))
int switch_computation(int mode, int value) {
    int result = value;
    int temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
    int temp5 = 0, temp6 = 0, temp7 = 0, temp8 = 0;
    
    /* Many local variables to create register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    
    switch (mode % 12) {
        case 0:
            v1 = value * 2;
            v2 = value + global_a;
            result = v1 ^ v2;
            asm volatile("" : : : "memory");
            break;
        case 1:
            v3 = value >> 1;
            v4 = value * value;
            result = v3 + v4;
            break;
        case 2:
            v5 = value & 0xFF;
            v6 = value | 0xAA;
            result = v5 * v6;
            break;
        case 3:
            v7 = value ^ 0x55;
            v8 = value + global_b;
            result = v7 - v8;
            asm volatile("" : : : "memory");
            break;
        case 4:
            v9 = value * 3;
            v10 = value / 2;
            result = v9 % (v10 + 1);
            break;
        case 5:
            temp1 = value + v1;
            temp2 = value + v2;
            result = temp1 * temp2;
            break;
        case 6:
            temp3 = value ^ v3;
            temp4 = value | v4;
            result = temp3 & temp4;
            asm volatile("" : : : "memory");
            break;
        case 7:
            temp5 = value - v5;
            temp6 = value + v6;
            result = temp5 ^ temp6;
            break;
        case 8:
            temp7 = value * v7;
            temp8 = value + v8;
            result = temp7 + temp8;
            break;
        case 9:
            result = (value + v9) * (value + v10);
            break;
        case 10:
            result = value << (value & 3);
            asm volatile("" : : : "memory");
            break;
        case 11:
            result = ~value;
            break;
    }
    
    /* Complex merge point with many operations */
    return result + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7 + temp8;
}

/* Recursive function to create call/return scheduling boundaries */
__attribute__((noinline))
int recursive_compute(int depth, int value) {
    if (depth <= 0) {
        return value;
    }
    
    int local1 = value * 2;
    int local2 = value + 1;
    int local3 = value ^ 0xAA;
    
    /* Memory operation between computations */
    asm volatile("" : : : "memory");
    
    /* Recursive calls with different operations */
    int r1 = recursive_compute(depth - 1, local1);
    int r2 = recursive_compute(depth - 1, local2);
    int r3 = recursive_compute(depth - 1, local3);
    
    /* Complex computation at return */
    return (r1 + r2) ^ r3 + global_a;
}

/* Function with irregular control flow using goto */
__attribute__((noinline))
int goto_computation(int limit) {
    int i = 0;
    int sum = 0;
    int counter = 0;
    
restart_point:
    while (i < limit) {
        counter++;
        
        /* Data-dependent break */
        if (__builtin_expect_with_probability(counter > 100, 0, 0.1)) {
            /* Uncommon path with goto */
            i = i / 2;
            goto restart_point;
        }
        
        /* Main computation path */
        sum += (i * 3) ^ (i + global_b);
        
        /* do-while with break */
        do {
            if (__builtin_expect_with_probability((sum & 0xF) == 0, 0, 0.2)) {
                sum += global_a;
                break;
            }
            sum = sum * 1103515245 + 12345;
        } while (0);
        
        i++;
    }
    
    return sum;
}

/* Software pipelining style computation */
__attribute__((noinline))
int pipeline_computation(int *data, int size) {
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int prod1 = 1, prod2 = 1;
    
    for (int i = 0; i < size; i++) {
        /* Independent computations that could be pipelined */
        int val = data[i];
        
        /* Chain 1 */
        sum1 = sum1 + val;
        sum1 = sum1 ^ (val >> 4);
        
        /* Chain 2 */
        sum2 = sum2 ^ val;
        sum2 = sum2 * 1664525;
        
        /* Chain 3 with memory barrier */
        sum3 = sum3 + (val * global_a);
        asm volatile("" : : : "memory");
        sum3 = sum3 ^ 0x55;
        
        /* Product chains */
        prod1 = prod1 * (val & 0xFF);
        prod2 = prod2 * ((val >> 8) & 0xFF);
        
        /* Conditional that might cause state save */
        if (__builtin_expect_with_probability((val & 0x3) == 0, 1, 0.7)) {
            sum1 = sum1 >> 1;
        }
    }
    
    return sum1 + sum2 + sum3 + prod1 + prod2;
}

/* Main orchestrator function */
int main() {
    unsigned long final_result = 0;
    
    /* Initialize global array with pattern */
    for (int i = 0; i < 256; i++) {
        global_array[i] = (i * 1103515245 + 12345) & 0xFF;
    }
    
    printf("Starting haifa-sched stress test...\n");
    
    /* Test 1: Long chain computation with data-dependent exit */
    printf("Running chain computation...\n");
    for (int i = 0; i < 100; i++) {
        final_result ^= chain_computation(final_result + i, 50 + (i % 10));
    }
    
    /* Test 2: Switch with many cases */
    printf("Running switch computation...\n");
    for (int i = 0; i < 200; i++) {
        final_result += switch_computation(i, final_result & 0xFFFF);
    }
    
    /* Test 3: Recursive computation */
    printf("Running recursive computation...\n");
    for (int i = 0; i < 50; i++) {
        final_result ^= recursive_compute(3, final_result + i);
    }
    
    /* Test 4: Goto-based irregular control flow */
    printf("Running goto computation...\n");
    for (int i = 0; i < 20; i++) {
        final_result += goto_computation(100 + i);
    }
    
    /* Test 5: Software pipelining style */
    printf("Running pipeline computation...\n");
    for (int i = 0; i < 10; i++) {
        final_result += pipeline_computation(global_array, 256);
    }
    
    /* Additional complex loop with mixed operations */
    printf("Running mixed computation...\n");
    {
        int a = 1, b = 2, c = 3, d = 4, e = 5;
        int f = 6, g = 7, h = 8, j = 9, k = 10;
        int l = 11, m = 12, n = 13, o = 14, p = 15;
        int q = 16, r = 17, s = 18, t = 19, u = 20;
        
        for (int i = 0; i < 1000; i++) {
            /* Many interdependent operations */
            a = a + b;
            b = b ^ c;
            c = c * d;
            d = d - e;
            e = e & f;
            f = f | g;
            g = g + h;
            h = h ^ j;
            j = j * k;
            k = k - l;
            l = l & m;
            m = m | n;
            n = n + o;
            o = o ^ p;
            p = p * q;
            q = q - r;
            r = r & s;
            s = s | t;
            t = t + u;
            u = u ^ a;
            
            /* Memory barrier every 100 iterations */
            if (__builtin_expect_with_probability((i % 100) == 0, 0, 0.1)) {
                asm volatile("" : : : "memory");
                final_result += a + b + c + d + e + f + g + h + j + k +
                              l + m + n + o + p + q + r + s + t + u;
            }
        }
        
        final_result += a ^ b ^ c ^ d ^ e ^ f ^ g ^ h ^ j ^ k ^
                       l ^ m ^ n ^ o ^ p ^ q ^ r ^ s ^ t ^ u;
    }
    
    printf("Final checksum: 0x%016lx\n", final_result);
    printf("Test completed.\n");
    
    return (final_result == 0) ? 0 : 1;
}
