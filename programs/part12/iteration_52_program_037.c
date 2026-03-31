/* test_resource.c - Program to trigger specific RTL patterns for coverage testing */

#include <stdint.h>
#include <string.h>

/* Prevent optimizations from eliminating our patterns */
#define NOOPT __attribute__((noinline, noipa, used))

/* Volatile to prevent dead code elimination */
static volatile int global_counter = 0;

/* ========== ZERO_EXTRACT Pattern ========== */
NOOPT void test_zero_extract(void) {
    /* Using bit-fields to trigger ZERO_EXTRACT */
    struct bitfield_struct {
        unsigned int full : 32;
        unsigned int part1 : 8;
        unsigned int part2 : 8;
        unsigned int part3 : 8;
        unsigned int part4 : 8;
    } volatile bf;
    
    /* Force write to bit-field - should generate ZERO_EXTRACT */
    bf.part2 = 0xAB;
    bf.part3 = 0xCD;
    
    /* Additional volatile operations to ensure RTL generation */
    global_counter += bf.part2 + bf.part3;
}

/* ========== STRICT_LOW_PART Pattern ========== */
NOOPT void test_strict_low_part(void) {
    volatile short low_part;
    volatile int full_reg;
    
    /* Using inline assembly to force STRICT_LOW_PART on x86 */
    #ifdef __x86_64__
    asm volatile (
        "movl $0x12345678, %%eax\n\t"
        "movw $0xABCD, %%ax\n\t"    /* This should generate STRICT_LOW_PART */
        "movl %%eax, %0\n\t"
        : "=r" (full_reg)
        : 
        : "%eax"
    );
    #else
    /* Portable version: volatile char assignment often generates low-part updates */
    volatile char *byte_ptr = (volatile char *)&full_reg;
    byte_ptr[0] = 0x12;  /* Low byte assignment */
    byte_ptr[1] = 0x34;  /* Next byte */
    #endif
    
    /* Use the result to prevent elimination */
    global_counter += full_reg & 0xFFFF;
}

/* ========== SUBREG Pattern ========== */
NOOPT void test_subreg(void) {
    /* Using packed structures and type punning */
    struct __attribute__((packed)) packed_data {
        int32_t a;
        int16_t b;
        int32_t c;
    } data;
    
    /* Initialize */
    data.a = 0x11223344;
    data.b = 0x5566;
    data.c = 0x778899AA;
    
    /* Operations that should generate SUBREG */
    volatile int16_t extracted = data.b;  /* This may use SUBREG to access part of struct */
    volatile int32_t combined = extracted + (data.c & 0xFFFF);
    
    /* Complex expression with type conversion */
    union {
        int64_t full;
        struct {
            int32_t low;
            int32_t high;
        } parts;
    } pun;
    
    pun.full = 0x1122334455667788LL;
    volatile int32_t low_part = pun.parts.low;  /* Likely SUBREG access */
    
    global_counter += extracted + combined + low_part;
}

/* ========== MEM_P with Complex Addressing ========== */
NOOPT void test_complex_mem(void) {
    /* Multi-dimensional array with complex indexing */
    volatile int array[10][10][10];
    
    /* Initialize to prevent elimination */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                array[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Complex addressing expression */
    volatile int *ptr = &array[0][0][0];
    volatile int index1 = global_counter % 10;
    volatile int index2 = (global_counter / 10) % 10;
    volatile int index3 = (global_counter / 100) % 10;
    
    /* This should generate complex address calculation */
    volatile int value = array[index1][index2][index3];
    volatile int value2 = *(ptr + index1 * 100 + index2 * 10 + index3);
    
    /* Structure with pointer chains */
    struct node {
        int value;
        struct node *next;
    } nodes[10];
    
    /* Initialize linked structure */
    for (int i = 0; i < 9; i++) {
        nodes[i].value = i * 10;
        nodes[i].next = &nodes[i + 1];
    }
    nodes[9].value = 90;
    nodes[9].next = NULL;
    
    /* Chain of pointer dereferences */
    volatile struct node *current = &nodes[0];
    volatile int sum = 0;
    while (current) {
        sum += current->value;  /* Complex memory access through pointer */
        current = current->next;
    }
    
    global_counter += value + value2 + sum;
}

/* ========== Combined Test Function ========== */
NOOPT void test_combined(void) {
    /* Test all patterns in one function */
    
    /* ZERO_EXTRACT via bit-field */
    struct {
        unsigned int field1 : 4;
        unsigned int field2 : 4;
        unsigned int field3 : 24;
    } volatile bits;
    
    bits.field2 = 0xF;
    
    /* STRICT_LOW_PART via byte assignment */
    volatile int reg = 0x12345678;
    volatile char *byte = (volatile char *)&reg;
    byte[0] = 0xAA;  /* Low byte */
    
    /* SUBREG via packed struct */
    struct __attribute__((packed)) {
        char a;
        short b;
        int c;
    } volatile packed = {1, 2, 3};
    
    volatile short subreg_val = packed.b;
    
    /* Complex MEM_P via array with variable indices */
    volatile int arr[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            arr[i][j] = i * 5 + j;
        }
    }
    
    volatile int idx1 = global_counter % 5;
    volatile int idx2 = (global_counter * 7) % 5;
    volatile int mem_val = arr[idx1][idx2];
    
    /* Use all values to prevent elimination */
    global_counter += bits.field2 + reg + subreg_val + mem_val;
}

/* ========== Main Function ========== */
int main(void) {
    /* Call all test functions multiple times with different conditions */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
        
        /* Modify global to change behavior in loops */
        global_counter++;
    }
    
    /* Dummy return */
    return global_counter > 0 ? 0 : 1;
}
