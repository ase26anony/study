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

/* Mixed-type 10-parameter function */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent elimination */
    volatile int result = a + (int)b + (int)c + *d + e + f + (int)g;
    result += (int)((size_t)h % 256) + (int)i + j;
    
    /* Complex operations mixing types */
    double temp = c * b + a;
    if (d) temp += *d;
    
    return (int)temp + result;
}

/* 10-parameter function with struct parameter */
static inline double func10_struct(int a, struct Data16 s1, float b, 
                                   double c, int* d, char e, short f, 
                                   long g, void* h, unsigned j) {
    /* Use struct members */
    volatile double result = s1.a + s1.b + s1.c + s1.d;
    result += a + b + c + *d + e + f + g + (double)((size_t)h) + j;
    
    /* Prevent optimization */
    if (result > 1000.0) {
        return result * 0.5;
    }
    return result * 2.0;
}

/* 11-parameter function */
static inline long func11(int a, float b, double c, int* d, char e,
                          short f, long g, void* h, size_t i, 
                          unsigned j, struct Data32 s2) {
    /* Use all 11 parameters */
    volatile long result = a + (long)b + (long)c + *d + e + f + g;
    result += (long)((size_t)h) + (long)i + j + s2.x + s2.y + s2.z + s2.w;
    
    /* Complex conditional to prevent optimization */
    if (c > 0.0) {
        result *= 2;
    } else {
        result /= 2;
    }
    
    return result;
}

/* Another 11-parameter variant */
static inline float func11_mixed(double a, int b, float c, short* d, 
                                 long e, void* f, size_t g, char h,
                                 unsigned i, struct Data16 s3, int j) {
    /* Mix all parameters in computation */
    volatile float result = (float)a + b + c + *d + (float)e;
    result += (float)((size_t)f) + g + h + i + j;
    result += s3.a + s3.b + s3.c + s3.d;
    
    /* Loop to increase complexity */
    for (int k = 0; k < 3; k++) {
        result *= 1.1f;
    }
    
    return result;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef long (*Func11Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned, struct Data32);
typedef float (*Func11MixedPtr)(double, int, float, short*, long, void*, size_t, char, unsigned, struct Data16, int);

/* Array of function pointers */
static Func10Ptr func10_array[] = {func10, (Func10Ptr)func10_struct};
static Func11Ptr func11_array[] = {func11};
static Func11MixedPtr func11_mixed_array[] = {func11_mixed};

/* Helper to create test data */
static struct Data16 make_data16(int seed) {
    struct Data16 d;
    d.a = seed;
    d.b = seed * 2;
    d.c = seed * 0.5f;
    d.d = seed * 0.25f;
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
    volatile int seed = 42; /* volatile to prevent constant propagation */
    int results[10] = {0};
    double double_results[10] = {0.0};
    float float_results[10] = {0.0f};
    
    int local_int = 100;
    float local_float = 3.14f;
    double local_double = 2.71828;
    short local_short = 32767;
    char local_char = 'A';
    long local_long = 123456789L;
    size_t local_size = 1024;
    unsigned local_unsigned = 4294967295U;
    
    /* Create structs */
    struct Data16 s1 = make_data16(seed);
    struct Data16 s3 = make_data16(seed + 1);
    struct Data32 s2 = make_data32(seed);
    
    /* Call 10-parameter functions in a loop */
    for (int i = 0; i < 5; i++) {
        /* Vary arguments to prevent optimization */
        int idx = i % 2;
        
        /* Call through function pointer */
        results[i] = func10_array[idx](
            seed + i,
            local_float + i,
            local_double + i,
            &local_int,
            local_char + i,
            local_short + i,
            local_long + i,
            (void*)((size_t)local_size + i),
            local_size + i,
            local_unsigned - i
        );
        
        /* Direct call to 10-parameter function with struct */
        if (i % 2 == 0) {
            double_results[i] = func10_struct(
                seed + i * 2,
                make_data16(i),  /* Pass struct by value */
                local_float * i,
                local_double / (i + 1),
                &local_int,
                local_char - i,
                local_short >> i,
                local_long << i,
                (void*)(local_size * i),
                local_unsigned % (i + 1)
            );
        }
    }
    
    /* Call 11-parameter functions */
    for (int i = 0; i < 3; i++) {
        /* Call through function pointer array */
        results[i + 5] = (int)func11_array[0](
            seed + i * 3,
            local_float - i,
            local_double * (i + 1),
            &local_int,
            local_char + i * 2,
            local_short - i * 100,
            local_long + i * 1000,
            (void*)(local_size * (i + 1)),
            local_size << i,
            local_unsigned >> i,
            make_data32(i + 10)  /* 11th parameter - struct by value */
        );
        
        /* Call mixed 11-parameter function */
        float_results[i] = func11_mixed_array[0](
            local_double + i * 0.1,
            seed - i,
            local_float * 0.5f,
            &local_short,
            local_long / (i + 1),
            (void*)(local_size + i * 100),
            local_size * 2,
            local_char + i,
            local_unsigned - i * 1000,
            make_data16(i * 5),  /* 10th parameter - struct */
            i * 100               /* 11th parameter */
        );
    }
    
    /* Conditional calls to increase complexity */
    volatile int choice = seed % 3;
    switch (choice) {
        case 0:
            results[8] = func10(
                choice, 1.0f, 2.0, &local_int, 'X', 100, 1000L,
                NULL, 2048, 65535
            );
            break;
        case 1:
            results[8] = (int)func11(
                choice * 10, 2.0f, 3.0, &local_int, 'Y', 200, 2000L,
                (void*)4096, 8192, 131071, make_data32(choice)
            );
            break;
        case 2:
            results[8] = (int)func11_mixed(
                4.0, choice * 20, 4.0f, &local_short, 3000L,
                (void*)8192, 16384, 'Z', 262143, make_data16(choice), 500
            );
            break;
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results: ");
    for (int i = 0; i < 9; i++) {
        printf("%d ", results[i]);
    }
    printf("\nDouble results: ");
    for (int i = 0; i < 5; i++) {
        printf("%f ", double_results[i]);
    }
    printf("\nFloat results: ");
    for (int i = 0; i < 3; i++) {
        printf("%f ", float_results[i]);
    }
    printf("\n");
    
    return 0;
}
