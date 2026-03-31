#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_var1 = 42;
volatile int g_var2 = 73;
int g_array[256] = {0};

/* Helper functions marked noinline to prevent inlining */
__attribute__((noinline, optimize("O3")))
int compute_chain(int seed) {
    volatile int barrier;
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    int i = seed + 8, j = seed + 9, k = seed + 10, l = seed + 11;
    int m = seed + 12, n = seed + 13, o = seed + 14, p = seed + 15;
    
    /* Long chain of dependent operations */
    a += b * c;
    d ^= a >> 3;
    e = d * f + g;
    h = (e & 0xFF) | (i << 8);
    j = k * l - m;
    n = o ^ p;
    
    /* Memory barrier to split scheduling region */
    asm volatile("" : : : "memory");
    
    /* More dependent operations */
    a += j * n;
    b ^= h & 0xAA;
    c = d * e / (f + 1);
    g = (h | i) & (j ^ k);
    
    /* Data-dependent loop with unpredictable exit */
    int counter = 0;
    while (__builtin_expect_with_probability(counter < 5, 1, 0.7)) {
        a += b;
        b ^= c;
        c *= d;
        d -= e;
        counter++;
        
        /* Memory operation with volatile */
        barrier = g_var1;
        if (__builtin_expect_with_probability(barrier > 100, 0, 0.3)) {
            break;
        }
    }
    
    /* Final computation */
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
}

__attribute__((noinline, optimize("O3")))
int recursive_compute(int depth, int val) {
    if (__builtin_expect_with_probability(depth <= 0, 0, 0.2)) {
        return val;
    }
    
    int local1 = val * 3;
    int local2 = val + 7;
    int local3 = val ^ 0x55;
    
    /* Create register pressure */
    int t1 = local1 * local2;
    int t2 = local2 ^ local3;
    int t3 = local3 + local1;
    int t4 = t1 & t2;
    int t5 = t2 | t3;
    int t6 = t3 ^ t4;
    int t7 = t4 * t5;
    int t8 = t5 - t6;
    int t9 = t6 / (t7 + 1);
    int t10 = t7 ^ t8;
    
    /* Recursive call */
    int result = recursive_compute(depth - 1, t10);
    
    /* Post-recursion computation */
    result += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9;
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

__attribute__((noinline, optimize("O3")))
int switch_complex(int selector, int base) {
    int r1 = base, r2 = base + 1, r3 = base + 2, r4 = base + 3;
    int r5 = base + 4, r6 = base + 5, r7 = base + 6, r8 = base + 7;
    int r9 = base + 8, r10 = base + 9, r11 = base + 10, r12 = base + 11;
    
    /* Complex switch with many cases */
    switch (selector & 0xF) {
        case 0:
            r1 += r2 * r3;
            r4 ^= r5;
            /* Fall through */
        case 1:
            r6 = r7 & r8;
            r9 |= r10;
            break;
        case 2:
            r2 *= r3;
            r4 -= r5;
            r6 ^= r7;
            break;
        case 3:
            r8 = r9 + r10;
            r11 = r12 * r1;
            break;
        case 4:
            r3 = r4 | r5;
            r6 &= r7;
            r8 ^= r9;
            break;
        case 5:
            r10 += r11;
            r12 *= r2;
            r3 -= r4;
            break;
        case 6:
            r5 = r6 ^ r7;
            r8 |= r9;
            r10 &= r11;
            break;
        case 7:
            r12 += r1;
            r2 *= r3;
            r4 ^= r5;
            break;
        case 8:
            r6 = r7 - r8;
            r9 |= r10;
            r11 &= r12;
            break;
        case 9:
            r1 ^= r2;
            r3 += r4;
            r5 *= r6;
            break;
        case 10:
            r7 = r8 & r9;
            r10 |= r11;
            r12 ^= r1;
            break;
        case 11:
            r2 += r3;
            r4 *= r5;
            r6 ^= r7;
            break;
        case 12:
            r8 = r9 | r10;
            r11 &= r12;
            r1 += r2;
            break;
        case 13:
            r3 *= r4;
            r5 ^= r6;
            r7 |= r8;
            break;
        case 14:
            r9 = r10 - r11;
            r12 &= r1;
            r2 ^= r3;
            break;
        default:  /* case 15 */
            r4 += r5;
            r6 *= r7;
            r8 ^= r9;
            break;
    }
    
    /* Merge point computation */
    int sum = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12;
    
    /* Irregular control flow with goto */
    if (__builtin_expect_with_probability(sum & 1, 1, 0.6)) {
        goto compute_more;
    }
    
    return sum;
    
compute_more:
    /* Additional computation after goto */
    r1 ^= r2;
    r3 += r4;
    r5 *= r6;
    return r1 + r3 + r5 + sum;
}

__attribute__((noinline, optimize("O3")))
int loop_nest_compute(int iterations) {
    int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Software pipelining style */
        int a = i, b = i + 1, c = i + 2, d = i + 3;
        int e = i + 4, f = i + 5, g = i + 6, h = i + 7;
        
        /* Inner computation loop */
        do {
            a += b;
            b ^= c;
            c *= d;
            d -= e;
            
            /* Conditional break */
            if (__builtin_expect_with_probability(a > 1000, 0, 0.1)) {
                break;
            }
            
            e = f & g;
            f |= h;
            g ^= a;
            h += b;
            
            /* Memory operation */
            volatile int temp = g_var2;
            if (temp & 1) {
                c += temp;
            }
            
        } while (0);  /* do-while(0) with internal break */
        
        total += a + b + c + d + e + f + g + h;
        
        /* Pointer aliasing to create uncertainty */
        int *ptr1 = (int*)&g_var1;
        int *ptr2 = (int*)&g_array[i & 0xFF];
        *ptr2 = *ptr1 + i;
    }
    
    return total;
}

int main() {
    int result = 0;
    
    printf("Starting scheduler stress test...\n");
    
    /* Kernel 1: Long chain with data-dependent loop */
    result ^= compute_chain(1);
    
    /* Kernel 2: Complex switch statement */
    for (int i = 0; i < 20; i++) {
        result += switch_complex(i, result);
    }
    
    /* Kernel 3: Recursive computation */
    result ^= recursive_compute(4, result);
    
    /* Kernel 4: Nested loop with software pipelining style */
    result += loop_nest_compute(50);
    
    /* Additional mixed pattern */
    for (int i = 0; i < 100; i++) {
        int local_vars[20];
        
        /* Initialize many local variables */
        for (int j = 0; j < 20; j++) {
            local_vars[j] = result + i + j;
        }
        
        /* Complex computation with many dependencies */
        for (int j = 1; j < 19; j++) {
            local_vars[j] += local_vars[j-1] * local_vars[j+1];
            local_vars[j] ^= (local_vars[j] >> 3);
            
            /* Insert scheduling barrier periodically */
            if (j % 5 == 0) {
                asm volatile("" : : : "memory");
            }
        }
        
        /* Aggregate results */
        for (int j = 0; j < 20; j++) {
            result += local_vars[j];
        }
        
        /* Unpredictable branch */
        if (__builtin_expect_with_probability(result & 0x100, 0, 0.4)) {
            result >>= 1;
        }
    }
    
    printf("Result checksum: %d\n", result);
    
    /* Ensure computations aren't eliminated */
    if (result == 0) {
        printf("Unexpected zero result\n");
    }
    
    return 0;
}
