/* test_resource.c - Coverage test for GCC resource.cc mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 0;
volatile int g_value = 42;
volatile int g_temp = 0;

/* ========== ZERO_EXTRACT patterns ========== */
NOINLINE void test_zero_extract(void) {
    /* Bit-field assignment - should generate ZERO_EXTRACT in RTL */
    struct {
        volatile unsigned int field1 : 3;
        volatile unsigned int field2 : 5;
        volatile unsigned int field3 : 8;
    } bit_struct;
    
    bit_struct.field1 = 5;      /* ZERO_EXTRACT destination */
    bit_struct.field2 = 31;     /* Another bit-field write */
    bit_struct.field3 = g_value & 0xFF;
    
    /* Volatile bit-field in union */
    union {
        volatile uint32_t full;
        struct {
            volatile uint32_t low : 10;
            volatile uint32_t mid : 10;
            volatile uint32_t high : 12;
        } bits;
    } u;
    
    u.bits.low = 511;           /* ZERO_EXTRACT */
    u.bits.mid = g_value;       /* ZERO_EXTRACT with variable */
    u.full = u.full ^ 0x1234;   /* Force use */
}

/* ========== STRICT_LOW_PART patterns ========== */
/* Compile with -m32 for better STRICT_LOW_PART generation */
NOINLINE __attribute__((optimize("O1"))) 
void test_strict_low_part(int x) {
    volatile short src16;
    volatile char src8;
    int dest32;
    
    /* Mixed-size assignments that may generate STRICT_LOW_PART */
    src16 = x & 0xFFFF;
    dest32 = src16;             /* Possible STRICT_LOW_PART */
    
    src8 = x & 0xFF;
    dest32 = src8;              /* Another possible STRICT_LOW_PART */
    
    /* Inline assembly forcing partial register write */
    asm volatile (
        "movw %w1, %0\n\t"      /* 16-bit write to 32-bit register */
        : "=r" (dest32)
        : "r" (src16)
    );
    
    g_temp = dest32;
}

/* ========== SUBREG patterns ========== */
NOINLINE void test_subreg(void) {
    /* GCC vector types - SUBREG common with vectors */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v8hi vec_short;
    
    /* Vector operations generate SUBREG */
    vec_a = vec_a + vec_b;      /* May have SUBREG in SET_DEST */
    
    /* Type punning through union */
    union {
        float f;
        int i;
        char c[4];
    } pun;
    
    pun.f = 3.14f;
    pun.i = pun.i ^ 0x80000000; /* SUBREG may appear here */
    
    /* Cast between different integer sizes */
    int32_t i32 = 0x12345678;
    int16_t i16 = i32;          /* Truncation - SUBREG */
    int64_t i64 = i32;          /* Extension - may use SUBREG */
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r" (i16), "+r" (i64));
}

/* ========== MEM_P patterns ========== */
NOINLINE void test_mem_dest(int *arr, int n) {
    /* Complex memory destinations */
    for (int i = 0; i < n; i++) {
        arr[i * 2] = g_value + i;      /* MEM with index computation */
    }
    
    /* Pointer arithmetic creating complex address */
    int *ptr = arr + g_index;
    *(ptr + 3) = 99;                    /* MEM destination */
    
    /* Struct member through pointer */
    struct Point {
        int x;
        int y;
        int z;
    } points[4];
    
    struct Point *p = &points[g_index % 4];
    p->y = g_value;                     /* MEM with struct offset */
    
    /* Multi-dimensional array */
    int matrix[3][3];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            matrix[i][j] = i * 10 + j;  /* MEM with 2D indexing */
        }
    }
}

/* ========== Combined test function ========== */
NOINLINE __attribute__((optimize("O2")))
void combined_test(int *arr, int size) {
    /* Mix all patterns in one function */
    
    /* ZERO_EXTRACT */
    struct {
        volatile unsigned bits : 4;
    } bf;
    bf.bits = (g_value & 0x0F);
    
    /* STRICT_LOW_PART (short to int) */
    volatile short s = 0xABCD;
    int i = s;
    
    /* SUBREG via vector */
    typedef float v4f __attribute__((vector_size(16)));
    v4f v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4f v2 = v1 * 2.0f;
    float f = v2[0];  /* Element extraction - SUBREG */
    
    /* MEM destination with complex address */
    if (size > 0) {
        arr[(g_index * 17) % size] = i + (int)f;
    }
    
    /* Another bit-field for good measure */
    volatile struct {
        unsigned a : 2;
        unsigned b : 6;
        unsigned c : 8;
    } multi_bf;
    
    multi_bf.a = 1;
    multi_bf.b = 63;
    multi_bf.c = 255;
}

/* ========== Main driver ========== */
int main(void) {
    int array[100];
    
    /* Initialize array with non-zero values */
    for (int i = 0; i < 100; i++) {
        array[i] = i * 3;
    }
    
    /* Call all test functions multiple times with different data */
    for (int iter = 0; iter < 10; iter++) {
        test_zero_extract();
        test_strict_low_part(iter * 100);
        test_subreg();
        test_mem_dest(array, 50);
        combined_test(array, 100);
        
        /* Modify globals to change behavior */
        g_index = (g_index + 7) % 50;
        g_value = g_value * 13 + 1;
    }
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += array[i];
    }
    
    return sum > 0 ? 0 : 1;
}
