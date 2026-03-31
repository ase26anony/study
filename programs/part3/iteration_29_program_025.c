#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

/* Small structs to pass by value - forces complex parameter passing */
struct Data16 {
    int a;
    int b;
    float c;
    float d;
};

struct Data24 {
    double x;
    double y;
    int z;
};

/* Mixed volatile variables to prevent constant propagation */
volatile int v1 = 1;
volatile float v2 = 2.0f;
volatile double v3 = 3.0;
volatile char v4 = 'A';

/* Function with exactly 10 parameters - mixed types */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent dead code elimination */
    int sum = a + (int)b + (int)c + *d + e + f + (int)g;
    sum += (int)((size_t)h % 256) + (int)i + j;
    
    /* Complex arithmetic mixing types */
    float fsum = b * (float)c + (float)a;
    double dsum = c * (double)b + (double)*d;
    
    /* Return value depends on all parameters */
    return sum + (int)fsum + (int)dsum;
}

/* Function with exactly 11 parameters - includes struct by value */
static inline double func11(int a, struct Data16 s1, float b, double c, 
                           int* d, char e, short f, struct Data24 s2, 
                           size_t i, unsigned j, long k) {
    /* Use all parameters including structs */
    double total = (double)a + s1.a + s1.b + s1.c + s1.d;
    total += b + c + *d + e + f;
    total += s2.x + s2.y + s2.z + i + j + k;
    
    /* Complex operations to ensure all params are used */
    total += (s1.c * s2.x) / (b + 1.0f);
    total += ((double)s1.d * s2.y) / (c + 1.0);
    
    return total;
}

/* Another 10-parameter function with different signature */
static inline long func10_alt(float a, double b, int c, short d, long e,
                             void* f, size_t g, unsigned h, int i, float j) {
    long result = (long)(a * 100) + (long)(b * 1000);
    result += c * d * e;
    result += (long)((size_t)f % 1000) + g + h + i + (long)(j * 10);
    
    /* Branch to prevent over-optimization */
    if (result > 1000000) {
        return result / 2;
    }
    return result * 3;
}

/* Another 11-parameter function */
static inline float func11_alt(struct Data24 s1, int a, float b, double c,
                              int* d, char e, short f, long g, void* h,
                              size_t i, unsigned j) {
    float base = s1.x + s1.y + s1.z + a + b;
    base += (float)c + *d + e + f + g;
    base += (float)((size_t)h % 100) + i + j;
    
    /* Complex floating point operations */
    for (int idx = 0; idx < 3; idx++) {
        base += (s1.x * idx) / (b + 1.0f);
    }
    
    return base;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(int, struct Data16, float, double, int*, char, short, struct Data24, size_t, unsigned, long);

/* Array of function pointers */
Func10Ptr func10_array[] = {func10, (Func10Ptr)func10_alt};
Func11Ptr func11_array[] = {func11, (Func11Ptr)func11_alt};

/* Helper to generate test data */
struct Data16 make_data16(int seed) {
    struct Data16 d;
    d.a = seed;
    d.b = seed * 2;
    d.c = seed * 3.0f;
    d.d = seed * 4.0f;
    return d;
}

struct Data24 make_data24(int seed) {
    struct Data24 d;
    d.x = seed * 1.5;
    d.y = seed * 2.5;
    d.z = seed * 3;
    return d;
}

int main(int argc, char** argv) {
    int int_val = v1;
    float float_val = v2;
    double double_val = v3;
    char char_val = v4;
    short short_val = 100;
    long long_val = 1000L;
    size_t size_val = 1024;
    unsigned uint_val = 2048;
    
    int local_int = 42;
    int* int_ptr = &local_int;
    void* void_ptr = (void*)&local_int;
    
    /* Initialize structs */
    struct Data16 s1 = make_data16(int_val);
    struct Data24 s2 = make_data24(int_val + 1);
    
    /* Array to store results */
    int results_10[10] = {0};
    double results_11[10] = {0.0};
    
    /* Loop calling 10-parameter functions */
    for (int i = 0; i < 10; i++) {
        /* Vary arguments to prevent optimization */
        int_val += i;
        float_val += i * 0.5f;
        double_val += i * 0.25;
        
        /* Call direct */
        results_10[i] = func10(
            int_val, float_val, double_val, int_ptr,
            char_val + i, short_val + i, long_val + i,
            void_ptr, size_val + i, uint_val + i
        );
        
        /* Call through function pointer with volatile guard */
        if (v1 > 0) {
            results_10[i] += func10_array[i % 2](
                int_val + 1, float_val + 1.0f, double_val + 1.0,
                int_ptr, char_val + i + 1, short_val + i + 1,
                long_val + i + 1, void_ptr, size_val + i + 1,
                uint_val + i + 1
            );
        }
    }
    
    /* Loop calling 11-parameter functions */
    for (int i = 0; i < 10; i++) {
        /* Update structs */
        s1 = make_data16(int_val + i);
        s2 = make_data24(int_val + i + 2);
        
        /* Call direct */
        results_11[i] = func11(
            int_val + i, s1, float_val + i * 0.3f,
            double_val + i * 0.2, int_ptr, char_val + i,
            short_val + i, s2, size_val + i * 2,
            uint_val + i * 3, long_val + i * 4
        );
        
        /* Call through function pointer */
        if (v2 > 0.0f) {
            struct Data24 s3 = make_data24(int_val + i * 3);
            results_11[i] += func11_array[i % 2](
                int_val + i * 2, s1, float_val + i * 0.4f,
                double_val + i * 0.3, int_ptr, char_val + i + 2,
                short_val + i + 2, s3, size_val + i * 3,
                uint_val + i * 4, long_val + i * 5
            );
        }
    }
    
    /* Use results to prevent dead code elimination */
    int final_sum = 0;
    double final_double_sum = 0.0;
    
    for (int i = 0; i < 10; i++) {
        final_sum += results_10[i];
        final_double_sum += results_11[i];
    }
    
    printf("Results: %d, %f\n", final_sum, final_double_sum);
    
    /* Additional complex calling pattern */
    if (argc > 1) {
        /* Nested calls with mixed parameters */
        int nested_result = func10(
            func10(1, 2.0f, 3.0, int_ptr, 'a', 10, 100L, void_ptr, 1000, 2000),
            float_val, double_val, int_ptr, char_val, short_val,
            func10_alt(1.0f, 2.0, 3, 4, 5L, void_ptr, 6, 7, 8, 9.0f),
            void_ptr, size_val, uint_val
        );
        printf("Nested: %d\n", nested_result);
    }
    
    return final_sum > 1000 ? 0 : 1;
}
