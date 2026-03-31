/* test_optabs_10_11_params.c
 * Designed to trigger the 10 and 11 parameter expansion cases in optabs.cc
 * Compile with: gcc -O2 -fdump-rtl-expand test.c -o test
 */

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

/* Prevent excessive optimization */
static volatile int g_volatile_seed = 42;

/* Small structs to pass by value (forcing complex parameter passing) */
struct SmallStruct16 {
    int a;
    int b;
    float c;
    float d;
};

struct SmallStruct24 {
    double x;
    int y;
    char z[8];
};

/* Mixed parameter types for 10-parameter function */
static int func_10_params(int a, float b, double c, int* d, char e, 
                          short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent dead code elimination */
    int result = a + (int)b + (int)c + *d + e + f + (int)g;
    result += (int)((size_t)h % 256) + (int)i + j;
    
    /* Some arithmetic mixing types */
    float temp = b * (float)c + (float)a;
    *d += (int)temp;
    
    return result % 256; /* Keep result bounded */
}

/* Mixed parameter types for 11-parameter function */
static double func_11_params(int a, double b, float c, int* d, char e,
                             short f, long g, void* h, size_t i, 
                             unsigned j, struct SmallStruct16 s) {
    /* Use all parameters */
    double result = (double)a + b + (double)c + (double)*d + (double)e;
    result += (double)f + (double)g + (double)((size_t)h % 256);
    result += (double)i + (double)j + (double)s.a + (double)s.b + s.c + s.d;
    
    /* Modify through pointer */
    *d += s.a + (int)s.c;
    
    /* Return computation involving all parameters */
    return result * 0.5;
}

/* Another 10-parameter function with struct parameter */
static long func_10_params_struct(int a, float b, struct SmallStruct24 s1,
                                  int* d, char e, short f, long g,
                                  void* h, size_t i, unsigned j) {
    long result = a + (long)b + (long)s1.x + s1.y + *d + e + f + g;
    result += (long)((size_t)h) + i + j;
    
    /* Use struct fields */
    for (int k = 0; k < 8; k++) {
        result += s1.z[k];
    }
    
    *d = (int)result;
    return result;
}

/* 11-parameter function with two structs */
static float func_11_params_mixed(struct SmallStruct16 s1, int a, double b,
                                  float c, int* d, char e, short f,
                                  struct SmallStruct24 s2, size_t i,
                                  unsigned j, long g) {
    float result = (float)a + (float)b + c + (float)*d + (float)e + (float)f;
    result += s1.c + s1.d + (float)s2.x + (float)s2.y;
    result += (float)i + (float)j + (float)g;
    
    /* Use struct fields */
    *d += s1.a + s2.y;
    
    return result;
}

/* Function pointer types for obfuscation */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long,
                         void*, size_t, unsigned);

typedef double (*Func11Ptr)(int, double, float, int*, char, short, long,
                            void*, size_t, unsigned, struct SmallStruct16);

/* Array of function pointers to prevent direct call optimization */
static Func10Ptr func10_array[] = {
    (Func10Ptr)func_10_params,
    (Func10Ptr)func_10_params_struct
};

static Func11Ptr func11_array[] = {
    (Func11Ptr)func_11_params,
    (Func11Ptr)func_11_params_mixed
};

/* Helper to create structs with volatile values */
static struct SmallStruct16 make_struct16(void) {
    struct SmallStruct16 s;
    s.a = g_volatile_seed;
    s.b = g_volatile_seed + 1;
    s.c = (float)g_volatile_seed * 0.5f;
    s.d = (float)g_volatile_seed * 0.25f;
    return s;
}

static struct SmallStruct24 make_struct24(void) {
    struct SmallStruct24 s;
    s.x = (double)g_volatile_seed * 0.1;
    s.y = g_volatile_seed * 2;
    for (int i = 0; i < 8; i++) {
        s.z[i] = (char)(g_volatile_seed + i);
    }
    return s;
}

int main(void) {
    int results[4] = {0};
    double double_results[4] = {0.0};
    
    /* Volatile variables to prevent constant propagation */
    volatile int v1 = g_volatile_seed;
    volatile float v2 = (float)g_volatile_seed * 1.5f;
    volatile double v3 = (double)g_volatile_seed * 0.75;
    volatile char v4 = (char)(g_volatile_seed + 10);
    volatile short v5 = (short)(g_volatile_seed + 20);
    volatile long v6 = (long)g_volatile_seed * 3;
    volatile size_t v7 = (size_t)g_volatile_seed * 4;
    volatile unsigned v8 = (unsigned)g_volatile_seed * 5;
    
    int local_var = 100;
    int* ptr_var = &local_var;
    
    /* Create structs */
    struct SmallStruct16 s16 = make_struct16();
    struct SmallStruct24 s24 = make_struct24();
    
    /* Loop to create multiple call sites with varying parameters */
    for (int iteration = 0; iteration < 100; iteration++) {
        /* Vary parameters slightly each iteration */
        int base = g_volatile_seed + iteration;
        
        /* Call 10-parameter functions directly */
        results[0] += func_10_params(
            base, v2, v3, ptr_var, v4 + iteration,
            v5, v6, (void*)(size_t)base, v7, v8
        );
        
        results[1] += (int)func_10_params_struct(
            base + 1, v2 * 2.0f, s24,
            ptr_var, v4, v5 + iteration, v6,
            (void*)(size_t)(base + 2), v7 + 1, v8 + 2
        );
        
        /* Call 11-parameter functions directly */
        double_results[0] += func_11_params(
            base, v3, v2, ptr_var, v4,
            v5, v6, (void*)(size_t)base, v7, v8, s16
        );
        
        /* Modify struct slightly */
        s16.a += iteration;
        s16.c += (float)iteration * 0.1f;
        
        double_results[1] += func_11_params_mixed(
            s16, base + 3, v3 * 0.5, v2 * 0.25f,
            ptr_var, v4 + 1, v5 + 2, s24,
            v7 + 3, v8 + 4, v6 + 5
        );
        
        /* Call through function pointers (prevents some optimizations) */
        if (iteration % 2 == 0) {
            results[2] += func10_array[0](
                base + 10, v2, v3, ptr_var, v4,
                v5, v6, (void*)(size_t)base, v7, v8
            );
            
            double_results[2] += func11_array[0](
                base + 20, v3, v2, ptr_var, v4,
                v5, v6, (void*)(size_t)base, v7, v8, s16
            );
        } else {
            results[3] += (int)func10_array[1](
                base + 30, v2 * 3.0f, s24,
                ptr_var, v4 + 2, v5 + 3, v6 + 4,
                (void*)(size_t)(base + 40), v7 + 5, v8 + 6
            );
            
            /* Modify struct for next call */
            s24.y += iteration;
            s24.x += (double)iteration * 0.01;
            
            double_results[3] += func11_array[1](
                s16, base + 50, v3 * 2.0, v2 * 1.5f,
                ptr_var, v4 + 3, v5 + 4, s24,
                v7 + 6, v8 + 7, v6 + 8
            );
        }
        
        /* Prevent loop unrolling from simplifying everything */
        if (iteration % 10 == 0) {
            g_volatile_seed = (g_volatile_seed * 1103515245 + 12345) & 0x7fffffff;
        }
    }
    
    /* Use results to prevent dead code elimination */
    int final_result = 0;
    for (int i = 0; i < 4; i++) {
        final_result += results[i];
        final_result += (int)double_results[i];
    }
    
    printf("Result: %d (seed: %d)\n", final_result, g_volatile_seed);
    return final_result != 0 ? 0 : 1;
}
