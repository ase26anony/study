#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to create aliasing and data dependencies */
volatile int g_counter = 0;
int g_array[256];
int g_results[4] = {0};
int* volatile g_ptr1 = &g_array[0];
int* volatile g_ptr2 = &g_array[128];

/* Function attributes to control optimization and scheduling */
__attribute__((noinline, optimize("O3")))
int complex_chain(int seed) {
    volatile int barrier = 0;
    int a = seed, b = seed * 2, c = seed * 3, d = seed * 4;
    int e = seed * 5, f = seed * 6, g = seed * 7, h = seed * 8;
    int i = seed * 9, j = seed * 10, k = seed * 11, l = seed * 12;
    int m = seed * 13, n = seed * 14, o = seed * 15, p = seed * 16;
    int q = seed * 17, r = seed * 18, s = seed * 19, t = seed * 20;
    
    /* Long chain of dependent operations */
    a += b; b ^= c; c *= d; d -= e;
    e |= f; f &= g; g <<= h; h >>= i;
    i = (i + j) * k; j = (j - k) ^ l;
    k = (k * l) + m; l = (l ^ m) | n;
    m = (m & n) << o; n = (n | o) >> p;
    o = (o ^ p) + q; p = (p & q) * r;
    q = (q | r) - s; r = (r ^ s) << t;
    
    /* Memory barrier to split scheduling regions */
    asm volatile("" : : : "memory");
    
    /* More operations with data-dependent control flow */
    if (__builtin_expect_with_probability((a & 0xFF) > 128, 1, 0.7)) {
        s = (s * t) ^ a;
        t = (t + a) | b;
    } else {
        s = (s ^ t) & a;
        t = (t | a) + b;
    }
    
    /* Another scheduling barrier */
    barrier = 1;
    
    /* Final computation with pointer aliasing */
    *g_ptr1 = a + b;
    *g_ptr2 = c + d;
    
    return (a ^ b ^ c ^ d ^ e ^ f ^ g ^ h ^ i ^ j ^ 
            k ^ l ^ m ^ n ^ o ^ p ^ q ^ r ^ s ^ t);
}

__attribute__((noinline, optimize("O3")))
int switch_computation(int value) {
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0;
    int r7 = 0, r8 = 0, r9 = 0, r10 = 0, r11 = 0, r12 = 0;
    
    /* Complex switch with many cases - creates merge points */
    switch (value & 0xF) {
        case 0:
            r1 = value * 2; r2 = value + 1;
            r3 = value ^ 0xAA; r4 = value | 0x55;
            break;
        case 1:
            r1 = value / 2; r2 = value - 1;
            r3 = value & 0x55; r4 = value << 1;
            break;
        case 2:
            r1 = value * 3; r2 = value + 2;
            r3 = value ^ 0xBB; r4 = value | 0xAA;
            break;
        case 3:
            r1 = value / 3; r2 = value - 2;
            r3 = value & 0xBB; r4 = value << 2;
            break;
        case 4:
            r1 = value * 5; r2 = value + 3;
            r3 = value ^ 0xCC; r4 = value | 0xBB;
            break;
        case 5:
            r1 = value / 5; r2 = value - 3;
            r3 = value & 0xCC; r4 = value >> 1;
            break;
        case 6:
            r1 = value * 7; r2 = value + 4;
            r3 = value ^ 0xDD; r4 = value | 0xCC;
            break;
        case 7:
            r1 = value / 7; r2 = value - 4;
            r3 = value & 0xDD; r4 = value >> 2;
            break;
        case 8:
            r1 = value * 11; r2 = value + 5;
            r3 = value ^ 0xEE; r4 = value | 0xDD;
            break;
        case 9:
            r1 = value / 11; r2 = value - 5;
            r3 = value & 0xEE; r4 = value >> 3;
            break;
        case 10:
            r1 = value * 13; r2 = value + 6;
            r3 = value ^ 0xFF; r4 = value | 0xEE;
            break;
        case 11:
            r1 = value / 13; r2 = value - 6;
            r3 = value & 0xFF; r4 = value >> 4;
            break;
        case 12:
            r1 = value * 17; r2 = value + 7;
            r3 = value ^ 0x11; r4 = value | 0xFF;
            break;
        case 13:
            r1 = value / 17; r2 = value - 7;
            r3 = value & 0x11; r4 = value >> 5;
            break;
        case 14:
            r1 = value * 19; r2 = value + 8;
            r3 = value ^ 0x22; r4 = value | 0x11;
            break;
        default: /* case 15 */
            r1 = value / 19; r2 = value - 8;
            r3 = value & 0x22; r4 = value >> 6;
            break;
    }
    
    /* Post-switch computation that depends on all results */
    r5 = r1 + r2; r6 = r3 ^ r4;
    r7 = r5 * r6; r8 = r5 - r6;
    r9 = r7 & r8; r10 = r7 | r8;
    r11 = r9 << 2; r12 = r10 >> 2;
    
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12;
}

