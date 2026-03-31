/* test_resource.c - Comprehensive test for uncovered lines in resource.cc */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

/* Prevent optimizations from removing our test patterns */
#define NOOPT __attribute__((noinline, noipa, used))

/* Volatile to force actual memory operations */
volatile int global_counter = 0;

/* ===== ZERO_EXTRACT Pattern ===== */

/* Bit-field structure that should generate ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int field1 : 4;
    unsigned int field2 : 8;
    unsigned int field3 : 4;
    unsigned int field4 : 16;
} __attribute__((packed));

/* Test ZERO_EXTRACT through volatile bit-field writes */
NOOPT void test_zero_extract(void) {
    volatile struct bitfield_struct bf;
    
    /* Multiple bit-field assignments to increase chances */
    bf.field1 = 5;
    bf.field2 = 0xAB;
    bf.field3 = 3;
    bf.field4 = 0xDEAD;
    
    /* Complex bit-field extraction and assignment */
    unsigned int temp = (bf.field2 << 4) | bf.field1;
    bf.field3 = temp & 0xF;
    
    global_counter += bf.field1 + bf.field2;
}

/* Alternative approach using __builtin_bitfield */
NOOPT void test_zero_extract_builtin(void) {
    volatile uint32_t value = 0x12345678;
    
    /* Write to specific bit ranges */
    __builtin_bitfield_write32(&value, 8, 4, 0xA);  /* bits 8-11 */
    __builtin_bitfield_write32(&value, 16, 8, 0xBC); /* bits 16-23 */
    
    /* Read from bit ranges */
    uint32_t extracted = __builtin_bitfield_read32(&value, 4, 12);
    global_counter += extracted;
}

/* ===== STRICT_LOW_PART Pattern ===== */

/* Force STRICT_LOW_PART through partial register updates */
NOOPT void test_strict_low_part(void) {
    volatile uint16_t short_var;
    volatile uint8_t char_var;
    volatile uint32_t int_var = 0x12345678;
    
    /* These assignments may generate STRICT_LOW_PART */
    short_var = (uint16_t)(int_var >> 8);
    char_var = (uint8_t)(int_var & 0xFF);
    
    /* Inline assembly with %b0 (low byte) modifier on x86 */
    uint32_t reg_var;
    asm volatile (
        "movl $0x89ABCDEF, %0\n\t"
        "movb $0x12, %b0\n\t"
        : "=r" (reg_var)
        :
        : "cc"
    );
    
    global_counter += short_var + char_var + reg_var;
}

/* More explicit STRICT_LOW_PART generation */
NOOPT void test_strict_low_part_asm(void) {
    uint32_t x;
    
    /* Using %L0 modifier for low part (x86-specific) */
    asm volatile (
        "movl $0xFFFFFFFF, %0\n\t"
        /* Force low byte update */
        "movb $0xAA, %L0\n\t"
        : "=r" (x)
        :
        : "cc"
    );
    
    /* Force compiler to use partial register */
    volatile uint8_t *byte_ptr = (volatile uint8_t*)&x;
    *byte_ptr = 0xBB;
    
    global_counter += x;
}

/* ===== SUBREG Pattern ===== */

/* Packed structure to force SUBREG accesses */
struct packed_data {
    uint32_t a;
    uint16_t b;
    uint8_t c;
    uint32_t d;
} __attribute__((packed, aligned(1)));

NOOPT void test_subreg(void) {
    volatile struct packed_data pd;
    pd.a = 0x11223344;
    pd.b = 0x5566;
    pd.c = 0x77;
    pd.d = 0x8899AABB;
    
    /* Type punning through union to force SUBREG */
    union {
        uint32_t full;
        struct {
            uint16_t low;
            uint16_t high;
        } parts;
    } pun;
    
    pun.full = 0x12345678;
    pun.parts.low = 0x9ABC;  /* This may generate SUBREG */
    
    /* Vector operations that use SUBREG */
    typedef uint32_t v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    uint32_t element = vec[2];  /* Element extraction may use SUBREG */
    
    /* Cast through different types */
    uint64_t big = 0x1122334455667788ULL;
    uint32_t *half = (uint32_t*)&big;
    half[1] = 0xAABBCCDD;  /* SUBREG through pointer arithmetic */
    
    global_counter += pd.a + pun.full + element + half[0];
}

/* ===== MEM_P with Complex Addressing Pattern ===== */

/* Complex structure for memory addressing */
struct complex_struct {
    int data[256];
    struct complex_struct *next;
    int more_data[128];
};

NOOPT void test_mem_complex_address(void) {
    static volatile struct complex_struct cs[10];
    volatile int * volatile ptr;
    int i, sum = 0;
    
    /* Initialize linked list */
    for (i = 0; i < 9; i++) {
        cs[i].next = &cs[i + 1];
    }
    cs[9].next = &cs[0];
    
    /* Complex addressing modes */
    ptr = &cs[0].data[0];
    
    /* Multiple complex memory accesses */
    for (i = 0; i < 100; i++) {
        /* Complex address calculation */
        cs[(i * 3) % 10].data[(i * 7) % 256] = i;
        cs[i % 10].more_data[(i * 11) % 128] = i * 2;
        
        /* Pointer chain with offset */
        struct complex_struct *current = &cs[0];
        for (int j = 0; j < (i % 5); j++) {
            current = current->next;
        }
        current->data[i % 256] = i * 3;
    }
    
    /* Multi-dimensional array with complex indexing */
    volatile int md_array[10][20][30];
    for (i = 0; i < 100; i++) {
        md_array[i % 10][(i * 2) % 20][(i * 3) % 30] = i;
    }
    
    /* Inline assembly with memory clobber */
    asm volatile (
        "leal (%0, %1, 4), %%eax\n\t"
        "movl (%%eax), %%eax\n\t"
        : 
        : "r" (ptr), "r" (global_counter)
        : "eax", "memory"
    );
    
    /* Compute sum to prevent elimination */
    for (i = 0; i < 10; i++) {
        sum += cs[i].data[i * 10] + cs[i].more_data[i * 5];
    }
    global_counter += sum;
}

/* ===== Combined Test Function ===== */

/* Function that combines all patterns */
NOOPT void test_combined(void) {
    /* ZERO_EXTRACT through bit-field */
    struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 24;
    } bf;
    bf.a = 3;
    bf.b = 7;
    bf.c = 0x123456;
    
    /* STRICT_LOW_PART through byte assignment */
    volatile uint32_t x = 0x87654321;
    volatile uint8_t *xp = (volatile uint8_t*)&x;
    xp[0] = 0xAA;
    
    /* SUBREG through union type punning */
    union {
        uint64_t qword;
        uint32_t dwords[2];
    } u;
    u.qword = 0x1122334455667788ULL;
    u.dwords[1] = 0xAABBCCDD;
    
    /* MEM_P with complex address */
    volatile int array[100];
    for (int i = 0; i < 50; i++) {
        array[(i * 7 + 3) % 100] = array[(i * 11 + 5) % 100] + i;
    }
    
    global_counter += bf.a + bf.c + x + u.dwords[0] + array[42];
}

/* ===== Main Function ===== */

int main(void) {
    int i;
    
    /* Call all test functions multiple times */
    for (i = 0; i < 10; i++) {
        test_zero_extract();
        test_zero_extract_builtin();
        test_strict_low_part();
        test_strict_low_part_asm();
        test_subreg();
        test_mem_complex_address();
        test_combined();
    }
    
    /* Dummy computation to prevent dead code elimination */
    volatile int result = global_counter;
    
    /* Print to prevent optimization (optional) */
    printf("Result: %d\n", result);
    
    return 0;
}
