/* test_resource.c - Coverage test for GCC resource.cc mark_referenced_resources */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 0;
volatile int g_value = 42;
volatile int g_temp = 0;

/* ===== ZERO_EXTRACT patterns ===== */
/* Bit-field assignments often generate ZERO_EXTRACT in RTL */
struct bitfield_struct {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
    volatile unsigned int field4 : 4;  /* volatile to prevent optimization */
};

NOINLINE void test_zero_extract(void) {
    /* These assignments should generate ZERO_EXTRACT as SET_DEST */
    volatile struct bitfield_struct bfs;
    bfs.field1 = 5;      /* ZERO_EXTRACT expected */
    bfs.field2 = 31;     /* ZERO_EXTRACT expected */
    bfs.field3 = 255;    /* ZERO_EXTRACT expected */
    bfs.field4 = 15;     /* ZERO_EXTRACT with volatile */
    
    /* Force use of results */
    g_temp = bfs.field1 + bfs.field2;
}

/* Another ZERO_EXTRACT pattern using unions */
union bit_union {
    uint32_t full;
    struct {
        uint32_t low : 10;
        uint32_t mid : 10;
        uint32_t high : 12;
    } parts;
};

NOINLINE void test_zero_extract_union(int a, int b) {
    volatile union bit_union bu;
    bu.parts.low = a & 0x3FF;    /* ZERO_EXTRACT expected */
    bu.parts.mid = b & 0x3FF;    /* ZERO_EXTRACT expected */
    bu.parts.high = (a + b) & 0xFFF; /* ZERO_EXTRACT expected */
    
    g_temp = bu.full;
}

/* ===== STRICT_LOW_PART patterns ===== */
/* Partial register writes, especially on 32-bit x86 */
NOINLINE void test_strict_low_part(void) {
    /* Mixed-size assignments that may generate STRICT_LOW_PART */
    volatile short src16 = 0x1234;
    volatile char src8 = 0x56;
    volatile int dest32 = 0;
    
    /* These assignments might generate STRICT_LOW_PART when optimized */
    dest32 = src16;      /* Possible STRICT_LOW_PART for lower 16 bits */
    dest32 = src8;       /* Possible STRICT_LOW_PART for lower 8 bits */
    
    /* Use inline assembly to force partial register writes on x86 */
    #ifdef __i386__
    asm volatile (
        "movw %1, %0\n\t"        /* 16-bit write to 32-bit register */
        : "=r" (dest32)
        : "r" (src16)
    );
    #endif
    
    g_temp = dest32;
}

/* More STRICT_LOW_PART patterns with arithmetic */
NOINLINE int test_partial_reg_update(int x, short y) {
    /* This sequence might generate STRICT_LOW_PART */
    int result = x;
    result = y;          /* Assign short to int - partial update */
    
    /* Complex expression to prevent optimization */
    if (result & 1) {
        char c = result & 0xFF;
        result = c;      /* Another partial update */
    }
    
    return result;
}

/* ===== SUBREG patterns ===== */
/* Vector types and type punning generate SUBREG */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE void test_subreg(void) {
    /* Vector operations generate SUBREG for lane access */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c;
    
    vec_c = vec_a + vec_b;  /* Vector operation */
    
    /* Extract lane - generates SUBREG */
    int lane0 = vec_c[0];   /* SUBREG expected as SET_DEST */
    int lane1 = vec_c[1];
    
    /* Type punning through union generates SUBREG */
    union {
        float f;
        int i;
    } pun;
    
    pun.f = 3.14159f;
    int int_bits = pun.i;   /* SUBREG expected */
    
    /* Mixed vector sizes */
    v8hi short_vec = {1, 2, 3, 4, 5, 6, 7, 8};
    short_vec[3] = 99;      /* SUBREG for vector lane store */
    
    g_temp = lane0 + int_bits + short_vec[0];
}

/* ===== MEM_P patterns ===== */
/* Complex memory addresses */
int global_array[100];
struct complex_addr {
    int data[20];
    int offset;
};

NOINLINE void test_mem_dest(int idx, int value) {
    /* Complex array indexing */
    global_array[idx * 2 + 1] = value;  /* MEM with complex address */
    
    /* Pointer arithmetic */
    int *ptr = &global_array[10];
    ptr[idx & 0xF] = value * 2;         /* MEM with computed address */
    
    /* Struct member through pointer */
    struct complex_addr ca;
    struct complex_addr *cap = &ca;
    cap->data[idx % 20] = value;        /* MEM with struct offset */
    
    /* Multi-dimensional array */
    int md_array[10][10];
    md_array[idx / 10][idx % 10] = value; /* MEM with 2D index */
    
    /* Force address computation */
    volatile int *volatile_ptr = &global_array[0];
    volatile_ptr += idx;
    *volatile_ptr = value;              /* MEM with volatile pointer */
}

/* Combined test that mixes all patterns */
NOINLINE int combined_test(int x, int y) {
    /* ZERO_EXTRACT */
    volatile struct bitfield_struct bfs;
    bfs.field1 = x & 0x7;
    
    /* STRICT_LOW_PART */
    short s = y & 0xFFFF;
    int partial = s;
    
    /* SUBREG */
    v4si vec = {x, y, x+y, x-y};
    int elem = vec[1];
    
    /* MEM_P */
    global_array[(x + y) % 100] = elem;
    
    /* Complex control flow to prevent optimization */
    int result = bfs.field1 + partial + elem;
    if (result > 1000) {
        test_mem_dest(x % 50, result);
    }
    
    return result;
}

/* Main driver that calls all test functions */
int main(void) {
    int i;
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract();
    test_zero_extract_union(100, 200);
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part();
    for (i = 0; i < 10; i++) {
        test_partial_reg_update(i * 100, i * 10);
    }
    
    /* Test SUBREG patterns */
    test_subreg();
    
    /* Test MEM_P patterns */
    for (i = 0; i < 20; i++) {
        test_mem_dest(i, i * 3);
    }
    
    /* Combined test */
    for (i = 0; i < 5; i++) {
        combined_test(i * 11, i * 7);
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(g_temp));
    
    return 0;
}
