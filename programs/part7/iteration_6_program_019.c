/* Test program to cover lines 282-290 in resource.cc (mark_referenced_resources) */
/* Compile with: gcc -O2 -fno-omit-frame-pointer -fschedule-insns -fprofile-arcs -ftest-coverage test.c -o test */

#include <stdint.h>
#include <assert.h>

/* Ensure optimization is enabled for RTL generation */
#ifndef __OPTIMIZE__
#error "Compile with optimization enabled (-O1, -O2, or -O3)"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Bit-field structure for ZERO_EXTRACT */
struct bitfield {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 12;
};

/* Function 1: Generate ZERO_EXTRACT through bit-field operations */
NOINLINE static unsigned int test_zero_extract(struct bitfield *bf) {
    /* Multiple bit-field accesses to increase chances */
    unsigned int val1 = bf->field1;
    unsigned int val2 = bf->field2;
    unsigned int val3 = bf->field3;
    
    /* Bit-field extraction with shifting */
    unsigned int combined = (val1 << 16) | (val2 << 8) | val3;
    
    /* Explicit bit extraction that may generate ZERO_EXTRACT */
    unsigned int extracted = (combined >> 4) & 0xFFF;  /* 12-bit extraction */
    
    /* Another extraction pattern */
    extracted |= (combined >> 8) & 0xF;  /* 4-bit extraction */
    
    return extracted;
}

/* Function 2: Generate STRICT_LOW_PART through inline assembly (x86/x86_64) */
NOINLINE static uint32_t test_strict_low_part(uint32_t input) {
    uint32_t output = 0;
    
#if defined(__i386__) || defined(__x86_64__)
    /* Byte operation that may generate STRICT_LOW_PART */
    asm volatile (
        "movb %b1, %b0\n\t"          /* Move byte part */
        : "=q"(output)               /* =q constraint for byte-addressable register */
        : "r"(input)
        : "cc"
    );
    
    /* Half-word operation */
    uint16_t halfword;
    asm volatile (
        "movw %w1, %w0\n\t"          /* Move word part */
        : "=r"(halfword)
        : "r"(input)
        : "cc"
    );
    output |= halfword;
#else
    /* Fallback: type punning that may also generate partial register accesses */
    union {
        uint32_t full;
        struct {
            uint16_t low;
            uint16_t high;
        } parts;
    } converter;
    
    converter.full = input;
    output = converter.parts.low;  /* Access low 16 bits */
#endif
    
    return output;
}

/* Function 3: Generate SUBREG through type conversions and partial accesses */
NOINLINE static int test_subreg(int a, short b, char c) {
    /* Type conversions that may generate SUBREG */
    short s1 = a;           /* int to short */
    char c1 = a;            /* int to char */
    int i1 = b;             /* short to int */
    int i2 = c;             /* char to int */
    
    /* Structure with mixed types */
    struct mixed {
        int x;
        short y;
        char z;
    } m;
    
    m.x = a;
    m.y = b;
    m.z = c;
    
    /* Access different-sized components */
    short ys = m.y;         /* May involve SUBREG */
    char zc = m.z;          /* May involve SUBREG */
    
    /* Pointer casting between types */
    int *int_ptr = &a;
    short *short_ptr = (short *)int_ptr;  /* Type punning */
    short hs = *short_ptr;                /* Load through different type */
    
    return s1 + c1 + i1 + i2 + ys + zc + hs;
}

/* Function 4: Generate complex MEM_P operands with addressing modes */
NOINLINE static int test_mem_operands(int *base, int index1, int index2) {
    /* Multi-dimensional array access */
    int matrix[10][10];
    
    /* Complex addressing with variable indices */
    int val1 = matrix[index1][index2];
    int val2 = matrix[index2][index1];
    
    /* Pointer arithmetic with non-constant offsets */
    int *ptr1 = base + index1;
    int *ptr2 = base + index2 * 2;
    
    /* Structure pointer chasing */
    struct node {
        int value;
        struct node *next;
    } nodes[5];
    
    /* Chain of memory accesses */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        nodes[i].value = base[i];
        nodes[i].next = &nodes[i + 1];
        sum += nodes[i].value;
    }
    
    /* More complex addressing: base + index * scale + displacement */
    sum += ptr1[index2];          /* base[index1 + index2] */
    sum += *(ptr2 - index1);      /* *(base + index2*2 - index1) */
    
    return sum + val1 + val2;
}

/* Function 5: Combined operations in a loop to engage scheduling passes */
NOINLINE static int test_combined_loop(int iterations) {
    struct bitfield bf = {1, 2, 3};
    int array[100];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        array[i] = i;
    }
    
    /* Loop with mixed operations to generate various RTL patterns */
    for (int i = 0; i < iterations; i++) {
        /* Alternate between different operations */
        if (i & 1) {
            sum += test_zero_extract(&bf);
        } else {
            sum += test_strict_low_part(i);
        }
        
        /* SUBREG operations */
        sum += test_subreg(i, i & 0xFFFF, i & 0xFF);
        
        /* Memory operations with complex addressing */
        if (i < 90) {
            sum += test_mem_operands(array, i % 10, (i + 5) % 10);
        }
        
        /* Prevent loop elimination */
        bf.field1 = (bf.field1 + 1) & 0xF;
        bf.field2 = (bf.field2 + 1) & 0xFF;
    }
    
    return sum;
}

/* Main function that drives all tests */
int main(void) {
    /* Compile-time check for optimization */
    _Static_assert(__OPTIMIZE__, "Optimization must be enabled");
    
    int result = 0;
    
    /* Test each pattern individually */
    struct bitfield bf = {5, 10, 20};
    result += test_zero_extract(&bf);
    
    result += test_strict_low_part(0x12345678);
    
    result += test_subreg(1000, 500, 100);
    
    int base_array[20];
    for (int i = 0; i < 20; i++) {
        base_array[i] = i * 2;
    }
    result += test_mem_operands(base_array, 3, 7);
    
    /* Combined test with loop to engage scheduling/resource marking */
    result += test_combined_loop(50);
    
    /* Return non-zero result to indicate successful execution */
    return (result != 0) ? 0 : 1;
}
