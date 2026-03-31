#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0x42;
volatile long global_long = 0x9876543210ABCDEF;

/* Global arrays for memory access patterns */
volatile int int_array[32];
volatile short short_array[64];
volatile char char_array[128];

/* Bitfield structs to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int low_bits : 8;
    unsigned int mid_bits : 16;
    unsigned int high_bits : 8;
    unsigned int extra : 24;
} volatile bitfield_global = {0xAA, 0xBBCC, 0xDD, 0xEEFF00};

/* Another bitfield struct with different layout */
struct mixed_bitfield {
    signed int signed_field : 12;
    unsigned int unsigned_field : 20;
    short short_field : 10;
    char char_field : 6;
} volatile mixed_bf = {-2048, 0xABCDE, 512, 31};

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* This should generate MEM_P with complex address */
    return int_array[idx1 * 3 + idx2 * 7 - 5];
}

/* Function to force SUBREG appearances through type conversions */
static short type_punning_conversion(int val) {
    /* Multiple type conversions to generate SUBREG */
    char c = (char)(val >> 8);
    short s = (short)(c * 256 + (val & 0xFF));
    int i = (int)s * 2;
    return (short)(i >> 4);
}

/* Function for bitfield extraction patterns */
static unsigned extract_bits(volatile struct bitfield_struct *bf, int mode) {
    unsigned result = 0;
    
    if (mode == 0) {
        /* Direct bitfield access - may generate ZERO_EXTRACT */
        result = bf->low_bits;
    } else if (mode == 1) {
        /* Combined bitfield access */
        result = (bf->mid_bits << 8) | bf->low_bits;
    } else {
        /* Manual bit extraction similar to bitfield */
        volatile unsigned int *p = (volatile unsigned int *)bf;
        result = (*p >> 5) & 0x1F;  /* Should generate ZERO_EXTRACT */
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int i, j;
    unsigned int checksum = 0;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 32; i++) {
        int_array[i] = i * 0x1234567;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = i * 0xABCD;
    }
    for (i = 0; i < 128; i++) {
        char_array[i] = i * 0x23;
    }
    
    /* Use command line arguments to create runtime variability */
    int base_idx = (argc > 1) ? atoi(argv[1]) % 16 : 5;
    int shift_amount = (argc > 2) ? atoi(argv[2]) % 8 : 3;
    int mode_selector = (argc > 3) ? atoi(argv[3]) % 4 : 0;
    
    /* Loop with opaque control flow */
    for (i = 0; i < 100; i++) {
        /* Vary behavior based on loop counter and external input */
        int effective_idx = (i + base_idx) & 31;
        int alt_idx = (i * 7 + shift_amount) & 63;
        
        /* Pattern 1: Memory access with complex addressing */
        if ((i & 1) == 0) {
            /* Array access with non-constant offset */
            volatile int *ptr1 = &int_array[effective_idx];
            volatile int *ptr2 = &int_array[alt_idx & 31];
            
            /* Complex pointer arithmetic */
            int val1 = ptr1[shift_amount];
            int val2 = ptr2[-shift_amount];
            
            /* Combine with bit manipulation */
            checksum += (val1 >> shift_amount) & 0xFF;
            checksum += (val2 << shift_amount) & 0xFF00;
        }
        
        /* Pattern 2: Bitfield operations */
        if ((i & 2) == 0) {
            /* Access bitfield struct through pointer */
            volatile struct bitfield_struct *bf_ptr = &bitfield_global;
            
            /* Multiple bitfield extractions */
            unsigned bits1 = extract_bits(bf_ptr, mode_selector);
            unsigned bits2 = bf_ptr->high_bits;
            
            /* Manual bit extraction that may generate ZERO_EXTRACT */
            volatile unsigned int *raw_ptr = (volatile unsigned int *)bf_ptr;
            unsigned extracted = (*raw_ptr >> (i % 16)) & ((1 << (shift_amount + 1)) - 1);
            
            checksum ^= (bits1 << 16) | bits2;
            checksum += extracted;
        }
        
        /* Pattern 3: Type punning and SUBREG generation */
        if ((i & 4) == 0) {
            /* Multiple type conversions */
            volatile int *int_ptr = &global_int;
            volatile short *short_ptr = (volatile short *)int_ptr;
            volatile char *char_ptr = (volatile char *)int_ptr;
            
            /* Access different views of the same memory */
            short s_val = short_ptr[(i >> 1) & 1];
            char c_val = char_ptr[i & 3];
            
            /* More type conversions */
            int converted = type_punning_conversion(s_val * c_val);
            
            /* Pointer casting between different sizes */
            long long_val = *(volatile long *)(&short_array[alt_idx]);
            int truncated = (int)long_val;
            
            checksum += converted + truncated;
        }
        
        /* Pattern 4: Combined operation in single statement */
        /* This tries to create nested RTL expressions */
        if ((i & 8) == 0) {
            /* Complex expression combining multiple patterns */
            int combined = 
                ((*(volatile short *)(&int_array[effective_idx]) >> shift_amount) & 0x1F) +
                ((mixed_bf.signed_field << 4) | (mixed_bf.char_field));
            
            /* Additional memory indirection */
            volatile int **indirect_ptr = (volatile int **)&int_array;
            if ((i % 3) == 0) {
                combined += **indirect_ptr;
            }
            
            checksum = (checksum << 3) | (combined & 0x7);
        }
        
        /* Pattern 5: STRICT_LOW_PART simulation through bitfield assignment */
        if ((i % 5) == 0) {
            /* Modify bitfield - may generate STRICT_LOW_PART */
            struct bitfield_struct local_bf;
            volatile struct bitfield_struct *bfp = &local_bf;
            
            /* Initialize */
            local_bf.low_bits = i & 0xFF;
            local_bf.mid_bits = (i * 3) & 0xFFFF;
            local_bf.high_bits = (i >> 4) & 0xFF;
            
            /* Access through pointer with offset */
            unsigned *as_uint = (unsigned *)bfp;
            checksum += as_uint[0] ^ as_uint[1];
        }
    }
    
    /* Final memory access with very complex addressing */
    volatile int *final_ptr = &int_array[
        ((checksum & 0xF) * base_idx + 
         (checksum >> 4 & 0xF) * shift_amount) & 31
    ];
    checksum += *final_ptr;
    
    /* Access via multiple pointer indirections */
    volatile char *char_ptr_base = char_array;
    for (j = 0; j < 8; j++) {
        checksum += char_ptr_base[(base_idx * j + shift_amount) & 127];
    }
    
    printf("Final checksum: %u\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
