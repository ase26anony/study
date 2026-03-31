/* test_resource.c - Program to trigger specific RTL patterns in GCC's mark_referenced_resources */

#include <stdint.h>
#include <string.h>

/* Prevent optimization from eliminating our patterns */
#define NO_OPT __attribute__((noinline, noipa))

/* Volatile to force actual memory operations */
volatile int global_counter = 0;

/* ==================== ZERO_EXTRACT Pattern ==================== */

/* Bit-field structure that should generate ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int full : 32;
    volatile unsigned int low_bits : 8;
    volatile unsigned int middle_bits : 16;
    volatile unsigned int high_bit : 1;
    volatile unsigned int remaining : 7;
};

NO_OPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    
    /* These assignments should generate ZERO_EXTRACT for bit-field stores */
    bf.low_bits = 0xAB;
    bf.middle_bits = 0xCDEF;
    bf.high_bit = 1;
    bf.remaining = 0x7F;
    
    /* Mix with arithmetic to prevent dead code elimination */
    global_counter += bf.low_bits + bf.middle_bits;
}

/* Alternative using __builtin_bitfield */
NO_OPT void test_zero_extract_builtin(void) {
    volatile uint32_t value = 0x12345678;
    
    /* Extract and modify bit fields - may generate ZERO_EXTRACT */
    uint32_t field = __builtin_bitfield_extract(value, 4, 8);
    field = (field + 1) & 0xFF;
    value = __builtin_bitfield_insert(value, field, 4, 8);
    
    global_counter += value;
}

/* ==================== STRICT_LOW_PART Pattern ==================== */

NO_OPT void test_strict_low_part(void) {
    volatile uint16_t short_var;
    volatile uint8_t byte_var;
    volatile uint32_t int_var = 0x12345678;
    
    /* These assignments to partial registers may generate STRICT_LOW_PART */
    
    /* Assignment to low 16 bits */
    short_var = (uint16_t)(int_var + 1);
    
    /* Assignment to low 8 bits */
    byte_var = (uint8_t)(int_var + 2);
    
    /* Inline assembly that explicitly uses low part modifier */
    uint32_t result;
    __asm__ volatile (
        "movl $0x12345678, %0\n\t"
        "addl $1, %k0"  /* %k0 for 32-bit register (low part on x86) */
        : "=r" (result)
        :
        : "cc"
    );
    
    global_counter += short_var + byte_var + result;
}

/* ==================== SUBREG Pattern ==================== */

/* Packed structure to force sub-register accesses */
struct __attribute__((packed)) packed_data {
    uint8_t a;
    uint16_t b;
    uint8_t c;
    uint32_t d;
};

NO_OPT void test_subreg(void) {
    struct packed_data pd;
    pd.a = 0x11;
    pd.b = 0x2233;
    pd.c = 0x44;
    pd.d = 0x55667788;
    
    /* Type punning through union - may generate SUBREG */
    union {
        uint32_t full;
        uint16_t halves[2];
    } u;
    u.full = 0x89ABCDEF;
    
    /* Operations on sub-registers */
    u.halves[0] += 1;
    u.halves[1] -= 1;
    
    /* Vector operations (SIMD) can generate SUBREG */
    typedef uint32_t v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    uint32_t element = vec[2];  /* May use SUBREG to extract element */
    
    global_counter += pd.b + u.halves[0] + element;
}

/* ==================== MEM_P with Complex Addressing Pattern ==================== */

#define ARRAY_SIZE 100

NO_OPT void test_complex_mem(void) {
    volatile int array[ARRAY_SIZE][ARRAY_SIZE];
    volatile int *ptr_array[ARRAY_SIZE];
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < ARRAY_SIZE; j++) {
            array[i][j] = i * 100 + j;
        }
        ptr_array[i] = &array[i][0];
    }
    
    /* Complex addressing modes */
    int sum = 0;
    
    /* Multi-dimensional array with complex index */
    sum += array[global_counter % 50][(global_counter * 3) % 50];
    
    /* Pointer arithmetic with multiple offsets */
    int *ptr = &array[10][10];
    sum += *(ptr + global_counter % 20 - 10);
    
    /* Structure with pointer chain */
    struct node {
        int value;
        struct node *next;
    };
    
    struct node nodes[10];
    for (int i = 0; i < 9; i++) {
        nodes[i].value = i * 10;
        nodes[i].next = &nodes[i + 1];
    }
    nodes[9].value = 90;
    nodes[9].next = NULL;
    
    /* Complex memory access through pointer chain */
    sum += nodes[0].next->next->next->value;
    
    /* Inline assembly with memory clobber */
    int temp = 0;
    __asm__ volatile (
        "movl $42, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (temp)
        :
        : "eax", "memory"
    );
    
    sum += temp;
    
    global_counter += sum;
}

/* ==================== Combined Test Function ==================== */

NO_OPT void test_combined(void) {
    /* Test all patterns in one function */
    
    /* ZERO_EXTRACT via bit-field */
    struct {
        volatile unsigned int field1 : 4;
        volatile unsigned int field2 : 12;
    } bf;
    bf.field1 = 0xF;
    bf.field2 = 0xABC;
    
    /* STRICT_LOW_PART via partial register */
    volatile uint16_t low_part;
    uint32_t reg = 0x87654321;
    low_part = (uint16_t)reg;
    
    /* SUBREG via type punning */
    union {
        uint64_t full;
        uint32_t half[2];
    } u;
    u.full = 0x1122334455667788ULL;
    u.half[0] = 0x9999;
    
    /* Complex MEM_P via array with complex index */
    volatile int matrix[5][5] = {{0}};
    int idx1 = global_counter % 5;
    int idx2 = (global_counter * 7) % 5;
    matrix[idx1][idx2] = 42;
    
    global_counter += bf.field1 + low_part + u.half[0] + matrix[0][0];
}

/* ==================== Main Function ==================== */

int main(void) {
    /* Call all test functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_zero_extract_builtin();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
        
        /* Modify global to change behavior in loops */
        global_counter = (global_counter * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Dummy computation to prevent dead code elimination */
    volatile int result = global_counter;
    
    /* Return 0 to indicate success */
    return 0;
}
