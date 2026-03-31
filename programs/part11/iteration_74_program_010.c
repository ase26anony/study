/* Test program to trigger early rematerialization validation logic */
#include <stdio.h>
#include <stdint.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Global arrays to prevent optimization */
volatile int global_seed = 12345;
int results[100];
float fresults[100];

/* Non-inlineable functions to increase register pressure */
NOINLINE int get_int(int i) {
    return global_seed + i * 1103515245;
}

NOINLINE float get_float(int i) {
    return (global_seed + i) * 0.12345f;
}

/* Integer-intensive test with many live variables */
NOINLINE int test_int_remat(int iterations) {
    volatile int v1 = get_int(1);
    volatile int v2 = get_int(2);
    volatile int v3 = get_int(3);
    volatile int v4 = get_int(4);
    volatile int v5 = get_int(5);
    volatile int v6 = get_int(6);
    volatile int v7 = get_int(7);
    volatile int v8 = get_int(8);
    volatile int v9 = get_int(9);
    volatile int v10 = get_int(10);
    
    int a = v1, b = v2, c = v3, d = v4, e = v5;
    int f = v6, g = v7, h = v8, i = v9, j = v10;
    
    int sum = 0;
    
    for (int it = 0; it < iterations; it++) {
        /* Many independent computations creating register pressure */
        int t1 = a + b * c;      /* remat candidate: b * c */
        int t2 = d - e + f;      /* remat candidate: d - e */
        int t3 = g * h >> 2;     /* remat candidate: g * h */
        int t4 = i ^ j & 0xFF;   /* remat candidate: j & 0xFF */
        int t5 = t1 | t2;
        int t6 = t3 & t4;
        int t7 = t5 + t6;
        int t8 = t7 * 3;
        int t9 = t8 - a;
        int t10 = t9 + b;
        
        /* Force all values to be live simultaneously */
        sum += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
        
        /* Modify variables to prevent constant propagation */
        a += 1; b += 2; c += 3; d += 4; e += 5;
        f += 6; g += 7; h += 8; i += 9; j += 10;
        
        /* Inline assembly to clobber registers and increase pressure */
        asm volatile("" : : : "memory", "r0", "r1", "r2", "r3", "r4", "r5");
    }
    
    return sum;
}

/* Floating-point test mixing int and float operations */
NOINLINE float test_fp_remat(int iterations) {
    volatile float f1 = get_float(1);
    volatile float f2 = get_float(2);
    volatile float f3 = get_float(3);
    volatile float f4 = get_float(4);
    volatile float f5 = get_float(5);
    volatile int i1 = get_int(1);
    volatile int i2 = get_int(2);
    volatile int i3 = get_int(3);
    
    float a = f1, b = f2, c = f3, d = f4, e = f5;
    int idx1 = i1, idx2 = i2, idx3 = i3;
    
    float sum = 0.0f;
    
    for (int it = 0; it < iterations; it++) {
        /* Mix float and int computations */
        float t1 = a * b + c;            /* remat candidate: a * b */
        float t2 = d / e - 1.5f;         /* remat candidate: d / e */
        float t3 = t1 + t2 * 2.0f;       /* remat candidate: t2 * 2.0f */
        float t4 = (float)idx1 * t3;     /* remat candidate: (float)idx1 */
        float t5 = t4 / (float)idx2;
        
        /* Integer address calculations mixed in */
        int offset = idx1 + idx2 * 3 - idx3;  /* remat candidate: idx2 * 3 */
        float t6 = t5 * (float)offset;
        
        sum += t1 + t2 + t3 + t4 + t5 + t6;
        
        /* Modify values */
        a += 0.1f; b += 0.2f; c += 0.3f; d += 0.4f; e += 0.5f;
        idx1 += 1; idx2 += 2; idx3 += 3;
        
        /* Clobber more registers */
        asm volatile("" : : : "memory", "r0", "r1", "r2", "r3", 
                     "s0", "s1", "s2", "s3", "s4");
    }
    
    return sum;
}

