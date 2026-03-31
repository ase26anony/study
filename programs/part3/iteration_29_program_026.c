#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Small structs to pass by value - forces complex parameter passing */
struct Data16 {
    int a;
    float b;
    short c;
    char d;
    int e;
};

struct Data32 {
    double x;
    long y;
    int z[3];
    char w;
};

/* Mixed-type 10-parameter function */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent dead code elimination */
    volatile int result = a + (int)b + (int)c + *d + e + f + (int)g;
    result += (int)((size_t)h % 256) + (int)i + j;
    
    /* Complex arithmetic mixing types */
    *d = result * ((int)b + e - f);
    return result ^ ((unsigned)g << 3);
}

/* Mixed-type 11-parameter function */
static inline double func11(double a, int b, float c, long* d, short e,
                           unsigned f, size_t g, char h, void* i, 
                           struct Data16 s, int j) {
    /* Use all parameters */
    volatile double result = a + b + c + *d + e + f + g + h;
    result += (double)((size_t)i % 1000);
    
    /* Use struct members */
    result += s.a * s.b + s.c - s.d + s.e;
    
    /* Modify pointer parameter */
    *d = (long)(result * j);
    
    return result * (c + 1.0f) / (j + 1);
}

/* 10-parameter function with struct parameter */
static inline struct Data32 func10_struct(int a, struct Data16 s1, float b,
                                         double c, int* d, char e,
                                         short f, long g, void* h,
                                         unsigned j) {
    struct Data32 result = {0};
    
    /* Use all parameters */
    result.x = a + s1.a + b + c + *d + e + f + g + ((size_t)h % 100) + j;
    result.y = (long)(result.x * s1.b);
    result.z[0] = a * s1.c;
    result.z[1] = (int)(b * 100);
    result.z[2] = *d + e + f;
    result.w = (char)(g % 256);
    
    *d = result.z[0] + result.z[1] + result.z[2];
    return result;
}

/* 11-parameter function with two structs */
static inline long func11_structs(struct Data16 s1, struct Data32 s2,
                                 int a, float b, double c, int* d,
                                 char e, short f, long g, void* h,
                                 unsigned j) {
    /* Complex computation using all parameters */
    long result = s1.a + s1.e + (long)s2.x + s2.y + a + (long)b + (long)c;
    result += *d + e + f + g + ((size_t)h % 1000) + j;
    
    /* Use array from struct */
    result += s2.z[0] + s2.z[1] + s2.z[2];
    
    /* Modify through pointer */
    *d = (int)(result % INT32_MAX);
    
    return result * (s1.c + 1) * (s2.w + 1);
}

/* Function pointer types for obfuscation */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(double, int, float, long*, short, unsigned, size_t, char, void*, struct Data16, int);
typedef struct Data32 (*Func10StructPtr)(int, struct Data16, float, double, int*, char, short, long, void*, unsigned);
typedef long (*Func11StructsPtr)(struct Data16, struct Data32, int, float, double, int*, char, short, long, void*, unsigned);

/* Array of function pointers to prevent direct optimization */
static volatile Func10Ptr func10_array[] = {
    (Func10Ptr)func10,
    (Func10Ptr)func10,
    (Func10Ptr)func10
};

static volatile Func11Ptr func11_array[] = {
    (Func11Ptr)func11,
    (Func11Ptr)func11,
    (Func11Ptr)func11
};

static volatile Func10StructPtr func10struct_array[] = {
    (Func10StructPtr)func10_struct,
    (Func10StructPtr)func10_struct
};

static volatile Func11StructsPtr func11structs_array[] = {
    (Func11StructsPtr)func11_structs,
    (Func11StructsPtr)func11_structs
};

int main() {
    volatile int seed = 42;  /* Prevent constant propagation */
    int total = 0;
    
    /* Initialize test data */
    int int_val = 100;
    float float_val = 3.14f;
    double double_val = 2.71828;
    int int_ptr_val = 999;
    int* int_ptr = &int_ptr_val;
    long long_ptr_val = 123456;
    long* long_ptr = &long_ptr_val;
    
    struct Data16 s16 = {10, 2.5f, 5, 'A', 20};
    struct Data32 s32 = {3.14159, 987654, {1, 2, 3}, 'Z'};
    
    /* Loop to create multiple call sites with varying arguments */
    for (int i = 0; i < 100; i++) {
        /* Vary parameters to prevent constant folding */
        int a = seed + i;
        float b = float_val + i * 0.1f;
        double c = double_val + i * 0.01;
        char e = 'A' + (i % 26);
        short f = (short)(100 + i);
        long g = 1000L + i * 10L;
        size_t sz = (size_t)(1000 + i);
        unsigned u = 500 + i * 2;
        
        /* Modify structs slightly each iteration */
        s16.a = 10 + i;
        s16.b = 2.5f + i * 0.01f;
        s32.x = 3.14159 + i * 0.001;
        s32.y = 987654 + i;
        
        /* Call 10-parameter function directly */
        int res1 = func10(a, b, c, int_ptr, e, f, g, (void*)(size_t)sz, sz, u);
        
        /* Call 11-parameter function directly */
        double res2 = func11(c, a, b, long_ptr, f, u, sz, e, (void*)(size_t)g, s16, i);
        
        /* Call 10-parameter function with struct */
        struct Data32 res3 = func10_struct(a, s16, b, c, int_ptr, e, f, g, 
                                          (void*)(size_t)sz, u);
        
        /* Call 11-parameter function with two structs */
        long res4 = func11_structs(s16, s32, a, b, c, int_ptr, e, f, g,
                                  (void*)(size_t)sz, u);
        
        /* Call through function pointers (prevents inlining in some cases) */
        if (i % 3 == 0) {
            int res5 = func10_array[i % 3](a, b, c, int_ptr, e, f, g, 
                                          (void*)(size_t)sz, sz, u);
            total += res5;
        }
        
        if (i % 4 == 0) {
            double res6 = func11_array[i % 3](c, a, b, long_ptr, f, u, sz, e,
                                            (void*)(size_t)g, s16, i);
            total += (int)res6;
        }
        
        if (i % 5 == 0) {
            struct Data32 res7 = func10struct_array[i % 2](a, s16, b, c, int_ptr,
                                                         e, f, g, 
                                                         (void*)(size_t)sz, u);
            total += res7.z[0];
        }
        
        if (i % 7 == 0) {
            long res8 = func11structs_array[i % 2](s16, s32, a, b, c, int_ptr,
                                                  e, f, g, (void*)(size_t)sz, u);
            total += (int)res8;
        }
        
        total += res1 + (int)res2 + (int)res3.x + (int)res4;
        
        /* Prevent loop unrolling from simplifying everything */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    printf("Result: %d\n", total);
    return 0;
}
