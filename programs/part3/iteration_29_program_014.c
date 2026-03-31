#include <stdio.h>
#include <stddef.h>

/* Small structs to pass by value - forces complex parameter passing */
struct Vec2 { float x, y; };
struct Data { int id; double value; char tag; };
struct Point { short x, y, z; };

/* Function with exactly 10 parameters - mixed types */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j)
{
    /* Use all parameters to prevent dead code elimination */
    *d = a + (int)b + (int)c + e + f + (int)g + (int)(size_t)h + (int)i + j;
    float temp = b * c;
    return (int)(temp + a + *d + g + i + j);
}

/* Function with exactly 11 parameters - mixed types */
static inline double func11(int a, float b, double c, int* d, char e,
                           short f, long g, void* h, size_t i, unsigned j,
                           struct Vec2 v)
{
    /* Complex usage of all parameters */
    double result = c * b * v.x * v.y;
    *d = a + (int)result + e + f + (int)g + (int)(size_t)h + (int)i + j;
    result += (double)a + (double)*d + (double)g + (double)i + (double)j;
    return result;
}

/* Another 10-parameter function with struct parameter */
static inline struct Data func10_struct(int a, float b, double c, int* d,
                                       char e, short f, struct Point p,
                                       void* h, size_t i, unsigned j)
{
    struct Data result;
    result.id = a + p.x + p.y + p.z;
    result.value = b * c * (double)result.id;
    result.tag = e + (char)f;
    *d = result.id + (int)result.value + result.tag;
    return result;
}

/* 11-parameter function with multiple structs */
static inline float func11_mixed(int a, float b, double c, int* d,
                                char e, short f, long g, struct Vec2 v1,
                                struct Point p, size_t i, unsigned j)
{
    float result = b + v1.x + v1.y + (float)p.x + (float)p.y + (float)p.z;
    result *= (float)c * (float)a * (float)g;
    *d = (int)result + e + f + (int)(size_t)&v1 + (int)i + j;
    return result;
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned, struct Vec2);
typedef struct Data (*Func10StructPtr)(int, float, double, int*, char, short, struct Point, void*, size_t, unsigned);

/* Array of function pointers to obfuscate calls */
static Func10Ptr func10_array[] = {
    (Func10Ptr)func10,
    (Func10Ptr)func10_struct,  /* Cast is safe due to compatible calling convention */
    NULL
};

static Func11Ptr func11_array[] = {
    func11,
    func11_mixed,
    NULL
};

/* Volatile variables to prevent constant propagation */
static volatile int vol_int = 42;
static volatile float vol_float = 3.14f;
static volatile double vol_double = 2.71828;

int main(void)
{
    int results[10];
    double dresults[10];
    struct Data data_results[10];
    
    /* Initialize some data */
    struct Vec2 vec = {1.5f, 2.5f};
    struct Point point = {10, 20, 30};
    int counter = 0;
    
    /* Call 10-parameter functions in a loop with varying arguments */
    for (int i = 0; i < 5; i++) {
        int local_var = vol_int + i;  /* Prevent optimization */
        float local_float = vol_float * (i + 1);
        double local_double = vol_double / (i + 1);
        
        /* Call func10 directly */
        results[counter++] = func10(
            local_var,              /* int */
            local_float,            /* float */
            local_double,           /* double */
            &results[i],            /* int* */
            'A' + i,               /* char */
            (short)(100 + i),      /* short */
            1000L * i,             /* long */
            (void*)(size_t)i,      /* void* */
            sizeof(int) * i,       /* size_t */
            500u + i               /* unsigned */
        );
        
        /* Call func10_struct */
        data_results[i] = func10_struct(
            local_var,
            local_float,
            local_double,
            &results[i+1],
            'B' + i,
            (short)(200 + i),
            point,                  /* struct Point by value */
            (void*)(size_t)(i * 2),
            sizeof(struct Point) * i,
            600u + i
        );
        
        /* Modify struct for next iteration */
        point.x += 5;
        point.y += 5;
        point.z += 5;
    }
    
    /* Call 11-parameter functions */
    for (int i = 0; i < 5; i++) {
        int local_var = vol_int - i;
        float local_float = vol_float + i;
        double local_double = vol_double * (i + 1);
        
        /* Call func11 */
        dresults[i] = func11(
            local_var,
            local_float,
            local_double,
            &results[i],
            'C' + i,
            (short)(300 + i),
            2000L * i,
            (void*)(size_t)(i * 3),
            sizeof(double) * i,
            700u + i,
            vec                     /* struct Vec2 by value */
        );
        
        /* Call func11_mixed */
        float fresult = func11_mixed(
            local_var,
            local_float,
            local_double,
            &results[i+2],
            'D' + i,
            (short)(400 + i),
            3000L * i,
            vec,                    /* struct Vec2 by value */
            point,                  /* struct Point by value */
            sizeof(float) * i,
            800u + i
        );
        
        dresults[i] += fresult;
        
        /* Modify structs for next iteration */
        vec.x += 0.5f;
        vec.y += 0.5f;
    }
    
    /* Call through function pointers with volatile guard */
    volatile int selector = vol_int % 2;
    
    if (selector == 0 && func10_array[0] != NULL) {
        results[9] = func10_array[0](
            999,
            9.99f,
            99.99,
            &results[8],
            'Z',
            9999,
            99999L,
            (void*)0x1000,
            sizeof(results),
            9999u
        );
    }
    
    if (selector == 1 && func11_array[0] != NULL) {
        dresults[9] = func11_array[0](
            111,
            1.11f,
            11.11,
            &results[7],
            'X',
            1111,
            11111L,
            (void*)0x2000,
            sizeof(dresults),
            1111u,
            (struct Vec2){9.9f, 8.8f}
        );
    }
    
    /* Use results to prevent elimination */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += results[i];
        sum += (int)dresults[i];
        sum += data_results[i % 5].id;
    }
    
    printf("Result: %d\n", sum);
    return sum > 0 ? 0 : 1;
}
