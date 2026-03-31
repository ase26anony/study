#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Small structs to pass by value (forcing complex parameter passing) */
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
volatile int v1 = 1;
volatile float v2 = 2.0f;
volatile double v3 = 3.0;
volatile int* v4 = NULL;

/* Function with exactly 10 parameters - mixed types */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j)
{
    /* Use all parameters in non-trivial ways to prevent elimination */
    int sum = a + (int)b + (int)c + (*d ? *d : 0) + e + f + (int)g;
    float prod = b * c * (h ? 1.0f : 2.0f);
    size_t addr = (size_t)h + i + j;
    
    /* Complex enough to not be optimized away entirely */
    return (sum * (int)prod) ^ (addr & 0xFFFF);
}

/* Function with exactly 11 parameters - including struct by value */
static inline double func11(struct Data16 s1, int a, float b, double c, 
                           int* d, char e, short f, long g, void* h, 
                           size_t i, unsigned j)
{
    /* Use all parameters including struct members */
    double base = s1.a * s1.b + s1.c * s1.d;
    double val = base + a + b + c + (*d ? *d : 0.0) + e + f + g;
    
    /* Pointer arithmetic that uses h, i, j */
    uintptr_t ptr_val = (uintptr_t)h;
    ptr_val += i * 8 + j * 4;
    
    return val * (ptr_val & 0xFF ? 1.5 : 2.5);
}

/* Another 10-parameter function with different signature */
static inline long func10_alt(double a, int b, float c, void* d, 
                             short e, char f, long g, int* h, 
                             size_t i, unsigned j)
{
    /* Different computation pattern */
    long result = (long)(a * b * c);
    result += (d ? (long)d : 0L);
    result += e * f * g;
    result += (*h) * i * j;
    
    return result & 0x7FFFFFFF;
}

/* Another 11-parameter function with struct parameter */
static inline float func11_alt(int a, float b, struct Data24 s, double c,
                              int* d, char e, short f, long g,
                              void* h, size_t i, unsigned j)
{
    /* Use struct members extensively */
    float struct_contrib = s.x * s.y + s.z;
    for (int k = 0; k < 6; k++) {
        struct_contrib += s.w[k];
    }
    
    float total = struct_contrib + a + b + c + (*d ? *d : 0.0f);
    total += e + f + g;
    
    /* Use pointer parameter */
    if (h) {
        total *= 1.1f;
    }
    
    return total + i + j;
}

/* Function pointer types for obfuscation */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(struct Data16, int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef long (*Func10AltPtr)(double, int, float, void*, short, char, long, int*, size_t, unsigned);
typedef float (*Func11AltPtr)(int, float, struct Data24, double, int*, char, short, long, void*, size_t, unsigned);

/* Array of function pointers to prevent direct call optimization */
static Func10Ptr func10_array[] = {func10, (Func10Ptr)func10_alt};
static Func11Ptr func11_array[] = {func11, (Func11Ptr)func11_alt};

/* Helper to create test data */
static struct Data16 make_data16(int seed) {
    struct Data16 d = {seed, seed * 2, seed * 0.5f, seed * 0.25f};
    return d;
}

static struct Data24 make_data24(int seed) {
    struct Data24 d = {seed * 1.5, seed, (short)(seed % 1000)};
    for (int i = 0; i < 6; i++) {
        d.w[i] = (char)(seed + i);
    }
    return d;
}

int main(void) {
    int result_int = 0;
    double result_double = 0.0;
    long result_long = 0;
    float result_float = 0.0f;
    
    /* Local variables that will be used as arguments */
    int local_int = 42;
    float local_float = 3.14159f;
    double local_double = 2.71828;
    int local_array[4] = {1, 2, 3, 4};
    char local_char = 'A';
    short local_short = 100;
    long local_long = 1000L;
    void* local_ptr = &local_int;
    size_t local_size = sizeof(struct Data16);
    unsigned local_unsigned = 0xDEADBEEF;
    
    /* Loop to create multiple call sites with varying arguments */
    for (int i = 0; i < 100; i++) {
        /* Vary arguments to prevent constant propagation */
        int arg1 = v1 + i;
        float arg2 = v2 + i * 0.1f;
        double arg3 = v3 + i * 0.01;
        int* arg4 = v4 ? v4 : &local_array[i % 4];
        
        /* Call 10-parameter functions directly */
        result_int += func10(arg1, arg2, arg3, arg4,
                           local_char + i, local_short - i,
                           local_long * i, local_ptr,
                           local_size + i, local_unsigned ^ i);
        
        /* Call 10-parameter alternate function */
        result_long += func10_alt(arg3, arg1, arg2, local_ptr,
                                local_short, local_char + i,
                                local_long, arg4,
                                local_size, local_unsigned);
        
        /* Prepare struct arguments */
        struct Data16 s1 = make_data16(i);
        struct Data24 s2 = make_data24(i);
        
        /* Call 11-parameter functions */
        result_double += func11(s1, arg1, arg2, arg3, arg4,
                              local_char, local_short, local_long,
                              local_ptr, local_size, local_unsigned);
        
        result_float += func11_alt(arg1, arg2, s2, arg3, arg4,
                                 local_char, local_short, local_long,
                                 local_ptr, local_size, local_unsigned);
        
        /* Use function pointers (prevents some optimizations) */
        if (i % 3 == 0) {
            result_int += func10_array[i % 2](arg1, arg2, arg3, arg4,
                                            local_char, local_short,
                                            local_long, local_ptr,
                                            local_size, local_unsigned);
        }
        
        if (i % 4 == 0) {
            result_double += func11_array[i % 2](s1, arg1, arg2, arg3, arg4,
                                               local_char, local_short,
                                               local_long, local_ptr,
                                               local_size, local_unsigned);
        }
        
        /* Conditional calls to create different expansion paths */
        if (arg1 % 5 == 0) {
            /* Different argument ordering */
            result_int += func10(local_int, local_float, local_double,
                               &local_array[0], 'X', 999, 7777L,
                               &result_int, sizeof(int), 0xABCD);
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %f, %ld, %f\n", 
           result_int & 0xFF, result_double, 
           result_long & 0xFFFF, result_float);
    
    return (result_int + (int)result_double + (int)result_long + (int)result_float) != 0;
}
