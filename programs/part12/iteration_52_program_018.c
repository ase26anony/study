/* test_resource.c - Program to trigger specific RTL patterns in GCC's mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent optimization from eliminating our patterns */
#define NO_OPT __attribute__((noinline, noipa))

/* Volatile to force actual memory operations */
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
    
    /* Complex bit-field assignment with computation */
    unsigned int temp = global_counter;
    bf.low_bits = (temp & 0xFF) ^ 0x55;
    
    /* Nested bit-field operations */
    struct bitfield_struct local_bf;
    local_bf.middle_bits = bf.low_bits | 0x1000;
    bf.high_bit = local_bf.middle_bits > 0x2000;
}

/* ========== STRICT_LOW_PART Pattern ========== */
/* Using inline assembly with % modifier for low part */
NO_OPT void test_strict_low_part(void) {
    int value = global_counter;
    short low_part;
    
    /* Inline assembly that should generate STRICT_LOW_PART */
    asm volatile (
        "movw %w1, %0\n\t"  /* %w1 for word (16-bit) register */
        : "=r" (low_part)
        : "r" (value)
        : "cc"
    );
    
    /* Another approach: volatile char assignment */
    volatile char *byte_ptr = (volatile char *)&value;
    *byte_ptr = 0x42;
    
    /* Using union for type punning */
    union {
        int full;
        struct {
            short low;
            short high;
        } parts;
    } u;
    u.full = value;
    u.parts.low = 0x1234;  /* Should generate partial register update */
}

/* ========== SUBREG Pattern ========== */
/* Operations that generate SUBREG RTL */
typedef int v2si __attribute__((vector_size(8)));

NO_OPT void test_subreg(void) {
    /* Vector operations often use SUBREG */
    v2si vec1 = {1, 2};
    v2si vec2 = {3, 4};
    v2si result = vec1 + vec2;
    
    /* Accessing vector elements */
    int first_element = ((int*)&result)[0];
    global_counter += first_element;
    
    /* Type punning through union */
    union {
        uint64_t full;
        uint32_t halves[2];
    } data;
    data.full = 0x123456789ABCDEF0ULL;
    
    /* Operation on sub-register */
    uint32_t low_half = data.halves[0];
    low_half = (low_half << 4) | (low_half >> 28);
    data.halves[0] = low_half;
    
    /* Packed structure access */
    struct __attribute__((packed)) packed_struct {
        char a;
        int b;
        char c;
    } ps;
    
    ps.b = 0xDEADBEEF;
    int extracted = ps.b;  /* May involve SUBREG due to misalignment */
    global_counter += extracted & 0xFF;
}

/* ========== MEM_P with Complex Addressing ========== */
#define ARRAY_SIZE 100

struct nested {
    int values[ARRAY_SIZE];
    struct nested *next;
};

volatile struct nested complex_array[10][10];

NO_OPT void test_complex_mem(void) {
    /* Complex addressing modes */
    int sum = 0;
    
    /* Multi-dimensional array with complex index */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Complex address calculation */
            sum += complex_array[(i * 3) % 10][(j * 7) % 10].values[
                (i + j * 2) % ARRAY_SIZE];
        }
    }
    
    /* Pointer chain with offsets */
    struct nested *ptr = (struct nested *)&complex_array[0][0];
    for (int i = 0; i < 5; i++) {
        ptr->values[i * 3] = global_counter + i;
        if (ptr->next) {
            ptr = ptr->next;
        }
    }
    
    /* Inline assembly with memory operand */
    int dummy;
    asm volatile (
        "movl %1, %0\n\t"
        "addl $1, %0\n\t"
        "movl %0, %1"
        : "=r" (dummy)
        : "m" (global_counter)
        : "cc", "memory"
    );
}

/* ========== Combined Test Function ========== */
/* Function that uses all patterns together */
NO_OPT void test_combined(void) {
    /* ZERO_EXTRACT via bit-field */
    struct {
        unsigned int field1 : 4;
        unsigned int field2 : 12;
        unsigned int field3 : 16;
    } bits;
    
    bits.field1 = global_counter & 0xF;
    bits.field2 = (global_counter >> 4) & 0xFFF;
    bits.field3 = (bits.field1 + bits.field2) & 0xFFFF;
    
    /* STRICT_LOW_PART via inline assembly */
    int val = 0x12345678;
    short low_val;
    asm volatile (
        "movw %w1, %0"
        : "=r" (low_val)
        : "r" (val)
    );
    
    /* SUBREG via vector */
    typedef float v4f __attribute__((vector_size(16)));
    v4f fvec = {1.0f, 2.0f, 3.0f, 4.0f};
    fvec = fvec * 2.0f;
    
    /* Complex MEM access */
    volatile int *mem_ptr = &global_counter;
    for (int i = 0; i < 10; i++) {
        *(mem_ptr + i * 2) = i;
    }
}

/* ========== Main Function ========== */
int main(void) {
    /* Initialize global data */
    memset((void*)complex_array, 0, sizeof(complex_array));
    
    /* Call all test functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        global_counter = i * 100;
        
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
        
        /* Prevent loop optimization */
        asm volatile ("" : : : "memory");
    }
    
    /* Final computation to use results */
    int final_sum = global_counter;
    final_sum += bf.low_bits;
    final_sum += bf.middle_bits;
    final_sum += bf.high_bit;
    
    return final_sum == 0 ? 0 : 1;
}
