/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to create register pressure points */
extern int external_func1(int);
extern int external_func2(int, int);
extern double external_func3(double);
extern long long external_func4(long long);

/* Dummy implementations to satisfy linker */
int external_func1(int x) { return x ^ 0x55AA55AA; }
int external_func2(int x, int y) { return (x * y) ^ 0x12345678; }
double external_func3(double x) { return x * 1.234567; }
long long external_func4(long long x) { return x * 0x987654321LL; }

/* Vector type to engage different machine modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int test_high_int_pressure(int seed) {
    volatile int result = 0;
    int a = seed * 3 + 7;
    int b = seed / 2 - 5;
    int c = seed ^ 0xABCDEF;
    int d = seed + 12345;
    int e = seed * seed;
    int f = seed | 0xFF00FF;
    int g = seed & 0x00FF00FF;
    int h = ~seed;
    
    /* Force values into registers with inline asm */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
    
    /* Long sequence of independent computations */
    int t1 = a * b + c - d;
    int t2 = (e & f) | (g ^ h);
    int t3 = t1 * t2 / (a + 1);
    int t4 = (b << 3) | (c >> 2);
    int t5 = t3 ^ t4;
    int t6 = t5 * 0x1234567;
    int t7 = t6 + a * b * c;
    int t8 = t7 - d * e * f;
    int t9 = t8 | g * h;
    int t10 = t9 & 0xFFFFFFFF;
    
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4));
    
    /* Function call creates register pressure */
    int t11 = external_func1(t10);
    int t12 = external_func2(t11, t5);
    
    /* More computations */
    int t13 = t12 * 137 + t6;
    int t14 = (t13 << 1) | (t13 >> 31);
    int t15 = t14 ^ t7 ^ t8;
    int t16 = t15 * 0x9E3779B9;
    int t17 = t16 + t9 + t10;
    int t18 = t17 * 1103515245 + 12345;
    
    asm volatile("" : : "r"(t11), "r"(t12), "r"(t13), "r"(t14));
    
    /* Another function call */
    int t19 = external_func2(t18, t15);
    
    /* Final computation chain */
    int t20 = t19 * 3 + 5;
    int t21 = t20 / 7 * 11;
    int t22 = t21 ^ t16;
    int t23 = t22 | t17;
    int t24 = t23 & 0x7FFFFFFF;
    
    result = t24;
    return result;
}

/* Test 2: Mixed integer and floating-point operations */
double test_mixed_types(int seed) {
    volatile double result = 0.0;
    double d1 = seed * 1.234;
    double d2 = seed / 3.14159;
    float f1 = seed * 0.5f;
    float f2 = seed + 2.718f;
    int i1 = seed * 7;
    int i2 = seed + 11;
    
    asm volatile("" : : "r"(i1), "r"(i2));
    
    /* Mixed computations */
    double t1 = d1 * d2 + f1 * f2;
    float t2 = f1 / f2 - d1 + d2;
    int t3 = i1 * i2 + (int)d1;
    double t3_d = t3 * 0.01;
    
    asm volatile("" : : "r"(t3));
    
    /* Function calls with different types */
    double t4 = external_func3(t1);
    int t5 = external_func1(t3);
    
    /* More mixed ops */
    double t6 = t4 * t3_d + t2;
    float t7 = t2 * 2.0f - (float)t4;
    int t8 = t5 ^ (int)t6;
    double t9 = t6 / t3_d * t7;
    
    asm volatile("" : : "r"(t8));
    
    /* Another call */
    long long t10 = external_func4(t8);
    double t11 = t9 + (double)t10 * 0.000001;
    
    result = t11;
    return result;
}

/* Test 3: Nested loops with complex induction variables */
int test_nested_loops(int N) {
    volatile int sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Non-trivial induction variable */
        for (int j = i * 2; j < N; j += 3) {
            int a = i * j + 7;
            int b = (i ^ j) * 3;
            int c = a * b - j;
            int d = c / (i + 1) + 11;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            
            /* Function call inside inner loop */
            int e = external_func1(d);
            
            /* More computations */
            int f = e * 137 + a;
            int g = (f << 2) | (f >> 30);
            int h = g ^ b ^ c;
            
            asm volatile("" : : "r"(e), "r"(f), "r"(g));
            
            sum += h;
            
            /* Another call with register pressure */
            if (j % 5 == 0) {
                int k = external_func2(h, i);
                sum += k;
            }
        }
        
        /* Additional computation between outer loop iterations */
        int outer_temp = i * 0x9E3779B9;
        outer_temp = (outer_temp << 1) | (outer_temp >> 31);
        sum += outer_temp;
        
        asm volatile("" : : "r"(outer_temp));
    }
    
    return sum;
}

