/* Test program to trigger uncovered lines in resource.cc */
#include <stdint.h>
#include <string.h>

/* Prevent optimization from eliminating our patterns */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile to force actual operations */
volatile int global_counter = 0;

/* ===== ZERO_EXTRACT Pattern ===== */
/* Using bit-fields to trigger ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int full : 32;
    unsigned int part1 : 8;
    unsigned int part2 : 8;
    unsigned int part3 : 8;
    unsigned int part4 : 8;
} __attribute__((packed));

NOOPT void test_zero_extract(void) {
    volatile struct bitfield_struct bf;
    
    /* Writing to bit-fields often generates ZERO_EXTRACT */
    bf.part1 = 0xAA;
    bf.part2 = 0xBB;
    bf.part3 = 0xCC;
    bf.part4 = 0xDD;
    
    /* Mix with computation to prevent dead code elimination */
    global_counter += bf.part1 + bf.part2;
}

/* ===== STRICT_LOW_PART Pattern ===== */
NOOPT void test_strict_low_part(void) {
    volatile uint16_t short_var;
    volatile uint32_t int_var;
    
    /* Force partial register updates */
    /* Using inline assembly with %L0 modifier for low part */
    asm volatile (
        "movw %1, %%ax\n\t"
        "movw %%ax, %0"
        : "=m" (short_var)
        : "r" ((uint16_t)0x1234)
        : "ax"
    );
    
    /* Another approach: char assignment to volatile */
    volatile char char_var;
    int_var = 0xDEADBEEF;
    char_var = (char)int_var;  /* May generate STRICT_LOW_PART */
    
    global_counter += short_var + char_var;
}

/* ===== SUBREG Pattern ===== */
NOOPT void test_subreg(void) {
    /* Use type punning through unions */
    union {
        uint32_t full;
        uint16_t halves[2];
    } u;
    
    u.full = 0x12345678;
    
    /* Operations on sub-parts often generate SUBREG */
    u.halves[0] += 1;
    u.halves[1] -= 1;
    
    /* Packed struct with different sized members */
    struct __attribute__((packed)) mixed {
        char a;
        int b;
        short c;
    } s;
    
    s.a = 10;
    s.b = 20;
    s.c = 30;
    
    /* Access through pointer with different type */
    uint16_t *ptr = (uint16_t*)&s.b;
    *ptr = 0xABCD;
    
    global_counter += u.full + s.b;
}

/* ===== MEM_P with Complex Addressing Pattern ===== */
NOOPT void test_complex_mem(void) {
    volatile int array[100][100];
    volatile int *ptr_array[100];
    
    /* Initialize */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            array[i][j] = i * 100 + j;
        }
        ptr_array[i] = &array[i][0];
    }
    
    /* Complex addressing: multi-dimensional with offset */
    int sum = 0;
    for (int i = 1; i < 99; i++) {
        for (int j = 1; j < 99; j++) {
            /* Complex address calculation */
            sum += array[i-1][j-1] + array[i+1][j+1] +
                   ptr_array[i][j*2] + *(ptr_array[i-1] + j + 5);
        }
    }
    
    /* Structure with pointer chain */
    struct node {
        int value;
        struct node *next;
    };
    
    volatile struct node nodes[10];
    for (int i = 0; i < 9; i++) {
        nodes[i].value = i * 10;
        nodes[i].next = &nodes[i+1];
    }
    nodes[9].value = 90;
    nodes[9].next = &nodes[0];
    
    /* Complex memory access through pointer chain */
    volatile struct node *current = &nodes[0];
    for (int i = 0; i < 20; i++) {
        sum += current->value;
        current = current->next;
    }
    
    global_counter += sum;
}

/* ===== Combined Test Function ===== */
/* This function combines multiple patterns in sequence */
NOOPT void test_combined(void) {
    /* ZERO_EXTRACT via bit-field in struct */
    struct {
        volatile unsigned int field1 : 4;
        volatile unsigned int field2 : 12;
        volatile unsigned int field3 : 16;
    } bits;
    
    bits.field1 = 0xF;
    bits.field2 = 0xFFF;
    bits.field3 = 0xFFFF;
    
    /* STRICT_LOW_PART via byte store */
    volatile uint32_t dword = 0x12345678;
    volatile uint8_t *byte_ptr = (volatile uint8_t*)&dword;
    byte_ptr[1] = 0xAA;  /* Modify middle byte */
    
    /* SUBREG via type punning */
    union {
        uint64_t quad;
        uint32_t dwords[2];
    } u64;
    u64.quad = 0x1122334455667788ULL;
    u64.dwords[0] = u64.dwords[0] ^ 0xFFFFFFFF;
    
    /* Complex MEM access */
    volatile int matrix[10][10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                matrix[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    int result = 0;
    for (int i = 1; i < 9; i++) {
        for (int j = 1; j < 9; j++) {
            for (int k = 1; k < 9; k++) {
                /* Very complex addressing */
                result += matrix[i-1][j][k] * matrix[i][j-1][k] / 
                         (matrix[i][j][k-1] + 1);
            }
        }
    }
    
    global_counter += bits.field1 + bits.field2 + bits.field3 + 
                     dword + u64.dwords[0] + result;
}

/* ===== Main Function ===== */
int main(void) {
    /* Call all test functions multiple times to ensure coverage */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
    }
    
    /* Return the global counter to prevent dead code elimination */
    return global_counter == 0 ? 0 : 1;
}
