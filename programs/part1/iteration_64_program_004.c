#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_var1 = 1;
volatile int g_var2 = 2;
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
    b ^= d << 2;
    c = (c + e) * f;
    d = d ^ g ^ h;
    e += i * j;
    f = f ^ k ^ l;
    g = (g + m) * n;
    h ^= o << 3;
    
    /* Memory barrier to split scheduling region */
    asm volatile("" : : : "memory");
    
    /* More dependent operations */
    i = i + j + k;
    j = j * l * m;
    k = k ^ n ^ o;
    l = (l + p) * a;
    m = m ^ b ^ c;
    n = n + d + e;
    o = o * f * g;
    p = p ^ h ^ i;
    
    /* Another memory barrier */
    barrier = a;
    asm volatile("" : : : "memory");
    
    /* Final computation with data-dependent exit */
    int result = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
    return result;
}

__attribute__((noinline, optimize("O3")))
int recursive_compute(int depth, int val) {
    if (__builtin_expect_with_probability(depth <= 0, 0, 0.7)) {
        return val;
    }
    
    int local1 = val * 3;
    int local2 = val + 7;
    int local3 = val ^ 0x55AA;
    
    /* Create register pressure */
    int t1 = local1 * local2;
    int t2 = local2 ^ local3;
    int t3 = local3 + local1;
    int t4 = t1 ^ t2;
    int t5 = t2 * t3;
    int t6 = t3 + t4;
    int t7 = t4 ^ t5;
    int t8 = t5 * t6;
    
    /* Recursive call */
    int rec_result = recursive_compute(depth - 1, t7 + t8);
    
    /* Complex merge point computation */
    return rec_result + t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8;
}

__attribute__((noinline, optimize("O3")))
int switch_complex(int selector, int base) {
    int r1 = base, r2 = base + 1, r3 = base + 2, r4 = base + 3;
    int r5 = base + 4, r6 = base + 5, r7 = base + 6, r8 = base + 7;
    int r9 = base + 8, r10 = base + 9, r11 = base + 10, r12 = base + 11;
    
    /* Switch with many cases - creates complex control flow */
    switch (selector & 0xF) {
        case 0:
            r1 = r2 * r3;
            r4 = r5 ^ r6;
            r7 = r8 + r9;
            break;
        case 1:
            r2 = r3 + r4;
            r5 = r6 * r7;
            r8 = r9 ^ r10;
            break;
        case 2:
            r3 = r4 ^ r5;
            r6 = r7 + r8;
            r9 = r10 * r11;
            break;
        case 3:
            r4 = r5 * r6;
            r7 = r8 ^ r9;
            r10 = r11 + r12;
            break;
        case 4:
            r5 = r6 + r7;
            r8 = r9 * r10;
            r11 = r12 ^ r1;
            break;
        case 5:
            r6 = r7 ^ r8;
            r9 = r10 + r11;
            r12 = r1 * r2;
            break;
        case 6:
            r7 = r8 * r9;
            r10 = r11 ^ r12;
            r1 = r2 + r3;
            break;
        case 7:
            r8 = r9 + r10;
            r11 = r12 * r1;
            r2 = r3 ^ r4;
            break;
        case 8:
            r9 = r10 ^ r11;
            r12 = r1 + r2;
            r3 = r4 * r5;
            break;
        case 9:
            r10 = r11 * r12;
            r1 = r2 ^ r3;
            r4 = r5 + r6;
            break;
        case 10:
            r11 = r12 + r1;
            r2 = r3 * r4;
            r5 = r6 ^ r7;
            break;
        case 11:
            r12 = r1 ^ r2;
            r3 = r4 + r5;
            r6 = r7 * r8;
            break;
        case 12:
            r1 = r2 * r3;
            r4 = r5 ^ r6;
            r7 = r8 + r9;
            break;
        case 13:
            r2 = r3 + r4;
            r5 = r6 * r7;
            r8 = r9 ^ r10;
            break;
        case 14:
            r3 = r4 ^ r5;
            r6 = r7 + r8;
            r9 = r10 * r11;
            break;
        default: /* case 15 */
            r4 = r5 * r6;
            r7 = r8 ^ r9;
            r10 = r11 + r12;
            break;
    }
    
    /* Memory clobber at switch merge point */
    asm volatile("" : : : "memory");
    
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12;
}

