#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_var1 = 42;
volatile int g_var2 = 73;
int g_array[256];

/* Helper functions marked noinline to prevent inlining */
__attribute__((noinline, optimize("O3")))
int compute_chain(int seed) {
    volatile int barrier;
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    
    /* Long chain of dependent operations */
    a += b * c;
    b ^= d << 2;
    c = (c + d) * e;
    d = (d ^ e) + f;
    e = e * f + g;
    f = f ^ g * h;
    g = g + h * a;
    h = h ^ a + b;
    
    /* Memory barrier to split scheduling region */
    asm volatile("" : : : "memory");
    
    /* More dependent operations */
    a = (a << 3) | (b >> 2);
    b = b * c + d;
    c = c ^ d * e;
    d = d + e ^ f;
    e = e * f + g;
    f = f ^ g + h;
    g = g * h + a;
    h = h ^ a * b;
    
    /* Data-dependent exit condition */
    barrier = g_var1;
    if (__builtin_expect_with_probability(barrier > 100, 0, 0.7)) {
        a += 1000;
    }
    
    return a + b + c + d + e + f + g + h;
}

__attribute__((noinline, optimize("O3")))
int recursive_compute(int n, int acc) {
    if (n <= 0) return acc;
    
    int local_vars[8] = {acc, acc+1, acc+2, acc+3, acc+4, acc+5, acc+6, acc+7};
    
    /* Complex computation with many local variables */
    for (int i = 0; i < 8; i++) {
        local_vars[i] = local_vars[i] * local_vars[(i+1)%8] + 
                       local_vars[(i+2)%8] ^ local_vars[(i+3)%8];
    }
    
    /* Memory operation with uncertain latency */
    volatile int* ptr = (volatile int*)&g_var2;
    int mem_val = *ptr;
    
    /* Branch with probability hint */
    if (__builtin_expect_with_probability(mem_val > 50, 1, 0.6)) {
        for (int i = 0; i < 8; i++) {
            local_vars[i] += mem_val;
        }
    }
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += local_vars[i];
    }
    
    return recursive_compute(n - 1, sum + acc);
}

__attribute__((noinline, optimize("O3")))
int switch_complex(int selector, int base) {
    int r1 = base, r2 = base+1, r3 = base+2, r4 = base+3;
    int r5 = base+4, r6 = base+5, r7 = base+6, r8 = base+7;
    int r9 = base+8, r10 = base+9, r11 = base+10, r12 = base+11;
    
    /* Large switch statement with different computation patterns */
    switch (selector % 12) {
        case 0:
            r1 = r1 * r2 + r3;
            r2 = r2 ^ r4 << 1;
            r3 = r3 + r5 * r6;
            break;
        case 1:
            r4 = r4 | r7;
            r5 = r5 & r8;
            r6 = r6 ^ r9;
            break;
        case 2:
            r7 = r7 + r10;
            r8 = r8 - r11;
            r9 = r9 * r12;
            break;
        case 3:
            r10 = r10 ^ r1;
            r11 = r11 | r2;
            r12 = r12 & r3;
            break;
        case 4:
            r1 = r1 << r4;
            r2 = r2 >> r5;
            r3 = r3 ^ r6;
            break;
        case 5:
            r4 = r4 + r7 * r8;
            r5 = r5 - r9 ^ r10;
            r6 = r6 & r11 | r12;
            break;
        case 6:
            r7 = r7 * r1 + r2;
            r8 = r8 ^ r3 * r4;
            r9 = r9 + r5 & r6;
            break;
        case 7:
            r10 = r10 | r7 << 2;
            r11 = r11 ^ r8 >> 1;
            r12 = r12 + r9 * 3;
            break;
        case 8:
            r1 = r1 - r10;
            r2 = r2 + r11;
            r3 = r3 ^ r12;
            break;
        case 9:
            r4 = r4 * r1 | r2;
            r5 = r5 + r3 ^ r4;
            r6 = r6 & r5 | r6;
            break;
        case 10:
            r7 = r7 << r8;
            r8 = r8 >> r9;
            r9 = r9 ^ r10;
            break;
        case 11:
            r10 = r10 + r11 * r12;
            r11 = r11 - r1 ^ r2;
            r12 = r12 & r3 | r4;
            break;
    }
    
    /* Irregular control flow with goto */
    if (__builtin_expect_with_probability(r1 > 1000, 0, 0.3)) {
        goto merge_point;
    }
    
    /* Additional computation */
    r1 = r1 * 2;
    r2 = r2 + 3;
    
merge_point:
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12;
}

__attribute__((noinline, optimize("O3")))
int loop_with_inner_function(int iterations) {
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Data-dependent loop exit */
        if (__builtin_expect_with_probability(g_array[i & 255] == 0, 0, 0.8)) {
            break;
        }
        
        /* Call to small helper function */
        result += compute_chain(i);
        
        /* Software pipelining style computation */
        int temp = g_array[(i + 1) & 255];
        g_array[i & 255] = temp * 3 + result;
        
        /* Memory operation */
        volatile int* alias1 = (volatile int*)&g_var1;
        volatile int* alias2 = (volatile int*)&g_var1;  // Same variable, different pointer
        
        int val1 = *alias1;
        int val2 = *alias2;
        
        result = result ^ (val1 * val2);
        
        /* do-while with break */
        int j = 0;
        do {
            if (j > 5) break;
            result += j * i;
            j++;
        } while (1);
    }
    
    return result;
}

int main() {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 3 + 1;
    }
    
    int checksum = 0;
    
    /* Kernel 1: Long chain with data-dependent break */
    checksum += compute_chain(42);
    
    /* Kernel 2: Switch with many cases */
    for (int i = 0; i < 50; i++) {
        checksum += switch_complex(i, checksum);
    }
    
    /* Kernel 3: Loop with inner function calls */
    checksum += loop_with_inner_function(100);
    
    /* Kernel 4: Recursive computation */
    checksum += recursive_compute(4, checksum);
    
    /* Additional complex control flow */
    int counter = 0;
    int target = 100;
    
restart_loop:
    while (counter < target) {
        /* Mix of operations */
        checksum = checksum * 3 + counter;
        
        /* Branch with probability hint */
        if (__builtin_expect_with_probability(counter % 37 == 0, 0, 0.4)) {
            target -= 10;
            goto restart_loop;  /* Irregular control flow */
        }
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
        
        counter++;
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
