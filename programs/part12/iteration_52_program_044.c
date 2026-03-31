/* test_resource.c - Program to trigger specific RTL patterns in GCC's mark_referenced_resources */

#include <stdint.h>
#include <string.h>

/* Prevent optimizations from eliminating our patterns */
#define NOOPT __attribute__((noinline, noipa, used))

/* Volatile to force memory operations */
volatile int global_counter = 0;

/* ========== ZERO_EXTRACT Pattern ========== */
/* Bit-field operations often generate ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 4;
    volatile unsigned int padding : 16;
};

NOOPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    
    /* Multiple bit-field writes to force ZERO_EXTRACT */
    bf.field1 = 5;
    bf.field2 = 0xAB;
    bf.field3 = 3;
    
    /* Complex bit-field expression */
    bf.field2 = (bf.field1 << 2) | (bf.field3 & 0x3);
    
    /* Use __builtin_bitfield for explicit ZERO_EXTRACT */
    unsigned int val = 0x12345678;
    unsigned int result = __builtin_bitfield((val >> 8), 0, 8);
    (void)result;
    
    global_counter += bf.field1 + bf.field2;
}

/* ========== STRICT_LOW_PART Pattern ========== */
NOOPT void test_strict_low_part(void) {
    volatile short s_val;
    volatile char c_val;
    
    /* These often generate STRICT_LOW_PART on x86 */
    asm volatile (
        "movw %1, %%ax\n\t"
        "movb %%al, %0\n\t"
        : "=m" (c_val)
        : "r" ((short)0x1234)
        : "ax"
    );
    
    /* Another approach using inline assembly with low-part modifier */
    int x = 0xABCD;
    asm volatile (
        "mov{l %[input], %[output] | %[output], %[input]}\n\t"
        : [output] "=r" (s_val)
        : [input] "r" (x)
    );
    
    /* Partial register update through volatile */
    volatile int partial_reg;
    partial_reg = 0xFF;  /* Low byte assignment */
    
    global_counter += s_val + c_val + partial_reg;
}

/* ========== SUBREG Pattern ========== */
/* Union for type-punning to generate SUBREG */
union type_pun {
    uint32_t full;
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
    uint8_t bytes[4];
};

NOOPT void test_subreg(void) {
    union type_pun u;
    u.full = 0xDEADBEEF;
    
    /* Operations on sub-parts generate SUBREG */
    u.parts.low += 0x100;
    u.parts.high = u.parts.low << 1;
    
    /* Vector operations can also generate SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    int element = vec[2];  /* This may use SUBREG */
    
    /* Packed structure access */
    struct __attribute__((packed)) packed_struct {
        char a;
        int b;
        char c;
    } ps;
    
    ps.b = 0x12345678;
    int extracted = ps.b;  /* May involve SUBREG due to misalignment */
    
    global_counter += u.parts.low + element + extracted;
}

/* ========== MEM_P with Complex Addressing ========== */
#define ARRAY_SIZE 100

NOOPT void test_complex_mem(void) {
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
    
    /* Pointer chain with offset */
    sum += *(ptr_array[global_counter % 30] + (global_counter % 70));
    
    /* Structure with pointer arithmetic */
    struct nested {
        int data[10][10];
        struct nested *next;
    } ns[5];
    
    /* Initialize nested structure */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                ns[i].data[j][k] = i * 100 + j * 10 + k;
            }
        }
        ns[i].next = (i < 4) ? &ns[i + 1] : NULL;
    }
    
    /* Complex memory access through structure chain */
    if (ns[0].next && ns[0].next->next) {
        sum += ns[0].next->next->data[5][5];
    }
    
    /* Inline assembly with memory operand */
    int temp = 0;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (temp)
        : "m" (array[10][20])
        : "eax"
    );
    
    sum += temp;
    global_counter += sum;
}

/* ========== Combined Test Function ========== */
NOOPT void test_combined(void) {
    /* Test all patterns in one function */
    
    /* ZERO_EXTRACT via bit-field */
    struct {
        volatile unsigned int bits : 10;
    } bf = {0};
    bf.bits = 0x1FF;
    
    /* STRICT_LOW_PART via char assignment */
    volatile int reg = 0x12345678;
    volatile char low_byte = (char)reg;
    
    /* SUBREG via union access */
    union {
        uint64_t full;
        uint32_t half[2];
    } u;
    u.full = 0x1122334455667788ULL;
    u.half[0] = u.half[1] + 1;
    
    /* MEM_P with complex address */
    volatile int matrix[50][50];
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            matrix[i][j] = i * 50 + j;
        }
    }
    
    int idx1 = global_counter % 30;
    int idx2 = (global_counter * 7) % 30;
    int complex_access = matrix[idx1][idx2] + matrix[idx2][idx1];
    
    /* Use results to prevent elimination */
    global_counter += bf.bits + low_byte + u.half[0] + complex_access;
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
        
        /* Modify global to change behavior */
        global_counter = (global_counter * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    return 0;
}
