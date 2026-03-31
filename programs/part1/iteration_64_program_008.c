#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_var1 = 42;
volatile int g_var2 = 73;
int g_array[256];

/* Helper functions marked noinline to prevent inlining */
__attribute__((noinline, optimize("O3")))
static long compute_chain(int seed) {
    volatile int mem_barrier = seed;
    long a = seed * 3;
    long b = seed + 7;
    long c = seed ^ 0x5A5A;
    long d = seed - 19;
    
    /* Long chain of dependent operations */
    for (int i = 0; i < 32; i++) {
        a += b * c;
        b ^= d + i;
        c *= a - b;
        d += c ^ b;
        
        /* Memory barrier to split scheduling regions */
        if (i % 8 == 0) {
            asm volatile("" : : : "memory");
            mem_barrier = g_var1;
        }
        
        /* Data-dependent exit condition */
        if (a % 1000 > 950) {
            b += g_var2;
            break;
        }
    }
    
    /* Another chain with __builtin_expect */
    for (int i = 0; i < 16; i++) {
        if (__builtin_expect_with_probability((a + i) % 7 == 0, 0, 0.3)) {
            c += d * 3;
            asm volatile("" : : : "memory");
        } else {
            d ^= b << (i & 3);
        }
        
        a += c - d;
        b *= a | 1;
    }
    
    return a + b + c + d;
}

__attribute__((noinline, optimize("O3")))
static int switch_complex(int val) {
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
    int r6 = 0, r7 = 0, r8 = 0, r9 = 0, r10 = 0;
    int r11 = 0, r12 = 0, r13 = 0, r14 = 0, r15 = 0;
    
    /* Complex switch with many cases - creates merge points */
    switch (val % 13) {
        case 0:
            r1 = val * 3; r2 = val + 1; r3 = val ^ 0xFF;
            for (int i = 0; i < 8; i++) r1 += r2 * r3;
            break;
        case 1:
            r4 = val << 2; r5 = val >> 1; r6 = val | 0xAA;
            r4 ^= r5; r5 |= r6; r6 &= r4;
            break;
        case 2:
            r7 = val * val; r8 = val + val; r9 = val - 100;
            r7 += g_var1; r8 -= g_var2;
            break;
        case 3:
            r10 = val % 17; r11 = val % 23; r12 = val % 31;
            r10 *= r11; r11 += r12; r12 ^= r10;
            break;
        case 4:
            r13 = val + 0x1000; r14 = val - 0x200; r15 = val ^ 0xCC;
            r13 |= r14; r14 &= r15; r15 ^= r13;
            break;
        case 5:
            r1 = val * 5; r3 = val * 7; r5 = val * 11;
            r1 += r3; r3 -= r5; r5 *= r1;
            break;
        case 6:
            r2 = val << 3; r4 = val >> 2; r6 = val & 0x55;
            r2 ^= r4; r4 |= r6; r6 &= r2;
            break;
        case 7:
            r7 = val + 999; r9 = val - 888; r11 = val ^ 777;
            r7 *= r9; r9 += r11; r11 ^= r7;
            break;
        case 8:
            r8 = val % 19; r10 = val % 29; r12 = val % 37;
            r8 += r10; r10 -= r12; r12 *= r8;
            break;
        case 9:
            r13 = val | 0xF0F0; r14 = val & 0x0F0F; r15 = val ^ 0xAAAA;
            r13 += r14; r14 ^= r15; r15 |= r13;
            break;
        case 10:
            r1 = val * 13; r4 = val * 17; r7 = val * 19;
            r1 ^= r4; r4 |= r7; r7 &= r1;
            break;
        case 11:
            r2 = val + 0x1234; r5 = val - 0x5678; r8 = val ^ 0x9ABC;
            r2 *= r5; r5 += r8; r8 ^= r2;
            break;
        case 12:
            r3 = val << 4; r6 = val >> 3; r9 = val & 0x33;
            r3 += r6; r6 ^= r9; r9 |= r3;
            break;
    }
    
    /* Merge all results */
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + 
           r11 + r12 + r13 + r14 + r15;
}

