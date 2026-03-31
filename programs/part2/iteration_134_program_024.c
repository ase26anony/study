/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to create register pressure points */
extern int ext_func1(int);
extern int ext_func2(int, int);
extern double ext_func3(double);
extern long long ext_func4(long long);

/* Prevent optimization */
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Test 1: High integer register pressure with complex expressions */
int test1_high_int_pressure(int seed) {
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0x1234;
    int d = seed - 19;
    
    /* Long sequence of independent computations */
    int t1 = a * b + c;
    asm volatile("" : : "r"(t1));
    
    int t2 = (b << 3) | (c >> 2);
    asm volatile("" : : "r"(t2));
    
    int t3 = ext_func1(d);  /* Function call creates pressure */
    
    int t4 = t1 ^ t2 ^ t3;
    asm volatile("" : : "r"(t4));
    
    int t5 = (a & b) | (c & d);
    asm volatile("" : : "r"(t5));
    
    int t6 = ext_func2(t4, t5);
    
    int t7 = t3 * t6 - t1;
    asm volatile("" : : "r"(t7));
    
    int t8 = (t2 << 1) + (t5 >> 1);
    asm volatile("" : : "r"(t8));
    
    int t9 = t4 % 31 + t7;
    asm volatile("" : : "r"(t9));
    
    int t10 = ext_func1(t8);
    
    /* More computations to increase pressure */
    int t11 = t9 * 3 + t10;
    int t12 = t7 ^ t8 ^ t9;
    int t13 = (t10 << 2) | (t11 >> 2);
    int t14 = t12 - t13;
    int t15 = ext_func2(t14, t11);
    
    /* Force all values to be used */
    int result = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 + 
                 t11 + t12 + t13 + t14 + t15;
    
    return result;
}

/* Test 2: Mixed integer and floating-point pressure */
double test2_mixed_types(int seed) {
    double f1 = seed * 1.5;
    double f2 = seed / 3.14159;
    int i1 = seed * 2;
    int i2 = seed + 100;
    
    /* Mix operations */
    double t1 = f1 * f2 + i1;
    asm volatile("" : : "r"(t1));
    
    int t2 = i1 * i2 - (int)f1;
    asm volatile("" : : "r"(t2));
    
    double t3 = ext_func3(f2);  /* External FP call */
    
    int t3_int = (int)t3;
    double t4 = t1 * t3 + f2;
    asm volatile("" : : "r"(t4));
    
    int t5 = ext_func1(t2);
    
    double t6 = t4 / 2.0 + t3;
    asm volatile("" : : "r"(t6));
    
    int t7 = t5 ^ t3_int;
    asm volatile("" : : "r"(t7));
    
    /* More mixed computations */
    double t8 = t6 * 3.14159;
    int t9 = t7 * 2 + i2;
    double t10 = ext_func3(t8);
    int t11 = ext_func2(t9, t5);
    double t12 = t10 - t8 + t4;
    
    /* Force use of all values */
    double result = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 + t11 + t12;
    
    return result;
}

/* Test 3: Nested loops with complex induction variables */
long long test3_nested_loops(int N) {
    long long total = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable */
        for (int j = i * 2; j < N; j += 3) {
            /* Inner loop computations with register pressure */
            int a = i * j;
            int b = j + 7;
            int c = a ^ b;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            
            int d = ext_func1(c);
            int e = (a << 2) | (b >> 2);
            
            asm volatile("" : : "r"(d), "r"(e));
            
            int f = ext_func2(d, e);
            total += f + c;
            
            /* Additional computations to increase pressure */
            int g = f * 3 - i;
            int h = j % 17 + g;
            long long k = (long long)g * h;
            
            asm volatile("" : : "r"(g), "r"(h), "r"(k));
            
            total += k;
        }
        
        /* Function call between loop iterations */
        if (i % 5 == 0) {
            total += ext_func4(total);
        }
    }
    
    return total;
}

