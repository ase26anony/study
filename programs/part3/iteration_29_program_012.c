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
volatile int v1 = 1;
volatile float v2 = 2.0f;
volatile double v3 = 3.0;
volatile int* v4 = NULL;

/* Function with exactly 10 parameters - mixed types */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j)
{
    /* Use all parameters in non-trivial ways */
    int sum = a + (int)b + (int)c + (*d ? *d : 0) + e + f + (int)g;
    float prod = b * c * (h ? 1.0f : 2.0f);
    size_t addr = i + (size_t)h + j;
    
    /* Prevent dead code elimination */
    if (sum > 1000) return (int)(prod + addr);
    return sum + (int)prod + (int)addr;
}

/* Function with exactly 11 parameters - includes struct by value */
static inline double func11(struct Data16 s1, int a, float b, double c, 
                           int* d, char e, short f, long g, void* h, 
                           size_t i, unsigned j)
{
    /* Use all parameters including struct members */
    double base = s1.a * s1.b + s1.c * s1.d;
    double val = base + a + b + c + (*d ? *d : 0.0) + e + f + g;
    
    /* Complex computation using all parameters */
    if (h) val += (double)((size_t)h % 1000);
    val += i * 0.5 + j * 0.25;
    
    /* Volatile memory access to prevent optimization */
    if (v4) val += *v4;
    
    return val;
}

/* Another 10-parameter function with different signature */
static inline long func10_alt(double a, int b, float c, struct Data24 s,
                             void* p, short q, char r, size_t t, 
                             unsigned u, int* out)
{
    /* Use struct members */
    double struct_val = s.x * s.y + s.z + s.w[0];
    
    /* Complex computation */
    long result = (long)(a * b + c * struct_val);
    result += (long)p + q + r + t + u;
    
    if (out) *out = (int)result;
    
    /* Use volatile to prevent optimization */
    result += v1;
    
    return result;
}

/* Another 11-parameter function */
static inline float func11_alt(int a, float b, double c, int* d,
                              struct Data16 s1, struct Data24 s2,
                              char e, short f, long g, void* h,
                              unsigned j)
{
    /* Combine struct data */
    float struct_comb = s1.c * s2.x + s1.d * s2.y;
    
    /* Use all parameters */
    float result = b + (float)c + struct_comb;
    result += a * 0.5f + (*d ? 0.25f : 0.75f);
    result += e + f + g;
    
    if (h) result += (float)((size_t)h % 100);
    result += j * 0.1f;
    
    /* Volatile access */
    result += v2;
    
    return result;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(struct Data16, int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef long (*Func10AltPtr)(double, int, float, struct Data24, void*, short, char, size_t, unsigned, int*);
typedef float (*Func11AltPtr)(int, float, double, int*, struct Data16, struct Data24, char, short, long, void*, unsigned);

/* Array of function pointers */
static Func10Ptr func10_array[] = {func10, NULL};
static Func11Ptr func11_array[] = {func11, NULL};
static Func10AltPtr func10_alt_array[] = {func10_alt, NULL};
static Func11AltPtr func11_alt_array[] = {func11_alt, NULL};

int main(void)
{
    int local_int = 42;
    float local_float = 3.14f;
    double local_double = 2.71828;
    int local_array[5] = {1, 2, 3, 4, 5};
    char local_char = 'A';
    short local_short = 100;
    long local_long = 1000L;
    void* local_ptr = &local_int;
    size_t local_size = 1024;
    unsigned local_unsigned = 65535;
    
    /* Initialize structs */
    struct Data16 s1 = {10, 20, 30.0f, 40.0f};
    struct Data24 s2 = {50.0, 60, 70, {'X', 'Y', 'Z', 'W'}};
    
    int total = 0;
    double total_d = 0.0;
    
    /* Loop to create multiple call sites with varying arguments */
    for (int i = 0; i < 100; i++) {
        /* Vary arguments to prevent constant propagation */
        int arg1 = v1 + i;
        float arg2 = v2 + i * 0.1f;
        double arg3 = v3 + i * 0.01;
        int* arg4 = (i % 2) ? &local_array[i % 5] : NULL;
        char arg5 = local_char + (i % 26);
        short arg6 = local_short + i;
        long arg7 = local_long + i * 100L;
        void* arg8 = (i % 3) ? local_ptr : NULL;
        size_t arg9 = local_size + i;
        unsigned arg10 = local_unsigned + i;
        
        /* Modify structs slightly each iteration */
        s1.a += i;
        s2.x += i * 0.5;
        
        /* Call 10-parameter function directly */
        int result10 = func10(arg1, arg2, arg3, arg4, arg5, 
                             arg6, arg7, arg8, arg9, arg10);
        total += result10;
        
        /* Call 11-parameter function directly */
        double result11 = func11(s1, arg1, arg2, arg3, arg4, arg5,
                                arg6, arg7, arg8, arg9, arg10);
        total_d += result11;
        
        /* Call alternate 10-parameter function */
        int out_val;
        long result10_alt = func10_alt(arg3, arg1, arg2, s2, arg8,
                                      arg6, arg5, arg9, arg10, &out_val);
        total += (int)result10_alt + out_val;
        
        /* Call alternate 11-parameter function */
        float result11_alt = func11_alt(arg1, arg2, arg3, arg4, s1, s2,
                                       arg5, arg6, arg7, arg8, arg10);
        total_d += result11_alt;
        
        /* Call through function pointers (prevents inlining in some cases) */
        if (i % 10 == 0) {
            int idx = (i / 10) % 1;  /* Only one function in array, but compiler doesn't know */
            
            if (func10_array[idx]) {
                int ptr_result = func10_array[idx](arg1, arg2, arg3, arg4, arg5,
                                                  arg6, arg7, arg8, arg9, arg10);
                total += ptr_result;
            }
            
            if (func11_array[idx]) {
                double ptr_result = func11_array[idx](s1, arg1, arg2, arg3, arg4, arg5,
                                                     arg6, arg7, arg8, arg9, arg10);
                total_d += ptr_result;
            }
        }
    }
    
    printf("Results: total=%d, total_d=%.2f\n", total, total_d);
    
    /* Additional calls with constant arguments to allow specialization */
    const int const_args[] = {1, 2, 3, 4, 5};
    
    /* These might get specialized during optimization */
    func10(1, 2.0f, 3.0, const_args, 'X', 10, 100L, NULL, 1000, 2000);
    func11(s1, 1, 2.0f, 3.0, const_args, 'Y', 20, 200L, &total, 2000, 3000);
    func10_alt(1.0, 2, 3.0f, s2, &total, 30, 'Z', 3000, 4000, &local_int);
    func11_alt(1, 2.0f, 3.0, const_args, s1, s2, 'W', 40, 400L, &total_d, 5000);
    
    return total > 0 ? 0 : 1;
}
