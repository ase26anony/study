/* test_resource.c - Program to trigger specific RTL patterns in GCC's resource.cc */

#include <stdint.h>
#include <string.h>

/* Prevent optimizations from removing our test patterns */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile to prevent dead code elimination */
static volatile int global_counter = 0;

/* ==================== ZERO_EXTRACT Pattern ==================== */

/* Using bit-fields to trigger ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int full : 32;
    unsigned int part1 : 8;
    unsigned int part2 : 8;
    unsigned int part3 : 8;
    unsigned int part4 : 8;
} __attribute__((packed));

/* Also try __builtin_bitfield for modern GCC */
NOOPT void test_zero_extract(void) {
    volatile struct bitfield_struct bf;
    
    /* Writing to bit-field should generate ZERO_EXTRACT */
    bf.part1 = 0xAA;
    bf.part2 = 0xBB;
    bf.part3 = 0xCC;
    bf.part4 = 0xDD;
    
    /* Another approach using unions */
    union {
        uint32_t full;
        struct {
            uint32_t low16 : 16;
            uint32_t high16 : 16;
        } bits;
    } u;
    
    u.bits.low16 = 0x1234;
    u.bits.high16 = 0x5678;
    
    global_counter += bf.part1 + u.bits.low16;
}

/* ==================== STRICT_LOW_PART Pattern ==================== */

NOOPT void test_strict_low_part(void) {
    volatile uint16_t short_var;
    volatile uint8_t byte_var;
    
    /* Force partial register updates */
    asm volatile (
        "movw %1, %%ax\n\t"
        "movb %%al, %0\n\t"
        : "=m" (byte_var)
        : "r" ((uint16_t)0xABCD)
        : "ax"
    );
    
    /* Another approach using inline assembly with low-part modifier */
    uint32_t val32 = 0x12345678;
    uint16_t val16;
    
    asm volatile (
        "mov{l} {%1, %0 | %0, %1}\n\t"
        : "=r" (val16)
        : "r" (val32)
    );
    
    /* Force char assignment to volatile to prevent coalescing */
    volatile char char_target;
    int source_int = 0xDEADBEEF;
    char_target = (char)source_int;  /* Should generate low-part store */
    
    global_counter += byte_var + val16 + char_target;
}

/* ==================== SUBREG Pattern ==================== */

NOOPT void test_subreg(void) {
    /* Use packed structures and type punning */
    struct packed_data {
        int16_t a;
        int16_t b;
    } __attribute__((packed));
    
    volatile struct packed_data pd;
    pd.a = 100;
    pd.b = 200;
    
    /* Type punning through union */
    union subreg_union {
        uint32_t full;
        uint16_t halves[2];
    } su;
    
    su.full = 0x87654321;
    su.halves[0] = 0x1234;  /* Should involve SUBREG */
    
    /* Vector operations can also generate SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    int element = vec[2];  /* Element extraction may use SUBREG */
    
    global_counter += pd.a + su.halves[0] + element;
}

/* ==================== MEM_P with Complex Addressing ==================== */

NOOPT void test_complex_mem(void) {
    /* Multi-dimensional array with complex indexing */
    volatile int matrix[10][10][10];
    
    /* Complex address calculation */
    int i = global_counter % 10;
    int j = (global_counter + 1) % 10;
    int k = (global_counter + 2) % 10;
    
    matrix[i][j][k] = 42;
    int val = matrix[k][j][i];  /* Different complex address */
    
    /* Structure pointer chain */
    struct node {
        int value;
        struct node *next;
    };
    
    volatile struct node nodes[5];
    for (int idx = 0; idx < 4; idx++) {
        nodes[idx].value = idx * 10;
        nodes[idx].next = &nodes[idx + 1];
    }
    nodes[4].next = NULL;
    
    /* Chain of pointer dereferences */
    volatile struct node *current = &nodes[0];
    int sum = 0;
    while (current) {
        sum += current->value;
        current = current->next;
    }
    
    /* Inline assembly with memory clobber */
    int array[100];
    asm volatile (
        "movl $1, %0\n\t"
        : "=m" (array[10 + i * 3 + j])  /* Complex address */
        :
        : "memory"
    );
    
    global_counter += val + sum + array[10];
}

/* ==================== Combined Test ==================== */

NOOPT void test_combined(void) {
    /* Try to combine multiple patterns in one function */
    
    /* Bit-field (ZERO_EXTRACT) */
    struct {
        unsigned int field1 : 4;
        unsigned int field2 : 4;
    } bits;
    bits.field1 = 5;
    bits.field2 = 10;
    
    /* Partial register (STRICT_LOW_PART) */
    volatile short short_dest;
    int int_source = 0x12345678;
    short_dest = int_source;  /* Low part assignment */
    
    /* Complex memory access */
    volatile int arr[3][3][3];
    int x = bits.field1 % 3;
    int y = bits.field2 % 3;
    arr[x][y][0] = 99;
    
    global_counter += bits.field1 + short_dest + arr[x][y][0];
}

/* ==================== Main Function ==================== */

int main(void) {
    /* Call all test functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
        
        /* Modify global to prevent loop unrolling */
        global_counter += i;
    }
    
    /* Dummy computation to ensure code isn't eliminated */
    volatile int result = global_counter;
    
    return result != 0 ? 0 : 1;  /* Always return 0 unless everything was eliminated */
}
