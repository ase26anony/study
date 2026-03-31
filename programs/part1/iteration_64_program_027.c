#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_var1 = 42;
volatile int g_var2 = 73;
int g_array[256];

/* Helper functions with specific attributes */
__attribute__((noinline, optimize("O3")))
static int compute_chain(int seed) {
    volatile int barrier;
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    int i = seed + 8, j = seed + 9, k = seed + 10, l = seed + 11;
    int m = seed + 12, n = seed + 13, o = seed + 14, p = seed + 15;
    int q = seed + 16, r = seed + 17, s = seed + 18, t = seed + 19;
    
    /* Long chain of dependent operations */
    a += b * c;
    d ^= e | f;
    g *= h + i;
    j -= k ^ l;
    m &= n | o;
    p += q * r;
    s ^= t + a;
    
    /* Memory barrier to split scheduling regions */
    asm volatile("" : : : "memory");
    
    /* More dependent operations */
    b += c * d;
    e ^= f | g;
    h *= i + j;
    k -= l ^ m;
    n &= o | p;
    q += r * s;
    t ^= a + b;
    
    /* Volatile read creates uncertainty */
    barrier = g_var1;
    
    /* Final computation with branch probability hint */
    if (__builtin_expect_with_probability(barrier > 0, 1, 0.7)) {
        a += barrier * 3;
    } else {
        a -= barrier * 2;
    }
    
    return a + b + c + d + e + f + g + h + i + j + 
           k + l + m + n + o + p + q + r + s + t;
}

__attribute__((noinline, optimize("O3")))
static int switch_complex(int val) {
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
    int r6 = 0, r7 = 0, r8 = 0, r9 = 0, r10 = 0;
    
    /* Complex switch with many cases */
    switch (val & 0xF) {
        case 0:
            r1 = val * 2;
            r2 = val + 1;
            asm volatile("" : : : "memory");
            break;
        case 1:
            r3 = val ^ 0xAAAA;
            r4 = val | 0x5555;
            break;
        case 2:
            r5 = val << 3;
            r6 = val >> 2;
            break;
        case 3:
            r7 = val * val;
            r8 = val % 17;
            break;
        case 4:
            r9 = val + g_var2;
            r10 = val - g_var2;
            break;
        case 5:
            r1 = val & 0xFF;
            r2 = val | 0xFF00;
            break;
        case 6:
            r3 = val * 3;
            r4 = val / 3;
            break;
        case 7:
            r5 = val ^ val;
            r6 = val | val;
            break;
        case 8:
            r7 = val << 1;
            r8 = val >> 1;
            break;
        case 9:
            r9 = val + 999;
            r10 = val - 999;
            break;
        case 10:
            r1 = val * 5;
            r2 = val % 5;
            break;
        case 11:
            r3 = val ^ 0x1234;
            r4 = val & 0x4321;
            break;
        case 12:
            r5 = val << 4;
            r6 = val >> 4;
            break;
        case 13:
            r7 = val * 7;
            r8 = val / 7;
            break;
        case 14:
            r9 = val + 777;
            r10 = val - 777;
            break;
        default:  /* case 15 */
            r1 = val * val * val;
            r2 = val ^ 0xDEAD;
            break;
    }
    
    /* Merge point with data-dependent computation */
    int result = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    
    /* Loop with irregular control flow */
    int counter = 0;
    volatile int* ptr = &g_var1;
    
    do {
        if (__builtin_expect_with_probability(*ptr > 0, 1, 0.6)) {
            result += counter * 2;
            if (counter > 5) break;
        } else {
            result -= counter;
        }
        counter++;
        
        /* Goto creating irregular CFG */
        if (counter == 3) {
            goto special_case;
        }
        
        continue;
        
    special_case:
        result ^= 0xABCD;
    } while (counter < 10);
    
    return result;
}

__attribute__((noinline, optimize("O3")))
static int nested_loops(int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        volatile int temp = g_array[i & 255];
        
        /* Inner loop with data-dependent exit */
        int j = 0;
        while (g_array[j] != 0 && j < 100) {
            sum += i * j + temp;
            j++;
            
            /* Memory clobber affecting scheduling */
            asm volatile("" : : : "memory");
        }
        
        /* Software pipelining style computation */
        int a = sum, b = sum + 1, c = sum + 2;
        for (int k = 0; k < 4; k++) {
            a = (a * 3) ^ b;
            b = (b + 7) & c;
            c = (c << 1) | a;
            
            if (__builtin_expect_with_probability(k == 2, 0, 0.3)) {
                a += g_var2;
            }
        }
        sum = a + b + c;
    }
    
    return sum;
}

/* Recursive function to create return state restoration points */
__attribute__((noinline, optimize("O3")))
static int recursive_compute(int depth, int val) {
    if (depth <= 0) {
        return val;
    }
    
    int local1 = val * 2;
    int local2 = val + depth;
    int local3 = val ^ 0x1234;
    int local4 = val & 0xABCD;
    int local5 = val | 0xDCBA;
    
    /* Mix of operations before recursion */
    local1 += local2 * local3;
    local4 ^= local5 | local1;
    
    /* Recursive call - scheduler may save state here */
    int rec_result = recursive_compute(depth - 1, local1 + local4);
    
    /* Operations after return - scheduler may restore state */
    local2 = rec_result * 3;
    local3 = rec_result + g_var1;
    
    /* Memory barrier before final computation */
    asm volatile("" : : : "memory");
    
    return local2 + local3 + rec_result;
}

__attribute__((optimize("O3")))
int main() {
    /* Initialize array with non-zero values */
    for (int i = 0; i < 256; i++) {
        g_array[i] = (i * 37 + 13) & 0xFF;
    }
    
    int checksum = 0;
    
    /* Kernel 1: Long chain with branch probability hints */
    checksum += compute_chain(1);
    
    /* Kernel 2: Complex switch with irregular control flow */
    for (int i = 0; i < 50; i++) {
        checksum += switch_complex(i);
    }
    
    /* Kernel 3: Nested loops with memory barriers */
    checksum += nested_loops(100);
    
    /* Kernel 4: Recursive computation */
    checksum += recursive_compute(4, 42);
    
    /* Additional stress: pointer aliasing */
    int* alias1 = (int*)&g_var1;
    int* alias2 = (int*)&g_var2;
    
    for (int i = 0; i < 1000; i++) {
        *alias1 += i;
        *alias2 ^= i;
        
        /* Create data dependency through aliasing */
        if (__builtin_expect_with_probability(*alias1 != *alias2, 1, 0.8)) {
            checksum += *alias1 * *alias2;
        }
        
        /* Loop with unpredictable exit */
        int j = 0;
        while (j < 10 && g_array[(i + j) & 255] > 0) {
            checksum += j;
            j++;
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
