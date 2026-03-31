/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to force register pressure */
extern int ext_func1(int);
extern double ext_func2(double);
extern long long ext_func3(long long);

/* Dummy external functions */
int ext_func1(int x) { return x ^ 0x1234; }
double ext_func2(double x) { return x * 1.234; }
long long ext_func3(long long x) { return x + 0xABCDEF; }

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High register pressure with mixed integer operations */
int test_mixed_integer_pressure(int seed) {
    volatile int result = 0;
    int a = seed * 3 + 1;
    int b = seed / 2 - 5;
    int c = seed + 0x7FFF;
    int d = seed ^ 0xABCD;
    
    /* Long sequence of independent computations */
    int t1 = a * b + c;
    asm volatile("" : : "r"(t1)); /* Force into register */
    
    int t2 = (c & d) | (a ^ b);
    asm volatile("" : : "r"(t2));
    
    int t3 = t1 * t2 - a;
    asm volatile("" : : "r"(t3));
    
    /* Function call creates pressure point */
    int t4 = ext_func1(t3);
    
    int t5 = t4 * 7 + b;
    asm volatile("" : : "r"(t5));
    
    int t6 = (t5 << 3) | (t4 >> 2);
    asm volatile("" : : "r"(t6));
    
    int t7 = t6 * t3 / 5;
    asm volatile("" : : "r"(t7));
    
    int t8 = ext_func1(t7);
    
    int t9 = t8 ^ t5 ^ t2;
    asm volatile("" : : "r"(t9));
    
    int t10 = t9 * 11 - t1;
    asm volatile("" : : "r"(t10));
    
    result = t10;
    return result;
}

/* Test 2: Floating-point and integer mix with nested loops */
double test_fp_int_mix(int N) {
    double sum = 0.0;
    volatile double check = 0.0;
    
    for (int i = 0; i < N; i++) {
        /* Outer loop with complex induction */
        double base = i * 1.5;
        int mod = i % 7;
        
        for (int j = i * 2; j < N; j += 3) {
            /* Inner loop with derived induction variable */
            double fp_val = base + j * 0.25;
            int int_val = j * mod + i;
            
            /* Mixed computations */
            double t1 = fp_val * int_val;
            asm volatile("" : : "r"(t1));
            
            double t2 = ext_func2(t1);
            
            int t3 = int_val ^ (j << 2);
            asm volatile("" : : "r"(t3));
            
            double t4 = t2 + t3;
            asm volatile("" : : "r"(t4));
            
            int t5 = ext_func1(t3);
            
            double t6 = t4 * t5 / (j + 1);
            asm volatile("" : : "r"(t6));
            
            sum += t6;
            
            /* Another function call */
            if (j % 5 == 0) {
                double t7 = ext_func2(sum);
                asm volatile("" : : "r"(t7));
                check += t7;
            }
        }
    }
    
    return sum + check;
}

/* Test 3: Long long and 64-bit operations */
long long test_64bit_ops(long long seed) {
    volatile long long result = 0;
    long long a = seed * 0x123456789ABCDEFLL;
    long long b = seed + 0xFEDCBA9876543210LL;
    long long c = seed ^ 0xAAAAAAAAAAAAAAAALL;
    
    /* Chain of 64-bit operations */
    long long t1 = a * b >> 3;
    asm volatile("" : : "r"(t1));
    
    long long t2 = (c & b) | (a ^ 0x5555);
    asm volatile("" : : "r"(t2));
    
    long long t3 = ext_func3(t1);
    
    long long t4 = t3 * t2 - a;
    asm volatile("" : : "r"(t4));
    
    long long t5 = (t4 << 5) | (t3 >> 11);
    asm volatile("" : : "r"(t5));
    
    long long t6 = ext_func3(t5);
    
    long long t7 = t6 * 19 + b;
    asm volatile("" : : "r"(t7));
    
    long long t8 = t7 ^ t4 ^ t1;
    asm volatile("" : : "r"(t8));
    
    long long t9 = ext_func3(t8);
    
    long long t10 = t9 * 7 - c;
    asm volatile("" : : "r"(t10));
    
    result = t10;
    return result;
}

/* Test 4: Vector operations with mixed modes */
int test_vector_ops(int iterations) {
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = {9, 10, 11, 12};
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Vector operations */
        v4si r1 = v1 * v2 + v3;
        asm volatile("" : : "r"(r1));
        
        v4si r2 = r1 & v2 | v3;
        asm volatile("" : : "r"(r2));
        
        /* Scalar extraction and computation */
        int s1 = r1[0] + r1[1];
        int s2 = r2[2] * r2[3];
        asm volatile("" : : "r"(s1), "r"(s2));
        
        int s3 = ext_func1(s1);
        int s4 = s3 * s2 - i;
        asm volatile("" : : "r"(s4));
        
        v4si r3 = r1 + r2 * s4;
        asm volatile("" : : "r"(r3));
        
        /* Update vectors */
        v1 = v2 + r3;
        v2 = v3 * r1;
        v3 = r2 - v1;
        
        sum += r3[0] + r3[1] + r3[2] + r3[3];
    }
    
    return sum;
}

/* Test 5: Complex expression chains with multiple function calls */
double test_complex_chains(int depth) {
    volatile double final_result = 0.0;
    double acc = 1.0;
    int int_acc = depth;
    
    for (int i = 0; i < depth; i++) {
        /* Create deep dependency chain */
        double d1 = acc * 1.6180339887;
        int i1 = int_acc * 3 + i;
        asm volatile("" : : "r"(d1), "r"(i1));
        
        double d2 = ext_func2(d1);
        int i2 = ext_func1(i1);
        
        double d3 = d2 * i2 / (i + 1);
        asm volatile("" : : "r"(d3));
        
        int i3 = i2 ^ (i1 << 1);
        asm volatile("" : : "r"(i3));
        
        double d4 = d3 + ext_func2(d3);
        int i4 = ext_func1(i3);
        
        double d5 = d4 * i4;
        asm volatile("" : : "r"(d5));
        
        acc = d5;
        int_acc = i4;
        
        if (i % 4 == 0) {
            double d6 = ext_func2(acc);
            asm volatile("" : : "r"(d6));
            final_result += d6;
        }
    }
    
    return final_result + acc;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int N = argc > 2 ? atoi(argv[2]) : 100;
    
    printf("Testing early rematerialization patterns...\n");
    
    /* Run all tests to create various register pressure scenarios */
    int r1 = test_mixed_integer_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_fp_int_mix(N);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_64bit_ops(seed);
    printf("Test 3 result: %lld\n", r3);
    
    int r4 = test_vector_ops(N / 10);
    printf("Test 4 result: %d\n", r4);
    
    double r5 = test_complex_chains(50);
    printf("Test 5 result: %f\n", r5);
    
    /* Use results to prevent optimization */
    volatile int checksum = r1 + (int)r2 + (int)r3 + r4 + (int)r5;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
