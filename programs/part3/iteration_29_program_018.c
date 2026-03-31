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
    char w[6];
};

/* Volatile variables to prevent constant propagation */
volatile int v1 = 1;
volatile float v2 = 2.0f;
volatile double v3 = 3.0;
volatile int* v4 = NULL;

/* Function with exactly 10 parameters - mixed types */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters in non-trivial ways */
    int sum = a + (int)b + (int)c + (*d ? *d : 0) + e + f + (int)g;
    float prod = b * (float)c * (float)(g & 0xFF);
    double diff = c - (double)b - (double)a;
    
    /* Force pointer usage */
    if (h != NULL) {
        *(int*)h = sum;
    }
    
    /* Complex return combining multiple types */
    return (sum & 0xFF) + (int)(prod * 100.0f) + (int)(diff * 10.0) + 
           (int)(i % 256) + (j % 256);
}

/* Function with exactly 11 parameters - includes struct by value */
static inline double func11(int a, struct Data16 s1, float b, double c, 
                           int* d, char e, short f, struct Data24 s2, 
                           size_t i, unsigned j, long k) {
    /* Use all parameters including struct fields */
    double base = (double)a + s1.a + s1.b + s1.c + s1.d + b + c;
    
    if (d != NULL) {
        base += *d;
    }
    
    base += e + f + s2.x + s2.y + s2.z;
    
    /* Use struct array elements */
    for (int idx = 0; idx < 6; idx++) {
        base += s2.w[idx];
    }
    
    base += i + j + k;
    
    /* Complex computation to prevent optimization */
    return base * (1.0 + (k % 100) / 1000.0) / 
           (1.0 + (a % 50) / 500.0);
}

/* Another 10-parameter function with different signature */
static inline long func10_alt(double a, int b, float c, void* d, 
                             short e, long f, size_t g, char h, 
                             unsigned i, int* j) {
    /* Force usage of all parameters */
    long result = (long)(a * 1000.0) + b * 100 + (long)(c * 10.0f);
    result += (d != NULL) ? (long)((uintptr_t)d % 1000) : 0;
    result += e * f + g + h + i;
    
    if (j != NULL) {
        *j = (int)(result % 10000);
    }
    
    return result;
}

/* Another 11-parameter function */
static inline float func11_alt(struct Data24 s1, int a, float b, 
                              double c, int* d, short e, long f, 
                              void* g, size_t h, char i, unsigned j) {
    float total = s1.x + s1.y + s1.z;
    
    for (int idx = 0; idx < 6; idx++) {
        total += s1.w[idx];
    }
    
    total += a + b + (float)c;
    
    if (d != NULL) {
        total += *d;
    }
    
    total += e + f + (g != NULL ? 1.0f : 0.0f) + h + i + j;
    
    /* Prevent simple optimization */
    return total * (1.0f + (j % 10) / 100.0f) - 
           (float)((a * b) % 100) / 50.0f;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, 
                        long, void*, size_t, unsigned);
                        
typedef double (*Func11Ptr)(int, struct Data16, float, double, 
                           int*, char, short, struct Data24, 
                           size_t, unsigned, long);
                           
typedef long (*Func10AltPtr)(double, int, float, void*, short, 
                            long, size_t, char, unsigned, int*);
                            
typedef float (*Func11AltPtr)(struct Data24, int, float, double, 
                             int*, short, long, void*, size_t, 
                             char, unsigned);

/* Array of function pointers */
static Func10Ptr func10_array[] = {func10, (Func10Ptr)func10_alt};
static Func11Ptr func11_array[] = {func11, (Func11Ptr)func11_alt};

/* Helper to create test structs */
static struct Data16 make_data16(int seed) {
    struct Data16 d;
    d.a = seed;
    d.b = seed * 2;
    d.c = (float)seed / 10.0f;
    d.d = (float)seed / 20.0f;
    return d;
}

static struct Data24 make_data24(int seed) {
    struct Data24 d;
    d.x = (double)seed * 1.5;
    d.y = seed + 100;
    d.z = (short)(seed % 1000);
    for (int i = 0; i < 6; i++) {
        d.w[i] = (char)((seed + i) % 128);
    }
    return d;
}

