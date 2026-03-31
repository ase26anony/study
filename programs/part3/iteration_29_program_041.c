#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Small structs to pass by value - forces complex parameter passing */
struct Point2D {
    int x;
    int y;
};

struct DataChunk {
    char a;
    short b;
    int c;
    float d;
};

struct MixedData {
    double value;
    void* ptr;
    int id;
};

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_int = 42;
volatile float g_volatile_float = 3.14f;
volatile double g_volatile_double = 2.71828;

/* Function with exactly 10 parameters - mixed types */
static inline int func_10_params(
    int a, float b, double c, int* d, char e,
    short f, long g, void* h, size_t i, unsigned j
) {
    /* Use all parameters in non-trivial ways to prevent dead code elimination */
    int result = a + (int)b + (int)c + *d + e + f + (int)g;
    result += (int)((size_t)h % 256);  /* Use pointer as integer */
    result += (int)(i % 1000);
    result += (int)(j % 1000);
    
    /* Side effects to prevent optimization */
    *d += result;
    return result;
}

/* Function with exactly 11 parameters - includes struct by value */
static inline double func_11_params(
    struct Point2D p1, int a, float b, double c,
    int* d, char e, short f, struct DataChunk chunk,
    void* h, size_t i, unsigned j
) {
    /* Complex computation using all parameters */
    double result = p1.x * p1.y + a + b + c;
    result += *d + e + f;
    result += chunk.a + chunk.b + chunk.c + chunk.d;
    result += (double)((size_t)h % 1000);
    result += (double)(i % 1000);
    result += (double)(j % 1000);
    
    /* Modify through pointer */
    *d += (int)result;
    
    return result;
}

/* Another 10-parameter function with different signature */
static inline long func_10_params_alt(
    double a, int b, float c, void* d, short e,
    struct MixedData mdata, char f, int* g, size_t h, unsigned i
) {
    long result = (long)a + b + (long)c + (long)d;
    result += e + f + (long)mdata.value + (long)mdata.ptr + mdata.id;
    result += *g + (long)h + (long)i;
    
    *g += (int)result;
    return result;
}

/* Another 11-parameter function */
static inline float func_11_params_alt(
    int a, float b, double c, struct Point2D p1,
    struct Point2D p2, int* d, char e, short f,
    void* h, size_t i, unsigned j
) {
    float result = (float)a + b + (float)c;
    result += (float)(p1.x * p2.y - p1.y * p2.x); /* Cross product */
    result += *d + e + f;
    result += (float)((size_t)h % 1000);
    result += (float)(i % 1000);
    result += (float)(j % 1000);
    
    *d += (int)result;
    return result;
}

/* Function pointer types for obfuscation */
typedef int (*Func10Ptr)(
    int, float, double, int*, char,
    short, long, void*, size_t, unsigned
);

typedef double (*Func11Ptr)(
    struct Point2D, int, float, double,
    int*, char, short, struct DataChunk,
    void*, size_t, unsigned
);

typedef long (*Func10AltPtr)(
    double, int, float, void*, short,
    struct MixedData, char, int*, size_t, unsigned
);

typedef float (*Func11AltPtr)(
    int, float, double, struct Point2D,
    struct Point2D, int*, char, short,
    void*, size_t, unsigned
);

/* Array of function pointers to prevent direct call optimization */
static Func10Ptr func10_array[] = {func_10_params, NULL};
static Func11Ptr func11_array[] = {func_11_params, NULL};
static Func10AltPtr func10_alt_array[] = {func_10_params_alt, NULL};
static Func11AltPtr func11_alt_array[] = {func_11_params_alt, NULL};

/* Helper to get volatile values */
static int get_volatile_int(void) {
    return g_volatile_int;
}

static float get_volatile_float(void) {
    return g_volatile_float;
}

static double get_volatile_double(void) {
    return g_volatile_double;
}

