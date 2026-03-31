#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variables to prevent constant folding */
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
    unsigned int mid_bits : 12;
    unsigned int high_bits : 12;
    volatile unsigned int padding;
};

struct mixed_bitfields {
    volatile unsigned short a : 4;
    volatile unsigned short b : 6;
    volatile unsigned short c : 6;
    unsigned int d : 16;
};

/* Global bitfield instances */
volatile struct bitfield_struct bf1;
volatile struct mixed_bitfields bf2;

/* Function to create complex addressing modes */
static int complex_memory_access(int index, int offset) {
    /* Create SUBREG patterns through type conversions */
    short temp_short = *(volatile short*)(&int_array[index]);
    char temp_char = *(volatile char*)(&short_array[offset]);
    
    /* Combine with bit extraction */
    int result = ((temp_short >> 4) & 0x0F) | ((temp_char << 4) & 0xF0);
    
    /* More type punning for SUBREG */
    long temp_long = *(volatile long*)(&int_array[index % 8]);
    int truncated = (int)temp_long;  /* Potential SUBREG here */
    
    return result + truncated;
}

/* Function to generate ZERO_EXTRACT patterns */
static unsigned int bitfield_operations(int shift, int mask) {
    unsigned int result = 0;
    
    /* Direct bit extraction from volatile - may generate ZERO_EXTRACT */
    result = (global_int >> shift) & mask;
    
    /* Access bitfield through pointer - may generate STRICT_LOW_PART */
    volatile unsigned int* ptr = (volatile unsigned int*)&bf1.low_bits;
    result ^= *ptr;
    
    /* Complex bitfield extraction with multiple operations */
    struct mixed_bitfields local_bf;
    local_bf.a = (global_char >> 2) & 0x0F;
    local_bf.b = (global_short >> 4) & 0x3F;
    local_bf.c = (global_int >> 8) & 0x3F;
    
    /* Take address of bitfield member */
    volatile unsigned short* bf_ptr = (volatile unsigned short*)&local_bf.b;
    result += *bf_ptr;
    
    return result;
}

/* Function with combined patterns in single statements */
static int combined_patterns(int idx, int shift_val) {
    /* Complex statement combining memory access, bit extraction, and type conversion */
    int val = (*(volatile short*)(&int_array[idx]) >> shift_val) & 0xFF;
    
    /* Nested operations */
    val += ((*(volatile int*)(&char_array[idx * 2]) << 4) & 0xF0) | 
           ((*(volatile char*)(&short_array[idx]) >> 2) & 0x0F);
    
    /* Pointer arithmetic with non-constant offset */
    volatile int* dyn_ptr = int_array + (idx * 3) / 2;
    val ^= *dyn_ptr;
    
    return val;
}

int main(int argc, char *argv[]) {
    int i, j;
    unsigned int checksum = 0;
    
    /* Initialize arrays with pseudo-random but deterministic values */
    for (i = 0; i < 32; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = (i * 1664525 + 1013904223) & 0x7FFF;
    }
    for (i = 0; i < 128; i++) {
        char_array[i] = (i * 134775813 + 1) & 0x7F;
    }
    
    /* Initialize bitfields */
    bf1.low_bits = 0xAA;
    bf1.mid_bits = 0xBBB;
    bf1.high_bits = 0xCCC;
    bf1.padding = 0xDEADBEEF;
    
    bf2.a = 0x5;
    bf2.b = 0x15;
    bf2.c = 0x25;
    bf2.d = 0xABCD;
    
    /* Use command line arguments to create dynamic values */
    int base_shift = (argc > 1) ? atoi(argv[1]) % 16 : 3;
    int base_index = (argc > 2) ? atoi(argv[2]) % 28 : 7;
    
    /* Main loop with opaque control flow */
    for (i = 0; i < 100; i++) {
        int dynamic_shift = (base_shift + i) % 24;
        int dynamic_mask = 0xFF >> (i % 8);
        int array_idx = (base_index + i * 3) % 28;
        
        /* Branch with external-dependent condition */
        if (argc > 1 && argv[1][0] % 2) {
            /* Pattern 1: Bitfield operations */
            checksum += bitfield_operations(dynamic_shift, dynamic_mask);
            
            /* Access bitfield through complex pointer expression */
            volatile struct bitfield_struct* bf_ptr = &bf1;
            checksum ^= bf_ptr->low_bits;
            checksum += (bf_ptr->mid_bits >> 2) & 0xFF;
        } else {
            /* Pattern 2: Memory access with complex addressing */
            checksum += complex_memory_access(array_idx, i % 60);
            
            /* Structure pointer with offset calculation */
            volatile char* char_ptr = char_array + (array_idx * 2) % 120;
            checksum += *char_ptr;
        }
        
        /* Always execute combined patterns */
        checksum += combined_patterns(i % 24, dynamic_shift % 12);
        
        /* Additional SUBREG generation through type mixing */
        if (i % 3 == 0) {
            long temp = global_long + i;
            short s_val = (short)temp;  /* SUBREG potential */
            char c_val = (char)(temp >> 16);  /* Another SUBREG */
            checksum += s_val * c_val;
        }
        
        /* Memory indirect with pointer arithmetic */
        volatile int* indirect_ptr = &int_array[(checksum + i) % 32];
        checksum ^= *indirect_ptr;
        
        /* Bit extraction from memory location */
        checksum |= (*(volatile short*)indirect_ptr >> 4) & 0x0F;
    }
    
    /* Additional loop with different pattern */
    for (j = 0; j < 50; j++) {
        /* Create ZERO_EXTRACT from memory */
        volatile int* mem_loc = &int_array[j % 16];
        int extracted = (*mem_loc >> (j % 20)) & ((1 << (8 + (j % 8))) - 1);
        checksum += extracted;
        
        /* Type punning between different sizes */
        volatile long* long_view = (volatile long*)&short_array[j * 2 % 60];
        checksum ^= (int)(*long_view);  /* SUBREG from long to int */
        
        /* Bitfield struct member access via pointer */
        volatile unsigned int* bf_access = (volatile unsigned int*)&bf2;
        checksum += (*bf_access >> 8) & 0xFF;
    }
    
    printf("Final checksum: %u\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
