#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Small structs to pass by value (forces complex parameter passing) */
struct Data16 {
    int a;
    int b;
    float c;
    float d;
};

struct Data32 {
    long long x;
    long long y;
    double z;
    int w;
};

/* Mixed parameter types to prevent optimization */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent dead code elimination */
    int sum = a + (int)b + (int)c + *d + e + f + (int)g;
    sum += (int)((size_t)h % 256) + (int)i + (int)j;
    
    /* Complex arithmetic mixing types */
    double result = (double)a * b + c / (*d + 1) + (double)e * f;
    result += (double)g * 0.5 + (double)((size_t)h % 1000) * 0.01;
    
    return sum + (int)result;
}

static inline double func11(int a, float b, double c, int* d, char e,
                           short f, long g, void* h, size_t i, unsigned j,
                           struct Data16 s) {
    /* Use all parameters including struct */
    double total = (double)a + b + c + (double)*d + (double)e + (double)f;
    total += (double)g + (double)((size_t)h % 1000) * 0.001 + (double)i + (double)j;
    
    /* Use struct members */
    total += s.a * 1.5 + s.b * 2.0 + s.c * 3.0 + s.d * 4.0;
    
    /* Complex computation to ensure all params are used */
    if (a > 0) {
        total *= 1.0 + (b / 100.0f);
    }
    if (*d < 0) {
        total -= c * 0.5;
    }
    
    return total;
}

/* Another 10-parameter function with different types */
static inline long func10_alt(double a, int b, float* c, long d, short e,
                             unsigned f, size_t g, char* h, int i, struct Data32 s) {
    long result = (long)a + b + d + (long)(*c * 100.0f);
    result += e + (long)f + (long)g + (long)h[0] + i;
    
    /* Use struct members */
    result += (long)(s.x % 1000) + (long)(s.y % 1000) + (long)s.z + s.w;
    
    /* Conditional use to prevent optimization */
    if (a > 100.0) {
        result *= 2;
    }
    if (b < 0) {
        result -= (long)(*c * 50.0f);
    }
    
    return result;
}

/* 11-parameter function with struct parameter */
static inline float func11_struct(int a, float b, double c, int* d, char e,
                                 short f, long g, void* h, size_t i, unsigned j,
                                 struct Data32 s) {
    float total = (float)a + b + (float)c + (float)*d + (float)e + (float)f;
    total += (float)g + (float)((size_t)h % 100) + (float)i + (float)j;
    
    /* Use struct members in computation */
    total += (float)(s.x % 100) * 0.1f + (float)(s.y % 100) * 0.2f;
    total += (float)s.z * 0.3f + (float)s.w * 0.4f;
    
    /* Complex branching to use all parameters */
    if (a % 2 == 0) {
        total *= 1.5f + b * 0.01f;
    } else {
        total /= 1.0f + c * 0.001;
    }
    
    return total;
}

/* Function pointer types for obfuscation */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned, struct Data16);
typedef long (*Func10AltPtr)(double, int, float*, long, short, unsigned, size_t, char*, int, struct Data32);
typedef float (*Func11StructPtr)(int, float, double, int*, char, short, long, void*, size_t, unsigned, struct Data32);

/* Array of function pointers */
static Func10Ptr func10_array[] = {func10, (Func10Ptr)func10_alt};
static Func11Ptr func11_array[] = {func11, (Func11Ptr)func11_struct};

int main(void) {
    volatile int seed = 42; /* volatile to prevent constant propagation */
    int result_int;
    double result_double;
    long result_long;
    float result_float;
    
    /* Local variables for parameters */
    int x = 10;
    float y = 3.14f;
    double z = 2.71828;
    int arr[5] = {1, 2, 3, 4, 5};
    char ch = 'A';
    short sh = 100;
    long lg = 1000L;
    void* ptr = &x;
    size_t sz = 1000;
    unsigned un = 500;
    
    /* Struct instances */
    struct Data16 s16 = {1, 2, 3.0f, 4.0f};
    struct Data32 s32 = {100LL, 200LL, 300.0, 400};
    
    /* Call 10-parameter functions in a loop */
    for (int i = 0; i < 100; i++) {
        /* Vary parameters to prevent optimization */
        x = (seed + i) % 100;
        y = 3.14f + i * 0.1f;
        z = 2.71828 + i * 0.01;
        arr[0] = i;
        
        /* Call func10 directly */
        result_int = func10(x, y, z, arr, ch + i, sh + i, lg + i, 
                           ptr, sz + i, un + i);
        
        /* Call through function pointer */
        int idx = i % 2;
        result_int += func10_array[idx](x + 1, y + 1.0f, z + 1.0, 
                                       arr + 1, ch + 1, sh + 1, 
                                       lg + 1, ptr, sz + 1, un + 1);
        
        /* Call 11-parameter function */
        s16.a = i;
        s16.b = i * 2;
        result_double = func11(x, y, z, arr, ch, sh, lg, ptr, sz, un, s16);
        
        /* Call through function pointer array */
        result_double += func11_array[0](x, y, z, arr, ch, sh, lg, ptr, sz, un, s16);
        
        /* Call alternate 10-parameter function with struct */
        float* fptr = &y;
        char* cptr = (char*)&ch;
        result_long = func10_alt(z, x, fptr, lg, sh, un, sz, cptr, i, s32);
        
        /* Call 11-parameter function with struct32 */
        s32.x = i * 100LL;
        result_float = func11_struct(x, y, z, arr, ch, sh, lg, ptr, sz, un, s32);
        
        /* Prevent dead code elimination */
        if (result_int > 1000000 || result_double > 1000000.0 || 
            result_long > 1000000 || result_float > 1000000.0f) {
            printf("Unexpected large result\n");
        }
    }
    
    /* Conditional calls based on runtime values */
    int choice;
    for (int i = 0; i < 50; i++) {
        choice = (seed + i * 3) % 4;
        
        switch (choice) {
            case 0:
                result_int = func10(i, y, z, arr, ch, sh, lg, ptr, sz, un);
                break;
            case 1:
                s16.a = i;
                result_double = func11(i, y, z, arr, ch, sh, lg, ptr, sz, un, s16);
                break;
            case 2:
                result_long = func10_alt(z, i, &y, lg, sh, un, sz, (char*)&ch, i, s32);
                break;
            case 3:
                s32.w = i;
                result_float = func11_struct(i, y, z, arr, ch, sh, lg, ptr, sz, un, s32);
                break;
        }
        
        /* Use results to prevent optimization */
        seed += result_int + (int)result_double + (int)result_long + (int)result_float;
    }
    
    printf("Final seed: %d\n", seed);
    printf("Results: int=%d, double=%f, long=%ld, float=%f\n", 
           result_int, result_double, result_long, result_float);
    
    return 0;
}
