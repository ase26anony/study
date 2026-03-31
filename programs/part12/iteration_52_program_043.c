/* test_resource.c - Program to trigger specific RTL patterns in GCC's mark_referenced_resources */

#include <stdint.h>
#include <string.h>

/* Prevent optimizations from removing our test patterns */
#define NOOPT __attribute__((noinline, noipa, used))

/* Volatile to prevent dead code elimination */
static volatile int global_counter = 0;

/* ===== ZERO_EXTRACT Pattern ===== */
/* Using bit-fields to generate ZERO_EXTRACT RTL */
struct bitfield_struct {
    unsigned int full : 32;
    unsigned int low_bits : 8;
    unsigned int middle_bits : 16;
    unsigned int high_bit : 1;
    unsigned int reserved : 7;
} __attribute__((packed));

static volatile struct bitfield_struct bf;

NOOPT void test_zero_extract(void) {
    /* Writing to bit-fields often generates ZERO_EXTRACT */
    bf.low_bits = 0xAB;
    bf.middle_bits = 0xCDEF;
    bf.high_bit = 1;
    
    /* Another approach using __builtin_bitfield */
    unsigned int val = 0x12345678;
    /* Extract and modify bits 8-15 */
    unsigned int extracted = __builtin_bitfield_extract(val, 8, 8);
    extracted = (extracted + 1) & 0xFF;
    /* This might generate ZERO_EXTRACT when storing back */
    val = __builtin_bitfield_insert(val, extracted, 8, 8);
    
    global_counter += bf.low_bits + val;
}

/* ===== STRICT_LOW_PART Pattern ===== */
NOOPT void test_strict_low_part(void) {
    volatile char char_var;
    volatile short short_var;
    volatile int int_var = 0x12345678;
    
    /* Writing to low parts of variables can generate STRICT_LOW_PART */
    char_var = (char)int_var;  /* Low byte assignment */
    short_var = (short)int_var; /* Low word assignment */
    
    /* Inline assembly with %L0 modifier for x86 low part */
    int result;
    __asm__ volatile (
        "movl $0x12345678, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=m" (char_var)
        : 
        : "%eax"
    );
    
    global_counter += char_var + short_var;
}

/* ===== SUBREG Pattern ===== */
NOOPT void test_subreg(void) {
    /* Using unions for type-punning */
    union pun {
        uint32_t full;
        uint16_t halves[2];
        uint8_t bytes[4];
    } u;
    
    u.full = 0xDEADBEEF;
    /* Accessing sub-parts through union may generate SUBREG */
    u.halves[0] = 0xCAFE;  /* Modify low 16 bits */
    u.bytes[2] = 0x42;     /* Modify third byte */
    
    /* Vector types can also generate SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    /* Extract element - may use SUBREG */
    int element = vec[2];
    vec[1] = element * 2;
    
    global_counter += u.full + element;
}

/* ===== MEM_P with Complex Addressing Pattern ===== */
NOOPT void test_complex_mem(void) {
    /* Multi-dimensional array with complex indexing */
    volatile int array[10][10][10];
    
    /* Initialize to prevent optimization */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                array[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Complex addressing expression */
    int *ptr = (int *)array;
    volatile int index1 = 5;
    volatile int index2 = 3;
    volatile int index3 = 7;
    
    /* This should generate complex address calculation */
    int value = array[index1][index2][index3];
    array[index1+1][index2-1][index3*2 % 10] = value * 2;
    
    /* Pointer arithmetic with multiple offsets */
    struct nested {
        int a;
        int b[5];
        struct {
            int x;
            int y;
        } inner;
    } nested_array[20];
    
    /* Complex structure access */
    nested_array[10].b[2] = 42;
    nested_array[5].inner.x = nested_array[10].b[2] + 1;
    
    global_counter += value + nested_array[5].inner.x;
}

/* ===== Combined Test Function ===== */
NOOPT void test_combined(void) {
    /* Test all patterns in one function to potentially trigger
       multiple paths in a single mark_referenced_resources call */
    
    /* ZERO_EXTRACT via bit-field */
    struct {
        unsigned int field1 : 4;
        unsigned int field2 : 12;
        unsigned int field3 : 16;
    } bits = {0};
    
    bits.field2 = 0xFFF;
    
    /* STRICT_LOW_PART via byte store */
    volatile unsigned int word = 0x12345678;
    volatile unsigned char *byte_ptr = (volatile unsigned char *)&word;
    byte_ptr[0] = 0xAA;  /* Low byte store */
    
    /* SUBREG via type punning */
    union {
        uint64_t dword;
        uint32_t words[2];
    } u;
    u.dword = 0x1122334455667788ULL;
    u.words[0] = 0xDEADBEEF;  /* Modify low 32 bits */
    
    /* Complex MEM access */
    volatile int matrix[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix[i][j] = i * 5 + j;
        }
    }
    
    volatile int idx = 2;
    int sum = matrix[idx][idx] + matrix[idx+1][idx-1];
    matrix[idx][idx] = sum;
    
    global_counter += bits.field2 + word + u.words[0] + sum;
}

/* Main function that calls all test patterns */
int main(void) {
    /* Initialize global counter */
    global_counter = 0;
    
    /* Execute all test patterns */
    test_zero_extract();
    test_strict_low_part();
    test_subreg();
    test_complex_mem();
    test_combined();
    
    /* Use the results to prevent dead code elimination */
    if (global_counter > 0) {
        return 0;
    }
    return 1;
}
