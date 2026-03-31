#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Small structs to pass by value - forces complex parameter passing */
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
    char w[4];
};

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_int = 42;
volatile float g_volatile_float = 3.14f;
volatile double g_volatile_double = 2.71828;

/* Function with exactly 10 parameters - mixed types */
static inline int func_10_params(int a, float b, double c, int* d, char e, 
                                 short f, long g, void* h, size_t i, unsigned j)
{
    /* Use all parameters to prevent dead code elimination */
    int result = a + (int)b + (int)c + *d + e + f + (int)g;
    result += (int)((size_t)h % 256) + (int)i + (int)j;
    
    /* Mix operations to create complex RTL */
    float float_res = b * (float)c * (float)a;
    double double_res = c / (double)b + (double)result;
    
    /* Pointer dereference */
    if (d) *d = result + (int)float_res;
    
    return result + (int)double_res;
}

/* Function with exactly 11 parameters - including struct by value */
static inline double func_11_params(int a, struct Data16 s1, float b, double c, 
                                    int* d, char e, short f, struct Data24 s2,
                                    size_t i, unsigned j, long k)
{
    /* Use all parameters including struct fields */
    double result = (double)a + (double)s1.a + (double)s1.b + s1.c + s1.d;
    result += b + c + (double)(*d) + (double)e + (double)f;
    result += s2.x + (double)s2.y + (double)s2.z;
    result += (double)i + (double)j + (double)k;
    
    /* Complex operations mixing all types */
    if (d) {
        *d = (int)(result * 100.0) + s1.a + s2.y;
    }
    
    /* Struct member operations */
    s1.c = b * 2.0f;
    s2.x = c / 2.0;
    
    return result + (double)s1.c + s2.x;
}

/* Another 10-parameter function with different signature */
static inline void* func_10_params_ptr(void* p1, int a, float b, double c, 
                                       int* d, char e, short f, long g, 
                                       size_t i, unsigned j)
{
    /* Complex pointer arithmetic */
    char* cp = (char*)p1;
    cp += a + (int)b + (int)c + e + f + (int)g + (int)i + (int)j;
    
    if (d) {
        *d = (int)((uintptr_t)cp % 1000);
    }
    
    /* Return modified pointer */
    return (void*)cp;
}

/* Another 11-parameter function */
static inline long func_11_params_mixed(struct Data16 s1, int a, float b, 
                                        double c, int* d, char e, short f,
                                        void* h, struct Data24 s2, size_t i, 
                                        unsigned j)
{
    long result = (long)s1.a * (long)s1.b + (long)a;
    result += (long)(b * 1000.0f) + (long)(c * 1000.0);
    result += (long)(*d) * (long)e * (long)f;
    result += (long)((uintptr_t)h % 1000);
    result += (long)s2.x + (long)s2.y * (long)s2.z;
    result += (long)i * (long)j;
    
    /* Modify through pointer */
    if (d) *d = (int)(result % INT32_MAX);
    
    return result;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(int, struct Data16, float, double, int*, char, short, struct Data24, size_t, unsigned, long);
typedef void* (*Func10Ptr2)(void*, int, float, double, int*, char, short, long, size_t, unsigned);
typedef long (*Func11Ptr2)(struct Data16, int, float, double, int*, char, short, void*, struct Data24, size_t, unsigned);

/* Array of function pointers to obfuscate calls */
static Func10Ptr func10_array[] = {
    (Func10Ptr)func_10_params,
    NULL
};

static Func11Ptr func11_array[] = {
    (Func11Ptr)func_11_params,
    NULL
};

static Func10Ptr2 func10_ptr_array[] = {
    func_10_params_ptr,
    NULL
};

static Func11Ptr2 func11_ptr_array[] = {
    func_11_params_mixed,
    NULL
};

int main(void) {
    int counter = 0;
    int data_int = 100;
    float data_float = 1.5f;
    double data_double = 2.5;
    char data_char = 'A';
    short data_short = 32000;
    long data_long = 1000000L;
    size_t data_size = 1024;
    unsigned data_unsigned = 500;
    
    /* Initialize structs */
    struct Data16 s1 = {10, 20, 3.0f, 4.0f};
    struct Data24 s2 = {5.0, 30, 40, {'a', 'b', 'c', 'd'}};
    
    /* Buffer for pointer operations */
    char buffer[256];
    for (int i = 0; i < 256; i++) {
        buffer[i] = (char)i;
    }
    
    /* Loop with varying arguments to trigger different expansion paths */
    for (int i = 0; i < 100; i++) {
        /* Mix volatile and non-volatile arguments */
        int arg1 = g_volatile_int + i;
        float arg2 = g_volatile_float + (float)i;
        double arg3 = g_volatile_double + (double)i;
        
        /* Call 10-parameter function directly */
        int result10 = func_10_params(
            arg1, arg2, arg3, &data_int,
            data_char + i, data_short - i, data_long + i,
            buffer + i, data_size + i, data_unsigned + i
        );
        
        /* Call 11-parameter function directly with structs by value */
        double result11 = func_11_params(
            arg1, s1, arg2, arg3, &data_int,
            data_char + i, data_short - i, s2,
            data_size + i, data_unsigned + i, data_long + i
        );
        
        /* Modify structs for next iteration */
        s1.a += result10 % 10;
        s1.c += (float)(result11 / 100.0);
        s2.x += (double)(result10 % 20);
        s2.y += result11 > 0 ? 1 : -1;
        
        /* Call through function pointers (prevents optimization) */
        if (func10_array[0]) {
            int ptr_result = func10_array[0](
                arg1, arg2, arg3, &data_int,
                data_char, data_short, data_long,
                buffer, data_size, data_unsigned
            );
            counter += ptr_result % 2;
        }
        
        if (func11_array[0]) {
            double ptr_result = func11_array[0](
                arg1, s1, arg2, arg3, &data_int,
                data_char, data_short, s2,
                data_size, data_unsigned, data_long
            );
            counter += (int)ptr_result % 2;
        }
        
        /* Call the other 10-parameter function */
        void* ptr_result = func_10_params_ptr(
            buffer, arg1, arg2, arg3, &data_int,
            data_char, data_short, data_long,
            data_size, data_unsigned
        );
        
        /* Call the other 11-parameter function */
        long long_result = func_11_params_mixed(
            s1, arg1, arg2, arg3, &data_int,
            data_char, data_short, buffer, s2,
            data_size, data_unsigned
        );
        
        /* Use results to prevent dead code elimination */
        counter += (int)((uintptr_t)ptr_result % 256);
        counter += (int)(long_result % 256);
        
        /* Conditional calls to create complex control flow */
        if (i % 3 == 0) {
            func_10_params(
                counter, arg2, arg3, &data_int,
                'X', 100, 200L, NULL,
                512, 256
            );
        } else if (i % 3 == 1) {
            struct Data16 temp_s1 = {counter, i, arg2, arg2 * 2.0f};
            func_11_params(
                counter, temp_s1, arg2, arg3, &data_int,
                'Y', 200, s2,
                256, 128, 300L
            );
        }
    }
    
    printf("Final counter: %d\n", counter);
    printf("Data int: %d\n", data_int);
    
    return 0;
}
