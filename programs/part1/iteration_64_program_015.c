#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
int global_a = 42;
int global_b = 73;
int global_c = 19;
int global_d = 88;

/* Volatile variables to create scheduling uncertainty */
volatile int volatile_sink = 0;
volatile int volatile_source = 1;

/* ========== Helper Functions ========== */

/* Long chain of dependent operations */
__attribute__((noinline, optimize("O3")))
static int long_dependency_chain(int seed, int iterations) {
    int a = seed;
    int b = seed * 2;
    int c = seed + 7;
    int d = seed ^ 0x55AA55AA;
    int e = seed - 19;
    int f = seed | 0x12345678;
    
    for (int i = 0; i < iterations; i++) {
        /* Create data dependencies */
        a += b * c;
        b ^= d + e;
        c *= f - a;
        d += b ^ c;
        e = (e << 3) | (e >> 29);
        f = f * 13 + 17;
        
        /* Memory barrier to split scheduling regions */
        if (i % 7 == 0) {
            asm volatile ("" : : : "memory");
        }
        
        /* Data-dependent exit condition */
        if (a > 1000000 && __builtin_expect_with_probability(i > 5, 1, 0.7)) {
            volatile_sink = a;
            break;
        }
    }
    
    /* Final mixing */
    return a ^ b ^ c ^ d ^ e ^ f;
}

/* Recursive function with arithmetic */
__attribute__((noinline))
static int recursive_compute(int n, int depth) {
    if (depth <= 0 || n == 0) {
        return n;
    }
    
    int result = n;
    for (int i = 0; i < 3; i++) {
        result = (result * 1103515245 + 12345) & 0x7fffffff;
        if (__builtin_expect((result & 255) == 0, 0)) {
            asm volatile ("" : : : "memory");
        }
    }
    
    return result + recursive_compute(n - 1, depth - 1);
}

/* Function with many local variables */
__attribute__((noinline, optimize("O3")))
static int many_variables(int input) {
    /* Declare many local variables to create register pressure */
    int v1 = input;
    int v2 = input + 1;
    int v3 = input * 2;
    int v4 = input ^ 0xFF;
    int v5 = input - 17;
    int v6 = input | 0xAA;
    int v7 = input & 0x55;
    int v8 = input + 42;
    int v9 = input * 3;
    int v10 = input / 2;
    int v11 = input % 13;
    int v12 = input << 2;
    int v13 = input >> 1;
    int v14 = ~input;
    int v15 = input + 999;
    int v16 = input * input;
    int v17 = input + v1;
    int v18 = v2 * v3;
    int v19 = v4 ^ v5;
    int v20 = v6 | v7;
    
    /* Complex computation with all variables */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        v1 += v2;
        v2 ^= v3;
        v3 *= v4;
        v4 += v5;
        v5 ^= v6;
        v6 |= v7;
        v7 &= v8;
        v8 -= v9;
        v9 *= v10;
        v10 /= (v11 + 1);
        v11 = (v11 << 1) | (v11 >> 31);
        v12 ^= v13;
        v13 += v14;
        v14 *= v15;
        v15 ^= v16;
        v16 += v17;
        v17 |= v18;
        v18 &= v19;
        v19 -= v20;
        v20 = (v20 * 13 + 17) & 0xFFFF;
        
        /* Memory operation with volatile */
        if (i % 3 == 0) {
            v1 += volatile_source;
        }
        
        /* Branch with probability hint */
        if (__builtin_expect_with_probability(v1 > 10000, 0, 0.3)) {
            asm volatile ("" : : : "memory");
            break;
        }
    }
    
    /* Aggregate all variables */
    sum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
          v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    
    return sum;
}

/* Switch-based computation */
__attribute__((noinline))
static int switch_computation(int selector, int value) {
    int result = value;
    
    switch (selector % 12) {
        case 0:
            result = (result * 3 + 7) ^ 0x1234;
            for (int i = 0; i < 5; i++) result += i;
            break;
        case 1:
            result = (result << 4) | (result >> 28);
            result ^= 0xABCDEF;
            break;
        case 2:
            result = result * result + 42;
            asm volatile ("" : : : "memory");
            break;
        case 3:
            result = (result & 0xF0F0F0F0) | 0x0A0A0A0A;
            result += global_a;
            break;
        case 4:
            result = result ^ global_b;
            result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
            break;
        case 5:
            result = result | global_c;
            result = ~result;
            break;
        case 6:
            result = result - global_d;
            result = result * 7 + 3;
            break;
        case 7:
            result = (result + 777) * 13;
            if (__builtin_expect(result > 1000, 1)) {
                result /= 2;
            }
            break;
        case 8:
            result = result ^ (result << 16);
            result = result ^ (result >> 16);
            break;
        case 9:
            result = result + volatile_source;
            result = result * 2 - 1;
            break;
        case 10:
            result = (result % 17) * 31 + 11;
            asm volatile ("" : : : "memory");
            break;
        case 11:
            result = result & 0x55555555;
            result = result | 0xAAAAAAAA;
            break;
        default:
            result = 0;
    }
    
    return result;
}

