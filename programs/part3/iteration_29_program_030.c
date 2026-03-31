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
    char w[5];
};

/* Volatile variables to prevent constant propagation */
volatile int v1 = 1;
volatile float v2 = 2.0f;
volatile double v3 = 3.0;
volatile char v4 = 'A';

/* Function with exactly 10 parameters */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent dead code elimination */
    int sum = a + (int)b + (int)c + *d + e + f + (int)g;
    sum += (int)((size_t)h % 256);  /* Use pointer as integer */
    sum += (int)i + (int)j;
    
    /* Mix operations with different types */
    float fsum = b * (float)c + (float)a;
    double dsum = c * (double)b + (double)*d;
    
    /* Return a combination of all computations */
    return sum + (int)fsum + (int)dsum;
}

/* Function with exactly 11 parameters */
static inline double func11(double a, int b, float c, void* d, 
                            short e, long f, char g, size_t h,
                            unsigned i, int* j, float k) {
    /* Complex usage of all parameters */
    double result = a * (double)b + (double)c;
    result += (double)((size_t)d % 1000);
    result += (double)e * (double)f;
    result += (double)g * 256.0;
    result += (double)h + (double)i;
    result += (double)*j * k;
    
    /* Additional floating point operations */
    result = result * k + a - c;
    
    return result;
}

/* Function with 10 parameters including struct by value */
static inline struct Data16 func10_struct(int a, struct Data16 s1, float b,
                                         double c, int* d, char e,
                                         short f, void* h, size_t i) {
    struct Data16 result;
    
    /* Use all parameters */
    result.a = a + s1.a + s1.b + (int)b + (int)c + *d;
    result.b = e + f + (int)((size_t)h % 256) + (int)i;
    result.c = b + s1.c + s1.d;
    result.d = (float)c * 0.5f + s1.c;
    
    return result;
}

/* Function with 11 parameters including two structs by value */
static inline double func11_structs(double a, struct Data16 s1, 
                                   struct Data24 s2, int b, float c,
                                   void* d, short e, char g,
                                   size_t h, unsigned i, int* j) {
    /* Complex computation using all parameters */
    double result = a + (double)s1.a + (double)s1.b + s1.c + s1.d;
    result += s2.x + (double)s2.y + (double)s2.z;
    
    for (int idx = 0; idx < 5; idx++) {
        result += (double)s2.w[idx];
    }
    
    result += (double)b * c;
    result += (double)((size_t)d % 100);
    result += (double)e * (double)g;
    result += (double)h + (double)i + (double)*j;
    
    return result;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(double, int, float, void*, short, long, char, size_t, unsigned, int*, float);
typedef struct Data16 (*Func10StructPtr)(int, struct Data16, float, double, int*, char, short, void*, size_t);

/* Array of function pointers */
Func10Ptr func10_array[3] = {NULL, NULL, NULL};
Func11Ptr func11_array[3] = {NULL, NULL, NULL};
Func10StructPtr func10struct_array[2] = {NULL, NULL};

int main(void) {
    int local_int = 42;
    float local_float = 3.14f;
    double local_double = 2.71828;
    int local_array[5] = {1, 2, 3, 4, 5};
    char local_char = 'X';
    short local_short = 100;
    long local_long = 1000L;
    size_t local_size = 1024;
    unsigned local_unsigned = 65535;
    
    /* Initialize structs */
    struct Data16 s1 = {10, 20, 1.5f, 2.5f};
    struct Data24 s2 = {3.14159, 42, 7, {'H', 'e', 'l', 'l', 'o'}};
    
    /* Initialize function pointer arrays */
    func10_array[0] = func10;
    func10_array[1] = func10;  /* Same function, different slot */
    func11_array[0] = func11;
    func11_array[1] = func11;
    func10struct_array[0] = func10_struct;
    
    double total = 0.0;
    
    /* Loop calling functions with many parameters */
    for (int i = 0; i < 100; i++) {
        /* Mix volatile and non-volatile arguments */
        int arg1 = v1 + i;
        float arg2 = v2 + (float)i;
        double arg3 = v3 + (double)i;
        int* arg4 = &local_array[i % 5];
        char arg5 = v4 + (char)(i % 26);
        short arg6 = local_short + i;
        long arg7 = local_long + i;
        void* arg8 = (void*)((size_t)&local_int + i);
        size_t arg9 = local_size + i;
        unsigned arg10 = local_unsigned + i;
        float arg11 = local_float + (float)i;
        
        /* Call 10-parameter function directly */
        int result10 = func10(arg1, arg2, arg3, arg4, arg5,
                             arg6, arg7, arg8, arg9, arg10);
        
        /* Call 11-parameter function directly */
        double result11 = func11(arg3, arg1, arg2, arg8,
                                arg6, arg7, arg5, arg9,
                                arg10, arg4, arg11);
        
        /* Call through function pointer (prevents inlining in some cases) */
        if (func10_array[i % 2]) {
            int ptr_result = func10_array[i % 2](arg1, arg2, arg3, arg4, arg5,
                                                arg6, arg7, arg8, arg9, arg10);
            total += (double)ptr_result;
        }
        
        /* Call struct version */
        struct Data16 s_result = func10_struct(arg1, s1, arg2, arg3, arg4,
                                              arg5, arg6, arg8, arg9);
        
        /* Update struct for next iteration */
        s1.a = s_result.b;
        s1.c = s_result.d;
        
        /* Call 11-parameter struct version */
        double struct_result = func11_structs(arg3, s1, s2, arg1, arg2,
                                             arg8, arg6, arg5, arg9,
                                             arg10, arg4);
        
        /* Accumulate results to prevent elimination */
        total += (double)result10 + result11 + struct_result + s_result.a + s_result.c;
        
        /* Modify some values for next iteration */
        local_array[i % 5] += result10;
        local_int += (int)result11;
    }
    
    /* Conditional block with more calls */
    if (total > 1000.0) {
        /* Another series of calls with different argument patterns */
        for (int j = 0; j < 50; j++) {
            /* Use different argument order and types */
            double r = func11(
                (double)j,                    /* double */
                j * 2,                        /* int */
                (float)j * 0.5f,              /* float */
                (void*)((size_t)j * 100),     /* void* */
                (short)(j + 100),             /* short */
                (long)j * 1000,               /* long */
                (char)('A' + (j % 26)),       /* char */
                (size_t)j * 1024,             /* size_t */
                (unsigned)j * 10000,          /* unsigned */
                &local_int,                   /* int* */
                (float)j * 1.5f               /* float */
            );
            
            total += r;
        }
    }
    
    printf("Result: %f\n", total);
    return (int)total % 256;
}
