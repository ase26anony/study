/* test_resource.c - Coverage test for GCC resource.cc mark_referenced_resources */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile int g_condition = 1;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Bit-field structure - should generate ZERO_EXTRACT for assignments */
struct bitfield_struct {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
    unsigned int field4 : 16;
};

/* Volatile bit-field to prevent optimization */
volatile struct bitfield_struct g_bf;

NOINLINE void test_zero_extract(void) {
    /* Assignment to bit-field members - should generate ZERO_EXTRACT as SET_DEST */
    struct bitfield_struct local_bf;
    
    /* Multiple bit-field assignments */
    local_bf.field1 = 5;      /* Likely ZERO_EXTRACT destination */
    local_bf.field2 = 0x1F;   /* Another bit-field assignment */
    
    /* Volatile assignment to prevent elimination */
    g_bf.field3 = 0x55;
    
    /* Complex expression with bit-field */
    int x = g_value;
    local_bf.field4 = (x & 0xFF) | 0x100;
    
    /* Use asm to mark as used */
    asm volatile("" : : "r"(local_bf.field1), "r"(local_bf.field2));
}

/* ==================== STRICT_LOW_PART patterns ==================== */

NOINLINE void test_strict_low_part(void) {
    /* Mixed-size integer assignments - may generate STRICT_LOW_PART */
    short src_short = 0x1234;
    char src_char = 0x56;
    int dest_int = 0;
    
    /* These assignments might use STRICT_LOW_PART to write partial registers */
    dest_int = src_short;          /* 16-bit to 32-bit */
    dest_int = src_char;           /* 8-bit to 32-bit */
    
    /* Volatile to force actual store */
    volatile int vol_dest = 0;
    vol_dest = src_short;
    
    /* Pointer-based partial write */
    int *ptr = &dest_int;
    *(short *)ptr = src_short;     /* Partial store through pointer */
    
    /* Use the results */
    asm volatile("" : : "r"(dest_int), "r"(vol_dest));
}

/* ==================== SUBREG patterns ==================== */

/* GCC vector types for SUBREG generation */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE void test_subreg(void) {
    /* Vector operations that generate SUBREG */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    
    /* Vector assignment - may involve SUBREG */
    v4si vec_c = vec_a + vec_b;
    
    /* Extract element - generates SUBREG */
    int element = vec_c[g_index & 3];
    
    /* Type punning through union - often uses SUBREG */
    union {
        float f;
        int i;
    } pun;
    pun.f = 3.14159f;
    int int_bits = pun.i;          /* SUBREG for type conversion */
    
    /* Mixed vector types */
    v8hi short_vec = {1, 2, 3, 4, 5, 6, 7, 8};
    v4si int_vec = __builtin_convertvector(short_vec, v4si);
    
    /* Use results */
    asm volatile("" : : "r"(element), "r"(int_bits), "r"(int_vec[0]));
}

/* ==================== MEM_P patterns ==================== */

/* Helper to create complex address expressions */
NOINLINE int *get_pointer(int *base, int offset) {
    return base + (offset * 2) + (g_condition ? 1 : -1);
}

NOINLINE void test_mem_dest(void) {
    int array[32];
    int *ptr_array = array;
    
    /* Complex memory destination with index calculation */
    array[g_index + 1] = g_value;                     /* MEM with index */
    array[g_index * 2] = array[g_index] + 1;          /* MEM both src and dest */
    
    /* Pointer with arithmetic */
    int *ptr = get_pointer(array, g_index);
    *ptr = g_value * 2;                               /* MEM with complex address */
    
    /* Struct member through pointer */
    struct {
        int a;
        int b;
        int c;
    } mystruct;
    
    struct mystruct *sptr = &mystruct;
    sptr->b = g_value;                                /* MEM with field offset */
    
    /* Multi-dimensional array */
    int matrix[4][4];
    matrix[g_index & 3][(g_index + 1) & 3] = g_value; /* MEM with 2D index */
    
    /* Use results */
    asm volatile("" : : "r"(array[0]), "r"(*ptr), "r"(mystruct.b), "r"(matrix[0][0]));
}

/* ==================== Combined test function ==================== */

/* Function with all patterns combined */
NOINLINE __attribute__((optimize("O2"))) 
void test_combined(int param) {
    /* ZERO_EXTRACT pattern */
    struct {
        unsigned int bits : 7;
    } bf_local;
    bf_local.bits = param & 0x7F;
    
    /* STRICT_LOW_PART pattern */
    short short_val = param & 0xFFFF;
    int int_val = 0;
    int_val = short_val;          /* Potential STRICT_LOW_PART */
    
    /* SUBREG pattern */
    typedef float v2f __attribute__((vector_size(8)));
    v2f vec = {1.0f, 2.0f};
    float f = vec[param & 1];     /* SUBREG for element extraction */
    
    /* MEM_P pattern */
    int local_array[8];
    int idx = (param * 3) & 7;
    local_array[idx] = param;     /* MEM with computed index */
    
    /* Use all results */
    asm volatile("" : : "r"(bf_local.bits), "r"(int_val), "r"(f), "r"(local_array[0]));
}

/* ==================== Main function ==================== */

int main(void) {
    /* Call test functions multiple times with different values */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_mem_dest();
        test_combined(i);
        
        /* Modify globals to vary behavior */
        g_index = (g_index * 7 + 1) & 31;
        g_value = g_value + i;
        g_condition = i & 1;
    }
    
    return 0;
}
