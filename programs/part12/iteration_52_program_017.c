/* test_resource.c - Coverage test for mark_referenced_resources patterns */

#include <stdint.h>
#include <string.h>

/* Prevent optimizations from merging or eliminating patterns */
#define NOOPT __attribute__((noinline, noipa, used))

/* Volatile to force actual operations */
volatile int global_counter = 0;

/* ===== ZERO_EXTRACT Pattern ===== */
/* Bit-field operations often generate ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 12;
    volatile unsigned int padding : 8;
};

NOOPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    
    /* Multiple bit-field writes to force ZERO_EXTRACT */
    bf.field1 = 5;
    bf.field2 = 0xAB;
    bf.field3 = 0x7FF;
    
    /* Complex bit-field expression */
    bf.field2 = (bf.field1 << 2) | 0x3;
    
    /* Use builtins for explicit bit-field manipulation */
    unsigned int val = 0x12345678;
    unsigned int result = __builtin_bitfield((val >> 4) & 0xF, 8, 4);
    
    global_counter += bf.field1 + bf.field2 + bf.field3 + result;
}

/* ===== STRICT_LOW_PART Pattern ===== */
NOOPT void test_strict_low_part(void) {
    volatile char char_var;
    volatile short short_var;
    volatile int int_var = 0x12345678;
    
    /* These assignments often generate STRICT_LOW_PART */
    char_var = (char)int_var;          /* Low byte assignment */
    short_var = (short)int_var;        /* Low word assignment */
    
    /* Inline assembly with low-part modifier (x86 specific) */
    int input = 0xABCD;
    int output;
    
    asm volatile (
        "movl %1, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=r" (output)
        : "r" (input)
        : "%eax"
    );
    
    /* Multiple partial register updates */
    volatile struct {
        char a;
        char b;
        char c;
        char d;
    } chars;
    
    chars.a = 0x11;
    chars.b = 0x22;
    chars.c = 0x33;
    chars.d = 0x44;
    
    global_counter += char_var + short_var + output + chars.a;
}

/* ===== SUBREG Pattern ===== */
NOOPT void test_subreg(void) {
    /* Type punning through union often generates SUBREG */
    union {
        uint32_t full;
        struct {
            uint16_t low;
            uint16_t high;
        } parts;
    } converter;
    
    converter.full = 0xDEADBEEF;
    converter.parts.low = 0xCAFE;      /* This may generate SUBREG */
    
    /* Packed structure forces sub-register access */
    struct __attribute__((packed)) packed_struct {
        char a;
        int b;
        char c;
    } ps;
    
    ps.b = 0x12345678;
    ps.a = 0xAA;
    
    /* Vector operations can generate SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    int element = vec[2];              /* Element extraction */
    
    /* Cast through different pointer types */
    uint64_t big_val = 0x1122334455667788ULL;
    uint32_t *ptr = (uint32_t*)&big_val;
    uint32_t half = ptr[1];            /* Accesses high 32 bits */
    
    global_counter += converter.parts.low + ps.b + element + half;
}

/* ===== MEM_P with Complex Addressing Pattern ===== */
NOOPT void test_complex_mem(void) {
    /* Multi-dimensional array with complex indexing */
    volatile int matrix[10][10][10];
    
    /* Complex addressing expression */
    int i = global_counter % 10;
    int j = (global_counter + 1) % 10;
    int k = (global_counter + 2) % 10;
    
    matrix[i][j][k] = 42;
    matrix[k][i][j] = matrix[j][k][i] + matrix[i][j][k];
    
    /* Structure with pointer chains */
    struct node {
        int value;
        struct node *next;
        struct node *prev;
    };
    
    struct node nodes[5];
    for (int idx = 0; idx < 4; idx++) {
        nodes[idx].next = &nodes[idx + 1];
        nodes[idx].prev = (idx > 0) ? &nodes[idx - 1] : 0;
        nodes[idx].value = idx * 10;
    }
    
    /* Complex pointer arithmetic */
    struct node *current = &nodes[0];
    current->next->prev->next->value = 999;
    
    /* Array of structs with field access */
    struct complex {
        int a[5];
        int b[3][3];
        struct {
            int x;
            int y;
        } nested;
    } arr[4];
    
    arr[1].a[2] = 100;
    arr[2].b[1][1] = 200;
    arr[3].nested.x = 300;
    
    /* Inline assembly with memory operand */
    int mem_val = 0;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (mem_val)
        : "m" (global_counter)
        : "%eax", "memory"
    );
    
    global_counter += matrix[0][0][0] + current->value + arr[1].a[0] + mem_val;
}

/* ===== Combined Test Function ===== */
NOOPT void test_combined(void) {
    /* Mix patterns in one function */
    volatile struct {
        unsigned int bits : 8;
        unsigned int full;
    } mixed;
    
    /* ZERO_EXTRACT pattern */
    mixed.bits = 0x55;
    
    /* STRICT_LOW_PART pattern */
    volatile short *ptr = (volatile short*)&mixed.full;
    *ptr = 0x1234;
    
    /* Complex MEM_P pattern */
    volatile int array[10];
    int idx = mixed.bits % 10;
    array[idx] = array[(idx + 1) % 10] + 1;
    
    global_counter += mixed.bits + *ptr + array[0];
}

/* Main function to ensure all tests are called */
int main(void) {
    /* Initialize to prevent dead code elimination */
    global_counter = 1;
    
    /* Execute all pattern tests */
    test_zero_extract();
    test_strict_low_part();
    test_subreg();
    test_complex_mem();
    test_combined();
    
    /* Additional iterations to increase coverage probability */
    for (int i = 0; i < 3; i++) {
        test_zero_extract();
        test_strict_low_part();
    }
    
    /* Return dummy value based on operations */
    return global_counter > 0 ? 0 : 1;
}
