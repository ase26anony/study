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

/* Test 1: High register pressure with integer operations */
int test1_high_pressure_int(int seed) {
    volatile int result = 0;
    int a = seed * 3 + 1;
    int b = seed * 5 - 2;
    int c = seed * 7 + 3;
    int d = seed * 11 - 4;
    int e = seed * 13 + 5;
    int f = seed * 17 - 6;
    int g = seed * 19 + 7;
    int h = seed * 23 - 8;
    
    /* Long sequence of independent computations */
    int t1 = a * b + c;
    asm volatile("" : : "r"(t1));
    int t2 = d & e | f;
    asm volatile("" : : "r"(t2));
    int t3 = g ^ h << 2;
    asm volatile("" : : "r"(t3));
    int t4 = t1 - t2 * t3;
    asm volatile("" : : "r"(t4));
    
    /* Function call creates pressure point */
    int t5 = ext_func1(t4);
    
    int t6 = t5 * 31 + a;
    asm volatile("" : : "r"(t6));
    int t7 = b / 7 + t6;
    asm volatile("" : : "r"(t7));
    int t8 = c % 13 ^ t7;
    asm volatile("" : : "r"(t8));
    int t9 = d * 2 - t8;
    asm volatile("" : : "r"(t9));
    
    /* Another function call */
    int t10 = ext_func2(t9, seed);
    
    int t11 = e + t10 * 3;
    asm volatile("" : : "r"(t11));
    int t12 = f - t11 / 5;
    asm volatile("" : : "r"(t12));
    int t13 = g | t12 << 1;
    asm volatile("" : : "r"(t13));
    int t14 = h & t13 >> 2;
    asm volatile("" : : "r"(t14));
    
    result = t1 + t2 - t3 + t4 - t5 + t6 + t7 - t8 + t9 + t10 - t11 + t12 - t13 + t14;
    use(&result);
    return result;
}

/* Test 2: Mixed integer and floating-point operations */
double test2_mixed_types(int seed) {
    volatile double result = 0.0;
    double da = seed * 1.5;
    double db = seed * 2.7;
    float fa = seed * 0.3f;
    float fb = seed * 0.7f;
    int ia = seed * 11;
    int ib = seed * 13;
    
    /* Mixed computations */
    double t1 = da * db + 3.14159;
    asm volatile("" : : "r"(t1));
    float t2 = fa / fb - 2.71828f;
    asm volatile("" : : "r"(t2));
    int t3 = ia & ib | 0xFF;
    asm volatile("" : : "r"(t3));
    
    /* Function call with floating point */
    double t4 = ext_func3(t1);
    
    double t5 = t4 * 2.0 + da;
    asm volatile("" : : "r"(t5));
    float t6 = (float)t5 * fa;
    asm volatile("" : : "r"(t6));
    int t7 = t3 ^ (int)t6;
    asm volatile("" : : "r"(t7));
    
    /* More mixed operations */
    long long t8 = (long long)seed * 1000000007LL;
    asm volatile("" : : "r"(t8));
    double t9 = (double)t8 / 1000.0;
    asm volatile("" : : "r"(t9));
    
    /* External call with long long */
    long long t10 = ext_func4(t8);
    
    result = t1 + t2 + t4 + t5 + t6 + t7 + t9 + (double)t10;
    use(&result);
    return result;
}

/* Test 3: Nested loops with complex induction variables */
int test3_nested_loops(int N) {
    volatile int total = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable for inner loop */
        for (int j = i * 2; j < N; j += 3) {
            /* High pressure computation in loop body */
            int a = i * 3 + 1;
            int b = j * 5 - 2;
            int c = a * b + i;
            asm volatile("" : : "r"(c));
            
            int d = c & 0xFF | j;
            asm volatile("" : : "r"(d));
            
            /* Function call inside loop */
            int e = ext_func1(d);
            
            int f = e * 7 + a;
            asm volatile("" : : "r"(f));
            int g = b / 3 ^ f;
            asm volatile("" : : "r"(g));
            
            total += c + d - e + f - g;
            
            /* Additional pressure with floating point */
            if (j % 5 == 0) {
                double h = (double)j * 1.234;
                asm volatile("" : : "r"(h));
                total += (int)h;
            }
        }
        
        /* Pressure point between outer loop iterations */
        if (i % 7 == 0) {
            int temp = ext_func2(i, total);
            total ^= temp;
        }
    }
    
    use(&total);
    return total;
}

