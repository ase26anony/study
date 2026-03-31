/* test_resource.c - Coverage test for GCC resource.cc mark_referenced_resources */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile int g_temp;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Bit-field structure - should generate ZERO_EXTRACT in RTL */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 3;
    unsigned int field3 : 8;
    unsigned int field4 : 16;
};

/* Volatile bit-field to prevent optimization */
volatile struct bitfield_struct g_bf;

NOINLINE void test_zero_extract(void) {
    /* Assignment to bit-field members - should generate ZERO_EXTRACT as SET_DEST */
    struct bitfield_struct bf;
    
    /* Multiple bit-field assignments */
    bf.field1 = 0x1F;           /* Max value for 5 bits */
    bf.field2 = 0x3;            /* Should generate ZERO_EXTRACT */
    bf.field3 = 0xFF;           /* Max value for 8 bits */
    bf.field4 = 0xABCD;         /* 16-bit value */
    
    /* Volatile assignment to prevent dead code elimination */
    g_bf.field2 = bf.field2;
    
    /* Complex expression with bit-field destination */
    int x = g_value;
    bf.field3 = (x & 0xFF) | 0x80;
    
    /* Use asm to mark variable as used */
    asm volatile("" : : "r"(bf.field1));
}

/* ==================== STRICT_LOW_PART patterns ==================== */

NOINLINE void test_strict_low_part(void) {
    /* Mixed-size integer assignments - may generate STRICT_LOW_PART */
    short src_short = 0x1234;
    char src_char = 0x56;
    int dest_int = 0;
    
    /* These assignments might generate STRICT_LOW_PART on x86 */
    dest_int = src_short;       /* 16-bit to 32-bit */
    g_temp = dest_int;
    
    /* Cast through union to force partial register write */
    union {
        int full;
        struct {
            short low;
            short high;
        } parts;
    } u;
    
    u.full = 0xFFFFFFFF;
    u.parts.low = src_short;    /* Should write only low 16 bits */
    
    /* Byte assignment to larger type */
    unsigned long big = 0x12345678;
    *(unsigned char*)&big = src_char;  /* Write single byte */
    
    /* Use volatile to prevent optimization */
    asm volatile("" : : "r"(u.full), "r"(big));
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
    
    /* Vector assignment */
    vec_a = vec_b;              /* May generate SUBREG operations */
    
    /* Extract lane - generates SUBREG */
    int lane0 = vec_a[0];
    int lane2 = vec_a[2];
    
    /* Mixed vector types */
    v8hi short_vec = {1, 2, 3, 4, 5, 6, 7, 8};
    v4si int_vec;
    
    /* Type conversion through memory (force SUBREG) */
    memcpy(&int_vec, &short_vec, sizeof(short_vec));
    
    /* Float/int reinterpretation */
    float f = 3.14159f;
    int i;
    memcpy(&i, &f, sizeof(f));  /* Bit-cast, may use SUBREG */
    
    /* Complex number (C99) */
    _Complex double c = 1.0 + 2.0i;
    __real__ c = 3.0;           /* Real part assignment */
    
    asm volatile("" : : "r"(lane0), "r"(lane2), "r"(i), "r"(c));
}

/* ==================== MEM_P patterns ==================== */

NOINLINE void test_mem_p(int *arr, int n) {
    /* Complex memory destinations */
    
    /* Array with variable index */
    arr[g_index] = g_value;     /* MEM with complex address */
    
    /* Pointer arithmetic */
    int *ptr = arr + n;
    *ptr = g_value + 1;         /* Another MEM destination */
    
    /* Struct member through pointer */
    struct data {
        int a;
        int b;
        int c[4];
    } d;
    
    struct data *dptr = &d;
    dptr->b = g_value;          /* MEM with field offset */
    dptr->c[n % 4] = g_value * 2; /* MEM with array index */
    
    /* Two-dimensional access */
    int matrix[4][4];
    int idx1 = g_index % 4;
    int idx2 = (g_index + 1) % 4;
    matrix[idx1][idx2] = g_value; /* Nested MEM access */
    
    /* Volatile memory write */
    volatile int *volatile_ptr = (volatile int*)arr;
    volatile_ptr[n] = g_value;  /* Volatile MEM */
    
    asm volatile("" : : "r"(dptr->b), "r"(matrix[0][0]));
}

/* ==================== Combined test function ==================== */

/* Force O2 optimization for this function */
__attribute__((optimize("O2")))
NOINLINE void test_combined(void) {
    /* Mix all patterns in one function */
    
    /* ZERO_EXTRACT */
    struct {
        unsigned int flags : 4;
        unsigned int mode : 3;
    } opts;
    opts.flags = g_value & 0xF;
    opts.mode = (g_value >> 4) & 0x7;
    
    /* STRICT_LOW_PART */
    unsigned int val32 = 0x87654321;
    unsigned short val16 = 0xABCD;
    *(unsigned short*)&val32 = val16;  /* Partial write */
    
    /* SUBREG */
    v4sf fvec = {1.0f, 2.0f, 3.0f, 4.0f};
    float f = fvec[g_index % 4];
    
    /* MEM_P */
    int local_arr[10];
    int idx = g_index % 10;
    local_arr[idx] = g_value;
    local_arr[idx + 1] = f;
    
    asm volatile("" : : "r"(opts.flags), "r"(val32), "r"(f), "r"(local_arr[0]));
}

/* ==================== Main driver ==================== */

int main(void) {
    int array[16];
    
    /* Initialize array */
    for (int i = 0; i < 16; i++) {
        array[i] = i * 10;
    }
    
    /* Call test functions multiple times with different contexts */
    for (int iter = 0; iter < 10; iter++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_mem_p(array, iter % 8);
        test_combined();
        
        /* Modify globals to change behavior */
        g_index = (g_index * 13 + 7) % 16;
        g_value = (g_value * 3 + 1) % 100;
    }
    
    /* Final validation */
    g_temp = array[g_index];
    
    return 0;
}
