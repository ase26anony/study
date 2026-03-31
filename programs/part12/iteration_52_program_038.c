/* test_resource.c - Program to trigger specific RTL patterns in GCC's mark_referenced_resources */

#include <stdint.h>
#include <string.h>

/* Prevent optimizations from eliminating our patterns */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile to force actual operations */
volatile int global_counter = 0;

/* ========== ZERO_EXTRACT Pattern ========== */
/* Bit-field operations often generate ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 12;
    volatile unsigned int field4 : 8;
};

NOOPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    
    /* Multiple bit-field writes to force ZERO_EXTRACT */
    bf.field1 = 5;
    bf.field2 = 0xAB;
    bf.field3 = 0x7FF;
    bf.field4 = 0xCD;
    
    /* Complex bit-field expression */
    bf.field2 = (bf.field1 << 2) | (bf.field4 & 0x3);
    
    /* Use __builtin_bitfield to potentially generate ZERO_EXTRACT */
    unsigned int val = 0x12345678;
    unsigned int result = __builtin_bitfield((val >> 8) & 0xFFF, 4, 8);
    
    global_counter += bf.field1 + bf.field2 + bf.field3 + bf.field4 + result;
}

/* ========== STRICT_LOW_PART Pattern ========== */
NOOPT void test_strict_low_part(void) {
    volatile short s_val;
    volatile char c_val;
    volatile int i_val = 0x12345678;
    
    /* These assignments often generate STRICT_LOW_PART */
    s_val = i_val;          /* Low 16-bit part */
    c_val = i_val;          /* Low 8-bit part */
    
    /* Inline assembly with %L0 modifier for x86 low-part */
    int x = 42;
    asm volatile (
        "movl %1, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=m" (c_val)
        : "r" (x)
        : "%eax"
    );
    
    /* More low-part operations */
    volatile struct {
        char a;
        short b;
    } packed;
    
    packed.a = i_val & 0xFF;
    packed.b = i_val & 0xFFFF;
    
    global_counter += s_val + c_val + packed.a + packed.b;
}

/* ========== SUBREG Pattern ========== */
NOOPT void test_subreg(void) {
    /* Type punning through union often generates SUBREG */
    union pun {
        uint32_t full;
        uint16_t half[2];
        uint8_t bytes[4];
    } u;
    
    u.full = 0xDEADBEEF;
    
    /* Operations on sub-parts generate SUBREG */
    u.half[0] = u.half[1] + 1;
    u.bytes[2] = u.bytes[0] * 2;
    
    /* Packed structure operations */
    struct __attribute__((packed)) packed_struct {
        uint16_t a;
        uint32_t b;
    } ps;
    
    ps.a = 0x1234;
    ps.b = 0x56789ABC;
    
    /* Extract and manipulate sub-register values */
    uint32_t temp = ps.b;
    uint16_t low_half = temp & 0xFFFF;
    uint16_t high_half = (temp >> 16) & 0xFFFF;
    ps.a = low_half + high_half;
    
    /* Vector operations can generate SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    int element = vec[2];  /* This may use SUBREG */
    
    global_counter += u.full + ps.a + ps.b + element;
}

/* ========== MEM_P with Complex Addressing ========== */
NOOPT void test_complex_mem(void) {
    volatile int array[256][256];
    volatile int *ptr_array[100];
    volatile struct nested {
        int a;
        int b[10];
        struct nested *next;
    } nodes[50];
    
    /* Initialize to prevent dead code elimination */
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            array[i][j] = i * j;
        }
    }
    
    /* Complex addressing modes */
    int sum = 0;
    
    /* Multi-dimensional array with complex index */
    sum += array[global_counter % 256][(global_counter * 3) % 256];
    
    /* Pointer arithmetic with multiple offsets */
    volatile int *ptr = &array[0][0];
    sum += *(ptr + global_counter + 64);
    sum += *(ptr + (global_counter << 2) + 128);
    
    /* Structure pointer chain */
    for (int i = 0; i < 49; i++) {
        nodes[i].next = &nodes[i + 1];
        nodes[i].a = i * 10;
        for (int j = 0; j < 10; j++) {
            nodes[i].b[j] = i + j;
        }
    }
    
    /* Complex structure access */
    struct nested *current = &nodes[0];
    sum += current->next->next->b[current->a % 10];
    
    /* Inline assembly with memory clobber */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (array[100][100])
        : "m" (array[50][50])
        : "%eax", "memory"
    );
    
    global_counter += sum;
}

/* ========== Combined Test Function ========== */
NOOPT void test_combined(void) {
    /* Combined operations that might generate multiple patterns */
    volatile struct {
        unsigned int bitfield : 10;
        unsigned int full;
        unsigned short half;
        unsigned char byte;
    } combined;
    
    /* ZERO_EXTRACT pattern */
    combined.bitfield = (global_counter & 0x3FF);
    
    /* STRICT_LOW_PART pattern */
    combined.byte = combined.full;
    
    /* SUBREG pattern via type punning */
    union {
        uint32_t dword;
        uint16_t words[2];
    } u_comb;
    u_comb.dword = 0xA5A5A5A5;
    combined.half = u_comb.words[0] + u_comb.words[1];
    
    /* Complex MEM_P pattern */
    volatile int *ptr = &combined.full;
    *(ptr + (global_counter & 1)) = 0xDEADBEEF;
    
    global_counter += combined.bitfield + combined.byte + combined.half;
}

/* ========== Main Function ========== */
int main(void) {
    /* Call all test functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
        
        /* Modify global to affect future iterations */
        global_counter = (global_counter * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Dummy computation to prevent dead code elimination */
    volatile int result = global_counter;
    
    /* Return something to make compiler happy */
    return result == 0 ? 0 : 1;
}