/* Test 4: Vector-like operations using structs */
typedef struct {
    int x, y, z, w;
} Vec4;

int test4_vector_ops(int seed) {
    Vec4 v1 = {seed, seed*2, seed*3, seed*4};
    Vec4 v2 = {seed*5, seed*6, seed*7, seed*8};
    volatile int result = 0;
    
    /* Simulate vector operations */
    int t1 = v1.x * v2.x + v1.y;
    asm volatile("" : : "r"(t1));
    int t2 = v1.z & v2.z | v1.w;
    asm volatile("" : : "r"(t2));
    int t3 = v2.y ^ v1.x << 1;
    asm volatile("" : : "r"(t3));
    int t4 = v2.w / 3 + t1;
    asm volatile("" : : "r"(t4));
    
    /* Chain of dependent computations */
    for (int i = 0; i < 4; i++) {
        int temp = t1 + i * t2;
        asm volatile("" : : "r"(temp));
        temp = temp * 3 - t3;
        asm volatile("" : : "r"(temp));
        temp = temp & 0xFF | t4;
        asm volatile("" : : "r"(temp));
        
        /* External call in loop */
        temp = ext_func1(temp);
        
        result += temp;
        
        /* Floating point in vector context */
        double ftemp = (double)temp * 1.5;
        asm volatile("" : : "r"(ftemp));
        result += (int)ftemp;
    }
    
    use(&result);
    return result;
}

/* Test 5: Extreme register pressure with many live values */
long long test5_extreme_pressure(int seed) {
    /* Declare many variables to increase pressure */
    int v1 = seed * 1, v2 = seed * 2, v3 = seed * 3, v4 = seed * 4;
    int v5 = seed * 5, v6 = seed * 6, v7 = seed * 7, v8 = seed * 8;
    int v9 = seed * 9, v10 = seed * 10, v11 = seed * 11, v12 = seed * 12;
    int v13 = seed * 13, v14 = seed * 14, v15 = seed * 15, v16 = seed * 16;
    
    volatile long long total = 0;
    
    /* Unrolled computation sequence */
    #define COMPUTE(i, a, b, c) \
        do { \
            int t##i = a * b + c; \
            asm volatile("" : : "r"(t##i)); \
            total += t##i; \
        } while(0)
    
    COMPUTE(1, v1, v2, v3);
    COMPUTE(2, v4, v5, v6);
    COMPUTE(3, v7, v8, v9);
    COMPUTE(4, v10, v11, v12);
    COMPUTE(5, v13, v14, v15);
    
    /* Function call in middle of sequence */
    int mid = ext_func1(v16);
    
    COMPUTE(6, mid, v1, v2);
    COMPUTE(7, v3, v4, v5);
    COMPUTE(8, v6, v7, v8);
    COMPUTE(9, v9, v10, v11);
    COMPUTE(10, v12, v13, v14);
    
    /* More mixed types */
    double d1 = (double)v1 * 1.1;
    double d2 = (double)v2 * 2.2;
    asm volatile("" : : "r"(d1), "r"(d2));
    total += (long long)(d1 + d2);
    
    long long ll1 = (long long)v3 * 1000000009LL;
    long long ll2 = (long long)v4 * 1000000007LL;
    asm volatile("" : : "r"(ll1), "r"(ll2));
    total += ll1 - ll2;
    
    #undef COMPUTE
    
    use(&total);
    return total;
}

/* Dummy external functions */
int ext_func1(int x) { return x ^ 0x55AA55AA; }
int ext_func2(int x, int y) { return (x * y) & 0xFFFFFFFF; }
double ext_func3(double x) { return x * 2.718281828; }
long long ext_func4(long long x) { return x * 6364136223846793005LL; }

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    volatile int checksum = 0;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to exercise different patterns */
    checksum += test1_high_pressure_int(seed);
    checksum += (int)test2_mixed_types(seed);
    checksum += test3_nested_loops(seed % 50 + 10);
    checksum += test4_vector_ops(seed);
    checksum += (int)test5_extreme_pressure(seed);
    
    /* Use checksum to prevent dead code elimination */
    volatile int *volatile ptr = &checksum;
    asm volatile("" : : "r"(ptr) : "memory");
    
    printf("Final checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
