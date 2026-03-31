/* test_resource.c - Coverage test for GCC's resource.cc mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile int g_condition = 1;

/* ==================== ZERO_EXTRACT patterns ==================== */
NOINLINE void test_zero_extract(void) {
    /* Bit-field assignment - should generate ZERO_EXTRACT in RTL */
    struct {
        volatile unsigned int field1 : 3;
        volatile unsigned int field2 : 5;
        volatile unsigned int field3 : 8;
    } bit_struct;
    
    bit_struct.field1 = 5;      /* ZERO_EXTRACT destination */
    bit_struct.field2 = 31;     /* Another bit-field write */
    bit_struct.field3 = 255;    /* Full byte bit-field */
    
    /* Volatile bit-field in union */
    union {
        volatile unsigned int full;
        struct {
            volatile unsigned int low : 16;
            volatile unsigned int high : 16;
        } parts;
    } bit_union;
    
    bit_union.parts.low = 0xABCD;   /* ZERO_EXTRACT for 16-bit field */
    bit_union.parts.high = 0x1234;  /* Another ZERO_EXTRACT */
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(bit_struct.field1), "r"(bit_union.full));
}

/* ==================== STRICT_LOW_PART patterns ==================== */
NOINLINE __attribute__((optimize("O1"))) void test_strict_low_part(void) {
    /* Mixed-size integer assignments - may generate STRICT_LOW_PART */
    volatile short src_short = -12345;
    volatile char src_char = 100;
    volatile int dest_int;
    volatile long dest_long;
    
    /* These assignments might use STRICT_LOW_PART for partial register writes */
    dest_int = src_short;      /* short -> int, lower 16 bits only */
    dest_long = src_char;      /* char -> long, lower 8 bits only */
    
    /* Pointer casting with smaller store */
    volatile int32_t wide = 0x12345678;
    volatile int16_t *half_ptr = (int16_t*)&wide;
    *half_ptr = 0xABCD;        /* Store to lower 16 bits only */
    
    /* Use inline assembly hint for partial register */
    asm volatile("" : "+r"(dest_int), "+r"(dest_long));
}

/* ==================== SUBREG patterns ==================== */
NOINLINE void test_subreg(void) {
    /* GCC vector types - SUBREG common with vector operations */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Vector operations that might use SUBREG */
    vec_a = vec_a + vec_b;          /* Vector operation */
    
    /* Extract lane - might use SUBREG */
    int lane = vec_a[g_index & 3];  /* Variable index lane extraction */
    
    /* Type punning through union - often uses SUBREG */
    union {
        float f;
        int i;
    } pun;
    
    pun.f = 3.14159f;
    pun.i = pun.i ^ 0x80000000;    /* Flip sign bit via integer */
    
    /* Cast between vector types */
    v8hi converted = (v8hi)vec_a;   /* Type conversion */
    
    asm volatile("" : : "r"(lane), "r"(pun.f), "r"(converted));
}

/* ==================== MEM_P patterns ==================== */
NOINLINE void test_mem_p(void) {
    /* Complex memory addresses for MEM destinations */
    int array[100];
    int *ptr;
    
    /* Store with complex address calculation */
    array[g_index * 7 + 3] = g_value;      /* Non-constant index */
    
    /* Pointer arithmetic store */
    ptr = &array[50];
    ptr[g_index - 2] = g_value * 2;        /* Pointer with offset */
    
    /* Struct with pointer member access */
    struct {
        int data[10];
        int count;
    } obj;
    
    obj.data[g_index & 7] = g_value;       /* Struct member array */
    obj.count = g_index;
    
    /* Double indirection */
    int **pptr = &ptr;
    (*pptr)[g_index] = g_value;            /* Double pointer deref */
    
    /* Prevent optimization */
    asm volatile("" : : "r"(array[0]), "r"(obj.count), "r"(**pptr));
}

/* ==================== Combined test function ==================== */
NOINLINE __attribute__((optimize("O2"))) void test_combined(int seed) {
    /* Mix all patterns in one function with control flow */
    volatile int x = seed;
    
    /* Bit-field (ZERO_EXTRACT) */
    struct {
        volatile unsigned bits : 4;
    } bf;
    bf.bits = x & 0xF;
    
    /* Conditional STRICT_LOW_PART-like pattern */
    if (x & 1) {
        volatile short s = x;
        volatile int i = s;      /* Potential STRICT_LOW_PART */
        x = i;
    }
    
    /* Loop with MEM stores */
    int buf[10];
    for (int i = 0; i < 10; i++) {
        buf[i] = x + i;          /* MEM destination in loop */
    }
    
    /* Vector SUBREG usage */
    typedef float v4f __attribute__((vector_size(16)));
    v4f v1 = {x, x+1, x+2, x+3};
    v4f v2 = v1 * 2.0f;
    float f = v2[seed % 4];      /* Potential SUBREG extract */
    
    /* Complex MEM address */
    int *dynamic = &buf[seed % 10];
    dynamic[0] = f;              /* MEM with computed address */
    
    asm volatile("" : : "r"(bf.bits), "r"(x), "r"(buf[0]), "r"(f));
}

/* Helper to create complex address expressions */
NOINLINE int* get_pointer(int *base, int offset) {
    return base + (offset * 2) + (g_condition ? 1 : -1);
}

/* ==================== Main driver ==================== */
int main(void) {
    /* Call test functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_mem_p();
        test_combined(i);
        
        /* Additional MEM_P test with helper */
        int buffer[20];
        int *complex_ptr = get_pointer(buffer, i);
        *complex_ptr = i * 100;      /* MEM store with complex address */
        
        /* Update globals to vary behavior */
        g_index = (g_index * 13 + 7) & 31;
        g_value = (g_value + i) & 0xFF;
        g_condition = i & 1;
    }
    
    return 0;
}
