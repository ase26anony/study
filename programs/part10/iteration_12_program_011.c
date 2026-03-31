#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimization */
volatile int global_counter = 0;
extern int opaque(int);  /* Prevent constant propagation */

/* Force specific RTL patterns */

/* 1. ZERO_EXTRACT patterns */
__attribute__((noinline, optimize("O0")))
int test_zero_extract(void) {
    /* Bitfield structure */
    struct bitfields {
        unsigned int flag:3;
        unsigned int value:5;
        unsigned int mode:4;
        unsigned int data:20;
    } bf;
    
    volatile struct bitfields *pbf = &bf;
    int result = 0;
    
    /* Operations that may generate ZERO_EXTRACT */
    bf.flag = 5;
    bf.value = 20;
    bf.mode = 9;
    bf.data = 0x12345;
    
    /* Explicit bit extraction that may compile to ZERO_EXTRACT */
    unsigned int combined = *(unsigned int*)&bf;
    result = (combined >> 3) & 0x1F;  /* Extract 'value' field */
    result += (combined >> 8) & 0xF;   /* Extract 'mode' field */
    
    /* More complex extraction */
    int shift = global_counter & 7;
    int mask = (1 << 5) - 1;
    result += (combined >> shift) & mask;
    
    /* Bitfield assignment from extracted value */
    pbf->value = (combined >> 10) & 0x1F;
    
    return result + opaque(bf.flag);
}

/* 2. STRICT_LOW_PART patterns */
__attribute__((noinline, optimize("O0")))
int test_strict_low_part(void) {
    volatile int int_var = 0x12345678;
    volatile short short_var = 0;
    volatile char char_var = 0;
    int result = 0;
    
    /* Byte-sized store into integer - may generate STRICT_LOW_PART */
    *(volatile unsigned char*)&int_var = 0xFF;
    result += int_var;
    
    /* Union for type punning */
    union {
        uint32_t full;
        uint8_t bytes[4];
    } u;
    u.full = 0xDEADBEEF;
    
    /* Modify low byte only */
    u.bytes[0] = 0xAB;
    result += u.full;
    
    /* Truncation that preserves high bits in source */
    int temp = opaque(0x98765432);
    char_var = temp & 0xFF;  /* Only low byte */
    result += char_var;
    
    /* Multiple byte operations */
    for (int i = 0; i < 4; i++) {
        u.bytes[i] = opaque(i) & 0xFF;
    }
    result += u.full;
    
    return result;
}

/* 3. SUBREG patterns */
__attribute__((noinline, optimize("O0")))
int test_subreg(void) {
    /* Vector extensions for SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec = {1, 2, 3, 4};
    v8hi short_vec = {10, 20, 30, 40, 50, 60, 70, 80};
    int result = 0;
    
    /* Vector element extraction - may use SUBREG */
    result += vec[0];
    result += vec[global_counter & 3];
    
    /* Type punning between different sizes */
    int int_val = 0x12345678;
    short short_val = *(short*)&int_val;  /* Low half */
    result += short_val;
    
    /* Float/int bitcasting */
    float f = 3.14159f;
    int int_from_float = *(int*)&f;
    result += int_from_float & 0xFFFF;
    
    /* Mixed vector operations */
    short_vec[3] = int_val & 0xFFFF;
    result += short_vec[3];
    
    /* Complex subregister access */
    struct mixed {
        int a;
        short b;
        char c;
    } m;
    m.a = 0x11223344;
    m.b = 0x5566;
    m.c = 0x77;
    
    /* Access parts of the structure */
    result += *(short*)&m.a;  /* Low 16 bits of m.a */
    result += m.b;
    
    return result;
}

/* 4. Memory operand patterns */
__attribute__((noinline, optimize("O0")))
int test_memory_operand(void) {
    volatile int buffer[100];
    volatile int *ptr1, *ptr2, *ptr3;
    int result = 0;
    
    /* Initialize */
    for (int i = 0; i < 100; i++) {
        buffer[i] = opaque(i);
    }
    
    /* Complex pointer chasing */
    ptr1 = &buffer[10];
    ptr2 = &buffer[20];
    ptr3 = &buffer[30];
    
    /* Multi-level dereferencing */
    volatile int **pptr = (volatile int**)&ptr1;
    volatile int ***ppptr = (volatile int***)&pptr;
    
    result += ***ppptr;
    
    /* Array indexing with volatile index */
    volatile int idx = global_counter % 50;
    result += buffer[idx];
    result += buffer[idx + 10];
    result += buffer[idx * 2];
    
    /* Structure with nested arrays */
    struct nested {
        int data[10];
        struct nested *next;
    } n1, n2;
    
    n1.next = &n2;
    n2.next = &n1;
    
    for (int i = 0; i < 10; i++) {
        n1.data[i] = opaque(i * 2);
        n2.data[i] = opaque(i * 3);
    }
    
    /* Complex memory addressing */
    result += n1.next->data[5];
    result += n1.data[n2.data[3] % 10];
    
    /* Pointer arithmetic with multiple bases */
    volatile int *base1 = &buffer[0];
    volatile int *base2 = &buffer[50];
    
    for (int i = 0; i < 10; i++) {
        result += base1[i * 3];
        result += base2[i * 2];
    }
    
    return result;
}

/* 5. Inline assembly for specific RTL */
__attribute__((noinline, optimize("O0")))
int test_inline_asm(void) {
    int result = 0;
    int in_val = 0x12345678;
    int out_val = 0;
    
    /* Assembly that may generate STRICT_LOW_PART */
    asm volatile (
        "movb %b1, %0\n\t"
        : "=r"(out_val)
        : "r"(in_val)
        : "cc"
    );
    result += out_val;
    
    /* Memory operand with complex addressing */
    volatile int mem_buffer[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int idx = global_counter % 8;
    
    asm volatile (
        "addl $5, %1\n\t"
        "movl (%1), %0\n\t"
        : "=r"(out_val)
        : "r"(&mem_buffer[idx])
        : "memory"
    );
    result += out_val;
    
    return result;
}

/* Opaque function to prevent optimization */
int opaque(int x) {
    return x ^ 0x55AA55AA;
}

int main(void) {
    int total = 0;
    
    printf("Starting resource pattern tests...\n");
    
    /* Run all tests */
    total += test_zero_extract();
    total += test_strict_low_part();
    total += test_subreg();
    total += test_memory_operand();
    total += test_inline_asm();
    
    /* Modify global to ensure side effects */
    global_counter = total;
    
    printf("Result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return total != 0 ? 0 : 1;
}
