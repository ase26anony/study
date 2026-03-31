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
    /* Bit-field assignment - should generate ZERO_EXTRACT in RTL */
    struct {
        volatile unsigned int field1 : 3;
        volatile unsigned int field2 : 5;
        volatile unsigned int field3 : 8;
    } bit_struct;
    
    bit_struct.field1 = 5;      /* ZERO_EXTRACT destination */
    bit_struct.field2 = 31;     /* Another bit-field write */
    bit_struct.field3 = 255;    /* Full byte bit-field */
    
    /* Volatile bit-field in union */
    union {
        volatile uint32_t full;
        struct {
            volatile uint32_t low : 10;
            volatile uint32_t mid : 10;
            volatile uint32_t high : 12;
        } bits;
    } bit_union;
    
    bit_union.bits.low = 1023;   /* ZERO_EXTRACT */
    bit_union.bits.mid = 512;    /* ZERO_EXTRACT */
    bit_union.bits.high = 4095;  /* ZERO_EXTRACT */
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(bit_struct), "r"(bit_union));
}

/* ==================== STRICT_LOW_PART patterns ==================== */
NOINLINE void test_strict_low_part(void) {
    /* Mixed-size integer assignments - may generate STRICT_LOW_PART */
    volatile short src16 = -12345;
    volatile char src8 = 127;
    volatile int dest32;
    
    /* These assignments might use STRICT_LOW_PART for partial register writes */
    dest32 = src16;      /* short -> int, preserving sign */
    dest32 = src8;       /* char -> int */
    
    /* Pointer-based partial writes */
    volatile uint32_t wide = 0xFFFFFFFF;
    volatile uint16_t *half_ptr = (volatile uint16_t*)&wide;
    half_ptr[0] = 0x1234;  /* Partial write to 32-bit variable */
    half_ptr[1] = 0x5678;  /* Another partial write */
    
    /* Use inline assembly hint for partial register write */
    uint32_t reg_var = 0;
    asm volatile("movw %1, %0" : "=r"(reg_var) : "i"(0xABCD));
    
    asm volatile("" : : "r"(dest32), "r"(wide), "r"(reg_var));
}

/* ==================== SUBREG patterns ==================== */
NOINLINE void test_subreg(void) {
    /* GCC vector types - often generate SUBREG operations */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v8hi vec_short = {1, 2, 3, 4, 5, 6, 7, 8};
    
    /* Vector operations that might use SUBREG */
    vec_a = vec_a + vec_b;          /* Vector arithmetic */
    int lane = vec_a[g_index & 3];  /* Lane extraction - SUBREG source/dest */
    
    /* Type punning through unions - can create SUBREG */
    union {
        float f;
        uint32_t i;
    } pun;
    
    pun.f = 3.14159f;
    pun.i = pun.i ^ 0x80000000;  /* Flip sign bit - SUBREG may be involved */
    
    /* Mixed-size accesses */
    volatile uint64_t big = 0x1122334455667788ULL;
    volatile uint32_t *big_half = (volatile uint32_t*)&big;
    big_half[0] = 0xAABBCCDD;  /* Partial write to 64-bit */
    
    asm volatile("" : : "r"(vec_a), "r"(lane), "r"(pun.f), "r"(big));
}

/* ==================== MEM_P patterns ==================== */
NOINLINE void test_mem_p(void) {
    /* Complex memory addresses - will generate MEM with address expressions */
    int array[100];
    int *ptr;
    
    /* Store with complex address calculation */
    array[g_index * 7 + 3] = g_value;  /* MEM destination with index expr */
    
    /* Pointer arithmetic with multiple operations */
    ptr = &array[10];
    ptr[g_index * 2 - 1] = g_value * 2;  /* More complex address */
    
    /* Struct with pointer chain */
    struct node {
        int value;
        struct node *next;
    } nodes[5];
    
    /* Initialize linked list */
    for (int i = 0; i < 4; i++) {
        nodes[i].value = i * 10;
        nodes[i].next = &nodes[i + 1];
    }
    nodes[4].next = NULL;
    
    /* Store through pointer chain */
    struct node *current = &nodes[0];
    for (int i = 0; i < 3 && current; i++) {
        current->value = g_value + i;  /* MEM destination through pointer */
        current = current->next;
    }
    
    /* Multi-dimensional array with variable indices */
    int matrix[10][10];
    int row = g_index % 10;
    int col = (g_index * 3) % 10;
    matrix[row][col] = g_value;  /* MEM with 2D index calculation */
    
    asm volatile("" : : "r"(array), "r"(ptr), "r"(nodes), "r"(matrix));
}

/* ==================== Combined test function ==================== */
/* Use O2 optimization specifically for this function */
__attribute__((optimize("O2"))) 
NOINLINE void test_combined(void) {
    /* Mix all patterns in one function */
    volatile struct {
        unsigned int bf1 : 4;
        unsigned int bf2 : 4;
    } bits = {0};
    
    bits.bf1 = 7;  /* ZERO_EXTRACT */
    
    volatile short s = 1000;
    volatile int i = s;  /* Possible STRICT_LOW_PART */
    
    /* Vector type for SUBREG */
    typedef float v2f __attribute__((vector_size(8)));
    v2f v = {1.0f, 2.0f};
    float f = v[0];  /* SUBREG extraction */
    
    /* MEM with address calculation */
    int arr[20];
    arr[g_index + 2] = i;  /* MEM destination */
    
    /* Loop to prevent over-optimization */
    for (int j = 0; j < 5; j++) {
        bits.bf2 = j & 0xF;  /* ZERO_EXTRACT in loop */
        arr[j] = s + j;      /* MEM in loop */
    }
    
    asm volatile("" : : "r"(bits), "r"(i), "r"(f), "r"(arr));
}

/* ==================== Main driver ==================== */
int main(void) {
    /* Call test functions multiple times with different conditions */
    for (int iter = 0; iter < 10; iter++) {
        g_index = (iter * 13) % 97;
        g_value = iter * 17;
        g_condition = iter & 1;
        
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_mem_p();
        test_combined();
    }
    
    return 0;
}
