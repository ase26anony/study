/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to force register pressure */
extern int ext_func1(int);
extern double ext_func2(double);
extern long long ext_func3(long long);

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High register pressure with mixed integer operations */
int test_mixed_integer_pressure(int seed) {
    volatile int use_result = 0;
    int a = seed * 3 + 1;
    int b = seed / 2 - 5;
    int c = seed + 12345;
    int d = seed ^ 0xABCDEF;
    
    /* Force values into registers */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
    
    /* Long sequence of independent computations */
    int t1 = a * b + c;
    int t2 = (a & b) | (c ^ d);
    int t3 = t1 - t2 * 3;
    int t4 = (t1 >> 4) + (t2 << 2);
    int t5 = t3 * t4 - 123;
    int t6 = t5 & 0xFF00FF;
    int t7 = t6 | (t4 << 16);
    int t8 = t7 ^ t3;
    int t9 = t8 * 7 + 11;
    int t10 = t9 - t6;
    
    /* Function call to clobber registers */
    int t11 = ext_func1(t10);
    
    /* More computations after call */
    int t12 = t11 * a + b;
    int t13 = (t12 & c) | d;
    int t14 = t13 - t11;
    int t15 = t14 * 3 + 7;
    int t16 = t15 ^ t12;
    int t17 = t16 | t13;
    int t18 = t17 * 2 - 1;
    int t19 = t18 + t14;
    int t20 = t19 & 0x7FFFFFFF;
    
    /* Another function call */
    int t21 = ext_func1(t20);
    
    /* Final chain */
    int t22 = t21 * 16807 % 2147483647;
    int t23 = t22 ^ t19;
    int t24 = t23 + t18;
    int t25 = (t24 << 3) | (t24 >> 29);
    
    asm volatile("" : : "r"(t25));
    use_result = t25;
    
    return use_result;
}

/* Test 2: Floating-point and mixed-mode operations */
double test_floating_pressure(double seed) {
    volatile double use_result = 0.0;
    double f1 = seed * 1.2345;
    double f2 = seed + 6789.0123;
    double f3 = seed / 3.14159;
    double f4 = seed - 9876.5432;
    
    asm volatile("" : : "r"(f1), "r"(f2), "r"(f3), "r"(f4));
    
    double ft1 = f1 * f2 + f3;
    double ft2 = f4 - f1 / f2;
    double ft3 = ft1 * ft2 * 2.71828;
    double ft4 = ft3 / (ft2 + 1.0);
    
    /* Function call with double */
    double ft5 = ext_func2(ft4);
    
    /* Mixed integer/double */
    int i1 = (int)ft5;
    double ft6 = ft5 * i1 + f3;
    double ft7 = ft6 - (double)i1;
    
    /* More FP operations */
    double ft8 = ft7 * ft7;
    double ft9 = ft8 / (ft7 + 1e-10);
    double ft10 = ft9 + ft6;
    double ft11 = ft10 * 0.5;
    double ft12 = ft11 - ft8;
    
    /* Another call */
    double ft13 = ext_func2(ft12);
    
    double ft14 = ft13 * 3.141592653589793;
    double ft15 = ft14 + ft11;
    double ft16 = ft15 / (ft13 + 1.0);
    
    asm volatile("" : : "r"(ft16));
    use_result = ft16;
    
    return use_result;
}

/* Test 3: Nested loops with complex induction */
long long test_nested_loops(int N) {
    volatile long long use_result = 0;
    long long sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable */
        for (int j = i * 2; j < N; j += 3) {
            /* Inner loop computations with register pressure */
            long long a = i * 1000LL + j;
            long long b = j * 123456789LL;
            long long c = a ^ b;
            long long d = (a << 3) | (b >> 5);
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            long long t1 = a * b - c;
            long long t2 = d + (a >> 2);
            long long t3 = t1 ^ t2;
            long long t4 = t3 * 6364136223846793005LL;
            
            /* Function call in inner loop */
            long long t5 = ext_func3(t4);
            
            long long t6 = t5 + j * 7LL;
            long long t7 = t6 - i * 11LL;
            long long t8 = t7 * t4;
            long long t9 = t8 | t5;
            
            sum += t9;
            
            /* More pressure */
            long long t10 = sum * 127LL;
            long long t11 = t10 ^ t9;
            long long t12 = t11 + t8;
            
            asm volatile("" : : "r"(t12));
        }
        
        /* Outer loop computations */
        long long outer1 = i * 987654321LL;
        long long outer2 = outer1 ^ sum;
        long long outer3 = outer2 * 1140671485LL;
        
        asm volatile("" : : "r"(outer3));
        
        sum = (sum + outer3) & 0x7FFFFFFFFFFFFFFFLL;
    }
    
    use_result = sum;
    return use_result;
}