__attribute__((noinline, optimize("O3")))
static int recursive_scheduler(int n, int depth) {
    if (depth >= 4) return n;
    
    int a = n * 3;
    int b = n + 7;
    int c = n ^ 0x55;
    
    /* Create register pressure with many variables */
    int v1 = a, v2 = b, v3 = c, v4 = a + b, v5 = b + c;
    int v6 = a ^ c, v7 = a * b, v8 = b * c, v9 = a * c;
    int v10 = v1 + v2, v11 = v3 + v4, v12 = v5 + v6;
    int v13 = v7 + v8, v14 = v9 + v10, v15 = v11 + v12;
    
    /* Memory operation in the middle */
    volatile int barrier = g_array[n & 255];
    
    /* Chain of operations */
    for (int i = 0; i < 4; i++) {
        v1 += v2 * v3;
        v2 ^= v4 + i;
        v3 *= v5 - v6;
        v4 += v7 ^ v8;
        v5 -= v9 & v10;
        
        if (__builtin_expect((v1 + i) % 5 == 0, 0)) {
            v6 += barrier;
            asm volatile("" : : : "memory");
        }
    }
    
    /* Recursive call */
    int res = recursive_scheduler(v1 + v2 + v3, depth + 1);
    
    /* More operations after recursion (scheduler may restore state here) */
    v7 += res * 2;
    v8 ^= res + 1;
    v9 *= res | 1;
    
    return v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
}

__attribute__((noinline, optimize("O3")))
static void software_pipelined_loop(int *arr, int size) {
    int i = 0;
    
    /* Loop with irregular control flow */
    loop_start:
    if (i >= size) goto loop_end;
    
    /* Small inner computation in helper-like pattern */
    int x = arr[i];
    int y = x * 3;
    int z = x + 7;
    
    /* Data-dependent loop with break */
    int j = 0;
    while (j < 8) {
        y += z * j;
        z ^= y + j;
        
        if (__builtin_expect_with_probability((y + z) % 100 > 90, 0, 0.2)) {
            y += g_var1;
            break;
        }
        
        if (j == 4) {
            asm volatile("" : : : "memory");
        }
        
        j++;
    }
    
    arr[i] = y + z;
    
    /* Irregular goto to create complex CFG */
    if (__builtin_expect((i % 7) == 0, 0)) {
        i += 2;
        goto loop_start;
    }
    
    i++;
    goto loop_start;
    
    loop_end:
    return;
}

int main() {
    long total = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 3 + 1;
    }
    
    /* Kernel 1: Long chain with data-dependent breaks */
    for (int i = 0; i < 100; i++) {
        total += compute_chain(i + total);
        
        /* Memory barrier between iterations */
        if (i % 25 == 0) {
            asm volatile("" : : : "memory");
            g_var1 ^= i;
        }
    }
    
    /* Kernel 2: Complex switch statements */
    for (int i = 0; i < 50; i++) {
        total += switch_complex(i + (total & 0xFF));
        
        /* Create pointer aliasing effect */
        int *p1 = (int*)&g_var1;
        int *p2 = (int*)&g_var2;
        *p1 += i % 3;
        *p2 ^= i % 5;
    }
    
    /* Kernel 3: Recursive scheduler stress */
    for (int i = 0; i < 30; i++) {
        total += recursive_scheduler(i + (total % 100), 0);
    }
    
    /* Kernel 4: Software pipelined style */
    int work_array[128];
    for (int i = 0; i < 128; i++) {
        work_array[i] = i * 2 + 1;
    }
    
    software_pipelined_loop(work_array, 128);
    
    for (int i = 0; i < 128; i++) {
        total += work_array[i];
    }
    
    /* Kernel 5: Mixed operations with volatile accesses */
    unsigned long mix = total;
    for (int i = 0; i < 40; i++) {
        mix = (mix * 1103515245 + 12345) & 0x7FFFFFFF;
        
        int temp = mix & 0xFF;
        if (__builtin_expect(temp > 200, 0)) {
            g_var2 += temp;
            asm volatile("" : : : "memory");
        }
        
        /* Do-while with break */
        int k = 0;
        do {
            if (k == 5) {
                mix ^= g_var1;
                break;
            }
            mix += (mix << 3) ^ (mix >> 5);
            k++;
        } while (k < 8);
    }
    
    total += mix;
    
    printf("Result checksum: %ld\n", total);
    return 0;
}
