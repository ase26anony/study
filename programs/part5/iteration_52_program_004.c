/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-gcse -o test test.c */
/* For maximum stress: gcc -O3 -funroll-loops -fno-gcse -march=native -fno-schedule-insns */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to create opaque values */
extern int get_opaque_value(int seed);

/* Stress function with complex register pressure patterns */
static int __attribute__((noinline)) 
stress_computation(volatile int seed, int iterations) {
    /* Volatile variables to prevent optimization */
    volatile int v1 = seed;
    volatile int v2 = seed * 2;
    volatile int v3 = seed + 7;
    volatile int v4 = seed - 3;
    
    /* Complex arithmetic chain creating many temporaries */
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Long dependency chain with volatile accesses */
        int t1 = v1 * v2 + v3 / (v4 + 1);
        int t2 = t1 % (v2 + 1) - v3 * v4;
        int t3 = t2 + (v1 << 2) | (v2 >> 1);
        int t4 = t3 ^ (v3 * 2) & (v4 - 1);
        
        /* Multi-use temporary value - candidate for rematerialization */
        int base = t4 * 3 + get_opaque_value(i);
        
        /* Use base in multiple different contexts */
        if (i % 3 == 0) {
            result += base * 2;
        } else if (i % 3 == 1) {
            result += base / 2;
        } else {
            result += base % 100;
        }
        
        /* Address computation with multiple offsets */
        char buffer[256];
        char *ptr = &buffer[i % 128];
        
        /* Multiple uses of ptr with different offsets */
        ptr[0] = (char)(base & 0xFF);
        ptr[1] = (char)((base >> 8) & 0xFF);
        ptr[2] = (char)((base >> 16) & 0xFF);
        ptr[-1] = (char)(result & 0xFF);
        
        /* Inline assembly to clobber registers */
        #ifdef __OPTIMIZE__
        asm volatile (
            "# Clobber multiple registers\n"
            :
            :
            : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
        );
        #endif
        
        /* More complex arithmetic to increase register pressure */
        v1 = (v1 * 1103515245 + 12345) & 0x7fffffff;
        v2 = (v2 * 1664525 + 1013904223) & 0x7fffffff;
        v3 = v3 ^ (v1 >> 16);
        v4 = v4 + (v2 % 17);
    }
    
    return result;
}

/* Another stress function with different patterns */
static int __attribute__((noinline))
stress_computation2(int seed, int n) {
    volatile int a = seed;
    volatile int b = seed + 1;
    volatile int c = seed + 2;
    volatile int d = seed + 3;
    
    int sum = 0;
    
    /* Loop with volatile dependencies preventing optimization */
    for (volatile int i = 0; i < n; i = i + 1) {
        /* Complex expression chain */
        int x = a * b + c / (d + 1);
        int y = x % (b + 1) - c * d;
        int z = y + (a << 3) | (b >> 2);
        
        /* Create value used in multiple places */
        int multi_use = z * a - b + c;
        
        /* Use in switch to create different basic blocks */
        switch (i % 4) {
            case 0:
                sum += multi_use * 2;
                break;
            case 1:
                sum += multi_use / 2;
                break;
            case 2:
                sum += multi_use % 255;
                break;
            case 3:
                sum += multi_use ^ 0xAA;
                break;
        }
        
        /* More register pressure */
        a = (a * 3) % 100;
        b = (b + 5) % 100;
        c = (c * 7) % 100;
        d = (d * 11) % 100;
        
        #ifdef __OPTIMIZE__
        /* Additional clobber for different register set */
        asm volatile (
            "# Clobber more registers\n"
            :
            :
            : "r6", "r7", "r8", "r9", "r10", "memory"
        );
        #endif
    }
    
    return sum;
}

/* Opaque function implementation */
int get_opaque_value(int seed) {
    /* Use system rand to prevent compiler analysis */
    return rand() ^ (seed * 31);
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    srand(time(NULL));
    
    /* Call stress functions multiple times from different contexts */
    int total = 0;
    
    for (int j = 0; j < 3; j++) {
        total += stress_computation(j * 100, iterations);
        total += stress_computation2(j * 50, iterations / 2);
        
        /* Additional call with different parameters */
        if (j % 2 == 0) {
            total += stress_computation(total, 10);
        } else {
            total += stress_computation2(total, 15);
        }
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
