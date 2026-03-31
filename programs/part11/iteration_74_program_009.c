/* Test program to trigger early rematerialization pass in GCC */
/* Specifically targeting uncovered lines 930-937 in early-remat.cc */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Global arrays to prevent optimization */
volatile int global_seed = 12345;
int results[100] = {0};
int result_index = 0;

/* Non-inlineable functions to force register usage */
__attribute__((noinline, noipa)) int get_value(int idx) {
    return global_seed + idx * 1103515245;
}

__attribute__((noinline, noipa)) float get_float(int idx) {
    return (float)(global_seed + idx) * 0.12345f;
}

__attribute__((noinline, noipa)) double get_double(int idx) {
    return (double)(global_seed - idx) * 0.67891;
}

/* Integer-intensive test with many live variables */
int test_int_remat(int iterations) {
    /* Declare many integer variables to create register pressure */
    volatile int v0 = get_value(0);
    volatile int v1 = get_value(1);
    volatile int v2 = get_value(2);
    volatile int v3 = get_value(3);
    volatile int v4 = get_value(4);
    volatile int v5 = get_value(5);
    volatile int v6 = get_value(6);
    volatile int v7 = get_value(7);
    volatile int v8 = get_value(8);
    volatile int v9 = get_value(9);
    
    int sum = 0;
    
    /* Complex loop with many independent computations */
    for (int i = 0; i < iterations; i++) {
        /* Many arithmetic operations creating rematerialization candidates */
        int t0 = v0 + v1 * 3;      /* Candidate for remat */
        int t1 = v2 - v3 / 2;      /* Candidate for remat */
        int t2 = v4 ^ v5;          /* Candidate for remat */
        int t3 = v6 | v7;          /* Candidate for remat */
        int t4 = v8 & v9;          /* Candidate for remat */
        
        /* More computations keeping many values live */
        int t5 = t0 * t1 + i;
        int t6 = t2 ^ t3 ^ t4;
        int t7 = (t5 << 3) | (t6 >> 2);
        int t8 = t7 * 0x1234567;
        int t9 = t8 + t0 - t1;
        
        /* Inline assembly to clobber registers and increase pressure */
        /* Adjust clobber list based on target architecture */
        asm volatile(
            "# Force register pressure\n"
            : 
            : 
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12"
        );
        
        /* Use all computed values to prevent elimination */
        sum += t0 + t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9;
        
        /* Modify some values to create new live ranges */
        v0 += t0 & 0xFF;
        v1 -= t1 % 31;
        v2 ^= t2;
        v3 |= t3;
        v4 &= t4;
    }
    
    return sum;
}

/* Floating-point intensive test */
float test_fp_remat(int iterations) {
    /* Mix float and double variables */
    volatile float f0 = get_float(0);
    volatile float f1 = get_float(1);
    volatile float f2 = get_float(2);
    volatile float f3 = get_float(3);
    volatile double d0 = get_double(0);
    volatile double d1 = get_double(1);
    volatile double d2 = get_double(2);
    volatile double d3 = get_double(3);
    
    float sum = 0.0f;
    
    for (int i = 0; i < iterations; i++) {
        /* Many FP operations creating rematerialization opportunities */
        float ft0 = f0 * 1.5f + f1;      /* Remat candidate */
        float ft1 = f2 / 2.0f - f3;      /* Remat candidate */
        double dt0 = d0 * 0.75 + d1;     /* Remat candidate */
        double dt1 = d2 / 1.333 + d3;    /* Remat candidate */
        
        /* Mixed precision computations */
        float ft2 = (float)(dt0) + ft0;
        float ft3 = (float)(dt1) * ft1;
        double dt2 = (double)(ft0) * d0;
        double dt3 = (double)(ft1) / d1;
        
        /* More computations to increase live ranges */
        float ft4 = ft2 * ft3 - (float)i;
        double dt4 = dt2 + dt3 * i;
        float ft5 = ft4 + (float)dt4;
        double dt5 = dt4 - (double)ft4;
        
        /* Use all values */
        sum += ft0 + ft1 + ft2 + ft3 + ft4 + ft5 + (float)(dt0 + dt1 + dt2 + dt3 + dt4 + dt5);
        
        /* Modify values for next iteration */
        f0 += ft0 * 0.1f;
        f1 -= ft1 * 0.05f;
        d0 *= 1.01;
        d1 /= 1.02;
    }
    
    return sum;
}

