/* test_resource.c - Coverage test for GCC's resource.cc mark_referenced_resources */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile int g_temp;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Bit-field structure for ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
    unsigned int field4 : 16;
};

/* Volatile bit-field to force ZERO_EXTRACT generation */
NOINLINE void test_zero_extract(void) {
    /* This should generate ZERO_EXTRACT in SET_DEST */
    volatile struct bitfield_struct bfs;
    bfs.field1 = 5;      /* ZERO_EXTRACT of 3 bits */
    bfs.field2 = 31;     /* ZERO_EXTRACT of 5 bits */
    bfs.field3 = 255;    /* ZERO_EXTRACT of 8 bits */
    bfs.field4 = 65535;  /* ZERO_EXTRACT of 16 bits */
    
    /* Use the values to prevent dead code elimination */
    g_temp = bfs.field1 + bfs.field2 + bfs.field3 + bfs.field4;
}

/* Another ZERO_EXTRACT pattern using unions */
NOINLINE void test_zero_extract_union(void) {
    union {
        volatile uint32_t full;
        struct {
            volatile uint32_t low : 10;
            volatile uint32_t mid : 10;
            volatile uint32_t high : 12;
        } bits;
    } u;
    
    u.bits.low = 1023;   /* ZERO_EXTRACT */
    u.bits.mid = 512;    /* ZERO_EXTRACT */
    u.bits.high = 4095;  /* ZERO_EXTRACT */
    
    g_temp = u.full;
}

/* ==================== STRICT_LOW_PART patterns ==================== */

/* Mixed-size assignments for STRICT_LOW_PART */
NOINLINE void test_strict_low_part(void) {
    volatile short src16 = -12345;
    volatile char src8 = 127;
    volatile int dest32;
    
    /* These assignments may generate STRICT_LOW_PART on x86 */
    dest32 = src16;      /* Possible STRICT_LOW_PART for 16->32 */
    g_temp = dest32;
    
    dest32 = src8;       /* Possible STRICT_LOW_PART for 8->32 */
    g_temp += dest32;
    
    /* Pointer casting for partial writes */
    volatile uint32_t *ptr32 = (volatile uint32_t*)&g_temp;
    volatile uint16_t *ptr16 = (volatile uint16_t*)ptr32;
    
    *ptr16 = 0xABCD;     /* Writing only 16 bits of 32-bit location */
}

/* Function with optimization level hint for STRICT_LOW_PART */
__attribute__((optimize("O1")))
NOINLINE void test_strict_low_part_opt(void) {
    volatile int a = 0x12345678;
    volatile short b;
    
    /* Extract lower 16 bits - may use STRICT_LOW_PART */
    b = (short)a;
    g_temp = b;
    
    /* Chain of partial writes */
    volatile char c = 0x7F;
    volatile int d = c;  /* char to int conversion */
    g_temp += d;
}

/* ==================== SUBREG patterns ==================== */

/* GCC vector types for SUBREG generation */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE void test_subreg(void) {
    /* Vector operations generate SUBREG */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c;
    
    vec_c = vec_a + vec_b;  /* Vector operation */
    
    /* Extract lane - generates SUBREG */
    int lane0 = vec_c[0];
    int lane1 = vec_c[1];
    g_temp = lane0 + lane1;
    
    /* Type punning through union - generates SUBREG */
    union {
        float f;
        int i;
    } pun;
    
    pun.f = 3.14159f;
    g_temp += pun.i;  /* SUBREG through type punning */
}

/* Mixed precision operations */
NOINLINE void test_subreg_mixed(void) {
    volatile double d = 2.71828;
    volatile float f;
    
    /* double to float conversion generates SUBREG */
    f = (float)d;
    
    /* Use GCC builtin for bit manipulation */
    volatile int i = __builtin_bit_cast(int, f);
    g_temp = i;
}

/* ==================== MEM_P patterns ==================== */

/* Complex memory addressing for MEM_P */
NOINLINE void test_mem_complex_addr(int idx) {
    volatile int array[100];
    volatile int *ptr;
    
    /* Complex address calculation */
    ptr = &array[idx * 2 + g_index];
    *ptr = g_value;  /* MEM with complex address */
    
    /* Pointer arithmetic with multiple operations */
    ptr = array + (idx & 0xF) * 3;
    ptr[g_index] = idx;  /* Another MEM with complex address */
    
    /* Struct with pointer chain */
    struct nested {
        int data[10];
        struct nested *next;
    } s1, s2;
    
    s1.next = &s2;
    s1.next->data[g_index] = 99;  /* MEM through pointer chain */
}

/* Function returning pointer for address computation */
static volatile int* get_ptr(int base, int offset) {
    static int buffer[256];
    return &buffer[(base + offset) & 0xFF];
}

NOINLINE void test_mem_func_addr(void) {
    /* MEM with address from function call */
    volatile int *p = get_ptr(g_index, 10);
    *p = 0xDEADBEEF;  /* MEM_P with function-derived address */
    
    /* Multiple indirections */
    volatile int **pp = &p;
    **pp = 0xCAFEBABE;  /* MEM through double pointer */
}

/* ==================== Combined test function ==================== */

/* Function that combines all patterns */
NOINLINE void test_combined(int seed) {
    /* ZERO_EXTRACT */
    volatile struct {
        unsigned int a : 4;
        unsigned int b : 12;
    } bits = {0};
    
    bits.a = seed & 0xF;      /* ZERO_EXTRACT */
    bits.b = seed & 0xFFF;    /* ZERO_EXTRACT */
    
    /* STRICT_LOW_PART */
    volatile short s = seed;
    volatile int i = s;       /* Possible STRICT_LOW_PART */
    
    /* SUBREG */
    v4sf vec = {1.0f, 2.0f, 3.0f, 4.0f};
    volatile float f = vec[seed & 3];  /* SUBREG */
    
    /* MEM_P with complex address */
    volatile int arr[16];
    int idx = (seed * 7) & 0xF;
    arr[idx] = i + bits.a + bits.b + (int)f;  /* MEM */
    
    g_temp = arr[idx];
}

/* ==================== Main driver ==================== */

int main(void) {
    int i;
    
    /* Test each pattern multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        test_zero_extract();
        test_zero_extract_union();
        test_strict_low_part();
        test_strict_low_part_opt();
        test_subreg();
        test_subreg_mixed();
        test_mem_complex_addr(i);
        test_mem_func_addr();
        test_combined(i);
    }
    
    /* Ensure all results are used */
    asm volatile("" : : "r"(g_temp));
    
    return 0;
}
