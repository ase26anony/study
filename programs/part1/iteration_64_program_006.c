#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to create aliasing and memory dependencies */
volatile int g_counter = 0;
int g_array[256];
int g_results[4] = {0};

/* Prevent inlining to create scheduling region boundaries */
__attribute__((noinline)) 
__attribute__((optimize("O3")))
int compute_chain(int seed, int iterations) {
    int a = seed;
    int b = seed * 2;
    int c = seed + 1;
    int d = seed - 1;
    int e = seed ^ 0x55AA55AA;
    
    /* Long chain of dependent operations */
    for (int i = 0; i < iterations; i++) {
        a = (a * 1103515245 + 12345) & 0x7fffffff;
        b = b ^ (a >> 16);
        c = c + (b & 0xFF);
        d = d * 1664525 + 1013904223;
        e = e ^ d ^ c;
        
        /* Memory barrier to potentially split scheduling regions */
        if (i % 8 == 0) {
            asm volatile("" : : : "memory");
            g_counter++;
        }
        
        /* Data-dependent exit to create uncertainty */
        if (a % 1000000 == 999999) {
            break;
        }
    }
    
    return a + b + c + d + e;
}

/* Another noinline function with switch-based control flow */
__attribute__((noinline))
__attribute__((optimize("O3")))
int switch_computation(int value, int mode) {
    int result = 0;
    int temp1 = value, temp2 = value * 2, temp3 = value / 2;
    int temp4 = value ^ 0x12345678, temp5 = value + 100;
    int temp6 = value - 50, temp7 = value | 0xFF00, temp8 = value & 0x00FF;
    
    /* Complex switch with many cases - may create scheduling merge points */
    switch (mode % 12) {
        case 0:
            result = temp1 + temp2 - temp3;
            /* Use __builtin_expect to influence branch prediction */
            if (__builtin_expect(value > 100, 0)) {
                result *= 2;
            }
            break;
        case 1:
            result = temp2 * temp3 + temp4;
            for (int i = 0; i < 3; i++) {
                result = (result << 3) | (result >> 29);
            }
            break;
        case 2:
            result = temp4 ^ temp5 ^ temp6;
            /* Volatile read creates scheduling uncertainty */
            volatile int v = g_counter;
            result += v;
            break;
        case 3:
            result = temp5 - temp6 + temp7;
            /* Inline asm with memory clobber */
            asm volatile("" : : : "memory");
            break;
        case 4:
            result = temp6 * temp7 / (temp8 + 1);
            break;
        case 5:
            result = temp7 | temp8 | temp1;
            /* Another memory barrier */
            asm volatile("" : : : "memory");
            break;
        case 6:
            result = temp8 & temp2 & temp3;
            break;
        case 7:
            result = (temp1 << 2) + (temp2 >> 3);
            break;
        case 8:
            result = temp3 * 3 + temp4 * 5;
            break;
        case 9:
            result = temp5 ^ 0xAAAAAAAA;
            break;
        case 10:
            result = temp6 + temp7 * 2;
            break;
        case 11:
            result = temp8 - temp1 + 42;
            /* Conditional goto creating irregular CFG */
            if (result < 0) {
                goto adjust_result;
            }
            break;
    }
    
    return result;

adjust_result:
    return result + 1000;
}

/* Recursive function to create call/return scheduling boundaries */
__attribute__((noinline))
__attribute__((optimize("O3")))
int recursive_compute(int n, int depth) {
    if (depth <= 0 || n == 0) {
        return n;
    }
    
    int local_vars[8];
    for (int i = 0; i < 8; i++) {
        local_vars[i] = n + i * depth;
    }
    
    /* Mix of operations creating register pressure */
    int sum = 0;
    sum += local_vars[0] * local_vars[1];
    sum -= local_vars[2] ^ local_vars[3];
    sum += local_vars[4] | local_vars[5];
    sum -= local_vars[6] & local_vars[7];
    
    /* Recursive call */
    int rec_result = recursive_compute(n - 1, depth - 1);
    
    /* More operations after recursion */
    sum = (sum * 6364136223846793005LL) & 0x7FFFFFFF;
    
    return sum + rec_result;
}

