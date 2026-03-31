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

struct Data32 {
    long long x;
    long long y;
    int z;
    int w;
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
    sum += (int)((size_t)h % 256) + (int)i + (int)j;
    
    /* Cross-type operations */
    float fsum = b + (float)c + (float)a;
    double dsum = c + (double)b + (double)*d;
    
    return sum + (int)fsum + (int)dsum;
}

/* Another 10-parameter function with struct parameter */
static inline int func10_struct(int a, float b, struct Data16 s, double c, 
                                int* d, char e, short f, long g, void* h) {
    /* Use struct members */
    int struct_sum = s.a + s.b + (int)s.c + (int)s.d;
    
    /* Use all other parameters */
    int total = a + (int)b + (int)c + *d + e + f + (int)g;
    total += (int)((size_t)h % 256) + struct_sum;
    
    /* Floating operations */
    float ftotal = b + s.c + s.d + (float)c;
    return total + (int)ftotal;
}

/* Function with exactly 11 parameters */
static inline double func11(int a, float b, double c, int* d, char e,
                           short f, long g, void* h, size_t i, 
                           unsigned j, struct Data32 s) {
    /* Use all 11 parameters */
    double sum = (double)a + (double)b + c + (double)*d + (double)e;
    sum += (double)f + (double)g + (double)((size_t)h % 256);
    sum += (double)i + (double)j + (double)s.x + (double)s.y;
    sum += (double)s.z + (double)s.w;
    
    /* Mixed precision operations */
    float fsum = b + (float)c + (float)*d + (float)s.z;
    long lsum = g + (long)s.x + (long)s.y;
    
    return sum + (double)fsum + (double)lsum;
}

/* Another 11-parameter variant */
static inline void* func11_ptr(void* p1, int a, float b, double c, int* d,
                              char e, short f, long g, void* h, size_t i,
                              unsigned j) {
    /* Use all parameters */
    size_t offset = (size_t)p1 + a + (int)b + (int)c + *d + e + f + g;
    offset += (size_t)h + i + j;
    
    /* Return a computed pointer (dummy operation) */
    return (void*)(offset % 1024);
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned, struct Data32);
typedef void* (*Func11Ptr2)(void*, int, float, double, int*, char, short, long, void*, size_t, unsigned);

/* Array of function pointers */
static Func10Ptr func10_array[] = {func10, (Func10Ptr)func10_struct};
static Func11Ptr func11_array[] = {func11};
static Func11Ptr2 func11_ptr_array[] = {func11_ptr};

/* Helper to create test data */
static struct Data16 make_data16(int seed) {
    struct Data16 d;
    d.a = seed;
    d.b = seed * 2;
    d.c = (float)seed / 10.0f;
    d.d = (float)seed / 20.0f;
    return d;
}

static struct Data32 make_data32(int seed) {
    struct Data32 d;
    d.x = seed * 100LL;
    d.y = seed * 200LL;
    d.z = seed * 3;
    d.w = seed * 4;
    return d;
}

int main() {
    int local_int = 100;
    int* int_ptr = &local_int;
    char local_char = 'A';
    short local_short = 32767;
    long local_long = 123456789L;
    void* void_ptr = (void*)0x1000;
    size_t local_size = sizeof(struct Data16);
    unsigned local_unsigned = 4000000000U;
    
    int result_int = 0;
    double result_double = 0.0;
    void* result_ptr = NULL;
    
    /* Loop to call functions multiple times with varying arguments */
    for (int i = 0; i < 100; i++) {
        /* Mix compile-time constants with volatile variables */
        int arg1 = i + g_volatile_int;
        float arg2 = (float)i * 0.5f + g_volatile_float;
        double arg3 = (double)i * 0.25 + g_volatile_double;
        
        /* Create structs */
        struct Data16 s16 = make_data16(i);
        struct Data32 s32 = make_data32(i);
        
        /* Call 10-parameter functions directly */
        result_int += func10(
            arg1, arg2, arg3, int_ptr, local_char + i,
            local_short - i, local_long + i, void_ptr + i,
            local_size + i, local_unsigned - i
        );
        
        /* Call 10-parameter function with struct */
        result_int += func10_struct(
            arg1, arg2, s16, arg3, int_ptr,
            local_char + i, local_short - i, local_long + i,
            void_ptr + i
        );
        
        /* Call 11-parameter function */
        result_double += func11(
            arg1, arg2, arg3, int_ptr, local_char + i,
            local_short - i, local_long + i, void_ptr + i,
            local_size + i, local_unsigned - i, s32
        );
        
        /* Call through function pointers (prevents inlining in some cases) */
        if (i % 2 == 0) {
            result_int += func10_array[0](
                arg1, arg2, arg3, int_ptr, local_char + i,
                local_short - i, local_long + i, void_ptr + i,
                local_size + i, local_unsigned - i
            );
            
            result_double += func11_array[0](
                arg1, arg2, arg3, int_ptr, local_char + i,
                local_short - i, local_long + i, void_ptr + i,
                local_size + i, local_unsigned - i, s32
            );
            
            result_ptr = func11_ptr_array[0](
                result_ptr, arg1, arg2, arg3, int_ptr,
                local_char + i, local_short - i, local_long + i,
                void_ptr + i, local_size + i, local_unsigned - i
            );
        }
        
        /* Conditional calls to create different expansion paths */
        if (i % 3 == 0) {
            /* Different argument order and types */
            result_int += func10(
                g_volatile_int, g_volatile_float, (double)g_volatile_int,
                &local_int, 'Z', 100, 999999L, NULL,
                sizeof(s32), 3000000000U
            );
        }
    }
    
    printf("Results: int=%d, double=%f, ptr=%p\n", 
           result_int, result_double, result_ptr);
    
    return 0;
}
