#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variables to prevent optimization */
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
    unsigned int field1 : 4;
    unsigned int field2 : 8;
    unsigned int field3 : 12;
    unsigned int field4 : 8;
    volatile unsigned int raw;
};

struct mixed_bitfields {
    volatile unsigned short low : 6;
    volatile unsigned short mid : 5;
    volatile unsigned short high : 5;
    unsigned int padding;
};

/* Global struct instances */
struct bitfield_struct bf_global;
struct mixed_bitfields mixed_bf;

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* Force non-constant address computation */
    volatile int *ptr;
    
    /* Array indexing with variable offset - may generate MEM with complex address */
    int val1 = global_array[idx1 * 2 + idx2];
    
    /* Pointer arithmetic that can't be simplified */
    ptr = &global_array[16] + (idx1 - idx2);
    int val2 = *ptr;
    
    /* Structure pointer with offset */
    char *char_ptr = (char *)&bf_global + idx1;
    int val3 = *char_ptr;
    
    return val1 + val2 + val3;
}

/* Function to force SUBREG generation through type conversions */
static short type_punning_conversions(int value) {
    /* Various type conversions that may generate SUBREG */
    volatile int vi = value;
    
    /* int -> short through pointer cast - likely SUBREG */
    short s1 = *(volatile short *)&vi;
    
    /* int -> char through shift and mask */
    char c1 = (vi >> 8) & 0xFF;
    
    /* Combine with different sizes */
    long l1 = (long)s1 * (long)c1;
    
    /* Return as short, forcing truncation */
    return (short)(l1 & 0xFFFF);
}

/* Function for bitfield operations targeting ZERO_EXTRACT */
static unsigned int bitfield_operations(int shift, unsigned mask) {
    unsigned int result = 0;
    
    /* Direct bitfield access - may generate ZERO_EXTRACT */
    bf_global.field2 = (global_int >> shift) & 0xFF;
    
    /* Explicit bit extraction - may generate ZERO_EXTRACT */
    unsigned int extracted = (global_long >> (shift * 2)) & mask;
    
    /* Mixed bitfield operations */
    mixed_bf.mid = (extracted >> 3) & 0x1F;
    
    /* Complex bit manipulation with memory */
    volatile unsigned int *int_ptr = &bf_global.raw;
    *int_ptr = (*int_ptr & ~(0xFF << shift)) | ((extracted & 0xFF) << shift);
    
    /* Combine results */
    result = bf_global.field2 | (mixed_bf.mid << 8) | (extracted << 16);
    
    return result;
}

int main(int argc, char **argv) {
    int i, j;
    unsigned int checksum = 0;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 32; i++) {
        global_array[i] = i * 0x11111111;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = i * 0x1111;
    }
    for (i = 0; i < 128; i++) {
        char_array[i] = i;
    }
    
    /* Initialize bitfield structs */
    bf_global.raw = 0xDEADBEEF;
    mixed_bf.padding = 0xCAFEBABE;
    
    /* Use command line arguments to get dynamic values */
    int base_shift = (argc > 1) ? atoi(argv[1]) % 16 : 3;
    int base_index = (argc > 2) ? atoi(argv[2]) % 8 : 1;
    
    /* Main loop combining all patterns */
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 8; j++) {
            int dynamic_shift = (base_shift + i + j) % 24;
            unsigned dynamic_mask = 0xFF >> (j % 4);
            int idx1 = (base_index + i) % 28;
            int idx2 = (base_index + j) % 28;
            
            /* Pattern 1: Bitfield operations (ZERO_EXTRACT/STRICT_LOW_PART) */
            unsigned int bf_result = bitfield_operations(dynamic_shift, dynamic_mask);
            checksum ^= bf_result;
            
            /* Pattern 2: Type conversions (SUBREG) */
            short conv_result = type_punning_conversions(bf_result + i);
            checksum += (unsigned int)conv_result;
            
            /* Pattern 3: Complex memory addressing (MEM_P) */
            int mem_result = complex_address(idx1, idx2);
            checksum ^= (mem_result << (i % 16));
            
            /* Combined pattern: Memory access with bit extraction and type conversion */
            if (i & 1) {
                /* Access memory, extract bits, convert type */
                volatile int *mem_ptr = &global_array[(idx1 + idx2) % 32];
                unsigned int mem_val = *mem_ptr;
                
                /* This complex expression may generate nested RTL:
                   MEM -> ZERO_EXTRACT -> SUBREG */
                short combined = (short)((mem_val >> dynamic_shift) & dynamic_mask);
                checksum += combined;
                
                /* Another combined pattern with pointer casting */
                volatile char *byte_ptr = (volatile char *)mem_ptr + j;
                int byte_val = *byte_ptr;
                checksum ^= (byte_val << 8);
            }
            
            /* Additional SUBREG patterns with different integer sizes */
            {
                volatile long big_val = global_long + checksum;
                int int_val = (int)big_val;  /* Potential SUBREG for truncation */
                char small_val = (char)(int_val >> (dynamic_shift % 8));
                checksum += small_val;
            }
        }
    }
    
    /* Final mixed operation to ensure all patterns are used */
    {
        /* Force ZERO_EXTRACT from memory location */
        volatile int *final_ptr = &global_array[checksum % 32];
        unsigned int final_bits = (*final_ptr >> 4) & 0x0F0F0F0F;
        
        /* Convert through different types (SUBREG) */
        short final_short = *(volatile short *)&final_bits;
        char final_char = (final_bits >> 16) & 0xFF;
        
        /* Complex address calculation */
        volatile int *addr_ptr = &global_array[0] + (final_short % 16) + (final_char % 8);
        checksum ^= *addr_ptr;
    }
    
    printf("Final checksum: 0x%08X\n", checksum);
    return checksum & 0xFF;
}
