/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to force register pressure */
extern int ext_func1(int);
extern int ext_func2(int, int);
extern double ext_func3(double);
extern long long ext_func4(long long);

/* Prevent optimization */
static void use(volatile void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Test 1: High integer register pressure with complex expressions */
int test1_high_int_pressure(int seed) {
    volatile int result = 0;
    
    /* Create many independent integer computations */
    int a = seed * 3 + 7;
    int b = seed / 2 - 5;
    int c = seed ^ 0xABCD;
    int d = seed << 3;
    int e = seed >> 2;
    int f = seed | 0x1234;
    int g = seed & 0xF0F0;
    int h = ~seed;
    
    /* Force values to be kept in registers */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
    
    /* Complex expressions creating many temporaries */
    int t1 = a * b + c - d;
    int t2 = (e & f) | (g ^ h);
    int t3 = t1 * t2 / (a + 1);
    int t4 = (b << 2) + (c >> 1) - (d & 0xFF);
    
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4));
    
    /* Function call that clobbers caller-saved registers */
    int r1 = ext_func1(t1);
    
    /* More computations after call */
    int t5 = t3 + r1 * 2;
    int t6 = t4 ^ (r1 << 3);
    int t7 = t5 * 3 - t6 / 2;
    
    /* Another function call */
    int r2 = ext_func2(t5, t6);
    
    /* Final complex expression */
    result = t7 + r2 + (a * b * c * d) / 1000;
    
    /* Use volatile to prevent elimination */
    use(&result);
    return result;
}

/* Test 2: Mixed integer and floating-point pressure */
double test2_mixed_types(int seed) {
    volatile double result = 0.0;
    
    /* Integer computations */
    int i1 = seed * 2;
    int i2 = seed + 100;
    int i3 = seed ^ 0xDEAD;
    int i4 = seed | 0xBEEF;
    
    /* Floating-point computations */
    double f1 = seed * 1.234;
    double f2 = seed / 3.14159;
    double f3 = f1 * f2;
    double f4 = f1 + f2 * 2.0;
    
    /* Force both types into registers */
    asm volatile("" : : "r"(i1), "r"(i2), "r"(i3), "r"(i4));
    asm volatile("" : : "r"(f1), "r"(f2), "r"(f3), "r"(f4));
    
    /* Mixed-type expressions */
    double m1 = f1 * i1 + f2 * i2;
    double m2 = (f3 + i3) * (f4 - i4);
    
    /* Function call with floating point */
    double r1 = ext_func3(m1);
    
    /* More mixed computations */
    int i5 = i1 * i2 + (int)r1;
    double f5 = f3 * f4 - r1;
    
    /* Long long computations */
    long long ll1 = (long long)seed * 1000000LL;
    long long ll2 = ll1 ^ 0x123456789ABCDEF0LL;
    
    asm volatile("" : : "r"(ll1), "r"(ll2));
    
    /* Another external call */
    long long r2 = ext_func4(ll1);
    
    /* Final result */
    result = m2 + f5 + i5 + (double)r2;
    
    use(&result);
    return result;
}

/* Test 3: Nested loops with complex induction variables */
int test3_nested_loops(int N) {
    volatile int total = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable for inner loop */
        for (int j = i * 2; j < N; j += 3) {
            /* Many independent computations in loop body */
            int a = i * j + 7;
            int b = (i ^ j) << 2;
            int c = a * b - j;
            int d = (i & j) | 0x55;
            
            /* Force values to registers */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            /* Function call inside loop */
            int r = ext_func1(c);
            
            /* More computations */
            int e = d * r + a;
            int f = (b >> 1) ^ r;
            
            /* Use volatile assembly to prevent optimization */
            asm volatile("" : "+r"(e), "+r"(f));
            
            total += e + f;
        }
        
        /* Additional computation between inner loops */
        int t = i * i - i + 5;
        asm volatile("" : : "r"(t));
        total += ext_func2(t, i);
    }
    
    use(&total);
    return total;
}

