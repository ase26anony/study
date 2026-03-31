/* test_resource.c - Coverage test for GCC's resource.cc mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile char g_char = 'A';

/* ==================== ZERO_EXTRACT patterns ==================== */
NOINLINE void test_zero_extract(void) {
    /* Bit-field assignment should generate ZERO_EXTRACT in RTL */
    struct bitfield {
        unsigned int field1 : 3;
        unsigned int field2 : 5;
        unsigned int field3 : 8;
    };
    volatile struct bitfield bf;
    
    /* Multiple bit-field assignments */
    bf.field1 = 5;      /* Should generate ZERO_EXTRACT */
    bf.field2 = 20;     /* Should generate ZERO_EXTRACT */
    bf.field3 = 100;    /* Should generate ZERO_EXTRACT */
    
    /* Use asm to prevent dead code elimination */
    asm volatile("" : : "r"(bf));
}

NOINLINE void test_zero_extract_mixed(void) {
    /* Mixed bit-field and regular operations */
    struct mixed {
        unsigned short bf1 : 4;
        unsigned short bf2 : 6;
        unsigned int regular;
    };
    volatile struct mixed m;
    
    m.bf1 = 7;
    m.regular = 0xABCD;
    m.bf2 = 31;
    
    /* Force use of all fields */
    asm volatile("" : : "r"(m.bf1), "r"(m.regular), "r"(m.bf2));
}

/* ==================== STRICT_LOW_PART patterns ==================== */
NOINLINE void test_strict_low_part(void) {
    /* Partial register writes - common on x86 */
    volatile short src16 = 0x1234;
    volatile char src8 = 0x56;
    volatile int dest32 = 0xFFFFFFFF;
    
    /* These assignments may generate STRICT_LOW_PART */
    dest32 = src16;      /* 16-bit to 32-bit assignment */
    
    volatile int another = 0;
    another = src8;      /* 8-bit to 32-bit assignment */
    
    /* Mixed-size operations */
    volatile long long big = 0x1122334455667788LL;
    volatile int medium = 0xAABBCCDD;
    
    /* Casting through pointers can create partial writes */
    *(short*)(&medium) = src16;
    
    asm volatile("" : : "r"(dest32), "r"(another), "r"(medium), "r"(big));
}

/* ==================== SUBREG patterns ==================== */
NOINLINE void test_subreg(void) {
    /* GCC vector extensions often use SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v8hi vec_short = {1, 2, 3, 4, 5, 6, 7, 8};
    
    /* Vector operations */
    vec_a = vec_a + vec_b;
    
    /* Extract lane - may use SUBREG */
    volatile int lane = vec_a[2];
    
    /* Type punning through unions */
    union pun {
        float f;
        int i;
    } u;
    
    u.f = 3.14159f;
    volatile int int_bits = u.i;  /* Bitcast through union */
    
    /* Mixed vector/scalar operations */
    vec_short[3] = (short)lane;
    
    asm volatile("" : : "r"(vec_a), "r"(lane), "r"(int_bits), "r"(vec_short));
}

/* ==================== MEM_P patterns ==================== */
NOINLINE void test_mem_complex_address(void) {
    /* Complex memory addresses */
    int array[32];
    volatile int *ptr = array;
    
    /* Store with complex address calculation */
    ptr[g_index] = g_value;                     /* Indexed store */
    ptr[g_index + 2] = ptr[g_index] * 3;        /* Load and store */
    
    /* Pointer arithmetic */
    *(ptr + (g_index & 7)) = 99;
    
    /* Struct with pointer member */
    struct data {
        int values[8];
        int count;
    };
    
    struct data d;
    d.values[g_index % 8] = g_value;
    d.count = g_index;
    
    /* Multi-dimensional array */
    int matrix[4][4];
    matrix[g_index % 4][g_index / 4] = g_value;
    
    asm volatile("" : : "r"(array), "r"(ptr), "r"(d.values[0]), "r"(matrix[0][0]));
}

/* ==================== Combined test ==================== */
NOINLINE void test_combined(void) {
    /* Mix all patterns in one function */
    
    /* ZERO_EXTRACT */
    struct {
        unsigned int flags : 8;
        unsigned int mode : 4;
    } settings;
    settings.flags = 0xA5;
    settings.mode = 3;
    
    /* STRICT_LOW_PART */
    volatile short sval = 0x1234;
    volatile int ival = 0;
    ival = sval;  /* Potential STRICT_LOW_PART */
    
    /* SUBREG */
    typedef float v4f __attribute__((vector_size(16)));
    v4f fvec = {1.0f, 2.0f, 3.0f, 4.0f};
    volatile float felem = fvec[1];
    
    /* MEM_P with complex address */
    int buffer[16];
    for (int i = 0; i < 8; i++) {
        buffer[i * 2] = i + settings.flags;  /* Complex addressing */
    }
    
    /* More bit-field operations */
    settings.mode = (ival & 0xF);
    
    asm volatile("" : : "r"(settings.flags), "r"(ival), "r"(felem), "r"(buffer[0]));
}

/* ==================== Main driver ==================== */
int main(void) {
    /* Call all test functions multiple times with different conditions */
    for (int i = 0; i < 3; i++) {
        g_index = i;
        g_value = i * 100;
        
        test_zero_extract();
        test_zero_extract_mixed();
        test_strict_low_part();
        test_subreg();
        test_mem_complex_address();
        test_combined();
    }
    
    return 0;
}
