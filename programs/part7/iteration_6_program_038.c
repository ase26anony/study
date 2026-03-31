/* Test program to cover GCC resource.cc lines 282-290 */
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
    volatile unsigned int field3 : 20;
};

/* Function 1: Generate ZERO_EXTRACT RTL through bit-field operations */
NOINLINE static unsigned int test_zero_extract(struct bitfield *bf) {
    /* Multiple bit-field operations to increase chances */
    unsigned int val1 = bf->field1;
    unsigned int val2 = bf->field2;
    unsigned int val3 = bf->field3;
    
    /* Combined operations that may produce ZERO_EXTRACT */
    return (val1 << 16) | (val2 << 8) | (val3 & 0xFFF);
}

/* Function 2: Generate STRICT_LOW_PART RTL using inline assembly (x86) */
NOINLINE static uint32_t test_strict_low_part(uint32_t x) {
    uint32_t result;
    
#ifdef __x86_64__ || __i386__
    /* Byte operation that may generate STRICT_LOW_PART */
    asm volatile (
        "movb %b1, %b0\n\t"
        : "=q"(result)      /* q = a, b, c, d registers (byte-addressable) */
        : "r"(x)
        : "cc"
    );
    
    /* Half-word operation */
    uint16_t hw;
    asm volatile (
        "movw %w1, %w0\n\t"
        : "=r"(hw)
        : "r"(x)
        : "cc"
    );
    result += hw;
#else
    /* Fallback: operations on partial types that might generate similar RTL */
    result = (uint8_t)x + (uint16_t)x;
#endif
    
    return result;
}

/* Function 3: Generate SUBREG RTL through type conversions */
NOINLINE static int test_subreg(int a, short b, char c) {
    /* Various type conversions that may produce SUBREG */
    short s1 = a;           /* int to short */
    char c1 = b;            /* short to char */
    int i1 = s1;            /* short to int */
    int i2 = c1;            /* char to int */
    
    /* Operations on different-sized parts */
    long long ll = (long long)a * b;
    int ll_low = (int)ll;   /* Extract low part */
    int ll_high = (int)(ll >> 32); /* Extract high part */
    
    return i1 + i2 + ll_low + ll_high + c;
}

/* Function 4: Generate complex MEM_P RTL with addressing modes */
NOINLINE static int test_mem_operands(int *base, int index1, int index2) {
    /* Complex addressing calculations */
    int *ptr1 = base + index1;
    int *ptr2 = ptr1 + index2 * 2;
    int *ptr3 = &ptr2[index1 & 3];
    
    /* Multiple memory accesses with variable offsets */
    int sum = base[index1] 
            + ptr1[index2]
            + ptr2[1]
            + ptr3[0]
            + *(base + (index1 * index2) / 2);
    
    /* Multi-dimensional array access */
    int arr2d[4][8];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            arr2d[i][j] = i * j;
        }
    }
    
    sum += arr2d[index1 & 3][index2 & 7];
    
    return sum;
}

/* Function 5: Mixed operations to increase RTL pattern variety */
NOINLINE static int test_mixed_operations(void) {
    volatile int var = 42;
    struct bitfield bf = {1, 2, 3};
    
    /* Combine all patterns */
    int a = test_zero_extract(&bf);
    int b = test_strict_low_part(var);
    int c = test_subreg(var, 5, 7);
    
    int array[16];
    for (int i = 0; i < 16; i++) {
        array[i] = i * 2;
    }
    
    int d = test_mem_operands(array, var & 7, (var >> 2) & 3);
    
    return a + b + c + d;
}

/* Main function with loop to ensure RTL generation and resource marking */
int main(void) {
    int result = 0;
    
    /* Loop to increase chance of scheduling/resource marking passes */
    for (int i = 0; i < 100; i++) {
        result += test_mixed_operations();
        
        /* Conditional to prevent loop optimization */
        if (result > 1000000) {
            result = 0;
        }
    }
    
    /* Use result to prevent dead code elimination */
    return result % 256;
}