/* Test 4: Vector-like operations using wide types */
long long test4_wide_types(int seed) {
    volatile long long result = 0;
    
    /* Use 64-bit operations */
    long long ll1 = (long long)seed * 1000000007LL;
    long long ll2 = ll1 ^ 0xFEDCBA9876543210LL;
    long long ll3 = ll1 + ll2 * 3;
    long long ll4 = (ll1 & ll2) | (ll3 << 5);
    
    /* Force 64-bit values into registers */
    asm volatile("" : : "r"(ll1), "r"(ll2), "r"(ll3), "r"(ll4));
    
    /* Complex 64-bit expressions */
    long long t1 = ll1 * ll2 / (ll3 + 1);
    long long t2 = (ll4 ^ ll1) + (ll2 & ll3);
    long long t3 = t1 * 7 - t2 / 3;
    
    /* Mix with 32-bit operations */
    int i1 = seed * 3;
    int i2 = seed + 777;
    long long t4 = t3 + (long long)i1 * i2;
    
    /* External call with 64-bit argument */
    long long r = ext_func4(t4);
    
    /* Final computation */
    result = t1 + t2 + t3 + t4 + r;
    
    use(&result);
    return result;
}

/* Test 5: Extreme register pressure with many live values */
double test5_extreme_pressure(int seed) {
    volatile double checksum = 0.0;
    
    /* Create many live variables of different types */
    int v1 = seed;
    int v2 = v1 * 2;
    int v3 = v1 + v2;
    int v4 = v1 ^ v2;
    int v5 = v3 & v4;
    int v6 = v5 | v1;
    int v7 = v6 << 3;
    int v8 = v7 >> 1;
    int v9 = v8 - v3;
    int v10 = v9 * 7;
    
    double f1 = v1 * 0.1;
    double f2 = v2 * 0.2;
    double f3 = v3 * 0.3;
    double f4 = v4 * 0.4;
    double f5 = v5 * 0.5;
    
    long long l1 = v6 * 100LL;
    long long l2 = v7 * 200LL;
    long long l3 = v8 * 300LL;
    
    /* Force ALL values into registers simultaneously */
    asm volatile("" : : 
        "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5),
        "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10),
        "r"(f1), "r"(f2), "r"(f3), "r"(f4), "r"(f5),
        "r"(l1), "r"(l2), "r"(l3));
    
    /* Complex web of dependencies */
    double r1 = f1 * f2 + f3;
    int r2 = ext_func1(v10);
    double r3 = r1 * f4 - f5;
    long long r4 = ext_func4(l1);
    
    int r5 = v9 * r2 + v8;
    double r6 = r3 / (f1 + 1.0);
    long long r7 = l2 + r4 * 2;
    
    /* Final aggregation */
    checksum = r1 + r3 + r6 + (double)r2 + (double)r5 + (double)r4 + (double)r7;
    
    use(&checksum);
    return checksum;
}

/* Dummy external functions */
int ext_func1(int x) { return x ^ 0x1234; }
int ext_func2(int x, int y) { return (x + y) * 2; }
double ext_func3(double x) { return x * 1.5; }
long long ext_func4(long long x) { return x + 0x1000; }

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    
    printf("Running early rematerialization tests...\n");
    
    /* Run all tests to exercise different patterns */
    int r1 = test1_high_int_pressure(seed);
    printf("Test1 result: %d\n", r1);
    
    double r2 = test2_mixed_types(seed);
    printf("Test2 result: %f\n", r2);
    
    int r3 = test3_nested_loops(seed % 50 + 10);
    printf("Test3 result: %d\n", r3);
    
    long long r4 = test4_wide_types(seed);
    printf("Test4 result: %lld\n", r4);
    
    double r5 = test5_extreme_pressure(seed);
    printf("Test5 result: %f\n", r5);
    
    /* Final checksum to prevent elimination */
    volatile int final = r1 + (int)r2 + r3 + (int)r4 + (int)r5;
    printf("Final checksum: %d\n", final);
    
    return 0;
}
