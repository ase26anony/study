#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    unsigned int low8 : 8;
    unsigned int mid16 : 16;
    unsigned int high8 : 8;
    unsigned int extra : 24;
    unsigned int last16 : 16;
};

struct mixed_bitfields {
    signed int signed_field : 10;
    unsigned int unsigned_field : 22;
    short short_field : 12;
    char char_field : 4;
    unsigned long long_field : 20;
};

/* Global bitfield structs */
volatile struct bitfield_struct bf1;
volatile struct mixed_bitfields bf2;

/* Function to create complex addressing modes */
static int complex_memory_access(int index, int offset) {
    /* Create MEM_P with complex address: array + variable index + offset */
    return int_array[(index * 3 + offset) & 31];
}

/* Function to force SUBREG generation through type punning */
static short type_punning_short(int value) {
    /* This should generate SUBREG when accessing partial register */
    volatile int temp = value;
    return *(volatile short*)(&temp);
}

/* Function to force SUBREG with different integer sizes */
static char type_punning_char(long value) {
    /* Access different sized portions of a larger type */
    volatile long temp = value;
    return *(volatile char*)((char*)&temp + (value & 3));
}

/* Function for bitfield extraction (ZERO_EXTRACT pattern) */
static unsigned extract_bits(volatile unsigned int value, int shift, int width) {
    /* Explicit bit manipulation that may generate ZERO_EXTRACT */
    return (value >> shift) & ((1 << width) - 1);
}

/* Function combining multiple patterns in one statement */
static int combined_operation(volatile int *mem_ptr, int idx, int shift) {
    /* Complex statement: memory access + bit extraction + type conversion */
    int val = (*(volatile short*)((char*)mem_ptr + idx) >> shift) & 0x3F;
    return val;
}

/* Initialize arrays with pseudo-random but deterministic values */
static void init_arrays(void) {
    for (int i = 0; i < 32; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    for (int i = 0; i < 64; i++) {
        short_array[i] = (i * 1664525 + 1013904223) & 0x7FFF;
    }
    for (int i = 0; i < 128; i++) {
        char_array[i] = (i * 134775813 + 1) & 0x7F;
    }
    
    /* Initialize bitfield structs */
    bf1.low8 = 0xAA;
    bf1.mid16 = 0xBBCC;
    bf1.high8 = 0xDD;
    bf1.extra = 0xEEFF00;
    bf1.last16 = 0x1234;
    
    bf2.signed_field = -512;
    bf2.unsigned_field = 0x3FFFFF;
    bf2.short_field = 0x7FF;
    bf2.char_field = 0xF;
    bf2.long_field = 0xFFFFF;
}

int main(int argc, char **argv) {
    int checksum = 0;
    int i, j;
    
    /* Use argc to get dynamic values for indices */
    int base_index = argc > 1 ? atoi(argv[1]) % 16 : 5;
    int shift_amount = argc > 2 ? atoi(argv[2]) % 8 : 3;
    
    init_arrays();
    
    /* Loop with opaque control flow to stress resource analysis */
    for (i = 0; i < 100; i++) {
        int condition = i & 0xF;
        
        /* Pattern 1: Bitfield struct access (ZERO_EXTRACT/STRICT_LOW_PART) */
        if (condition < 4) {
            /* Access bitfield members - may generate ZERO_EXTRACT */
            unsigned int val1 = bf1.mid16;
            unsigned int val2 = bf1.low8;
            checksum += val1 - val2;
            
            /* Take address of bitfield member */
            volatile unsigned int *ptr = &bf1.mid16;
            checksum += *ptr;
        }
        
        /* Pattern 2: Explicit bit manipulation (ZERO_EXTRACT) */
        if (condition < 8) {
            /* Extract bits from volatile global */
            unsigned extracted = extract_bits(global_int, (i + shift_amount) & 0x1F, 5);
            checksum += extracted;
            
            /* Another bit extraction pattern */
            unsigned mask = (1 << ((i % 7) + 1)) - 1;
            unsigned bits = (global_long >> (i % 32)) & mask;
            checksum += bits;
        }
        
        /* Pattern 3: Type punning (SUBREG generation) */
        if (condition < 12) {
            /* Convert between different integer sizes */
            short sval = type_punning_short(global_int + i);
            checksum += sval;
            
            /* More type punning */
            char cval = type_punning_char(global_long ^ i);
            checksum += cval;
            
            /* Direct cast between different pointer types */
            int int_from_short = *(volatile int*)(&global_short);
            checksum += int_from_short & 0xFF;
        }
        
        /* Pattern 4: Complex memory addressing (MEM_P with address computation) */
        if (condition > 2) {
            /* Array access with variable index */
            int idx = (base_index + i * 7) & 31;
            int mem_val = complex_memory_access(idx, shift_amount);
            checksum += mem_val;
            
            /* Structure pointer with offset */
            volatile char *byte_ptr = (volatile char*)int_array;
            byte_ptr += (i * 13) & 127;
            checksum += *byte_ptr;
        }
        
        /* Pattern 5: Combined operation in single statement */
        if ((i % 3) == 0) {
            int combined = combined_operation((volatile int*)short_array, 
                                            (i * 5) & 63, 
                                            (i + shift_amount) & 7);
            checksum += combined;
        }
        
        /* Pattern 6: Mixed bitfield and memory operations */
        if ((i % 5) == 0) {
            /* Access bitfield, then use result for memory access */
            unsigned field_val = bf2.unsigned_field;
            int array_idx = field_val & 31;
            checksum += int_array[array_idx];
            
            /* Modify bitfield based on memory value */
            bf2.short_field = short_array[i & 63] & 0xFFF;
        }
    }
    
    /* Additional complex expressions outside loop */
    
    /* Nested extractions: extract bits from a memory location that itself
       comes from array access with variable index */
    int outer_idx = (checksum ^ base_index) & 31;
    volatile int *mem_loc = &int_array[outer_idx];
    unsigned nested_bits = (*(volatile short*)mem_loc >> shift_amount) & 0x1F;
    checksum += nested_bits;
    
    /* Pointer chain with type conversions */
    volatile void *ptr_chain = &global_int;
    volatile short *short_ptr = (volatile short*)ptr_chain;
    volatile char *char_ptr = (volatile char*)(short_ptr + 1);
    checksum += *char_ptr;
    
    /* Final print to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}
