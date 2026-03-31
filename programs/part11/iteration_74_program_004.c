/* Test program to trigger early rematerialization pass in GCC */
/* Compile with: gcc -O3 -fno-omit-frame-pointer -funroll-loops -mtune=native -fdump-rtl-early-remat -o remat_test remat_test.c */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Global arrays to prevent optimization */
volatile int global_seed = 12345;
int results[100] = {0};
int result_index = 0;

/* Non-inlineable functions to force register pressure */
__attribute__((noinline, noipa)) int get_value1(int x) { return x * 3 + 7; }
__attribute__((noinline, noipa)) int get_value2(int x) { return x * 5 - 11; }
__attribute__((noinline, noipa)) float get_float1(float x) { return x * 1.5f; }
__attribute__((noinline, noipa)) double get_double1(double x) { return x * 2.5; }

/* Integer-intensive test with many live variables */
int test_int_remat(void) {
    volatile int seed = global_seed;
    
    /* Declare many integer variables to create register pressure */
    int a = seed + 1;
    int b = seed * 2;
    int c = seed / 3;
    int d = seed - 100;
    int e = seed ^ 0x55AA;
    int f = seed << 2;
    int g = seed >> 1;
    int h = seed | 0xFF00;
    int i = seed & 0x0F0F;
    int j = ~seed;
    int k = seed % 17;
    int l = seed * 3;
    int m = seed + 255;
    int n = seed - 512;
    int o = seed * seed;
    
    /* Complex computation chain keeping many values live */
    int t1 = a + b + c;
    int t2 = d * e - f;
    int t3 = g ^ h | i;
    int t4 = j + k * 2;
    int t5 = l / 3 + m;
    int t6 = n ^ o;
    
    /* More intermediate values */
    int u1 = t1 * t2;
    int u2 = t3 + t4;
    int u3 = t5 - t6;
    int u4 = t1 ^ t2;
    int u5 = t3 * t4;
    int u6 = t5 + t6;
    
    /* Inline assembly to clobber registers and increase pressure */
    /* Adjust clobber list based on target architecture */
    asm volatile(
        "# Force register pressure\n"
        : 
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "r14"
    );
    
    /* Final computation using all intermediate values */
    int result = u1 + u2 * 3 - u3 / 2 + u4 ^ u5 | u6;
    
    /* Use non-inlineable calls to prevent optimization */
    result += get_value1(a);
    result += get_value2(b);
    
    return result;
}

/* Floating-point intensive test */
float test_fp_remat(void) {
    volatile float fseed = (float)global_seed / 1000.0f;
    
    /* Many floating-point variables */
    float fa = fseed + 1.1f;
    float fb = fseed * 2.2f;
    float fc = fseed / 3.3f;
    float fd = fseed - 4.4f;
    float fe = fseed * fseed;
    float ff = 1.0f / fseed;
    float fg = fseed + 5.5f;
    float fh = fseed * 6.6f;
    float fi = fseed - 7.7f;
    float fj = fseed / 8.8f;
    
    /* Mixed integer indexing */
    int idx1 = (int)(fseed * 10);
    int idx2 = (int)(fseed * 20);
    int idx3 = (int)(fseed * 30);
    int idx4 = (int)(fseed * 40);
    
    /* Complex floating-point computations */
    float ft1 = fa + fb * fc;
    float ft2 = fd - fe / ff;
    float ft3 = fg * fh + fi;
    float ft4 = fj * fa - fb;
    float ft5 = fc + fd * fe;
    float ft6 = ff - fg / fh;
    
    /* More computations keeping values live */
    float fu1 = ft1 * ft2 + ft3;
    float fu2 = ft4 - ft5 * ft6;
    float fu3 = ft1 / ft2 + ft3;
    float fu4 = ft4 * ft5 - ft6;
    
    /* Non-inlineable calls */
    fu1 += get_float1(fa);
    fu2 += get_float1(fb);
    
    /* Integer operations mixed in */
    fu3 += (float)get_value1(idx1);
    fu4 += (float)get_value2(idx2);
    
    /* Final result */
    float result = fu1 + fu2 * 2.0f - fu3 / 3.0f + fu4;
    
    return result;
}

