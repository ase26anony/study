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
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0x55AA55AA;
    int d = seed | 0x12345678;
    
    /* Long sequence of independent computations */
    int t1 = a * b + c;
    asm volatile("" : : "r"(t1)); /* Pin in register */
    
    int t2 = (b << 3) | (c >> 2);
    int t3 = t1 ^ t2;
    asm volatile("" : : "r"(t3));
    
    int t4 = ext_func1(t3); /* Function call creates pressure point */
    
    int t5 = t4 * 17 - 23;
    int t6 = (t5 & 0xFF00FF00) >> 8;
    asm volatile("" : : "r"(t6));
    
    int t7 = t6 + t4 * 3;
    int t8 = t7 ^ (t7 << 16);
    asm volatile("" : : "r"(t8));
    
    int t9 = ext_func1(t8); /* Another pressure point */
    
    int t10 = t9 * 31 + 47;
    int t11 = (t10 >> 4) & 0x0F0F0F0F;
    int t12 = t11 * 59 - 71;
    asm volatile("" : : "r"(t12));
    
    int t13 = t12 | t9;
    int t14 = t13 * 73 + 89;
    asm volatile("" : : "r"(t14));
    
    int t15 = ext_func1(t14);
    
    /* More computations to increase pressure */
    int t16 = t15 * 97 - 101;
    int t17 = (t16 & 0x33333333) << 2;
    int t18 = t17 | (t16 & 0xCCCCCCCC) >> 2;
    asm volatile("" : : "r"(t18));
    
    int t19 = t18 * 103 + 107;
    int t20 = t19 ^ 0xDEADBEEF;
    asm volatile("" : : "r"(t20));
    
    use_result = t20;
    return use_result;
}

/* Test 2: Floating point and integer mix with nested loops */
double test_floating_point_pressure(int N) {
    double result = 0.0;
    volatile double vol_result = 0.0;
    
    for (int i = 0; i < N; i++) {
        double base = i * 1.234567;
        double acc = base;
        
        /* Inner loop with complex induction */
        for (int j = i * 2; j < N; j += 3) {
            double x = j * 0.987654;
            double y = x * x + 2.0 * x + 1.0;
            double z = y / (x + 1.0);
            
            /* Force register pressure with inline asm */
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
            
            acc += z * (j - i);
            
            /* Function call creates register pressure */
            if (j % 7 == 0) {
                acc = ext_func2(acc);
            }
        }
        
        /* More computations after inner loop */
        double t1 = acc * 3.14159;
        double t2 = t1 + base * 2.71828;
        asm volatile("" : : "r"(t1), "r"(t2));
        
        double t3 = ext_func2(t2);
        double t4 = t3 * t3 - 2.0 * t3 + 1.0;
        asm volatile("" : : "r"(t4));
        
        result += t4;
        
        /* Integer computation mixed in */
        int int_val = (int)result ^ i;
        asm volatile("" : : "r"(int_val));
        result += int_val * 0.001;
    }
    
    vol_result = result;
    return vol_result;
}

/* Test 3: 64-bit operations and vector types */
long long test_64bit_vector_pressure(int iterations) {
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    long long ll_result = 0;
    volatile long long vol_ll = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Vector operations */
        v4si vec3 = vec1 + vec2;
        v4si vec4 = vec1 * vec2;
        v4si vec5 = vec3 | vec4;
        v4si vec6 = vec5 & vec2;
        
        /* Force vector values to registers */
        asm volatile("" : : "r"(vec3), "r"(vec4), "r"(vec5), "r"(vec6));
        
        /* 64-bit computations */
        long long ll1 = (long long)i << 32;
        long long ll2 = ll1 + 0x123456789ABCDEFLL;
        long long ll3 = ll2 * 31;
        asm volatile("" : : "r"(ll1), "r"(ll2), "r"(ll3));
        
        long long ll4 = ext_func3(ll3); /* External call */
        
        long long ll5 = ll4 ^ (ll4 >> 32);
        long long ll6 = ll5 * 59 + 73;
        asm volatile("" : : "r"(ll5), "r"(ll6));
        
        /* More 64-bit operations */
        long long ll7 = ll6 << 16;
        long long ll8 = ll7 | 0xFFFF0000FFFF0000LL;
        long long ll9 = ll8 - ll6;
        asm volatile("" : : "r"(ll7), "r"(ll8), "r"(ll9));
        
        long long ll10 = ext_func3(ll9);
        
        /* Mix with vector results */
        int vec_elem = vec6[0] + vec6[1] + vec6[2] + vec6[3];
        ll_result += ll10 + vec_elem;
        
        /* Update vectors for next iteration */
        vec1 = vec1 + vec3;
        vec2 = vec2 + vec4;
    }
    
    vol_ll = ll_result;
    return vol_ll;
}

