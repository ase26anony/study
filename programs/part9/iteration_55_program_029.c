/* test_resource.c - Coverage test for GCC's resource.cc mark_referenced_resources */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile char g_char = 'A';
volatile short g_short = 1234;

/* ========== ZERO_EXTRACT patterns ========== */
NOINLINE void test_zero_extract(void) {
    /* Bit-field assignment - likely generates ZERO_EXTRACT in SET_DEST */
    struct {
        volatile unsigned int field1 : 3;
        volatile unsigned int field2 : 5;
        volatile unsigned int field3 : 8;
    } bit_struct;
    
    /* Multiple assignments to ensure coverage */
    bit_struct.field1 = g_value & 0x7;        /* Should generate ZERO_EXTRACT */
    bit_struct.field2 = (g_value >> 3) & 0x1F;
    bit_struct.field3 = g_char & 0xFF;
    
    /* Volatile bit-field in union */
    union {
        volatile uint32_t full;
        struct {
            volatile uint32_t low : 10;
            volatile uint32_t mid : 10;
            volatile uint32_t high : 12;
        } bits;
    } bit_union;
    
    bit_union.bits.low = g_value;
    bit_union.bits.mid = g_short;
    bit_union.bits.high = g_index;
    
    /* Use asm to prevent dead code elimination */
    asm volatile("" : : "r"(bit_struct), "r"(bit_union));
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE void test_strict_low_part(void) {
    /* Mixed-size assignments - may generate STRICT_LOW_PART on x86 */
    int dest_int;
    short src_short = g_short;
    char src_char = g_char;
    
    /* These assignments might use STRICT_LOW_PART for partial register writes */
    dest_int = src_short;          /* short -> int */
    dest_int = src_char;           /* char -> int */
    
    /* Use volatile to prevent optimization */
    volatile int vol_int = dest_int;
    
    /* Pointer casting with different sizes */
    int32_t wide = 0x12345678;
    int16_t *narrow_ptr = (int16_t*)&wide;
    *narrow_ptr = g_short;         /* Partial write to 32-bit through 16-bit ptr */
    
    asm volatile("" : : "r"(vol_int), "r"(wide));
}

/* ========== SUBREG patterns ========== */
NOINLINE void test_subreg(void) {
    /* GCC vector extensions - often generate SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v8hi vec_short = {1, 2, 3, 4, 5, 6, 7, 8};
    
    /* Vector operations that might use SUBREG */
    vec_a = vec_b + vec_a;
    
    /* Extract lane - might use SUBREG */
    int lane = vec_a[g_index & 3];
    
    /* Type punning through union */
    union {
        float f;
        int i;
        char c[4];
    } pun;
    
    pun.f = 3.14159f;
    pun.i = g_value;               /* Type change might use SUBREG */
    pun.c[1] = g_char;             /* Partial write */
    
    /* Complex number assignment */
    _Complex float comp1 = 1.0f + 2.0fi;
    _Complex float comp2 = 3.0f + 4.0fi;
    comp1 = comp2;                 /* Complex assignment might use SUBREG */
    
    asm volatile("" : : "r"(lane), "r"(pun), "r"(comp1));
}

/* ========== MEM_P patterns ========== */
NOINLINE void test_mem_p(int *array, int size) {
    /* Complex memory stores - generate MEM in SET_DEST */
    
    /* Array with variable index */
    array[g_index % size] = g_value;
    
    /* Pointer arithmetic */
    int *ptr = array + (g_index & 7);
    *ptr = g_value * 2;
    
    /* Struct member through pointer */
    struct {
        int a;
        int b;
        int c;
    } data;
    
    struct data *data_ptr = &data;
    data_ptr->b = g_value;
    data_ptr->c = array[g_index % size];
    
    /* Multi-dimensional array */
    int md_array[4][4];
    md_array[g_index & 3][(g_index >> 2) & 3] = g_value;
    
    /* Memory with computed address */
    int offset = (g_char * g_short) % 16;
    *(array + offset) = g_value + g_index;
    
    asm volatile("" : : "r"(array), "r"(data_ptr), "r"(md_array));
}

/* ========== Combined test function ========== */
/* Use O2 optimization specifically for this function */
__attribute__((optimize("O2"))) 
NOINLINE void combined_test(int *arr, int n) {
    /* Mix all patterns in one function */
    
    /* ZERO_EXTRACT */
    struct {
        volatile unsigned bits : 4;
    } bf;
    bf.bits = g_value & 0xF;
    
    /* STRICT_LOW_PART hint */
    volatile long big = 0x123456789ABCDEF0LL;
    volatile int medium = big;      /* truncation */
    
    /* SUBREG */
    typedef float v2f __attribute__((vector_size(8)));
    v2f v1 = {1.0f, 2.0f};
    v2f v2 = {3.0f, 4.0f};
    v1 = v1 + v2;
    float f = v1[0];
    
    /* MEM_P */
    if (n > 0) {
        arr[g_index % n] = f + medium + bf.bits;
    }
    
    /* Loop to create more RTL opportunities */
    for (int i = 0; i < 4; i++) {
        arr[i] += g_value;
    }
    
    asm volatile("" : : "r"(bf), "r"(big), "r"(v1), "r"(arr));
}

/* ========== Main driver ========== */
int main(void) {
    int test_array[32];
    
    /* Initialize array with non-zero values */
    for (int i = 0; i < 32; i++) {
        test_array[i] = i * 3 + 1;
    }
    
    /* Call all test functions multiple times with different data */
    for (int iter = 0; iter < 10; iter++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_mem_p(test_array, 32);
        combined_test(test_array, 32);
        
        /* Modify globals to vary behavior */
        g_index = (g_index * 13 + 7) % 31;
        g_value = (g_value * 3 + 1) % 100;
        g_char = (g_char + 1) & 0x7F;
        g_short = (g_short * 2 + 1) & 0x3FFF;
    }
    
    /* Final validation - compute checksum */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += test_array[i];
    }
    
    return sum > 0 ? 0 : 1;
}
