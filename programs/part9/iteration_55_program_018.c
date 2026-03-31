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
NOINLINE void test_zero_extract(void) {
    /* Bit-field assignment - should generate ZERO_EXTRACT in SET_DEST */
    struct {
        volatile unsigned int field1 : 3;
        volatile unsigned int field2 : 5;
        volatile unsigned int field3 : 8;
    } bit_struct;
    
    bit_struct.field1 = 5;      /* ZERO_EXTRACT for 3-bit field */
    bit_struct.field2 = 17;     /* ZERO_EXTRACT for 5-bit field */
    bit_struct.field3 = 123;    /* ZERO_EXTRACT for 8-bit field */
    
    /* Volatile bit-field in union */
    union {
        volatile unsigned int full;
        struct {
            volatile unsigned int low : 10;
            volatile unsigned int high : 22;
        } bits;
    } bit_union;
    
    bit_union.bits.low = 511;   /* ZERO_EXTRACT for low 10 bits */
    bit_union.bits.high = 12345; /* ZERO_EXTRACT for high 22 bits */
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(bit_struct), "r"(bit_union));
}

/* ==================== STRICT_LOW_PART patterns ==================== */
NOINLINE void test_strict_low_part(short src_short, char src_char) {
    int dest_int;
    long dest_long;
    
    /* Mixed-size assignments - may generate STRICT_LOW_PART */
    dest_int = src_short;       /* short -> int, low part only */
    dest_long = src_char;       /* char -> long, low part only */
    
    /* Multiple partial writes */
    volatile short vs = 1234;
    volatile int vi;
    vi = vs;                    /* Potential STRICT_LOW_PART */
    
    /* Use inline assembly hint for partial register */
    asm volatile("# Partial reg write" : "=r"(dest_int) : "0"(src_short));
    
    /* Prevent optimization */
    asm volatile("" : : "r"(dest_int), "r"(dest_long), "r"(vi));
}

/* ==================== SUBREG patterns ==================== */
NOINLINE void test_subreg(void) {
    /* GCC vector types - SUBREG common with vector operations */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v8hi vec_short = {1, 2, 3, 4, 5, 6, 7, 8};
    
    /* Vector operations that may use SUBREG */
    vec_a = vec_a + vec_b;      /* Vector operation */
    
    /* Extract lane - may use SUBREG as destination */
    int lane0 = vec_a[0];       /* SUBREG extract from vector */
    short short_lane = vec_short[3]; /* Another SUBREG extract */
    
    /* Type punning through union - often uses SUBREG */
    union {
        float f;
        int i;
    } pun;
    
    pun.f = 3.14159f;
    int int_bits = pun.i;       /* SUBREG for type conversion */
    
    /* Complex number assignment */
    __complex__ double cd = 1.0 + 2.0i;
    __complex__ float cf;
    cf = cd;                    /* May involve SUBREG */
    
    asm volatile("" : : "r"(lane0), "r"(short_lane), "r"(int_bits), "r"(cf));
}

/* ==================== MEM_P patterns ==================== */
NOINLINE void test_mem_p(int *base, int offset, int value) {
    /* Complex address computation for MEM destination */
    int *ptr;
    
    /* Array with variable index */
    int array[16];
    array[g_index] = value;             /* MEM with global index */
    array[offset % 16] = value + 1;     /* MEM with computed index */
    
    /* Pointer arithmetic */
    ptr = base + offset;
    *ptr = value;                       /* MEM with pointer arithmetic */
    
    /* Struct with pointer access */
    struct {
        int a;
        int b;
        int c;
    } s, *sptr = &s;
    
    sptr->b = value;                    /* MEM with struct field */
    
    /* Multi-dimensional array */
    int matrix[4][4];
    matrix[offset % 4][g_index % 4] = value; /* MEM with 2D index */
    
    /* Address with shift */
    int *shifted = (int*)((char*)base + (offset << 2));
    *shifted = value;                   /* MEM with shifted address */
    
    asm volatile("" : : "r"(array), "r"(ptr), "r"(sptr), "r"(matrix), "r"(shifted));
}

/* ==================== Combined test function ==================== */
NOINLINE __attribute__((optimize("O2"))) 
void combined_test(int arg) {
    /* Mix all patterns in one function */
    volatile struct {
        unsigned int bf1 : 4;
        unsigned int bf2 : 12;
    } bits;
    
    bits.bf1 = arg & 0xF;           /* ZERO_EXTRACT */
    bits.bf2 = (arg >> 4) & 0xFFF;  /* ZERO_EXTRACT */
    
    short s = arg & 0xFFFF;
    int i = s;                      /* Potential STRICT_LOW_PART */
    
    /* Vector extract */
    typedef float v4f __attribute__((vector_size(16)));
    v4f vf = {1.0f, 2.0f, 3.0f, 4.0f};
    float f = vf[arg % 4];          /* SUBREG extract */
    
    /* Memory store with complex address */
    int buffer[32];
    int idx = (arg * 1103515245 + 12345) & 31;  /* Pseudorandom index */
    buffer[idx] = i;                /* MEM_P destination */
    
    /* Additional memory pattern */
    int *dynamic = buffer + (arg % 16);
    *dynamic = f;                   /* Another MEM_P */
    
    asm volatile("" : : "r"(bits), "r"(i), "r"(f), "r"(buffer), "r"(dynamic));
}

/* Helper to generate varying addresses */
NOINLINE int* get_pointer(int *base, int selector) {
    if (selector & 1) {
        return base + (selector % 8);
    } else {
        return base + 16 + (selector % 8);
    }
}

/* ==================== Main test driver ==================== */
int main(void) {
    int test_array[64];
    int i;
    
    /* Initialize test array */
    for (i = 0; i < 64; i++) {
        test_array[i] = i * 3;
    }
    
    /* Run tests multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part(i, i * 2);
        test_subreg();
        test_mem_p(test_array, i, i * 100);
        combined_test(i);
        
        /* Additional memory test with helper */
        int *ptr = get_pointer(test_array, i);
        *ptr = i * 200;                    /* MEM_P through helper */
    }
    
    /* Force use of all results */
    asm volatile("" : : "r"(test_array));
    
    return 0;
}
