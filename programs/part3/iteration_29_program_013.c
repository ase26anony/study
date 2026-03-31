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
    char e;
    char f;
    char g;  /* 16 bytes total on most systems */
};

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_int = 42;
volatile float g_volatile_float = 3.14f;
volatile double g_volatile_double = 2.71828;

/* Function with exactly 10 parameters - mixed types */
static inline int func_10_params(int a, float b, double c, int* d, 
                                 char e, short f, long g, void* h, 
                                 size_t i, unsigned j) {
    /* Use all parameters to prevent dead code elimination */
    int result = a + (int)b + (int)c + *d + e + f + (int)g;
    result += (int)((size_t)h % 256);  /* Use pointer as integer */
    result += (int)(i % 256) + j;
    
    /* Complex arithmetic mixing types */
    double temp = c * b * a;
    result += (int)temp;
    
    return result;
}

/* Another 10-parameter function with struct parameter */
static inline int func_10_params_with_struct(struct SmallStruct s1, int a, 
                                             float b, double c, int* d,
                                             char e, short f, long g, 
                                             void* h, size_t i) {
    /* Use all parameters including struct fields */
    int result = s1.x + s1.y + (int)s1.z + s1.w;
    result += a + (int)b + (int)c + *d + e + f + (int)g;
    result += (int)((size_t)h % 256);
    result += (int)(i % 256);
    
    /* Mix struct fields with other parameters */
    double temp = c * b * s1.z;
    result += (int)temp;
    
    return result;
}

/* Function with exactly 11 parameters */
static inline double func_11_params(int a, float b, double c, int* d,
                                    char e, short f, long g, void* h,
                                    size_t i, unsigned j, struct MediumStruct ms) {
    /* Use all 11 parameters */
    double result = (double)a + (double)b + c + (double)*d;
    result += (double)e + (double)f + (double)g;
    result += (double)((size_t)h % 256);
    result += (double)(i % 256) + (double)j;
    
    /* Use struct fields */
    result += ms.a + (double)ms.b + (double)ms.c + (double)ms.d;
    
    /* Complex computation mixing all parameter types */
    result *= 1.0 + (c / (b + 1.0));
    result += ms.a * (double)a;
    
    return result;
}

/* Another 11-parameter function */
static inline long func_11_params_mixed(double a, int b, float c, short* d,
                                        char e, long f, void* g, size_t h,
                                        unsigned i, int j, struct SmallStruct s) {
    /* Use all parameters */
    long result = (long)a + b + (long)c + *d + e + f;
    result += (long)((size_t)g % 256);
    result += (long)(h % 256) + i + j;
    result += s.x + s.y + (long)s.z + s.w;
    
    /* Complex operations */
    result *= (1 + (f % 10));
    result += (long)(a * c * s.z);
    
    return result;
}

/* Function pointer types for obfuscation */
typedef int (*func10_ptr_t)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*func11_ptr_t)(int, float, double, int*, char, short, long, void*, size_t, unsigned, struct MediumStruct);
typedef long (*func11_mixed_ptr_t)(double, int, float, short*, char, long, void*, size_t, unsigned, int, struct SmallStruct);

/* Array of function pointers to prevent direct call optimization */
static func10_ptr_t func10_array[] = {
    (func10_ptr_t)func_10_params,
    (func10_ptr_t)func_10_params_with_struct  /* Note: type mismatch but we'll cast carefully */
};

static func11_ptr_t func11_array[] = {
    func_11_params
};

static func11_mixed_ptr_t func11_mixed_array[] = {
    func_11_params_mixed
};

/* Helper to call through function pointer with 10 params */
static int call_through_ptr_10(func10_ptr_t fp, int idx) {
    int local_int = g_volatile_int + idx;
    float local_float = g_volatile_float + idx;
    double local_double = g_volatile_double + idx;
    int local_array[4] = {idx, idx+1, idx+2, idx+3};
    char local_char = 'A' + idx;
    short local_short = 100 + idx;
    long local_long = 1000L + idx;
    void* local_ptr = (void*)((size_t)&local_array + idx);
    size_t local_size = 10000 + idx;
    unsigned local_unsigned = 500 + idx;
    
    /* This call must go through the 10-parameter expansion */
    return fp(local_int, local_float, local_double, local_array,
              local_char, local_short, local_long, local_ptr,
              local_size, local_unsigned);
}

