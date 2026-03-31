#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_counter = 0;
int g_array[256];
int g_result = 0;

/* Helper functions marked noinline to prevent inlining */
__attribute__((noinline, optimize("O3")))
int compute_chain(int seed) {
    volatile int barrier;
    int a = seed, b = seed * 2, c = seed * 3, d = seed * 4;
    int e = seed * 5, f = seed * 6, g = seed * 7, h = seed * 8;
    
    /* Long chain of dependent operations */
    a += b ^ c;
    b = c * d + a;
    c = d ^ e + b;
    d = e * f + c;
    e = f ^ g + d;
    f = g * h + e;
    g = h ^ a + f;
    h = a * b + g;
    
    /* Memory barrier to split scheduling region */
    asm volatile("" : : : "memory");
    
    /* More dependent operations */
    a = b + c * d;
    b = c + d ^ e;
    c = d + e * f;
    d = e + f ^ g;
    e = f + g * h;
    f = g + h ^ a;
    g = h + a * b;
    h = a + b ^ c;
    
    barrier = a; /* Volatile write */
    
    /* Final computation with data-dependent exit */
    int i = 0;
    while (i < 8) {
        a = (a * 1103515245 + 12345) & 0x7fffffff;
        if (__builtin_expect_with_probability(a & 0x100, 0, 0.3)) {
            /* Unlikely path that might trigger state save */
            b = c ^ d;
            c = d * e;
            d = e ^ f;
        }
        i += (a & 1);
    }
    
    return a + b + c + d + e + f + g + h;
}

__attribute__((noinline, optimize("O3")))
int recursive_compute(int n, int acc) {
    if (n <= 0) return acc;
    
    int local_vars[8];
    for (int i = 0; i < 8; i++) {
        local_vars[i] = (acc + i) * n;
    }
    
    /* Mix of operations that create register pressure */
    int t1 = local_vars[0] + local_vars[1];
    int t2 = local_vars[2] * local_vars[3];
    int t3 = local_vars[4] ^ local_vars[5];
    int t4 = local_vars[6] - local_vars[7];
    
    /* Memory clobber to affect scheduling */
    asm volatile("" : : : "memory");
    
    int new_acc = t1 + t2 + t3 + t4;
    
    /* Recursive call - return point may need state restoration */
    return recursive_compute(n - 1, new_acc);
}

__attribute__((noinline, optimize("O3")))
int switch_complex(int val) {
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0;
    int r7 = 0, r8 = 0, r9 = 0, r10 = 0, r11 = 0, r12 = 0;
    
    /* Switch with many cases - creates complex CFG */
    switch (val & 0xF) {
        case 0:
            r1 = val * 2;
            r2 = val + 1;
            /* Fall through */
        case 1:
            r3 = val ^ 0xAAAA;
            r4 = val | 0x5555;
            break;
        case 2:
            r5 = val << 3;
            r6 = val >> 2;
            break;
        case 3:
            r7 = val * 3;
            r8 = val / 2;
            break;
        case 4:
            r9 = val + 100;
            r10 = val - 50;
            break;
        case 5:
            r11 = val & 0xFF00;
            r12 = val | 0x00FF;
            break;
        case 6:
            r1 = r2 = val * val;
            break;
        case 7:
            r3 = r4 = val + val;
            break;
        case 8:
            r5 = r6 = val ^ val;
            break;
        case 9:
            r7 = r8 = ~val;
            break;
        case 10:
            r9 = r10 = val * 7;
            break;
        case 11:
            r11 = r12 = val % 13;
            break;
        case 12:
            r1 = r3 = r5 = val;
            break;
        case 13:
            r2 = r4 = r6 = val + 1;
            break;
        case 14:
            r7 = r9 = r11 = val * 2;
            break;
        case 15:
            r8 = r10 = r12 = val / 2;
            break;
    }
    
    /* Merge point with many live variables */
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12;
}

__attribute__((noinline, optimize("O3")))
int loop_with_inner_function(int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Data-dependent loop exit */
        if (__builtin_expect_with_probability(i > iterations / 2, 0, 0.2)) {
            /* Early exit path - scheduler may save state here */
            sum += i * 3;
            continue;
        }
        
        /* Call to small function creates scheduling region boundary */
        int temp = compute_chain(i);
        
        /* Software pipelining style computation */
        do {
            if (temp & 1) {
                sum += temp;
                break; /* Creates internal control flow edge */
            }
            temp >>= 1;
        } while (temp > 0);
        
        /* Irregular control flow with goto */
        if (sum & 0x100) {
            goto add_extra;
        }
        continue;
        
    add_extra:
        sum += 1000;
    }
    
    return sum;
}

__attribute__((optimize("O3")))
int main() {
    int final_result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 3 + 1;
    }
    
    /* Kernel 1: Long chain with data-dependent exit */
    printf("Starting kernel 1...\n");
    for (int i = 0; i < 100; i++) {
        int idx = 0;
        /* Data-dependent loop exit */
        while (g_array[idx] != 0) {
            final_result ^= compute_chain(g_array[idx]);
            idx = (idx + 1) & 0xFF;
            if (__builtin_expect_with_probability(idx == 0, 0, 0.1)) {
                break;
            }
        }
    }
    
    /* Kernel 2: Switch with many cases */
    printf("Starting kernel 2...\n");
    for (int i = 0; i < 500; i++) {
        final_result += switch_complex(i);
        
        /* Pointer aliasing to create uncertainty */
        int *ptr1 = &g_array[i & 0xFF];
        volatile int *ptr2 = (volatile int *)&g_array[(i + 1) & 0xFF];
        
        *ptr1 = *ptr2 + i;
        g_counter = *ptr1;
    }
    
    /* Kernel 3: Recursive computation */
    printf("Starting kernel 3...\n");
    final_result += recursive_compute(4, final_result & 0xFF);
    
    /* Kernel 4: Loop with inner function calls */
    printf("Starting kernel 4...\n");
    final_result += loop_with_inner_function(200);
    
    /* Kernel 5: Manual software pipelining */
    printf("Starting kernel 5...\n");
    {
        int a = 1, b = 2, c = 3, d = 4, e = 5;
        int f = 6, g = 7, h = 8, i = 9, j = 10;
        int k = 11, l = 12, m = 13, n = 14, o = 15;
        int p = 16, q = 17, r = 18, s = 19, t = 20;
        
        for (int iter = 0; iter < 50; iter++) {
            /* Independent operations that can be pipelined */
            a = b + c;
            b = c * d;
            c = d ^ e;
            d = e + f;
            e = f * g;
            
            /* Memory barrier */
            asm volatile("" : : : "memory");
            
            f = g ^ h;
            g = h + i;
            h = i * j;
            i = j ^ k;
            j = k + l;
            
            /* Volatile access */
            g_counter = iter;
            
            k = l * m;
            l = m ^ n;
            m = n + o;
            n = o * p;
            o = p ^ q;
            
            p = q + r;
            q = r * s;
            r = s ^ t;
            s = t + a;
            t = a * b;
            
            final_result += a + b + c + d + e + f + g + h + i + j +
                          k + l + m + n + o + p + q + r + s + t;
        }
    }
    
    printf("Final result: %d\n", final_result);
    return final_result != 0 ? 0 : 1;
}
