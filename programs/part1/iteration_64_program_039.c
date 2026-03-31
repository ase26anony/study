#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_var1 = 42;
volatile int g_var2 = 73;
int g_array[256];

/* Helper functions marked noinline to prevent inlining */
__attribute__((noinline, optimize("O3")))
int complex_chain(int seed) {
    volatile int mem_barrier;
    int a = seed, b = seed * 2, c = seed + 1;
    int d = 0, e = 0, f = 0, g = 0, h = 0;
    int i = 0, j = 0, k = 0, l = 0, m = 0;
    int n = 0, o = 0, p = 0, q = 0, r = 0;
    int s = 0, t = 0, u = 0, v = 0, w = 0;
    
    /* Long chain of dependent operations */
    a += b * c;
    b ^= a << 3;
    c = (c + b) | a;
    
    /* Memory barrier to split scheduling region */
    asm volatile("" : : : "memory");
    
    d = a * b - c;
    e = d ^ (a >> 2);
    f = e + c * 3;
    g = f & 0xFFFF;
    h = g * 7 + e;
    
    /* Data-dependent loop with unpredictable exit */
    i = 0;
    while (__builtin_expect_with_probability(g_array[i] != 0, 0, 0.7)) {
        h += g_array[i] * i;
        i = (i + 1) & 255;
        if (i == 0) break;
    }
    
    /* More arithmetic chains */
    j = h * 11;
    k = j ^ h;
    l = k + j * 3;
    m = l | (h << 4);
    n = m - k;
    o = n * 13;
    p = o ^ n;
    q = p + o / 5;
    r = q & 0xFFFFFF;
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    s = r * 17;
    t = s ^ r;
    u = t + s * 7;
    v = u | (r << 3);
    w = v - t;
    
    /* Access volatile global to prevent elimination */
    mem_barrier = g_var1;
    w += mem_barrier;
    
    return w;
}

__attribute__((noinline, optimize("O3")))
int switch_computation(int selector, int input) {
    int result = input;
    int tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0;
    int tmp5 = 0, tmp6 = 0, tmp7 = 0, tmp8 = 0;
    
    /* Large switch to create complex control flow */
    switch (selector & 0xF) {
        case 0:
            result = result * 3 + 1;
            tmp1 = result ^ 0xAA;
            tmp2 = tmp1 * 7;
            break;
        case 1:
            result = (result << 4) | (result >> 28);
            tmp3 = result + 0x1234;
            tmp4 = tmp3 & 0xABCD;
            break;
        case 2:
            result = result ^ result << 1;
            tmp5 = result * 13;
            tmp6 = tmp5 - 0xBE;
            break;
        case 3:
            result = ~result;
            tmp7 = result | 0x55;
            tmp8 = tmp7 * 3;
            break;
        case 4:
            result = result + result * 2;
            tmp1 = result ^ 0xCC;
            tmp2 = tmp1 + 0x10;
            break;
        case 5:
            result = result & 0xF0F0F0F0;
            tmp3 = result * 5;
            tmp4 = tmp3 | 0x0F0F0F0F;
            break;
        case 6:
            result = result - 0x1000;
            tmp5 = result ^ 0x8888;
            tmp6 = tmp5 * 11;
            break;
        case 7:
            result = result >> 4;
            tmp7 = result + 0x2000;
            tmp8 = tmp7 & 0x3333;
            break;
        case 8:
            result = result * result;
            tmp1 = result % 10007;
            tmp2 = tmp1 ^ 0x9999;
            break;
        case 9:
            result = result | 0x11111111;
            tmp3 = result * 3;
            tmp4 = tmp3 - 0x2222;
            break;
        case 10:
            result = result ^ 0x55555555;
            tmp5 = result << 2;
            tmp6 = tmp5 | 0xAAAA;
            break;
        case 11:
            result = result + 0x7777;
            tmp7 = result & 0xFFFF;
            tmp8 = tmp7 * 17;
            break;
        case 12:
            result = result / 3;
            tmp1 = result ^ 0xBBBB;
            tmp2 = tmp1 + 0xCCCC;
            break;
        case 13:
            result = result << 8;
            tmp3 = result | 0xFF;
            tmp4 = tmp3 * 19;
            break;
        case 14:
            result = result - 0x10000;
            tmp5 = result ^ 0xEEEE;
            tmp6 = tmp5 & 0x7FFF;
            break;
        default: /* case 15 */
            result = result * 23;
            tmp7 = result % 1009;
            tmp8 = tmp7 ^ 0xDDDD;
            break;
    }
    
    /* Merge point with complex data flow */
    return result + tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8;
}

__attribute__((noinline, optimize("O3")))
int recursive_compute(int depth, int value) {
    if (__builtin_expect(depth <= 0, 0)) {
        return value;
    }
    
    int local1 = value * 2;
    int local2 = value + 1;
    int local3 = value ^ 0x9E3779B9;
    
    /* Create register pressure */
    int t1 = local1 * local2;
    int t2 = local2 + local3;
    int t3 = local3 ^ local1;
    int t4 = t1 * t2;
    int t5 = t2 + t3;
    int t6 = t3 ^ t1;
    
    /* Recursive call */
    int rec_result = recursive_compute(depth - 1, t4 + t5 + t6);
    
    /* Complex merge computation */
    return rec_result * 3 + local1 - local2 + local3;
}

__attribute__((noinline, optimize("O3")))
int software_pipelined_kernel(int iterations) {
    int sum = 0;
    int i, j;
    
    for (i = 0; i < iterations; i++) {
        /* Inner loop with independent operations */
        int acc = 0;
        for (j = 0; j < 8; j++) {
            acc += g_array[(i * 8 + j) & 255] * j;
        }
        
        /* Data-dependent control flow */
        if (__builtin_expect_with_probability(acc & 1, 1, 0.3)) {
            sum += acc * 3;
        } else {
            sum += acc / 2;
        }
        
        /* Irregular goto to create complex CFG */
        if ((i & 7) == 0) {
            goto loop_continue;
        }
        
        /* More computation */
        sum ^= (i * 0x9E3779B9);
        
    loop_continue:
        /* Do-while with break */
        do {
            if (sum > 1000000) {
                sum -= 500000;
                break;
            }
            sum += i;
        } while (0);
    }
    
    return sum;
}

int main() {
    int i, result = 0;
    
    /* Initialize global array with pseudo-random data */
    for (i = 0; i < 256; i++) {
        g_array[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    
    printf("Starting scheduler stress test...\n");
    
    /* Kernel 1: Long dependent chains with data-dependent loops */
    for (i = 0; i < 100; i++) {
        result ^= complex_chain(i);
        
        /* Access global via pointer aliasing */
        int *alias1 = (int*)&g_var1;
        int *alias2 = (int*)((char*)&g_var1 + 0);
        *alias1 += 1;
        result += *alias2;
    }
    
    /* Kernel 2: Switch-based computation */
    for (i = 0; i < 200; i++) {
        result += switch_computation(i, result);
    }
    
    /* Kernel 3: Recursive computation */
    result += recursive_compute(4, result);
    
    /* Kernel 4: Software pipelined style */
    result += software_pipelined_kernel(50);
    
    /* Final mixing */
    result = (result * 0xCC9E2D51) ^ (result >> 16);
    result = (result * 0x1B873593) ^ (result >> 13);
    
    printf("Result checksum: %d\n", result);
    printf("Global var1: %d, var2: %d\n", g_var1, g_var2);
    
    return result != 0 ? 0 : 1;
}
