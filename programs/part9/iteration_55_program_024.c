/* test_resource.c - Coverage test for GCC resource.cc mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile int g_temp;

/* ========== ZERO_EXTRACT patterns ========== */
NOINLINE void test_zero_extract(void) {
    /* Bit-field assignment should generate ZERO_EXTRACT in SET_DEST */
    struct bitfield_s {
        unsigned int field1 : 3;
        unsigned int field2 : 5;
        unsigned int field3 : 8;
    };
    
    volatile struct bitfield_s bf;
    bf.field1 = 5;      /* Should generate ZERO_EXTRACT for 3-bit field */
    bf.field2 = 31;     /* Should generate ZERO_EXTRACT for 5-bit field */
    bf.field3 = 255;    /* Should generate ZERO_EXTRACT for 8-bit field */
    
    /* Force use of bit-field to prevent optimization */
    g_temp = bf.field1 + bf.field2 + bf.field3;
}

/* Mixed bit-field types to ensure different extraction patterns */
NOINLINE void test_zero_extract_mixed(void) {
    struct mixed_bf {
        signed int sfield : 4;    /* Signed bit-field */
        unsigned long long lfield : 12;  /* Larger bit-field */
        int normal_int;
    };
    
    volatile struct mixed_bf mbf;
    mbf.sfield = -3;              /* Signed bit-field assignment */
    mbf.lfield = 2047;            /* 12-bit field assignment */
    mbf.normal_int = 100;
    
    /* Complex expression to prevent optimization */
    if (mbf.sfield < 0) {
        mbf.lfield = mbf.sfield + 8;
    }
    
    g_temp = mbf.normal_int;
}

/* ========== STRICT_LOW_PART patterns ========== */
/* Compile with -m32 for better STRICT_LOW_PART generation on x86 */
NOINLINE void test_strict_low_part(void) {
    /* Partial register writes - common on x86 */
    volatile short src16 = -12345;
    volatile char src8 = 127;
    
    int dest32;
    long long dest64;
    
    /* These assignments may generate STRICT_LOW_PART */
    dest32 = src16;      /* short -> int, preserving only low 16 bits */
    dest32 = src8;       /* char -> int, preserving only low 8 bits */
    
    /* 64-bit operations */
    dest64 = src16;      /* short -> long long */
    dest64 = src32;      /* int -> long long on 32-bit arch */
    
    /* Use inline assembly to force partial register writes */
    asm volatile (
        "movw %1, %%ax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (dest32)
        : "r" (src16)
        : "%eax"
    );
    
    g_temp = dest32 + dest64;
}

NOINLINE void test_strict_low_part_complex(void) {
    volatile unsigned char data[4] = {1, 2, 3, 4};
    unsigned int combined = 0;
    
    /* Building an integer from bytes may generate partial writes */
    combined = data[0];
    combined |= (data[1] << 8);
    combined |= (data[2] << 16);
    combined |= (data[3] << 24);
    
    /* Conditional partial write */
    if (g_value > 0) {
        combined = data[0] | (data[1] << 8);  /* Only affects lower 16 bits */
    }
    
    g_temp = combined;
}

/* ========== SUBREG patterns ========== */
NOINLINE void test_subreg(void) {
    /* GCC vector extensions often generate SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v8hi vec_short = {1, 2, 3, 4, 5, 6, 7, 8};
    
    /* Vector operations that may generate SUBREG in SET_DEST */
    vec_a = vec_a + vec_b;          /* Vector operation */
    
    /* Extract lane - may generate SUBREG */
    int lane0 = vec_a[0];
    int lane1 = vec_a[1];
    
    /* Type punning through union - generates SUBREG */
    union {
        float f;
        int i;
    } pun;
    pun.f = 3.14159f;
    int int_bits = pun.i;           /* float -> int via SUBREG */
    
    /* Cast between vector types */
    vec_short = __builtin_convertvector(vec_a, v8hi);
    
    g_temp = lane0 + lane1 + int_bits + vec_short[0];
}

/* ========== MEM_P patterns ========== */
NOINLINE void test_mem_dest(void) {
    /* Complex memory destinations */
    int array[16];
    int *ptr = array;
    
    /* Store with complex address calculation */
    array[g_index] = g_value;                    /* MEM with index */
    array[g_index + 2] = g_value * 2;            /* More complex index */
    
    /* Pointer arithmetic store */
    *(ptr + g_index) = g_value;                  /* MEM with pointer arithmetic */
    
    /* Struct member store through pointer */
    struct point {
        int x;
        int y;
        int z;
    } points[4];
    
    points[g_index % 4].x = g_value;             /* MEM with struct offset */
    points[g_index % 4].y = g_value * 2;
    
    /* Multi-dimensional array */
    int matrix[4][4];
    matrix[g_index % 4][g_index % 3] = g_value;  /* MEM with 2D indexing */
    
    /* Use computed goto to force address calculation? */
    void *labels[] = {&&L1, &&L2, &&L3};
    goto *labels[g_index % 3];
    
L1:
    array[0] = 1;
    goto END;
L2:
    array[1] = 2;
    goto END;
L3:
    array[2] = 3;
    goto END;
END:
    g_temp = array[0] + array[1] + array[2];
}

/* ========== Combined test function ========== */
/* Force O2 optimization specifically for this function */
__attribute__((optimize("O2")))
NOINLINE void test_combined(void) {
    /* Mix all patterns in one function */
    
    /* ZERO_EXTRACT */
    struct {
        unsigned int bf1 : 4;
        unsigned int bf2 : 4;
    } s;
    s.bf1 = g_value & 0x0F;
    s.bf2 = (g_value >> 4) & 0x0F;
    
    /* STRICT_LOW_PART */
    short src = g_value;
    int dest = src;  /* May generate STRICT_LOW_PART */
    
    /* SUBREG */
    typedef float v2f __attribute__((vector_size(8)));
    v2f vf = {1.0f, 2.0f};
    float f0 = vf[0];  /* Lane extract */
    
    /* MEM_P */
    int buf[8];
    for (int i = 0; i < 8; i++) {
        buf[i] = g_value + i;  /* Store to memory */
    }
    
    /* Use results to prevent optimization */
    g_temp = s.bf1 + s.bf2 + dest + (int)f0 + buf[0];
}

/* ========== Main driver ========== */
int main(void) {
    /* Call all test functions multiple times with different conditions */
    for (int i = 0; i < 10; i++) {
        g_index = i;
        g_value = i * 10;
        
        test_zero_extract();
        test_zero_extract_mixed();
        test_strict_low_part();
        test_strict_low_part_complex();
        test_subreg();
        test_mem_dest();
        test_combined();
    }
    
    /* Final barrier to prevent tail optimization */
    asm volatile("" : : "r"(g_temp));
    
    return g_temp > 0 ? 0 : 1;
}
