#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Small structs to pass by value - forces complex parameter passing */
struct Vec2 { float x, y; };
struct Data { int id; char tag; short count; void* ptr; };
struct Mixed { double d; int i; float f; };

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_int = 42;
volatile float g_volatile_float = 3.14f;
volatile void* g_volatile_ptr = NULL;

/* ========== 10-PARAMETER FUNCTIONS ========== */

static inline int func10_a(int a, float b, double c, int* d, char e, 
                          short f, long g, void* h, size_t i, unsigned j)
{
    /* Use all parameters in mixed computations */
    int result = a + (int)b + (int)c + *d + e + f + (int)g;
    result += (int)((uintptr_t)h % 256);
    result += (int)(i % 256) + j;
    
    /* Prevent dead code elimination */
    if (g_volatile_int > 0) {
        *d = result;
    }
    return result;
}

static inline double func10_b(struct Vec2 v1, int a, double b, float c,
                             struct Data d, short e, long f, void* g, 
                             size_t h, unsigned i)
{
    /* Complex computation using all parameters */
    double result = v1.x + v1.y + a + b + c + d.id + e + f;
    result += (double)((uintptr_t)g / 1000.0);
    result += h + i;
    
    /* Use struct fields */
    if (d.ptr != NULL) {
        result += 1.0;
    }
    return result;
}

/* ========== 11-PARAMETER FUNCTIONS ========== */

static inline long func11_a(int a, float b, double c, int* d, char e,
                           short f, long g, void* h, size_t i, unsigned j,
                           struct Mixed m)
{
    /* Use all 11 parameters */
    long result = a + (long)b + (long)c + *d + e + f + g;
    result += (long)((uintptr_t)h % 1024);
    result += i + j + (long)m.d + m.i + (long)m.f;
    
    if (g_volatile_int < 100) {
        *d = (int)result;
    }
    return result;
}

static inline float func11_b(struct Vec2 v1, struct Vec2 v2, int a, double b,
                            float c, struct Data d, short e, long f,
                            void* g, size_t h, unsigned i)
{
    /* Complex computation with 11 params including 2 structs */
    float result = v1.x * v2.x + v1.y * v2.y;
    result += a + (float)b + c + d.id + e + f;
    result += (float)((uintptr_t)g / 10000.0f);
    result += (float)(h + i);
    
    /* Use struct pointer */
    if (d.count > 0) {
        result *= 1.5f;
    }
    return result;
}

/* ========== FUNCTION POINTER ARRAYS ========== */

typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, 
                        void*, size_t, unsigned);

typedef long (*Func11Ptr)(int, float, double, int*, char, short, long,
                         void*, size_t, unsigned, struct Mixed);

static Func10Ptr func10_array[] = {
    (Func10Ptr)func10_a,
    (Func10Ptr)func10_b  /* Note: actual signature differs but compatible for testing */
};

static Func11Ptr func11_array[] = {
    func11_a,
    (Func11Ptr)func11_b  /* Compatible cast for testing */
};

/* ========== HELPER FUNCTIONS ========== */

static void test_10_param_functions(int iterations)
{
    int local_data[10] = {0};
    struct Vec2 v = {1.0f, 2.0f};
    struct Data d = {100, 'A', 5, &local_data[0]};
    
    for (int i = 0; i < iterations; i++) {
        /* Call func10_a with mixed arguments */
        int result1 = func10_a(
            i,                          /* int */
            g_volatile_float + i,       /* float */
            (double)i * 1.5,            /* double */
            &local_data[i % 10],        /* int* */
            'A' + (i % 26),             /* char */
            (short)(i * 2),             /* short */
            (long)i * 1000,             /* long */
            (void*)(uintptr_t)i,        /* void* */
            (size_t)(i * 10),           /* size_t */
            (unsigned)(i * 3)           /* unsigned */
        );
        
        /* Call func10_b with struct by value */
        double result2 = func10_b(
            v,                          /* struct Vec2 by value */
            i + 100,                    /* int */
            (double)i / 2.0,            /* double */
            g_volatile_float,           /* float */
            d,                          /* struct Data by value */
            (short)(i % 100),           /* short */
            (long)i * 2000,             /* long */
            g_volatile_ptr,             /* void* */
            (size_t)(i * 20),           /* size_t */
            (unsigned)(i * 4)           /* unsigned */
        );
        
        /* Prevent elimination of results */
        local_data[i % 10] += result1 + (int)result2;
        
        /* Modify struct for next iteration */
        v.x += 0.5f;
        v.y += 0.25f;
        d.id++;
    }
}

