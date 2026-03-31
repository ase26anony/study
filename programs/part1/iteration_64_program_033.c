#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Global variables to create pointer aliasing
volatile int global_a = 42;
volatile int global_b = 73;
int global_array[256];

// Function attributes to control optimization
__attribute__((noinline)) 
__attribute__((optimize("O3")))
static int complex_chain(int seed) {
    int a = seed, b = seed * 2, c = seed + 1;
    int d = 0, e = 0, f = 0, g = 0, h = 0;
    int i = 0, j = 0, k = 0, l = 0, m = 0;
    int n = 0, o = 0, p = 0, q = 0, r = 0;
    int s = 0, t = 0, u = 0, v = 0, w = 0;
    
    // Long chain of dependent operations
    a += global_a;
    b ^= a;
    c *= b;
    d = c + global_b;
    e = d ^ seed;
    f = e * 3;
    g = f + a;
    h = g ^ b;
    i = h * 5;
    j = i + c;
    k = j ^ d;
    l = k * 7;
    m = l + e;
    n = m ^ f;
    o = n * 11;
    p = o + g;
    q = p ^ h;
    r = q * 13;
    s = r + i;
    t = s ^ j;
    u = t * 17;
    v = u + k;
    w = v ^ l;
    
    // Memory barrier to split scheduling regions
    asm volatile("" : : : "memory");
    
    // Data-dependent loop with unpredictable exit
    int idx = seed & 0xFF;
    while (global_array[idx] != 0) {
        w += global_array[idx];
        idx = (idx * 13 + 17) & 0xFF;
        if (__builtin_expect_with_probability(idx == 0, 0, 0.1)) {
            break;
        }
    }
    
    return w;
}

__attribute__((noinline))
__attribute__((optimize("O3")))
static int recursive_compute(int depth, int val) {
    if (depth <= 0) {
        return val;
    }
    
    int local1 = val * 2;
    int local2 = val + global_a;
    int local3 = val ^ global_b;
    
    // Memory operation in the middle
    volatile int mem_read = global_array[val & 0xFF];
    local1 += mem_read;
    
    // Recursive call - scheduler may save/restore state around this
    int result = recursive_compute(depth - 1, local1 + local2 + local3);
    
    // More operations after recursion
    result ^= local1;
    result *= local2;
    result += local3;
    
    return result;
}

__attribute__((noinline))
__attribute__((optimize("O3")))
static int switch_complex(int selector) {
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
    int r6 = 0, r7 = 0, r8 = 0, r9 = 0, r10 = 0;
    
    // Large switch to create complex control flow
    switch (selector & 0xF) {
        case 0:
            r1 = global_a * 2;
            r2 = global_b ^ r1;
            for (int i = 0; i < 3; i++) r1 += i;
            break;
        case 1:
            r3 = global_b + 1;
            r4 = global_a * r3;
            asm volatile("" : : : "memory");
            break;
        case 2:
            r5 = complex_chain(selector);
            r6 = r5 ^ selector;
            break;
        case 3:
            r7 = selector * 7;
            r8 = r7 + global_a;
            break;
        case 4:
            r9 = global_b - selector;
            r10 = r9 * 3;
            break;
        case 5:
            r1 = selector ^ 0x55;
            r2 = r1 + global_a;
            break;
        case 6:
            r3 = global_b * selector;
            r4 = r3 >> 2;
            break;
        case 7:
            r5 = selector + 100;
            r6 = r5 - global_a;
            break;
        case 8:
            r7 = complex_chain(selector + 1);
            r8 = r7 * 2;
            break;
        case 9:
            r9 = selector | 0xFF;
            r10 = r9 & global_b;
            break;
        case 10:
            r1 = global_a + global_b;
            r2 = r1 * selector;
            break;
        case 11:
            r3 = selector << 3;
            r4 = r3 ^ global_a;
            break;
        case 12:
            r5 = recursive_compute(2, selector);
            r6 = r5 + 42;
            break;
        case 13:
            r7 = selector % 17;
            r8 = r7 + global_b;
            break;
        case 14:
            r9 = global_a ^ global_b;
            r10 = r9 * selector;
            break;
        case 15:
            r1 = selector * selector;
            r2 = r1 - global_a;
            break;
        default:
            r1 = 0;
    }
    
    // Merge point with many live variables
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
}

__attribute__((noinline))
__attribute__((optimize("O3")))
static int software_pipelined_loop(int iterations) {
    int acc = 0;
    
    // Outer loop that calls inner computation
    for (int i = 0; i < iterations; i++) {
        // Small inner computation that benefits from pipelining
        int t1 = i * 3;
        int t2 = t1 + global_a;
        int t3 = t2 ^ global_b;
        int t4 = t3 * 5;
        
        // Memory operation with uncertain latency
        volatile int* ptr = &global_array[i & 0xFF];
        int mem_val = *ptr;
        
        int t5 = t4 + mem_val;
        int t6 = t5 * 7;
        
        // Conditional that might be speculated
        if (__builtin_expect_with_probability((i & 0x3) == 0, 0, 0.25)) {
            t6 = complex_chain(t6);
        }
        
        acc += t6;
        
        // Irregular control flow with goto
        if ((i & 0x7) == 0) {
            static int counter = 0;
            counter++;
            if (counter < 3) {
                i--;  // Create loop with irregular progression
                continue;
            }
        }
    }
    
    return acc;
}

int main() {
    // Initialize global array with non-zero values
    for (int i = 0; i < 256; i++) {
        global_array[i] = (i * 31 + 7) & 0xFF;
    }
    
    int checksum = 0;
    
    // Kernel 1: Long chain with data-dependent exit
    checksum += complex_chain(42);
    
    // Kernel 2: Switch with many cases
    for (int i = 0; i < 32; i++) {
        checksum ^= switch_complex(i);
    }
    
    // Kernel 3: Recursive computation
    checksum += recursive_compute(4, checksum & 0xFF);
    
    // Kernel 4: Software-pipelined style loop
    checksum += software_pipelined_loop(100);
    
    // Kernel 5: Mixed operations with irregular control flow
    int x = 0;
    do {
        int y = 0;
        while (y < 10) {
            x += global_array[(x + y) & 0xFF];
            y++;
            if (__builtin_expect_with_probability((y & 1) == 0, 0, 0.3)) {
                x ^= complex_chain(y);
            }
        }
        
        // Break inside conditional
        if (x > 1000) {
            break;
        }
        
        x = switch_complex(x & 0xF);
    } while (0);
    
    checksum += x;
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
