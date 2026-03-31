#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Small structs to pass by value - forcing complex parameter passing */
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
    char w;
    int pad;
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
    /* Use all parameters to prevent dead code elimination */
    int result = a + (int)b + (int)c;
    if (d) result += *d;
    result += e + f + (int)g;
    if (h) result += (int)((uintptr_t)h & 0xFF);
    result += (int)i + (int)j;
    
    /* Complex arithmetic mixing types */
    float float_res = b * (float)c + (float)a;
    double double_res = c * (double)b + (double)result;
    
    return (int)(float_res + double_res) % 256;
}

/* Function with exactly 11 parameters - including struct by value */
static inline double func11(int a, struct Data16 s1, float b, double c, 
                           int* d, char e, short f, struct Data24 s2, 
                           size_t i, unsigned j, long k)
{
    /* Use all parameters extensively */
    double result = (double)a + (double)s1.a + (double)s1.b + 
                   (double)s1.c + (double)s1.d;
    result += b + c;
    
    if (d) result += (double)*d;
    result += (double)e + (double)f;
    result += (double)s2.x + (double)s2.y + (double)s2.z + (double)s2.w;
    result += (double)i + (double)j + (double)k;
    
    /* Cross-type operations */
    result = result * c / (b + 1.0f);
    result += s1.c * s2.x;
    
    return result;
}

/* Another 10-parameter function with different signature */
static inline void* func10_ptr(void* p1, int a, float b, double c, 
                              struct Data16 s, int* d, char e, 
                              short f, long g, size_t h)
{
    /* Use parameters to compute an address offset */
    int offset = a + (int)b + (int)c + s.a + s.b;
    if (d) offset += *d;
    offset += e + f + (int)g + (int)h;
    
    /* Return pointer with offset */
    return (char*)p1 + (offset % 64);
}

/* Another 11-parameter function */
static inline int func11_mixed(double a, float b, int c, struct Data24 s1,
                              int* d, char e, short f, long g, 
                              void* h, size_t i, struct Data16 s2)
{
    int result = (int)a + (int)b + c;
    result += (int)s1.x + s1.y + s1.z + s1.w;
    if (d) result ^= *d;
    result += e * f + (int)g;
    if (h) result += (int)((uintptr_t)h & 0xFFFF);
    result += (int)i + s2.a + s2.b;
    
    return result;
}

/* Function pointer types */
typedef int (*Func10Type)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Type)(int, struct Data16, float, double, int*, char, short, struct Data24, size_t, unsigned, long);
typedef void* (*Func10PtrType)(void*, int, float, double, struct Data16, int*, char, short, long, size_t);

/* Array of function pointers to obfuscate calls */
static Func10Type func10_array[] = {func10, NULL};
static Func11Type func11_array[] = {func11, NULL};
static Func10PtrType func10_ptr_array[] = {func10_ptr, NULL};

/* Helper to create structs */
static struct Data16 make_data16(int a, int b, float c, float d) {
    struct Data16 s = {a, b, c, d};
    return s;
}

static struct Data24 make_data24(double x, int y, short z, char w) {
    struct Data24 s = {x, y, z, w, 0};
    return s;
}

int main() {
    int result = 0;
    double dresult = 0.0;
    void* presult = NULL;
    
    int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    struct Data16 s1 = make_data16(100, 200, 3.14f, 2.718f);
    struct Data24 s2 = make_data24(1.618, 42, 7, 'A');
    
    /* Call 10-parameter function multiple times with different args */
    for (int i = 0; i < 100; i++) {
        /* Mix compile-time constants with volatile variables */
        result += func10(
            v1 + i,            /* int */
            v2 + (float)i,     /* float */
            v3 + (double)i,    /* double */
            &array[i % 10],    /* int* */
            (char)('A' + i),   /* char */
            (short)(i * 2),    /* short */
            (long)i * 100L,    /* long */
            (void*)&array,     /* void* */
            (size_t)i * 10,    /* size_t */
            (unsigned)i * 20   /* unsigned */
        );
        
        /* Also call through function pointer */
        if (func10_array[0]) {
            result += func10_array[0](
                i, 2.0f, 3.0, &array[0], 'X', 100, 1000L, 
                (void*)&result, (size_t)i, (unsigned)result
            );
        }
    }
    
    /* Call 11-parameter function */
    for (int i = 0; i < 50; i++) {
        dresult += func11(
            v1 + i * 2,
            make_data16(i, i+1, (float)i*0.5f, (float)i*0.25f),
            v2 + (float)(i % 10),
            v3 + (double)(i / 10),
            &array[i % 10],
            (char)('Z' - i),
            (short)(i * 3),
            make_data24((double)i, i*2, (short)(i%100), (char)('A' + i%26)),
            (size_t)i * 100,
            (unsigned)i * 200,
            (long)i * 1000
        );
        
        /* Call through function pointer array */
        if (func11_array[0]) {
            dresult += func11_array[0](
                i, s1, 1.5f, 2.5, &array[5], 'M', 500, s2,
                (size_t)result, (unsigned)dresult, 9999L
            );
        }
    }
    
    /* Call the pointer-returning 10-parameter function */
    for (int i = 0; i < 25; i++) {
        presult = func10_ptr(
            (void*)&array,
            i * 10,
            (float)i * 0.1f,
            (double)i * 0.01,
            make_data16(i*2, i*3, (float)i*0.2f, (float)i*0.3f),
            &array[i % 10],
            (char)('a' + i),
            (short)(50 - i),
            (long)i * 500L,
            (size_t)i * 1000
        );
        
        /* Use the result to prevent optimization */
        if (presult) {
            result += *(int*)presult;
        }
    }
    
    /* Call the mixed 11-parameter function */
    for (int i = 0; i < 30; i++) {
        result += func11_mixed(
            (double)i * 1.5,
            (float)i * 0.75f,
            i * 3,
            make_data24((double)i/2.0, i+10, (short)(i*2), (char)('G' + i)),
            &array[i % 10],
            (char)('k' + i),
            (short)(100 + i),
            (long)i * 2000L,
            (void*)&dresult,
            (size_t)i * 3000,
            make_data16(i*4, i*5, (float)i*0.4f, (float)i*0.5f)
        );
    }
    
    printf("Result: %d\n", result);
    printf("Double result: %f\n", dresult);
    printf("Final check: %p\n", presult);
    
    return result % 256;
}
