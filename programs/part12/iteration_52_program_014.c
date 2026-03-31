/* test_resource.c - Program to trigger specific RTL patterns in GCC's mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent optimizations from eliminating our patterns */
#define NOOPT __attribute__((noinline, noipa, used))

/* Volatile to force actual memory operations */
volatile int global_counter = 0;

/* Structure with bit-fields for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int full : 32;
    volatile unsigned int low_bits : 8;
    volatile unsigned int middle_bits : 16;
    volatile unsigned int high_bit : 1;
    volatile unsigned int padding : 7;
};

/* Packed structure for SUBREG generation */
struct __attribute__((packed)) packed_data {
    char a;
    short b;
    int c;
    char d;
};

/* Complex structure for MEM_P with addressing */
struct complex_struct {
    int data[256];
    struct complex_struct *next;
};

/* Union for type-punning */
union type_pun {
    uint32_t full;
    uint16_t halves[2];
    uint8_t bytes[4];
};

/* 1. ZERO_EXTRACT pattern via bit-field operations */
NOOPT void test_zero_extract(void) {
    struct bitfield_struct bf = {0};
    
    /* These assignments should generate ZERO_EXTRACT in RTL */
    bf.low_bits = 0xAB;          /* 8-bit ZERO_EXTRACT from 32-bit */
    bf.middle_bits = 0xCDEF;     /* 16-bit ZERO_EXTRACT */
    bf.high_bit = 1;             /* 1-bit ZERO_EXTRACT */
    
    /* Force compiler to generate SET with ZERO_EXTRACT as destination */
    volatile unsigned int temp = bf.full;
    bf.full = temp | 0x1000;     /* Additional bit-field manipulation */
    
    global_counter += bf.low_bits + bf.middle_bits;
}

/* 2. STRICT_LOW_PART pattern via inline assembly and partial register updates */
NOOPT void test_strict_low_part(void) {
    volatile short s_val = 0x1234;
    volatile char c_val = 0x56;
    volatile int result;
    
    /* Inline assembly that should generate STRICT_LOW_PART on x86 */
    __asm__ volatile (
        "movw %1, %%ax\n\t"          /* Load 16-bit value */
        "movb %2, %%al\n\t"          /* Modify low 8 bits - may generate STRICT_LOW_PART */
        "movw %%ax, %0\n\t"
        : "=r" (result)
        : "r" (s_val), "r" (c_val)
        : "ax"
    );
    
    /* Another approach: volatile char assignment to force partial register update */
    volatile char *byte_ptr = (volatile char *)&result;
    byte_ptr[0] = 0x78;              /* May generate STRICT_LOW_PART for byte store */
    byte_ptr[1] = 0x9A;
    
    global_counter += result;
}

/* 3. SUBREG pattern via packed structures and type-punning */
NOOPT void test_subreg(void) {
    struct packed_data pd = {1, 2, 3, 4};
    union type_pun pun;
    
    /* Operations on packed structure members may generate SUBREG */
    pd.b = pd.a + 10;                /* short from char may use SUBREG */
    pd.c = pd.b * 20;                /* int from short may use SUBREG */
    
    /* Type-punning through union */
    pun.full = 0x12345678;
    pun.halves[0] = pun.bytes[1] + pun.bytes[2];  /* Likely SUBREG operations */
    
    /* Vector-style operations that generate SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    int element = vec[2];            /* May generate SUBREG for element extraction */
    
    global_counter += pd.c + pun.halves[0] + element;
}

/* 4. MEM_P with complex addressing pattern */
NOOPT void test_mem_complex_address(void) {
    struct complex_struct cs[10];
    struct complex_struct *ptr = &cs[0];
    int i, sum = 0;
    
    /* Initialize data */
    for (i = 0; i < 10; i++) {
        memset(cs[i].data, i, sizeof(cs[i].data));
        cs[i].next = (i < 9) ? &cs[i + 1] : NULL;
    }
    
    /* Complex addressing modes that should generate non-trivial MEM addresses */
    for (i = 0; i < 256; i++) {
        /* Multi-dimensional array with index arithmetic */
        sum += cs[global_counter % 10].data[(i * 3 + 7) % 256];
        
        /* Pointer chasing with offset */
        if (ptr) {
            sum += ptr->data[i % 128] * 2;
            ptr = ptr->next;
        }
    }
    
    /* Even more complex addressing */
    volatile int *volatile volatile_ptr = &cs[5].data[100];
    sum += *(volatile_ptr + global_counter % 50);        /* Complex address calculation */
    sum += cs[2].data[cs[1].data[0] % 256];              /* Nested array indexing */
    
    global_counter += sum;
}

/* 5. Combined test with all patterns */
NOOPT void test_combined(void) {
    /* Mix different patterns in one function */
    struct bitfield_struct bf = {0};
    union type_pun pun;
    volatile int arr[100];
    int i;
    
    /* ZERO_EXTRACT */
    bf.middle_bits = 0xABCD;
    
    /* SUBREG via type-punning */
    pun.full = 0xDEADBEEF;
    pun.halves[0] = pun.bytes[2] << 8;
    
    /* Complex MEM addressing */
    for (i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Access with complex address */
    volatile int complex_sum = 0;
    for (i = 0; i < 50; i++) {
        complex_sum += arr[(i * 7 + 3) % 100] * arr[(i * 11 + 5) % 100];
    }
    
    /* Partial register update (hinting at STRICT_LOW_PART) */
    volatile short *short_ptr = (volatile short *)&pun.full;
    *short_ptr = 0x1234;
    
    global_counter += bf.middle_bits + pun.halves[0] + complex_sum;
}

/* Main function that calls all tests */
int main(void) {
    /* Initialize global counter to prevent dead code elimination */
    global_counter = 1;
    
    /* Execute all pattern tests */
    test_zero_extract();
    test_strict_low_part();
    test_subreg();
    test_mem_complex_address();
    test_combined();
    
    /* Additional iterations to increase coverage probability */
    for (int i = 0; i < 10; i++) {
        test_combined();
        global_counter = (global_counter * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Return the counter to prevent optimization */
    return global_counter == 0;
}
