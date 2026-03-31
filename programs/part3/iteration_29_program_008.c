#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Small structs to pass by value - forces complex parameter passing */
struct Point3D {
    float x, y, z;
    int id;
}; /* 16 bytes on most systems */

struct DataChunk {
    int counter;
    double value;
    char tag;
    char padding[7]; /* align to 8 bytes */
}; /* 24 bytes on most systems */

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_int = 42;
volatile float g_volatile_float = 3.14f;
volatile double g_volatile_double = 2.71828;

/* Function with exactly 10 parameters - mixed types */
static inline int func_10_params(
    int a, float b, double c, int* d, char e, 
    short f, long g, void* h, size_t i, unsigned j)
{
    /* Use all parameters to prevent dead code elimination */
    int sum = a + (int)b + (int)c + *d + e + f + (int)g;
    sum += (int)((size_t)h % 256); /* Use pointer as integer */
    sum += (int)i + (int)j;
    
    /* Some arithmetic mixing types */
    double product = b * c * (*d);
    float ratio = (float)g / (b + 1.0f);
    
    /* Return a value that depends on all inputs */
    return sum + (int)product + (int)ratio;
}

/* Function with exactly 11 parameters - includes struct by value */
static inline double func_11_params(
    int a, struct Point3D p, double c, int* d, char e,
    short f, long g, void* h, size_t i, unsigned j, float k)
{
    /* Use all parameters including the struct */
    double sum = a + p.x + p.y + p.z + c + *d + e + f + g;
    sum += (double)((size_t)h % 1024);
    sum += (double)i + (double)j + k;
    
    /* Complex computation with struct fields */
    double volume = p.x * p.y * p.z;
    double weighted = volume * c * k;
    
    /* Return computation involving all parameters */
    return sum + weighted + p.id;
}

/* Another 10-parameter function with struct parameter */
static inline long func_10_mixed(
    struct DataChunk data, int a, float b, double c,
    int* d, char e, short f, long g, void* h, size_t i)
{
    /* Use struct fields */
    long result = data.counter * a + (long)data.value;
    result += (long)b + (long)c + *d + e + f + g;
    result += (long)((size_t)h) + i;
    result += data.tag;
    
    /* Cross-type operations */
    double temp = data.value * c / (b + 1.0f);
    result += (long)temp;
    
    return result;
}

/* 11-parameter function with two structs */
static inline float func_11_complex(
    struct Point3D p1, struct DataChunk d1, int a, float b,
    double c, int* ptr, char ch, short s, long l, void* v, size_t sz)
{
    /* Complex computation using both structs */
    float dot = p1.x * p1.x + p1.y * p1.y + p1.z * p1.z;
    float struct_mix = dot * (float)d1.value + d1.counter;
    
    /* Use all other parameters */
    float result = struct_mix + a + b + (float)c + *ptr + ch + s + l;
    result += (float)((size_t)v % 1000) + (float)sz;
    result += d1.tag;
    
    return result;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(int, struct Point3D, double, int*, char, short, long, void*, size_t, unsigned, float);
typedef long (*Func10MixedPtr)(struct DataChunk, int, float, double, int*, char, short, long, void*, size_t);
typedef float (*Func11ComplexPtr)(struct Point3D, struct DataChunk, int, float, double, int*, char, short, long, void*, size_t);

/* Array of function pointers - prevents direct optimization */
static Func10Ptr func10_array[] = { 
    (Func10Ptr)func_10_params,
    NULL
};

static Func11Ptr func11_array[] = {
    (Func11Ptr)func_11_params,
    NULL
};

static Func10MixedPtr func10mixed_array[] = {
    (Func10MixedPtr)func_10_mixed,
    NULL
};

static Func11ComplexPtr func11complex_array[] = {
    (Func11ComplexPtr)func_11_complex,
    NULL
};

int main(void) {
    int local_int = 100;
    int* int_ptr = &local_int;
    char local_char = 'A';
    short local_short = 32767;
    long local_long = 123456789L;
    void* void_ptr = (void*)&local_int;
    size_t local_size = sizeof(struct Point3D);
    unsigned local_unsigned = 40000;
    float local_float = 1.618f;
    double local_double = 3.14159;
    
    /* Initialize structs */
    struct Point3D point = {1.0f, 2.0f, 3.0f, 42};
    struct DataChunk data = {99, 2.5, 'X', {0}};
    
    int total = 0;
    double total_double = 0.0;
    
    /* Loop to call functions multiple times with varying arguments */
    for (int i = 0; i < 100; i++) {
        /* Vary arguments using volatile and loop counter */
        point.x = g_volatile_float + i;
        point.id = g_volatile_int + i;
        data.counter = i;
        data.value = g_volatile_double + i;
        
        /* Call 10-parameter function directly */
        int result10 = func_10_params(
            i + g_volatile_int,
            g_volatile_float + i,
            local_double + i,
            int_ptr,
            local_char + i,
            local_short - i,
            local_long + i,
            void_ptr,
            local_size + i,
            local_unsigned + i
        );
        total += result10;
        
        /* Call 11-parameter function directly */
        double result11 = func_11_params(
            i,
            point,
            local_double * i,
            int_ptr,
            local_char,
            local_short,
            local_long,
            void_ptr,
            local_size,
            local_unsigned,
            local_float + i
        );
        total_double += result11;
        
        /* Call 10-parameter function with struct */
        long result10mixed = func_10_mixed(
            data,
            i,
            local_float,
            local_double,
            int_ptr,
            local_char,
            local_short,
            local_long,
            void_ptr,
            local_size
        );
        total += (int)result10mixed;
        
        /* Call 11-parameter function with two structs */
        float result11complex = func_11_complex(
            point,
            data,
            i,
            local_float,
            local_double,
            int_ptr,
            local_char,
            local_short,
            local_long,
            void_ptr,
            local_size
        );
        total_double += result11complex;
        
        /* Call through function pointers (prevents inlining in some cases) */
        if (func10_array[0] != NULL && (i % 3) == 0) {
            int ptr_result = func10_array[0](
                i, local_float, local_double, int_ptr,
                local_char, local_short, local_long,
                void_ptr, local_size, local_unsigned
            );
            total += ptr_result;
        }
        
        if (func11_array[0] != NULL && (i % 5) == 0) {
            double ptr_result = func11_array[0](
                i, point, local_double, int_ptr,
                local_char, local_short, local_long,
                void_ptr, local_size, local_unsigned, local_float
            );
            total_double += ptr_result;
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Total int: %d\n", total);
    printf("Total double: %f\n", total_double);
    
    return (total > 0) ? 0 : 1;
}