__attribute__((noinline, optimize("O3")))
int nested_loops(int iterations) {
    int sum = 0;
    int i, j;
    
    /* Outer loop with data-dependent inner loop */
    for (i = 0; i < iterations; i++) {
        int inner_limit = (i & 0x3) + 2;  /* Varies from 2 to 5 */
        
        /* Inner loop with dependent operations */
        for (j = 0; j < inner_limit; j++) {
            sum += (i * j) ^ (i + j);
            sum = (sum << 1) | (sum >> 31);  /* Rotate */
            
            /* Conditional break with probability */
            if (__builtin_expect_with_probability((sum & 0xFF) == 0, 0, 0.95)) {
                break;
            }
        }
        
        /* Memory operation in outer loop */
        g_array[i & 0xFF] = sum;
        
        /* Scheduling barrier */
        asm volatile("" : : : "memory");
    }
    
    return sum;
}

__attribute__((noinline))
int recursive_compute(int n, int depth) {
    if (depth >= 4 || n <= 1) {
        return n;
    }
    
    int a = recursive_compute(n - 1, depth + 1);
    int b = recursive_compute(n / 2, depth + 1);
    
    /* Complex computation at return point */
    int result = (a * b) ^ (a + b) ^ (a - b);
    
    /* Create register pressure with many temporaries */
    int t1 = result * 2, t2 = result / 2, t3 = result ^ 0x55;
    int t4 = result | 0xAA, t5 = result & 0xFF, t6 = result << 3;
    int t7 = result >> 3, t8 = result + 0x11, t9 = result - 0x22;
    
    /* Use all temporaries to prevent elimination */
    result = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9;
    
    return result;
}

__attribute__((noinline, optimize("O3")))
int irregular_control_flow(int limit) {
    int x = 1, y = 2, z = 3, w = 4;
    int v1 = 5, v2 = 6, v3 = 7, v4 = 8;
    int count = 0;
    
restart_point:
    /* Do-while with internal break */
    do {
        x = (x * y) + z;
        y = (y ^ z) | w;
        z = (z + w) & v1;
        w = (w * v1) ^ v2;
        
        count++;
        
        /* Conditional break inside do-while */
        if (__builtin_expect_with_probability(count > (limit / 2), 0, 0.8)) {
            v1 = (v1 + v2) * v3;
            break;
        }
        
        v2 = (v2 - v3) | v4;
        v3 = (v3 ^ v4) & x;
        v4 = (v4 * x) + y;
        
        /* Another conditional with goto */
        if ((count & 0x3) == 0) {
            v1 = v2;
            v2 = v3;
            goto restart_point;
        }
    } while (count < limit);
    
    return x + y + z + w + v1 + v2 + v3 + v4 + count;
}

int main() {
    int total = 0;
    int i;
    
    /* Initialize global array */
    for (i = 0; i < 256; i++) {
        g_array[i] = i;
    }
    
    /* Kernel 1: Long chain with data-dependent control flow */
    for (i = 0; i < 100; i++) {
        total ^= complex_chain(i);
        g_counter++;
    }
    
    /* Kernel 2: Switch-based computation */
    for (i = 0; i < 50; i++) {
        total += switch_computation(total + i);
    }
    
    /* Kernel 3: Nested loops with varying bounds */
    total += nested_loops(200);
    
    /* Kernel 4: Recursive computation */
    for (i = 1; i < 20; i++) {
        total ^= recursive_compute(i, 0);
    }
    
    /* Kernel 5: Irregular control flow */
    total += irregular_control_flow(150);
    
    /* Final aggregation with memory operations */
    for (i = 0; i < 4; i++) {
        g_results[i] = total + i;
        total += g_results[i];
    }
    
    /* Use result to prevent elimination */
    printf("Result checksum: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
