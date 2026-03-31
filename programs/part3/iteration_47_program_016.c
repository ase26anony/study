#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0xEF;
volatile long global_long = 0x9ABCDEF0;

/* Global array for memory access patterns */
volatile int global_array[32];
volatile short short_array[64];

/* Bitfield structs for ZERO_EXTRACT/STRICT_LOW_PART generation */
struct bitfield_struct {
    unsigned int low_bits : 8;
    unsigned int middle_bits : 12;
    unsigned int high_bits : 12;
    volatile unsigned int padding;
};

struct mixed_bitfields {
    volatile unsigned short part1 : 4;
    volatile unsigned short part2 : 6;
    volatile unsigned short part3 : 6;
};

/* Global bitfield structs */
struct bitfield_struct bf_global;
struct mixed_bitfields mixed_bf;

/* Function to create complex addressing modes */
static int complex_memory_access(int index, int offset) {
    /* Create MEM_P with complex address expression */
    volatile int *ptr;
    
    /* Array access with variable index and offset */
    ptr = &global_array[(index * 3 + offset) & 31];
    
    /* Additional pointer arithmetic */
    ptr = (volatile int *)((char *)ptr + (offset & 3));
    
    return *ptr;
}

/* Function to generate SUBREG patterns through type punning */
static short generate_subreg(int value) {
    volatile int temp = value;
    
    /* Type conversions that may generate SUBREG */
    short s1 = *(volatile short *)&temp;           /* Cast int* to short* */
    char c1 = *(volatile char *)(&temp + 1);       /* Different byte */
    
    /* Mix types in expressions */
    return (short)((temp >> 8) & 0xFF) + s1 - c1;
}

/* Function to create ZERO_EXTRACT patterns */
static unsigned extract_bits(volatile unsigned int source, int start, int length) {
    /* Explicit bit extraction - may generate ZERO_EXTRACT */
    unsigned mask = (1u << length) - 1;
    return (source >> start) & mask;
}

/* Function using struct bitfield address taking */
static int bitfield_address_ops(void) {
    int result = 0;
    volatile unsigned int *ptr;
    
    /* Take address of bitfield member (may generate special RTL) */
    ptr = &bf_global.padding;
    result = *ptr;
    
    /* Access through pointer with offset */
    ptr = (volatile unsigned int *)&mixed_bf;
    result ^= *(volatile unsigned short *)ptr;
    
    return result;
}

int main(int argc, char **argv) {
    int i, j;
    unsigned int checksum = 0;
    int loop_count = 100;
    
    /* Use argc to determine loop behavior - prevents dead code elimination */
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0 || loop_count > 1000) {
            loop_count = 100;
        }
    }
    
    /* Initialize global data */
    for (i = 0; i < 32; i++) {
        global_array[i] = i * 0x12345 + 0x6789;
    }
    
    for (i = 0; i < 64; i++) {
        short_array[i] = (short)(i * 0xABCD - 0x1234);
    }
    
    bf_global.low_bits = 0xAA;
    bf_global.middle_bits = 0xBBB;
    bf_global.high_bits = 0xCCC;
    bf_global.padding = 0xDDDDDDDD;
    
    mixed_bf.part1 = 0x5;
    mixed_bf.part2 = 0x15;
    mixed_bf.part3 = 0x25;
    
    /* Main loop combining all patterns */
    for (i = 0; i < loop_count; i++) {
        volatile int temp;
        volatile short stemp;
        volatile char ctemp;
        
        /* Pattern 1: ZERO_EXTRACT from memory with shifting */
        temp = global_int;
        checksum += extract_bits(temp, i & 7, 5);          /* 5-bit extract at variable position */
        checksum += (temp >> (i & 15)) & 0x3F;            /* Another extract pattern */
        
        /* Pattern 2: SUBREG generation through type mixing */
        stemp = generate_subreg(global_int + i);
        checksum += stemp;
        
        /* Pattern 3: Complex memory addressing */
        checksum += complex_memory_access(i, i >> 1);
        
        /* Pattern 4: Bitfield struct operations */
        checksum += bitfield_address_ops();
        
        /* Pattern 5: Combined operation - memory access with bit extraction */
        /* This may generate nested RTL patterns */
        temp = global_array[(i * 7) & 31];
        checksum += (temp >> ((i * 3) & 31)) & ((1 << ((i & 3) + 1)) - 1);
        
        /* Pattern 6: Pointer casting between different integer types */
        ctemp = *(volatile char *)(&global_long + (i & 3));
        checksum += ctemp;
        
        /* Pattern 7: Access short array with int pointer (may generate SUBREG) */
        if (i & 1) {
            checksum += *(volatile int *)(&short_array[(i * 2) & 63]);
        }
        
        /* Pattern 8: Bitfield extraction from volatile */
        /* Force evaluation of bitfield lvalues */
        mixed_bf.part1 = (mixed_bf.part1 + 1) & 0xF;
        mixed_bf.part2 = (mixed_bf.part2 ^ checksum) & 0x3F;
        checksum += mixed_bf.part1 + mixed_bf.part2;
        
        /* Pattern 9: Memory access with bitfield-like extraction */
        /* Using shift and mask on memory operand */
        volatile int *mem_ptr = &global_array[i & 15];
        checksum += ((*mem_ptr) >> 8) & 0xFF;    /* Extract middle byte */
        
        /* Pattern 10: Nested complex expression */
        /* Combines memory access, shift, mask, and type conversion */
        checksum += (short)((global_array[(i + 1) & 15] >> 4) & 0xFFF);
    }
    
    /* Additional test cases outside loop for different contexts */
    if (argc > 2) {
        /* Test STRICT_LOW_PART-like patterns with small integer types */
        volatile short *sp = (volatile short *)&global_int;
        *sp = (short)checksum;  /* Writing to part of a larger object */
        
        /* More bitfield manipulation */
        struct {
            volatile unsigned int low : 16;
            volatile unsigned int high : 16;
        } split_int;
        
        split_int.low = checksum & 0xFFFF;
        split_int.high = (checksum >> 16) & 0xFFFF;
        
        checksum += split_int.low * split_int.high;
    }
    
    printf("Checksum: %u\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
