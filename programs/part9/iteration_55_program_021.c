/* test_resource.c - Coverage test for GCC resource.cc mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile char g_char = 'A';
volatile short g_short = 1234;

/* ===== ZERO_EXTRACT patterns ===== */
/* Bit-field assignments often generate ZERO_EXTRACT in RTL */
struct bitfield_struct {
    unsigned int field3 : 3;
    unsigned int field5 : 5;
    unsigned int field8 : 8;
};

NOINLINE void test_zero_extract(void) {
    volatile struct bitfield_struct bfs;
    
    /* These assignments should generate ZERO_EXTRACT as SET_DEST */
    bfs.field3 = g_value & 0x7;      /* 3-bit field */
    bfs.field5 = g_value & 0x1F;     /* 5-bit field */
    bfs.field8 = g_value & 0xFF;     /* 8-bit field */
    
    /* Use asm to prevent dead code elimination */
    asm volatile("" : : "r"(bfs));
}

/* ===== STRICT_LOW_PART patterns ===== */
/* Partial register writes, especially on 32-bit x86 */
NOINLINE void test_strict_low_part(void) {
    int dest32;
    short src16 = g_short;
    char src8 = g_char;
    
    /* These assignments may generate STRICT_LOW_PART */
    dest32 = src16;          /* short -> int */
    dest32 = src8;           /* char -> int */
    
    /* Mixed operations to encourage partial register updates */
    dest32 = (dest32 & 0xFFFF0000) | (src16 & 0xFFFF);
    
    asm volatile("" : : "r"(dest32));
}

/* ===== SUBREG patterns ===== */
/* Vector types and type punning generate SUBREG */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE void test_subreg(void) {
    v4si vec_int = {1, 2, 3, 4};
    v4sf vec_float = {1.0f, 2.0f, 3.0f, 4.0f};
    int lane;
    float f;
    
    /* Vector lane extraction - often uses SUBREG */
    lane = vec_int[g_index & 3];
    
    /* Type punning through union - may generate SUBREG */
    union {
        float f;
        int i;
    } u;
    u.f = 3.14f;
    lane = u.i;  /* float bits -> int */
    
    /* Vector conversion - can generate SUBREG */
    vec_int = __builtin_convertvector(vec_float, v4si);
    
    asm volatile("" : : "r"(lane), "r"(vec_int));
}

/* ===== MEM_P patterns ===== */
/* Complex memory addresses for MEM destinations */
NOINLINE void test_mem_dest(int *base, int offset) {
    int array[16];
    int *ptr;
    
    /* Complex addressing modes */
    base[offset] = g_value;                     /* Base + offset */
    array[g_index & 0xF] = g_value;             /* Array with variable index */
    
    /* Pointer arithmetic */
    ptr = base + (g_index * 2);
    *ptr = g_value;
    
    /* Two-dimensional access simulation */
    int matrix[4][4];
    matrix[g_index & 3][(g_index >> 2) & 3] = g_value;
    
    asm volatile("" : : "r"(array), "r"(matrix));
}

/* ===== Combined test with all patterns ===== */
NOINLINE void test_combined(int *mem, int idx) {
    /* ZERO_EXTRACT via bit-field */
    struct {
        unsigned int a : 4;
        unsigned int b : 12;
    } s;
    s.a = idx & 0xF;
    s.b = (idx * 7) & 0xFFF;
    
    /* STRICT_LOW_PART via partial assignment */
    int x = 0x12345678;
    short y = idx;
    x = (x & 0xFFFF0000) | y;  /* Only modify low 16 bits */
    
    /* SUBREG via vector operation */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;
    int element = v3[idx & 3];
    
    /* MEM_P with complex address */
    mem[idx] = element;
    mem[idx + 1] = x;
    
    asm volatile("" : : "r"(s), "r"(x), "r"(element), "r"(mem));
}

/* ===== Main test driver ===== */
int main(void) {
    int buffer[32] = {0};
    int i;
    
    /* Run each test multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_mem_dest(buffer, i & 0xF);
        test_combined(buffer, i & 0x1F);
    }
    
    /* Return something based on the results */
    return buffer[0] + buffer[31];
}
