/* test_resource.c - Coverage test for GCC's resource.cc mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile int g_condition = 1;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Bit-field structure - should generate ZERO_EXTRACT in RTL */
struct bitfield_struct {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
};

NOINLINE void test_zero_extract(void) {
    /* Volatile to prevent optimization */
    volatile struct bitfield_struct bf;
    
    /* These assignments should generate ZERO_EXTRACT as SET_DEST */
    bf.field1 = 5;      /* ZERO_EXTRACT: writing to 3-bit field */
    bf.field2 = 20;     /* ZERO_EXTRACT: writing to 5-bit field */
    bf.field3 = 100;    /* ZERO_EXTRACT: writing to 8-bit field */
    
    /* Use asm to prevent dead code elimination */
    asm volatile("" : : "r"(bf));
}

/* Another ZERO_EXTRACT pattern using unions */
union mixed_bf {
    struct {
        unsigned short low : 4;
        unsigned short high : 12;
    } bits;
    unsigned short full;
};

NOINLINE void test_zero_extract_union(void) {
    volatile union mixed_bf u;
    u.bits.low = 7;     /* ZERO_EXTRACT: 4-bit field */
    u.bits.high = 255;  /* ZERO_EXTRACT: 12-bit field */
    asm volatile("" : : "r"(u.full));
}

/* ==================== STRICT_LOW_PART patterns ==================== */

/* Force partial register writes - more common in 32-bit */
NOINLINE void test_strict_low_part(void) {
    volatile short src16 = 0x1234;
    volatile char src8 = 0x56;
    
    /* These should generate STRICT_LOW_PART on x86 */
    int dest32;
    dest32 = src16;     /* STRICT_LOW_PART: short -> int */
    dest32 = src8;      /* STRICT_LOW_PART: char -> int */
    
    /* Mix with arithmetic to prevent optimization */
    dest32 = dest32 + src16;
    asm volatile("" : : "r"(dest32));
}

/* Use explicit types to encourage partial writes */
NOINLINE int test_partial_reg_mix(int a, short b, char c) {
    int result = a;
    result = b;         /* STRICT_LOW_PART: short -> int */
    result = c;         /* STRICT_LOW_PART: char -> int */
    return result + a + b + c;
}

/* ==================== SUBREG patterns ==================== */

/* GCC vector types often generate SUBREG */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE void test_subreg_vectors(void) {
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    
    /* Vector operations */
    v4si vec_c = vec_a + vec_b;
    
    /* Extract element - should generate SUBREG */
    int element = vec_c[0];  /* SUBREG: extracting from vector */
    
    /* Store to vector element - SET_DEST with SUBREG */
    vec_c[2] = element;      /* SUBREG as destination */
    
    /* Type punning through union */
    union {
        v4si vec;
        int arr[4];
    } u;
    u.vec = vec_c;
    u.arr[1] = 99;           /* SUBREG access */
    
    asm volatile("" : : "r"(vec_c), "r"(element), "r"(u.arr[0]));
}

/* Mixed size operations */
NOINLINE void test_subreg_mixed(void) {
    volatile float f = 3.14f;
    volatile double d = 2.71828;
    
    /* Type punning - can generate SUBREG */
    uint32_t float_bits;
    memcpy(&float_bits, &f, sizeof(float_bits));
    
    uint64_t double_bits;
    memcpy(&double_bits, &d, sizeof(double_bits));
    
    /* Cast through union - another SUBREG source */
    union {
        float f;
        uint32_t i;
    } fu;
    fu.f = f;
    fu.i = fu.i ^ 0x80000000;  /* Flip sign bit - SUBREG in RTL */
    
    asm volatile("" : : "r"(float_bits), "r"(double_bits), "r"(fu.i));
}

/* ==================== MEM_P patterns ==================== */

/* Complex memory addresses */
NOINLINE void test_mem_complex_address(int *base, int offset) {
    /* Various MEM destinations with complex addresses */
    base[offset] = g_value;                     /* MEM with index */
    base[offset * 2 + 1] = g_value + 1;         /* More complex address */
    *(base + (offset & 0x3)) = g_value * 2;     /* Masked index */
    
    /* Pointer arithmetic */
    int *ptr = base + g_index;
    *ptr = 123;                                 /* MEM with computed address */
    
    /* Struct member through pointer */
    struct data {
        int x;
        int y;
        int z;
    };
    struct data d;
    struct data *dptr = &d;
    dptr->y = 456;                              /* MEM with field offset */
    
    asm volatile("" : : "r"(base[0]), "r"(d.x));
}

/* Array with computed index */
NOINLINE void test_mem_array_store(void) {
    int array[100];
    volatile int idx = g_index;
    
    /* Store with variable index - MEM destination */
    array[idx] = g_value;                       /* MEM: array[index] */
    array[idx * 3 % 100] = g_value + idx;       /* MEM: more complex index */
    
    /* Nested array access */
    int matrix[10][10];
    int i = idx % 10;
    int j = (idx * 7) % 10;
    matrix[i][j] = g_value * 2;                 /* MEM: 2D array */
    
    asm volatile("" : : "r"(array[0]), "r"(matrix[0][0]));
}

/* ==================== Combined test function ==================== */

/* Function with multiple patterns to hit all uncovered lines */
NOINLINE int combined_test(int param) {
    /* ZERO_EXTRACT */
    volatile struct bitfield_struct bf;
    bf.field1 = param & 0x7;
    bf.field2 = (param >> 3) & 0x1F;
    
    /* STRICT_LOW_PART */
    short s = param & 0xFFFF;
    int i = s;                      /* Should generate STRICT_LOW_PART */
    
    /* SUBREG */
    v4si vec = {param, param+1, param+2, param+3};
    int element = vec[param % 4];   /* SUBREG extract */
    
    /* MEM_P with complex address */
    int buffer[20];
    buffer[param % 20] = element;   /* MEM store */
    
    /* More MEM with pointer */
    int *ptr = &buffer[g_index];
    *ptr = i;                       /* Another MEM store */
    
    /* Additional ZERO_EXTRACT */
    bf.field3 = element & 0xFF;
    
    /* Force use of all values */
    asm volatile("" : : "r"(bf), "r"(i), "r"(element), "r"(buffer[0]));
    
    return i + element + buffer[0];
}

/* ==================== Main driver ==================== */

int main(void) {
    int result = 0;
    
    /* Call all test functions with varying inputs */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_zero_extract_union();
        test_strict_low_part();
        result += test_partial_reg_mix(i, i * 2, i * 3);
        test_subreg_vectors();
        test_subreg_mixed();
        
        int array[50];
        test_mem_complex_address(array, i);
        test_mem_array_store();
        
        result += combined_test(i);
    }
    
    /* Ensure all code is used */
    asm volatile("" : : "r"(result));
    
    return result > 0 ? 0 : 1;
}
