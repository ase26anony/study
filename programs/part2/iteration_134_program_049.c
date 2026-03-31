/* test_early_remat.c - Comprehensive test for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to force register pressure */
extern int external_func1(int);
extern int external_func2(int, int);
extern double external_func3(double);
extern long long external_func4(long long);

/* Dummy external functions */
int external_func1(int x) { return x ^ 0x55AA55AA; }
int external_func2(int x, int y) { return x * y + (x ^ y); }
double external_func3(double x) { return x * 1.5 - 0.25; }
long long external_func4(long long x) { return x * 3LL + 1LL; }

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int test_integer_pressure(int seed) {
    volatile int use_result = 0;
    int a = seed * 3 + 1;
    int b = seed / 2 - 5;
    int c = seed ^ 0x12345678;
    int d = seed + 0xABCDEF;
    
    /* Force values into registers */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
    
    /* Long sequence of independent computations */
    int t1 = a * b + c - d;
    int t2 = (a & b) | (c ^ d);
    int t3 = t1 * t2 + seed;
    int t4 = t2 - t1 * 3;
    
    /* Use inline asm to prevent optimization */
    asm volatile("" : "+r"(t3), "+r"(t4));
    
    /* Function call creates register pressure point */
    int t5 = external_func1(t3);
    int t6 = external_func2(t4, t3);
    
    /* More computations */
    int t7 = t5 * t6 + t3 / 2;
    int t8 = (t5 ^ t6) & (t7 | t4);
    int t9 = t7 * 2 - t8 / 3;
    int t10 = t8 + t9 * 5;
    
    asm volatile("" : : "r"(t7), "r"(t8), "r"(t9), "r"(t10));
    
    /* Another function call */
    int t11 = external_func2(t9, t10);
    int t12 = external_func1(t11);
    
    /* Final complex expression */
    int result = (t1 + t2) * (t3 - t4) + (t5 * t6) / (t7 + 1) 
                 - (t8 ^ t9) | (t10 & t11) + t12;
    
    /* Prevent dead code elimination */
    use_result = result;
    asm volatile("" : : "r"(use_result));
    
    return result;
}

/* Test 2: Mixed integer and floating-point pressure */
double test_mixed_pressure(int seed) {
    volatile double use_result = 0.0;
    
    /* Integer computations */
    int i1 = seed * 2;
    int i2 = seed + 1000;
    int i3 = seed ^ 0xF0F0F0F0;
    
    /* Floating-point computations */
    double f1 = seed * 1.2345;
    double f2 = seed / 3.14159;
    double f3 = f1 * f2 - 2.71828;
    
    asm volatile("" : : "r"(i1), "r"(i2), "r"(i3));
    asm volatile("" : : "f"(f1), "f"(f2), "f"(f3));
    
    /* Mixed operations */
    double f4 = f1 * i1 + f2 * i2;
    int i4 = (int)(f3 * 1000) + i3;
    
    /* Function calls with different types */
    double f5 = external_func3(f4);
    int i5 = external_func1(i4);
    
    /* More mixed computations */
    double f6 = f5 * i5 / 256.0;
    int i6 = (int)f6 * 3 + i5;
    double f7 = external_func3(f6);
    int i7 = external_func2(i6, (int)f7);
    
    /* Complex final expression */
    double result = (f1 + f2) * (f3 - f4) + (f5 * f6) / (f7 + 1.0)
                    + (i1 + i2) * (i3 - i4) / 1000.0
                    + (i5 * i6) / (double)(i7 + 1);
    
    use_result = result;
    asm volatile("" : : "f"(use_result));
    
    return result;
}

/* Test 3: Nested loops with complex induction variables */
long long test_nested_loops(int N) {
    volatile long long use_result = 0;
    long long sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable for inner loop */
        for (int j = i * 2; j < N; j += 3) {
            /* Multiple independent computations in loop body */
            int a = i * j + 1;
            int b = (i ^ j) * 3;
            int c = a * b - j;
            int d = (a & b) | (c ^ i);
            
            /* Force values into registers */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            /* Function call increases pressure */
            int e = external_func1(c);
            int f = external_func2(d, e);
            
            /* More computations */
            int g = a * e + b * f;
            int h = (c ^ d) & (e | f);
            
            asm volatile("" : : "r"(g), "r"(h));
            
            /* External call with long long */
            long long k = external_func4((long long)g * h);
            
            sum += k + i + j;
            
            /* Additional pressure point */
            if ((i + j) % 7 == 0) {
                int temp = external_func1(i * j);
                sum += temp;
            }
        }
        
        /* Break up loop with function call */
        if (i % 5 == 0) {
            sum += external_func4(sum);
        }
    }
    
    use_result = sum;
    asm volatile("" : : "r"(use_result));
    
    return sum;
}