/* Address calculation intensive test */
int test_addr_remat(int iterations) {
    /* Create arrays and pointers */
    int array1[256];
    int array2[256];
    int array3[256];
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        array1[i] = get_value(i);
        array2[i] = get_value(i + 256);
        array3[i] = get_value(i + 512);
    }
    
    int sum = 0;
    
    /* Complex addressing patterns */
    for (int i = 0; i < iterations; i++) {
        /* Many address calculations - prime candidates for remat */
        int idx0 = (i * 7) & 0xFF;           /* Remat candidate */
        int idx1 = (i * 13 + 5) & 0xFF;      /* Remat candidate */
        int idx2 = (i * 31 - 11) & 0xFF;     /* Remat candidate */
        int idx3 = (i * 19 + 7) & 0xFF;      /* Remat candidate */
        int idx4 = (i * 23 - 3) & 0xFF;      /* Remat candidate */
        
        /* More complex index calculations */
        int idx5 = (idx0 * 3 + idx1) & 0xFF;
        int idx6 = (idx2 * 5 - idx3) & 0xFF;
        int idx7 = (idx4 * 7 + idx5) & 0xFF;
        int idx8 = (idx6 * 11 - idx7) & 0xFF;
        int idx9 = (idx8 * 13 + i) & 0xFF;
        
        /* Multiple array accesses with complex addressing */
        int val0 = array1[idx0] + array2[idx1];
        int val1 = array3[idx2] * array1[idx3];
        int val2 = array2[idx4] ^ array3[idx5];
        int val3 = array1[idx6] | array2[idx7];
        int val4 = array3[idx8] & array1[idx9];
        
        /* Even more address calculations */
        int idx10 = (val0 + idx0) & 0xFF;
        int idx11 = (val1 - idx1) & 0xFF;
        int idx12 = (val2 ^ idx2) & 0xFF;
        int idx13 = (val3 | idx3) & 0xFF;
        int idx14 = (val4 & idx4) & 0xFF;
        
        /* Additional array accesses */
        int val5 = array2[idx10] + array3[idx11];
        int val6 = array1[idx12] * array2[idx13];
        int val7 = array3[idx14] - array1[idx10];
        
        /* Combine all values */
        sum += val0 + val1 + val2 + val3 + val4 + val5 + val6 + val7;
        
        /* Update arrays to create new live ranges */
        array1[idx0] += val0;
        array2[idx1] -= val1;
        array3[idx2] ^= val2;
    }
    
    return sum;
}

/* Test with mixed operations and control flow */
int test_mixed_remat(int iterations) {
    volatile int a = get_value(0);
    volatile int b = get_value(1);
    volatile int c = get_value(2);
    volatile int d = get_value(3);
    volatile int e = get_value(4);
    volatile int f = get_value(5);
    volatile int g = get_value(6);
    volatile int h = get_value(7);
    
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Create many rematerialization candidates */
        int base = i * 0x12345;          /* Remat candidate */
        int offset1 = a * 3 + b;         /* Remat candidate */
        int offset2 = c * 5 - d;         /* Remat candidate */
        int offset3 = e ^ f;             /* Remat candidate */
        int offset4 = g | h;             /* Remat candidate */
        
        /* Complex expression tree */
        int t0 = base + offset1;
        int t1 = base - offset2;
        int t2 = base * offset3;
        int t3 = base & offset4;
        
        int t4 = t0 * t1 + t2;
        int t5 = t2 - t3 * t0;
        int t6 = t4 ^ t5;
        int t7 = t6 | (t0 + t1);
        int t8 = t7 & (t2 - t3);
        int t9 = t8 * 0xABCDEF;
        
        /* Conditional to create different control flow paths */
        if (i & 1) {
            t0 = t0 * 2;
            t1 = t1 + t9;
            /* More computations in this path */
            int t10 = t0 * t1 - t2;
            int t11 = t3 ^ t4 | t5;
            sum += t10 + t11;
        } else {
            t2 = t2 / 2;
            t3 = t3 - t9;
            /* Different computations in this path */
            int t12 = t6 & t7 | t8;
            int t13 = t0 * t1 + t2 * t3;
            sum += t12 + t13;
        }
        
        /* Use all temporaries */
        sum += t0 + t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9;
        
        /* Modify base variables */
        a += t0 & 0xF;
        b -= t1 % 17;
        c ^= t2;
        d |= t3;
    }
    
    return sum;
}

int main() {
    int total_sum = 0;
    
    printf("Starting early rematerialization stress test...\n");
    
    /* Run all tests multiple times with different iteration counts */
    for (int run = 0; run < 3; run++) {
        printf("Run %d:\n", run + 1);
        
        /* Test 1: Integer operations */
        int int_result = test_int_remat(100 + run * 50);
        results[result_index++] = int_result;
        total_sum += int_result;
        printf("  Integer test: %d\n", int_result);
        
        /* Test 2: Floating point operations */
        float fp_result = test_fp_remat(50 + run * 25);
        results[result_index++] = (int)fp_result;
        total_sum += (int)fp_result;
        printf("  FP test: %f\n", fp_result);
        
        /* Test 3: Address calculations */
        int addr_result = test_addr_remat(75 + run * 35);
        results[result_index++] = addr_result;
        total_sum += addr_result;
        printf("  Address test: %d\n", addr_result);
        
        /* Test 4: Mixed operations */
        int mixed_result = test_mixed_remat(60 + run * 30);
        results[result_index++] = mixed_result;
        total_sum += mixed_result;
        printf("  Mixed test: %d\n", mixed_result);
    }
    
    /* Compute final checksum */
    int checksum = total_sum;
    for (int i = 0; i < result_index; i++) {
        checksum ^= results[i] * 0x1234567;
        checksum = (checksum << 3) | (checksum >> 29); /* rotate */
    }
    
    printf("\nFinal checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return checksum != 0 ? 0 : 1;
}