int main() {
    int result_int;
    double result_double;
    long result_long;
    float result_float;
    
    int local_int = 42;
    float local_float = 3.14159f;
    double local_double = 2.71828;
    int local_array[10] = {0};
    void* local_ptr = &local_array;
    
    /* Call 10-parameter functions directly */
    for (int i = 0; i < 100; i++) {
        /* Mix of constants and volatile variables */
        result_int = func10(
            v1 + i,                    /* int */
            v2 + (float)i,             /* float */
            v3 + (double)i,            /* double */
            &local_int,                /* int* */
            (char)(i % 128),           /* char */
            (short)(i * 2),            /* short */
            (long)i * 1000L,           /* long */
            local_ptr,                 /* void* */
            (size_t)i * 10,            /* size_t */
            (unsigned)i * 20           /* unsigned */
        );
        
        /* Alternate 10-parameter function */
        result_long = func10_alt(
            local_double + i,          /* double */
            v1 * i,                    /* int */
            local_float,               /* float */
            (i % 3) ? local_ptr : NULL,/* void* */
            (short)(i % 1000),         /* short */
            (long)i * i,               /* long */
            (size_t)i << 2,            /* size_t */
            (char)(65 + i % 26),       /* char */
            (unsigned)i * 30,          /* unsigned */
            &local_int                 /* int* */
        );
        
        /* Store results to prevent elimination */
        local_array[i % 10] += result_int + (int)result_long;
    }
    
    /* Call 11-parameter functions directly */
    for (int i = 0; i < 100; i++) {
        struct Data16 d16 = make_data16(i);
        struct Data24 d24 = make_data24(i);
        
        result_double = func11(
            i * 2,                     /* int */
            d16,                       /* struct Data16 by value */
            local_float * i,           /* float */
            local_double / (i + 1),    /* double */
            (i % 5) ? &local_int : NULL, /* int* */
            (char)(70 + i % 20),       /* char */
            (short)(500 + i),          /* short */
            d24,                       /* struct Data24 by value */
            (size_t)i * 100,           /* size_t */
            (unsigned)i * 200,         /* unsigned */
            (long)i * 3000L            /* long */
        );
        
        result_float = func11_alt(
            d24,                       /* struct Data24 by value */
            v1 + i * 3,                /* int */
            v2 * (float)(i + 1),       /* float */
            v3 - (double)i,            /* double */
            &local_int,                /* int* */
            (short)(1000 - i),         /* short */
            (long)i * 4000L,           /* long */
            local_ptr,                 /* void* */
            (size_t)i * 50,            /* size_t */
            (char)(97 + i % 26),       /* char */
            (unsigned)i * 60           /* unsigned */
        );
        
        /* Use results */
        local_array[i % 10] += (int)result_double + (int)result_float;
    }
    
    /* Call through function pointers - prevents inlining decisions */
    int selector = local_array[0] % 4;
    
    for (int i = 0; i < 50; i++) {
        struct Data16 d16 = make_data16(i + 100);
        struct Data24 d24 = make_data24(i + 200);
        
        switch (selector) {
            case 0:
                result_int = func10_array[0](
                    i, v2, v3, &local_int, 'A', 100, 1000L,
                    local_ptr, i * 2, i * 3
                );
                break;
            case 1:
                result_double = func11_array[0](
                    i, d16, local_float, local_double,
                    &local_int, 'B', 200, d24,
                    i * 4, i * 5, i * 6L
                );
                break;
            case 2:
                result_long = ((Func10AltPtr)func10_array[1])(
                    local_double, i, local_float, local_ptr,
                    300, 4000L, i * 8, 'C', i * 9, &local_int
                );
                break;
            case 3:
                result_float = ((Func11AltPtr)func11_array[1])(
                    d24, i, local_float, local_double,
                    &local_int, 500, 6000L, local_ptr,
                    i * 10, 'D', i * 11
                );
                break;
        }
        
        selector = (selector + 1) % 4;
        local_int += result_int + (int)result_double + 
                    (int)result_long + (int)result_float;
    }
    
    /* Final output to prevent dead code elimination */
    printf("Results: %d %f %ld %f\n", 
           local_int, local_double, result_long, result_float);
    
    return local_array[0] % 256;
}
