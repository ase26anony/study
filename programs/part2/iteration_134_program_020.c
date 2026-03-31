/* test_early_remat.c - Program to trigger GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to force register pressure */
extern int ext_func1(int);
extern int ext_func2(int, int);
extern double ext_func3(double);
extern long long ext_func4(long long);

/* Dummy external functions to prevent inlining */
int ext_func1(int x) { return x ^ 0x55AA55AA; }
int ext_func2(int x, int y) { return (x * y) ^ 0x12345678; }
double ext_func3(double x) { return x * 3.141592653589793; }
long long ext_func4(long long x) { return x * 0x1122334455667788LL; }

/* Vector type to engage different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int test_high_int_pressure(int seed) {
    volatile int result = 0;
    int a = seed * 3 + 1;
    int b = seed / 2 - 5;
    int c = seed ^ 0xDEADBEEF;
    int d = seed + 0x12345678;
    
    /* Force values into registers */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
    
    /* Long sequence of independent computations */
    int t1 = a * b + c - d;
    int t2 = (a & b) | (c ^ d);
    int t3 = t1 * t2 / (a + 1);
    int t4 = (t2 << 3) | (t3 >> 2);
    int t5 = t3 * t4 - t1;
    int t6 = t4 ^ t5 ^ t2;
    int t7 = t5 * 7 + t6 * 13;
    int t8 = (t6 & 0xFF) | (t7 << 8);
    int t9 = t7 / (t8 + 1) + t5;
    int t10 = t8 * t9 - t6;
    
    /* Function call to clobber caller-saved registers */
    int f1 = ext_func1(t10);
    
    /* More computations after call */
    int t11 = f1 * t9 + t8;
    int t12 = (t11 & t10) | (f1 ^ t9);
    int t13 = t11 * 3 - t12 * 5;
    int t14 = t12 + t13 * 7;
    int t15 = (t13 << 1) | (t14 >> 1);
    
    /* Another function call */
    int f2 = ext_func2(t15, t14);
    
    /* Final computations */
    int t16 = f2 * t15 + t14;
    int t17 = t16 ^ t15 ^ t14;
    int t18 = t17 * 19 - t16 / 3;
    
    /* Use volatile to prevent elimination */
    asm volatile("" : : "r"(t18));
    
    result = t18;
    return result;
}

/* Test 2: Mixed integer and floating-point pressure */
double test_mixed_pressure(int seed) {
    volatile double result = 0.0;
    double da = seed * 1.234567;
    double db = seed / 3.14159;
    float fa = seed * 0.987654f;
    float fb = seed + 2.71828f;
    
    /* Force FP values into registers */
    asm volatile("" : : "f"(da), "f"(db), "f"(fa), "f"(fb));
    
    /* Mixed computations */
    double dt1 = da * db + fa - fb;
    float ft1 = fa * fb - (float)da;
    double dt2 = dt1 * 2.5 - ft1 * 1.5;
    float ft2 = (float)dt2 * ft1 + 0.5f;
    
    /* Function call clobbering FP registers */
    double df1 = ext_func3(dt2);
    
    /* More mixed computations */
    double dt3 = df1 * ft2 + dt1;
    float ft3 = (float)dt3 * ft2 - 1.0f;
    double dt4 = dt3 / ft3 * 3.14159;
    
    /* Integer computations interleaved */
    int it1 = (int)dt4 * seed;
    int it2 = it1 ^ (int)ft3;
    int it3 = it2 * 7 - it1 / 3;
    
    /* Another call */
    double df2 = ext_func3(dt4);
    
    /* Final mixed computation */
    double dt5 = df2 * (double)it3 + dt4;
    float ft4 = (float)dt5 * ft3;
    double dt6 = dt5 - (double)ft4;
    
    asm volatile("" : : "f"(dt6));
    
    result = dt6;
    return result;
}

