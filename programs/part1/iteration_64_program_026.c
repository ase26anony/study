/* haifa-sched-context-test.c
 * Designed to trigger scheduler context save/restore in haifa-sched.cc
 * Compile with: gcc -O3 -fschedule-insns2 -fno-guess-branch-probability -fno-inline -fsel-sched-pipelining haifa-sched-context-test.c -o haifa-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for creating aliasing and memory dependencies */
volatile int g_mem_barrier = 0;
int g_array[256] = {0};
int g_counter = 0;

/* Prevent inlining to create clear scheduling boundaries */
__attribute__((noinline)) 
long compute_chain(int seed, int iterations) {
    long a = seed;
    long b = seed * 2;
    long c = seed * 3;
    volatile int* barrier = &g_mem_barrier;
    
    /* Long chain of dependent operations */
    for (int i = 0; i < iterations; i++) {
        /* Data-dependent control flow that scheduler can't perfectly predict */
        if (__builtin_expect_with_probability((a & 0xFF) == 0, 0, 0.3)) {
            /* Memory barrier to potentially trigger scheduler state save */
            asm volatile("" : : : "memory");
            a = (a * 1103515245 + 12345) & 0x7FFFFFFF;
        }
        
        /* Dependent computation chain */
        a = a ^ (b << 3);
        b = b + (c >> 2);
        c = c ^ a;
        a = a * 6364136223846793005ULL;
        b = b - c;
        c = c + (a & 0xFFF);
        
        /* Another potential state save point */
        if (__builtin_expect_with_probability((i & 0x7) == 0, 0, 0.2)) {
            *barrier = i;  /* Volatile write */
            a = a ^ (*barrier);
        }
    }
    
    return a + b + c;
}

__attribute__((noinline))
int recursive_scheduler_stress(int depth, int value) {
    if (depth <= 0) {
        /* Base case with computation */
        volatile int temp = value;
        asm volatile("" : "+r"(value) : : "memory");
        return value ^ 0x55AA55AA;
    }
    
    /* Create multiple paths with different computation patterns */
    int result = 0;
    if (value & 1) {
        result = recursive_scheduler_stress(depth - 1, value * 3 + 1);
        result ^= value;
    } else {
        result = recursive_scheduler_stress(depth - 1, value / 2);
        result += value * 7;
    }
    
    /* Additional computation after recursion return */
    for (int i = 0; i < 4; i++) {
        result = (result << 5) | (result >> 27);
        result ^= i * 0x9E3779B9;
    }
    
    return result;
}

__attribute__((noinline, optimize("O3")))
void complex_switch_scheduler(int init, int* results) {
    int a = init, b = init * 2, c = init * 3, d = init * 4;
    int e = init * 5, f = init * 6, g = init * 7, h = init * 8;
    int i = init * 9, j = init * 10, k = init * 11, l = init * 12;
    
    /* Switch with many cases - creates complex control flow */
    switch (init & 0xF) {
        case 0:
            a = b + c; d = e ^ f; g = h * i;
            asm volatile("" : : : "memory");
            break;
        case 1:
            b = c - d; e = f & g; h = i | j;
            break;
        case 2:
            c = d * e; f = g ^ h; i = j << 2;
            break;
        case 3:
            d = e / (f + 1); g = h + i; j = k - l;
            break;
        case 4:
            e = f % 17; h = i * j; k = l ^ a;
            break;
        case 5:
            f = g << 3; i = j >> 1; l = a & b;
            break;
        case 6:
            g = h | c; j = k ^ d; a = b + e;
            break;
        case 7:
            h = i & 0xFF; k = l | 0xF0; b = c * d;
            break;
        case 8:
            i = j - k; l = a ^ b; c = d + e;
            break;
        case 9:
            j = k * l; a = b - c; d = e ^ f;
            break;
        case 10:
            k = l / 3; b = c & d; e = f | g;
            break;
        case 11:
            l = a << 1; c = d ^ e; f = g + h;
            break;
        case 12:
            a = b % 19; d = e * f; g = h - i;
            break;
        case 13:
            b = c | d; e = f & g; h = i ^ j;
            break;
        case 14:
            c = d + e; f = g - h; i = j * k;
            break;
        case 15:
            d = e ^ f; g = h | i; j = k & l;
            volatile int barrier = g_mem_barrier;
            asm volatile("" : : "r"(barrier) : "memory");
            break;
    }
    
    /* Store all results to prevent elimination */
    results[0] = a; results[1] = b; results[2] = c; results[3] = d;
    results[4] = e; results[5] = f; results[6] = g; results[7] = h;
    results[8] = i; results[9] = j; results[10] = k; results[11] = l;
}

