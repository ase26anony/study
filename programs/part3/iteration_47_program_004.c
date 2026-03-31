#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0x42;
volatile long global_long = 0x9876543210ABCDEF;

/* Global arrays for memory access patterns */
volatile int int_array[32];
volatile short short_array[64];
volatile char char_array[128];

/* Bitfield structs for ZERO_EXTRACT/STRICT_LOW_PART generation */
struct bitfield_struct {
    unsigned int low8 : 8;
    unsigned int mid16 : 16;
    unsigned int high8 : 8;
};

struct mixed_bitfields {
    signed short s_field : 9;
    unsigned char c_field : 7;
    unsigned int i_field : 12;
    signed long l_field : 28;
};

/* Global bitfield instances */
volatile struct bitfield_struct bf1;
volatile struct mixed_bitfields bf2;

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* This should generate MEM with complex address */
    return int_array[idx1 * 3 + idx2 * 7 - 5];
}

/* Function to force SUBREG through type conversions */
static short type_punning_conversion(int val) {
    /* Multiple type conversions to generate SUBREG */
    char c = (char)(val >> 8);
    short s = (short)(c * 3);
    int i = (int)s + 0x100;
    return (short)(i & 0xFFFF);
}

int main(int argc, char **argv) {
    int i, j;
    int checksum = 0;
    
    /* Initialize arrays with pseudo-random but deterministic values */
    for (i = 0; i < 32; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    for (i = 0; i < 128; i++) {
        char_array[i] = (i * 1103515245 + 12345) & 0x7F;
    }
    
    /* Use command line arguments to create dynamic indices */
    int base_idx = (argc > 1) ? atoi(argv[1]) % 16 : 3;
    int shift_amount = (argc > 2) ? atoi(argv[2]) % 8 : 2;
    int mask_val = (argc > 3) ? atoi(argv[3]) % 32 : 0x1F;
    
    /* Loop with opaque control flow to stress resource analysis */
    for (i = 0; i < 100; i++) {
        int dynamic_idx = (i * base_idx) % 32;
        int alt_idx = (i * 7 + shift_amount) % 64;
        
        /* PATTERN 1: ZERO_EXTRACT from memory with bit manipulation */
        /* This should generate ZERO_EXTRACT RTL */
        int extracted_bits = (global_int >> shift_amount) & mask_val;
        checksum ^= extracted_bits;
        
        /* PATTERN 2: Bitfield access - may generate ZERO_EXTRACT or STRICT_LOW_PART */
        bf1.low8 = (i & 0xFF);
        bf1.mid16 = (i * 3) & 0xFFFF;
        volatile unsigned int *bf_ptr = (volatile unsigned int*)&bf1.low8;
        checksum += *bf_ptr;
        
        /* PATTERN 3: Complex memory reference with array indexing */
        /* Should generate MEM with complex address calculation */
        int mem_val = int_array[dynamic_idx + alt_idx / 2];
        checksum += mem_val * 3;
        
        /* PATTERN 4: Type punning with pointer casting - may generate SUBREG */
        /* Access short as char and vice versa */
        volatile char *char_ptr = (volatile char*)&short_array[alt_idx];
        checksum += char_ptr[0] + char_ptr[1];
        
        volatile short *short_ptr = (volatile short*)&int_array[dynamic_idx];
        checksum += short_ptr[0] - short_ptr[1];
        
        /* PATTERN 5: Combined operation - memory access with bit extraction */
        /* May generate nested patterns in RTL */
        int combined = (int_array[(i + base_idx) % 32] >> (i % 8)) & 0xF;
        checksum ^= combined << 4;
        
        /* PATTERN 6: Structure pointer with offset calculation */
        /* Complex addressing mode */
        volatile int *struct_member = &((volatile int*)&bf2)[i % 2];
        checksum += *struct_member;
        
        /* PATTERN 7: Inline assembly to force partial register access */
        /* May generate SUBREG in RTL representation */
        {
            int temp = checksum;
            short temp_short;
            /* Force register constraints that might create SUBREG */
            asm volatile ("movw %w1, %w0" : "=r" (temp_short) : "r" (temp));
            checksum = temp_short * 5;
        }
        
        /* PATTERN 8: Mixed bitfield operations */
        bf2.s_field = (i * 2) & 0x1FF;
        bf2.c_field = (i + 5) & 0x7F;
        bf2.i_field = (checksum >> 4) & 0xFFF;
        
        /* Access bitfield through volatile pointer */
        volatile signed short *sptr = &bf2.s_field;
        checksum += *sptr;
        
        /* PATTERN 9: Multi-level type conversion chain */
        /* Should generate multiple SUBREG operations */
        long long_val = global_long + i;
        int int_val = (int)long_val;
        short short_val = type_punning_conversion(int_val);
        char char_val = (char)(short_val >> 4);
        checksum += char_val;
        
        /* PATTERN 10: Conditional access to different patterns */
        /* Opaque control flow */
        if (checksum & 1) {
            /* Bit extraction from memory */
            int conditional_extract = (char_array[i % 128] >> (checksum % 4)) & 3;
            checksum += conditional_extract;
        } else {
            /* Type-punned memory access */
            int conditional_access = *(volatile short*)(&char_array[(i * 2) % 128]);
            checksum ^= conditional_access;
        }
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 7 == 0) {
            checksum += complex_address(i % 8, (i * 2) % 8);
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    return checksum & 0xFF;
}
