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

/* Bit-field structure for ZERO_EXTRACT */
struct bitfields {
    unsigned int field3 : 3;
    unsigned int field5 : 5;
    unsigned int field8 : 8;
    volatile unsigned int padding : 16;
};

NOINLINE void test_zero_extract(void) {
    /* This should generate ZERO_EXTRACT in SET_DEST */
    volatile struct bitfields bf;
    bf.field3 = 5;      /* ZERO_EXTRACT for 3-bit field */
    bf.field5 = 31;     /* ZERO_EXTRACT for 5-bit field */
    bf.field8 = 255;    /* ZERO_EXTRACT for 8-bit field */
    
    /* Use asm to prevent dead code elimination */
    asm volatile("" : : "r"(bf.field3), "r"(bf.field5), "r"(bf.field8));
}

/* Another ZERO_EXTRACT pattern using direct bit operations */
NOINLINE void test_zero_extract2(int x) {
    volatile unsigned int v = 0;
    /* This may generate ZERO_EXTRACT when optimized */
    unsigned int mask = 0x7;  /* 3 bits */
    v = (v & ~mask) | (x & mask);
    
    asm volatile("" : : "r"(v));
}

/* ==================== STRICT_LOW_PART patterns ==================== */

/* Mixed-size assignments for STRICT_LOW_PART */
NOINLINE void test_strict_low_part(short src_short, char src_char) {
    int dest_int = 0;
    long dest_long = 0;
    
    /* These assignments may generate STRICT_LOW_PART */
    dest_int = src_short;   /* short -> int */
    dest_long = src_char;   /* char -> long */
    
    /* Force use of results */
    asm volatile("" : : "r"(dest_int), "r"(dest_long));
}

/* Inline assembly that explicitly writes partial registers */
NOINLINE void test_strict_low_part_asm(void) {
    int x = 0;
    short y = 0x1234;
    
    /* Inline asm that writes only lower 16 bits */
    asm volatile("movw %1, %0\n\t"
                 : "=r"(x)
                 : "r"(y)
                 : /* No clobber */);
    
    asm volatile("" : : "r"(x));
}

/* ==================== SUBREG patterns ==================== */

/* Vector types for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE void test_subreg(void) {
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    
    /* Vector operations that may generate SUBREG */
    vec_a = vec_b;                     /* Whole vector copy */
    int lane = vec_a[g_index & 3];     /* Lane extraction - SUBREG */
    
    /* Type punning through union for SUBREG */
    union {
        float f;
        int i;
    } u;
    u.f = 3.14f;
    int int_from_float = u.i;          /* Bitcast - may use SUBREG */
    
    /* Mixed vector operations */
    v8hi short_vec = {1, 2, 3, 4, 5, 6, 7, 8};
    v4si int_vec = __builtin_convertvector(short_vec, v4si);
    
    asm volatile("" : : "r"(lane), "r"(int_from_float), "r"(int_vec[0]));
}

/* Complex number operations often use SUBREG */
NOINLINE void test_complex_subreg(float _Complex cf, double _Complex cd) {
    float _Complex cf2 = cf;
    double _Complex cd2 = cd;
    
    /* Real/imaginary part extraction */
    float re = __real__ cf2;
    float im = __imag__ cf2;
    
    /* Complex assignment */
    cf2 = cf2 + cf2;
    
    asm volatile("" : : "r"(re), "r"(im));
}

/* ==================== MEM_P patterns ==================== */

/* Complex memory addressing for MEM_P */
NOINLINE void test_mem_dest(int *base, int offset, int value) {
    /* Various complex address calculations */
    base[offset] = value;                     /* Array indexing */
    *(base + offset + g_index) = value + 1;   /* Pointer arithmetic */
    
    /* Struct with pointer arithmetic */
    struct {
        int a;
        int b;
        int c[5];
    } s;
    
    int *ptr = &s.a;
    ptr[g_index & 3] = value;                /* Complex struct access */
    
    /* Multi-dimensional array */
    int arr2d[4][4];
    arr2d[g_index & 3][offset & 3] = value;
    
    asm volatile("" : : "r"(s.a), "r"(arr2d[0][0]));
}

/* Function returning pointer for complex addressing */
int* get_pointer(int *base, int idx) {
    return base + idx * 2;
}

NOINLINE void test_mem_complex_addr(int *array, int size) {
    /* Use function call in address calculation */
    *get_pointer(array, g_index) = g_value;
    
    /* Conditional address calculation */
    int *dest = g_condition ? array : array + size/2;
    dest[g_index] = g_value * 2;
    
    /* Loop with memory stores */
    for (int i = 0; i < (size & 7); i++) {
        array[i * 2] = g_value + i;
    }
    
    asm volatile("" : : "r"(array[0]));
}

/* ==================== Combined test function ==================== */

/* Function that combines all patterns */
NOINLINE __attribute__((optimize("O2"))) 
void test_combined(int *mem, short src_short, char src_char) {
    /* ZERO_EXTRACT */
    volatile struct bitfields bf;
    bf.field5 = src_char & 0x1F;
    
    /* STRICT_LOW_PART */
    int partial_reg = src_short;
    
    /* SUBREG */
    v4si vec = {1, 2, 3, 4};
    int lane = vec[g_index & 3];
    
    /* MEM_P with complex address */
    mem[g_index] = partial_reg + lane + bf.field5;
    
    /* Additional MEM with pointer arithmetic */
    int *ptr = mem + (g_index * 2);
    *ptr = src_char;
    
    asm volatile("" : : "r"(partial_reg), "r"(lane), "r"(mem[0]));
}

/* ==================== Main driver ==================== */

int main(void) {
    int array[64] = {0};
    short short_val = 1000;
    char char_val = 100;
    
    /* Test each pattern individually */
    test_zero_extract();
    test_zero_extract2(5);
    
    test_strict_low_part(short_val, char_val);
    test_strict_low_part_asm();
    
    test_subreg();
    test_complex_subreg(1.0f + 2.0fi, 3.0 + 4.0i);
    
    test_mem_dest(array, 10, 42);
    test_mem_complex_addr(array, 64);
    
    /* Combined test with optimization */
    test_combined(array, short_val, char_val);
    
    /* Loop to increase execution count */
    for (int i = 0; i < 10; i++) {
        g_index = i & 15;
        g_condition = i & 1;
        test_zero_extract2(i);
        test_mem_dest(array, i, i * 10);
    }
    
    /* Prevent dead code elimination of entire program */
    asm volatile("" : : "r"(array[0]), "r"(short_val), "r"(char_val));
    
    return 0;
}
