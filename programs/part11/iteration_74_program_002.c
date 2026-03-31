/* early-remat-test.c
 * Test program to trigger GCC's early rematerialization pass
 * and reach uncovered validation logic in early-remat.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Global arrays to store results and prevent optimization */
volatile int global_results[100];
volatile float global_floats[100];
volatile double global_doubles[100];

/* Non-inlineable functions to force register usage */
__attribute__((noinline, noipa)) int get_int(int seed) {
    return seed * 1103515245 + 12345;
}

__attribute__((noinline, noipa)) float get_float(int seed) {
    return (seed * 0.001f) + 0.5f;
}

__attribute__((noinline, noipa)) double get_double(int seed) {
    return (seed * 0.0001) + 0.25;
}

/* Test 1: Integer-intensive computations with many live variables */
__attribute__((noinline)) int test_int_remat(int seed) {
    volatile int v1 = seed;
    volatile int v2 = seed + 1;
    volatile int v3 = seed + 2;
    volatile int v4 = seed + 3;
    volatile int v5 = seed + 4;
    volatile int v6 = seed + 5;
    volatile int v7 = seed + 6;
    volatile int v8 = seed + 7;
    volatile int v9 = seed + 8;
    volatile int v10 = seed + 9;
    
    int result = 0;
    
    /* Complex expression with many intermediate values that must stay live */
    int t1 = v1 * v2 + v3;
    int t2 = v4 * v5 - v6;
    int t3 = v7 * v8 / (v9 + 1);
    int t4 = v10 * v1 - v2;
    int t5 = v3 * v4 + v5;
    int t6 = v6 * v7 - v8;
    int t7 = v9 * v10 + v1;
    int t8 = v2 * v3 - v4;
    int t9 = v5 * v6 + v7;
    int t10 = v8 * v9 - v10;
    
    /* Force all temporaries to be live simultaneously */
    result = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
    
    /* Inline assembly to clobber registers and increase pressure */
    asm volatile (
        "# Clobber registers to force rematerialization\n"
        :
        :
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9",
          "r10", "r11", "r12", "r14"
    );
    
    /* More computations using the same variables */
    t1 = t1 * 2 + v1;
    t2 = t2 * 3 - v2;
    t3 = t3 * 4 + v3;
    t4 = t4 * 5 - v4;
    t5 = t5 * 6 + v5;
    t6 = t6 * 7 - v6;
    t7 = t7 * 8 + v7;
    t8 = t8 * 9 - v8;
    t9 = t9 * 10 + v9;
    t10 = t10 * 11 - v10;
    
    result += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
    
    return result;
}

/* Test 2: Mixed floating-point and integer computations */
__attribute__((noinline)) float test_fp_remat(int seed) {
    volatile float f1 = get_float(seed);
    volatile float f2 = get_float(seed + 100);
    volatile float f3 = get_float(seed + 200);
    volatile float f4 = get_float(seed + 300);
    volatile double d1 = get_double(seed);
    volatile double d2 = get_double(seed + 100);
    
    volatile int i1 = seed;
    volatile int i2 = seed + 1;
    volatile int i3 = seed + 2;
    volatile int i4 = seed + 3;
    
    float result = 0.0f;
    
    /* Mixed computations creating register pressure across classes */
    float ft1 = f1 * f2 + f3;
    float ft2 = f4 * f1 - f2;
    double dt1 = d1 * 2.0 + d2;
    double dt2 = d2 * 3.0 - d1;
    
    int it1 = i1 * i2 + i3;
    int it2 = i4 * i1 - i2;
    int it3 = i3 * i4 + i1;
    int it4 = i2 * i3 - i4;
    
    /* Cross-type conversions and computations */
    ft1 = ft1 + (float)dt1;
    ft2 = ft2 - (float)dt2;
    dt1 = dt1 + (double)ft1;
    dt2 = dt2 - (double)ft2;
    
    /* More operations to keep values live */
    for (int j = 0; j < 3; j++) {
        ft1 = ft1 * 1.1f + (float)it1;
        ft2 = ft2 / 1.2f - (float)it2;
        dt1 = dt1 * 1.01 + (double)it3;
        dt2 = dt2 / 1.02 - (double)it4;
        
        it1 = it1 + (int)ft1;
        it2 = it2 - (int)ft2;
        it3 = it3 + (int)dt1;
        it4 = it4 - (int)dt2;
    }
    
    result = ft1 + ft2 + (float)dt1 + (float)dt2 + 
             (float)it1 + (float)it2 + (float)it3 + (float)it4;
    
    /* Clobber both integer and floating-point registers */
    asm volatile (
        "# Clobber multiple register classes\n"
        :
        :
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
          "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7"
    );
    
    return result;
}

