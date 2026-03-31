#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_var1 = 42;
volatile int g_var2 = 73;
int g_array[256];

/* Noinline functions to create scheduling boundaries */
__attribute__((noinline, optimize("O3")))
int compute_chain(int seed) {
    volatile int barrier;
    int a = seed, b = seed * 2, c = seed + 1, d = seed ^ 0x55;
    int e = 0, f = 0, g = 0, h = 0;
    
    /* Long chain of dependent operations */
    for (int i = 0; i < 32; i++) {
        a += b * c;
        b ^= d + i;
        c = (c * 3) ^ a;
        d += b - c;
        
        /* Memory barrier to split scheduling regions */
        if (i == 16) {
            asm volatile ("" : : : "memory");
        }
        
        /* Data-dependent exit condition */
        if (a & (1 << (i & 7))) {
            e += a;
            f ^= b;
        }
    }
    
    /* More dependent operations */
    g = a * b + c * d;
    h = (e ^ f) + g;
    
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect_with_probability((h & 0xFF) > 128, 0, 0.7)) {
        barrier = g_var1;
        return h + barrier;
    } else {
        barrier = g_var2;
        return h - barrier;
    }
}

__attribute__((noinline, optimize("O3")))
int recursive_compute(int n, int acc) {
    if (n <= 0) return acc;
    
    int local_vars[8] = {acc, n, n*2, n*3, n^0xAA, n+acc, acc-n, n|acc};
    
    /* Complex computation with many variables */
    for (int i = 0; i < 8; i++) {
        local_vars[i] += local_vars[(i+1)&7] * local_vars[(i+2)&7];
        local_vars[i] ^= local_vars[(i+3)&7];
    }
    
    /* Memory operation with uncertain latency */
    volatile int mem_read = g_array[n & 0xFF];
    
    /* Recursive call - scheduler may save/restore state around this */
    int result = recursive_compute(n - 1, acc + local_vars[n & 7] + mem_read);
    
    /* Post-recursion computation */
    result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
    
    return result;
}

__attribute__((noinline, optimize("O3")))
int switch_complex(int value) {
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0;
    int r7 = 0, r8 = 0, r9 = 0, r10 = 0, r11 = 0, r12 = 0;
    
    /* Large switch to create complex control flow */
    switch (value & 0xF) {
        case 0:
            r1 = value * 2;
            r2 = value ^ 0xFF;
            /* Fall through */
        case 1:
            r3 = r1 + r2;
            r4 = r1 - r2;
            break;
        case 2:
            r5 = value << 3;
            r6 = value >> 2;
            break;
        case 3:
            r7 = value * value;
            r8 = value % 17;
            break;
        case 4:
            r9 = value | 0xAA55;
            r10 = value & 0x55AA;
            break;
        case 5:
            r11 = value + g_var1;
            r12 = value - g_var2;
            break;
        case 6:
            r1 = value * 3;
            r2 = value / 5;
            break;
        case 7:
            r3 = value ^ g_var1;
            r4 = value | g_var2;
            break;
        case 8:
            r5 = ~value;
            r6 = value * 7;
            break;
        case 9:
            r7 = value + 0x1234;
            r8 = value - 0x5678;
            break;
        case 10:
            r9 = value << 2;
            r10 = value >> 4;
            break;
        case 11:
            r11 = value * 11;
            r12 = value % 13;
            break;
        case 12:
            r1 = value + r2;
            r2 = value - r1;
            break;
        case 13:
            r3 = value * r4;
            r4 = value / (r3 ? r3 : 1);
            break;
        case 14:
            r5 = value ^ r6;
            r6 = value & r5;
            break;
        default: /* case 15 */
            r7 = value | r8;
            r8 = value ^ r7;
            break;
    }
    
    /* Merge point with many live variables */
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12;
}

__attribute__((noinline, optimize("O3")))
int loop_with_irregular_exit(int iterations) {
    int sum = 0;
    int i = 0;
    
    /* Loop with irregular control flow using goto */
    start_loop:
    if (i >= iterations) goto end_loop;
    
    /* Data-dependent computation */
    int temp = i * 3 + 1;
    
    /* Conditional break inside do-while(0) */
    do {
        if (temp & 0x100) {
            sum += temp;
            i++;
            goto start_loop;  /* Jump back creates irregular CFG */
        }
        
        /* Normal computation path */
        sum += temp * 2;
        
        /* Another memory barrier */
        asm volatile ("" : : : "memory");
        
        temp = (temp * 1103515245 + 12345) & 0xFF;
        
        if (temp < 64) {
            break;  /* Early exit from do-while */
        }
        
        sum += temp;
    } while (0);
    
    i++;
    goto start_loop;
    
    end_loop:
    return sum;
}

__attribute__((noinline, optimize("O3")))
int software_pipelined_kernel(int *data, int size) {
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int prod1 = 1, prod2 = 1;
    
    /* Manual software pipelining attempt */
    for (int i = 0; i < size; i++) {
        /* Phase 1: Load and initial compute */
        int val = data[i];
        int t1 = val * 2;
        
        /* Phase 2: More computation (could be from previous iteration in real SWP) */
        sum1 += t1;
        int t2 = val + sum2;
        
        /* Phase 3: Even more computation */
        prod1 *= (t2 & 0xFF);
        sum3 ^= val;
        
        /* Rotate sums for next iteration */
        int temp = sum1;
        sum1 = sum2;
        sum2 = sum3;
        sum3 = temp;
        
        /* Use __builtin_expect to influence scheduling */
        if (__builtin_expect((val & 0x3) == 0, 0)) {
            prod2 *= (val ? val : 1);
        }
    }
    
    return sum1 + sum2 + sum3 + prod1 + prod2;
}

int main() {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        g_array[i] = (i * 1103515245 + 12345) & 0xFF;
    }
    
    int result = 0;
    
    /* Kernel 1: Long chain with data-dependent exit */
    result ^= compute_chain(12345);
    
    /* Kernel 2: Recursive computation */
    result += recursive_compute(4, result);
    
    /* Kernel 3: Complex switch statement */
    for (int i = 0; i < 32; i++) {
        result += switch_complex(result + i);
    }
    
    /* Kernel 4: Loop with irregular control flow */
    result ^= loop_with_irregular_exit(100);
    
    /* Kernel 5: Software pipelined style computation */
    int local_data[64];
    for (int i = 0; i < 64; i++) {
        local_data[i] = (result + i * 7) & 0xFFF;
    }
    result += software_pipelined_kernel(local_data, 64);
    
    /* Final mixing to prevent elimination */
    result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
    
    printf("Result: %d\n", result);
    return 0;
}
