/* test_resource.c - Program to trigger specific RTL patterns for coverage testing */

#include <stdint.h>
#include <string.h>

/* Prevent optimizations from removing our test patterns */
#define NO_OPT __attribute__((noinline, noipa))

/* Volatile to force actual operations */
volatile int global_counter = 0;

/* ========== ZERO_EXTRACT Pattern ========== */
/* Using bit-fields to generate ZERO_EXTRACT RTL */
struct bitfield_struct {
    volatile unsigned int full : 32;
    volatile unsigned int low_bits : 8;
    volatile unsigned int middle_bits : 16;
    volatile unsigned int high_bit : 1;
};

NO_OPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    
    /* These assignments should generate ZERO_EXTRACT for bit-field stores */
    bf.low_bits = 0xAB;          /* 8-bit field */
    bf.middle_bits = 0xCDEF;     /* 16-bit field */
    bf.high_bit = 1;             /* 1-bit field */
    
    /* Complex bit-field operation */
    bf.low_bits = (bf.middle_bits >> 4) & 0x0F;
    
    /* Force use to prevent elimination */
    global_counter += bf.low_bits + bf.high_bit;
}

/* ========== STRICT_LOW_PART Pattern ========== */
NO_OPT void test_strict_low_part(void) {
    volatile uint32_t value32;
    volatile uint16_t value16;
    volatile uint8_t value8;
    
    /* These should generate STRICT_LOW_PART for partial register updates */
    value16 = 0x1234;
    value8 = 0xAB;
    
    /* Type punning through union to force partial register access */
    union {
        uint32_t dword;
        uint16_t word;
        uint8_t byte;
    } pun;
    
    pun.dword = 0xDEADBEEF;
    pun.word = 0xCAFE;      /* Low 16-bit update */
    pun.byte = 0x42;        /* Low 8-bit update */
    
    /* Inline assembly for explicit STRICT_LOW_PART on x86 */
    asm volatile (
        "movw %1, %%ax\n\t"
        "movb %2, %%al\n\t"
        "movw %%ax, %0\n\t"
        : "=m" (value16)
        : "r" ((uint16_t)0x1234), "r" ((uint8_t)0x56)
        : "ax"
    );
    
    global_counter += pun.dword + value16 + value8;
}

/* ========== SUBREG Pattern ========== */
NO_OPT void test_subreg(void) {
    /* Use vector types to force SUBREG operations */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short;
    
    /* Type punning between different vector sizes */
    memcpy(&vec_short, &vec_int, sizeof(vec_short));
    
    /* Extract and manipulate sub-elements */
    int temp = vec_int[0] + vec_int[1];
    vec_int[2] = temp;
    
    /* Union for type-punning SUBREG generation */
    union {
        uint32_t i;
        uint16_t s[2];
        uint8_t b[4];
    } converter;
    
    converter.i = 0x12345678;
    converter.s[0] = converter.s[1] + 1;  /* SUBREG for 16-bit access */
    converter.b[2] = converter.b[0] * 2;  /* SUBREG for 8-bit access */
    
    global_counter += vec_int[0] + converter.i;
}

/* ========== MEM_P with Complex Addressing Pattern ========== */
NO_OPT void test_complex_mem(void) {
    /* Multi-dimensional array with complex indexing */
    volatile int matrix[10][10][10];
    
    /* Complex addressing expressions */
    int i = global_counter % 10;
    int j = (global_counter * 3) % 10;
    int k = (global_counter * 7) % 10;
    
    /* These should generate complex address expressions */
    matrix[i][j][k] = matrix[k][j][i] + matrix[j][i][k];
    matrix[i+1][j*2][k/2] = matrix[i][j][k] * 3;
    
    /* Structure with pointer chains */
    struct node {
        int value;
        struct node *next;
        struct node *prev;
    };
    
    struct node nodes[5];
    struct node *ptr = &nodes[0];
    
    /* Initialize linked list */
    for (int idx = 0; idx < 4; idx++) {
        nodes[idx].next = &nodes[idx + 1];
        nodes[idx + 1].prev = &nodes[idx];
        nodes[idx].value = idx * 10;
    }
    
    /* Complex memory access through pointer chain */
    ptr->next->next->value = ptr->prev ? ptr->prev->value : 0;
    ptr->next->value = ptr->next->next->value + ptr->value;
    
    /* Array of pointers with offset calculation */
    int *ptr_array[5];
    int data[20];
    
    for (int idx = 0; idx < 5; idx++) {
        ptr_array[idx] = &data[idx * 3];
    }
    
    /* Very complex addressing */
    *(ptr_array[i] + j * 2 + k) = 
        *(ptr_array[j] + i * 3 + k) + 
        *(ptr_array[k] + j + i * 4);
    
    global_counter += matrix[0][0][0] + nodes[0].value + data[0];
}

/* ========== Combined Test Function ========== */
NO_OPT void test_combined(void) {
    /* Test all patterns in one function */
    
    /* ZERO_EXTRACT via bit-field */
    struct {
        volatile unsigned int field1 : 4;
        volatile unsigned int field2 : 12;
    } bits;
    
    bits.field1 = 0xF;
    bits.field2 = bits.field1 << 4;
    
    /* STRICT_LOW_PART via inline assembly */
    volatile short low_part;
    asm volatile (
        "movw $0x1234, %0\n\t"
        : "=r" (low_part)
        :
    );
    
    /* SUBREG via type punning */
    union {
        uint32_t dword;
        uint16_t words[2];
    } reg;
    
    reg.dword = 0xAABBCCDD;
    reg.words[0] = reg.words[1] + 1;
    
    /* MEM_P with complex addressing */
    volatile int array[10][10];
    int idx1 = global_counter % 10;
    int idx2 = (global_counter * 5) % 10;
    
    array[idx1][idx2] = array[idx2][idx1] * 2 + 1;
    array[idx1 + 1][idx2 - 1] = array[idx1][idx2] + 100;
    
    global_counter += bits.field1 + bits.field2 + low_part + reg.dword + array[0][0];
}

/* ========== Main Function ========== */
int main(void) {
    /* Initialize global counter */
    global_counter = 1;
    
    /* Execute all test functions multiple times with different values */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
        
        /* Modify global counter to vary patterns */
        global_counter = (global_counter * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Dummy computation to ensure code isn't eliminated */
    volatile int result = global_counter;
    
    /* Return success */
    return 0;
}
