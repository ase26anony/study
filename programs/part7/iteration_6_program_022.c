/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource.cc during compilation with optimization enabled.
 * It aims to cover lines 282-290 in mark_referenced_resources().
 */

#include <stdint.h>
#include <assert.h>

/* Ensure optimization is enabled for proper RTL generation */
#ifndef __OPTIMIZE__
#error "Compile with -O2 or -O3 for coverage"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Bit-field structure for ZERO_EXTRACT */
struct bitfield {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 12;
};

/* Function 1: Generate ZERO_EXTRACT RTL through bit-field operations */
NOINLINE static unsigned int test_zero_extract(void) {
    struct bitfield bf;
    unsigned int result = 0;
    
    /* Multiple bit-field accesses that may generate ZERO_EXTRACT */
    result = bf.field1;
    result |= (bf.field2 << 4);
    result |= (bf.field3 << 12);
    
    /* Additional bit-field extraction with masking */
    volatile unsigned int x = 0xABCDEF12;
    unsigned int y = (x >> 8) & 0xFFF;  /* Should generate ZERO_EXTRACT */
    
    return result + y;
}

/* Function 2: Generate STRICT_LOW_PART RTL using inline assembly (x86) */
NOINLINE static unsigned int test_strict_low_part(void) {
    unsigned int var = 0;
    unsigned int val = 0x12345678;
    
#ifdef __x86_64__ || __i386__
    /* Byte operation that may generate STRICT_LOW_PART */
    asm volatile ("movb %1, %0" 
                  : "=q"(var) 
                  : "r"(val) 
                  : "memory");
    
    /* Half-word operation */
    unsigned short svar;
    asm volatile ("movw %1, %0" 
                  : "=r"(svar) 
                  : "r"(val) 
                  : "memory");
    var += svar;
#else
    /* Fallback: Use volatile byte access that might generate similar pattern */
    volatile unsigned char *p = (volatile unsigned char *)&var;
    *p = (unsigned char)val;
#endif
    
    return var;
}

/* Function 3: Generate SUBREG RTL through type conversions and unions */
NOINLINE static unsigned int test_subreg(void) {
    /* Type conversions that generate SUBREG */
    long long ll = 0x123456789ABCDEF0LL;
    int a = (int)ll;           /* Truncation */
    short b = (short)a;        /* Further truncation */
    char c = (char)b;          /* More truncation */
    
    /* Union for type punning */
    union {
        uint64_t full;
        struct {
            uint32_t low;
            uint32_t high;
        } parts;
    } u;
    
    u.full = 0xDEADBEEFCAFEBABEULL;
    uint32_t low_part = u.parts.low;   /* May involve SUBREG */
    uint16_t half_part = (uint16_t)low_part;
    
    /* Mixed-size operations */
    int result = a + b + c + low_part + half_part;
    
    /* Frame pointer manipulation (encourages SUBREG usage) */
    volatile int stack_var = 42;
    result += stack_var;
    
    return result;
}

/* Function 4: Generate complex MEM_P RTL with addressing modes */
NOINLINE static unsigned int test_mem_operands(void) {
    /* Multi-dimensional array with variable indices */
    volatile int arr[10][20][30];
    static int indices[3] = {5, 10, 15};
    
    /* Complex addressing calculation */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            /* Non-constant offset addressing */
            sum += arr[indices[i]][j * 3][indices[j]];
        }
    }
    
    /* Pointer arithmetic with variable offsets */
    volatile int *ptr = (volatile int *)arr;
    int offset = indices[0] * 20 * 30 + indices[1] * 30 + indices[2];
    sum += *(ptr + offset);
    
    /* Structure field access through pointer */
    struct complex {
        int a[5];
        int b[10];
        int c;
    } s;
    
    volatile struct complex *sptr = &s;
    sum += sptr->a[2] + sptr->b[indices[0]];
    
    return sum;
}

/* Function 5: Combined operations to increase RTL pattern mixing */
NOINLINE static unsigned int test_combined(void) {
    volatile struct {
        unsigned int bits : 10;
        unsigned int value;
    } data;
    
    unsigned int result = 0;
    
    /* Mix bit-field (ZERO_EXTRACT) with memory access */
    result = data.bits;
    result += data.value;
    
    /* Type conversion (SUBREG) */
    short s_result = (short)result;
    
    /* Memory access with calculation */
    volatile int array[100];
    result += array[s_result * 2];
    
    return result;
}

/* Main function that calls all test patterns in a loop */
int main(void) {
    unsigned int total = 0;
    
    /* Loop to increase chance of RTL processing during optimization */
    for (int i = 0; i < 10; i++) {
        total += test_zero_extract();
        total += test_strict_low_part();
        total += test_subreg();
        total += test_mem_operands();
        total += test_combined();
    }
    
    /* Compile-time check for optimization */
    _Static_assert(__OPTIMIZE__, "Optimization must be enabled");
    
    /* Return non-zero to indicate success */
    return total != 0 ? 0 : 1;
}
