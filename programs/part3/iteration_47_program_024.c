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

/* Struct with bitfields for ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    volatile unsigned int low_bits : 8;
    volatile unsigned int mid_bits : 16;
    volatile unsigned int high_bits : 8;
};

/* Global struct instance */
struct bitfield_struct bf_global;

/* Function to create complex addressing modes */
static inline volatile int* complex_address(int idx1, int idx2) {
    return &int_array[(idx1 * 7 + idx2 * 3) & 31];
}

/* Function to force SUBREG through type conversions */
static short type_pun_int_to_short(volatile int* ptr) {
    /* This should generate SUBREG when accessing partial register */
    return *(volatile short*)ptr;
}

int main(int argc, char** argv) {
    int i, j;
    int checksum = 0;
    
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
    
    /* Initialize bitfield struct */
    bf_global.low_bits = 0xAA;
    bf_global.mid_bits = 0xBBBB;
    bf_global.high_bits = 0xCC;
    
    /* Use command-line arguments to create dynamic indices */
    int base_idx = (argc > 1) ? atoi(argv[1]) % 16 : 5;
    int shift_val = (argc > 2) ? atoi(argv[2]) % 8 : 3;
    
    /* Main loop combining all patterns */
    for (i = 0; i < 100; i++) {
        int dynamic_idx = (base_idx + i) & 31;
        
        /* PATTERN 1: ZERO_EXTRACT through bitfield access */
        /* Access bitfield member - may generate ZERO_EXTRACT */
        unsigned int extracted = bf_global.mid_bits;
        checksum ^= extracted;
        
        /* PATTERN 2: Explicit bit manipulation for ZERO_EXTRACT */
        /* This should generate ZERO_EXTRACT RTL */
        int bitfield_val = (global_int >> shift_val) & 0x1F;
        checksum += bitfield_val;
        
        /* PATTERN 3: STRICT_LOW_PART through bitfield assignment */
        /* Assign to bitfield - may generate STRICT_LOW_PART */
        bf_global.low_bits = (i * 7) & 0xFF;
        checksum ^= bf_global.low_bits;
        
        /* PATTERN 4: SUBREG through type conversion */
        /* Convert between types - should generate SUBREG */
        short converted = type_pun_int_to_short(&global_int);
        checksum += converted;
        
        /* Additional SUBREG pattern with direct casting */
        char char_val = *(volatile char*)&global_short;
        checksum ^= char_val;
        
        /* PATTERN 5: Complex memory reference with addressing mode */
        /* Array access with non-trivial index computation */
        volatile int* mem_ptr = complex_address(dynamic_idx, i & 3);
        int mem_val = *mem_ptr;
        checksum += mem_val;
        
        /* More complex memory reference with pointer arithmetic */
        volatile short* short_ptr = &short_array[(dynamic_idx * 2 + 7) & 63];
        checksum ^= *short_ptr;
        
        /* PATTERN 6: Combined operation - extract bits from memory */
        /* This may create nested RTL expressions */
        int combined = (*(volatile short*)(&int_array[dynamic_idx]) >> shift_val) & 0xF;
        checksum += combined;
        
        /* PATTERN 7: Structure pointer with offset */
        struct bitfield_struct* bf_ptr = &bf_global;
        /* Taking address of bitfield member */
        volatile unsigned int* bitfield_addr = &bf_ptr->mid_bits;
        checksum ^= *bitfield_addr;
        
        /* PATTERN 8: Conditional access to create different RTL paths */
        if (i & 1) {
            /* Access through different type when odd */
            long long_val = *(volatile long*)&int_array[dynamic_idx];
            checksum += (int)(long_val & 0xFFFFFFFF);
        } else {
            /* Bit manipulation when even */
            int shifted = (global_long >> (i & 31)) & 0xFF;
            checksum ^= shifted;
        }
        
        /* PATTERN 9: Inline assembly to force partial register access */
        /* This often generates SUBREG in RTL */
        register int reg_var asm ("eax") = global_int;
        register short short_part asm ("ax");
        asm volatile ("" : "=r"(short_part) : "0"((short)reg_var));
        checksum += short_part;
    }
    
    /* Final computation to use all values */
    checksum = (checksum ^ global_int) + (int)global_long;
    
    printf("Result: %d\n", checksum);
    return checksum & 1;
}
