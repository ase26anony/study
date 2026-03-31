/* test_resource.c - Coverage test for GCC's resource.cc mark_referenced_resources */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile int g_condition = 1;

/* ===== ZERO_EXTRACT patterns ===== */
/* Bit-field assignments generate ZERO_EXTRACT in RTL */
struct bitfield_struct {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
};

NOINLINE void test_zero_extract(void) {
    volatile struct bitfield_struct s;
    
    /* These assignments should generate ZERO_EXTRACT as SET_DEST */
    s.field1 = 5;        /* ZERO_EXTRACT for 3-bit field */
    s.field2 = 20;       /* ZERO_EXTRACT for 5-bit field */
    s.field3 = 100;      /* ZERO_EXTRACT for 8-bit field */
    
    /* Use asm to prevent dead code elimination */
    asm volatile("" : : "r"(s.field1), "r"(s.field2), "r"(s.field3));
}

/* ===== STRICT_LOW_PART patterns ===== */
/* Partial register writes on x86 generate STRICT_LOW_PART */
NOINLINE void test_strict_low_part(short src_short, char src_char) {
    int dest_int;
    long dest_long;
    
    /* These assignments may generate STRICT_LOW_PART for partial register writes */
    dest_int = src_short;    /* short -> int, writing only low 16 bits */
    dest_long = src_char;    /* char -> long, writing only low 8 bits */
    
    /* Force use of the results */
    asm volatile("" : : "r"(dest_int), "r"(dest_long));
    
    /* Mixed-size operations */
    unsigned short us = 0xABCD;
    unsigned int ui = us;    /* Potential STRICT_LOW_PART */
    
    /* Prevent optimization */
    asm volatile("" : : "r"(ui));
}

/* ===== SUBREG patterns ===== */
/* Using GCC vector extensions to generate SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE void test_subreg(void) {
    v4si vec_int = {1, 2, 3, 4};
    v4sf vec_float = {1.0f, 2.0f, 3.0f, 4.0f};
    int lane;
    float flane;
    
    /* Vector lane extraction generates SUBREG */
    lane = vec_int[g_index & 3];    /* SUBREG for extracting vector element */
    
    /* Type punning through union generates SUBREG */
    union {
        float f;
        int i;
    } pun;
    pun.f = 3.14f;
    int int_bits = pun.i;          /* SUBREG for type conversion */
    
    /* Vector conversion */
    v4si int_from_float = __builtin_convertvector(vec_float, v4si);
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(lane), "r"(int_bits), "r"(int_from_float[0]));
}

/* ===== MEM_P patterns ===== */
/* Complex memory addresses generate MEM with address expressions */
int global_array[100];
struct complex_addr {
    int data[10];
    struct complex_addr *next;
};

NOINLINE void test_mem_dest(int idx, int value) {
    /* Array with variable index - complex address calculation */
    global_array[idx * 2 + 1] = value;  /* MEM with address expression */
    
    /* Pointer arithmetic */
    int *ptr = &global_array[10];
    ptr[idx % 5] = value * 2;           /* Another MEM with address */
    
    /* Struct member through pointer */
    struct complex_addr node;
    struct complex_addr *node_ptr = &node;
    node_ptr->data[idx % 10] = value;   /* MEM with struct offset */
    
    /* Multi-dimensional array */
    int matrix[5][5];
    matrix[idx % 5][(idx + 1) % 5] = value;
    
    /* Prevent optimization */
    asm volatile("" : : "r"(global_array[0]), "r"(node.data[0]), "r"(matrix[0][0]));
}

/* ===== Combined test function ===== */
/* Function that mixes all patterns to maximize coverage */
NOINLINE __attribute__((optimize("O2"))) 
void combined_test(int param) {
    /* ZERO_EXTRACT pattern */
    struct {
        unsigned int bf1 : 4;
        unsigned int bf2 : 12;
    } bits;
    bits.bf1 = param & 0xF;
    bits.bf2 = (param >> 4) & 0xFFF;
    
    /* STRICT_LOW_PART pattern */
    short s_val = param & 0xFFFF;
    int i_val = s_val;  /* Potential STRICT_LOW_PART */
    
    /* SUBREG pattern with vectors */
    typedef char v16qi __attribute__((vector_size(16)));
    v16qi chars = {0};
    for (int i = 0; i < 16; i++) {
        chars[i] = (param + i) & 0xFF;  /* SUBREG for vector stores */
    }
    
    /* MEM_P pattern with complex addressing */
    int local_arr[20];
    for (int i = 0; i < 10; i++) {
        /* Complex address: base + scaled index + offset */
        local_arr[i * 2 + (param & 1)] = i_val + i;  /* MEM destination */
    }
    
    /* Use results to prevent optimization */
    asm volatile("" : : "r"(bits.bf1), "r"(i_val), "r"(chars[0]), "r"(local_arr[0]));
}

/* ===== Main test driver ===== */
int main(void) {
    /* Initialize global data */
    memset(global_array, 0, sizeof(global_array));
    
    /* Test each pattern individually */
    test_zero_extract();
    test_strict_low_part(12345, 67);
    test_subreg();
    test_mem_dest(g_index, g_value);
    
    /* Test combined patterns with different parameters */
    for (int i = 0; i < 10; i++) {
        combined_test(i * 100 + 123);
    }
    
    /* Additional stress test with more complex control flow */
    if (g_condition) {
        test_zero_extract();
        test_mem_dest(g_index + 1, g_value * 2);
    }
    
    return 0;
}
