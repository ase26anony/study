/* test_resource.c - Test program to trigger specific RTL patterns in GCC's resource.cc */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent optimizations that might eliminate our patterns */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile to prevent dead code elimination */
volatile int global_counter = 0;

/* ==================== ZERO_EXTRACT Pattern ==================== */

/* Using bit-fields to generate ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int full_field;
    unsigned int bitfield : 4;  /* 4-bit field that should generate ZERO_EXTRACT */
    unsigned int another : 8;   /* 8-bit field */
} __attribute__((packed));

/* Using __builtin_bitfield for explicit ZERO_EXTRACT generation */
NOOPT void test_zero_extract(void) {
    volatile struct bitfield_struct bf;
    bf.full_field = 0x12345678;
    
    /* Writing to bit-field should generate ZERO_EXTRACT */
    bf.bitfield = 0xA;  /* This should produce SET_DEST with ZERO_EXTRACT */
    
    /* Another approach using bit operations */
    unsigned int value = 0xDEADBEEF;
    unsigned int mask = 0xF;  /* 4-bit mask */
    unsigned int position = 8; /* Start at bit 8 */
    
    /* This might generate ZERO_EXTRACT when optimized */
    value = (value & ~(mask << position)) | ((0xB & mask) << position);
    
    /* Use the results to prevent elimination */
    global_counter += bf.bitfield + (value & 0xFF);
}

/* Alternative approach with union and bit-field */
union bitfield_union {
    struct {
        unsigned int low : 16;
        unsigned int high : 16;
    } bits;
    unsigned int full;
};

NOOPT void test_zero_extract_union(void) {
    volatile union bitfield_union u;
    u.full = 0x12345678;
    
    /* These assignments should generate ZERO_EXTRACT */
    u.bits.low = 0xABCD;
    u.bits.high = 0xEF01;
    
    global_counter += u.full;
}

/* ==================== STRICT_LOW_PART Pattern ==================== */

NOOPT void test_strict_low_part(void) {
    volatile unsigned short low_part;
    volatile unsigned int full_reg;
    
    /* Force partial register update - might generate STRICT_LOW_PART */
    full_reg = 0x12345678;
    
    /* Using inline assembly with %b0 for low byte on x86 */
    asm volatile (
        "movb %1, %b0"
        : "=r" (full_reg)
        : "r" ((unsigned char)0xAA)
        : "cc"
    );
    
    /* Another approach: char assignment to volatile */
    volatile char char_var;
    volatile int int_var = 0x12345678;
    
    /* This might generate STRICT_LOW_PART for the low byte */
    *(volatile char*)&int_var = 0x55;
    
    /* Using short assignment */
    *(volatile short*)&int_var = 0x1234;
    
    global_counter += full_reg + int_var;
}

/* x86-specific inline assembly for STRICT_LOW_PART */
#ifdef __x86_64__
NOOPT void test_strict_low_part_asm(void) {
    unsigned int result;
    
    /* Using %L0 modifier for low part */
    asm volatile (
        "movl $0x12345678, %0\n\t"
        "movb $0xAA, %b0"
        : "=r" (result)
        :
        : "cc"
    );
    
    /* Another example with word operation */
    unsigned int val2;
    asm volatile (
        "movl $0xDEADBEEF, %0\n\t"
        "movw $0x1234, %w0"
        : "=r" (val2)
        :
        : "cc"
    );
    
    global_counter += result + val2;
}
#endif

/* ==================== SUBREG Pattern ==================== */

NOOPT void test_subreg(void) {
    /* Using packed structures to force SUBREG accesses */
    struct __attribute__((packed)) packed_data {
        char a;
        int b;
        char c;
    } pdata;
    
    pdata.a = 'A';
    pdata.b = 0x12345678;  /* This might involve SUBREG when accessed */
    pdata.c = 'B';
    
    /* Type punning through union */
    union subreg_union {
        uint32_t full;
        uint16_t half[2];
        uint8_t bytes[4];
    } u;
    
    u.full = 0x12345678;
    /* Accessing halves should generate SUBREG */
    u.half[0] = 0xABCD;
    u.half[1] = 0xEF01;
    
    /* Vector types can also generate SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    int element = vec[2];  /* This might use SUBREG */
    
    /* Complex expression with type conversion */
    uint64_t large = 0x123456789ABCDEF0ULL;
    uint32_t truncated = (uint32_t)large;  /* Might generate SUBREG */
    
    global_counter += pdata.b + u.full + element + truncated;
}

/* ==================== MEM_P with Complex Addressing ==================== */

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
    int index1 = global_counter % 10;
    int index2 = (global_counter * 3) % 10;
    int index3 = (global_counter * 7) % 10;
    
    /* This should generate complex address expression */
    int value = array[index1][index2][index3];
    
    /* Even more complex addressing with pointer arithmetic */
    int value2 = *(ptr + index1 * 100 + index2 * 10 + index3);
    
    /* Structure with pointer chains */
    struct node {
        int value;
        struct node *next;
    } nodes[10];
    
    /* Initialize linked list */
    for (int i = 0; i < 9; i++) {
        nodes[i].value = i * 10;
        nodes[i].next = &nodes[i + 1];
    }
    nodes[9].value = 90;
    nodes[9].next = NULL;
    
    /* Complex memory access through pointer chain */
    int chain_value = nodes[index1].next->next->value;
    
    /* Inline assembly with memory clobber */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0"
        : "=m" (array[5][5][5])
        : "m" (array[3][3][3])
        : "eax", "memory"
    );
    
    global_counter += value + value2 + chain_value + array[5][5][5];
}

/* ==================== Combined Test Function ==================== */

NOOPT void test_combined(void) {
    /* Test all patterns in one function */
    
    /* ZERO_EXTRACT via bit-field */
    struct {
        unsigned int field : 8;
    } bf;
    bf.field = 0x55;
    
    /* STRICT_LOW_PART via char assignment */
    volatile int reg = 0x12345678;
    *(volatile char*)&reg = 0xAA;
    
    /* SUBREG via union access */
    union {
        uint32_t full;
        uint16_t half;
    } u;
    u.full = 0xDEADBEEF;
    u.half = 0x1234;
    
    /* Complex MEM access */
    volatile int arr[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    int idx = (global_counter * 3 + 7) % 10;
    int mem_val = arr[idx][idx];
    
    global_counter += bf.field + reg + u.full + mem_val;
}

/* ==================== Main Function ==================== */

int main(void) {
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Call all test functions multiple times with different parameters */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_zero_extract_union();
        test_strict_low_part();
        
        #ifdef __x86_64__
        test_strict_low_part_asm();
        #endif
        
        test_subreg();
        test_complex_mem();
        test_combined();
        
        /* Modify global to change behavior */
        global_counter += i;
    }
    
    printf("Final counter: %d\n", global_counter);
    
    /* Return non-zero if any tests might have failed (simplified) */
    return (global_counter > 0) ? 0 : 1;
}
