#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0xEF;
volatile long global_long = 0x9ABCDEF012345678L;

/* Global arrays for memory access patterns */
volatile int global_array[32];
volatile short short_array[64];
volatile char char_array[128];

/* Bitfield structs for ZERO_EXTRACT/STRICT_LOW_PART generation */
struct bitfield_struct {
    unsigned int low_bits : 8;
    unsigned int mid_bits : 12;
    unsigned int high_bits : 12;
    volatile unsigned int padding;
};

struct mixed_bitfields {
    volatile unsigned short part1 : 4;
    volatile unsigned short part2 : 6;
    volatile unsigned short part3 : 6;
    unsigned int full_word;
};

/* Global bitfield instances */
struct bitfield_struct global_bf = {0};
struct mixed_bitfields global_mixed_bf = {0};

/* Function to create complex addressing modes */
static int complex_address(int index, int offset) {
    /* This should generate MEM_P with non-trivial address */
    return global_array[(index * 3 + offset) & 31];
}

/* Function to force SUBREG generation through type punning */
static short type_pun_int_to_short(volatile int *ptr) {
    /* Casting between different-sized types often generates SUBREG */
    return *(volatile short *)ptr;
}

/* Function for bitfield extraction - may generate ZERO_EXTRACT */
static unsigned extract_bits(volatile unsigned value, int shift, int width) {
    /* Explicit bit manipulation */
    return (value >> shift) & ((1u << width) - 1);
}

int main(int argc, char *argv[]) {
    int i, j;
    unsigned int checksum = 0;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < 32; i++) {
        global_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = (i * 1664525 + 1013904223) & 0x7FFF;
    }
    for (i = 0; i < 128; i++) {
        char_array[i] = (i * 134775813 + 1) & 0x7F;
    }
    
    /* Use command line arguments to create dynamic values */
    int base_index = (argc > 1) ? atoi(argv[1]) % 16 : 5;
    int shift_amount = (argc > 2) ? atoi(argv[2]) % 24 : 3;
    int loop_count = (argc > 3) ? atoi(argv[3]) % 100 + 10 : 20;
    
    /* Complex loop combining all patterns */
    for (i = 0; i < loop_count; i++) {
        volatile int temp;
        
        /* Pattern 1: Memory access with complex addressing - triggers MEM_P path */
        temp = complex_address(base_index + i, shift_amount);
        checksum ^= temp;
        
        /* Pattern 2: Type punning with pointer casting - may generate SUBREG */
        short s_val = type_pun_int_to_short(&global_int);
        checksum += s_val;
        
        /* Pattern 3: Direct bitfield extraction - may generate ZERO_EXTRACT */
        unsigned extracted = extract_bits(global_int, (i + shift_amount) & 31, 8);
        checksum += extracted;
        
        /* Pattern 4: Struct bitfield access with address taken */
        global_bf.low_bits = (i * 7) & 0xFF;
        global_bf.mid_bits = (i * 13) & 0xFFF;
        global_bf.high_bits = (i * 17) & 0xFFF;
        
        /* Take address of bitfield member - may generate STRICT_LOW_PART */
        volatile unsigned int *bf_ptr = &global_bf.padding;
        *bf_ptr = (*bf_ptr & ~0xFF) | (global_bf.low_bits);
        checksum ^= *bf_ptr;
        
        /* Pattern 5: Mixed bitfield operations */
        global_mixed_bf.part1 = (i + 1) & 0xF;
        global_mixed_bf.part2 = (i * 3) & 0x3F;
        global_mixed_bf.part3 = (i * 5) & 0x3F;
        
        /* Access through different-sized pointers */
        char *char_ptr = (char *)&global_mixed_bf.full_word;
        char_ptr[0] = global_mixed_bf.part1;
        char_ptr[1] = global_mixed_bf.part2 & 0xFF;
        checksum += global_mixed_bf.full_word;
        
        /* Pattern 6: Array access with variable index and bit extraction */
        int idx = (base_index * i + shift_amount) & 63;
        int val = (short_array[idx] >> ((i + 2) & 7)) & 0x1F;
        checksum ^= val;
        
        /* Pattern 7: Nested operations combining multiple patterns */
        /* This complex expression may generate nested RTL with ZERO_EXTRACT and SUBREG */
        long complex_val = (long)((global_array[i & 31] >> shift_amount) & 0xFF) +
                          (long)(*(volatile short *)(&char_array[(i * 2) & 127]));
        checksum += (complex_val & 0xFFFFFFFF);
        
        /* Pattern 8: Inline assembly to force partial register access */
        /* May generate SUBREG in RTL representation */
        unsigned int asm_input = checksum;
        unsigned short asm_output;
        __asm__ volatile (
            "movw %w1, %0"
            : "=r" (asm_output)
            : "r" (asm_input)
            : "cc"
        );
        checksum += asm_output;
    }
    
    /* Additional complex statement combining all patterns in one line */
    /* This increases chance of hitting the uncovered block */
    int final_computation = 
        ((*(volatile short *)(&global_array[(base_index + shift_amount) & 31]) >> 4) & 0x0F) +
        (type_pun_int_to_short(&global_long) & 0x7F) +
        (global_bf.low_bits << 2);
    
    checksum ^= final_computation;
    
    printf("Final checksum: %u\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
