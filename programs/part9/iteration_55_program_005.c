/* test_resource.c
 * 
 * This program is designed to trigger specific code paths in GCC's
 * resource.cc, specifically the mark_referenced_resources function
 * when handling SET_DEST expressions of type ZERO_EXTRACT,
 * STRICT_LOW_PART, SUBREG, and MEM.
 *
 * Compile with: gcc -O2 -fprofile-arcs -ftest-coverage -fdump-rtl-all -c test_resource.c
 * Also try: gcc -m32 -O1 -fno-omit-frame-pointer -fprofile-arcs -ftest-coverage test_resource.c
 */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile char g_char = 'A';
volatile short g_short = 1234;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Bit-field assignment - likely generates ZERO_EXTRACT */
NOINLINE void test_zero_extract(void) {
    /* Volatile struct with bit-field */
    volatile struct {
        unsigned int field1 : 3;
        unsigned int field2 : 5;
        unsigned int field3 : 8;
    } bit_struct;
    
    /* Multiple bit-field assignments */
    bit_struct.field1 = 5;      /* Should generate ZERO_EXTRACT */
    bit_struct.field2 = 0x1F;   /* Max value for 5 bits */
    bit_struct.field3 = g_value & 0xFF;
    
    /* Another volatile bit-field */
    volatile unsigned int bits : 4;
    bits = g_value & 0x0F;
    
    /* Force use of results */
    asm volatile("" : : "r"(bit_struct.field1), "r"(bits));
}

/* ==================== STRICT_LOW_PART patterns ==================== */

/* Mixed-size integer assignments - may generate STRICT_LOW_PART */
NOINLINE void test_strict_low_part(int x) {
    volatile short src_short = g_short;
    volatile char src_char = g_char;
    
    /* These assignments might generate STRICT_LOW_PART on x86 */
    int dest_int = src_short;          /* short -> int */
    long dest_long = src_char;         /* char -> long */
    
    /* More explicit partial register writes */
    int partial_dest = 0;
    partial_dest = (short)(x & 0xFFFF);  /* Force low part assignment */
    
    /* Use inline assembly hint for partial register */
    asm volatile("" : "+r"(partial_dest));
    
    /* Prevent optimization */
    asm volatile("" : : "r"(dest_int), "r"(dest_long));
}

/* ==================== SUBREG patterns ==================== */

/* Vector and type-punning operations - generate SUBREG */
NOINLINE void test_subreg(void) {
    /* GCC vector types */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    
    /* Vector operations that may involve SUBREG */
    vec_a = vec_a + vec_b;
    
    /* Extract lane - likely generates SUBREG */
    int lane0 = vec_a[0];
    int lane1 = vec_a[1];
    
    /* Type punning through union */
    union {
        float f;
        int i;
    } pun;
    pun.f = 3.14159f;
    int int_bits = pun.i;  /* May generate SUBREG */
    
    /* Mixed size operations */
    v8hi short_vec = {1, 2, 3, 4, 5, 6, 7, 8};
    short_vec = short_vec + (v8hi){2, 2, 2, 2, 2, 2, 2, 2};
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(lane0), "r"(lane1), "r"(int_bits));
}

/* ==================== MEM patterns ==================== */

/* Complex memory stores - generate MEM with address expressions */
NOINLINE void test_mem_dest(int *base, int offset) {
    int array[16];
    int *ptr;
    
    /* Store with complex address calculation */
    array[g_index] = g_value;                     /* MEM with global index */
    array[offset * 2 + 1] = g_value * 2;          /* MEM with computation */
    
    /* Pointer arithmetic store */
    ptr = &array[0];
    ptr[offset % 8] = offset;                     /* MEM with modulo */
    
    /* Struct member through pointer */
    struct {
        int a;
        int b;
        int c;
    } s, *sptr = &s;
    
    sptr->b = g_value;                            /* MEM with struct offset */
    
    /* Multi-dimensional array */
    int matrix[4][4];
    matrix[offset % 4][offset / 4] = g_value;     /* MEM with 2D index */
    
    /* Force memory operations */
    asm volatile("" : : "m"(array[0]), "m"(s.a), "m"(matrix[0][0]));
}

/* ==================== Combined test function ==================== */

/* Function that combines all patterns */
NOINLINE void test_combined(int x) {
    /* ZERO_EXTRACT pattern */
    volatile struct {
        unsigned int bf1 : 2;
        unsigned int bf2 : 6;
    } bf;
    bf.bf1 = x & 0x3;
    bf.bf2 = (x >> 2) & 0x3F;
    
    /* STRICT_LOW_PART pattern */
    int dest;
    short src = x & 0x7FFF;
    dest = src;  /* Potential STRICT_LOW_PART */
    
    /* SUBREG pattern with vectors */
    typedef float v4f __attribute__((vector_size(16)));
    v4f v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4f v2 = v1 * 2.0f;
    float first = v2[0];  /* SUBREG extraction */
    
    /* MEM pattern */
    int mem_array[8];
    int idx = (x * 3) % 8;
    mem_array[idx] = x;  /* MEM store with computed index */
    
    /* Prevent optimization */
    asm volatile("" : : "r"(bf.bf1), "r"(dest), "r"(first), "m"(mem_array[0]));
}

/* ==================== Main driver ==================== */

int main(void) {
    int test_data[] = {1, 2, 3, 4, 5};
    int i;
    
    /* Call test functions multiple times with different inputs */
    for (i = 0; i < 5; i++) {
        test_zero_extract();
        test_strict_low_part(test_data[i]);
        test_subreg();
        test_mem_dest((int*)test_data, i);
        test_combined(test_data[i]);
    }
    
    /* Additional calls with edge cases */
    test_strict_low_part(0);
    test_strict_low_part(0xFFFF);
    
    return 0;
}
