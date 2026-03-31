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

/* Global volatile to prevent optimization */
volatile int global_seed = 42;

/* Test 1: High integer register pressure with complex expressions */
int test_integer_pressure(int n) {
    int a = n + 1;
    int b = n * 2;
    int c = n / 3;
    int d = n - 4;
    
    /* Force values into registers */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
    
    /* Long sequence of independent computations */
    int t1 = a * b + c;
    int t2 = b & c | d;
    int t3 = a ^ b ^ c ^ d;
    int t4 = (a << 3) | (b >> 2);
    int t5 = t1 * t2 - t3;
    int t6 = t4 & t5 | t1;
    int t7 = t2 ^ t3 ^ t4;
    int t8 = t5 * t6 + t7;
    int t9 = t6 & t7 | t8;
    int t10 = t8 ^ t9 ^ t1;
    
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4), "r"(t5));
    
    /* Function call that clobbers registers */
    int r1 = ext_func1(t1);
    
    /* More computations after call */
    int t11 = t10 * r1 + t2;
    int t12 = t9 & r1 | t3;
    int t13 = t8 ^ r1 ^ t4;
    int t14 = t7 * r1 - t5;
    int t15 = t6 & r1 | t11;
    
    asm volatile("" : : "r"(t6), "r"(t7), "r"(t8), "r"(t9), "r"(t10));
    
    /* Another function call */
    int r2 = ext_func1(t6);
    
    /* Final computations */
    int t16 = t15 * r2 + t12;
    int t17 = t14 & r2 | t13;
    int t18 = t11 ^ r2 ^ t15;
    
    /* Use volatile to prevent elimination */
    asm volatile("" : : "r"(t16), "r"(t17), "r"(t18));
    
    return t16 + t17 + t18;
}

/* Test 2: Mixed integer and floating-point pressure */
double test_mixed_pressure(double base) {
    double d1 = base * 1.1;
    double d2 = base / 2.2;
    double d3 = base + 3.3;
    double d4 = base - 4.4;
    
    int i1 = (int)base * 5;
    int i2 = (int)base / 6;
    int i3 = (int)base + 7;
    int i4 = (int)base - 8;
    
    asm volatile("" : : "r"(i1), "r"(i2), "r"(i3), "r"(i4));
    asm volatile("" : : "f"(d1), "f"(d2), "f"(d3), "f"(d4));
    
    /* Mixed computations */
    double dt1 = d1 * d2 + d3;
    double dt2 = d2 - d3 * d4;
    double dt3 = d1 / d2 + d4;
    
    int it1 = i1 * i2 + i3;
    int it2 = i2 & i3 | i4;
    int it3 = i1 ^ i2 ^ i3;
    
    /* Function call mixing types */
    double dr1 = ext_func2(dt1);
    int ir1 = ext_func1(it1);
    
    /* More mixed computations */
    double dt4 = dt1 * dr1 + (double)ir1;
    double dt5 = dt2 - dr1 * (double)it2;
    int it4 = it1 * ir1 + (int)dt3;
    int it5 = it2 & ir1 | (int)dt1;
    
    asm volatile("" : : "f"(dt4), "f"(dt5), "r"(it4), "r"(it5));
    
    /* Another call */
    double dr2 = ext_func2(dt4);
    
    /* Final mixed result */
    double result = dt5 * dr2 + (double)it4 - (double)it5;
    
    asm volatile("" : : "f"(result));
    return result;
}

/* Test 3: Nested loops with complex induction */
long long test_nested_loops(int N) {
    long long sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable */
        for (int j = i * 2; j < N; j += 3) {
            /* Inner loop computations with many temporaries */
            long long a = i * 100LL;
            long long b = j * 200LL;
            long long c = (i + j) * 300LL;
            long long d = (i - j) * 400LL;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            long long t1 = a * b + c;
            long long t2 = b & c | d;
            long long t3 = a ^ b ^ c;
            long long t4 = (a << 5) | (b >> 3);
            
            /* Function call in inner loop */
            long long r1 = ext_func3(t1);
            
            long long t5 = t2 * r1 + t3;
            long long t6 = t4 & r1 | t1;
            long long t7 = t3 ^ r1 ^ t2;
            
            sum += t5 + t6 + t7;
            
            /* Force register pressure */
            asm volatile("" : : "r"(t5), "r"(t6), "r"(t7));
        }
        
        /* Additional computation between outer loop iterations */
        int temp = ext_func1(i);
        sum += temp;
    }
    
    return sum;
}

