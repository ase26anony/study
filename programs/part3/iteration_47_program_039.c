#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variables to prevent optimization */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0x42;

/* Global array for memory access patterns */
volatile int global_array[32];

/* Bitfield structs for ZERO_EXTRACT/STRICT_LOW_PART generation */
struct bitfield_struct {
    volatile unsigned int low_bits : 8;
    volatile unsigned int mid_bits : 12;
    volatile unsigned int high_bits : 12;
};

struct mixed_bitfields {
    volatile unsigned short part1 : 4;
    volatile unsigned short part2 : 6;
    volatile unsigned short part3 : 6;
};

/* Global struct instances */
struct bitfield_struct bf_global;
struct mixed_bitfields mixed_bf;

/* Function to create complex addressing modes */
static inline volatile int* complex_address(int offset1, int offset2) {
    return &global_array[(offset1 * 3 + offset2 * 7) & 31];
}

/* Function to force SUBREG generation through type punning */
static short type_pun_int_to_short(volatile int* src) {
    /* This should generate SUBREG when accessing partial register */
    return *(volatile short*)src;
}

/* Function to create ZERO_EXTRACT patterns */
static unsigned extract_bits(volatile unsigned value, int shift, int width) {
    /* Explicit bit extraction - may generate ZERO_EXTRACT */
    return (value >> shift) & ((1u << width) - 1);
}

int main(int argc, char *argv[]) {
    int i, j;
    unsigned int checksum = 0;
    
    /* Initialize global array with pattern */
    for (i = 0; i < 32; i++) {
        global_array[i] = i * 0x12345 + 0x6789;
    }
    
    /* Initialize bitfield structs */
    bf_global.low_bits = 0xAB;
    bf_global.mid_bits = 0xCDE;
    bf_global.high_bits = 0xF12;
    
    mixed_bf.part1 = 0x5;
    mixed_bf.part2 = 0x2A;
    mixed_bf.part3 = 0x1F;
    
    /* Use command line arguments to create runtime variability */
    int base_shift = (argc > 1) ? atoi(argv[1]) % 16 : 3;
    int base_index = (argc > 2) ? atoi(argv[2]) % 16 : 5;
    
    /* Main loop with complex RTL generation patterns */
    for (i = 0; i < 100; i++) {
        volatile int temp;
        
        /* Pattern 1: Memory access with complex addressing (for MEM_P) */
        volatile int* mem_ptr = complex_address(i & 7, base_index);
        temp = *mem_ptr;
        checksum += temp;
        
        /* Pattern 2: Bitfield extraction from memory (ZERO_EXTRACT potential) */
        if (i & 1) {
            /* Access bitfield through pointer - may generate ZERO_EXTRACT */
            volatile unsigned int* bf_ptr = (volatile unsigned int*)&bf_global;
            unsigned extracted = extract_bits(*bf_ptr, (i + base_shift) & 7, 5);
            checksum ^= extracted;
            
            /* Another bitfield access pattern */
            extracted = (global_int >> ((i * 3) & 31)) & 0xFF;
            checksum += extracted;
        }
        
        /* Pattern 3: Type punning for SUBREG generation */
        if (i & 2) {
            /* Cast between different sized types */
            short s_val = type_pun_int_to_short(&global_int);
            checksum += s_val;
            
            /* Direct cast with volatile */
            char c_val = *(volatile char*)(&global_array[i & 31]);
            checksum += c_val;
        }
        
        /* Pattern 4: Combined operation - memory access with bit extraction */
        if (i & 4) {
            /* Complex expression combining multiple patterns */
            unsigned val = (*(volatile short*)(&global_array[(i + base_index) & 31]) 
                          >> (base_shift & 7)) & 0x1F;
            checksum += val;
        }
        
        /* Pattern 5: Struct bitfield member access via pointer */
        if (i & 8) {
            /* Take address of bitfield member */
            volatile unsigned short* part_ptr = &mixed_bf.part2;
            /* Dereference and shift - may create interesting RTL */
            unsigned part_val = (*part_ptr << 2) & 0x3F;
            checksum += part_val;
        }
        
        /* Pattern 6: Nested extractions and type conversions */
        if (i & 16) {
            /* Multi-step extraction with type changes */
            int int_val = global_array[i & 31];
            short short_val = (short)((int_val >> 8) & 0xFFFF);
            char char_val = (char)((short_val >> 4) & 0x0F);
            checksum += char_val;
        }
    }
    
    /* Additional complex one-liner combining multiple patterns */
    for (j = 0; j < 50; j++) {
        /* This statement has memory access, bit extraction, and type conversion */
        checksum += (*(volatile short*)(&global_array[(j * 2) & 31]) 
                    >> ((j + base_shift) & 7)) 
                    & ((1 << ((j % 5) + 1)) - 1);
    }
    
    /* Force STRICT_LOW_PART pattern through bitfield assignment */
    {
        struct {
            volatile unsigned int full : 32;
        } s;
        volatile unsigned int* ptr = (volatile unsigned int*)&s;
        
        /* Multiple assignments to same bitfield */
        for (i = 0; i < 10; i++) {
            s.full = (checksum + i) & 0xFFFF;
            checksum += s.full;
        }
    }
    
    printf("Final checksum: %u\n", checksum);
    return checksum & 0xFF;
}
