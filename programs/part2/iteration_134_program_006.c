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
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Test 1: High register pressure with integer operations */
int test1_high_pressure_int(int seed) {
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0x1234;
    int d = seed * seed;
    int e = seed / 2;
    int f = seed | 0xABCD;
    
    /* Force values into registers */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f));
    
    /* Long sequence of independent computations */
    int t1 = a * b + c;
    int t2 = d & e | f;
    int t3 = t1 ^ t2;
    int t4 = t3 * 7 - 13;
    int t5 = t4 >> 3;
    int t6 = t5 & 0xFF;
    int t7 = t6 * t1 + t2;
    int t8 = t7 | t3;
    int t9 = t8 ^ t4;
    int t10 = t9 * 31;
    
    /* Function call to clobber registers */
    int r1 = ext_func1(t10);
    
    /* More computations with different patterns */
    int t11 = r1 * a + b;
    int t12 = c * d - e;
    int t13 = f ^ t11;
    int t14 = t12 & t13;
    int t15 = t14 | r1;
    int t16 = t15 * 17;
    int t17 = t16 + t10;
    int t18 = t17 ^ t11;
    int t19 = t18 * 3;
    int t20 = t19 - t12;
    
    /* Another function call */
    int r2 = ext_func2(t20, t13);
    
    /* Final chain of computations */
    int result = r2;
    for (int i = 0; i < 8; i++) {
        result = result * 1103515245 + 12345;
        result = (result >> 16) & 0x7FFF;
        asm volatile("" : "+r"(result));
    }
    
    return result;
}

/* Test 2: Mixed integer and floating point operations */
double test2_mixed_types(int seed) {
    double d1 = seed * 1.5;
    double d2 = seed / 3.14159;
    float f1 = seed * 0.25f;
    float f2 = seed + 1.618f;
    
    int i1 = seed * 3;
    int i2 = seed + 11;
    long long ll1 = seed * 1000000LL;
    long long ll2 = seed * 2000000LL;
    
    /* Force all values into registers */
    asm volatile("" : : "r"(d1), "r"(d2), "r"(f1), "r"(f2), 
                  "r"(i1), "r"(i2), "r"(ll1), "r"(ll2));
    
    /* Mixed computations */
    double t1 = d1 * d2 + f1;
    int t2 = i1 * i2 + (int)f2;
    long long t3 = ll1 * ll2 / 1000;
    double t4 = t1 * t2 + t3;
    
    /* Function call with double */
    double r1 = ext_func3(t4);
    
    /* More mixed operations */
    float t5 = (float)r1 * f1 + f2;
    int t6 = t2 ^ (int)t5;
    double t6_d = t6;
    double t7 = t6_d * d1 - d2;
    long long t8 = t3 + (long long)t7;
    
    /* Another function call */
    long long r2 = ext_func4(t8);
    
    /* Complex expression with many temporaries */
    double result = r1;
    for (int i = 0; i < 4; i++) {
        double tmp1 = result * 1.1;
        float tmp2 = (float)tmp1 * 0.9f;
        int tmp3 = (int)tmp2 * i;
        long long tmp4 = r2 * (i + 1);
        double tmp5 = tmp3 + tmp4;
        result = result + tmp5 * 0.5;
        asm volatile("" : "+r"(result));
    }
    
    return result;
}

/* Test 3: Nested loops with complex induction variables */
int test3_nested_loops(int N) {
    int sum = 0;
    
    /* Outer loop with multiple induction variables */
    for (int i = 0; i < N; i++) {
        int i_sq = i * i;
        int i_cubed = i_sq * i;
        int i_mod = i % 17;
        
        /* Inner loop with derived induction variable */
        for (int j = i * 2; j < N; j += 3) {
            int j_shift = j << 2;
            int j_masked = j & 0xFF;
            
            /* Complex computation in inner loop */
            int t1 = i_sq * j_shift;
            int t2 = i_cubed + j_masked;
            int t3 = t1 ^ t2;
            int t4 = t3 * i_mod;
            int t5 = j * 7 - 13;
            int t6 = t4 & t5;
            int t7 = t6 | i;
            int t8 = t7 * 31;
            int t9 = t8 + j;
            int t10 = t9 ^ t4;
            
            /* Function call inside inner loop */
            int r = ext_func1(t10);
            
            sum += r;
            
            /* Force register pressure */
            asm volatile("" : : "r"(i_sq), "r"(i_cubed), "r"(i_mod),
                          "r"(j_shift), "r"(j_masked), "r"(t1), "r"(t2),
                          "r"(t3), "r"(t4), "r"(t5), "r"(t6), "r"(t7),
                          "r"(t8), "r"(t9), "r"(t10), "r"(r));
        }
        
        /* Additional computation between loops */
        int between = i * 137;
        sum += between;
        asm volatile("" : "+r"(sum));
    }
    
    return sum;
}

/* Test 4: Vector-like operations using structs */
typedef struct {
    int x, y, z, w;
} Vec4;

