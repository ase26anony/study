/* test_resource_coverage.c
 * This program is designed to generate RTL patterns that trigger
 * specific uncovered lines in GCC's resource.cc (lines 282-290).
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fschedule-insns -fprofile-arcs -ftest-coverage -o test test_resource_coverage.c
 */

#include <stdint.h>
#include <stdio.h>

/* Ensure optimization is enabled for coverage */
#ifndef __OPTIMIZE__
#warning "Compile with optimization (-O2 or -O3) for best coverage"
#endif

/* Prevent inlining to ensure separate RTL generation */
#define NOINLINE __attribute__((noinline))

/* Global volatile variables to prevent optimization */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0x42;

/* Structure with bit-fields for ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int field1 : 4;
    unsigned int field2 : 8;
    unsigned int field3 : 12;
    unsigned int field4 : 8;
} volatile bitfield = {1, 2, 3, 4};

/* Array for memory access patterns */
int memory_array[256] = {0};

/* Function 1: Generate ZERO_EXTRACT patterns through bit-field operations */
NOINLINE static int generate_zero_extract(void) {
    /* Multiple bit-field operations */
    unsigned int result = 0;
    
    /* These operations often generate ZERO_EXTRACT in RTL */
    result |= (global_int >> 4) & 0xF;        /* Extract 4 bits */
    result |= (global_int >> 8) & 0xFF;       /* Extract 8 bits */
    result |= (global_int >> 16) & 0xFFFF;    /* Extract 16 bits */
    
    /* Bit-field structure access */
    result += bitfield.field1;
    result += bitfield.field2 << 4;
    result += bitfield.field3 << 12;
    
    /* More complex extraction */
    int x = global_int;
    result += (x & 0x00FF0000) >> 16;
    result += (x & 0x0000FF00) >> 8;
    result += (x & 0x000000FF);
    
    return result;
}

/* Function 2: Generate STRICT_LOW_PART patterns using inline assembly (x86) */
NOINLINE static int generate_strict_low_part(void) {
    int result = 0;
    
#ifdef __x86_64__ || __i386__
    /* Byte operations that may generate STRICT_LOW_PART */
    unsigned char byte_val;
    unsigned short word_val;
    
    /* Assembly with byte constraint */
    asm volatile (
        "movb %1, %0\n\t"
        : "=q"(byte_val)
        : "r"((unsigned char)global_char)
        : "cc"
    );
    
    /* Assembly with word constraint */
    asm volatile (
        "movw %1, %0\n\t"
        : "=r"(word_val)
        : "r"((unsigned short)global_short)
        : "cc"
    );
    
    result = byte_val + word_val;
    
    /* Mixed-size operations */
    int temp = global_int;
    short half;
    
    /* This may generate STRICT_LOW_PART when storing to partial register */
    asm volatile (
        "movw %w1, %0\n\t"
        : "=m"(half)
        : "r"(temp)
        : "memory"
    );
    
    result += half;
#else
    /* Fallback: Use type punning to generate partial register accesses */
    union {
        int full;
        struct {
            short low;
            short high;
        } parts;
    } converter;
    
    converter.full = global_int;
    result = converter.parts.low + converter.parts.high;
#endif
    
    return result;
}

/* Function 3: Generate SUBREG patterns through type conversions */
NOINLINE static int generate_subreg(void) {
    int result = 0;
    
    /* Various type conversions that generate SUBREG */
    long long big_val = 0x123456789ABCDEF0LL;
    
    /* Truncation to smaller types */
    int truncated_int = (int)big_val;          /* SUBREG from 64 to 32 bits */
    short truncated_short = (short)big_val;    /* SUBREG from 64 to 16 bits */
    char truncated_char = (char)big_val;       /* SUBREG from 64 to 8 bits */
    
    result += truncated_int;
    result += truncated_short;
    result += truncated_char;
    
    /* Sign extension operations */
    signed char sc = -42;
    int extended = sc;                         /* SUBREG with sign extension */
    result += extended;
    
    /* Access halves of larger types */
    union {
        long long ll;
        int halves[2];
    } splitter;
    
    splitter.ll = big_val;
    result += splitter.halves[0];              /* Access low 32 bits */
    result += splitter.halves[1];              /* Access high 32 bits */
    
    /* Pointer-based type punning */
    int *int_ptr = &global_int;
    short *short_ptr = (short *)int_ptr;
    result += short_ptr[0] + short_ptr[1];     /* SUBREG through pointer */
    
    return result;
}

/* Function 4: Generate complex MEM_P patterns with addressing modes */
NOINLINE static int generate_mem_patterns(void) {
    int result = 0;
    volatile int index1 = global_int & 0xF;
    volatile int index2 = (global_int >> 4) & 0xF;
    volatile int index3 = (global_int >> 8) & 0xF;
    
    /* Complex array indexing with variable offsets */
    for (int i = 0; i < 16; i++) {
        /* Multi-dimensional access with variable indices */
        result += memory_array[i * 16 + index1];
        result += memory_array[index2 * 16 + i];
        
        /* Pointer arithmetic with non-constant offsets */
        int *ptr = &memory_array[0];
        result += *(ptr + i + index3);
        result += ptr[i * 2 + index1];
    }
    
    /* Structure pointer chasing */
    struct nested {
        int a;
        int b;
        struct nested *next;
    } node1, node2;
    
    node1.a = 100;
    node1.b = 200;
    node1.next = &node2;
    node2.a = 300;
    node2.b = 400;
    node2.next = &node1;
    
    /* Complex memory access pattern */
    result += node1.next->next->a;
    result += node1.next->b;
    
    /* Memory access with scaled index */
    for (int i = 0; i < 8; i++) {
        result += memory_array[i * 4 + index2];  /* Scaled index addressing */
    }
    
    return result;
}

/* Function 5: Combined patterns in a loop to increase RTL generation */
NOINLINE static int combined_patterns_loop(void) {
    int total = 0;
    
    /* Loop with multiple pattern-generating operations */
    for (volatile int i = 0; i < 10; i++) {
        total += generate_zero_extract();
        total += generate_strict_low_part();
        total += generate_subreg();
        total += generate_mem_patterns();
        
        /* Mix operations to create complex RTL */
        int temp = global_int + i;
        total += (temp >> (i & 3)) & 0xF;        /* ZERO_EXTRACT with variable shift */
        
        /* Memory access with computed address */
        total += memory_array[(temp + i) & 0xFF];
    }
    
    return total;
}

/* Main function that drives all patterns */
int main(void) {
    int result = 0;
    
    /* Initialize memory array */
    for (int i = 0; i < 256; i++) {
        memory_array[i] = i * 3;
    }
    
    /* Execute all pattern generators */
    result += generate_zero_extract();
    result += generate_strict_low_part();
    result += generate_subreg();
    result += generate_mem_patterns();
    result += combined_patterns_loop();
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
