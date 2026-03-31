#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0x42;
volatile long global_long = 0x9876543210ABCDEF;

/* Global arrays for memory access patterns */
volatile int int_array[100];
volatile short short_array[100];
volatile char char_array[100];

/* Bitfield structs to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 8;
    unsigned int field3 : 3;
    unsigned int field4 : 16;
};

struct mixed_bitfields {
    signed int sfield : 10;
    unsigned int ufield : 6;
    int normal_field;
};

/* Global bitfield instances */
volatile struct bitfield_struct global_bf;
volatile struct mixed_bitfields global_mixed_bf;

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* This should generate MEM with complex address */
    return int_array[idx1 * 3 + idx2 * 7];
}

/* Function to force SUBREG generation through type conversions */
static short type_punning_conversion(int val) {
    /* Multiple type conversions to force SUBREG */
    char c = (char)(val >> 8);
    short s = (short)(c * 3);
    int i = (int)s + 0x100;
    return (short)(i & 0xFFFF);
}

/* Function for bitfield extraction patterns */
static unsigned int extract_bits(volatile int *ptr, int shift, int mask) {
    /* This may generate ZERO_EXTRACT */
    return (*ptr >> shift) & mask;
}

/* Function that combines multiple patterns */
static int combined_operation(int index, int shift) {
    int result = 0;
    
    /* 1. Memory access with complex addressing - triggers MEM_P path */
    volatile int *mem_ptr = &int_array[index * 2 + (shift & 7)];
    result += *mem_ptr;
    
    /* 2. Bitfield extraction - may generate ZERO_EXTRACT */
    result += extract_bits(mem_ptr, shift, 0x1F);
    
    /* 3. Type punning with pointer casting - may generate SUBREG */
    volatile short *short_ptr = (volatile short *)mem_ptr;
    result += *short_ptr;
    
    /* 4. Direct bitfield struct access */
    result += global_bf.field2;
    result += global_bf.field4 >> 4;
    
    return result;
}

int main(int argc, char **argv) {
    int i, j;
    int checksum = 0;
    int loop_limit = 10;
    
    /* Use command line arguments to get dynamic values */
    if (argc > 1) {
        loop_limit = atoi(argv[1]);
        if (loop_limit <= 0 || loop_limit > 50) loop_limit = 10;
    }
    
    /* Initialize arrays with pseudo-random but deterministic values */
    for (i = 0; i < 100; i++) {
        int_array[i] = i * 3 + 7;
        short_array[i] = i * 5 - 3;
        char_array[i] = i & 0xFF;
    }
    
    /* Initialize bitfields */
    global_bf.field1 = 0x1F;
    global_bf.field2 = 0xAB;
    global_bf.field3 = 0x7;
    global_bf.field4 = 0xDEAD;
    
    global_mixed_bf.sfield = -512;
    global_mixed_bf.ufield = 0x3F;
    global_mixed_bf.normal_field = 0x123456;
    
    /* Main loop with complex operations */
    for (i = 0; i < loop_limit; i++) {
        for (j = 0; j < 5; j++) {
            int idx = (i * 7 + j * 13) % 50;
            int shift = (i + j * 3) & 0x1F;
            
            /* Pattern 1: Complex memory access with type conversion */
            volatile int *ptr1 = &int_array[idx];
            volatile short *ptr2 = (volatile short *)ptr1;
            volatile char *ptr3 = (volatile char *)ptr1;
            
            /* This may generate SUBREG for type conversion */
            short s_val = *ptr2;
            char c_val = *ptr3;
            
            /* Combine with bit extraction */
            int extracted = (*ptr1 >> shift) & ((1 << (shift % 8 + 1)) - 1);
            checksum += s_val + c_val + extracted;
            
            /* Pattern 2: Bitfield operations on struct members */
            /* Taking address of bitfield may generate interesting RTL */
            checksum += global_bf.field2;
            checksum -= global_bf.field4;
            
            /* Pattern 3: More complex addressing with arithmetic */
            int complex_idx = (idx * 3 + shift * 7) % 50;
            checksum += complex_address(idx, complex_idx);
            
            /* Pattern 4: Combined operation */
            checksum += combined_operation(idx, shift);
            
            /* Pattern 5: Direct manipulation with shifts and masks */
            /* May generate ZERO_EXTRACT in RTL */
            volatile long temp = global_long;
            int low_part = (temp >> (shift * 2)) & 0xFFFF;
            int high_part = (temp >> (shift * 2 + 16)) & 0xFFFF;
            checksum += low_part - high_part;
            
            /* Pattern 6: Pointer arithmetic with different types */
            volatile char *char_base = char_array;
            volatile int *int_from_char = (volatile int *)(char_base + idx * 2);
            checksum += *int_from_char & 0xFF;
            
            /* Pattern 7: Access bitfield through pointer */
            struct mixed_bitfields *bf_ptr = (struct mixed_bitfields *)&global_mixed_bf;
            checksum += bf_ptr->sfield;
            checksum += bf_ptr->ufield << 4;
        }
        
        /* Alternate between different operation patterns based on loop index */
        if (i & 1) {
            /* Use global variables directly */
            checksum += (global_int >> (i & 0xF)) & 0xF;
            checksum += type_punning_conversion(global_int + i);
        } else {
            /* Array operations */
            checksum += short_array[i % 50] * 3;
            checksum += (int_array[i % 50] >> 4) & 0xFFF;
        }
    }
    
    /* Final bit-twiddling to use all results */
    checksum = (checksum & 0xFFFF) + (checksum >> 16);
    checksum ^= global_bf.field1;
    checksum ^= global_bf.field3 << 8;
    
    printf("Final checksum: %d\n", checksum);
    return checksum & 0xFF;
}