/* Test 4: Vector operations with different modes */
int test_vector_ops(int seed) {
    volatile int result = 0;
    
    /* Create vector values */
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    v4si v3 = {seed ^ 1, seed ^ 2, seed ^ 3, seed ^ 4};
    
    /* Scalar computations to mix with vectors */
    int s1 = seed * 7;
    int s2 = seed + 11;
    int s3 = seed ^ 0x55AA;
    
    asm volatile("" : : "r"(s1), "r"(s2), "r"(s3));
    
    /* Vector operations */
    v4si v4 = v1 + v2;
    v4si v5 = v3 * v4;
    v4si v6 = v5 & v1;
    v4si v7 = v6 | v2;
    
    /* Extract elements to force register usage */
    int e1 = v7[0] + s1;
    int e2 = v7[1] * s2;
    int e3 = v7[2] ^ s3;
    int e4 = v7[3] - s1;
    
    asm volatile("" : : "r"(e1), "r"(e2), "r"(e3), "r"(e4));
    
    /* Function calls */
    int t1 = external_func1(e1);
    int t2 = external_func2(e2, e3);
    
    /* More mixed computations */
    v4si v8 = {t1, t2, e3, e4};
    v4si v9 = v8 * v7;
    
    int r1 = v9[0] + v9[1];
    int r2 = v9[2] * v9[3];
    int r3 = r1 ^ r2;
    
    asm volatile("" : : "r"(r1), "r"(r2));
    
    result = r3;
    return result;
}

/* Test 5: Extreme register pressure with unrolled computations */
long long test_extreme_pressure(int iterations) {
    volatile long long checksum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Manually unrolled computation block */
        long long a = i * 0x123456789ABCDEFLL;
        long long b = a ^ 0xFEDCBA9876543210LL;
        long long c = b * 6364136223846793005LL;
        long long d = c + 1442695040888963407LL;
        long long e = d ^ (d >> 32);
        long long f = e * 0x2545F4914F6CDD1DLL;
        long long g = f + a;
        long long h = g ^ b;
        long long j = h * c;
        long long k = j | d;
        long long l = k & e;
        long long m = l ^ f;
        long long n = m + g;
        long long o = n * h;
        long long p = o ^ j;
        long long q = p | k;
        long long r = q & l;
        long long s = r ^ m;
        long long t = s + n;
        long long u = t * o;
        long long v = u ^ p;
        long long w = v | q;
        long long x = w & r;
        long long y = x ^ s;
        long long z = y + t;
        
        /* Force all intermediates to registers */
        asm volatile("" : : 
            "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f),
            "r"(g), "r"(h), "r"(j), "r"(k), "r"(l), "r"(m),
            "r"(n), "r"(o), "r"(p), "r"(q), "r"(r), "r"(s),
            "r"(t), "r"(u), "r"(v), "r"(w), "r"(x), "r"(y), "r"(z));
        
        /* Function call to create register pressure */
        long long temp = external_func4(z);
        
        checksum += temp;
        
        /* Additional pressure point */
        if (i % 3 == 0) {
            int temp2 = external_func1((int)temp);
            checksum += temp2;
        }
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int N = argc > 2 ? atoi(argv[2]) : 100;
    int iterations = argc > 3 ? atoi(argv[3]) : 50;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to exercise different patterns */
    int r1 = test_high_int_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_mixed_types(seed);
    printf("Test 2 result: %f\n", r2);
    
    int r3 = test_nested_loops(N);
    printf("Test 3 result: %d\n", r3);
    
    int r4 = test_vector_ops(seed);
    printf("Test 4 result: %d\n", r4);
    
    long long r5 = test_extreme_pressure(iterations);
    printf("Test 5 result: %lld\n", r5);
    
    /* Final volatile store to prevent optimization */
    volatile int final_result = r1 + (int)r2 + r3 + r4 + (int)r5;
    printf("Final combined result: %d\n", final_result);
    
    return 0;
}