/* Test 4: Vector-like operations using structs */
typedef struct {
    int x, y, z, w;
} Vec4;

int test4_vector_ops(int seed) {
    Vec4 v1 = {seed, seed + 1, seed + 2, seed + 3};
    Vec4 v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    
    /* Simulate vector operations */
    int t1 = v1.x * v2.x;
    int t2 = v1.y * v2.y;
    int t3 = v1.z * v2.z;
    int t4 = v1.w * v2.w;
    
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4));
    
    int t5 = ext_func1(t1);
    int t6 = ext_func2(t2, t3);
    
    Vec4 v3;
    v3.x = t5 + t1;
    v3.y = t6 + t2;
    v3.z = t3 * 2 - t4;
    v3.w = t4 / 2 + t1;
    
    asm volatile("" : : "r"(v3.x), "r"(v3.y), "r"(v3.z), "r"(v3.w));
    
    /* More operations */
    int t7 = v3.x ^ v3.y;
    int t8 = v3.z | v3.w;
    int t9 = ext_func1(t7);
    int t10 = ext_func2(t8, t9);
    
    /* Long dependency chain */
    int t11 = t7 * 3 + t8;
    int t12 = t9 - t10;
    int t13 = t11 ^ t12;
    int t14 = ext_func1(t13);
    int t15 = t14 * 2 + t13;
    
    asm volatile("" : : "r"(t11), "r"(t12), "r"(t13), "r"(t14), "r"(t15));
    
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 + t11 + t12 + t13 + t14 + t15;
}

/* Test 5: 64-bit operations with high pressure */
long long test5_64bit_ops(long long seed) {
    long long a = seed * 3LL;
    long long b = seed + 0x123456789ABCDEFLL;
    long long c = seed ^ 0xFEDCBA9876543210LL;
    
    /* 64-bit computations */
    long long t1 = a * b + c;
    asm volatile("" : : "r"(t1));
    
    long long t2 = (b << 5) | (c >> 3);
    asm volatile("" : : "r"(t2));
    
    long long t3 = ext_func4(a);  /* External 64-bit call */
    
    long long t4 = t1 ^ t2 ^ t3;
    asm volatile("" : : "r"(t4));
    
    long long t5 = (a & b) | (c & seed);
    asm volatile("" : : "r"(t5));
    
    /* More 64-bit operations */
    long long t6 = t3 * t4 - t1;
    long long t7 = t2 + t5 * 2;
    long long t8 = ext_func4(t6);
    long long t9 = t7 % 1000000007 + t8;
    long long t10 = t4 * 3 + t9;
    
    asm volatile("" : : "r"(t6), "r"(t7), "r"(t8), "r"(t9), "r"(t10));
    
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
}

/* External function implementations */
int ext_func1(int x) {
    return x ^ 0x55AA55AA;
}

int ext_func2(int x, int y) {
    return (x * y) + (x ^ y);
}

double ext_func3(double x) {
    return x * 1.23456789;
}

long long ext_func4(long long x) {
    return x * 0x12345678LL;
}

int main(int argc, char **argv) {
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int N = argc > 2 ? atoi(argv[2]) : 100;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to trigger different patterns */
    int r1 = test1_high_int_pressure(seed);
    printf("Test1 result: %d\n", r1);
    
    double r2 = test2_mixed_types(seed);
    printf("Test2 result: %f\n", r2);
    
    long long r3 = test3_nested_loops(N);
    printf("Test3 result: %lld\n", r3);
    
    int r4 = test4_vector_ops(seed);
    printf("Test4 result: %d\n", r4);
    
    long long r5 = test5_64bit_ops(seed);
    printf("Test5 result: %lld\n", r5);
    
    /* Final checksum to prevent optimization */
    long long final = r1 + (long long)r2 + r3 + r4 + r5;
    volatile long long sink = final;
    (void)sink;
    
    printf("All tests completed.\n");
    return 0;
}
