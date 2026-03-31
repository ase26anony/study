#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0xEF;
volatile long global_long = 0x9ABCDEF012345678L;

/* Global arrays for memory access patterns */
volatile int int_array[32];
volatile short short_array[64];
volatile char char_array[128];

/* Bitfield structs for ZERO_EXTRACT/STRICT_LOW_PART generation */
struct bitfield_struct {
    unsigned int low_bits : 4;
    unsigned int middle_bits : 8;
    unsigned int high_bits : 20;
    volatile unsigned int padding;
};

struct nested_bitfield {
    struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 8;
    } inner;
    unsigned int d : 16;
};

/* Global bitfield instances */
volatile struct bitfield_struct bf1;
volatile struct nested_bitfield bf2;

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* This should generate MEM_P with complex address */
    return int_array[idx1 * 3 + idx2 * 7];
}

/* Function to force SUBREG generation through type punning */
static short type_pun_int_to_short(volatile int *ptr) {
    /* Casting between different-sized types often generates SUBREG */
    return *(volatile short *)ptr;
}

/* Function for bitfield extraction patterns */
static unsigned int extract_bits(volatile unsigned int value, int shift, int width) {
    /* Explicit bit manipulation for ZERO_EXTRACT */
    return (value >> shift) & ((1u << width) - 1);
}

int main(int argc, char *argv[]) {
    int i, j;
    unsigned int checksum = 0;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < 32; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    for (i = 0; i < 128; i++) {
        char_array[i] = (i * 1103515245 + 12345) & 0x7F;
    }
    
    /* Initialize bitfield structs */
    bf1.low_bits = 0xA;
    bf1.middle_bits = 0xBC;
    bf1.high_bits = 0x12345;
    bf1.padding = 0xDEADBEEF;
    
    bf2.inner.a = 0x5;
    bf2.inner.b = 0x12;
    bf2.inner.c = 0xAB;
    bf2.d = 0xCDEF;
    
    /* Use command line arguments to create dynamic indices */
    int base_idx = (argc > 1) ? atoi(argv[1]) % 16 : 5;
    int shift_val = (argc > 2) ? atoi(argv[2]) % 8 : 3;
    
    /* Main loop combining all patterns */
    for (i = 0; i < 100; i++) {
        int dynamic_idx = (base_idx + i) & 0x1F;
        int alt_idx = (i * 7) & 0x3F;
        
        /* PATTERN 1: Memory access with complex addressing (for MEM_P) */
        volatile int *mem_ptr;
        if (i & 1) {
            /* Array indexing with computation */
            mem_ptr = &int_array[dynamic_idx * 2 + alt_idx];
        } else {
            /* Pointer arithmetic */
            mem_ptr = (volatile int *)((char *)int_array + dynamic_idx * sizeof(int) * 3);
        }
        
        /* Combine with bit extraction */
        int mem_val = *mem_ptr;
        checksum += mem_val;
        
        /* PATTERN 2: Type punning for SUBREG generation */
        short subreg_val;
        if (i & 2) {
            /* Cast between different integer sizes */
            subreg_val = type_pun_int_to_short(&global_int);
        } else {
            /* Direct cast in expression */
            subreg_val = *(volatile short *)(&int_array[dynamic_idx]);
        }
        
        /* Further process to create more complex RTL */
        int extended_val = (int)subreg_val * 2;
        checksum += extended_val;
        
        /* PATTERN 3: Bitfield operations for ZERO_EXTRACT/STRICT_LOW_PART */
        unsigned int bitfield_result;
        if (i & 4) {
            /* Explicit bit extraction (likely ZERO_EXTRACT) */
            bitfield_result = extract_bits(global_int, shift_val + (i & 7), 4 + (i & 3));
        } else {
            /* Struct bitfield access */
            volatile unsigned int *bf_ptr;
            if (i & 8) {
                bf_ptr = &bf1.padding;
            } else {
                bf_ptr = (volatile unsigned int *)&bf2;
            }
            /* Combine with shift for more complex pattern */
            bitfield_result = (*bf_ptr >> (shift_val)) & 0xFF;
        }
        
        checksum += bitfield_result;
        
        /* PATTERN 4: Combined operation - memory access with bit extraction */
        if (i & 16) {
            /* Access char array, promote to int, extract bits */
            volatile char *char_ptr = &char_array[dynamic_idx * 2];
            int combined = ((int)(*char_ptr) << 8) | (int)char_array[alt_idx];
            int extracted = (combined >> (shift_val)) & ((1 << (4 + (i & 3))) - 1);
            checksum += extracted;
        }
        
        /* PATTERN 5: Nested complex expression */
        if (i & 32) {
            /* Multiple memory accesses with type conversions */
            long temp = (long)int_array[dynamic_idx] * (long)short_array[alt_idx];
            /* Extract portions using different type accesses */
            short low_part = (short)(temp & 0xFFFF);
            short high_part = (short)((temp >> 16) & 0xFFFF);
            /* Combine with bit manipulation */
            int nested_result = ((int)low_part << 8) | (int)high_part;
            nested_result = (nested_result >> (shift_val)) & 0x3FF;
            checksum += nested_result;
        }
    }
    
    /* Additional patterns outside loop for variety */
    
    /* Direct bitfield struct member access */
    checksum += bf1.low_bits;
    checksum += bf1.middle_bits;
    checksum += bf1.high_bits;
    
    /* Complex pointer chain */
    volatile char *chain_ptr = (volatile char *)&bf2;
    for (j = 0; j < 4; j++) {
        checksum += (unsigned int)(*(chain_ptr + j * 2));
    }
    
    /* Mixed-size array access with computation */
    int mixed_result = 0;
    for (j = 0; j < 8; j++) {
        mixed_result += short_array[j * 3] + char_array[j * 5];
    }
    checksum += mixed_result;
    
    printf("Final checksum: %u\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
