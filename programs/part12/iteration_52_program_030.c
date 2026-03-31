/* test_resource.c - Program to trigger specific RTL patterns in GCC's mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent optimizations that might eliminate our patterns */
#define NOOPT __attribute__((noinline, noipa, used))

/* Volatile to force actual memory operations */
volatile int global_counter = 0;

/* ========== ZERO_EXTRACT Pattern ========== */
/* Using bit-fields to generate ZERO_EXTRACT RTL */
struct bitfield_struct {
    unsigned int full : 32;
    unsigned int low_bits : 8;
    unsigned int middle_bits : 16;
    unsigned int high_bit : 1;
    unsigned int padding : 7;
} NOOPT;

/* Force ZERO_EXTRACT through bit-field operations */
NOOPT void test_zero_extract(void) {
    volatile struct bitfield_struct bf;
    
    /* These assignments should generate ZERO_EXTRACT in RTL */
    bf.low_bits = 0xAB;
    bf.middle_bits = 0xCDEF;
    bf.high_bit = 1;
    
    /* Mix with arithmetic to prevent dead code elimination */
    global_counter += bf.low_bits + bf.middle_bits;
}

/* Alternative using __builtin_bitfield */
NOOPT void test_zero_extract_builtin(void) {
    volatile uint32_t value = 0x12345678;
    
    /* Extract and set bit-fields using builtins */
    uint32_t extracted = __builtin_bitfield_extract(value, 4, 8);
    uint32_t inserted = __builtin_bitfield_insert(value, 0x9A, 12, 8);
    
    global_counter += extracted + inserted;
}

/* ========== STRICT_LOW_PART Pattern ========== */
/* Force STRICT_LOW_PART through partial register updates */
NOOPT void test_strict_low_part(void) {
    volatile uint16_t short_var;
    volatile uint8_t byte_var;
    volatile uint32_t int_var = 0x87654321;
    
    /* These should generate STRICT_LOW_PART for partial register updates */
    short_var = (uint16_t)(int_var & 0xFFFF);  /* Low 16 bits */
    byte_var = (uint8_t)(int_var & 0xFF);      /* Low 8 bits */
    
    /* Use inline assembly for explicit low-part constraint */
    uint32_t result;
    __asm__ volatile (
        "movl $0x12345678, %0\n\t"
        "movw $0xABCD, %w0"    /* %w0 selects low 16 bits */
        : "=r" (result)
        :
        : "cc"
    );
    
    global_counter += short_var + byte_var + result;
}

/* ========== SUBREG Pattern ========== */
/* Force SUBREG through type punning and packed structures */
struct packed_data {
    uint32_t a;
    uint16_t b;
    uint8_t c;
} __attribute__((packed)) NOOPT;

union type_pun {
    uint32_t full;
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
} NOOPT;

NOOPT void test_subreg(void) {
    volatile struct packed_data pd;
    volatile union type_pun tp;
    
    /* Initialize */
    pd.a = 0xDEADBEEF;
    pd.b = 0xCAFE;
    pd.c = 0x42;
    
    tp.full = 0x12345678;
    
    /* These operations should generate SUBREG accesses */
    uint16_t extracted = tp.parts.low;  /* SUBREG from 32-bit to 16-bit */
    uint32_t combined = (tp.parts.high << 16) | extracted;
    
    /* Packed struct access forces SUBREG */
    uint32_t from_packed = pd.a;
    uint16_t short_from_packed = pd.b;
    
    global_counter += extracted + combined + from_packed + short_from_packed;
}

/* ========== MEM_P with Complex Addressing Pattern ========== */
/* Create complex memory addressing expressions */
#define ARRAY_SIZE 100

struct nested {
    int data[10];
    struct nested *next;
};

NOOPT void test_complex_mem(void) {
    volatile int array[ARRAY_SIZE][ARRAY_SIZE];
    volatile struct nested complex_struct[50];
    volatile int * volatile ptr_array[100];
    
    /* Initialize to prevent optimization */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < ARRAY_SIZE; j++) {
            array[i][j] = i * 100 + j;
        }
    }
    
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 10; j++) {
            complex_struct[i].data[j] = i * 10 + j;
        }
        complex_struct[i].next = &complex_struct[(i + 1) % 50];
    }
    
    for (int i = 0; i < 100; i++) {
        ptr_array[i] = (int *)&array[i % ARRAY_SIZE][0];
    }
    
    /* Complex addressing patterns that should generate non-trivial MEM addresses */
    int sum = 0;
    
    /* Multi-dimensional array with index arithmetic */
    sum += array[global_counter % ARRAY_SIZE][(global_counter * 3) % ARRAY_SIZE];
    
    /* Pointer chain with offset */
    sum += complex_struct[global_counter % 50].data[(global_counter * 7) % 10];
    
    /* Pointer array dereference with computation */
    sum += ptr_array[(global_counter * 11) % 100][(global_counter * 13) % ARRAY_SIZE];
    
    /* Very complex addressing expression */
    sum += *(int *)((char *)&complex_struct[0] + 
                   sizeof(struct nested) * (global_counter % 50) +
                   offsetof(struct nested, data) +
                   sizeof(int) * ((global_counter * 17) % 10));
    
    global_counter += sum;
}

/* ========== Combined Test Function ========== */
/* Function that combines all patterns to maximize coverage */
NOOPT void test_combined(void) {
    /* ZERO_EXTRACT pattern using bit-field */
    struct {
        unsigned int field1 : 4;
        unsigned int field2 : 12;
        unsigned int field3 : 16;
    } bits = {0};
    
    bits.field1 = 0xF;
    bits.field2 = 0xABC;
    bits.field3 = 0xDEAD;
    
    /* STRICT_LOW_PART pattern */
    volatile uint32_t val32 = 0x12345678;
    volatile uint16_t val16 = (uint16_t)val32;
    
    /* SUBREG pattern via union */
    union {
        uint64_t full;
        uint32_t halves[2];
    } u;
    u.full = 0x1122334455667788ULL;
    uint32_t subreg_access = u.halves[0];
    
    /* Complex MEM addressing */
    volatile int matrix[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    int mem_access = matrix[global_counter % 10][(global_counter * 3) % 10];
    
    /* Use all results to prevent elimination */
    global_counter += bits.field1 + bits.field2 + bits.field3 + 
                     val16 + subreg_access + mem_access;
}

/* ========== Main Function ========== */
int main(void) {
    /* Call all test functions multiple times with different conditions */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_zero_extract_builtin();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
        
        /* Modify global to change behavior in loops */
        global_counter = (global_counter * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Return the global counter to prevent dead code elimination */
    return global_counter == 0 ? 0 : 1;
}
