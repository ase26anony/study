/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to force register pressure */
extern int ext_func1(int);
extern double ext_func2(double);
extern long long ext_func3(long long);

/* Dummy external functions */
int ext_func1(int x) { return x ^ 0x55AA55AA; }
double ext_func2(double x) { return x * 1.234567; }
long long ext_func3(long long x) { return x + 0x123456789ABCDEFLL; }

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int test_integer_pressure(int seed) {
    volatile int result = 0;
    int a = seed * 3 + 1;
    int b = seed / 2 - 5;
    int c = seed ^ 0x1234;
    int d = seed + 777;
    
    /* Long sequence of independent computations */
    int t1 = a * b + c - d;
    asm volatile("" : : "r"(t1)); /* Force into register */
    
    int t2 = (b << 3) | (c >> 2);
    int t3 = t1 ^ t2;
    
    /* Function call creates pressure point */
    int t4 = ext_func1(t3);
    
    int t5 = t4 * 7 + 12345;
    asm volatile("" : : "r"(t5));
    
    int t6 = (t5 & 0xFF00) >> 8;
    int t7 = t6 * t4 - t3;
    
    int t8 = ext_func1(t7);
    
    int t9 = t8 * 31 + 17;
    int t10 = t9 ^ t8;
    int t11 = t10 * 2 - t9;
    
    asm volatile("" : : "r"(t11));
    
    /* More computations */
    int t12 = t11 * a + b * c;
    int t13 = (t12 << 1) | (t12 >> 31);
    int t14 = ext_func1(t13);
    
    int t15 = t14 * 19 - 54321;
    int t16 = t15 ^ seed;
    int t17 = t16 + t15 * 2;
    
    asm volatile("" : : "r"(t17));
    
    result = t17;
    return result;
}

/* Test 2: Mixed integer and floating-point pressure */
double test_mixed_pressure(double seed) {
    volatile double result = 0.0;
    double f1 = seed * 1.5;
    double f2 = seed / 3.14159;
    int i1 = (int)seed * 3;
    int i2 = (int)(seed * 2.71828);
    
    /* Mix FP and integer ops */
    double ft1 = f1 * f2 + 123.456;
    asm volatile("" : : "r"(ft1)); /* Force FP register */
    
    int it1 = i1 * i2 + 777;
    asm volatile("" : : "r"(it1)); /* Force integer register */
    
    double ft2 = ext_func2(ft1);
    int it2 = ext_func1(it1);
    
    double ft3 = ft2 * (double)it2;
    int it3 = (int)ft3 ^ it2;
    
    /* More mixing */
    double ft4 = ft3 + (double)it3 / 256.0;
    int it4 = it3 * 3 + (int)ft4;
    
    asm volatile("" : : "r"(ft4), "r"(it4));
    
    double ft5 = ext_func2(ft4);
    int it5 = ext_func1(it4);
    
    result = ft5 + (double)it5;
    return result;
}

/* Test 3: Nested loops with complex induction */
long long test_nested_loops(int N) {
    volatile long long sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable */
        for (int j = i * 2; j < N; j += 3) {
            /* Inner loop with register pressure */
            int a = i * j + 123;
            int b = (j << 2) | (i >> 1);
            int c = a ^ b;
            
            asm volatile("" : : "r"(c));
            
            int d = ext_func1(c);
            int e = d * 7 - 31;
            
            /* More computations to increase pressure */
            long long f = (long long)e * j;
            long long g = f + (long long)i * 256;
            
            asm volatile("" : : "r"(g));
            
            long long h = ext_func3(g);
            sum += h;
            
            /* Additional pressure point */
            int k = (e * 3) & 0xFF;
            asm volatile("" : : "r"(k));
        }
    }
    
    return sum;
}

/* Test 4: 64-bit and vector operations */
v4si test_vector_ops(int seed) {
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    
    /* Vector operations */
    v4si v3 = v1 + v2;
    v4si v4 = v1 * v2;
    
    asm volatile("" : : "r"(v3), "r"(v4));
    
    /* Mix with scalar */
    long long ll1 = seed * 1000000007LL;
    long long ll2 = ll1 ^ 0x123456789ABCDEFLL;
    
    asm volatile("" : : "r"(ll2));
    
    v4si v5 = v3 - v4;
    long long ll3 = ext_func3(ll2);
    
    /* More vector-scalar mixing */
    int scalar = (int)(ll3 & 0xFFFFFFFF);
    v4si v6 = v5 * scalar;
    
    asm volatile("" : : "r"(v6));
    
    return v6;
}

/* Test 5: Extreme register pressure with many temporaries */
double test_extreme_pressure(int iterations) {
    volatile double total = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        /* Create many independent values */
        double a = i * 1.234;
        double b = a * 2.71828;
        double c = b / 3.14159;
        double d = c + 987.654;
        
        int ia = i * 3;
        int ib = ia ^ 0x55AA;
        int ic = ib * 7;
        int id = ic + 12345;
        
        /* Force all into registers */
        asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
        asm volatile("" : : "r"(ia), "r"(ib), "r"(ic), "r"(id));
        
        /* More computations */
        double e = ext_func2(d);
        int ie = ext_func1(id);
        
        double f = e * (double)ie;
        int ig = ie * 13 - 777;
        
        double g = f + (double)ig / 1000.0;
        int ih = ig ^ (int)g;
        
        asm volatile("" : : "r"(g), "r"(ih));
        
        total += g + (double)ih;
        
        /* Function call between dependent ops */
        double h = ext_func2(g);
        int ii = ext_func1(ih);
        
        total += h * (double)ii;
    }
    
    return total;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int N = argc > 2 ? atoi(argv[2]) : 100;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to trigger different parts of the pass */
    int r1 = test_integer_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_mixed_pressure((double)seed);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_nested_loops(N);
    printf("Test 3 result: %lld\n", r3);
    
    v4si r4 = test_vector_ops(seed);
    printf("Test 4 result: {%d, %d, %d, %d}\n", 
           r4[0], r4[1], r4[2], r4[3]);
    
    double r5 = test_extreme_pressure(N / 10);
    printf("Test 5 result: %f\n", r5);
    
    /* Final volatile store to prevent optimization */
    volatile int final_check = r1 + (int)r2 + (int)r3 + r4[0] + (int)r5;
    
    printf("All tests completed.\n");
    return 0;
}