int test4_vector_ops(int seed) {
    Vec4 v1 = {seed, seed + 1, seed * 2, seed / 2};
    Vec4 v2 = {seed * 3, seed + 7, seed ^ 0xFF, seed | 0xAA};
    
    /* Force struct components into registers */
    asm volatile("" : : "r"(v1.x), "r"(v1.y), "r"(v1.z), "r"(v1.w),
                      "r"(v2.x), "r"(v2.y), "r"(v2.z), "r"(v2.w));
    
    /* Multiple parallel computations */
    int t1 = v1.x * v2.x + v1.y;
    int t2 = v1.z * v2.z - v1.w;
    int t3 = v2.y & v2.w | v1.x;
    int t4 = t1 ^ t2 ^ t3;
    int t5 = t4 * 7;
    int t6 = t5 + v1.z;
    int t7 = t6 & 0xFFFF;
    int t8 = t7 * v2.x;
    int t9 = t8 | v1.w;
    int t10 = t9 ^ v2.y;
    
    /* Chain of dependent computations */
    int result = t10;
    for (int i = 0; i < 16; i++) {
        int tmp1 = result * 3;
        int tmp2 = tmp1 + i;
        int tmp3 = tmp2 ^ result;
        int tmp4 = tmp3 * 5;
        int tmp5 = tmp4 >> 1;
        int tmp6 = tmp5 & 0x7F;
        int tmp7 = tmp6 + v1.x;
        int tmp8 = tmp7 * v1.y;
        int tmp9 = tmp8 - v1.z;
        int tmp10 = tmp9 | v1.w;
        
        result = tmp10;
        
        /* Prevent optimization */
        if (i % 4 == 0) {
            int r = ext_func1(result);
            result += r;
        }
        
        asm volatile("" : "+r"(result));
    }
    
    return result;
}

/* Test 5: Extreme register pressure with many live values */
long long test5_extreme_pressure(int seed) {
    /* Declare many variables to increase register pressure */
    int v1 = seed * 1;
    int v2 = seed * 2;
    int v3 = seed * 3;
    int v4 = seed * 4;
    int v5 = seed * 5;
    int v6 = seed * 6;
    int v7 = seed * 7;
    int v8 = seed * 8;
    int v9 = seed * 9;
    int v10 = seed * 10;
    int v11 = seed * 11;
    int v12 = seed * 12;
    int v13 = seed * 13;
    int v14 = seed * 14;
    int v15 = seed * 15;
    int v16 = seed * 16;
    
    /* Force all into registers */
    asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4),
                      "r"(v5), "r"(v6), "r"(v7), "r"(v8),
                      "r"(v9), "r"(v10), "r"(v11), "r"(v12),
                      "r"(v13), "r"(v14), "r"(v15), "r"(v16));
    
    /* Massive computation graph */
    long long acc = 0;
    for (int i = 0; i < 32; i++) {
        int t1 = v1 * v2 + i;
        int t2 = v3 * v4 - i;
        int t3 = v5 & v6 | i;
        int t4 = v7 ^ v8 ^ i;
        int t5 = v9 * v10 * i;
        int t6 = v11 + v12 + i;
        int t7 = v13 - v14 - i;
        int t8 = v15 | v16 | i;
        
        int r1 = ext_func1(t1);
        int r2 = ext_func2(t2, t3);
        
        int t9 = r1 * r2 + t4;
        int t10 = t5 & t6 | t7;
        int t11 = t8 ^ t9 ^ t10;
        
        acc += t11;
        
        /* Rotate values to create complex live ranges */
        int tmp = v1;
        v1 = v2; v2 = v3; v3 = v4; v4 = v5;
        v5 = v6; v6 = v7; v7 = v8; v8 = v9;
        v9 = v10; v10 = v11; v11 = v12; v12 = v13;
        v13 = v14; v14 = v15; v15 = v16; v16 = tmp;
        
        asm volatile("" : "+r"(acc));
    }
    
    return acc;
}

/* Dummy external functions */
int ext_func1(int x) { return x ^ 0x55AA; }
int ext_func2(int x, int y) { return x * y + 1; }
double ext_func3(double x) { return x * 1.2345; }
long long ext_func4(long long x) { return x * 3LL; }

int main(int argc, char **argv) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    
    printf("Testing early rematerialization...\n");
    
    /* Run all tests to increase coverage chances */
    int r1 = test1_high_pressure_int(seed);
    printf("Test1 result: %d\n", r1);
    
    double r2 = test2_mixed_types(seed);
    printf("Test2 result: %f\n", r2);
    
    int r3 = test3_nested_loops(seed % 50 + 10);
    printf("Test3 result: %d\n", r3);
    
    int r4 = test4_vector_ops(seed);
    printf("Test4 result: %d\n", r4);
    
    long long r5 = test5_extreme_pressure(seed);
    printf("Test5 result: %lld\n", r5);
    
    /* Use results to prevent optimization */
    volatile int dummy = r1 + (int)r2 + r3 + r4 + (int)r5;
    
    return 0;
}