/* Function with irregular control flow */
__attribute__((noinline, optimize("O3")))
static int irregular_control_flow(int iterations) {
    int x = 1;
    int y = 2;
    int z = 3;
    int w = 4;
    
    int i = 0;
    
    /* Label for goto */
    restart_loop:
    
    do {
        x = x * 3 + y;
        y = y ^ z;
        z = z + w * 7;
        w = w | x;
        
        /* Complex break condition */
        if ((x + y + z + w) > 1000000) {
            asm volatile ("" : : : "memory");
            break;
        }
        
        /* Nested do-while with break */
        do {
            x += 1;
            if (x % 13 == 0) {
                y += volatile_source;
                break;
            }
            y -= 1;
        } while (0);
        
        /* Another scheduling barrier */
        asm volatile ("" : : : "memory");
        
        i++;
        
        /* Goto to create irregular CFG */
        if (__builtin_expect_with_probability(i == iterations / 2, 0, 0.2)) {
            goto restart_loop;
        }
        
    } while (i < iterations);
    
    return x + y + z + w;
}

/* Software pipelining style computation */
__attribute__((noinline))
static int pipelined_computation(int *data, int size) {
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    for (int i = 0; i < size; i++) {
        /* Multiple accumulation chains */
        sum1 += data[i] * 3;
        sum2 += data[i] ^ 0xAA;
        sum3 += data[i] + i;
        
        /* Memory operation every 4 iterations */
        if (i % 4 == 0) {
            volatile_sink = data[i];
        }
        
        /* Branch with probability hint */
        if (__builtin_expect_with_probability(sum1 > 100000, 0, 0.1)) {
            asm volatile ("" : : : "memory");
            sum1 /= 2;
        }
    }
    
    return sum1 ^ sum2 ^ sum3;
}

/* ========== Main Function ========== */

int main(void) {
    int result = 0;
    
    /* Seed computation */
    int seed = volatile_source + 123;
    
    /* 1. Long dependency chain with data-dependent exit */
    result ^= long_dependency_chain(seed, 50);
    
    /* 2. Recursive computation */
    result += recursive_compute(seed, 4);
    
    /* 3. Many variables with register pressure */
    result ^= many_variables(seed);
    
    /* 4. Switch-based computation */
    for (int i = 0; i < 24; i++) {
        result += switch_computation(i, seed + i);
    }
    
    /* 5. Irregular control flow */
    result ^= irregular_control_flow(100);
    
    /* 6. Software pipelining style */
    int data[64];
    for (int i = 0; i < 64; i++) {
        data[i] = (i * 13 + 7) & 0xFF;
    }
    result += pipelined_computation(data, 64);
    
    /* 7. Pointer aliasing to create dependencies */
    int *ptr1 = &global_a;
    int *ptr2 = (int*)((char*)&global_a + 0);
    
    for (int i = 0; i < 10; i++) {
        *ptr1 += *ptr2 + i;
        *ptr2 ^= *ptr1 - i;
        
        /* Force memory dependency */
        asm volatile ("" : : : "memory");
        
        result += *ptr1 + *ptr2;
    }
    
    /* 8. Complex loop with multiple exit points */
    int x = seed;
    int y = seed * 2;
    
    for (int i = 0; ; i++) {
        x = x * 3 + 7;
        y = y ^ (x << 1);
        
        /* Multiple exit conditions */
        if (x > 1000000) break;
        if (y < 0 && __builtin_expect_with_probability(i > 20, 1, 0.6)) break;
        if ((x + y) % 13 == 0) {
            asm volatile ("" : : : "memory");
            if (volatile_source) continue;
        }
        
        if (i >= 30) break;
    }
    
    result ^= x + y;
    
    /* Final result output to prevent elimination */
    printf("Result checksum: %d\n", result);
    
    return 0;
}
