#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Small structs to pass by value - forcing complex parameter passing */
struct Vec2 {
    double x;
    double y;
};

struct Data {
    int id;
    float value;
    char tag;
    short flags;
};

struct Mixed {
    void* ptr;
    int count;
    char type;
    double scale;
};

/* Function with exactly 10 parameters - mixed types */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent dead code elimination */
    volatile int result = a + (int)b + (int)c + *d + e + f + (int)g;
    result += (int)((size_t)h % 256) + (int)i + j;
    
    /* Complex operations mixing types */
    double temp = c * b + a;
    if (d != NULL) {
        temp += *d;
    }
    
    return (int)temp + result % 256;
}

/* Function with exactly 11 parameters - including struct by value */
static inline double func11(struct Vec2 v1, int a, float b, double c, 
                           int* d, char e, struct Data data, void* h, 
                           size_t i, unsigned j, long k) {
    /* Use all parameters extensively */
    double base = v1.x * v1.y + a + b + c;
    
    if (d != NULL) {
        base += *d * data.value;
    }
    
    base += e * data.id;
    base += (double)((size_t)h % 1024) / 1024.0;
    base += i + j + k;
    base += data.flags * 0.01;
    
    return base;
}

/* Another 10-parameter function with different signature */
static inline void* func10_ptr(struct Mixed m, int a, float b, double c,
                               int* d, char e, short f, long g, 
                               size_t i, unsigned j) {
    /* Complex pointer arithmetic using parameters */
    void* result = m.ptr;
    if (result != NULL) {
        char* p = (char*)result;
        p += a % 16;
        p += (int)(b * 10);
        p += (int)c % 8;
        result = (void*)p;
    }
    
    /* Use other parameters */
    volatile int check = *d + e + f + g + m.count + i + j;
    (void)check;  /* Prevent unused warning */
    
    return result;
}

/* 11-parameter function with struct parameters */
static inline int func11_mixed(int a, struct Vec2 v1, float b, double c,
                               struct Data d1, int* e, char f, 
                               struct Mixed m, size_t i, unsigned j, long k) {
    /* Complex computation using all parameters */
    int score = a + (int)(v1.x + v1.y) + (int)b;
    score += (int)c * d1.id;
    score += d1.flags * m.count;
    
    if (e != NULL) {
        score += *e;
    }
    
    score += f * 100;
    score += (int)((size_t)m.ptr % 1000);
    score += i % 256 + j % 256 + k % 256;
    
    return score;
}

/* Function pointer types */
typedef int (*Func10Type)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Type)(struct Vec2, int, float, double, int*, char, struct Data, void*, size_t, unsigned, long);
typedef void* (*Func10PtrType)(struct Mixed, int, float, double, int*, char, short, long, size_t, unsigned);
typedef int (*Func11MixedType)(int, struct Vec2, float, double, struct Data, int*, char, struct Mixed, size_t, unsigned, long);

/* Array of function pointers to obfuscate calls */
static Func10Type func10_array[] = {func10, NULL};
static Func11Type func11_array[] = {func11, NULL};
static Func10PtrType func10_ptr_array[] = {func10_ptr, NULL};
static Func11MixedType func11_mixed_array[] = {func11_mixed, NULL};

/* Helper to create volatile variables preventing constant propagation */
static volatile int volatile_seed = 42;

int main() {
    int result = 0;
    double total = 0.0;
    
    /* Initialize test data */
    int int_val = 100;
    float float_val = 3.14f;
    double double_val = 2.71828;
    int* int_ptr = &int_val;
    char char_val = 'A';
    short short_val = 32767;
    long long_val = 1000000L;
    void* void_ptr = (void*)&int_val;
    size_t size_val = sizeof(struct Vec2);
    unsigned uint_val = 4294967295U;
    
    /* Initialize structs */
    struct Vec2 vec = {1.5, 2.5};
    struct Data data = {123, 4.56f, 'X', 7};
    struct Mixed mixed = {&int_val, 99, 'T', 1.234};
    
    /* Call functions directly in a loop - forcing multiple expansions */
    for (int i = 0; i < 10; i++) {
        /* Vary arguments to prevent optimization */
        int_val += volatile_seed % 10;
        float_val += 0.1f;
        double_val += 0.01;
        char_val += 1;
        
        /* Call 10-parameter function */
        result += func10(int_val, float_val, double_val, int_ptr,
                        char_val, short_val, long_val, void_ptr,
                        size_val + i, uint_val);
        
        /* Call 11-parameter function */
        vec.x += 0.5;
        vec.y += 0.25;
        total += func11(vec, int_val, float_val, double_val, int_ptr,
                       char_val, data, void_ptr, size_val, uint_val, long_val);
        
        /* Call through function pointers (prevents inlining in some cases) */
        if (func10_array[0] != NULL) {
            result += func10_array[0](int_val + 1, float_val + 1.0f, 
                                     double_val + 1.0, int_ptr,
                                     char_val + 1, short_val - 1, 
                                     long_val + 1, void_ptr,
                                     size_val, uint_val - 1);
        }
        
        /* Call 11-parameter through pointer */
        if (func11_mixed_array[0] != NULL) {
            data.id += i;
            mixed.count += 1;
            result += func11_mixed_array[0](int_val, vec, float_val, double_val,
                                           data, int_ptr, char_val,
                                           mixed, size_val, uint_val, long_val);
        }
    }
    
    /* Additional calls with different argument patterns */
    struct Vec2 vec2 = {10.0, 20.0};
    struct Data data2 = {999, 8.88f, 'Z', 15};
    struct Mixed mixed2 = {&result, 50, 'M', 5.678};
    
    /* Chain of calls with 10/11 parameters */
    for (int i = 0; i < 5; i++) {
        /* These calls should trigger different expansion patterns */
        void* ptr_result = func10_ptr(mixed2, i * 10, float_val * i, 
                                     double_val * i, &result, 'A' + i,
                                     short_val / (i + 1), long_val - i * 1000,
                                     size_val * i, uint_val / (i + 1));
        
        (void)ptr_result;  /* Use result to prevent elimination */
        
        /* Nested calls - might trigger interesting expansions during inlining */
        int nested = func10(
            func10(i, float_val, double_val, &result, 'B', 100, 500, NULL, 10, 20) % 100,
            float_val, double_val, int_ptr, 'C', 200, 600, void_ptr, 30, 40
        );
        
        result += nested;
    }
    
    printf("Result: %d, Total: %f\n", result, total);
    
    /* Use volatile to ensure all calls happen */
    if (volatile_seed > 0) {
        return result % 256;
    }
    
    return 0;
}
