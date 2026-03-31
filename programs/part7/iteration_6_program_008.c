/*
 * Test program to cover lines 282-290 in GCC's resource.cc
 * These lines handle ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM_P operations
 */

#include <stdint.h>
#include <assert.h>

/* Ensure optimization is enabled for RTL generation */
#ifndef __OPTIMIZE__
#error "Compile with optimization (-O2 or -O3) to generate target RTL patterns"
#endif

/* Prevent inlining to ensure separate functions generate RTL */
#define NOINLINE __attribute__((noinline))

/* Structure with bit-fields for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 12;
    volatile unsigned int padding : 8;
};

/* Union for type punning and SUBREG operations */
union type_pun {
    uint32_t full;
    struct {
        uint16_t low;
        uint16_t high;
    } halves;
    uint8_t bytes[4];
};

/* Global variables to prevent constant propagation */
volatile int g_index1 = 3;
volatile int g_index2 = 7;
volatile int g_value = 0x12345678;

/* Function 1: Generate ZERO_EXTRACT patterns through bit-field operations */
NOINLINE static unsigned int test_zero_extract(struct bitfield_struct *bf)
{
    /* Multiple bit-field operations that may generate ZERO_EXTRACT */
    unsigned int result = 0;
    
    /* These operations should generate ZERO_EXTRACT in RTL */
    result |= (bf->field1 << 0);
    result |= (bf->field2 << 4);
    result |= (bf->field3 << 12);
    
    /* Additional bit-field extraction */
    bf->field1 = (result >> 0) & 0xF;
    bf->field2 = (result >> 4) & 0xFF;
    bf->field3 = (result >> 12) & 0xFFF;
    
    return result;
}

/* Function 2: Generate STRICT_LOW_PART patterns using inline assembly (x86) */
NOINLINE static uint32_t test_strict_low_part(uint32_t input)
{
    uint32_t output = 0;
    
#ifdef __x86_64__ || __i386__
    /* Byte operation that may generate STRICT_LOW_PART */
    asm volatile (
        "movb %b[in], %b[out]\n\t"
        : [out] "=q" (output)
        : [in] "r" (input)
        : "cc"
    );
    
    /* Half-word operation */
    uint16_t half_word;
    asm volatile (
        "movw %w[in], %w[out]\n\t"
        : [out] "=r" (half_word)
        : [in] "r" (input)
        : "cc"
    );
    output |= half_word;
#else
    /* Fallback for non-x86: use bit operations that might still generate interesting RTL */
    output = (input & 0xFF) | ((input & 0xFF00) >> 8);
#endif
    
    return output;
}

/* Function 3: Generate SUBREG patterns through type conversions */
NOINLINE static uint32_t test_subreg(union type_pun *tp)
{
    uint32_t result = 0;
    
    /* Type conversions that should generate SUBREG */
    uint16_t low_part = tp->full;          /* truncation: int -> short */
    uint16_t high_part = (tp->full >> 16); /* shift and truncate */
    
    /* Access different-sized parts */
    result |= tp->halves.low;
    result |= (tp->halves.high << 16);
    
    /* Byte access through pointer casting */
    uint8_t byte_val = *(uint8_t *)&tp->full;
    result += byte_val;
    
    /* Mix types to force conversions */
    tp->halves.low = (uint16_t)(result & 0xFFFF);
    tp->halves.high = (uint16_t)((result >> 16) & 0xFFFF);
    
    return result;
}

/* Function 4: Generate MEM_P patterns with complex addressing */
NOINLINE static int test_mem_operands(int *base, int index1, int index2)
{
    int result = 0;
    
    /* Multi-dimensional array-like access with variable indices */
    int array[10][10];
    
    /* Initialize to prevent optimization */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            array[i][j] = i * 10 + j;
        }
    }
    
    /* Complex memory addressing patterns */
    result += array[index1][index2];              /* 2D array access */
    result += *(base + index1 * 2 + index2);      /* Pointer arithmetic */
    result += array[index2][index1];              /* Swapped indices */
    
    /* Structure pointer access */
    struct {
        int a;
        int b;
        int c[5];
    } s;
    
    s.a = index1;
    s.b = index2;
    result += s.c[index1 % 5];
    
    return result;
}

/* Function 5: Combined operations to increase RTL complexity */
NOINLINE static int test_combined(void)
{
    int total = 0;
    struct bitfield_struct bf = {1, 2, 3, 0};
    union type_pun tp = {.full = 0x89ABCDEF};
    
    /* Loop to increase optimization opportunities */
    for (int i = 0; i < 10; i++) {
        total += test_zero_extract(&bf);
        total += test_strict_low_part(tp.full + i);
        total += test_subreg(&tp);
        
        /* Update values to prevent loop elimination */
        bf.field1 = (bf.field1 + 1) & 0xF;
        bf.field2 = (bf.field2 * 2) & 0xFF;
        tp.full += 0x11111111;
    }
    
    return total;
}

/* Main function that exercises all patterns */
int main(void)
{
    int result = 0;
    
    /* Test each pattern individually */
    struct bitfield_struct bf = {5, 10, 20, 0};
    union type_pun tp = {.full = 0xDEADBEEF};
    int base_array[100];
    
    /* Initialize base array */
    for (int i = 0; i < 100; i++) {
        base_array[i] = i * 2;
    }
    
    /* Exercise all test functions */
    result += test_zero_extract(&bf);
    result += test_strict_low_part(g_value);
    result += test_subreg(&tp);
    result += test_mem_operands(base_array, g_index1, g_index2);
    result += test_combined();
    
    /* Use result to prevent dead code elimination */
    volatile int sink = result;
    
    /* Simple validation */
    if (result != 0) {
        return 0;  /* Success */
    }
    
    return 1;  /* Should not reach here with normal execution */
}
