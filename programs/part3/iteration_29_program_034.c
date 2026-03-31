#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Small structs to pass by value */
struct SmallStruct {
    int x;
    float y;
    double z;
    char w;
};

struct MediumStruct {
    long a;
    double b;
    int c;
    float d;
};

/* Mixed-type functions with exactly 10 parameters */
static inline int func10_1(int a, float b, double c, int* d, char e, 
                          short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent elimination */
    *d = a + (int)b + (int)c + e + f + (int)g + (int)((size_t)h % 256) + (int)i + j;
    return *d;
}

static inline double func10_2(double a, int b, float c, struct SmallStruct s, 
                             long* d, char e, unsigned short f, size_t g, 
                             int h, float i) {
    /* Complex mixed operations */
    *d = (long)(a + b + c + s.x + s.y + s.z + e + f + g + h + i);
    return a * c + s.z - i;
}

/* Mixed-type functions with exactly 11 parameters */
static inline void* func11_1(int a, float b, double c, int* d, char e,
                            short f, long g, void* h, size_t i, unsigned j,
                            struct MediumStruct ms) {
    /* Use all 11 parameters */
    *d = a + (int)b + (int)c + e + f + (int)g + 
         (int)((size_t)h % 256) + (int)i + j + (int)ms.a + (int)ms.b + ms.c;
    return h;
}

static inline long func11_2(struct SmallStruct s1, int a, float b, double c,
                           int* d, char e, short f, long g, void* h,
                           size_t i, struct SmallStruct s2) {
    /* Complex computation using all parameters */
    long result = s1.x + (int)s1.y + (int)s1.z + a + (int)b + (int)c + 
                  e + f + g + (long)h + i + s2.x + (int)s2.y + (int)s2.z;
    *d = (int)result;
    return result;
}

/* Function pointer types */
typedef int (*Func10Ptr1)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func10Ptr2)(double, int, float, struct SmallStruct, long*, char, unsigned short, size_t, int, float);
typedef void* (*Func11Ptr1)(int, float, double, int*, char, short, long, void*, size_t, unsigned, struct MediumStruct);
typedef long (*Func11Ptr2)(struct SmallStruct, int, float, double, int*, char, short, long, void*, size_t, struct SmallStruct);

/* Array of function pointers */
static Func10Ptr1 func10_ptrs[] = {func10_1, NULL};
static Func10Ptr2 func10_ptrs2[] = {func10_2, NULL};
static Func11Ptr1 func11_ptrs[] = {func11_1, NULL};
static Func11Ptr2 func11_ptrs2[] = {func11_2, NULL};

/* Volatile variables to prevent constant propagation */
static volatile int volatile_seed = 42;
static volatile float volatile_float = 3.14f;
static volatile double volatile_double = 2.71828;

int main() {
    int result1, result2;
    double dresult;
    long lresult;
    void* presult;
    
    int d1 = 0, d2 = 0, d3 = 0, d4 = 0;
    long ld1 = 0;
    
    struct SmallStruct s1 = {10, 20.5f, 30.75, 'A'};
    struct SmallStruct s2 = {40, 50.25f, 60.125, 'B'};
    struct MediumStruct ms = {100, 200.5, 300, 400.75f};
    
    /* Call functions directly with mixed arguments */
    for (int i = 0; i < 10; i++) {
        /* Vary arguments using volatile values */
        int arg1 = volatile_seed + i;
        float arg2 = volatile_float + i;
        double arg3 = volatile_double * i;
        
        /* Call 10-parameter functions */
        result1 = func10_1(arg1, arg2, arg3, &d1, 
                          (char)('A' + i), (short)(i * 100), 
                          (long)(i * 1000), (void*)(size_t)i, 
                          (size_t)(i * 10000), (unsigned)(i * 100000));
        
        dresult = func10_2(arg3, arg1, arg2, s1, &ld1,
                          (char)('Z' - i), (unsigned short)(i * 50),
                          (size_t)(i * 5000), arg1 % 100, arg2 * 2.0f);
        
        /* Call 11-parameter functions */
        presult = func11_1(arg1, arg2, arg3, &d2,
                          (char)('M' + i), (short)(i * 75),
                          (long)(i * 750), (void*)(size_t)(i + 100),
                          (size_t)(i * 7500), (unsigned)(i * 75000),
                          ms);
        
        lresult = func11_2(s2, arg1, arg2, arg3, &d3,
                          (char)('N' + i), (short)(i * 80),
                          (long)(i * 800), (void*)(size_t)(i + 200),
                          (size_t)(i * 8000), s1);
    }
    
    /* Call through function pointers with volatile guard */
    if (volatile_seed > 0) {
        /* Use different function pointers based on runtime condition */
        int idx = volatile_seed % 2;
        
        if (func10_ptrs[idx] != NULL) {
            result2 = func10_ptrs[idx](volatile_seed, volatile_float, 
                                      volatile_double, &d4, 'X', 999, 
                                      1000L, (void*)0x1000, 5000UL, 60000U);
        }
        
        if (func10_ptrs2[idx] != NULL) {
            dresult = func10_ptrs2[idx](volatile_double, volatile_seed,
                                       volatile_float, s2, &ld1, 'Y',
                                       888, 4000UL, 300, 1.5f);
        }
        
        if (func11_ptrs[idx] != NULL) {
            presult = func11_ptrs[idx](volatile_seed + 1, volatile_float * 2,
                                      volatile_double / 2, &d1, 'Z', 777,
                                      888L, (void*)0x2000, 3000UL, 40000U,
                                      ms);
        }
        
        if (func11_ptrs2[idx] != NULL) {
            lresult = func11_ptrs2[idx](s1, volatile_seed - 1, volatile_float / 2,
                                       volatile_double * 3, &d2, 'W', 666,
                                       777L, (void*)0x3000, 2000UL, s2);
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d %d %lf %ld %p\n", result1, d1, dresult, lresult, presult);
    printf("d values: %d %d %d %d\n", d1, d2, d3, d4);
    printf("ld1: %ld\n", ld1);
    
    return 0;
}
