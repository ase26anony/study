/* test_resource.c - Program to trigger specific RTL patterns in GCC's mark_referenced_resources */

#include <stdint.h>
#include <string.h>

/* Prevent optimizations from removing our test patterns */
#define NOINLINE __attribute__((noinline, noipa))

/* Structure with bit-fields to trigger ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int full : 32;
    volatile unsigned int low : 8;
    volatile unsigned int middle : 16;
    volatile unsigned int high : 8;
};

/* Packed structure to trigger SUBREG */
struct __attribute__((packed)) packed_struct {
    char a;
    short b;
    int c;
    char d;
};

/* Complex structure for memory addressing */
struct complex_addr {
    int data[256];
    struct complex_addr *next;
};

/* Global volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile struct bitfield_struct bitfield_global;
volatile struct packed_struct packed_global;

/* Test 1: ZERO_EXTRACT pattern using bit-fields */
NOINLINE void test_zero_extract(void) {
    struct bitfield_struct local;
    
    /* Writing to bit-fields often generates ZERO_EXTRACT */
    local.low = 0xAB;      /* Should generate ZERO_EXTRACT for low 8 bits */
    local.middle = 0xCDEF; /* Should generate ZERO_EXTRACT for middle 16 bits */
    local.high = 0x12;     /* Should generate ZERO_EXTRACT for high 8 bits */
    
    /* Force usage to prevent dead code elimination */
    global_counter += local.low + local.middle + local.high;
    
    /* Also test with global */
    bitfield_global.low = local.low;
    bitfield_global.middle = local.middle;
}

/* Test 2: STRICT_LOW_PART pattern using inline assembly */
NOINLINE void test_strict_low_part(void) {
    volatile unsigned short low_part;
    volatile unsigned int full_reg;
    
    /* Method 1: Inline assembly with %L modifier (x86 specific) */
    asm volatile (
        "movl $0x12345678, %0\n\t"
        "movw $0x9ABC, %w1\n\t"
        : "=r" (full_reg), "=r" (low_part)
        :
        : "memory"
    );
    
    /* Method 2: Char assignment to volatile - may generate STRICT_LOW_PART */
    volatile char byte_var;
    volatile int int_var = 0xDEADBEEF;
    
    /* This assignment might generate STRICT_LOW_PART */
    byte_var = (char)0x42;
    
    /* Force compiler to consider partial register update */
    asm volatile (
        "movb %b0, %1\n\t"
        : "=r" (int_var)
        : "m" (byte_var)
        : "memory"
    );
    
    global_counter += low_part + byte_var;
}

/* Test 3: SUBREG pattern using packed structures and type punning */
NOINLINE void test_subreg(void) {
    struct packed_struct ps;
    
    /* Initialize */
    ps.a = 'A';
    ps.b = 12345;
    ps.c = 0xDEADBEEF;
    ps.d = 'Z';
    
    /* Accessing misaligned fields in packed struct often uses SUBREG */
    short b_copy = ps.b;  /* May involve SUBREG due to misalignment */
    int c_copy = ps.c;    /* May involve SUBREG due to misalignment */
    
    /* Type punning through union */
    union {
        uint32_t full;
        uint16_t halves[2];
    } pun;
    
    pun.full = 0x12345678;
    uint16_t low_half = pun.halves[0];  /* May use SUBREG */
    
    /* Vector operations (if supported) */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    int element = vec[2];  /* May use SUBREG for extraction */
    
    global_counter += b_copy + c_copy + low_half + element;
    packed_global = ps;
}

/* Test 4: MEM_P with complex addressing */
NOINLINE void test_complex_mem(void) {
    /* Create complex addressing patterns */
    struct complex_addr array[10];
    struct complex_addr *ptr = &array[0];
    
    /* Initialize */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 256; j++) {
            array[i].data[j] = i * 1000 + j;
        }
        if (i < 9) {
            array[i].next = &array[i + 1];
        } else {
            array[i].next = &array[0];
        }
    }
    
    /* Complex addressing patterns that should generate non-trivial MEM addresses */
    int sum = 0;
    
    /* Pattern 1: Multi-dimensional array with index calculation */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 256; j += 8) {
            /* Complex address: base + (i * 256 + j) * sizeof(int) */
            sum += array[i].data[j];
        }
    }
    
    /* Pattern 2: Pointer chasing with offset */
    ptr = &array[0];
    for (int i = 0; i < 5; i++) {
        /* Complex address: ptr->next->data[offset] */
        sum += ptr->next->data[i * 10];
        ptr = ptr->next;
    }
    
    /* Pattern 3: Array with variable index */
    volatile int idx = 128;
    sum += array[3].data[idx + global_counter % 64];  /* Very complex address */
    
    /* Pattern 4: Structure field with computed offset */
    int (*data_ptr)[256] = &array[5].data;
    sum += (*data_ptr)[100];
    
    global_counter += sum;
}

/* Test 5: Combined test with inline assembly for specific patterns */
NOINLINE void test_combined_asm(void) {
    volatile int x = 0;
    volatile short y = 0;
    volatile char z = 0;
    
    /* Try to trigger multiple patterns with inline assembly */
    asm volatile (
        /* Potential STRICT_LOW_PART for byte operation */
        "movb $0x55, %b0\n\t"
        
        /* Memory operation with complex addressing */
        "movl %1, (%2, %3, 4)\n\t"
        
        /* Register operation that might involve SUBREG */
        "movw %w4, %w0\n\t"
        : "+r" (x)
        : "r" (0x12345678), "r" (&global_counter), "r" (x), "r" (y)
        : "memory"
    );
    
    /* Bit-field operation that might generate ZERO_EXTRACT */
    struct {
        volatile unsigned int bits : 4;
    } bf;
    bf.bits = 0xF;
    
    global_counter += x + y + z + bf.bits;
}

/* Main function that runs all tests */
int main(void) {
    /* Run each test multiple times to ensure coverage */
    for (int i = 0; i < 3; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined_asm();
    }
    
    /* Dummy computation to prevent dead code elimination */
    volatile int result = global_counter;
    
    /* Return something based on the computations */
    return (result > 0) ? 0 : 1;
}
