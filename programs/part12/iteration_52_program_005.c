/* test_resource.c - Program to trigger specific RTL patterns in GCC's resource.cc */

#include <stdint.h>
#include <string.h>

/* Prevent optimizations that might eliminate our patterns */
#define NOOPT __attribute__((noinline, noipa, used))

/* Volatile to prevent dead code elimination */
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
    
    /* Multiple bit-field writes to force ZERO_EXTRACT patterns */
    bf.field1 = 7;
    bf.field2 = 42;
    bf.field3 = 1023;
    
    /* Complex bit-field expression */
    bf.field2 = (bf.field1 << 2) | 1;
    
    /* Prevent optimization */
    global_counter += bf.field1 + bf.field2 + bf.field3;
}

/* ===== STRICT_LOW_PART Pattern ===== */
NOOPT void test_strict_low_part(void) {
    volatile uint32_t full_reg;
    volatile uint16_t low_part;
    volatile uint8_t byte_part;
    
    /* Force partial register updates */
    full_reg = 0x12345678;
    
    /* These should generate STRICT_LOW_PART for low byte/word */
    low_part = 0xABCD;
    byte_part = 0xEF;
    
    /* Inline assembly with low-part modifier (x86 specific) */
    asm volatile (
        "movw %1, %%ax\n\t"
        "movw %%ax, %0"
        : "=r" (low_part)
        : "r" (full_reg)
        : "ax"
    );
    
    global_counter += full_reg + low_part + byte_part;
}

/* ===== SUBREG Pattern ===== */
NOOPT void test_subreg(void) {
    /* Use unions for type-punning to force SUBREG */
    union {
        uint32_t full;
        uint16_t halves[2];
        uint8_t bytes[4];
    } converter;
    
    /* Packed structure to force sub-register accesses */
    struct __attribute__((packed)) packed_struct {
        char a;
        int b;
        short c;
    } ps;
    
    /* Operations that extract parts of larger values */
    converter.full = 0xDEADBEEF;
    uint16_t low_half = converter.halves[0];
    uint16_t high_half = converter.halves[1];
    
    /* Vector operations can generate SUBREG */
    typedef int v4si __attribute__ ((vector_size (16)));
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;
    
    /* Access individual elements (may generate SUBREG) */
    int first_element = vec3[0];
    
    /* Packed structure access */
    ps.a = 'X';
    ps.b = 12345;
    ps.c = 6789;
    
    global_counter += low_half + high_half + first_element + ps.b + ps.c;
}

/* ===== MEM_P with Complex Addressing Pattern ===== */
NOOPT void test_complex_mem(void) {
    /* Multi-dimensional array with complex indexing */
    volatile int matrix[10][10][10];
    
    /* Structure with nested arrays */
    struct nested {
        int data[5][5];
        struct nested *next;
    } nodes[10];
    
    /* Initialize to prevent optimization */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                matrix[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Complex memory addressing expressions */
    int sum = 0;
    
    /* Multiple complex addressing modes */
    sum += matrix[1][2][3];
    sum += matrix[global_counter % 10][(global_counter + 1) % 10][(global_counter + 2) % 10];
    
    /* Pointer arithmetic with multiple offsets */
    int *ptr = &matrix[0][0][0];
    sum += *(ptr + 5 + global_counter);
    sum += *(ptr + 10 * global_counter + 7);
    
    /* Chain of structure accesses */
    for (int i = 0; i < 9; i++) {
        nodes[i].next = &nodes[i + 1];
        for (int x = 0; x < 5; x++) {
            for (int y = 0; y < 5; y++) {
                nodes[i].data[x][y] = i * 25 + x * 5 + y;
            }
        }
    }
    
    /* Complex structure access */
    sum += nodes[0].next->next->data[2][3];
    
    /* Inline assembly with memory clobber */
    asm volatile ("" : : "r" (ptr) : "memory");
    
    global_counter += sum;
}

/* ===== Combined Test Function ===== */
NOOPT void test_combined(void) {
    /* Combine multiple patterns in one function */
    
    /* ZERO_EXTRACT via bit-field */
    struct {
        volatile unsigned int bits : 10;
    } bf;
    bf.bits = 511;
    
    /* STRICT_LOW_PART via byte store */
    volatile uint32_t val = 0x87654321;
    volatile uint8_t *byte_ptr = (volatile uint8_t *)&val;
    byte_ptr[0] = 0xFF;
    
    /* SUBREG via union type-punning */
    union {
        uint64_t big;
        uint32_t parts[2];
    } u;
    u.big = 0x1122334455667788ULL;
    uint32_t part = u.parts[0];
    
    /* Complex MEM_P via multi-dimensional array */
    volatile int arr3d[3][3][3];
    arr3d[1][1][1] = part + bf.bits + byte_ptr[0];
    
    global_counter += arr3d[1][1][1];
}

/* ===== Main Function ===== */
int main(void) {
    /* Call all test functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
        
        /* Vary inputs slightly each iteration */
        global_counter = (global_counter * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Dummy return value */
    return global_counter == 0 ? 0 : 1;
}
