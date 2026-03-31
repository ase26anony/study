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

struct Data32 {
    long long x;
    long long y;
    double z;
    int w;
};

/* Volatile variables to prevent constant propagation */
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
    double dsum = c * (double)b + (double)g;
    
    /* Pointer arithmetic */
    if (d) *d += sum;
    
    return sum + (int)fsum + (int)dsum;
}

/* Function with exactly 11 parameters - including struct by value */
static inline double func11(int a, struct Data16 s1, double b, int* c, 
                           char d, short e, long f, void* g, 
                           size_t h, unsigned i, float j) {
    /* Use all parameters extensively */
    double total = (double)a + s1.a + s1.b + s1.c + s1.d + b;
    total += (double)*c + d + e + f;
    total += (double)((size_t)g % 1024) + h + i + j;
    
    /* Complex operations with struct members */
    total += s1.c * s1.d * j;
    total += (s1.a * s1.b) / (b + 1.0);
    
    /* Modify through pointer */
    if (c) *c += (int)total;
    
    return total;
}

/* Another 10-parameter function with struct parameter */
static inline long func10_struct(struct Data32 s, int a, float b, 
                                double c, int* d, char e, short f, 
                                long g, void* h, size_t i) {
    long result = s.x + s.y + (long)s.z + s.w;
    result += a + (long)b + (long)c + *d + e + f + g;
    result += (long)((size_t)h % 4096) + i;
    
    /* Complex computation with struct */
    result += (s.x * s.y) / (a + 1);
    result += (long)(s.z * c);
    
    if (d) *d ^= (int)result;
    
    return result;
}

/* Another 11-parameter function */
static inline float func11_mixed(float a, int b, double c, int* d, 
                                char e, short f, long g, void* h,
                                size_t i, unsigned j, struct Data16 s) {
    float total = a + (float)b + (float)c + (float)*d;
    total += (float)e + f + g;
    total += (float)((size_t)h % 512) + i + j;
    total += s.a + s.b + s.c + s.d;
    
    /* Cross-type operations */
    total *= 1.0f + s.c / 100.0f;
    total += (float)(c * (double)s.d);
    
    if (d) *d = (int)total;
    
    return total;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(int, struct Data16, double, int*, char, short, long, void*, size_t, unsigned, float);
typedef long (*Func10StructPtr)(struct Data32, int, float, double, int*, char, short, long, void*, size_t);

/* Array of function pointers */
static Func10Ptr func10_array[] = {
    (Func10Ptr)func10,
    NULL
};

static Func11Ptr func11_array[] = {
    (Func11Ptr)func11,
    (Func11Ptr)func11_mixed,
    NULL
};

static Func10StructPtr func10struct_array[] = {
    (Func10StructPtr)func10_struct,
    NULL
};

int main(int argc, char** argv) {
    int data1 = 100;
    int data2 = 200;
    int data3 = 300;
    int result_int;
    double result_double;
    long result_long;
    float result_float;
    
    /* Initialize structs */
    struct Data16 s16 = {10, 20, 30.0f, 40.0f};
    struct Data32 s32 = {1000LL, 2000LL, 3000.0, 4000};
    
    /* Call 10-parameter function directly */
    result_int = func10(
        v1,                    /* int */
        v2,                    /* float */
        v3,                    /* double */
        &data1,                /* int* */
        v4,                    /* char */
        (short)v1,             /* short */
        (long)v1 * 100,        /* long */
        (void*)&data1,         /* void* */
        (size_t)v1 * 1000,     /* size_t */
        (unsigned)v1 * 2000    /* unsigned */
    );
    
    printf("Result 10-param: %d\n", result_int);
    
    /* Call 11-parameter function directly */
    result_double = func11(
        v1 + 1,                /* int */
        s16,                   /* struct Data16 (by value) */
        v3 * 2.0,              /* double */
        &data2,                /* int* */
        v4 + 1,                /* char */
        (short)(v1 + 2),       /* short */
        (long)v1 * 200,        /* long */
        (void*)&data2,         /* void* */
        (size_t)v1 * 2000,     /* size_t */
        (unsigned)v1 * 4000,   /* unsigned */
        v2 * 3.0f              /* float */
    );
    
    printf("Result 11-param: %f\n", result_double);
    
    /* Call 10-parameter function with struct */
    result_long = func10_struct(
        s32,                   /* struct Data32 (by value) */
        v1 + 2,                /* int */
        v2 * 2.0f,             /* float */
        v3 * 3.0,              /* double */
        &data3,                /* int* */
        v4 + 2,                /* char */
        (short)(v1 + 3),       /* short */
        (long)v1 * 300,        /* long */
        (void*)&data3,         /* void* */
        (size_t)v1 * 3000      /* size_t */
    );
    
    printf("Result 10-param struct: %ld\n", result_long);
    
    /* Loop calling high-arity functions with varying arguments */
    for (int i = 0; i < 10; i++) {
        /* Vary struct contents */
        s16.a += i;
        s16.c += (float)i;
        s32.x += i;
        s32.z += (double)i;
        
        /* Call through function pointers (prevents inlining in some cases) */
        if (func10_array[0]) {
            result_int = func10_array[0](
                v1 + i,
                v2 + (float)i,
                v3 + (double)i,
                &data1,
                v4 + (char)i,
                (short)(v1 + i),
                (long)v1 * (100 + i),
                (void*)&data1,
                (size_t)v1 * (1000 + i),
                (unsigned)v1 * (2000 + i)
            );
        }
        
        if (func11_array[0]) {
            result_double = func11_array[0](
                v1 + i * 2,
                s16,
                v3 * (1.0 + i * 0.1),
                &data2,
                v4 + (char)(i * 2),
                (short)(v1 + i * 2),
                (long)v1 * (200 + i * 10),
                (void*)&data2,
                (size_t)v1 * (2000 + i * 100),
                (unsigned)v1 * (4000 + i * 200),
                v2 * (2.0f + i * 0.2f)
            );
        }
        
        if (func10struct_array[0]) {
            result_long = func10struct_array[0](
                s32,
                v1 + i * 3,
                v2 * (3.0f + i * 0.3f),
                v3 * (2.0 + i * 0.2),
                &data3,
                v4 + (char)(i * 3),
                (short)(v1 + i * 3),
                (long)v1 * (300 + i * 20),
                (void*)&data3,
                (size_t)v1 * (3000 + i * 200)
            );
        }
    }
    
    /* Conditional calls to create different expansion paths */
    if (argc > 1) {
        result_float = func11_mixed(
            v2 * 5.0f,
            v1 * 10,
            v3 * 7.0,
            &data1,
            'Z',
            (short)v1 * 8,
            (long)v1 * 9000,
            (void*)argv[0],
            (size_t)v1 * 5000,
            (unsigned)v1 * 6000,
            s16
        );
        printf("Conditional result: %f\n", result_float);
    }
    
    return 0;
}
