#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0xEF;
volatile long global_long = 0x9ABCDEF012345678UL;

/* Global arrays for memory access patterns */
volatile int int_array[32];
volatile short short_array[64];
volatile char char_array[128];

/* Bitfield structs for ZERO_EXTRACT/STRICT_LOW_PART generation */
struct bitfield_struct {
    unsigned int low8 : 8;
    unsigned int mid16 : 16;
    unsigned int high8 : 8;
    unsigned int extra : 24;
    unsigned int last16 : 16;
};

struct packed_bitfield {
    unsigned short a : 4;
    unsigned short b : 6;
    unsigned short c : 3;
    unsigned short d : 3;
};

/* Global bitfield instances */
volatile struct bitfield_struct bf1;
volatile struct packed_bitfield bf2;

/* Initialize arrays with pattern */
void init_arrays(void) {
    for (int i = 0; i < 32; i++) {
        int_array[i] = i * 0x11111111;
    }
    for (int i = 0; i < 64; i++) {
        short_array[i] = i * 0x1111;
    }
    for (int i = 0; i < 128; i++) {
        char_array[i] = i * 0x11;
    }
}

/* Complex function combining multiple patterns */
int process_bitfield(int index, int shift, int mask) {
    int result = 0;
    
    /* Pattern 1: ZERO_EXTRACT from memory with shifting */
    /* Should generate ZERO_EXTRACT RTL */
    volatile int temp = global_int;
    result ^= (temp >> shift) & mask;
    
    /* Pattern 2: Direct bitfield access - may generate STRICT_LOW_PART */
    bf1.low8 = (index & 0xFF);
    result ^= bf1.low8;
    
    /* Pattern 3: Type punning with different sizes - may generate SUBREG */
    volatile short* short_ptr = (volatile short*)&global_int;
    volatile char* char_ptr = (volatile char*)&global_long;
    
    /* Mixed type accesses */
    short s_val = *short_ptr;
    char c_val = *char_ptr;
    
    /* Combine with shifting - more SUBREG possibilities */
    result ^= ((int)s_val << 8) | c_val;
    
    /* Pattern 4: Complex memory addressing */
    /* Non-constant index calculation */
    int idx1 = (index * 3 + shift) & 31;
    int idx2 = (index * 5 + shift) & 63;
    int idx3 = (index * 7 + shift) & 127;
    
    /* Memory access with variable indices */
    result ^= int_array[idx1];
    result ^= short_array[idx2] << 16;
    result ^= char_array[idx3];
    
    /* Pattern 5: Packed bitfield extraction */
    bf2.a = (index & 0x0F);
    bf2.b = ((index >> 4) & 0x3F);
    bf2.c = ((index >> 10) & 0x07);
    bf2.d = ((index >> 13) & 0x07);
    
    /* Access bitfield through pointer - may generate ZERO_EXTRACT */
    volatile unsigned short* bf_ptr = (volatile unsigned short*)&bf2;
    result ^= (*bf_ptr & 0xFFF);
    
    return result;
}

/* Function with inline assembly to force SUBREG usage */
int force_subreg_operations(int x) {
    int result = 0;
    
    /* Inline assembly with register constraints */
    short s_val;
    char c_val;
    
    /* Force partial register access */
    asm volatile ("movw %1, %0" : "=r"(s_val) : "r"(x));
    asm volatile ("movb %1, %0" : "=r"(c_val) : "r"(x));
    
    /* Type conversions that may generate SUBREG */
    result = (int)s_val + (int)c_val;
    
    /* More type punning */
    volatile int* int_ptr = &global_int;
    volatile short* short_ptr = (volatile short*)int_ptr;
    
    /* Access different parts of the same memory */
    result ^= short_ptr[0];
    result ^= short_ptr[1];
    
    return result;
}

/* Main function with complex control flow */
int main(int argc, char** argv) {
    int checksum = 0;
    
    /* Initialize data */
    init_arrays();
    
    /* Use command line arguments for dynamic values */
    int base_index = argc > 1 ? atoi(argv[1]) % 16 : 5;
    int base_shift = argc > 2 ? atoi(argv[2]) % 24 : 3;
    int base_mask = argc > 3 ? atoi(argv[3]) % 256 : 0x3F;
    
    /* Complex loop with opaque control flow */
    for (int i = 0; i < 100; i++) {
        int index = (base_index + i) & 31;
        int shift = (base_shift + i * 3) & 31;
        int mask = (base_mask + i * 5) & 0xFF;
        
        /* Branch with external condition */
        if (argc > 1 && argv[1][0] % 2) {
            /* Path 1: Focus on bitfield operations */
            checksum ^= process_bitfield(index, shift, mask);
            
            /* Additional bit manipulation */
            volatile long* long_ptr = &global_long;
            volatile int* int_ptr = (volatile int*)long_ptr;
            
            /* Access different parts of long */
            checksum ^= int_ptr[0];
            checksum ^= int_ptr[1];
            
            /* More ZERO_EXTRACT patterns */
            checksum ^= (global_int >> (shift & 7)) & 0xF;
            checksum ^= (global_long >> (shift * 2)) & 0xFF;
        } else {
            /* Path 2: Focus on type conversions and SUBREG */
            checksum ^= force_subreg_operations(index);
            
            /* Complex memory addressing with pointer arithmetic */
            volatile char* ptr = char_array + index * 3;
            for (int j = 0; j < 4; j++) {
                checksum ^= ptr[j] << (j * 8);
            }
            
            /* Struct member access via pointer */
            volatile struct bitfield_struct* bf_ptr = &bf1;
            checksum ^= bf_ptr->mid16;
        }
        
        /* Alternate between different operations based on loop counter */
        if (i & 1) {
            /* Bitfield struct assignment */
            bf1.mid16 = checksum & 0xFFFF;
            bf1.high8 = (checksum >> 16) & 0xFF;
            
            /* Access via different type pointer */
            volatile unsigned int* uint_ptr = (volatile unsigned int*)&bf1;
            checksum ^= *uint_ptr;
        } else {
            /* Array access with complex index calculation */
            int complex_idx = (index * 11 + shift * 7 + mask) & 31;
            checksum ^= int_array[complex_idx];
            
            /* Pointer casting between types */
            volatile short* alias_ptr = (volatile short*)&int_array[complex_idx];
            checksum ^= alias_ptr[0] + alias_ptr[1];
        }
    }
    
    /* Final output to prevent dead code elimination */
    printf("Checksum: 0x%08X\n", checksum);
    
    return checksum & 0xFF;
}