/* Test 3: Address calculation heavy with array accesses */
__attribute__((noinline)) int test_addr_remat(int seed) {
    static volatile int array[256];
    
    /* Initialize array with values */
    for (int i = 0; i < 256; i++) {
        array[i] = (i * 1103515245 + 12345) & 0xFF;
    }
    
    volatile int idx1 = seed & 0xFF;
    volatile int idx2 = (seed + 37) & 0xFF;
    volatile int idx3 = (seed + 73) & 0xFF;
    volatile int idx4 = (seed + 101) & 0xFF;
    volatile int idx5 = (seed + 151) & 0xFF;
    volatile int idx6 = (seed + 199) & 0xFF;
    
    int result = 0;
    
    /* Complex address calculations that are rematerialization candidates */
    for (int i = 0; i < 50; i++) {
        /* Each of these creates a base+offset computation */
        int addr1 = (idx1 * 7 + i) & 0xFF;
        int addr2 = (idx2 * 11 + i * 3) & 0xFF;
        int addr3 = (idx3 * 13 + i * 5) & 0xFF;
        int addr4 = (idx4 * 17 + i * 7) & 0xFF;
        int addr5 = (idx5 * 19 + i * 9) & 0xFF;
        int addr6 = (idx6 * 23 + i * 11) & 0xFF;
        
        /* Multiple array accesses with different address computations */
        int val1 = array[addr1];
        int val2 = array[addr2];
        int val3 = array[addr3];
        int val4 = array[addr4];
        int val5 = array[addr5];
        int val6 = array[addr6];
        
        /* Complex computation keeping all values live */
        int sum1 = val1 * val2 + val3;
        int sum2 = val4 * val5 - val6;
        int sum3 = val1 * val3 + val5;
        int sum4 = val2 * val4 - val6;
        int sum5 = val3 * val5 + val1;
        int sum6 = val4 * val6 - val2;
        
        result += sum1 + sum2 + sum3 + sum4 + sum5 + sum6;
        
        /* Update indices with complex expressions */
        idx1 = (idx1 + val1) & 0xFF;
        idx2 = (idx2 + val2) & 0xFF;
        idx3 = (idx3 + val3) & 0xFF;
        idx4 = (idx4 + val4) & 0xFF;
        idx5 = (idx5 + val5) & 0xFF;
        idx6 = (idx6 + val6) & 0xFF;
    }
    
    /* Force address computations to be rematerialized */
    asm volatile (
        "# Force address register pressure\n"
        :
        :
        : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10"
    );
    
    return result;
}

/* Test 4: Nested loops with many induction variables */
__attribute__((noinline)) int test_loop_remat(int seed) {
    int result = 0;
    
    /* Many induction variables */
    volatile int i1 = seed;
    volatile int i2 = seed + 10;
    volatile int i3 = seed + 20;
    volatile int i4 = seed + 30;
    volatile int i5 = seed + 40;
    volatile int i6 = seed + 50;
    
    for (int outer = 0; outer < 10; outer++) {
        /* Inner loop with many live values across iterations */
        for (int inner = 0; inner < 20; inner++) {
            /* Each iteration computes many values that could be rematerialized */
            int t1 = i1 * outer + inner;
            int t2 = i2 * inner - outer;
            int t3 = i3 * (outer + inner) + i1;
            int t4 = i4 * (outer - inner) - i2;
            int t5 = i5 * t1 + t2;
            int t6 = i6 * t3 - t4;
            
            /* Complex expression tree */
            int expr1 = (t1 * t2) + (t3 * t4) - (t5 * t6);
            int expr2 = (t2 * t3) + (t4 * t5) - (t6 * t1);
            int expr3 = (t3 * t4) + (t5 * t6) - (t1 * t2);
            int expr4 = (t4 * t5) + (t6 * t1) - (t2 * t3);
            
            result += expr1 + expr2 + expr3 + expr4;
            
            /* Update induction variables in complex ways */
            i1 = (i1 * 3 + inner) & 0x7F;
            i2 = (i2 * 5 - outer) & 0x7F;
            i3 = (i3 * 7 + t1) & 0x7F;
            i4 = (i4 * 11 - t2) & 0x7F;
            i5 = (i5 * 13 + t3) & 0x7F;
            i6 = (i6 * 17 - t4) & 0x7F;
        }
        
        /* Additional computation between outer loop iterations */
        int inter = i1 + i2 + i3 + i4 + i5 + i6;
        result += inter * outer;
    }
    
    return result;
}

int main() {
    volatile int seed = 12345;
    int final_result = 0;
    
    printf("Starting early rematerialization stress test...\n");
    
    /* Run all tests multiple times with different seeds */
    for (int iteration = 0; iteration < 5; iteration++) {
        int current_seed = seed + iteration * 1000;
        
        printf("Iteration %d, seed = %d\n", iteration, current_seed);
        
        /* Test 1: Integer rematerialization */
        int int_result = test_int_remat(current_seed);
        global_results[iteration * 4] = int_result;
        final_result += int_result;
        printf("  Integer test: %d\n", int_result);
        
        /* Test 2: Floating-point rematerialization */
        float fp_result = test_fp_remat(current_seed);
        global_floats[iteration] = fp_result;
        final_result += (int)fp_result;
        printf("  FP test: %f\n", fp_result);
        
        /* Test 3: Address calculation rematerialization */
        int addr_result = test_addr_remat(current_seed);
        global_results[iteration * 4 + 1] = addr_result;
        final_result += addr_result;
        printf("  Address test: %d\n", addr_result);
        
        /* Test 4: Loop induction variable rematerialization */
        int loop_result = test_loop_remat(current_seed);
        global_results[iteration * 4 + 2] = loop_result;
        final_result += loop_result;
        printf("  Loop test: %d\n", loop_result);
        
        /* Additional mixed test */
        int mixed = test_int_remat(current_seed + 500) + 
                   (int)test_fp_remat(current_seed + 600);
        global_results[iteration * 4 + 3] = mixed;
        final_result += mixed;
    }
    
    printf("Final checksum: %d\n", final_result);
    printf("Test completed.\n");
    
    return final_result != 0 ? 0 : 1;
}