/* Test 4: Complex nested loops with mixed modes */
double test_complex_nested_loops(int limit) {
    double total = 0.0;
    volatile double vol_total = 0.0;
    
    for (int i = 1; i < limit; i++) {
        float f_acc = i * 0.5f;
        double d_acc = i * 1.5;
        long long ll_acc = i * 1000LL;
        
        /* First inner loop */
        for (int j = i * 3; j < limit; j += i + 1) {
            float f1 = j * 0.25f;
            float f2 = f1 * f1 - f1 + 2.0f;
            asm volatile("" : : "r"(f1), "r"(f2));
            
            f_acc += f2;
            
            double d1 = j * 0.125;
            double d2 = d1 * d1 * 3.14159;
            asm volatile("" : : "r"(d1), "r"(d2));
            
            d_acc += d2;
            
            /* Function call creates pressure */
            if (j % 5 == 0) {
                f_acc = ext_func2(f_acc);
            }
        }
        
        /* Second inner loop with different stride */
        for (int k = i * 4; k < limit; k += (i * 2) + 3) {
            long long ll1 = k * 10000LL;
            long long ll2 = ll1 ^ 0xAAAAAAAAAAAAAAAALL;
            long long ll3 = ll2 * 127 + 255;
            asm volatile("" : : "r"(ll1), "r"(ll2), "r"(ll3));
            
            ll_acc += ll3;
            
            double d3 = k * 0.0625;
            double d4 = d3 / (d_acc + 1.0);
            asm volatile("" : : "r"(d3), "r"(d4));
            
            d_acc += d4;
            
            /* Another external call */
            if (k % 11 == 0) {
                ll_acc = ext_func3(ll_acc);
            }
        }
        
        /* Final computations mixing all types */
        float f_final = f_acc * 2.0f;
        double d_final = d_acc * 1.5;
        long long ll_final = ll_acc >> 4;
        
        asm volatile("" : : "r"(f_final), "r"(d_final), "r"(ll_final));
        
        total += f_final + d_final + ll_final;
    }
    
    vol_total = total;
    return vol_total;
}

/* Dummy external functions */
int ext_func1(int x) {
    return x ^ 0x12345678;
}

double ext_func2(double x) {
    return x * 1.61803398875;
}

long long ext_func3(long long x) {
    return x + 0xFEDCBA9876543210LL;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int iterations = argc > 2 ? atoi(argv[2]) : 100;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to trigger different patterns */
    int result1 = test_mixed_integer_pressure(seed);
    printf("Test 1 result: %d\n", result1);
    
    double result2 = test_floating_point_pressure(iterations);
    printf("Test 2 result: %f\n", result2);
    
    long long result3 = test_64bit_vector_pressure(iterations / 2);
    printf("Test 3 result: %lld\n", result3);
    
    double result4 = test_complex_nested_loops(iterations);
    printf("Test 4 result: %f\n", result4);
    
    /* Final volatile store to prevent optimization */
    volatile int final_check = result1 + (int)result2 + (int)result3 + (int)result4;
    printf("Final checksum: %d\n", final_check);
    
    return 0;
}
