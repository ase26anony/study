#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Small structs to pass by value - forces complex parameter passing */
struct Point3D {
    float x, y, z;
    int id;
}; /* 16 bytes on most systems */

struct DataChunk {
    char tag;
    short count;
    int value;
    double timestamp;
}; /* Likely 24 bytes with padding */

/* Mixed-type struct to test alignment */
struct Mixed {
    char c;
    double d;
    int i;
    float f;
}; /* Likely 24 bytes */

/* Function with exactly 10 parameters */
static inline int func10(int a, float b, double c, int* d, char e, 
                         short f, long g, void* h, size_t i, unsigned j) {
    /* Use all parameters to prevent dead code elimination */
    volatile int result = a + (int)b + (int)c + *d + e + f + (int)g;
    result += (int)((size_t)h % 256) + (int)i + (int)j;
    
    /* Complex operations mixing types */
    float temp = b * (float)c;
    *d += (int)temp + a;
    
    return result % 256;
}

/* Function with exactly 11 parameters */
static inline double func11(double a, int b, float c, struct Point3D p, 
                           char* str, short s, long long ll, 
                           unsigned u, size_t sz, void* ptr, int flag) {
    /* Use all parameters */
    volatile double result = a + b + c + p.x + p.y + p.z + p.id;
    
    if (str && *str) {
        result += *str;
    }
    
    result += s + (double)ll + u + sz + (double)((size_t)ptr % 1000);
    
    /* Modify struct parameter (by value, so local copy) */
    p.x += (float)result;
    p.id = flag;
    
    return result * (flag ? -1.0 : 1.0);
}

/* Another 10-parameter function with struct by value */
static inline struct Point3D func10_struct(int a, float b, struct Point3D p1,
                                          struct DataChunk d, double x,
                                          char c, short s, long l,
                                          void* ptr, unsigned mask) {
    struct Point3D result = p1;
    
    /* Combine parameters in various ways */
    result.x += b + (float)x + (float)a;
    result.y += (float)d.value + (float)s;
    result.z += (float)l + (float)((size_t)ptr % 100);
    result.id = (p1.id + a + d.count + c) & mask;
    
    /* Use the DataChunk */
    d.timestamp += x;
    d.value += a;
    
    return result;
}

/* 11-parameter function with multiple structs */
static inline int func11_mixed(struct Mixed m1, int a, double b, float c,
                              struct Point3D p, char* buf, int len,
                              short s, unsigned u, size_t sz, int flag) {
    /* Complex computation using all parameters */
    volatile int result = (int)m1.d + m1.i + (int)m1.f + a + (int)b + (int)c;
    result += (int)p.x + (int)p.y + (int)p.z + p.id;
    
    if (buf && len > 0) {
        for (int i = 0; i < len && i < 10; i++) {
            result += buf[i];
        }
    }
    
    result += s + (int)u + (int)sz;
    
    /* Modify local struct copies */
    m1.c = (char)(result % 256);
    p.x += (float)result;
    
    return result * (flag ? -1 : 1);
}

/* Function pointer types */
typedef int (*Func10Ptr)(int, float, double, int*, char, short, long, void*, size_t, unsigned);
typedef double (*Func11Ptr)(double, int, float, struct Point3D, char*, short, long long, unsigned, size_t, void*, int);
typedef struct Point3D (*Func10StructPtr)(int, float, struct Point3D, struct DataChunk, double, char, short, long, void*, unsigned);

/* Array of function pointers to obfuscate calls */
static Func10Ptr func10_array[] = {func10, NULL};
static Func11Ptr func11_array[] = {func11, NULL};
static Func10StructPtr func10_struct_array[] = {func10_struct, NULL};

/* Helper to create test data */
static struct Point3D make_point(float x, float y, float z, int id) {
    struct Point3D p = {x, y, z, id};
    return p;
}

static struct DataChunk make_chunk(char tag, short count, int value, double ts) {
    struct DataChunk d = {tag, count, value, ts};
    return d;
}

static struct Mixed make_mixed(char c, double d, int i, float f) {
    struct Mixed m = {c, d, i, f};
    return m;
}