__attribute__((noinline))
void software_pipelined_loop(int* data, int size) {
    int reg1, reg2, reg3, reg4;
    int *ptr = data;
    
    /* Manual software pipelining attempt */
    reg1 = *ptr++;
    reg2 = *ptr++;
    
    for (int i = 2; i < size; i++) {
        /* Load next value early */
        reg3 = *ptr++;
        
        /* Process previous values */
        reg4 = reg1 * reg2;
        reg4 ^= reg3;
        
        /* Store result with barrier */
        if (__builtin_expect((i & 0x3F) == 0, 0)) {
            asm volatile("" : : : "memory");
            data[i-2] = reg4;
        } else {
            data[i-2] = reg4 + i;
        }
        
        /* Shift registers */
        reg1 = reg2;
        reg2 = reg3;
    }
    
    /* Handle remaining elements */
    data[size-2] = reg1 ^ reg2;
    data[size-1] = reg2;
}

__attribute__((noinline))
int irregular_control_flow(int start) {
    int x = start;
    int y = start * 2;
    int z = start * 3;
    
    /* Irregular loop with goto */
    int counter = 0;
    
loop_start:
    x = (x * 1103515245 + 12345) & 0x7FFFFFFF;
    
    /* do-while with internal break */
    do {
        y = y ^ x;
        if (__builtin_expect_with_probability((y & 0xFF) == 0, 0, 0.1)) {
            /* Memory operation that might trigger scheduler state save */
            g_array[counter & 0xFF] = y;
            break;
        }
        z = z + y;
        counter++;
    } while (0);
    
    x = x + z;
    
    /* Conditional backward jump */
    if (__builtin_expect(counter < 100, 1)) {
        if ((x & 0x3FF) != 0) {
            goto loop_start;
        }
    }
    
    return x + y + z;
}

int main() {
    unsigned long long checksum = 0;
    int switch_results[12];
    int loop_data[128];
    
    /* Initialize data */
    for (int i = 0; i < 128; i++) {
        loop_data[i] = i * 3 + 1;
    }
    for (int i = 0; i < 256; i++) {
        g_array[i] = i;
    }
    
    printf("Starting scheduler context test...\n");
    
    /* Test 1: Long computation chain with probabilistic branches */
    for (int i = 0; i < 50; i++) {
        long result = compute_chain(i + 1, 100 + (i % 10));
        checksum += result;
        checksum = (checksum << 13) | (checksum >> 51);
    }
    
    /* Test 2: Complex switch statement */
    for (int i = 0; i < 100; i++) {
        complex_switch_scheduler(i, switch_results);
        for (int j = 0; j < 12; j++) {
            checksum ^= switch_results[j];
            checksum = checksum * 6364136223846793005ULL + 1;
        }
    }
    
    /* Test 3: Software pipelined loop */
    software_pipelined_loop(loop_data, 128);
    for (int i = 0; i < 128; i++) {
        checksum += loop_data[i];
        checksum = (checksum << 7) | (checksum >> 57);
    }
    
    /* Test 4: Irregular control flow */
    for (int i = 0; i < 75; i++) {
        int result = irregular_control_flow(i * 7 + 3);
        checksum += result;
        checksum = checksum ^ (checksum >> 32);
    }
    
    /* Test 5: Recursive calls */
    for (int i = 1; i <= 20; i++) {
        int result = recursive_scheduler_stress(3 + (i % 3), i * 100);
        checksum = checksum * 31 + result;
    }
    
    /* Final computation to use all results */
    checksum += g_counter;
    for (int i = 0; i < 256; i += 16) {
        checksum ^= g_array[i];
    }
    
    printf("Final checksum: %llu\n", checksum);
    printf("Test completed. If scheduler context was saved/restored,\n");
    printf("the uncovered cleanup code in haifa-sched.cc should have been executed.\n");
    
    return (checksum & 0xFFFFFFFF) == 0 ? 1 : 0;
}