/* Function with software-pipelining style loop */
__attribute__((noinline))
__attribute__((optimize("O3")))
void pipeline_style_computation(int* results, int size) {
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int tmp1, tmp2, tmp3, tmp4;
    
    /* Manual software pipelining attempt */
    for (int i = 0; i < size; i++) {
        /* Stage 1: Load and initial computation */
        tmp1 = g_array[i] * 3;
        tmp2 = g_array[i] + 7;
        
        /* Stage 2: More computation (from previous iteration in pipelining) */
        if (i > 0) {
            acc3 = acc1 ^ tmp3;
            acc4 = acc2 | tmp4;
        }
        
        /* Stage 3: Final accumulation */
        tmp3 = tmp1 * 11;
        tmp4 = tmp2 - 5;
        
        acc1 += tmp3;
        acc2 += tmp4;
        
        /* Data-dependent loop exit */
        if (g_array[i] == 0x7FFFFFFF) {
            break;
        }
    }
    
    results[0] = acc1;
    results[1] = acc2;
    results[2] = acc3;
    results[3] = acc4;
}

/* Main orchestrator function */
int main() {
    int final_result = 0;
    
    /* Initialize global array with pattern */
    for (int i = 0; i < 256; i++) {
        g_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Kernel 1: Long chain with data-dependent exit */
    printf("Starting kernel 1...\n");
    for (int i = 0; i < 100; i++) {
        int r = compute_chain(i, 1000);
        final_result ^= r;
        
        /* Use __builtin_expect_with_probability if available */
        #ifdef __builtin_expect_with_probability
        if (__builtin_expect_with_probability(r % 7 == 0, 0, 0.3)) {
            final_result += 1;
        }
        #else
        if (__builtin_expect(r % 7 == 0, 0)) {
            final_result += 1;
        }
        #endif
    }
    
    /* Kernel 2: Switch-based computation */
    printf("Starting kernel 2...\n");
    for (int i = 0; i < 200; i++) {
        int r = switch_computation(i, i);
        final_result += r;
        
        /* Create pointer aliasing to limit scheduler freedom */
        int* p1 = &g_results[0];
        int* p2 = (int*)((char*)&g_results[0] + 0);
        *p1 = r;
        final_result ^= *p2;
    }
    
    /* Kernel 3: Recursive computation */
    printf("Starting kernel 3...\n");
    for (int i = 1; i <= 50; i++) {
        int r = recursive_compute(i, 4);
        final_result = (final_result * 31 + r) & 0x7FFFFFFF;
    }
    
    /* Kernel 4: Software-pipelined style */
    printf("Starting kernel 4...\n");
    pipeline_style_computation(g_results, 256);
    for (int i = 0; i < 4; i++) {
        final_result += g_results[i];
    }
    
    /* Kernel 5: Complex loop with goto creating irregular CFG */
    printf("Starting kernel 5...\n");
    {
        int x = 0, y = 0, z = 0;
        int i = 0;
        
    restart_loop:
        while (i < 100) {
            x = (x * 1664525 + 1013904223) & 0xFFFF;
            y = y ^ x;
            z = z + (y & 0xFF);
            
            /* Irregular control flow with goto */
            if ((x & 0xF) == 0) {
                i++;
                if (z % 19 == 0) {
                    goto restart_loop;
                }
            }
            
            /* do-while with internal break */
            do {
                if (y > 10000) {
                    break;
                }
                y += 1000;
            } while (0);
            
            /* Memory operation with volatile */
            volatile int* vp = &g_counter;
            (*vp)++;
        }
        
        final_result += x + y + z;
    }
    
    printf("Final result: %d\n", final_result);
    printf("Global counter: %d\n", g_counter);
    
    return final_result != 0 ? 0 : 1;
}