/* Address calculation intensive test */
NOINLINE int test_addr_remat(int iterations) {
    static int array[1024];
    
    /* Initialize array */
    for (int i = 0; i < 1024; i++) {
        array[i] = i * 3;
    }
    
    volatile int v1 = get_int(1);
    volatile int v2 = get_int(2);
    volatile int v3 = get_int(3);
    volatile int v4 = get_int(4);
    
    int a = v1, b = v2, c = v3, d = v4;
    int sum = 0;
    const int mask = 1023;
    
    for (int it = 0; it < iterations; it++) {
        /* Multiple complex address calculations - prime remat candidates */
        int idx1 = (a * 7 + b) & mask;      /* remat: a * 7 */
        int idx2 = (c * 13 + d) & mask;     /* remat: c * 13 */
        int idx3 = (a + b * 5) & mask;      /* remat: b * 5 */
        int idx4 = (c * 3 + d * 11) & mask; /* remat: c * 3, d * 11 */
        int idx5 = (a * 17 - b) & mask;     /* remat: a * 17 */
        
        /* Use all indices with different computations */
        int val1 = array[idx1] + array[idx2];
        int val2 = array[idx3] * array[idx4];
        int val3 = array[idx5] - array[idx1];
        int val4 = array[idx2] & array[idx3];
        int val5 = array[idx4] | array[idx5];
        
        /* Combine results */
        sum += val1 + val2 + val3 + val4 + val5;
        
        /* Modify base values */
        a = (a + 1) & mask;
        b = (b * 3) & mask;
        c = (c + 5) & mask;
        d = (d * 7) & mask;
        
        /* Force register pressure with inline asm */
        asm volatile("" : : : "memory", "r0", "r1", "r2", "r3", 
                     "r4", "r5", "r6", "r7", "r8", "r9");
    }
    
    return sum;
}

/* Combined test with nested loops for maximum pressure */
NOINLINE int test_combined_remat(int outer_iters, int inner_iters) {
    int total = 0;
    
    for (int outer = 0; outer < outer_iters; outer++) {
        volatile int base = get_int(outer);
        
        /* Many local variables in nested scope */
        int a = base + 1;
        int b = base + 2;
        int c = base + 3;
        int d = base + 4;
        int e = base + 5;
        int f = base + 6;
        int g = base + 7;
        int h = base + 8;
        int i = base + 9;
        int j = base + 10;
        
        int inner_sum = 0;
        
        for (int inner = 0; inner < inner_iters; inner++) {
            /* Complex expression with many intermediate values */
            int t1 = (a * b) + (c * d);     /* remat candidates: a*b, c*d */
            int t2 = (e << 2) | (f >> 3);   /* remat: e<<2, f>>3 */
            int t3 = (g & h) ^ (i | j);     /* remat: g&h, i|j */
            int t4 = t1 - t2 + t3;
            int t5 = t4 * 3 - a;
            int t6 = t5 / (b + 1);          /* remat: b+1 */
            int t7 = t6 + c - d;
            int t8 = t7 * e / f;
            int t9 = t8 | g & h;
            int t10 = t9 ^ i + j;
            
            inner_sum += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
            
            /* Rotate values to create varying live ranges */
            int tmp = a;
            a = b; b = c; c = d; d = e; e = f;
            f = g; g = h; h = i; i = j; j = tmp;
            
            /* Memory barrier and register clobber */
            asm volatile("" : : : "memory", "r0", "r1", "r2", "r3", "r4",
                         "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12");
        }
        
        total += inner_sum;
    }
    
    return total;
}

int main() {
    int checksum = 0;
    
    printf("Starting early rematerialization stress test...\n");
    
    /* Run all tests with different parameters */
    results[0] = test_int_remat(100);
    checksum += results[0];
    printf("test_int_remat: %d\n", results[0]);
    
    fresults[0] = test_fp_remat(50);
    checksum += (int)fresults[0];
    printf("test_fp_remat: %f\n", fresults[0]);
    
    results[1] = test_addr_remat(200);
    checksum += results[1];
    printf("test_addr_remat: %d\n", results[1]);
    
    results[2] = test_combined_remat(10, 20);
    checksum += results[2];
    printf("test_combined_remat: %d\n", results[2]);
    
    printf("Final checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
