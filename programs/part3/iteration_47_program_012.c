#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0xEF;
volatile long global_long = 0x9ABCDEF0;

/* Global array for memory access patterns */
volatile int global_array[32];

/* Bitfield structs to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int low8 : 8;
    unsigned int mid12 : 12;
    unsigned int high12 : 12;
    volatile unsigned int padding;
};

struct mixed_bitfield {
    volatile unsigned short part1 : 4;
    volatile unsigned short part2 : 6;
    volatile unsigned short part3 : 6;
};

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* Force non-trivial address computation */
    return (idx1 * 3 + idx2 * 7) & 31;
}

int main(int argc, char **argv) {
    int i, j;
    unsigned int checksum = 0;
    
    /* Initialize global array with pattern */
    for (i = 0; i < 32; i++) {
        global_array[i] = i * 0x11111111;
    }
    
    /* Use argc to create runtime-variable indices/shifts */
    int base_idx = argc > 1 ? atoi(argv[1]) % 16 : 5;
    int shift_amount = argc > 2 ? atoi(argv[2]) % 8 : 3;
    
    /* Create bitfield struct instances */
    struct bitfield_struct bf1;
    struct mixed_bitfield bf2;
    
    /* Initialize bitfields */
    bf1.low8 = 0xAA;
    bf1.mid12 = 0xBBB;
    bf1.high12 = 0xCCC;
    bf1.padding = 0xDDDDDDDD;
    
    bf2.part1 = 0x5;
    bf2.part2 = 0x15;
    bf2.part3 = 0x25;
    
    /* Loop with opaque control flow to stress resource tracking */
    for (i = 0; i < 100; i++) {
        int idx = (base_idx + i) & 31;
        
        /* Pattern 1: ZERO_EXTRACT from memory with bit manipulation */
        if (i & 1) {
            /* Extract bits 5-9 from global_int */
            unsigned int extracted = (global_int >> 5) & 0x1F;
            checksum += extracted;
            
            /* Extract bits from bitfield struct - may generate ZERO_EXTRACT */
            unsigned int bf_extract = bf1.mid12;
            checksum ^= bf_extract;
            
            /* Complex extraction with shift and mask */
            volatile int temp = global_array[idx];
            int shifted = (temp >> shift_amount) & ((1 << (shift_amount + 1)) - 1);
            checksum += shifted;
        }
        
        /* Pattern 2: Type punning to force SUBREG generation */
        if (i & 2) {
            /* Cast between different-sized types */
            short s_val = *(volatile short *)&global_int;
            checksum += s_val;
            
            /* Access char through int pointer */
            char c_val = *(volatile char *)((char *)&global_long + (i & 3));
            checksum += c_val;
            
            /* Mixed-type operations */
            long l_temp = global_long;
            int truncated = (int)l_temp;
            short half = (short)(truncated >> 16);
            checksum += half;
        }
        
        /* Pattern 3: Memory references with complex addresses */
        if (i & 4) {
            /* Array access with computed index */
            int complex_idx = complex_address(i, base_idx);
            int mem_val = global_array[complex_idx];
            checksum += mem_val;
            
            /* Pointer arithmetic with type conversion */
            volatile int *ptr = &global_array[0] + complex_idx;
            int deref = *ptr;
            checksum ^= deref;
            
            /* Structure member access via pointer */
            volatile unsigned int *bf_ptr = &bf1.padding;
            checksum += *bf_ptr;
        }
        
        /* Pattern 4: Combined operations in single statements */
        if (i & 8) {
            /* Extract bits from memory, convert type, and compute */
            int combined = (*(volatile short *)(&global_array[idx]) >> shift_amount) & 0xFF;
            checksum += combined;
            
            /* Bitfield extraction with type punning */
            unsigned int from_bf = *(volatile unsigned int *)&bf2;
            int masked = (from_bf >> 4) & 0x3F;  /* Extract part2 */
            checksum += masked;
        }
        
        /* Pattern 5: STRICT_LOW_PART simulation through bitfield assignment */
        if (i & 16) {
            /* Assign to bitfield - may generate STRICT_LOW_PART */
            bf1.low8 = (checksum & 0xFF);
            bf2.part2 = (checksum >> 4) & 0x3F;
            
            /* Update global with masked value */
            global_short = (global_short & 0xFF00) | (checksum & 0xFF);
        }
        
        /* Modify shift amount to create variation */
        shift_amount = (shift_amount + 1) & 7;
    }
    
    /* Additional complex expression to ensure coverage */
    {
        /* Nested extractions and type conversions */
        volatile long *long_ptr = (volatile long *)global_array;
        int val1 = (int)((*long_ptr >> 16) & 0xFFFF);
        
        volatile int *int_ptr = (volatile int *)&bf1;
        short val2 = (short)((*int_ptr >> 8) & 0xFF);
        
        checksum += val1 + val2;
        
        /* Force memory reference with address computation */
        int offset = base_idx * sizeof(int);
        char *byte_ptr = (char *)global_array + offset;
        int val3 = *(volatile int *)byte_ptr;
        checksum ^= val3;
    }
    
    printf("Final checksum: %u\n", checksum);
    return checksum & 0xFF;
}
