#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_var1 = 42;
volatile int g_var2 = 73;
int g_array[256] = {0};

/* Helper functions marked noinline to prevent inlining */
__attribute__((noinline, optimize("O3")))
int complex_chain(int start, int iterations) {
    int a = start, b = start * 2, c = start * 3;
    int d = start + 5, e = start - 3, f = start ^ 0x55;
    int result = 0;
    
    /* Long chain of dependent operations */
    for (int i = 0; i < iterations; i++) {
        a += b * c;
        b ^= d + e;
        c = (c * 3) + f;
        d = (d << 2) | (d >> 30);
        e = e * 7 + a;
        f = f ^ b ^ c;
        
        /* Memory barrier to split scheduling regions */
        asm volatile("" : : : "memory");
        
        /* Data-dependent exit condition */
        if (g_array[i % 256] != 0) {
            /* Additional operations on the speculative path */
            a += g_var1;
            b -= g_var2;
            asm volatile("" : : : "memory");
        }
        
        /* More dependent operations */
        result += a + b + c + d + e + f;
    }
    
    return result;
}

__attribute__((noinline, optimize("O3")))
int switch_computation(int selector, int base) {
    int v1 = base, v2 = base * 2, v3 = base * 3;
    int v4 = base + 1, v5 = base + 2, v6 = base + 3;
    int v7 = base - 1, v8 = base - 2, v9 = base - 3;
    int v10 = base ^ 0xFF, v11 = base | 0xAA, v12 = base & 0x55;
    int v13 = ~base, v14 = base << 2, v15 = base >> 2;
    int v16 = base % 17, v17 = base * base, v18 = base / 3;
    int v19 = base + 100, v20 = base - 100;
    
    /* Switch with many cases - creates complex control flow */
    switch (selector % 12) {
        case 0:
            v1 += v2; v3 ^= v4; v5 *= v6;
            v7 = (v7 << 1) | (v8 >> 1);
            v9 += g_var1;
            break;
        case 1:
            v2 -= v3; v4 |= v5; v6 &= v7;
            v8 = v9 * v10;
            v11 ^= g_var2;
            break;
        case 2:
            v3 *= v4; v5 += v6; v7 ^= v8;
            v9 = v10 | v11;
            v12 += g_var1 + g_var2;
            break;
        case 3:
            v4 ^= v5; v6 -= v7; v8 *= v9;
            v10 = v11 & v12;
            v13 = ~v14;
            break;
        case 4:
            v5 &= v6; v7 |= v8; v9 += v10;
            v11 = v12 * v13;
            v14 ^= 0xDEADBEEF;
            break;
        case 5:
            v6 += v7; v8 ^= v9; v10 *= v11;
            v12 = v13 | v14;
            v15 += g_var1;
            break;
        case 6:
            v7 -= v8; v9 |= v10; v11 &= v12;
            v13 = v14 * v15;
            v16 ^= g_var2;
            break;
        case 7:
            v8 *= v9; v10 += v11; v12 ^= v13;
            v14 = v15 | v16;
            v17 += g_var1 + g_var2;
            break;
        case 8:
            v9 ^= v10; v11 -= v12; v13 *= v14;
            v15 = v16 & v17;
            v18 = ~v19;
            break;
        case 9:
            v10 &= v11; v12 |= v13; v14 += v15;
            v16 = v17 * v18;
            v19 ^= 0xCAFEBABE;
            break;
        case 10:
            v11 += v12; v13 ^= v14; v15 *= v16;
            v17 = v18 | v19;
            v20 += g_var1;
            break;
        case 11:
            v12 -= v13; v14 |= v15; v16 &= v17;
            v18 = v19 * v20;
            v1 ^= g_var2;
            break;
    }
    
    /* Merge point with many live variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
}

__attribute__((noinline, optimize("O3")))
int nested_loops(int outer_iter, int inner_iter) {
    int sum = 0;
    
    for (int i = 0; i < outer_iter; i++) {
        int local_vars[8] = {i, i*2, i*3, i*4, i*5, i*6, i*7, i*8};
        
        /* Inner loop with software pipelining characteristics */
        for (int j = 0; j < inner_iter; j++) {
            /* Independent operations that could be pipelined */
            int t0 = local_vars[0] + j;
            int t1 = local_vars[1] * j;
            int t2 = local_vars[2] ^ j;
            int t3 = local_vars[3] | j;
            
            asm volatile("" : : : "memory");
            
            int t4 = local_vars[4] - j;
            int t5 = local_vars[5] & j;
            int t6 = local_vars[6] + t0;
            int t7 = local_vars[7] * t1;
            
            /* Cross-iteration dependency */
            local_vars[0] = t2;
            local_vars[1] = t3;
            local_vars[2] = t4;
            local_vars[3] = t5;
            local_vars[4] = t6;
            local_vars[5] = t7;
            local_vars[6] = t0 + t1;
            local_vars[7] = t2 ^ t3;
            
            sum += t0 + t1 + t2 + t3 + t4 + t5 + t6 + t7;
        }
        
        /* Branch with unpredictable probability */
        if (__builtin_expect_with_probability((i & 0xF) == 0, 0, 0.3)) {
            sum += g_var1;
        } else if (__builtin_expect_with_probability((i & 0x7) == 0, 0, 0.2)) {
            sum -= g_var2;
        }
    }
    
    return sum;
}

