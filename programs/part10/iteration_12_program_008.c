#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimization */
volatile int global_index = 0;
extern int opaque(int); /* Prevent constant propagation */

/* Bitfield operations for ZERO_EXTRACT */
struct __attribute__((packed)) bitfield_struct {
    unsigned int flag:3;
    unsigned int value:5;
    unsigned int data:8;
    unsigned int extra:16;
};

__attribute__((noinline, optimize("O0")))
int test_zero_extract(void) {
    volatile struct bitfield_struct bf;
    bf.flag = 5;
    bf.value = 20;
    bf.data = 100;
    bf.extra = 30000;
    
    /* Operations that may generate ZERO_EXTRACT */
    unsigned int combined = (bf.value << 3) | bf.flag;
    unsigned int extracted = (bf.extra >> 4) & 0xFFF;  /* Should generate ZERO_EXTRACT */
    
    /* Bitfield assignment that may use ZERO_EXTRACT */
    bf.data = (extracted & 0x3F) | (combined & 0xC0);
    
    /* Complex bitfield extraction */
    unsigned int mask = (1 << bf.flag) - 1;
    unsigned int masked = bf.extra & mask;
    
    return bf.flag + bf.value + bf.data + (extracted & 0xFF) + (masked & 0xFF);
}

/* Low-part register accesses for STRICT_LOW_PART */
__attribute__((noinline, optimize("O0")))
int test_strict_low_part(void) {
    volatile int int_var = 0x12345678;
    volatile short short_var = 0;
    volatile char char_var = 0;
    
    /* Byte-sized store that may generate STRICT_LOW_PART */
    *(volatile unsigned char*)&int_var = 0xFF;  /* Store to low byte */
    
    /* Union for type punning */
    union {
        int full;
        unsigned char bytes[4];
    } u;
    u.full = int_var;
    u.bytes[1] = 0xAA;  /* Modify second byte */
    
    /* Truncation that preserves high bits */
    char_var = int_var & 0xFF;  /* Only use low byte */
    
    /* Inline assembly for explicit low-part access */
    int result;
    asm volatile (
        "movb %b1, %0\n\t"  /* %b1 accesses low byte of register */
        : "=r" (result)
        : "r" (int_var)
        : "cc"
    );
    
    /* Another truncation pattern */
    short_var = int_var;  /* Implicit truncation */
    
    return u.full + char_var + short_var + result;
}

/* Subregister operations for SUBREG */
__attribute__((noinline, optimize("O0")))
int test_subreg(void) {
    /* Vector extensions for SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    
    /* Extract elements - may use SUBREG */
    int elem0 = vec[0];
    int elem2 = vec[2];
    
    /* Type punning between float and int */
    float f = 3.14159f;
    int i;
    memcpy(&i, &f, sizeof(int));  /* Bitcast using memcpy */
    
    /* Explicit casts between different sizes */
    long long big = 0x123456789ABCDEF0LL;
    int small = (int)big;  /* Truncation */
    short shorter = (short)small;
    
    /* Mixed operations */
    i = opaque(i);  /* Prevent optimization */
    vec[1] = i;
    
    return elem0 + elem2 + (i & 0xFF) + (small & 0xFFFF) + shorter;
}

/* Complex memory addressing for memory operand walking */
__attribute__((noinline, optimize("O0")))
int test_memory_operand(void) {
    volatile int buffer[100];
    volatile int *ptr1 = buffer;
    volatile int **ptr2 = &ptr1;
    volatile int ***ptr3 = &ptr2;
    
    /* Initialize */
    for (int i = 0; i < 100; i++) {
        buffer[i] = i * 2;
    }
    
    /* Complex addressing modes */
    int index = global_index % 100;
    
    /* Multi-level pointer dereferencing */
    int val1 = ***ptr3 + index;
    
    /* Array indexing with volatile index */
    int val2 = buffer[index] + buffer[index + 1];
    
    /* Structure with pointer chasing */
    struct node {
        int value;
        struct node *next;
    };
    
    struct node nodes[10];
    for (int i = 0; i < 9; i++) {
        nodes[i].value = i * 10;
        nodes[i].next = &nodes[i + 1];
    }
    nodes[9].value = 90;
    nodes[9].next = NULL;
    
    /* Walk linked list - creates complex memory accesses */
    struct node *current = &nodes[0];
    int sum = 0;
    while (current) {
        sum += current->value;
        current = current->next;
    }
    
    /* Volatile memory operation */
    volatile int *volatile_ptr = buffer;
    int val3 = *(volatile_ptr + index);
    
    return val1 + val2 + sum + val3;
}

/* Main function to execute all tests */
int main(void) {
    int total = 0;
    
    printf("Running resource pattern tests...\n");
    
    /* Execute each test function */
    total += test_zero_extract();
    total += test_strict_low_part();
    total += test_subreg();
    total += test_memory_operand();
    
    /* Use opaque function to prevent dead code elimination */
    total = opaque(total);
    
    printf("Result: %d\n", total);
    
    return total & 0xFF;  /* Return non-zero to indicate execution */
}

/* Dummy opaque function */
int opaque(int x) {
    /* Use inline assembly to prevent optimization */
    asm volatile ("" : "+r" (x));
    return x;
}
