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

/* Bitfield structs to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int field1 : 4;
    unsigned int field2 : 8;
    unsigned int field3 : 12;
    unsigned int field4 : 8;
};

struct mixed_bitfields {
    unsigned short low : 5;
    unsigned short mid : 6;
    unsigned short high : 5;
    unsigned char extra : 4;
};

/* Global bitfield instances */
volatile struct bitfield_struct bf1;
volatile struct mixed_bitfields bf2;

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* Force non-constant address computation */
    volatile int *ptr;
    
    /* Array indexing with variable offset - may generate MEM with complex address */
    ptr = &int_array[(idx1 * 3 + idx2 * 7) & 31];
    return *ptr;
}

/* Function to perform bitfield extraction */
static unsigned extract_bits(volatile unsigned int value, int shift, int width) {
    /* Explicit bit manipulation - may generate ZERO_EXTRACT */
    return (value >> shift) & ((1 << width) - 1);
}

/* Function with type punning for SUBREG generation */
static short type_punning_conversion(volatile int *src) {
    /* Cast between different-sized types - may generate SUBREG */
    volatile short *short_ptr = (volatile short *)src;
    return *short_ptr;
}

/* Function to combine multiple patterns */
static int combined_operation(int index, int shift) {
    int result = 0;
    
    /* 1. Memory access with complex addressing */
    volatile int *mem_ptr = &int_array[(index * 5 + shift * 3) & 31];
    
    /* 2. Type conversion through pointer casting - may generate SUBREG */
    volatile short *short_view = (volatile short *)mem_ptr;
    
    /* 3. Bit extraction from memory - may generate ZERO_EXTRACT from MEM */
    result = (*short_view >> (shift & 7)) & 0x1F;
    
    /* 4. Additional bitfield access */
    result |= (bf2.mid << 5);
    
    return result;
}

int main(int argc, char **argv) {
    int i, j;
    unsigned long checksum = 0;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < 32; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = (i * 1664525 + 1013904223) & 0xFFFF;
    }
    for (i = 0; i < 128; i++) {
        char_array[i] = (i * 214013 + 2531011) & 0xFF;
    }
    
    /* Initialize bitfields */
    bf1.field1 = 0xA;
    bf1.field2 = 0xBC;
    bf1.field3 = 0xDEF;
    bf1.field4 = 0x12;
    
    bf2.low = 0x1F;
    bf2.mid = 0x2A;
    bf2.high = 0x15;
    bf2.extra = 0x7;
    
    /* Use command-line arguments to create runtime variability */
    int base_shift = (argc > 1) ? atoi(argv[1]) % 16 : 3;
    int base_index = (argc > 2) ? atoi(argv[2]) % 32 : 7;
    
    /* Main loop with complex operations */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 10; j++) {
            int temp = 0;
            
            /* Pattern 1: Direct bitfield extraction from struct */
            /* May generate ZERO_EXTRACT or STRICT_LOW_PART */
            volatile unsigned int *bf_ptr;
            
            /* Take address of bitfield member - compiler may use bitfield extraction */
            bf_ptr = (volatile unsigned int*)&bf1.field2;
            temp = *bf_ptr & 0xFF;  /* Access through pointer */
            
            /* Pattern 2: Complex memory access with type conversion */
            /* May generate SUBREG when accessing different-sized types */
            if ((i + j) & 1) {
                volatile int *int_ptr = &int_array[(i + base_index) & 31];
                short short_val = type_punning_conversion((int *)int_ptr);
                temp ^= short_val;
            }
            
            /* Pattern 3: Combined operation with memory, bit extraction, and type punning */
            int combined = combined_operation(i + base_index, j + base_shift);
            temp += combined;
            
            /* Pattern 4: Explicit bit manipulation on volatile */
            /* May generate ZERO_EXTRACT */
            volatile unsigned int bit_source = global_int + i;
            unsigned extracted = extract_bits(bit_source, (j * 3) & 31, 5);
            temp |= extracted << 10;
            
            /* Pattern 5: Array access with complex index calculation */
            /* Creates MEM with non-trivial address */
            int idx = (i * 11 + j * 13 + base_index) & 63;
            volatile short *arr_ptr = &short_array[idx];
            temp ^= *arr_ptr;
            
            /* Pattern 6: Struct pointer with offset calculation */
            /* More complex addressing mode */
            volatile char *char_ptr = &char_array[(i * 17 + j * 19) & 127];
            temp += *char_ptr;
            
            /* Pattern 7: Direct bitfield assignment with masking */
            /* May generate STRICT_LOW_PART */
            if ((i ^ j) & 4) {
                /* Simulate bitfield store */
                unsigned int mask = (1 << 8) - 1;
                unsigned int value = (temp & mask);
                /* Force compiler to consider this as bitfield store */
                volatile unsigned int *target = (volatile unsigned int*)&bf2.mid;
                *target = value;  /* May be implemented as bitfield insert */
            }
            
            /* Accumulate checksum to prevent dead code elimination */
            checksum = (checksum * 31 + temp) & 0xFFFFFFFF;
        }
    }
    
    /* Additional test: Nested bitfield operations in loop */
    for (i = 0; i < 50; i++) {
        /* Access bitfield through pointer with offset */
        volatile struct mixed_bitfields *bf_ptr = &bf2;
        
        /* Chain of operations that may create complex RTL */
        unsigned val = bf_ptr->low;
        val |= (bf_ptr->mid << 5);
        val |= (bf_ptr->high << 11);
        val |= (bf_ptr->extra << 16);
        
        /* Memory access with the value */
        int_array[i & 31] ^= val;
        
        checksum += val;
    }
    
    /* Final output to ensure side effects */
    printf("Checksum: %lu\n", checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}
