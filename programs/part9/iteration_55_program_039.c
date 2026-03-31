/* test_resource.c - Coverage for GCC resource.cc mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile int g_condition = 1;

/* ==================== ZERO_EXTRACT patterns ==================== */
NOINLINE void test_zero_extract(void) {
    /* Bit-field assignment - should generate ZERO_EXTRACT in SET_DEST */
    struct bitfield_s {
        unsigned int field1 : 3;
        unsigned int field2 : 5;
        unsigned int field3 : 8;
    };
    volatile struct bitfield_s bf;
    
    /* Multiple bit-field assignments */
    bf.field1 = 5;          /* Should generate ZERO_EXTRACT for 3-bit field */
    bf.field2 = 0x1F;       /* Should generate ZERO_EXTRACT for 5-bit field */
    bf.field3 = g_value & 0xFF; /* Dynamic value prevents optimization */
    
    /* Nested bit-field in union */
    union {
        uint32_t full;
        struct {
            uint32_t low : 10;
            uint32_t mid : 10;
            uint32_t high : 12;
        } parts;
    } u;
    
    u.parts.low = 511;      /* Max 10-bit value */
    u.parts.mid = g_index;  /* Dynamic index */
    u.parts.high = 0;
    
    /* Force use of results */
    asm volatile("" : : "r"(bf), "r"(u));
}

/* ==================== STRICT_LOW_PART patterns ==================== */
NOINLINE void test_strict_low_part(void) {
    /* Mixed-size integer assignments - common source of STRICT_LOW_PART */
    volatile short src16 = -32768;
    volatile char src8 = 127;
    volatile int dest32;
    
    /* These assignments may generate STRICT_LOW_PART on x86 */
    dest32 = src16;         /* short -> int, preserving sign */
    dest32 = src8;          /* char -> int */
    
    /* Pointer-based partial writes */
    volatile uint32_t wide = 0xFFFFFFFF;
    volatile uint16_t *half_ptr = (volatile uint16_t*)&wide;
    half_ptr[0] = 0x1234;   /* Writing low 16 bits of 32-bit value */
    half_ptr[1] = 0x5678;   /* Writing high 16 bits */
    
    /* Arithmetic that forces partial register updates */
    volatile int accumulator = 0;
    for (int i = 0; i < 4; i++) {
        volatile short temp = i * 100;
        accumulator += temp;  /* May generate partial register writes */
    }
    
    asm volatile("" : : "r"(dest32), "r"(wide), "r"(accumulator));
}

/* ==================== SUBREG patterns ==================== */
NOINLINE void test_subreg(void) {
    /* GCC vector types - often generate SUBREG operations */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v8hi vec_short;
    
    /* Vector operations that generate SUBREG */
    vec_a = vec_a + vec_b;
    
    /* Type punning through union - generates SUBREG */
    union {
        v4si v;
        int a[4];
        float f[4];
    } u;
    
    u.v = vec_a;
    u.a[0] = g_value;       /* Array access may use SUBREG */
    
    /* Cast between vector types */
    vec_short = (v8hi)vec_a;  /* Type conversion */
    
    /* Complex number assignment - may use SUBREG */
    __complex__ double c1 = 1.0 + 2.0i;
    __complex__ double c2 = 3.0 + 4.0i;
    __complex__ float cf = (__complex__ float)c1;  /* Precision change */
    
    asm volatile("" : : "r"(vec_a), "r"(u), "r"(vec_short), "r"(cf));
}

/* ==================== MEM_P patterns ==================== */
NOINLINE void test_mem_p(void) {
    /* Complex memory addressing modes */
    int array[32];
    int *ptr;
    
    /* Store with complex address computation */
    for (int i = 0; i < 16; i++) {
        /* Non-constant index with arithmetic */
        int idx = (i * g_index + g_value) & 31;
        array[idx] = i * 100;  /* MEM with complex address */
    }
    
    /* Pointer arithmetic with multiple bases */
    ptr = &array[16];
    for (int i = -8; i < 8; i++) {
        ptr[i] = ptr[i] * 2 + 1;  /* Self-modifying with index */
    }
    
    /* Struct with pointer chain */
    struct node {
        int value;
        struct node *next;
    } nodes[8];
    
    /* Initialize linked structure */
    for (int i = 0; i < 7; i++) {
        nodes[i].value = i * 10;
        nodes[i].next = &nodes[i + 1];
    }
    nodes[7].next = NULL;
    
    /* Traversal with stores */
    struct node *current = &nodes[0];
    while (current) {
        current->value += g_value;  /* MEM through pointer indirection */
        current = current->next;
    }
    
    /* Multi-dimensional array with variable indices */
    int matrix[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            matrix[i][j] = i * 8 + j;  /* 2D array access */
        }
    }
    
    asm volatile("" : : "r"(array), "r"(ptr), "r"(nodes), "r"(matrix));
}

/* ==================== Combined patterns ==================== */
NOINLINE USED void test_combined(void) {
    /* Function that combines all patterns */
    
    /* ZERO_EXTRACT via bit-field */
    struct {
        unsigned int flags : 4;
        unsigned int mode : 3;
    } settings;
    settings.flags = g_value & 0x0F;
    settings.mode = g_index & 0x07;
    
    /* STRICT_LOW_PART via mixed sizes */
    volatile int32_t big = 0x12345678;
    volatile int16_t small = (int16_t)big;
    big = small;  /* Potential STRICT_LOW_PART */
    
    /* SUBREG via vector extraction */
    typedef float v4f __attribute__((vector_size(16)));
    v4f vf = {1.0f, 2.0f, 3.0f, 4.0f};
    float f = vf[g_index & 3];  /* Vector element extraction */
    
    /* MEM_P with complex addressing */
    int buffer[64];
    int *dynamic_ptr = &buffer[g_index];
    for (int i = 0; i < 16; i++) {
        dynamic_ptr[i * 2] = i * g_value;  /* Strided access */
    }
    
    asm volatile("" : : "r"(settings), "r"(big), "r"(f), "r"(buffer));
}

/* ==================== Main driver ==================== */
int main(void) {
    /* Call all test functions multiple times with different conditions */
    for (int i = 0; i < 3; i++) {
        g_index = i;
        g_value = 100 + i * 50;
        g_condition = i & 1;
        
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_mem_p();
        test_combined();
    }
    
    return 0;
}
