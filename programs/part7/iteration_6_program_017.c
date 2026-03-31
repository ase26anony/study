/* Test program to cover lines 282-290 in GCC's resource.cc
 * Generates RTL patterns for ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM_P
 */

#include <stdint.h>
#include <assert.h>

/* Ensure optimization is enabled for RTL generation */
#ifndef __OPTIMIZE__
#error "Compile with -O2 or -O3 to generate target RTL patterns"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Bit-field structure for ZERO_EXTRACT */
struct bitfield {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 12;
};

/* Function 1: Generate ZERO_EXTRACT patterns through bit-field operations */
NOINLINE static unsigned int test_zero_extract(struct bitfield *bf) {
    /* Multiple bit-field operations that may generate ZERO_EXTRACT */
    unsigned int val = 0;
    val |= (bf->field1 << 0);   /* May generate ZERO_EXTRACT for bit-field read */
    val |= (bf->field2 << 4);   /* Another bit-field access */
    val |= (bf->field3 << 12);  /* And another */
    
    /* Explicit bit extraction that may also generate ZERO_EXTRACT */
    volatile unsigned int source = 0xABCDEF12;
    unsigned int extracted = (source >> 8) & 0xFFF;  /* 12-bit extraction */
    
    return val + extracted;
}

/* Function 2: Generate STRICT_LOW_PART patterns using inline assembly (x86) */
NOINLINE static uint32_t test_strict_low_part(uint32_t x) {
    uint32_t result = 0;
    
    /* Byte operations that may generate STRICT_LOW_PART */
#ifdef __x86_64__ || __i386__
    /* Inline assembly for byte operations - may generate STRICT_LOW_PART */
    asm volatile (
        "movb %b1, %b0\n\t"          /* Move byte - may use STRICT_LOW_PART */
        : "=r"(result)
        : "r"(x)
        : "cc"
    );
    
    /* Another byte operation */
    uint16_t halfword;
    asm volatile (
        "movw %w1, %w0\n\t"          /* Move word - may use STRICT_LOW_PART */
        : "=r"(halfword)
        : "r"(x)
        : "cc"
    );
    result += halfword;
#else
    /* Generic fallback: operations on sub-parts that might generate similar RTL */
    result = (x & 0xFF) | ((x & 0xFF00) >> 8);
#endif
    
    return result;
}

/* Function 3: Generate SUBREG patterns through type conversions */
NOINLINE static int test_subreg(int a, short b, char c) {
    /* Various type conversions that may generate SUBREG */
    int result = 0;
    
    /* short to int conversion */
    result += (int)b;           /* May generate SUBREG */
    
    /* char to int conversion */
    result += (int)c;           /* May generate SUBREG */
    
    /* Accessing halves of 64-bit value */
    uint64_t big_val = 0x123456789ABCDEF0ULL;
    uint32_t lower_half = (uint32_t)big_val;      /* May generate SUBREG */
    uint32_t upper_half = (uint32_t)(big_val >> 32); /* Another SUBREG */
    
    result += lower_half + upper_half;
    
    /* Pointer casting between different sizes */
    uint16_t *short_ptr = (uint16_t*)&a;
    result += *short_ptr;       /* May involve SUBREG in addressing */
    
    return result;
}

/* Function 4: Generate complex MEM_P patterns with addressing modes */
NOINLINE static int test_mem_operands(int *base, int index1, int index2) {
    int result = 0;
    int array[4][8][16];  /* Multi-dimensional for complex addressing */
    
    /* Complex array indexing with variable offsets */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            /* Variable indexing that may generate complex MEM addresses */
            result += array[i][j][index1 & 15];
            result += array[i][(index2 + j) & 7][i];
        }
    }
    
    /* Pointer arithmetic with non-constant offsets */
    result += *(base + index1);          /* Base + variable offset */
    result += *(base + (index1 * 2));    /* Base + scaled variable offset */
    result += base[index1 + index2];     /* Array access with sum index */
    
    /* Structure field access through pointer */
    struct complex {
        int a[10];
        int b[5];
        int c;
    } *str_ptr = (struct complex*)base;
    
    result += str_ptr->a[index1 % 10];
    result += str_ptr->b[index2 % 5];
    
    return result;
}

/* Function 5: Mixed operations to increase RTL pattern diversity */
NOINLINE static int test_mixed_operations(void) {
    volatile int var = 0x12345678;
    int result = 0;
    
    /* Combination that might generate multiple target patterns */
    result = (var >> 4) & 0x0F0F0F0F;  /* Potential ZERO_EXTRACT */
    
    /* Type punning through union for SUBREG */
    union {
        uint32_t full;
        uint16_t halves[2];
        uint8_t bytes[4];
    } converter;
    
    converter.full = var;
    result += converter.halves[0] + converter.bytes[1];  /* SUBREG patterns */
    
    return result;
}

/* Main function that exercises all patterns */
int main(void) {
    int total = 0;
    
    /* Initialize test data */
    struct bitfield bf = {1, 2, 3};
    int data_array[100];
    for (int i = 0; i < 100; i++) {
        data_array[i] = i * 3;
    }
    
    /* Loop to increase chance of RTL generation and resource marking */
    for (int iteration = 0; iteration < 10; iteration++) {
        /* Exercise ZERO_EXTRACT patterns */
        total += test_zero_extract(&bf);
        
        /* Exercise STRICT_LOW_PART patterns */
        total += test_strict_low_part(0x89ABCDEF);
        
        /* Exercise SUBREG patterns */
        total += test_subreg(1000, 200, 30);
        
        /* Exercise MEM_P patterns with complex addressing */
        total += test_mem_operands(data_array, iteration, iteration * 2);
        
        /* Exercise mixed patterns */
        total += test_mixed_operations();
        
        /* Modify data to prevent dead code elimination */
        bf.field1 = (bf.field1 + 1) & 0xF;
        bf.field2 = (bf.field2 * 2) & 0xFF;
        data_array[iteration] = total;
    }
    
    /* Compile-time check for optimization */
    _Static_assert(__OPTIMIZE__, "Optimization required for RTL pattern generation");
    
    /* Return non-zero result to ensure execution */
    return (total != 0) ? 0 : 1;
}