/* Test 4: Vector operations for different modes */
v4si test_vector_ops(v4si v1, v4si v2) {
    v4si r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
    
    /* Many vector operations */
    r1 = v1 + v2;
    r2 = v1 - v2;
    r3 = v1 * v2;
    r4 = v1 & v2;
    r5 = v1 | v2;
    r6 = v1 ^ v2;
    r7 = r1 + r2;
    r8 = r3 - r4;
    r9 = r5 & r6;
    r10 = r7 | r8;
    
    /* Force vector values */
    asm volatile("" : : "x"(r1), "x"(r2), "x"(r3), "x"(r4), "x"(r5));
    asm volatile("" : : "x"(r6), "x"(r7), "x"(r8), "x"(r9), "x"(r10));
    
    /* Mix with scalar operations */
    int s1 = ((int*)&v1)[0];
    int s2 = ((int*)&v2)[0];
    int s3 = s1 * s2 + 42;
    int s4 = s1 & s2 | 123;
    
    /* Function call */
    int sr1 = ext_func1(s3);
    
    /* More mixed computations */
    v4si r11 = r9 + r10;
    v4si r12 = r11 * (v4si){sr1, sr1, sr1, sr1};
    
    asm volatile("" : : "x"(r11), "x"(r12), "r"(sr1));
    
    return r12;
}

/* Test 5: Extreme register pressure with all types */
double test_extreme_pressure(int iterations) {
    double d_acc = 0.0;
    long long ll_acc = 0;
    int i_acc = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Create many variables of different types */
        double d1 = iter * 1.234;
        double d2 = iter / 5.678;
        double d3 = d1 + d2;
        double d4 = d1 - d2;
        double d5 = d1 * d2;
        double d6 = d3 / d4;
        
        long long ll1 = iter * 1000LL;
        long long ll2 = iter * 2000LL;
        long long ll3 = ll1 + ll2;
        long long ll4 = ll1 - ll2;
        long long ll5 = ll1 * 100LL;
        long long ll6 = ll2 / 50LL;
        
        int i1 = iter * 3;
        int i2 = iter * 7;
        int i3 = i1 + i2;
        int i4 = i1 - i2;
        int i5 = i1 * 11;
        int i6 = i2 / 13;
        
        /* Force all into registers */
        asm volatile("" : : 
            "f"(d1), "f"(d2), "f"(d3), "f"(d4), "f"(d5), "f"(d6),
            "r"(ll1), "r"(ll2), "r"(ll3), "r"(ll4), "r"(ll5), "r"(ll6),
            "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5), "r"(i6));
        
        /* Complex computations mixing all types */
        double d7 = d5 * (double)ll3 + (double)i5;
        long long ll7 = ll5 * (long long)d6 + (long long)i6;
        int i7 = i3 * (int)d1 + (int)ll1;
        
        /* Function calls that clobber registers */
        double d8 = ext_func2(d7);
        long long ll8 = ext_func3(ll7);
        int i8 = ext_func1(i7);
        
        /* More computations */
        double d9 = d8 * 2.0 + (double)ll8 / 1000.0;
        long long ll9 = ll8 * 3 + (long long)d8;
        int i9 = i8 * 5 + (int)d8;
        
        /* Accumulate results */
        d_acc += d9;
        ll_acc += ll9;
        i_acc += i9;
        
        /* Prevent optimization */
        asm volatile("" : : "f"(d9), "r"(ll9), "r"(i9));
    }
    
    /* Final mixed result */
    double result = d_acc + (double)ll_acc + (double)i_acc;
    asm volatile("" : : "f"(result));
    return result;
}

/* Dummy external functions */
int ext_func1(int x) {
    return x ^ 0x55AA55AA;
}

double ext_func2(double x) {
    return x * 1.61803398875; /* golden ratio */
}

long long ext_func3(long long x) {
    return x * 6364136223846793005LL;
}

int main(int argc, char **argv) {
    int seed = argc > 1 ? atoi(argv[1]) : global_seed;
    srand(seed);
    
    int N = 50 + (rand() % 100);
    
    printf("Running early rematerialization tests...\n");
    
    /* Run all tests to maximize coverage */
    int r1 = test_integer_pressure(N);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_mixed_pressure((double)N);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_nested_loops(N / 10);
    printf("Test 3 result: %lld\n", r3);
    
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si r4 = test_vector_ops(v1, v2);
    printf("Test 4 result: {%d, %d, %d, %d}\n", 
           r4[0], r4[1], r4[2], r4[3]);
    
    double r5 = test_extreme_pressure(20 + (rand() % 30));
    printf("Test 5 result: %f\n", r5);
    
    /* Final checksum to prevent dead code elimination */
    volatile int checksum = r1 + (int)r2 + (int)r3 + r4[0] + (int)r5;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
