/* test_resource.c - Program to trigger specific RTL patterns in GCC's resource.cc */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

/* Prevent optimizations that might eliminate our patterns */
#define NOOPT __attribute__((noinline, noipa, optimize("O0")))

/* Volatile to prevent dead code elimination */
static volatile int global_counter = 0;

/* ==================== ZERO_EXTRACT Pattern ==================== */

/* Using bit-fields to generate ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int full : 32;
    volatile unsigned int low_bits : 8;
    volatile unsigned int middle_bits : 16;
    volatile unsigned int high_bit : 1;
};

/* Using __builtin_bitfield for ZERO_EXTRACT */
NOOPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    
    /* These assignments should generate ZERO_EXTRACT */
    bf.low_bits = 0xAB;          /* Extract low 8 bits */
    bf.middle_bits = 0xCDEF;     /* Extract middle 16 bits */
    bf.high_bit = 1;             /* Extract single high bit */
    
    /* Force use of bitfield to prevent elimination */
    global_counter += bf.low_bits + bf.middle_bits + bf.high_bit;
    
    /* Alternative: Using bit operations that might generate ZERO_EXTRACT */
    volatile uint32_t value = 0x12345678;
    uint32_t masked = value & 0x0000FFFF;  /* Might generate ZERO_EXTRACT */
    global_counter += masked;
}

/* ==================== STRICT_LOW_PART Pattern ==================== */

NOOPT void test_strict_low_part(void) {
    volatile uint16_t short_var;
    volatile uint8_t char_var;
    volatile uint32_t int_var = 0x87654321;
    
    /* These assignments to smaller types may generate STRICT_LOW_PART */
    short_var = (uint16_t)int_var;      /* Low 16-bit part */
    char_var = (uint8_t)int_var;        /* Low 8-bit part */
    
    /* Use inline assembly with low-part modifier for x86 */
    #ifdef __x86_64__
    asm volatile (
        "movl %1, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=m" (char_var)
        : "r" (int_var)
        : "%eax"
    );
    #endif
    
    /* Force use of variables */
    global_counter += short_var + char_var;
}

/* ==================== SUBREG Pattern ==================== */

/* Packed structure to force subregister accesses */
struct __attribute__((packed)) packed_data {
    uint8_t a;
    uint16_t b;
    uint8_t c;
    uint32_t d;
};

/* Union for type punning - often generates SUBREG */
union type_pun {
    uint32_t full;
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

NOOPT void test_subreg(void) {
    /* Packed structure access */
    struct packed_data pd;
    pd.a = 0x11;
    pd.b = 0x2233;      /* May require SUBREG for 16-bit in packed struct */
    pd.c = 0x44;
    pd.d = 0x55667788;
    
    /* Type punning via union */
    union type_pun pun;
    pun.full = 0xAABBCCDD;
    uint16_t extracted = pun.parts.low;  /* Likely generates SUBREG */
    
    /* Vector operations (if supported) */
    #ifdef __GNUC__
    typedef uint32_t v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    uint32_t elem = vec[2];  /* May generate SUBREG for element extraction */
    #endif
    
    /* Bit manipulation that extracts sub-word */
    uint32_t word = 0x12345678;
    uint16_t half = (word >> 8) & 0xFFFF;  /* Could generate SUBREG */
    
    global_counter += pd.b + extracted + half;
    #ifdef __GNUC__
    global_counter += elem;
    #endif
}

/* ==================== MEM_P with Complex Addressing ==================== */

/* Complex structure for nested access */
struct nested {
    int data[8];
    struct nested *next;
};

struct container {
    struct nested array[4][4];
    int offsets[16];
};

NOOPT void test_complex_mem(void) {
    static struct container cont;
    volatile int result = 0;
    
    /* Initialize data */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 8; k++) {
                cont.array[i][j].data[k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Complex addressing patterns that should generate non-trivial MEM addresses */
    
    /* 1. Multi-dimensional array with index calculation */
    result += cont.array[1][2].data[3];  /* Base + offsets */
    
    /* 2. Pointer arithmetic with multiple terms */
    int *ptr = &cont.array[0][0].data[0];
    result += *(ptr + 5 + global_counter % 3);  /* Complex address calculation */
    
    /* 3. Structure pointer chain */
    struct nested *current = &cont.array[2][1];
    result += current->data[4];  /* MEM with structure field offset */
    
    /* 4. Array with variable index in multiple dimensions */
    int idx1 = global_counter % 4;
    int idx2 = (global_counter / 4) % 4;
    int idx3 = (global_counter / 16) % 8;
    result += cont.array[idx1][idx2].data[idx3];  /* Very complex addressing */
    
    /* 5. Inline assembly with memory operand */
    #ifdef __x86_64__
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0\n\t"
        : "+m" (result)
        : "m" (cont.array[3][3].data[7])
        : "%eax"
    );
    #endif
    
    global_counter += result;
}

/* ==================== Combined Test Function ==================== */

/* Function that combines all patterns in sequence */
NOOPT void test_combined(void) {
    /* Force all patterns to be generated in one function */
    volatile uint64_t big_value = 0xFEDCBA9876543210ULL;
    
    /* ZERO_EXTRACT from 64-bit value */
    struct {
        volatile uint64_t full;
        volatile uint32_t low_part : 32;
        volatile uint32_t high_part : 32;
    } split;
    
    split.full = big_value;
    split.low_part = 0x12345678;  /* ZERO_EXTRACT pattern */
    
    /* STRICT_LOW_PART */
    volatile uint16_t low16 = (uint16_t)big_value;
    
    /* SUBREG via type punning */
    union {
        uint32_t dword;
        uint16_t words[2];
    } converter;
    converter.dword = 0xAABBCCDD;
    uint16_t extracted_word = converter.words[1];  /* SUBREG pattern */
    
    /* Complex MEM access */
    static int matrix[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            matrix[i][j] = i * 8 + j;
        }
    }
    
    /* Very complex addressing expression */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += matrix[i][(i + global_counter) % 8];  /* Complex MEM address */
    }
    
    /* Use all results to prevent elimination */
    global_counter += split.low_part + low16 + extracted_word + sum;
}

/* ==================== Main Function ==================== */

int main(void) {
    /* Call all test functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
        
        /* Modify global to change behavior in loops */
        global_counter = (global_counter * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Print something to prevent complete optimization */
    printf("Final counter: %d\n", global_counter);
    
    return 0;
}