static void test_11_param_functions(int iterations)
{
    int local_data[10] = {0};
    struct Vec2 v1 = {1.0f, 2.0f};
    struct Vec2 v2 = {3.0f, 4.0f};
    struct Data d = {200, 'B', 10, &local_data[0]};
    struct Mixed m = {3.14159, 42, 2.71828f};
    
    for (int i = 0; i < iterations; i++) {
        /* Call func11_a with all 11 parameters */
        long result1 = func11_a(
            i * 2,                      /* int */
            (float)i * 0.5f,            /* float */
            (double)i * 2.5,            /* double */
            &local_data[i % 10],        /* int* */
            'Z' - (i % 26),             /* char */
            (short)(i * 3),             /* short */
            (long)i * 3000,             /* long */
            (void*)(uintptr_t)(i * 100),/* void* */
            (size_t)(i * 30),           /* size_t */
            (unsigned)(i * 5),          /* unsigned */
            m                           /* struct Mixed by value */
        );
        
        /* Call func11_b with 11 parameters including 2 structs */
        float result2 = func11_b(
            v1,                         /* struct Vec2 by value */
            v2,                         /* struct Vec2 by value */
            i + 200,                    /* int */
            (double)i / 3.0,            /* double */
            g_volatile_float * 2.0f,    /* float */
            d,                          /* struct Data by value */
            (short)(i % 200),           /* short */
            (long)i * 4000,             /* long */
            &local_data[0],             /* void* */
            (size_t)(i * 40),           /* size_t */
            (unsigned)(i * 6)           /* unsigned */
        );
        
        /* Use results to prevent elimination */
        local_data[i % 10] += (int)result1 + (int)result2;
        
        /* Modify data for next iteration */
        v1.x += 0.1f;
        v1.y += 0.2f;
        v2.x -= 0.1f;
        v2.y -= 0.2f;
        m.d += 0.01;
        m.i++;
    }
}

/* ========== FUNCTION POINTER DISPATCH ========== */

static void dispatch_via_function_pointers(int selector)
{
    int local_int = g_volatile_int;
    float local_float = g_volatile_float;
    int data[5] = {1, 2, 3, 4, 5};
    struct Mixed m = {1.0, 2, 3.0f};
    
    /* Use volatile to prevent compile-time resolution */
    volatile int use_10_param = (selector % 2 == 0);
    volatile int func_index = selector % 2;
    
    if (use_10_param && func_index < 2) {
        /* Call through 10-parameter function pointer */
        int result = func10_array[func_index](
            local_int,
            local_float,
            (double)local_int,
            &data[0],
            'X',
            (short)local_int,
            (long)local_int * 10,
            (void*)(uintptr_t)local_int,
            (size_t)local_int * 20,
            (unsigned)local_int * 30
        );
        data[0] = result;
    } else if (func_index < 2) {
        /* Call through 11-parameter function pointer */
        long result = func11_array[func_index](
            local_int,
            local_float,
            (double)local_int,
            &data[1],
            'Y',
            (short)(local_int + 1),
            (long)local_int * 20,
            (void*)(uintptr_t)(local_int + 2),
            (size_t)local_int * 40,
            (unsigned)local_int * 50,
            m
        );
        data[1] = (int)result;
    }
}

/* ========== MAIN FUNCTION ========== */

int main(void)
{
    printf("Testing 10 and 11 parameter function expansion...\n");
    
    /* Test direct calls */
    test_10_param_functions(100);
    test_11_param_functions(100);
    
    /* Test function pointer dispatch */
    for (int i = 0; i < 50; i++) {
        dispatch_via_function_pointers(i);
    }
    
    /* Additional mixed usage to encourage inlining */
    {
        int temp = 0;
        struct Vec2 v = {1.0f, 2.0f};
        struct Data d = {300, 'C', 15, &temp};
        
        /* Inline expansion candidates */
        for (int i = 0; i < 20; i++) {
            temp += func10_a(i, i*0.5f, i*1.5, &temp, 'A'+i, i, i*1000, 
                           (void*)(uintptr_t)i, i*10, i*3);
            
            struct Mixed m = {i*0.1, i, i*0.2f};
            temp += (int)func11_a(i, i*0.3f, i*2.0, &temp, 'B'+i, i*2, i*2000,
                                (void*)(uintptr_t)(i*10), i*20, i*4, m);
        }
        
        printf("Final temp value: %d\n", temp);
    }
    
    return 0;
}
