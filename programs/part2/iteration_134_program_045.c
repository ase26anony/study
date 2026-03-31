/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to force register pressure */
extern int ext_func1(int);
extern int ext_func2(int, int);
extern double ext_func3(double);
extern long long ext_func4(long long);

/* Dummy external functions */
int ext_func1(int x) { return x ^ 0x55AA55AA; }
int ext_func2(int x, int y) { return x * y + (x ^ y); }
double ext_func3(double x) { return x * 1.5 - 0.25; }
long long ext_func4(long long x) { return x * 3 + 7; }

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int test_high_int_pressure(int seed) {
    volatile int result = 0;
    int a = seed * 3 + 1;
    int b = seed / 2 - 5;
    int c = seed + 0x123456;
    int d = seed ^ 0x789ABC;
    
    /* Long sequence of independent computations */
    int t1 = a * b + c - d;
    asm volatile("" : : "r"(t1)); /* Pin in register */
    int t2 = (a & b) | (c ^ d);
    int t3 = t1 * t2 - a + b;
    asm volatile("" : : "r"(t3));
    
    /* Function call creates pressure point */
    int t4 = ext_func1(t2);
    int t5 = t3 + t4 * 2;
    int t6 = (t5 << 3) | (t4 >> 2);
    
    /* More computations */
    int t7 = t6 * 7 + 11;
    int t8 = t7 ^ t5 & t6;
    asm volatile("" : : "r"(t8));
    
    /* Another function call */
    int t9 = ext_func2(t7, t8);
    int t10 = t9 - t6 + t5 * 3;
    
    /* Nested loops with complex induction */
    for (int i = seed; i < seed + 10; i++) {
        for (int j = i * 2; j < i * 2 + 5; j += 3) {
            int inner1 = j * i + t10;
            int inner2 = (inner1 ^ j) & i;
            asm volatile("" : : "r"(inner2));
            t10 += inner2;
            
            /* More pressure inside loops */
            int inner3 = ext_func1(inner1);
            int inner4 = inner3 * j - i;
            t10 ^= inner4;
        }
    }
    
    result = t10;
    return result;
}

/* Test 2: Mixed integer and floating-point operations */
double test_mixed_types(int seed) {
    volatile double result = 0.0;
    int i1 = seed * 2;
    int i2 = seed + 1000;
    double f1 = seed * 1.5;
    double f2 = seed / 3.0;
    
    /* Integer computations */
    int t1 = i1 * i2 + seed;
    int t2 = (i1 ^ i2) & seed;
    asm volatile("" : : "r"(t1), "r"(t2));
    
    /* Floating computations */
    double f3 = f1 * f2 - 2.5;
    double f4 = f1 / f2 + 1.25;
    asm volatile("" : : "f"(f3), "f"(f4));
    
    /* Function calls with mixed types */
    int t3 = ext_func1(t1);
    double f5 = ext_func3(f3);
    
    /* More mixed operations */
    double f6 = f5 * t3 + f4;
    int t4 = (int)f6 * t2 - t1;
    
    /* Nested loops with floating induction */
    for (int i = 0; i < 8; i++) {
        double loop_f = f6 + i * 0.5;
        for (int j = i * 3; j < i * 3 + 4; j++) {
            int loop_i = t4 + j * 2;
            double temp_f = loop_f * j + loop_i;
            asm volatile("" : : "r"(loop_i), "f"(temp_f));
            
            /* Function call inside loop */
            loop_f = ext_func3(temp_f);
            loop_i = ext_func2(loop_i, j);
            
            f6 += loop_f;
            t4 ^= loop_i;
        }
    }
    
    result = f6 + t4;
    return result;
}

/* Test 3: Long long and vector operations */
long long test_wide_types(int seed) {
    volatile long long result = 0;
    long long ll1 = (long long)seed * 1000000LL;
    long long ll2 = (long long)seed * 3000000LL;
    
    /* Vector operations */
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    
    /* Long sequence of long long operations */
    long long t1 = ll1 * ll2 + seed;
    long long t2 = ll1 ^ ll2 & seed;
    asm volatile("" : : "r"(t1), "r"(t2));
    
    /* Vector computations */
    v4si v3 = v1 + v2;
    v4si v4 = v1 * v2;
    asm volatile("" : : "x"(v3), "x"(v4));
    
    /* Function calls */
    long long t3 = ext_func4(t1);
    v4si v5 = v3 + v4;
    
    /* More computations */
    long long t4 = t3 * 5 - t2;
    v4si v6 = v5 * v1 - v2;
    
    /* Complex nested loops */
    for (long long i = 0; i < 6; i++) {
        for (long long j = i * 4; j < i * 4 + 3; j += 2) {
            long long inner_ll = t4 * j + i;
            v4si inner_v = v6 + (v4si){j, j+1, j+2, j+3};
            asm volatile("" : : "r"(inner_ll), "x"(inner_v));
            
            /* Multiple operations */
            inner_ll = ext_func4(inner_ll);
            inner_v = inner_v * v5 + v6;
            
            t4 += inner_ll;
            v6 = v6 + inner_v;
        }
    }
    
    /* Extract result from vector */
    long long vec_sum = v6[0] + v6[1] + v6[2] + v6[3];
    result = t4 + vec_sum;
    return result;
}

/* Test 4: Extreme register pressure with many temporaries */
int test_extreme_pressure(int seed) {
    volatile int result = 0;
    
    /* Create many independent variables */
    int v[20];
    for (int i = 0; i < 20; i++) {
        v[i] = seed * (i + 1) + i * 3;
    }
    
    /* Long chain of computations without reuse */
    int r1 = v[0] * v[1] + v[2];
    int r2 = v[3] ^ v[4] | v[5];
    int r3 = r1 + r2 * v[6];
    int r4 = (v[7] & v[8]) + (v[9] ^ v[10]);
    int r5 = r3 - r4 * 2;
    int r6 = v[11] * 3 + v[12] / 2;
    int r7 = (r5 << 2) | (r6 >> 1);
    int r8 = ext_func1(r7);
    int r9 = r8 * v[13] + v[14];
    int r10 = (r9 ^ v[15]) & v[16];
    
    /* Pin many values to increase pressure */
    asm volatile("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5));
    asm volatile("" : : "r"(r6), "r"(r7), "r"(r8), "r"(r9), "r"(r10));
    
    /* More computations with function calls */
    int r11 = ext_func2(r10, v[17]);
    int r12 = r11 * 7 - v[18] + v[19];
    int r13 = (r12 ^ r11) | r10;
    
    /* Deeply nested loops */
    for (int i = 0; i < 5; i++) {
        int outer = r13 + i * 100;
        for (int j = i * 3; j < i * 3 + 4; j++) {
            int middle = outer * j + r12;
            for (int k = j; k < j + 2; k++) {
                int inner = middle * k - r11;
                asm volatile("" : : "r"(inner));
                
                /* Function call in innermost loop */
                inner = ext_func1(inner);
                r13 += inner * k;
            }
            r12 ^= middle;
        }
        r11 += outer;
    }
    
    result = r11 + r12 + r13;
    return result;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    int sum = 0;
    
    printf("Testing early rematerialization with seed: %d\n", seed);
    
    /* Run all tests to maximize coverage */
    sum += test_high_int_pressure(seed);
    sum += (int)test_mixed_types(seed);
    sum += (int)test_wide_types(seed);
    sum += test_extreme_pressure(seed);
    
    /* Use result to prevent optimization */
    volatile int final_result = sum;
    printf("Final checksum: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
