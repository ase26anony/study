#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_counter = 0;
int g_array[256];
int g_result = 0;

/* Helper functions with specific attributes */
__attribute__((noinline, optimize("O3")))
int compute_chain(int seed) {
    volatile int barrier;
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    int i = seed + 8, j = seed + 9, k = seed + 10, l = seed + 11;
    int m = seed + 12, n = seed + 13, o = seed + 14, p = seed + 15;
    
    /* Long chain of dependent operations */
    a += b * c;
    d ^= e + f;
    g *= h - i;
    j |= k & l;
    m = n ^ o;
    p = a + d;
    
    /* Memory barrier to split scheduling region */
    asm volatile("" : : : "memory");
    
    /* More dependent operations */
    b = c + d;
    e = f * g;
    h = i ^ j;
    k = l | m;
    n = o - p;
    a = b + c;
    
    /* Data-dependent exit condition */
    int counter = 0;
    while (__builtin_expect_with_probability(counter < 8, 1, 0.7)) {
        a += b;
        b ^= c;
        c *= d;
        d |= e;
        e = f + g;
        counter++;
        
        /* Early break with probability */
        if (__builtin_expect_with_probability(a > 1000000, 0, 0.3)) {
            barrier = a;
            break;
        }
    }
    
    /* Final computation */
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
}

__attribute__((noinline, optimize("O3")))
int recursive_compute(int depth, int val) {
    if (__builtin_expect_with_probability(depth <= 0, 0, 0.4)) {
        return val;
    }
    
    int local1 = val * 3;
    int local2 = val + 7;
    int local3 = val ^ 0x55;
    int local4 = val | 0xFF;
    
    /* Create register pressure */
    int t1 = local1 + local2;
    int t2 = local3 * local4;
    int t3 = t1 ^ t2;
    int t4 = t1 | t2;
    int t5 = t3 & t4;
    int t6 = t5 << 2;
    int t7 = t6 >> 1;
    int t8 = t7 + 1;
    int t9 = t8 * 3;
    int t10 = t9 ^ 0xAA;
    
    /* Recursive call - scheduler may save/restore state here */
    int rec_result = recursive_compute(depth - 1, t10);
    
    /* More operations after recursion */
    asm volatile("" : : : "memory");
    
    return rec_result + t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
}

__attribute__((noinline, optimize("O3")))
int switch_complex(int selector) {
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0;
    int r7 = 0, r8 = 0, r9 = 0, r10 = 0, r11 = 0, r12 = 0;
    
    /* Complex switch with many cases - scheduler may need state management */
    switch (selector & 0xF) {
        case 0:
            r1 = selector * 2;
            r2 = selector + 1;
            r3 = r1 ^ r2;
            for (int i = 0; i < 4; i++) {
                r1 += r2;
                r2 ^= r3;
            }
            break;
        case 1:
            r4 = selector | 0x1111;
            r5 = selector & 0x2222;
            r6 = r4 - r5;
            asm volatile("" : : : "memory");
            break;
        case 2:
            r7 = selector << 3;
            r8 = selector >> 1;
            r9 = r7 | r8;
            break;
        case 3:
            r10 = selector * 3;
            r11 = selector + 7;
            r12 = r10 ^ r11;
            break;
        case 4:
            r1 = selector * 5;
            r3 = selector + 11;
            r5 = r1 & r3;
            break;
        case 5:
            r2 = selector | 0xAAAA;
            r4 = selector ^ 0x5555;
            r6 = r2 + r4;
            break;
        case 6:
            r7 = selector * 7;
            r9 = selector - 3;
            r11 = r7 | r9;
            break;
        case 7:
            r8 = selector << 2;
            r10 = selector >> 2;
            r12 = r8 ^ r10;
            break;
        case 8:
            r1 = selector + 100;
            r4 = selector * 13;
            r7 = r1 - r4;
            break;
        case 9:
            r2 = selector & 0x0F0F;
            r5 = selector | 0xF0F0;
            r8 = r2 + r5;
            break;
        case 10:
            r3 = selector * 17;
            r6 = selector + 19;
            r9 = r3 ^ r6;
            break;
        case 11:
            r4 = selector << 4;
            r7 = selector >> 4;
            r10 = r4 | r7;
            break;
        case 12:
            r5 = selector * 19;
            r8 = selector - 23;
            r11 = r5 & r8;
            break;
        case 13:
            r6 = selector ^ 0x3333;
            r9 = selector | 0xCCCC;
            r12 = r6 + r9;
            break;
        case 14:
            r7 = selector * 23;
            r10 = selector + 29;
            r1 = r7 - r10;
            break;
        case 15:
            r8 = selector << 5;
            r11 = selector >> 3;
            r2 = r8 ^ r11;
            break;
    }
    
    /* Merge point after switch - scheduler may restore state here */
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12;
}

__attribute__((noinline, optimize("O3")))
int loop_with_inner_function(int iterations) {
    int sum = 0;
    
    for (int i = 0; __builtin_expect_with_probability(i < iterations, 1, 0.8); i++) {
        /* Call to small function creates scheduling region boundaries */
        int inner_result = compute_chain(i);
        
        /* Software pipelining style computation */
        int a = inner_result;
        int b = a + i;
        int c = b * 3;
        int d = c ^ 0xFF;
        
        /* Irregular control flow with goto */
        if (__builtin_expect_with_probability((d & 0x3) == 0, 0, 0.2)) {
            sum += d;
            continue;
        }
        
        /* do-while with break */
        int counter = 0;
        do {
            if (__builtin_expect_with_probability(counter > 3, 0, 0.1)) {
                asm volatile("" : : : "memory");
                break;
            }
            a += b;
            b ^= c;
            c *= d;
            counter++;
        } while (counter < 5);
        
        sum += a + b + c + d;
        
        /* Memory operation with uncertain latency */
        g_array[i & 0xFF] = sum;
        volatile int mem_read = g_array[(i + 1) & 0xFF];
        (void)mem_read;
    }
    
    return sum;
}

int main() {
    int total = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 3;
    }
    
    /* Kernel 1: Long chain with data-dependent exit */
    total += compute_chain(42);
    
    /* Kernel 2: Complex switch statement */
    for (int i = 0; i < 32; i++) {
        total += switch_complex(i);
    }
    
    /* Kernel 3: Loop with inner function calls */
    total += loop_with_inner_function(50);
    
    /* Kernel 4: Recursive computation */
    total += recursive_compute(4, 100);
    
    /* Create pointer aliasing for scheduler uncertainty */
    int* alias1 = &g_result;
    volatile int* alias2 = (volatile int*)&g_result;
    
    /* Mixed operations with aliased pointers */
    for (int i = 0; i < 100; i++) {
        *alias1 += i;
        int temp = *alias2;
        *alias1 ^= temp;
        asm volatile("" : : : "memory");
    }
    
    total += g_result;
    
    /* Final checksum */
    printf("Result checksum: %d\n", total);
    
    return 0;
}