/* Helper to call through function pointer with 11 params */
static double call_through_ptr_11(func11_ptr_t fp, int idx) {
    int local_int = g_volatile_int + idx;
    float local_float = g_volatile_float + idx;
    double local_double = g_volatile_double + idx;
    int local_array[4] = {idx, idx+1, idx+2, idx+3};
    char local_char = 'B' + idx;
    short local_short = 200 + idx;
    long local_long = 2000L + idx;
    void* local_ptr = (void*)((size_t)&local_array + idx);
    size_t local_size = 20000 + idx;
    unsigned local_unsigned = 600 + idx;
    struct MediumStruct ms = {
        .a = 3.14159 + idx,
        .b = 42 + idx,
        .c = 7 + idx,
        .d = 'X' + idx,
        .e = 'Y',
        .f = 'Z',
        .g = 'W'
    };
    
    /* This call must go through the 11-parameter expansion */
    return fp(local_int, local_float, local_double, local_array,
              local_char, local_short, local_long, local_ptr,
              local_size, local_unsigned, ms);
}

int main() {
    int sum = 0;
    double dsum = 0.0;
    long lsum = 0L;
    
    /* Create structs to pass by value */
    struct SmallStruct s1 = {10, 20, 30.5f, 'S'};
    struct MediumStruct ms1 = {3.14159, 42, 7, 'M', 'N', 'O', 'P'};
    
    /* Loop with varying arguments to trigger different expansion paths */
    for (int i = 0; i < 100; i++) {
        int local_int = g_volatile_int + i;
        float local_float = g_volatile_float + i;
        double local_double = g_volatile_double + i;
        int local_array[4] = {i, i+1, i+2, i+3};
        char local_char = 'A' + (i % 26);
        short local_short = 100 + i;
        long local_long = 1000L + i;
        void* local_ptr = (void*)((size_t)&local_array + i);
        size_t local_size = 10000 + i;
        unsigned local_unsigned = 500 + i;
        
        /* Direct call to 10-parameter function */
        if (i % 3 == 0) {
            sum += func_10_params(local_int, local_float, local_double, local_array,
                                  local_char, local_short, local_long, local_ptr,
                                  local_size, local_unsigned);
        }
        
        /* Direct call to 10-parameter function with struct */
        if (i % 3 == 1) {
            /* Need to adjust struct for each iteration */
            struct SmallStruct dynamic_s = {i, i*2, i*3.0f, 'A' + (i % 26)};
            sum += func_10_params_with_struct(dynamic_s, local_int, local_float, 
                                              local_double, local_array, local_char,
                                              local_short, local_long, local_ptr,
                                              local_size);
        }
        
        /* Direct call to 11-parameter function */
        if (i % 3 == 2) {
            /* Modify struct for each iteration */
            struct MediumStruct dynamic_ms = {
                .a = local_double,
                .b = local_int,
                .c = (short)local_short,
                .d = local_char,
                .e = 'E',
                .f = 'F',
                .g = 'G'
            };
            dsum += func_11_params(local_int, local_float, local_double, local_array,
                                   local_char, local_short, local_long, local_ptr,
                                   local_size, local_unsigned, dynamic_ms);
        }
        
        /* Direct call to mixed 11-parameter function */
        if (i % 5 == 0) {
            struct SmallStruct dynamic_s = {i, i+1, i+2.0f, 'C' + (i % 20)};
            lsum += func_11_params_mixed(local_double, local_int, local_float,
                                         (short*)local_array, local_char,
                                         local_long, local_ptr, local_size,
                                         local_unsigned, i, dynamic_s);
        }
        
        /* Call through function pointer (obfuscates call site) */
        if (i % 7 == 0) {
            sum += call_through_ptr_10(func10_array[i % 2], i);
        }
        
        if (i % 11 == 0) {
            dsum += call_through_ptr_11(func11_array[0], i);
        }
    }
    
    /* Use results to prevent elimination */
    printf("Results: sum=%d, dsum=%f, lsum=%ld\n", sum, dsum, lsum);
    
    /* Additional complex calling patterns in nested loops */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int idx = i * 10 + j;
            int local_int = g_volatile_int + idx;
            float local_float = g_volatile_float + idx;
            double local_double = g_volatile_double + idx;
            int local_array[4] = {idx, idx+1, idx+2, idx+3};
            
            /* Create a complex calling pattern */
            if ((i + j) % 2 == 0) {
                struct MediumStruct ms = {local_double, local_int, (short)idx, 'M', 'N', 'O', 'P'};
                dsum += func_11_params(local_int, local_float, local_double, local_array,
                                       'A' + (idx % 26), (short)idx, 1000L + idx,
                                       (void*)&local_array, 10000 + idx, 500 + idx, ms);
            } else {
                struct SmallStruct ss = {i, j, i*j*1.0f, 'S'};
                sum += func_10_params_with_struct(ss, local_int, local_float, local_double,
                                                  local_array, 'B' + (idx % 26),
                                                  (short)idx, 2000L + idx,
                                                  (void*)&local_array, 20000 + idx);
            }
        }
    }
    
    printf("Final results: sum=%d, dsum=%f\n", sum, dsum);
    
    return 0;
}