int main(void) {
    volatile int seed = 42; /* Prevent constant propagation */
    int counter = 0;
    double total = 0.0;
    
    /* Test data */
    int x = 100;
    int* ptr = &x;
    char str[] = "test";
    struct Point3D point = make_point(1.0f, 2.0f, 3.0f, 1);
    struct DataChunk chunk = make_chunk('A', 10, 1000, 1234.56);
    struct Mixed mixed = make_mixed('X', 3.14159, 42, 2.71828f);
    
    /* Call 10-parameter functions in a loop */
    for (int i = 0; i < 100; i++) {
        /* Vary arguments to prevent optimization */
        int a = seed + i;
        float b = (float)(seed * i) / 100.0f;
        double c = (double)(seed + i * 2) / 50.0;
        char e = (char)('A' + (i % 26));
        short f = (short)(i * 100);
        long g = (long)(i * 1000);
        size_t sz = (size_t)(i * 10000);
        unsigned u = (unsigned)(i * 12345);
        
        /* Direct call to 10-parameter function */
        int r1 = func10(a, b, c, ptr, e, f, g, (void*)(size_t)i, sz, u);
        counter += r1;
        
        /* Call through function pointer */
        if (func10_array[0]) {
            int r2 = func10_array[0](a + 1, b + 1.0f, c + 1.0, 
                                    &x, e + 1, f + 1, g + 1, 
                                    (void*)(size_t)(i + 1), sz + 1, u + 1);
            counter += r2;
        }
        
        /* Call 10-parameter function with structs */
        struct Point3D p2 = make_point((float)i, (float)(i+1), (float)(i+2), i);
        struct DataChunk d2 = make_chunk('B' + (i % 20), (short)i, i * 100, (double)i);
        
        struct Point3D result = func10_struct(a, b, p2, d2, c, e, f, g, ptr, u);
        counter += result.id;
    }
    
    /* Call 11-parameter functions */
    for (int i = 0; i < 50; i++) {
        /* Vary arguments */
        double a = (double)(seed + i) / 10.0;
        int b = seed - i;
        float c = (float)(i * 2) / 3.0f;
        struct Point3D p = make_point((float)i * 0.5f, (float)i * 0.25f, 
                                     (float)i * 0.125f, i * 2);
        short s = (short)(i * 50);
        long long ll = (long long)i * 1000000LL;
        unsigned u = (unsigned)(i * 54321);
        size_t sz = (size_t)(i * 9999);
        int flag = i % 2;
        
        /* Direct call to 11-parameter function */
        double d1 = func11(a, b, c, p, str, s, ll, u, sz, (void*)(size_t)i, flag);
        total += d1;
        
        /* Call through function pointer */
        if (func11_array[0]) {
            double d2 = func11_array[0](a * 2.0, b + 10, c * 1.5f, 
                                       make_point(p.x + 1.0f, p.y + 1.0f, p.z + 1.0f, p.id + 1),
                                       str + (i % 2), s + 1, ll + 1000, 
                                       u + 1, sz + 100, (void*)(size_t)(i + 100), !flag);
            total += d2;
        }
        
        /* Call 11-parameter function with multiple structs */
        struct Mixed m = make_mixed('A' + (i % 26), (double)i / 10.0, 
                                   i * 3, (float)i / 5.0f);
        struct Point3D p3 = make_point((float)i, (float)(i*2), (float)(i*3), i);
        char buffer[20];
        for (int j = 0; j < 20; j++) {
            buffer[j] = (char)('0' + (i + j) % 10);
        }
        
        int r3 = func11_mixed(m, b, a, c, p3, buffer, 10, s, u, sz, flag);
        counter += r3;
    }
    
    /* Conditional calls based on runtime values */
    volatile int choice = seed % 3;
    
    if (choice == 0) {
        /* Another 10-parameter call with different argument pattern */
        int r = func10(seed, (float)seed, (double)seed, ptr, 'Z', 
                      (short)seed, (long)seed, (void*)(size_t)seed, 
                      (size_t)seed, (unsigned)seed);
        counter += r;
    } else if (choice == 1) {
        /* 11-parameter call */
        double d = func11((double)seed, seed, (float)seed, point, 
                         str, (short)seed, (long long)seed, 
                         (unsigned)seed, (size_t)seed, (void*)(size_t)seed, 1);
        total += d;
    } else {
        /* Mixed call */
        int r = func11_mixed(mixed, seed, (double)seed, (float)seed,
                            point, str, 4, (short)seed, 
                            (unsigned)seed, (size_t)seed, 0);
        counter += r;
    }
    
    printf("Result: counter=%d, total=%f\n", counter, total);
    return 0;
}