/* Recursive function with depth */
__attribute__((noinline, optimize("O3")))
int recursive_compute(int n, int depth) {
    if (depth <= 0) {
        return n;
    }
    
    int a = n * 3;
    int b = n + 7;
    int c = n ^ 0x99;
    
    /* Memory operation in the middle */
    volatile int mem_read = g_var1;
    a += mem_read;
    
    int r1 = recursive_compute(a, depth - 1);
    int r2 = recursive_compute(b, depth - 1);
    int r3 = recursive_compute(c, depth - 1);
    
    /* Complex merge computation */
    return (r1 * r2) ^ (r2 + r3) ^ (r3 - r1);
}

/* Irregular control flow with goto */
__attribute__((noinline, optimize("O3")))
int irregular_control_flow(int limit) {
    int x = 1, y = 2, z = 3;
    int counter = 0;
    
restart_point:
    while (counter < limit) {
        x = (x * 3 + y) ^ z;
        y = (y << 2) | (z >> 2);
        z = z * 5 + x;
        
        counter++;
        
        /* Data-dependent goto */
        if ((x & 0x7) == 0) {
            asm volatile("" : : : "memory");
            goto restart_point;
        }
        
        /* Do-while with break */
        do {
            x += g_var1;
            if (y > 1000) {
                y -= g_var2;
                break;
            }
            z ^= x;
        } while (0);
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
    }
    
    return x + y + z;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize global array with pattern */
    for (int i = 0; i < 256; i++) {
        g_array[i] = (i * 37) & 0xFF;
    }
    
    /* Kernel 1: Long dependent chain with data-dependent exit */
    checksum ^= complex_chain(1, 1000);
    
    /* Kernel 2: Switch with many cases */
    for (int i = 0; i < 50; i++) {
        checksum += switch_computation(i, i * 3);
    }
    
    /* Kernel 3: Nested loops for software pipelining */
    checksum ^= nested_loops(100, 50);
    
    /* Kernel 4: Recursive computation */
    checksum += recursive_compute(42, 4);
    
    /* Kernel 5: Irregular control flow */
    checksum ^= irregular_control_flow(500);
    
    /* Additional mixed pattern */
    for (int i = 0; i < 20; i++) {
        int temp = 0;
        
        /* Pointer aliasing to create uncertainty */
        int *p1 = &g_var1;
        volatile int *p2 = &g_var2;
        
        temp = *p1 + *p2;
        
        /* Chain with memory operations */
        for (int j = 0; j < 100; j++) {
            temp = (temp * 3) ^ g_array[j % 256];
            
            /* Branch with probability hint */
            if (__builtin_expect_with_probability((temp & 0x3) == 0, 1, 0.7)) {
                asm volatile("" : : : "memory");
                temp += *p2;
            } else {
                temp -= *p1;
            }
        }
        
        checksum += temp;
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
