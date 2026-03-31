/* test_resource.c - Coverage test for GCC's resource.cc mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile int g_temp;

/* ========== ZERO_EXTRACT patterns ========== */
NOINLINE void test_zero_extract(void) {
    /* Bit-field assignment - should generate ZERO_EXTRACT in SET_DEST */
    struct {
        volatile unsigned int field1 : 3;
        volatile unsigned int field2 : 5;
        volatile unsigned int field3 : 8;
    } bit_struct;
    
    bit_struct.field1 = 5;      /* ZERO_EXTRACT for bit-field store */
    bit_struct.field2 = 31;     /* Another bit-field */
    bit_struct.field3 = 255;    /* Full byte bit-field */
    
    /* Use asm to prevent optimization */
    asm volatile("" : : "r"(bit_struct.field1), "r"(bit_struct.field2));
}

NOINLINE void test_zero_extract_mixed(void) {
    /* Mixed bit-field and regular operations */
    union {
        struct {
            volatile unsigned int a : 4;
            volatile unsigned int b : 4;
            volatile unsigned int c : 4;
        } bits;
        volatile unsigned int full;
    } u;
    
    u.bits.a = 7;
    u.bits.b = 8;
    u.bits.c = 15;
    
    /* Force use of the union */
    g_temp = u.full;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE void test_strict_low_part(void) {
    /* Partial register writes - common on x86 */
    volatile short src16 = 0x1234;
    volatile char src8 = 0x56;
    
    /* These assignments may generate STRICT_LOW_PART */
    int dest32 = src16;          /* short -> int */
    long dest64 = src8;          /* char -> long */
    
    /* Mixed-size operations */
    dest32 = (dest32 & 0xFFFF0000) | src16;
    dest64 = (dest64 & 0xFFFFFFFFFFFFFF00) | src8;
    
    /* Use results */
    asm volatile("" : : "r"(dest32), "r"(dest64));
}

/* Compile with -m32 for better STRICT_LOW_PART generation */
NOINLINE __attribute__((optimize("O1"))) 
void test_strict_low_part_x86(void) {
    /* Explicit 32-bit x86 style partial register access */
    volatile uint16_t low16;
    volatile uint32_t full32;
    
    full32 = 0xDEADBEEF;
    low16 = 0x1234;
    
    /* This pattern often generates STRICT_LOW_PART on x86 */
    full32 = (full32 & 0xFFFF0000) | low16;
    
    g_temp = full32;
}

/* ========== SUBREG patterns ========== */
NOINLINE void test_subreg(void) {
    /* GCC vector extensions - often generate SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v8hi vec_short = {1, 2, 3, 4, 5, 6, 7, 8};
    
    /* Vector operations that may involve SUBREG */
    vec_a = vec_a + vec_b;
    
    /* Extract lane - may use SUBREG */
    int lane = vec_a[g_index & 3];
    
    /* Type punning through union - can generate SUBREG */
    union {
        float f;
        int i;
    } pun;
    
    pun.f = 3.14159f;
    int int_bits = pun.i;  /* SUBREG for type conversion */
    
    /* Mixed vector/scalar */
    vec_short[0] = (short)lane;
    
    asm volatile("" : : "r"(vec_a), "r"(int_bits), "r"(vec_short));
}

/* ========== MEM_P patterns ========== */
NOINLINE void test_mem_dest(void) {
    /* Complex memory destinations */
    int array[16];
    int *ptr;
    
    /* Store with complex address calculation */
    array[g_index * 2 + 1] = g_value;  /* MEM with index calculation */
    
    /* Pointer arithmetic store */
    ptr = &array[8];
    ptr[g_index - 2] = g_value * 2;    /* Another MEM destination */
    
    /* Struct member store through pointer */
    struct {
        int x;
        int y;
        int z;
    } point;
    
    struct *p = &point;
    p->y = g_value;                    /* MEM with field offset */
    
    /* Multi-dimensional array */
    int matrix[4][4];
    matrix[g_index & 3][g_index >> 2] = g_value;
    
    /* Use results to prevent elimination */
    asm volatile("" : : "m"(array), "m"(matrix), "m"(point));
}

/* ========== Combined test function ========== */
NOINLINE __attribute__((optimize("O2")))
void test_combined(void) {
    /* Mix all patterns in one function */
    
    /* ZERO_EXTRACT */
    struct {
        volatile unsigned int flags : 2;
        volatile unsigned int mode : 3;
    } settings;
    settings.flags = 1;
    settings.mode = 4;
    
    /* STRICT_LOW_PART */
    volatile short data16 = 1000;
    int combined = data16;
    
    /* SUBREG */
    typedef float v4f __attribute__((vector_size(16)));
    v4f floats = {1.0f, 2.0f, 3.0f, 4.0f};
    float f = floats[g_index & 3];
    
    /* MEM_P */
    int buffer[10];
    for (int i = 0; i < 10; i++) {
        buffer[i] = combined + i;  /* Store to memory */
    }
    
    /* Use everything */
    asm volatile("" : : "r"(settings.flags), "r"(combined), "r"(f), "m"(buffer));
}

/* Helper to create varying addresses */
NOINLINE int* get_pointer(int base, int offset) {
    static int storage[32];
    return &storage[(base + offset) & 31];
}

/* ========== Main test driver ========== */
int main(void) {
    /* Run tests multiple times with different conditions */
    for (int i = 0; i < 10; i++) {
        g_index = i;
        g_value = i * 10;
        
        test_zero_extract();
        test_zero_extract_mixed();
        test_strict_low_part();
        test_strict_low_part_x86();
        test_subreg();
        test_mem_dest();
        test_combined();
        
        /* Additional MEM_P test with helper */
        int* ptr = get_pointer(i, i * 2);
        *ptr = g_value;  /* MEM store with computed address */
    }
    
    return 0;
}
