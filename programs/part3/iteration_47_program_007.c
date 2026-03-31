#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0xEF;
volatile long global_long = 0x9ABCDEF0;

/* Global arrays for memory access patterns */
volatile int global_array[32];
volatile short short_array[64];
volatile char char_array[128];

/* Bitfield structs for ZERO_EXTRACT/STRICT_LOW_PART generation */
struct bitfield_struct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
};

struct mixed_bitfields {
    unsigned short low : 5;
    unsigned short mid : 6;
    unsigned short high : 5;
    unsigned char extra : 4;
};

/* Global bitfield instances */
volatile struct bitfield_struct bf1;
volatile struct mixed_bitfields bf2;

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* Force non-constant address computation */
    volatile int *ptr;
    
    /* Array access with variable offset - may generate MEM with complex address */
    int val1 = global_array[idx1 * 3 + idx2];
    
    /* Pointer arithmetic with type conversion */
    ptr = (volatile int *)((char *)global_array + idx1 * sizeof(int) * 2);
    int val2 = *ptr;
    
    /* Nested array access */
    int val3 = short_array[char_array[idx2] & 0x3F] + 
               char_array[idx1 * 2 + 7];
    
    return val1 + val2 + val3;
}

/* Function to generate SUBREG patterns through type punning */
static short type_punning_operations(int x, int y) {
    volatile int temp = x * y + 0x1234;
    
    /* Multiple type conversions to force SUBREG */
    char c1 = (temp >> 0) & 0xFF;
    char c2 = (temp >> 8) & 0xFF;
    char c3 = (temp >> 16) & 0xFF;
    char c4 = (temp >> 24) & 0xFF;
    
    /* Reassemble with different type */
    short s1 = (c2 << 8) | c1;
    short s2 = (c4 << 8) | c3;
    
    /* More type mixing */
    int combined = (s1 << 16) | s2;
    short result = (combined >> 8) & 0xFFFF;
    
    return result;
}

/* Function to generate ZERO_EXTRACT/STRICT_LOW_PART patterns */
static unsigned int bitfield_operations(int shift, int mask) {
    unsigned int result = 0;
    
    /* Explicit bit extraction - may generate ZERO_EXTRACT */
    result |= (global_int >> shift) & mask;
    result |= (global_int >> (shift + 4)) & (mask << 1);
    
    /* Bitfield struct access */
    bf1.a = (shift >> 0) & 0xF;
    bf1.b = (shift >> 4) & 0xFF;
    bf1.c = (shift >> 8) & 0xFFF;
    
    /* Take address of bitfield member (may generate complex RTL) */
    volatile unsigned int *p = (volatile unsigned int *)&bf1.a;
    result += *p;
    
    /* Mixed bitfield operations */
    bf2.low = result & 0x1F;
    bf2.mid = (result >> 5) & 0x3F;
    bf2.high = (result >> 11) & 0x1F;
    bf2.extra = (result >> 16) & 0x0F;
    
    /* Extract from bitfield */
    result = bf2.low | (bf2.mid << 5) | (bf2.high << 11) | (bf2.extra << 16);
    
    return result;
}

/* Complex combined operation hitting multiple patterns */
static int combined_operation(int idx, int shift) {
    int result = 0;
    
    /* Memory access with complex addressing */
    volatile int *mem_ptr = &global_array[idx % 32];
    
    /* Type punning on memory access */
    short s_val = *(volatile short *)((char *)mem_ptr + 2);
    
    /* Bit extraction from memory */
    int extracted = (s_val >> (shift % 8)) & 0x3F;
    
    /* More type conversion */
    char c_val = (extracted * 3) & 0xFF;
    int int_val = c_val;
    
    /* Combine with bitfield operation */
    result = bitfield_operations(shift, 0xFF) + int_val;
    
    /* Additional memory access pattern */
    result += complex_address(idx, shift % 8);
    
    return result;
}

int main(int argc, char **argv) {
    int i, j;
    unsigned int checksum = 0;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < 32; i++) {
        global_array[i] = i * 0x12345 + 0x6789;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = i * 0xABCD + 0x1234;
    }
    for (i = 0; i < 128; i++) {
        char_array[i] = (i * 0x37) & 0xFF;
    }
    
    /* Use command line arguments to get dynamic values */
    int base_shift = (argc > 1) ? atoi(argv[1]) % 16 : 3;
    int base_idx = (argc > 2) ? atoi(argv[2]) % 29 : 7;
    
    /* Main loop with opaque control flow */
    for (i = 0; i < 100; i++) {
        int shift = (base_shift + i) % 24;
        int idx = (base_idx + i * 3) % 31;
        
        /* Branch with external-dependent condition */
        if (argc > 3) {
            /* Pattern 1: Focus on bitfield operations */
            checksum += bitfield_operations(shift, 0x1F << (i % 5));
            
            /* Pattern 2: Type punning operations */
            short s_result = type_punning_operations(i, shift);
            checksum += s_result;
            
            /* Access bitfield through pointer */
            volatile unsigned char *bf_ptr = (volatile unsigned char *)&bf2;
            checksum += bf_ptr[(i + shift) % sizeof(bf2)];
        } else {
            /* Pattern 3: Combined operations with memory access */
            checksum += combined_operation(idx, shift);
            
            /* Additional memory pattern with complex addressing */
            volatile int *ptr = &global_array[(idx * 2 + shift) % 32];
            volatile short *sptr = (volatile short *)ptr;
            checksum += *sptr;
            
            /* More type conversion */
            int temp = *ptr;
            char c1 = (temp >> 8) & 0xFF;
            char c2 = (temp >> 16) & 0xFF;
            checksum += (c1 << 8) | c2;
        }
        
        /* Alternate between different operations based on loop counter */
        if (i % 7 == 0) {
            /* Force SUBREG through 64-bit operations on 32-bit system */
            long long_val = global_long + i;
            int truncated = (int)long_val;
            short s_trunc = (short)truncated;
            checksum += s_trunc * 2;
        }
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 13 == 0) {
            volatile int barrier = i;
            checksum += barrier;
        }
    }
    
    /* Final complex expression combining everything */
    int final = 0;
    final |= (checksum >> 0) & 0xFF;
    final |= (checksum >> 8) & 0xFF00;
    final |= (checksum >> 16) & 0xFF0000;
    final |= (checksum >> 24) & 0xFF000000;
    
    /* Mix with global variables */
    final ^= global_int;
    final += *(volatile short *)&global_int;
    final -= (global_char << 8);
    
    printf("Result: %u (0x%08x)\n", final, final);
    
    return final & 0x7FFFFFFF;  /* Return non-negative */
}
