/* test_resource.c - Program to trigger specific RTL patterns in GCC's mark_referenced_resources */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Prevent optimization from removing our patterns */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile to prevent dead code elimination */
volatile int global_counter = 0;

/* ===== ZERO_EXTRACT Pattern ===== */
/* Using bit-fields to generate ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int full : 32;
    volatile unsigned int low_bits : 8;
    volatile unsigned int middle_bits : 12;
    volatile unsigned int high_bits : 12;
};

/* Also try __builtin_bitfield operations */
NOOPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    volatile uint32_t value = 0x12345678;
    
    /* Writing to bit-field should generate ZERO_EXTRACT */
    bf.low_bits = (value & 0xFF);          /* Likely ZERO_EXTRACT */
    bf.middle_bits = (value >> 8) & 0xFFF; /* Another ZERO_EXTRACT candidate */
    
    /* Alternative approach with unions */
    union {
        volatile uint32_t full;
        struct {
            volatile uint32_t low : 10;
            volatile uint32_t mid : 10;
            volatile uint32_t high : 12;
        } bits;
    } u;
    
    u.full = 0;
    u.bits.mid = 0x3FF;  /* Should generate ZERO_EXTRACT */
    
    /* Use the values to prevent elimination */
    global_counter += bf.low_bits + u.bits.mid;
}

/* ===== STRICT_LOW_PART Pattern ===== */
NOOPT void test_strict_low_part(void) {
    volatile char char_var;
    volatile short short_var;
    volatile int int_var = 0x12345678;
    
    /* Disable optimization locally to prevent coalescing */
    asm volatile("" : : : "memory");
    
    /* These assignments may generate STRICT_LOW_PART */
    char_var = (char)int_var;    /* Low byte assignment */
    short_var = (short)int_var;  /* Low word assignment */
    
    /* Inline assembly with %L0 modifier for x86 low-part */
    int result;
    asm volatile (
        "movl %1, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=r" (result)
        : "r" (int_var)
        : "%eax"
    );
    
    /* Another approach: volatile struct with char member */
    struct {
        volatile int full;
        volatile char low_byte;
    } s;
    
    s.full = int_var;
    s.low_byte = 0x42;  /* May generate STRICT_LOW_PART */
    
    global_counter += char_var + short_var + result + s.low_byte;
}

/* ===== SUBREG Pattern ===== */
NOOPT void test_subreg(void) {
    /* Use packed structures to force sub-register accesses */
    struct __attribute__((packed)) packed_struct {
        char a;
        int b;
        char c;
    } ps;
    
    ps.b = 0x12345678;
    volatile int temp = ps.b;  /* May involve SUBREG due to misalignment */
    
    /* Type punning via union */
    union {
        uint32_t full;
        uint16_t halves[2];
        uint8_t bytes[4];
    } pun;
    
    pun.full = 0xDEADBEEF;
    volatile uint16_t half = pun.halves[1];  /* SUBREG likely */
    
    /* Vector types can generate SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    volatile int elem = vec[2];  /* Element extraction may use SUBREG */
    
    /* Complex bit manipulation */
    uint32_t x = 0x12345678;
    uint16_t y = (x >> 16) & 0xFFFF;  /* May involve SUBREG */
    
    global_counter += temp + half + elem + y;
}

/* ===== MEM_P with Complex Addressing Pattern ===== */
NOOPT void test_complex_mem(void) {
    /* Multi-dimensional array with complex indexing */
    volatile int arr[10][10][10];
    
    /* Complex address calculation */
    int i = global_counter % 10;
    int j = (global_counter + 1) % 10;
    int k = (global_counter + 2) % 10;
    
    /* These should generate complex MEM addresses */
    arr[i][j][k] = global_counter;
    volatile int val1 = arr[k][j][i];
    
    /* Structure pointer chains */
    struct node {
        int value;
        struct node *next;
        int data[5];
    };
    
    struct node nodes[5];
    for (int idx = 0; idx < 4; idx++) {
        nodes[idx].next = &nodes[idx + 1];
        nodes[idx].value = idx * 10;
    }
    
    /* Complex memory access through pointer chain */
    volatile int val2 = nodes[0].next->next->data[2];
    
    /* Pointer arithmetic with multiple offsets */
    int *base_ptr = &nodes[0].value;
    volatile int val3 = *(base_ptr + i + j * 2 + 3);
    
    /* Inline assembly with memory clobber */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (arr[5][5][5])
        : "m" (arr[0][0][0])
        : "%eax", "memory"
    );
    
    global_counter += val1 + val2 + val3 + arr[5][5][5];
}

/* ===== Combined Test Function ===== */
/* This function combines multiple patterns to maximize coverage */
NOOPT void test_combined(void) {
    /* ZERO_EXTRACT via bit-field in struct */
    struct {
        volatile unsigned int field1 : 5;
        volatile unsigned int field2 : 10;
        volatile unsigned int field3 : 15;
    } bits;
    
    bits.field2 = 0x2FF;  /* ZERO_EXTRACT */
    
    /* STRICT_LOW_PART via char assignment */
    volatile int src = 0xABCD1234;
    volatile char dst;
    dst = (char)src;  /* STRICT_LOW_PART */
    
    /* SUBREG via packed struct access */
    struct __attribute__((packed)) {
        char c;
        int i;
    } packed = {0, 0x87654321};
    volatile int extracted = packed.i;  /* SUBREG */
    
    /* Complex MEM access */
    volatile int matrix[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
    int idx1 = global_counter % 3;
    int idx2 = (global_counter + 1) % 3;
    volatile int mem_val = matrix[idx1][idx2];  /* Complex addressing */
    
    global_counter += bits.field2 + dst + extracted + mem_val;
}

/* Main function that runs all tests */
int main(void) {
    /* Initialize to prevent constant propagation */
    global_counter = 1;
    
    /* Run individual pattern tests */
    test_zero_extract();
    test_strict_low_part();
    test_subreg();
    test_complex_mem();
    
    /* Run combined test multiple times */
    for (int i = 0; i < 3; i++) {
        test_combined();
    }
    
    /* Dummy computation to ensure code isn't eliminated */
    volatile int result = global_counter;
    
    /* Return non-zero if any patterns were executed */
    return (result > 0) ? 0 : 1;
}
