/* test_resource.c - Program to trigger specific RTL patterns in GCC's mark_referenced_resources */

#include <stdint.h>
#include <string.h>

/* Prevent optimizations from eliminating our patterns */
#define NO_OPT __attribute__((noinline, noipa, used))

/* Volatile to force memory operations */
volatile int global_counter = 0;

/* ========== ZERO_EXTRACT Pattern ========== */
/* Using bit-fields to generate ZERO_EXTRACT RTL */
struct bitfield_struct {
    unsigned int full : 32;
    unsigned int low_bits : 8;
    unsigned int middle_bits : 16;
    unsigned int high_bit : 1;
} __attribute__((packed));

volatile struct bitfield_struct bf;

NO_OPT void test_zero_extract(void) {
    /* Writing to bit-fields often generates ZERO_EXTRACT */
    bf.low_bits = 0xAB;
    bf.middle_bits = 0xCDEF;
    bf.high_bit = 1;
    
    /* Using __builtin_bitfield for explicit ZERO_EXTRACT */
    unsigned int value = 0x12345678;
    unsigned int result = __builtin_bitfield((value >> 8) & 0xFF, 4, 8);
    
    /* Force usage to prevent elimination */
    global_counter += bf.low_bits + result;
}

/* ========== STRICT_LOW_PART Pattern ========== */
/* Using inline assembly with %L0 modifier on x86 */
NO_OPT void test_strict_low_part(void) {
    uint32_t reg_val;
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    asm volatile (
        "movl $0x12345678, %0\n\t"
        "movb $0xAA, %%al\n\t"
        "movb %%al, %b0\n\t"  /* %b0 accesses low byte */
        : "=r" (reg_val)
        :
        : "%eax"
    );
    
    /* Another approach: volatile char assignment to force partial register update */
    volatile uint32_t v = 0x87654321;
    volatile uint8_t *p = (volatile uint8_t*)&v;
    *p = 0x42;  /* This should generate STRICT_LOW_PART */
    
    global_counter += reg_val + v;
}

/* ========== SUBREG Pattern ========== */
/* Using unions and type-punning to generate SUBREG */
union type_pun {
    uint32_t full;
    uint16_t halves[2];
    uint8_t bytes[4];
};

NO_OPT void test_subreg(void) {
    union type_pun u;
    u.full = 0xDEADBEEF;
    
    /* Operations on sub-parts should generate SUBREG */
    u.halves[0] = u.halves[0] + 1;      /* SUBREG for 16-bit access */
    u.bytes[2] = u.bytes[1] ^ 0x55;     /* SUBREG for 8-bit access */
    
    /* Using vector types can also generate SUBREG */
    typedef uint32_t v2u32 __attribute__((vector_size(8)));
    v2u32 vec = {0x11111111, 0x22222222};
    uint32_t elem = vec[0] + vec[1];    /* May generate SUBREG for element access */
    
    global_counter += u.full + elem;
}

/* ========== MEM_P with Complex Addressing ========== */
/* Complex memory addressing expressions */
struct nested {
    int data[16];
    struct nested *next;
};

volatile struct nested complex_array[10][20];

NO_OPT void test_complex_mem(void) {
    int i, j, k;
    volatile int sum = 0;
    
    /* Complex addressing: multi-dimensional array with indices */
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 10; j++) {
            for (k = 0; k < 8; k++) {
                /* This should generate complex address expressions */
                sum += complex_array[i * 2][j * 2].data[k * 2];
            }
        }
    }
    
    /* Pointer chain with offsets */
    struct nested *ptr = &complex_array[0][0];
    for (i = 0; i < 5; i++) {
        if (ptr) {
            sum += ptr->data[0];
            ptr = ptr->next;  /* Force memory load */
        }
    }
    
    /* Inline assembly with memory clobber */
    asm volatile (
        "movl $0x1000, %%eax\n\t"
        "movl (%%eax, %%eax, 2), %%ebx\n\t"  /* Complex addressing: eax + eax*2 */
        "addl $0x10, %%ebx\n\t"
        "movl %%ebx, %0\n\t"
        : "=m" (sum)
        :
        : "%eax", "%ebx", "memory"
    );
    
    global_counter += sum;
}

/* ========== Combined Test Function ========== */
/* Function that combines all patterns */
NO_OPT void test_combined(void) {
    /* ZERO_EXTRACT via bit-field in struct */
    struct {
        volatile unsigned int field1 : 4;
        volatile unsigned int field2 : 12;
        volatile unsigned int field3 : 8;
    } bits;
    
    bits.field1 = 0xF;
    bits.field2 = 0xABC;
    bits.field3 = bits.field1 | bits.field2;
    
    /* STRICT_LOW_PART via volatile short */
    volatile uint32_t reg = 0x12345678;
    volatile uint16_t *short_ptr = (volatile uint16_t*)&reg;
    *short_ptr = 0x9ABC;  /* Partial register update */
    
    /* SUBREG via union access */
    union {
        uint64_t dword;
        uint32_t words[2];
    } u;
    u.dword = 0x1122334455667788ULL;
    u.words[0] = u.words[1] + 0x1000;
    
    /* Complex MEM_P via pointer arithmetic */
    volatile int array[100];
    volatile int *ptr = array;
    for (int i = 0; i < 10; i++) {
        ptr[i * 3 + 1] = ptr[i * 2] + i;  /* Complex addressing */
    }
    
    global_counter += bits.field3 + reg + u.words[0] + array[0];
}

/* ========== Main Function ========== */
int main(void) {
    /* Call all test functions multiple times to ensure coverage */
    for (int i = 0; i < 3; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
    }
    
    /* Dummy computation to prevent dead code elimination */
    volatile int result = global_counter;
    
    /* Return non-zero if any tests actually ran */
    return (result != 0) ? 0 : 1;
}
