#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Small structs to pass by value - forces complex parameter passing */
struct Vec2 { float x, y; };
struct Data { int a; char b; short c; void* ptr; };
struct Mixed { double d; int i; float f; };

/* Volatile variables to prevent constant propagation */
volatile int v1 = 1;
volatile float v2 = 2.0f;
volatile double v3 = 3.0;
volatile void* vptr = NULL;

/* Function with exactly 10 parameters - mixed types */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j)
{
    /* Use all parameters to prevent dead code elimination */
    int sum = a + (int)b + (int)c + *d + e + f + (int)g;
    sum += (int)((uintptr_t)h % 256);
    sum += (int)i + (int)j;
    
    /* Complex arithmetic mixing types */
    double result = (double)a * b + c / (double)(*d + 1);
    result += (double)e * f * g;
    
    /* Pointer arithmetic */
    if (h != NULL) {
        char* ch = (char*)h;
        sum += ch[0];
    }
    
    return sum + (int)result;
}

/* Function with exactly 11 parameters - includes struct by value */
static inline double func11(int a, struct Vec2 v, double c, int* d, 
                           char e, short f, long g, void* h, 
                           size_t i, unsigned j, struct Data data)
{
    /* Use all parameters extensively */
    double total = (double)a + v.x + v.y + c;
    total += (double)(*d) + (double)e + (double)f + (double)g;
    total += (double)((uintptr_t)h % 1000);
    total += (double)i + (double)j;
    total += (double)data.a + (double)data.b + (double)data.c;
    
    /* Complex operations */
    float float_part = v.x * v.y * (float)c;
    int int_part = a * *d * e * f;
    
    /* Struct member access */
    if (data.ptr != NULL) {
        int_part += *(int*)data.ptr;
    }
    
    return total + float_part + int_part;
}

/* Another 10-parameter function with different signature */
static inline long func10_alt(struct Mixed m, int a, float b, double c, 
                             int* d, char e, short f, long g, void* h, size_t i)
{
    long result = (long)m.d + m.i + (long)m.f;
    result += a + (long)b + (long)c + *d + e + f + g;
    result += (long)((uintptr_t)h % 1024);
    result += (long)i;
    
    /* Cross-type operations */
    double temp = m.d * (double)a + (double)m.i / b;
    result += (long)temp;
    
    return result;
}

/* Another 11-parameter function */
static inline float func11_alt(int a, int b, int c, int d, int e,
                              float f1, float f2, double d1, double d2,
                              struct Vec2 v, struct Data data)
{
    float result = f1 + f2 + (float)d1 + (float)d2 + v.x + v.y;
    result += (float)(a + b + c + d + e);
    result += (float)data.a + (float)data.b + (float)data.c;
    
    /* Complex floating point operations */
    result = result * (f1 / f2) + (float)(d1 - d2);
    result += v.x * v.y * 0.5f;
    
    return result;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(int, struct Vec2, double, int*, char, short, long, void*, size_t, unsigned, struct Data);
typedef long (*Func10AltPtr)(struct Mixed, int, float, double, int*, char, short, long, void*, size_t);
typedef float (*Func11AltPtr)(int, int, int, int, int, float, float, double, double, struct Vec2, struct Data);

/* Array of function pointers */
Func10Ptr func10_array[] = {func10, NULL};
Func11Ptr func11_array[] = {func11, NULL};
Func10AltPtr func10_alt_array[] = {func10_alt, NULL};
Func11AltPtr func11_alt_array[] = {func11_alt, NULL};

int main(void)
{
    int local_int = 42;
    int* int_ptr = &local_int;
    struct Vec2 vec = {1.5f, 2.5f};
    struct Data data = {100, 'A', 200, &local_int};
    struct Mixed mixed = {3.14159, 42, 2.71828f};
    
    long total = 0;
    double dtotal = 0.0;
    
    /* Call functions directly in a loop - forces RTL expansion */
    for (int i = 0; i < 100; i++) {
        /* Mix volatile and non-volatile arguments */
        int arg1 = v1 + i;
        float arg2 = v2 + (float)i;
        double arg3 = v3 + (double)i;
        
        /* Direct calls with 10 parameters */
        total += func10(arg1, arg2, arg3, int_ptr, 
                       (char)('A' + i), (short)i, (long)i * 10,
                       vptr, (size_t)i, (unsigned)i * 2);
        
        /* Direct calls with 11 parameters including struct by value */
        dtotal += func11(arg1, vec, arg3, int_ptr,
                        (char)('B' + i), (short)(i * 2), (long)i * 20,
                        vptr, (size_t)i * 3, (unsigned)i * 4, data);
        
        /* Alternate function calls */
        if (i % 2 == 0) {
            total += func10_alt(mixed, arg1, arg2, arg3, int_ptr,
                               (char)('C' + i), (short)(i * 3), (long)i * 30,
                               vptr, (size_t)i * 4);
        }
        
        if (i % 3 == 0) {
            dtotal += func11_alt(arg1, arg1 + 1, arg1 + 2, arg1 + 3, arg1 + 4,
                                arg2, arg2 * 2.0f, arg3, arg3 * 1.5,
                                vec, data);
        }
    }
    
    /* Call through function pointers - prevents optimization */
    int selector = v1 % 4;
    
    for (int i = 0; i < 50; i++) {
        switch (selector) {
            case 0:
                if (func10_array[0]) {
                    total += func10_array[0](i, (float)i, (double)i, int_ptr,
                                           (char)i, (short)i, (long)i,
                                           vptr, (size_t)i, (unsigned)i);
                }
                break;
            case 1:
                if (func11_array[0]) {
                    dtotal += func11_array[0](i, vec, (double)i, int_ptr,
                                            (char)i, (short)i, (long)i,
                                            vptr, (size_t)i, (unsigned)i, data);
                }
                break;
            case 2:
                if (func10_alt_array[0]) {
                    total += func10_alt_array[0](mixed, i, (float)i, (double)i, int_ptr,
                                                (char)i, (short)i, (long)i,
                                                vptr, (size_t)i);
                }
                break;
            case 3:
                if (func11_alt_array[0]) {
                    dtotal += func11_alt_array[0](i, i+1, i+2, i+3, i+4,
                                                 (float)i, (float)(i*2),
                                                 (double)i, (double)(i*1.5),
                                                 vec, data);
                }
                break;
        }
        
        selector = (selector + 1) % 4;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: total = %ld, dtotal = %f\n", total, dtotal);
    
    return (int)(total + (long)dtotal) % 256;
}
