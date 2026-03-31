/* test_resource.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile int g_temp;

/* ===== ZERO_EXTRACT patterns ===== */
/* Bit-field assignments generate ZERO_EXTRACT in RTL */
struct bitfield_struct {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
    unsigned int field4 : 16;
};

/* Volatile to prevent optimization */
volatile struct bitfield_struct g_bf;

NOINLINE void test_zero_extract(int a, int b, int c) {
    /* These assignments should generate ZERO_EXTRACT as SET_DEST */
    struct bitfield_struct local_bf;
    
    /* Assignment 1: Direct bit-field store */
    local_bf.field1 = a & 0x7;  /* Should generate ZERO_EXTRACT */
    
    /* Assignment 2: Volatile bit-field store */
    g_bf.field2 = b & 0x1F;     /* Should generate ZERO_EXTRACT */
    
    /* Assignment 3: Nested bit-field in struct */
    struct {
        unsigned int nested_field : 4;
    } inner;
    inner.nested_field = c & 0xF;
    
    /* Use the values to prevent dead code elimination */
    g_temp = local_bf.field1 + g_bf.field2 + inner.nested_field;
}

/* ===== STRICT_LOW_PART patterns ===== */
/* Partial register writes on x86 generate STRICT_LOW_PART */
NOINLINE void test_strict_low_part(short s_val, char c_val, int i_val) {
    int dest1, dest2, dest3;
    
    /* Assignment 1: short to int - may generate STRICT_LOW_PART on x86 */
    dest1 = s_val;  /* SET_DEST might be STRICT_LOW_PART for partial reg write */
    
    /* Assignment 2: char to long - similar pattern */
    long dest2_long;
    dest2_long = c_val;
    
    /* Assignment 3: Using bitwise AND to force partial write */
    dest3 = i_val & 0xFFFF;  /* Only modifying lower 16 bits */
    
    /* Mix with arithmetic to prevent optimization */
    dest1 = dest1 * 2 + dest2_long + (dest3 >> 8);
    
    /* Force use of values */
    g_temp = dest1;
}

/* For 32-bit x86, use specific patterns that often generate STRICT_LOW_PART */
#ifdef __i386__
NOINLINE void test_strict_low_part_x86(void) {
    /* These patterns are known to generate STRICT_LOW_PART on x86-32 */
    volatile short vs;
    volatile int vi;
    
    /* Pattern that often generates STRICT_LOW_PART */
    vi = vs;  /* Word to dword conversion */
    
    /* Another pattern */
    int x = 0;
    short y = 100;
    x = y;  /* Partial register update */
    
    g_temp = vi + x;
}
#endif

/* ===== SUBREG patterns ===== */
/* Type conversions and vector operations generate SUBREG */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE void test_subreg(float f_val, double d_val, int i_val) {
    /* Assignment 1: Float to int bit-cast - generates SUBREG */
    float f = f_val;
    int i;
    memcpy(&i, &f, sizeof(int));  /* Bit-cast using memcpy */
    
    /* Assignment 2: Vector lane extraction - generates SUBREG */
    v4si vec = {1, 2, 3, 4};
    int lane = vec[g_index & 3];  /* Vector lane extract */
    
    /* Assignment 3: Complex type conversion */
    v8hi short_vec = {1, 2, 3, 4, 5, 6, 7, 8};
    v4si int_vec;
    memcpy(&int_vec, &short_vec, sizeof(int_vec));
    
    /* Use values */
    g_temp = i + lane + int_vec[0];
}

/* ===== MEM_P patterns ===== */
/* Complex memory addresses generate MEM with address expressions */
NOINLINE void test_mem_dest(int *base, int offset, int value) {
    /* Assignment 1: Array with variable index - complex MEM address */
    int array[100];
    array[g_index * offset] = value;  /* SET_DEST is MEM with complex address */
    
    /* Assignment 2: Pointer arithmetic */
    int *ptr = base + (offset * 2);
    *ptr = value * 2;  /* MEM with pointer arithmetic */
    
    /* Assignment 3: Struct member through pointer */
    struct data {
        int a;
        int b;
        int c[5];
    } d;
    
    struct data *dptr = &d;
    dptr->c[offset % 5] = value;  /* Nested MEM access */
    
    /* Assignment 4: Multi-dimensional array */
    int matrix[10][10];
    matrix[offset % 10][g_index % 10] = value;
    
    /* Use values to prevent elimination */
    g_temp = array[0] + *ptr + dptr->c[0] + matrix[0][0];
}

/* ===== Combined test function ===== */
/* Mix all patterns in one function to increase coverage chance */
NOINLINE void test_combined(int a, int b, int *ptr) {
    /* ZERO_EXTRACT pattern */
    struct {
        unsigned int bf1 : 4;
        unsigned int bf2 : 12;
    } s;
    s.bf1 = a & 0xF;
    s.bf2 = b & 0xFFF;
    
    /* STRICT_LOW_PART pattern */
    short src_short = a;
    int dest_int = src_short;
    
    /* MEM_P pattern with complex addressing */
    int local_array[50];
    int idx = (a * b) % 50;
    local_array[idx] = dest_int;
    
    /* SUBREG pattern via type punning */
    float f = (float)a;
    int i;
    memcpy(&i, &f, sizeof(int));
    
    /* Another MEM_P with pointer */
    if (ptr) {
        ptr[(a + b) % 10] = i + s.bf1;
    }
    
    g_temp = local_array[0] + dest_int + i;
}

/* Main driver that calls all test functions */
int main(void) {
    int test_array[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        test_array[i] = i;
    }
    
    /* Run tests multiple times with different values */
    for (int iter = 0; iter < 10; iter++) {
        test_zero_extract(iter, iter * 2, iter * 3);
        test_strict_low_part(iter, iter + 1, iter * 5);
        
        #ifdef __i386__
        test_strict_low_part_x86();
        #endif
        
        test_subreg(iter * 1.5f, iter * 2.5, iter);
        test_mem_dest(test_array, iter, iter * 10);
        test_combined(iter, iter + 100, test_array);
    }
    
    /* Final validation */
    if (g_temp > 1000) {
        return 0;  /* Success */
    }
    
    return g_temp;  /* Return something based on computation */
}
