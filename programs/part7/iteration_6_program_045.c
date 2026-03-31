/* Test program to cover specific RTL patterns in GCC's resource.cc */
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Ensure optimization is enabled for RTL generation */
#ifndef __OPTIMIZE__
#error "Compile with optimization (-O2 or -O3) to generate target RTL patterns"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Structure for bit-field operations */
struct bitfield {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 12;
};

/* Union for type punning and SUBREG generation */
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
volatile unsigned int g_mask = 0xF0F0F0F0;

/* Function 1: Generate ZERO_EXTRACT through bit-field operations */
NOINLINE static unsigned int test_zero_extract(struct bitfield *bf)
{
    /* Multiple bit-field operations to increase chances */
    unsigned int val1 = (bf->field1 << 2) & 0x0F;
    unsigned int val2 = (bf->field2 >> 1) & 0x7F;
    unsigned int val3 = (bf->field3 << 4) & 0xFFF;
    
    /* Combined operation that might generate ZERO_EXTRACT */
    unsigned int combined = ((bf->field1 | bf->field2) >> 2) & 0x3F;
    
    return val1 + val2 + val3 + combined;
}

/* Function 2: Generate STRICT_LOW_PART via inline assembly (x86/x86_64) */
NOINLINE static uint32_t test_strict_low_part(uint32_t input)
{
    uint32_t output = 0;
    
#ifdef __x86_64__ || __i386__
    /* Byte operation that may generate STRICT_LOW_PART */
    asm volatile (
        "movb %b1, %b0\n\t"
        : "=q"(output)
        : "r"(input)
        : "cc"
    );
    
    /* Half-word operation */
    uint16_t half_output;
    asm volatile (
        "movw %w1, %w0\n\t"
        : "=r"(half_output)
        : "r"(input)
        : "cc"
    );
    output += half_output;
#else
    /* Fallback: operations that might still generate partial register access */
    output = (uint8_t)input;          /* Truncation to byte */
    output += (uint16_t)(input >> 8); /* Truncation to half-word */
#endif
    
    return output;
}

/* Function 3: Generate SUBREG through type conversions and unions */
NOINLINE static uint32_t test_subreg(uint32_t value)
{
    union type_pun pun;
    pun.full = value;
    
    /* Various type conversions that may generate SUBREG */
    uint16_t low_half = pun.halves.low;
    uint16_t high_half = pun.halves.high;
    
    uint8_t byte1 = pun.bytes[0];
    uint8_t byte2 = pun.bytes[1];
    
    /* Mix different-sized operations */
    uint32_t result = (uint32_t)low_half + 
                     ((uint32_t)high_half << 16) +
                     (byte1 * 256) +
                     (byte2 << 8);
    
    /* Additional conversions */
    short s1 = (short)low_half;
    int i1 = (int)s1;  /* May generate SUBREG */
    
    return result + i1;
}

/* Function 4: Generate complex MEM_P addressing modes */
NOINLINE static int test_mem_addressing(int *base, int size)
{
    int result = 0;
    
    /* Multi-dimensional array-like access with variable indices */
    int *arr = base;
    
    /* Complex addressing with multiple variables */
    result += arr[g_index1];                     /* Simple index */
    result += arr[g_index1 + g_index2];          /* Index sum */
    result += arr[g_index1 * 2 - g_index2];      /* Index expression */
    
    /* Pointer arithmetic with non-constant offsets */
    int *ptr1 = arr + g_index1;
    int *ptr2 = ptr1 + g_index2;
    result += *ptr1 + *ptr2;
    
    /* Structure-like access through pointer casting */
    struct {
        int a;
        int b;
        int c;
    } *struct_ptr = (void*)arr;
    
    result += struct_ptr->b + struct_ptr->c;
    
    return result;
}

/* Function 5: Mixed operations in a loop to engage scheduling passes */
NOINLINE static int test_mixed_loop(void)
{
    struct bitfield bf = {1, 2, 3};
    union type_pun pun = {0x12345678};
    int array[16];
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 16; i++) {
        array[i] = i * i + g_index1;
    }
    
    int sum = 0;
    
    /* Loop with mixed operations - compiler may schedule these */
    for (int i = 0; i < 100; i++) {
        /* Use all test functions to generate various RTL patterns */
        sum += test_zero_extract(&bf);
        sum += test_strict_low_part(pun.full + i);
        sum += test_subreg(pun.full ^ i);
        sum += test_mem_addressing(array, 16);
        
        /* Modify values to prevent loop elimination */
        bf.field1 = (bf.field1 + 1) & 0x0F;
        pun.full ^= 0x00FF00FF;
    }
    
    return sum;
}

/* Main function that drives the tests */
int main(void)
{
    /* Compile-time check for optimization */
    _Static_assert(__OPTIMIZE__, "Optimization must be enabled");
    
    printf("Starting RTL pattern generation test...\n");
    
    /* Run the mixed test which includes all patterns */
    int result = test_mixed_loop();
    
    /* Additional direct calls to ensure each pattern is generated */
    struct bitfield bf = {5, 10, 15};
    result += test_zero_extract(&bf);
    
    result += test_strict_low_part(0x89ABCDEF);
    
    result += test_subreg(0xDEADBEEF);
    
    int array[32];
    for (int i = 0; i < 32; i++) {
        array[i] = i * 3;
    }
    result += test_mem_addressing(array, 32);
    
    printf("Test completed with result: %d\n", result);
    
    /* Return non-zero to indicate success but prevent constant folding */
    return (result != 0) ? 0 : 1;
}
