#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Small structs to pass by value */
struct Data16 {
    int a;
    int b;
    float c;
    float d;
};

struct Data24 {
    double x;
    int y;
    short z;
    char w[6];
};

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_int = 42;
volatile float g_volatile_float = 3.14f;
volatile double g_volatile_double = 2.71828;

/* Function with exactly 10 parameters - mixed types */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent dead code elimination */
    int sum = a + (int)b + (int)c + *d + e + f + (int)g;
    sum += (int)((uintptr_t)h % 256) + (int)i + j;
    
    /* Mix operations to create complex RTL */
    float fsum = b * (float)c + (float)sum;
    double dsum = c * (double)fsum / (a + 1);
    
    /* Return value depends on all parameters */
    return (int)(dsum + fsum + sum) & 0xFF;
}

/* Function with exactly 11 parameters - includes struct by value */
static inline double func11(struct Data16 s1, int a, double b, float c, 
                           int* d, char e, short f, void* h, size_t i, 
                           unsigned j, long k) {
    /* Use all parameters including struct members */
    double result = s1.a + s1.b + s1.c + s1.d;
    result += a + b + c + *d + e + f + (double)((uintptr_t)h) + i + j + k;
    
    /* Complex arithmetic mixing types */
    result = result * b / (c + 1.0f);
    result += (s1.a * s1.c) - (s1.b * s1.d);
    
    /* Prevent optimization with volatile access */
    result += g_volatile_double;
    
    return result;
}

/* Another 10-parameter function with struct parameter */
static inline long func10_struct(struct Data24 s, int a, float b, 
                                double c, int* d, char e, short f, 
                                void* h, size_t i, unsigned j) {
    long result = (long)s.x + s.y + s.z;
    for (int idx = 0; idx < 6; idx++) {
        result += s.w[idx];
    }
    
    result += a + (long)b + (long)c + *d + e + f;
    result += (long)((uintptr_t)h) + i + j;
    
    /* Mix with volatile to prevent optimization */
    result *= g_volatile_int;
    
    return result;
}

/* 11-parameter function with pointer-heavy signature */
static inline float func11_pointers(int* p1, float* p2, double* p3, 
                                   char* p4, short* p5, long* p6,
                                   void** p7, size_t* p8, unsigned* p9,
                                   struct Data16* p10, int* p11) {
    float result = *p1 + *p2 + (float)*p3;
    result += *p4 + *p5 + (float)*p6;
    result += (float)((uintptr_t)*p7) + *p8 + *p9;
    result += p10->a + p10->b + p10->c + p10->d;
    result += *p11;
    
    /* Complex pointer arithmetic */
    result += *(p1 + (*p4 % 4)) * 0.5f;
    result -= *(p2) * *(p3);
    
    return result;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, 
                        long, void*, size_t, unsigned);
                        
typedef double (*Func11Ptr)(struct Data16, int, double, float, 
                           int*, char, short, void*, size_t, 
                           unsigned, long);

/* Array of function pointers */
static Func10Ptr func10_array[] = {
    (Func10Ptr)func10,
    (Func10Ptr)func10_struct
};

static Func11Ptr func11_array[] = {
    func11,
    (Func11Ptr)func11_pointers
};

/* Helper to create test data */
static struct Data16 make_data16(int seed) {
    struct Data16 d;
    d.a = seed;
    d.b = seed * 2;
    d.c = seed * 0.5f;
    d.d = seed * 0.25f;
    return d;
}

static struct Data24 make_data24(int seed) {
    struct Data24 d;
    d.x = seed * 1.5;
    d.y = seed * 3;
    d.z = (short)(seed % 1000);
    for (int i = 0; i < 6; i++) {
        d.w[i] = (char)((seed + i) % 128);
    }
    return d;
}

int main() {
    int int_array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    float float_var = 1.5f;
    double double_var = 2.5;
    char char_var = 'A';
    short short_var = 100;
    long long_var = 1000L;
    void* ptr_var = &int_array;
    size_t size_var = sizeof(int_array);
    unsigned uint_var = 12345;
    
    struct Data16 s1 = make_data16(10);
    struct Data24 s2 = make_data24(20);
    
    int result1, result2;
    double result3, result4;
    long result5;
    float result6;
    
    /* Call functions directly in a loop - forces multiple expansions */
    for (int i = 0; i < 100; i++) {
        /* Vary arguments to prevent constant folding */
        int idx = i % 10;
        
        /* Call 10-parameter function */
        result1 = func10(
            int_array[idx] + g_volatile_int,
            float_var + i * 0.1f,
            double_var + i * 0.01,
            &int_array[idx],
            char_var + (i % 26),
            short_var + i,
            long_var + i * 100L,
            (void*)((uintptr_t)ptr_var + i),
            size_var + i,
            uint_var + i
        );
        
        /* Call 11-parameter function with struct by value */
        s1.a += i;
        result3 = func11(
            s1,
            int_array[idx],
            double_var + i * 0.5,
            float_var + i * 0.2f,
            &int_array[(idx + 1) % 10],
            char_var + ((i + 1) % 26),
            short_var + i * 2,
            (void*)((uintptr_t)ptr_var + i * 16),
            size_var + i * 2,
            uint_var + i * 3,
            long_var + i * 50L
        );
        
        /* Call through function pointer array */
        if (i % 2 == 0) {
            result5 = ((long (*)(struct Data24, int, float, double, int*, 
                               char, short, void*, size_t, unsigned))func10_array[1])(
                s2,
                int_array[idx],
                float_var,
                double_var,
                &int_array[idx],
                char_var,
                short_var,
                ptr_var,
                size_var,
                uint_var
            );
        }
        
        /* Another 11-parameter variant */
        if (i % 3 == 0) {
            float f1 = g_volatile_float + i;
            double d1 = g_volatile_double + i * 0.1;
            unsigned u1 = uint_var + i * 7;
            
            result4 = func11(
                make_data16(i),
                int_array[(idx + 2) % 10],
                d1,
                f1,
                &int_array[(idx + 3) % 10],
                char_var + ((i + 2) % 26),
                short_var + i * 3,
                (void*)((uintptr_t)ptr_var + i * 32),
                size_var + i * 4,
                u1,
                long_var + i * 75L
            );
        }
    }
    
    /* Test pointer-heavy 11-parameter function */
    int x = 42;
    float y = 3.14f;
    double z = 2.71828;
    char c = 'X';
    short s = 256;
    long l = 1024L;
    void* p = &x;
    size_t sz = 100;
    unsigned u = 999;
    struct Data16 sd = make_data16(30);
    int w = 777;
    
    result6 = func11_pointers(
        &x, &y, &z, &c, &s, &l, &p, &sz, &u, &sd, &w
    );
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %f, %ld, %f, %f\n", 
           result1, result3, result5, result4, result6);
    
    return 0;
}
