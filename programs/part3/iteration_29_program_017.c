#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Small structs to pass by value - forces complex parameter passing */
struct SmallStruct {
    int x;
    int y;
    float z;
    char w;
};

struct MediumStruct {
    double a;
    int b;
    short c;
    char d;
    float e;
};

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_int = 42;
volatile float g_volatile_float = 3.14f;
volatile double g_volatile_double = 2.71828;

/* Function with exactly 10 parameters - mixed types */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent dead code elimination */
    int result = a + (int)b + (int)c + *d + e + f + (int)g;
    result += (int)((size_t)h % 256) + (int)i + j;
    
    /* Mix operations to create complex RTL */
    *d = result * 2;
    return result ^ (a * b) | (g & i);
}

/* Another 10-parameter function with struct parameter */
static inline double func10_struct(int a, struct SmallStruct s, double c, 
                                   int* d, char e, short f, long g, 
                                   void* h, size_t i, unsigned j) {
    /* Use struct members */
    double result = a + s.x + s.y + s.z + c + *d + e + f + g;
    result += (double)((size_t)h % 256) + i + j + s.w;
    
    *d = (int)result;
    return result * s.z / c;
}

/* Function with exactly 11 parameters */
static inline long func11(int a, float b, double c, int* d, char e,
                          short f, long g, void* h, size_t i, unsigned j,
                          int k) {
    /* Complex usage of all 11 parameters */
    long result = a * k + (long)b + (long)c + *d * k;
    result += e * f * g + (long)((size_t)h) + i * j;
    
    /* Store back through pointer */
    *d = (int)(result % 2147483647);
    return result ^ (g << 3) | (i & j);
}

/* 11-parameter function with two structs */
static inline float func11_structs(int a, struct SmallStruct s1, 
                                   struct MediumStruct s2, int* d, char e,
                                   short f, long g, void* h, size_t i, 
                                   unsigned j, int k) {
    /* Use both structs extensively */
    float result = a + s1.x + s1.y + s1.z + s1.w;
    result += s2.a + s2.b + s2.c + s2.d + s2.e;
    result += *d + e + f + g + (float)i + j + k;
    
    /* Complex computation */
    *d = (int)(result * s1.z / s2.e);
    return result * s2.a - s1.y + k;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func10StructPtr)(int, struct SmallStruct, double, int*, char, short, long, void*, size_t, unsigned);
typedef long (*Func11Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned, int);
typedef float (*Func11StructsPtr)(int, struct SmallStruct, struct MediumStruct, int*, char, short, long, void*, size_t, unsigned, int);

/* Array of function pointers to obfuscate calls */
static Func10Ptr func10_array[] = {
    (Func10Ptr)func10,
    (Func10Ptr)func10_struct
};

static Func11Ptr func11_array[] = {
    (Func11Ptr)func11,
    (Func11Ptr)func11_structs
};

/* Helper to create test structs */
static struct SmallStruct make_small_struct(int base) {
    struct SmallStruct s;
    s.x = base;
    s.y = base * 2;
    s.z = (float)base / 3.0f;
    s.w = (char)(base % 128);
    return s;
}

static struct MediumStruct make_medium_struct(int base) {
    struct MediumStruct s;
    s.a = (double)base * 1.5;
    s.b = base + 100;
    s.c = (short)(base % 32767);
    s.d = (char)(base % 64);
    s.e = (float)base / 7.0f;
    return s;
}

int main() {
    int results[10] = {0};
    double double_results[10] = {0.0};
    long long_results[10] = {0};
    float float_results[10] = {0.0f};
    
    /* Local volatile to prevent optimization */
    volatile int local_volatile = 7;
    
    /* Loop to create multiple call sites with varying arguments */
    for (int iter = 0; iter < 100; iter++) {
        int base = iter + g_volatile_int + local_volatile;
        
        /* Create structs */
        struct SmallStruct s1 = make_small_struct(base);
        struct MediumStruct s2 = make_medium_struct(base * 3);
        
        /* Call 10-parameter functions directly */
        int idx = iter % 10;
        int temp = base;
        
        /* Direct call to func10 */
        results[idx] += func10(
            base, 
            g_volatile_float + iter,
            g_volatile_double - iter,
            &temp,
            (char)(base % 256),
            (short)(base % 32767),
            (long)base * 3,
            (void*)(size_t)(base * 5),
            (size_t)base * 7,
            (unsigned)base * 11
        );
        
        /* Direct call to func10_struct */
        double_results[idx] += func10_struct(
            base * 2,
            s1,
            g_volatile_double * 2,
            &temp,
            (char)((base + 1) % 256),
            (short)((base + 2) % 32767),
            (long)base * 7,
            (void*)(size_t)(base * 11),
            (size_t)base * 13,
            (unsigned)base * 17
        );
        
        /* Direct call to func11 */
        long_results[idx] += func11(
            base * 3,
            g_volatile_float * 3,
            g_volatile_double * 3,
            &temp,
            (char)((base + 3) % 256),
            (short)((base + 4) % 32767),
            (long)base * 11,
            (void*)(size_t)(base * 17),
            (size_t)base * 19,
            (unsigned)base * 23,
            base * 5  /* 11th parameter */
        );
        
        /* Direct call to func11_structs */
        float_results[idx] += func11_structs(
            base * 4,
            s1,
            s2,
            &temp,
            (char)((base + 5) % 256),
            (short)((base + 6) % 32767),
            (long)base * 13,
            (void*)(size_t)(base * 19),
            (size_t)base * 23,
            (unsigned)base * 29,
            base * 7  /* 11th parameter */
        );
        
        /* Call through function pointers (prevents inlining in some cases) */
        if (iter % 3 == 0) {
            results[idx] += func10_array[iter % 2](
                base * 5,
                g_volatile_float * 5,
                g_volatile_double * 5,
                &temp,
                (char)((base + 7) % 256),
                (short)((base + 8) % 32767),
                (long)base * 17,
                (void*)(size_t)(base * 23),
                (size_t)base * 29,
                (unsigned)base * 31
            );
        }
        
        if (iter % 4 == 0) {
            long_results[idx] += func11_array[iter % 2](
                base * 6,
                g_volatile_float * 6,
                g_volatile_double * 6,
                &temp,
                (char)((base + 9) % 256),
                (short)((base + 10) % 32767),
                (long)base * 19,
                (void*)(size_t)(base * 29),
                (size_t)base * 31,
                (unsigned)base * 37,
                base * 9  /* 11th parameter */
            );
        }
    }
    
    /* Use results to prevent dead code elimination */
    int final_sum = 0;
    for (int i = 0; i < 10; i++) {
        final_sum += results[i] + (int)double_results[i] + 
                    (int)long_results[i] + (int)float_results[i];
    }
    
    printf("Result: %d\n", final_sum);
    return final_sum > 0 ? 0 : 1;
}