int main(void) {
    int counter = 0;
    int data1 = 100, data2 = 200, data3 = 300, data4 = 400;
    
    /* Initialize structs */
    struct Point2D p1 = {10, 20};
    struct Point2D p2 = {30, 40};
    struct DataChunk chunk = {'A', 123, 456, 7.89f};
    struct MixedData mdata = {3.14159, (void*)0x1000, 999};
    
    /* Call 10-parameter functions in a loop with varying arguments */
    for (int i = 0; i < 100; i++) {
        /* Mix of volatile and constant arguments */
        int result1 = func_10_params(
            get_volatile_int() + i,
            get_volatile_float() + i,
            get_volatile_double() + i,
            &data1,
            'A' + (i % 26),
            (short)(100 + i),
            (long)(1000 + i),
            (void*)(0x1000 + i),
            (size_t)(10000 + i),
            (unsigned)(500 + i)
        );
        
        /* Call through function pointer with volatile guard */
        if (func10_array[0] != NULL) {
            int result2 = func10_array[0](
                i * 2,
                (float)i * 1.5f,
                (double)i * 2.5,
                &data2,
                'Z' - (i % 26),
                (short)(200 - i),
                (long)(2000 - i),
                (void*)(0x2000 + i),
                (size_t)(20000 - i),
                (unsigned)(1000 - i)
            );
            counter += result2;
        }
        
        /* Call alternate 10-parameter function */
        long result3 = func_10_params_alt(
            (double)i * 3.14,
            i * 3,
            (float)i * 2.71f,
            (void*)(0x3000 + i),
            (short)(300 + i),
            mdata,
            'M' + (i % 10),
            &data3,
            (size_t)(30000 + i),
            (unsigned)(1500 + i)
        );
        
        counter += result1 + (int)result3;
    }
    
    /* Call 11-parameter functions */
    for (int i = 0; i < 50; i++) {
        /* Vary struct values */
        p1.x += i;
        p1.y -= i;
        chunk.c += i;
        chunk.d += (float)i;
        
        double result1 = func_11_params(
            p1,
            get_volatile_int() + i * 2,
            get_volatile_float() + i * 0.5f,
            get_volatile_double() + i * 0.25,
            &data1,
            'B' + (i % 25),
            (short)(150 + i),
            chunk,
            (void*)(0x4000 + i),
            (size_t)(40000 + i),
            (unsigned)(2000 + i)
        );
        
        /* Call through function pointer */
        if (func11_array[0] != NULL) {
            double result2 = func11_array[0](
                (struct Point2D){i, i * 2},
                i * 3,
                (float)i * 1.1f,
                (double)i * 1.2,
                &data2,
                'C' + (i % 24),
                (short)(250 + i),
                (struct DataChunk){'X', 111, 222, 3.33f},
                (void*)(0x5000 + i),
                (size_t)(50000 + i),
                (unsigned)(2500 + i)
            );
            counter += (int)result2;
        }
        
        /* Call alternate 11-parameter function */
        float result3 = func_11_params_alt(
            i * 4,
            (float)i * 3.3f,
            (double)i * 4.4,
            (struct Point2D){i * 5, i * 6},
            (struct Point2D){i * 7, i * 8},
            &data4,
            'D' + (i % 23),
            (short)(350 + i),
            (void*)(0x6000 + i),
            (size_t)(60000 + i),
            (unsigned)(3000 + i)
        );
        
        counter += (int)result1 + (int)result3;
    }
    
    /* Mix calls in conditional blocks */
    for (int i = 0; i < 25; i++) {
        if (i % 2 == 0) {
            /* Call 10-param function */
            func_10_params(
                i, (float)i * 0.5f, (double)i * 0.25,
                &data1, 'E', (short)i, (long)i,
                (void*)(uintptr_t)i, (size_t)i, (unsigned)i
            );
        } else {
            /* Call 11-param function */
            func_11_params(
                (struct Point2D){i, i + 1},
                i + 2, (float)(i + 3), (double)(i + 4),
                &data2, 'F', (short)(i + 5),
                (struct DataChunk){'G', (short)(i + 6), i + 7, (float)(i + 8)},
                (void*)(uintptr_t)(i + 9), (size_t)(i + 10), (unsigned)(i + 11)
            );
        }
        
        /* Call through alternate function pointer arrays */
        if (i % 3 == 0 && func10_alt_array[0] != NULL) {
            func10_alt_array[0](
                (double)i, i * 2, (float)i * 1.5f,
                (void*)(uintptr_t)(i * 3), (short)(i * 4),
                mdata, 'H', &data3, (size_t)(i * 5), (unsigned)(i * 6)
            );
        }
        
        if (i % 4 == 0 && func11_alt_array[0] != NULL) {
            func11_alt_array[0](
                i * 7, (float)i * 2.5f, (double)i * 3.5,
                (struct Point2D){i * 8, i * 9},
                (struct Point2D){i * 10, i * 11},
                &data4, 'I', (short)(i * 12),
                (void*)(uintptr_t)(i * 13), (size_t)(i * 14), (unsigned)(i * 15)
            );
        }
    }
    
    printf("Result: %d (data1=%d, data2=%d, data3=%d, data4=%d)\n",
           counter, data1, data2, data3, data4);
    
    return 0;
}