__attribute__((noinline, optimize("O3")))
int loop_with_break(int iterations) {
    int sum = 0;
    int i = 0;
    
    /* Loop with data-dependent break */
    do {
        if (__builtin_expect_with_probability(g_array[i] != 0, 0, 0.3)) {
            /* Early exit path - scheduler may save state here */
            sum += g_var1 * g_var2;
            break;
        }
        
        /* Dependent operations inside loop */
        int t1 = i * 3;
        int t2 = i ^ 0x1234;
        int t3 = t1 + t2;
        int t4 = t2 * t3;
        int t5 = t3 ^ t4;
        
        sum += t5;
        
        /* Memory operation with volatile */
        g_array[i] = sum & 0xFF;
        
        /* Scheduling barrier */
        asm volatile("" : : : "memory");
        
        i++;
    } while (i < iterations);
    
    return sum;
}

__attribute__((noinline, optimize("O3")))
int nested_loops(int outer, int inner) {
    int total = 0;
    
    for (int i = 0; i < outer; i++) {
        /* Software pipelining style computation */
        int acc1 = 0, acc2 = 0, acc3 = 0;
        
        for (int j = 0; j < inner; j++) {
            /* Independent computations that could be pipelined */
            acc1 += i * j + g_var1;
            acc2 ^= i ^ j ^ g_var2;
            acc3 = (acc3 + j) * 3;
            
            /* Create pointer aliasing confusion */
            int* ptr1 = (int*)&g_var1;
            int* ptr2 = (int*)&g_var2;
            if ((i ^ j) & 1) {
                *ptr1 += 1;
            } else {
                *ptr2 += 1;
            }
        }
        
        total += acc1 + acc2 + acc3;
        
        /* Irregular control flow with goto */
        if (__builtin_expect_with_probability((i & 3) == 0, 1, 0.6)) {
            /* Jump back to create non-trivial CFG */
            if (total > 1000) {
                i--;
                continue;
            }
        }
    }
    
    return total;
}

int main() {
    int result = 0;
    
    /* Initialize global array with pattern */
    for (int i = 0; i < 256; i++) {
        g_array[i] = (i * 37) & 0xFF;
    }
    
    /* Kernel 1: Long chain with data-dependent exit */
    result ^= compute_chain(42);
    
    /* Kernel 2: Recursive computation */
    result += recursive_compute(4, result & 0xFF);
    
    /* Kernel 3: Complex switch statement */
    for (int i = 0; i < 32; i++) {
        result ^= switch_complex(i, result);
    }
    
    /* Kernel 4: Loop with early break */
    result += loop_with_break(128);
    
    /* Kernel 5: Nested loops with software pipelining pattern */
    result += nested_loops(16, 32);
    
    /* Kernel 6: Manual software pipelining simulation */
    {
        int a = 1, b = 2, c = 3, d = 4;
        int stage1 = 0, stage2 = 0, stage3 = 0;
        
        for (int i = 0; i < 100; i++) {
            /* Pipeline stage 1 */
            stage1 = a * b + c;
            
            /* Pipeline stage 2 (depends on stage1 from previous iteration) */
            stage2 = stage2 ^ stage1;
            
            /* Pipeline stage 3 (depends on stage2 from previous iteration) */
            stage3 = stage3 + stage2 * d;
            
            /* Rotate values */
            a = b;
            b = c;
            c = d;
            d = stage3 & 0xFF;
            
            /* Memory clobber between pipeline stages */
            asm volatile("" : : : "memory");
        }
        result += stage1 + stage2 + stage3;
    }
    
    /* Kernel 7: Unrolled loop with many variables */
    {
        int v[24];
        for (int i = 0; i < 24; i++) v[i] = i + result;
        
        /* Partially unrolled computation */
        for (int i = 0; i < 20; i += 4) {
            v[i] = v[i] * v[i+1] + v[i+2];
            v[i+1] = v[i+1] ^ v[i+3] ^ v[i];
            v[i+2] = v[i+2] + v[i] * v[i+1];
            v[i+3] = v[i+3] ^ v[i+2] ^ v[i+1];
            
            /* Conditional goto creating irregular control flow */
            if (__builtin_expect_with_probability(v[i] > 1000, 0, 0.2)) {
                i -= 2;
                continue;
            }
        }
        
        int sum = 0;
        for (int i = 0; i < 24; i++) sum += v[i];
        result ^= sum;
    }
    
    printf("Final result: %d\n", result);
    return 0;
}