/* Test 4: Vector operations for different modes */
v4si test_vector_ops(int seed) {
    volatile v4si use_result;
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    v4si v3 = {seed ^ 0xFF, seed ^ 0xAA, seed ^ 0x55, seed ^ 0x33};
    
    /* Vector operations */
    v4si vt1 = v1 + v2;
    v4si vt2 = v1 * v3;
    v4si vt3 = vt1 & v2;
    v4si vt4 = vt2 | v3;
    
    /* Scalar operations mixed in */
    int s1 = seed * 7;
    int s2 = seed + 100;
    int s3 = s1 ^ s2;
    
    asm volatile("" : : "r"(vt1), "r"(vt2), "r"(vt3), "r"(vt4), "r"(s3));
    
    v4si vt5 = vt3 + vt4;
    v4si vt6 = vt5 * v1;
    
    /* Function call */
    int s4 = ext_func1(s3);
    
    v4si vt7 = vt6 + s4;
    v4si vt8 = vt7 & 0x7F7F7F7F;
    
    /* More vector ops */
    v4si vt9 = vt8 * 3;
    v4si vt10 = vt9 | vt5;
    v4si vt11 = vt10 ^ vt7;
    
    asm volatile("" : : "r"(vt11));
    use_result = vt11;
    
    return use_result;
}

/* Test 5: Extreme register pressure with all types */
long long test_extreme_pressure(int seed) {
    volatile long long use_result = 0;
    
    /* Integer computations */
    int i1 = seed * 3;
    int i2 = seed + 456;
    int i3 = i1 ^ i2;
    int i4 = i3 * 7 - 123;
    
    /* Floating computations */
    double f1 = (double)seed * 1.234;
    double f2 = (double)i2 / 3.14159;
    double f3 = f1 + f2;
    double f4 = f3 * 2.71828;
    
    /* Long long computations */
    long long ll1 = (long long)seed * 1234567890123LL;
    long long ll2 = ll1 ^ 0xF0F0F0F0F0F0F0F0LL;
    long long ll3 = ll2 + i4;
    
    /* Force all into registers */
    asm volatile("" : : "r"(i1), "r"(i2), "r"(i3), "r"(i4),
                       "r"(f1), "r"(f2), "r"(f3), "r"(f4),
                       "r"(ll1), "r"(ll2), "r"(ll3));
    
    /* Chain of dependent computations */
    int i5 = i4 * i3 + i2;
    double f5 = f4 / f3 + f2;
    long long ll4 = ll3 * 127LL - ll2;
    
    /* Function calls to clobber registers */
    int i6 = ext_func1(i5);
    double f6 = ext_func2(f5);
    long long ll5 = ext_func3(ll4);
    
    /* More mixing */
    double f7 = f6 * (double)i6;
    long long ll6 = ll5 + (long long)f7;
    int i7 = i6 ^ (int)ll6;
    
    /* Final computations */
    double f8 = f7 + (double)i7;
    long long ll7 = ll6 * 16807LL;
    int i8 = i7 * 3 + 5;
    
    /* Combine results */
    long long result = ll7 + (long long)f8 + i8;
    
    asm volatile("" : : "r"(result));
    use_result = result;
    
    return use_result;
}

/* Dummy external functions */
int ext_func1(int x) { return x ^ 0x5A5A5A5A; }
double ext_func2(double x) { return x * 1.41421356237; }
long long ext_func3(long long x) { return x * 6364136223846793005LL; }

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int N = argc > 2 ? atoi(argv[2]) : 100;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests */
    int r1 = test_mixed_integer_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_floating_pressure((double)seed);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_nested_loops(N);
    printf("Test 3 result: %lld\n", r3);
    
    v4si r4 = test_vector_ops(seed);
    printf("Test 4 result: {%d, %d, %d, %d}\n", r4[0], r4[1], r4[2], r4[3]);
    
    long long r5 = test_extreme_pressure(seed);
    printf("Test 5 result: %lld\n", r5);
    
    /* Use results to prevent optimization */
    volatile int final = r1 + (int)r2 + (int)r3 + r4[0] + (int)r5;
    printf("Final checksum: %d\n", final);
    
    return 0;
}