/* Test 3: Nested loops with complex induction */
long long test_nested_loops(int N) {
    volatile long long total = 0;
    
    for (int i = 0; i < N; i++) {
        /* Non-trivial induction variable */
        for (int j = i * 2; j < N; j += 3) {
            /* Complex loop body with register pressure */
            int a = i * j + 1;
            int b = (i ^ j) * 3;
            int c = a * b - j;
            int d = (c << 2) | (a >> 1);
            
            /* Force values */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            /* Independent computations */
            int t1 = a * 7 + b * 13;
            int t2 = (c & 0xFF) | (d << 8);
            int t3 = t1 * t2 - a;
            int t4 = t2 ^ t3 ^ b;
            
            /* Function call in inner loop */
            int f1 = ext_func1(t4);
            
            /* More computations */
            int t5 = f1 * t3 + t2;
            int t6 = (t5 & t4) | (f1 ^ t3);
            
            total += t5 + t6;
            
            /* Additional pressure with long long */
            long long lt1 = (long long)t5 * t6;
            long long lt2 = lt1 ^ 0x123456789ABCDEFLL;
            long long lt3 = ext_func4(lt2);
            
            total += lt3;
        }
        
        /* Outer loop computations */
        int outer1 = i * 3 - 1;
        int outer2 = (outer1 << 1) | (i >> 1);
        int outer3 = ext_func2(outer1, outer2);
        
        total += outer3;
    }
    
    asm volatile("" : : "r"(total));
    return total;
}

/* Test 4: Vector operations for vector modes */
int test_vector_ops(int seed) {
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    v4si v3 = {seed ^ 1, seed ^ 2, seed ^ 3, seed ^ 4};
    
    /* Vector computations */
    v4si r1 = v1 + v2;
    v4si r2 = v1 * v3;
    v4si r3 = r1 & v2;
    v4si r4 = r2 | v3;
    
    /* Scalarize to increase pressure */
    int s1 = r1[0] + r1[1] + r1[2] + r1[3];
    int s2 = r2[0] * r2[1] - r2[2] * r2[3];
    int s3 = r3[0] ^ r3[1] ^ r3[2] ^ r3[3];
    int s4 = r4[0] | r4[1] | r4[2] | r4[3];
    
    /* Force vector and scalar values */
    asm volatile("" : : "x"(r1), "x"(r2), "r"(s1), "r"(s2), "r"(s3), "r"(s4));
    
    /* More mixed computations */
    int t1 = s1 * s2 + s3 - s4;
    int t2 = (s1 & s2) | (s3 ^ s4);
    int t3 = t1 * 7 + t2 * 13;
    int t4 = ext_func1(t3);
    int t5 = t4 * t2 - t1;
    
    asm volatile("" : : "r"(t5));
    
    return t5;
}

/* Test 5: Extreme register pressure with all types */
long long test_extreme_pressure(int seed) {
    volatile long long checksum = 0;
    
    /* Integer computations */
    int i1 = seed * 3;
    int i2 = seed / 2;
    int i3 = seed ^ 0x1234;
    int i4 = seed + 0x5678;
    
    /* Floating computations */
    double d1 = seed * 1.234;
    double d2 = seed / 5.678;
    float f1 = seed * 0.901f;
    float f2 = seed + 2.345f;
    
    /* Long long computations */
    long long ll1 = seed * 0x11223344LL;
    long long ll2 = seed ^ 0xAABBCCDDLL;
    
    /* Force all into registers */
    asm volatile("" : : "r"(i1), "r"(i2), "r"(i3), "r"(i4),
                       "f"(d1), "f"(d2), "f"(f1), "f"(f2),
                       "r"(ll1), "r"(ll2));
    
    /* Massive computation sequence */
    for (int k = 0; k < 50; k++) {
        i1 = i1 * 3 + k;
        i2 = i2 / 2 - k;
        i3 = i3 ^ i1;
        i4 = i4 | i2;
        
        d1 = d1 * 1.1 + k;
        d2 = d2 / 1.2 - k;
        f1 = f1 * 1.3f + k;
        f2 = f2 - 0.7f * k;
        
        ll1 = ll1 * 3 + i1;
        ll2 = ll2 ^ ll1;
        
        /* Periodic function calls */
        if (k % 7 == 0) {
            i1 = ext_func1(i1);
            d1 = ext_func3(d1);
            ll1 = ext_func4(ll1);
        }
        
        /* Use volatile to prevent elimination */
        asm volatile("" : : "r"(i1), "r"(i2), "f"(d1), "f"(f1), "r"(ll1));
        
        checksum += i1 + i2 + (long long)d1 + (long long)f1 + ll1;
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    long long total = 0;
    
    srand(seed);
    
    /* Run all tests to maximize coverage opportunities */
    total += test_high_int_pressure(seed);
    total += (long long)test_mixed_pressure(seed);
    total += test_nested_loops(seed % 100 + 10);
    total += test_vector_ops(seed);
    total += test_extreme_pressure(seed);
    
    /* Prevent dead code elimination */
    volatile long long final_result = total;
    printf("Result: %lld\n", final_result);
    
    return (final_result != 0) ? 0 : 1;
}