/* Test 4: Vector operations for different modes */
int test_vector_operations(int seed) {
    volatile int use_result = 0;
    
    /* Create vector values */
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    v4si v3 = {seed ^ 1, seed ^ 2, seed ^ 3, seed ^ 4};
    
    /* Scalar computations to mix with vectors */
    int s1 = seed * 7 + 11;
    int s2 = seed / 3 - 5;
    int s3 = s1 * s2 + seed;
    
    asm volatile("" : : "r"(s1), "r"(s2), "r"(s3));
    
    /* Vector operations */
    v4si v4 = v1 + v2;
    v4si v5 = v2 * v3;
    v4si v6 = v4 & v5;
    v4si v7 = v5 | v6;
    
    /* Mix vectors and scalars */
    int s4 = s3 + ((int*)&v4)[0];
    int s5 = s2 * ((int*)&v5)[1];
    int s6 = s1 ^ ((int*)&v6)[2];
    
    /* Function calls */
    int s7 = external_func1(s4);
    int s8 = external_func2(s5, s6);
    
    /* More vector operations */
    v4si v8 = v7 * s7;
    v4si v9 = v6 + s8;
    
    /* Extract results */
    int result = ((int*)&v8)[0] + ((int*)&v9)[1] + s7 * s8;
    
    /* Additional pressure with double */
    double d1 = seed * 1.5;
    double d2 = external_func3(d1);
    result += (int)(d2 * 100);
    
    use_result = result;
    asm volatile("" : : "r"(use_result));
    
    return result;
}

/* Test 5: Extreme register pressure with many live values */
long long test_extreme_pressure(int iterations) {
    volatile long long use_result = 0;
    long long acc = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Create many independent live values */
        long long v1 = i * 3LL + 1LL;
        long long v2 = i * 5LL - 2LL;
        long long v3 = v1 * v2 + i;
        long long v4 = v2 ^ v1 * 7LL;
        long long v5 = external_func4(v3);
        long long v6 = v4 * v5 / 3LL;
        
        double d1 = i * 1.234;
        double d2 = d1 * 2.718;
        double d3 = external_func3(d2);
        double d4 = d1 + d3 * 0.5;
        
        int i1 = i * 11;
        int i2 = i1 ^ 0xAA55AA55;
        int i3 = external_func1(i2);
        int i4 = i3 * 3 + i1;
        
        /* Force all values to be live simultaneously */
        asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), 
                     "r"(v5), "r"(v6), "r"(i1), "r"(i2), "r"(i3), "r"(i4));
        asm volatile("" : : "f"(d1), "f"(d2), "f"(d3), "f"(d4));
        
        /* Complex expression using all values */
        long long temp = (v1 + v2) * (v3 - v4) + v5 * v6 / 7LL
                         + (long long)(d1 * d2 - d3 + d4) * 1000LL
                         + (i1 * i2 + i3 - i4) * 3LL;
        
        acc += temp;
        
        /* Function call to create pressure point */
        if (i % 4 == 0) {
            acc = external_func4(acc);
        }
    }
    
    use_result = acc;
    asm volatile("" : : "r"(use_result));
    
    return acc;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int N = argc > 2 ? atoi(argv[2]) : 100;
    int iterations = argc > 3 ? atoi(argv[3]) : 50;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to exercise different patterns */
    int r1 = test_integer_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_mixed_pressure(seed);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_nested_loops(N);
    printf("Test 3 result: %lld\n", r3);
    
    int r4 = test_vector_operations(seed);
    printf("Test 4 result: %d\n", r4);
    
    long long r5 = test_extreme_pressure(iterations);
    printf("Test 5 result: %lld\n", r5);
    
    /* Final checksum to prevent optimization */
    volatile long long final = r1 + (long long)r2 + r3 + r4 + r5;
    printf("Final checksum: %lld\n", final);
    
    return 0;
}
