#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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

/* Mixed parameter types to prevent optimization */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent dead code elimination */
    volatile int result = a + (int)b + (int)c + *d + e + f + (int)g;
    result += (int)((size_t)h % 256) + (int)i + j;
    
    /* Complex arithmetic mixing types */
    *d = (*d * a) + (int)(b * 100.0f) + (int)(c * 10.0);
    return result & 0xFF;
}

static inline double func11(int a, float b, double c, int* d, char e,
                           short f, long g, void* h, size_t i, unsigned j,
                           struct Data16 s) {
    /* Use all parameters including struct */
    double result = (double)a + (double)b + c + (double)*d + (double)e;
    result += (double)f + (double)g + (double)((size_t)h % 256);
    result += (double)i + (double)j + (double)s.a + (double)s.b + (double)s.c + (double)s.d;
    
    /* Modify pointer parameter */
    *d = s.a * s.b + (int)(s.c * s.d);
    
    return result;
}

/* Another 10-parameter function with different signature */
static inline long func10_alt(struct Data24 s1, int a, float b, double c, 
                             int* d, char e, short f, long g, void* h, size_t i) {
    /* Use struct and other parameters */
    long result = s1.y * a + (long)(b * 1000.0f) + (long)c;
    result += *d * e * f * g;
    result += (long)((size_t)h) + i;
    
    /* Complex operation with struct */
    *d = (int)(s1.x * 100.0) + s1.z;
    return result;
}

/* 11-parameter function with struct in middle */
static inline float func11_mid(int a, float b, struct Data16 s, double c,
                              int* d, char e, short f, long g, void* h,
                              size_t i, unsigned j) {
    /* Mix struct with other parameters */
    float result = b + s.c + s.d + (float)c;
    result += (float)(a * s.a * s.b) / 100.0f;
    result += (float)(*d + e + f + g + (int)((size_t)h % 256) + (int)i + j);
    
    *d = (int)(result * 10.0f);
    return result;
}

/* Function pointer types for obfuscation */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned, struct Data16);
typedef long (*Func10AltPtr)(struct Data24, int, float, double, int*, char, short, long, void*, size_t);

/* Array of function pointers */
static Func10Ptr func10_array[] = {func10, NULL};
static Func11Ptr func11_array[] = {func11, func11_mid, NULL};
static Func10AltPtr func10_alt_array[] = {func10_alt, NULL};

/* Helper to create test data */
static struct Data16 make_data16(int seed) {
    struct Data16 d = {
        .a = seed * 2,
        .b = seed * 3,
        .c = (float)seed * 1.5f,
        .d = (float)seed * 2.5f
    };
    return d;
}

static struct Data24 make_data24(int seed) {
    struct Data24 d = {
        .x = (double)seed * 3.14159,
        .y = seed * 4,
        .z = (short)(seed * 5)
    };
    d.w[0] = 'A' + (seed % 26);
    d.w[1] = 'a' + (seed % 26);
    d.w[2] = '0' + (seed % 10);
    d.w[3] = '\0';
    return d;
}

int main(int argc, char** argv) {
    volatile int seed = 42; /* volatile to prevent constant propagation */
    int iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    int total = 0;
    double accum = 0.0;
    long alt_accum = 0;
    
    /* Loop with varying arguments to trigger different expansion paths */
    for (int i = 0; i < iterations; i++) {
        int local_var = seed + i;
        float f_var = (float)local_var * 0.5f;
        double d_var = (double)local_var * 0.25;
        int int_val = local_var * 3;
        char char_val = 'A' + (local_var % 26);
        short short_val = (short)(local_var * 7);
        long long_val = (long)local_var * 11L;
        void* ptr_val = (void*)((size_t)local_var * 13);
        size_t size_val = (size_t)local_var * 17;
        unsigned uint_val = (unsigned)local_var * 19;
        
        /* Create structs */
        struct Data16 s16 = make_data16(local_var);
        struct Data24 s24 = make_data24(local_var);
        
        /* Direct calls - may be inlined */
        int result10 = func10(local_var, f_var, d_var, &int_val, char_val,
                             short_val, long_val, ptr_val, size_val, uint_val);
        
        double result11 = func11(local_var, f_var, d_var, &int_val, char_val,
                                short_val, long_val, ptr_val, size_val, uint_val, s16);
        
        long result10_alt = func10_alt(s24, local_var, f_var, d_var, &int_val,
                                      char_val, short_val, long_val, ptr_val, size_val);
        
        float result11_mid = func11_mid(local_var, f_var, s16, d_var, &int_val,
                                       char_val, short_val, long_val, ptr_val,
                                       size_val, uint_val);
        
        total += result10 + (int)result11 + (int)result10_alt + (int)result11_mid;
        accum += result11 + result11_mid;
        alt_accum += result10_alt;
        
        /* Function pointer calls - prevents inlining in some cases */
        if (func10_array[0] != NULL) {
            int fp_result = func10_array[0](local_var + 1, f_var + 1.0f, d_var + 1.0,
                                           &int_val, char_val + 1, short_val + 1,
                                           long_val + 1, ptr_val, size_val + 1, uint_val + 1);
            total += fp_result;
        }
        
        if (func11_array[0] != NULL && i % 2 == 0) {
            struct Data16 s16_alt = make_data16(local_var * 2);
            double fp_result = func11_array[0](local_var, f_var, d_var, &int_val, char_val,
                                              short_val, long_val, ptr_val, size_val, uint_val, s16_alt);
            accum += fp_result;
        }
        
        if (func10_alt_array[0] != NULL && i % 3 == 0) {
            struct Data24 s24_alt = make_data24(local_var * 3);
            long fp_result = func10_alt_array[0](s24_alt, local_var, f_var, d_var, &int_val,
                                                char_val, short_val, long_val, ptr_val, size_val);
            alt_accum += fp_result;
        }
        
        /* Vary parameters for next iteration */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    printf("Results: total=%d, accum=%.2f, alt_accum=%ld\n", 
           total, accum, alt_accum);
    
    return total > 0 ? 0 : 1;
}