/* Address calculation intensive test */
int test_addr_remat(void) {
    volatile int seed = global_seed;
    
    /* Large array to work with */
    static int array[1024];
    
    /* Initialize array with pattern */
    for (int i = 0; i < 1024; i++) {
        array[i] = i * 3 + seed;
    }
    
    /* Many index calculations */
    int i1 = seed % 100;
    int i2 = (seed * 3) % 100;
    int i3 = (seed + 7) % 100;
    int i4 = (seed * 5) % 100;
    int i5 = (seed - 11) % 100;
    int i6 = (seed ^ 0xAA) % 100;
    int i7 = (seed << 2) % 100;
    int i8 = (seed >> 1) % 100;
    
    /* Complex address calculations - good remat candidates */
    int *p1 = &array[i1 * 3 + 1];
    int *p2 = &array[i2 * 7 - 2];
    int *p3 = &array[i3 * 5 + 3];
    int *p4 = &array[i4 * 11 - 4];
    int *p5 = &array[i5 * 13 + 5];
    int *p6 = &array[i6 * 17 - 6];
    int *p7 = &array[i7 * 19 + 7];
    int *p8 = &array[i8 * 23 - 8];
    
    /* More complex indices */
    int idx1 = (i1 + i2 * 2) & 0xFF;
    int idx2 = (i3 ^ i4) | 0x7F;
    int idx3 = (i5 * i6) % 256;
    int idx4 = (i7 + i8 * 3) & 0xFF;
    
    int *q1 = &array[idx1];
    int *q2 = &array[idx2];
    int *q3 = &array[idx3];
    int *q4 = &array[idx4];
    
    /* Access through pointers creating address computations */
    int v1 = *p1 + *p2;
    int v2 = *p3 - *p4;
    int v3 = *p5 * *p6;
    int v4 = *p7 / (*p8 + 1);
    
    int v5 = *q1 ^ *q2;
    int v6 = *q3 | *q4;
    int v7 = *p1 & *q1;
    int v8 = *p2 + *q2;
    
    /* Inline assembly with memory clobber to force recomputation */
    asm volatile(
        "# Force memory operations\n"
        : 
        : 
        : "memory"
    );
    
    /* Complex final computation */
    int result = v1 * 3 + v2 / 2 - v3 + v4 ^ v5 | v6 & v7 + v8;
    
    /* More pointer arithmetic */
    for (int j = 0; j < 10; j++) {
        int offset = (j * seed) % 256;
        result += array[offset];
        result -= array[offset + 1];
        result ^= array[offset + 2];
    }
    
    return result;
}

/* Mixed type test with loops */
double test_mixed_remat(void) {
    volatile double dseed = (double)global_seed / 100.0;
    
    double total = 0.0;
    
    /* Loop with many live variables */
    for (int iter = 0; iter < 50; iter++) {
        /* Many variables declared inside loop to increase pressure */
        double d1 = dseed + iter * 0.1;
        double d2 = dseed * iter * 0.2;
        double d3 = dseed / (iter + 1);
        double d4 = dseed - iter * 0.3;
        
        float f1 = (float)d1 * 1.1f;
        float f2 = (float)d2 * 2.2f;
        float f3 = (float)d3 * 3.3f;
        float f4 = (float)d4 * 4.4f;
        
        int i1 = (int)(d1 * 100);
        int i2 = (int)(d2 * 200);
        int i3 = (int)(d3 * 300);
        int i4 = (int)(d4 * 400);
        
        /* Complex computations */
        double mix1 = d1 * f1 + i1;
        double mix2 = d2 / f2 - i2;
        double mix3 = d3 + f3 * i3;
        double mix4 = d4 - f4 / i4;
        
        /* Non-inlineable calls */
        mix1 += get_double1(d1);
        mix2 += get_double1(d2);
        
        /* Update total */
        total += mix1 + mix2 * 0.5 - mix3 / 2.0 + mix4;
        
        /* Inline assembly to prevent optimization */
        if (iter % 7 == 0) {
            asm volatile(
                "# Periodic clobber\n"
                : 
                : 
                : "r0", "r1", "r2", "r3"
            );
        }
    }
    
    return total;
}

/* Main test driver */
int main(void) {
    int checksum = 0;
    
    printf("Starting early rematerialization test...\n");
    
    /* Run integer test multiple times with different seeds */
    for (int i = 0; i < 5; i++) {
        global_seed = 12345 + i * 1000;
        int int_result = test_int_remat();
        results[result_index++] = int_result;
        checksum += int_result;
        printf("Integer test %d: %d\n", i, int_result);
    }
    
    /* Run floating-point test */
    global_seed = 54321;
    float fp_result = test_fp_remat();
    results[result_index++] = (int)fp_result;
    checksum += (int)fp_result;
    printf("FP test: %f\n", fp_result);
    
    /* Run address calculation test */
    for (int i = 0; i < 3; i++) {
        global_seed = 9999 + i * 777;
        int addr_result = test_addr_remat();
        results[result_index++] = addr_result;
        checksum += addr_result;
        printf("Address test %d: %d\n", i, addr_result);
    }
    
    /* Run mixed type test */
    global_seed = 24680;
    double mixed_result = test_mixed_remat();
    results[result_index++] = (int)mixed_result;
    checksum += (int)mixed_result;
    printf("Mixed test: %f\n", mixed_result);
    
    /* Additional stress test with unrolled loops */
    printf("Running stress test...\n");
    for (int stress = 0; stress < 10; stress++) {
        global_seed = 1000 + stress * 123;
        
        /* Unrolled computation to create many live values */
        int s1 = global_seed * 1;
        int s2 = global_seed * 2;
        int s3 = global_seed * 3;
        int s4 = global_seed * 4;
        int s5 = global_seed * 5;
        int s6 = global_seed * 6;
        int s7 = global_seed * 7;
        int s8 = global_seed * 8;
        int s9 = global_seed * 9;
        int s10 = global_seed * 10;
        
        /* Chain of dependent operations */
        int r1 = s1 + s2;
        int r2 = s3 - s4;
        int r3 = s5 * s6;
        int r4 = s7 / (s8 + 1);
        int r5 = s9 ^ s10;
        int r6 = r1 + r2;
        int r7 = r3 - r4;
        int r8 = r5 * r6;
        int r9 = r7 + r8;
        int r10 = r9 ^ s1;
        
        /* Force many values to be live simultaneously */
        int final_result = r1 + r2 * 2 - r3 / 3 + r4 ^ r5 | r6 & r7 + r8 - r9 * r10;
        
        results[result_index++] = final_result;
        checksum += final_result;
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return checksum != 0 ? 0 : 1;
}
