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

/* Bitfield structs to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int low_bits : 8;
    unsigned int middle_bits : 16;
    unsigned int high_bits : 8;
    volatile unsigned int padding;
};

struct nested_bitfield {
    struct {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 8;
    } inner;
    unsigned int d : 16;
};

/* Global bitfield instances */
volatile struct bitfield_struct bf1;
volatile struct nested_bitfield bf2;

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* This should generate MEM_P with complex address */
    return int_array[idx1 * 3 + idx2 * 7 - 5];
}

/* Function to force SUBREG generation through type conversions */
static short type_punning_conversion(int value) {
    /* Multiple type conversions to force SUBREG */
    char c = (char)(value >> 8);
    short s = (short)(c * 3);
    int i = (int)s + 0x100;
    return (short)(i & 0xFFFF);
}

/* Function for bitfield extraction patterns */
static unsigned int extract_bits(volatile unsigned int source, 
                                 int shift, int width) {
    /* Explicit bit manipulation - may generate ZERO_EXTRACT */
    unsigned int mask = (1u << width) - 1;
    return (source >> shift) & mask;
}

int main(int argc, char **argv) {
    /* Use command line arguments for dynamic values */
    int base_idx = argc > 1 ? atoi(argv[1]) % 16 : 5;
    int shift_amount = argc > 2 ? atoi(argv[2]) % 24 : 3;
    int loop_count = argc > 3 ? atoi(argv[3]) % 8 + 4 : 8;
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 32; i++) {
        int_array[i] = global_int ^ (i * 0x11111111);
    }
    for (int i = 0; i < 64; i++) {
        short_array[i] = global_short + i;
    }
    for (int i = 0; i < 128; i++) {
        char_array[i] = global_char ^ i;
    }
    
    /* Initialize bitfields */
    bf1.low_bits = 0xAA;
    bf1.middle_bits = 0xBBCC;
    bf1.high_bits = 0xDD;
    bf1.padding = 0xEEFF0011;
    
    bf2.inner.a = 0x5;
    bf2.inner.b = 0xA;
    bf2.inner.c = 0xBC;
    bf2.d = 0xDEF0;
    
    unsigned int checksum = 0;
    
    /* Main loop combining all patterns */
    for (int i = 0; i < loop_count; i++) {
        int dynamic_idx = (base_idx + i * 3) & 31;
        int dynamic_shift = (shift_amount + i) & 31;
        
        /* PATTERN 1: Memory access with complex addressing (for MEM_P) */
        volatile int *mem_ptr;
        if (i & 1) {
            /* Array indexing with computation */
            mem_ptr = &int_array[dynamic_idx * 2 + 1];
        } else {
            /* Pointer arithmetic */
            mem_ptr = (volatile int *)((char *)int_array + dynamic_idx * sizeof(int) * 3);
        }
        int mem_val = *mem_ptr;
        checksum ^= mem_val;
        
        /* PATTERN 2: Bitfield extraction (for ZERO_EXTRACT/STRICT_LOW_PART) */
        unsigned int extracted;
        if (i & 2) {
            /* Direct bitfield access */
            extracted = bf1.middle_bits;
            
            /* Take address of bitfield member - may generate complex RTL */
            volatile unsigned int *bf_ptr = &bf1.padding;
            extracted ^= (*bf_ptr >> 8) & 0xFF;  /* ZERO_EXTRACT pattern */
        } else {
            /* Explicit bit manipulation */
            extracted = extract_bits(global_long, dynamic_shift, 12);
            
            /* Nested extraction */
            extracted |= (bf2.d >> 4) & 0x0F0F;
        }
        checksum += extracted;
        
        /* PATTERN 3: Type punning and conversions (for SUBREG) */
        short converted;
        if (i & 4) {
            /* Pointer casting between types */
            volatile int *int_ptr = &int_array[dynamic_idx];
            converted = *(volatile short *)int_ptr;  /* Likely SUBREG */
        } else {
            /* Multiple conversions */
            converted = type_punning_conversion(mem_val + i);
        }
        checksum ^= (unsigned int)converted * 0x10001;
        
        /* PATTERN 4: Combined operation - memory bitfield extraction */
        /* This single statement combines multiple patterns */
        int combined = (*(volatile short *)((char *)short_array + dynamic_idx * 2) >> 
                       (dynamic_shift & 7)) & 0x1F;
        checksum += combined * 3;
        
        /* PATTERN 5: Struct member access via computed pointer */
        struct bitfield_struct *bf_ptr = (struct bitfield_struct *)&bf1;
        volatile unsigned int *volatile_ptr = &bf_ptr->padding;
        int struct_val = *volatile_ptr;
        checksum ^= struct_val >> 16;
        
        /* PATTERN 6: Complex expression with all elements */
        if (dynamic_idx > 16) {
            /* Nested expression: memory → extract → convert */
            long temp = global_long ^ int_array[dynamic_idx - 16];
            int result = (int)((temp >> (dynamic_shift * 2)) & 0xFFFF);
            result = (short)result + (char)(temp >> 24);  /* Mixed-type ops */
            checksum += result;
        }
    }
    
    /* Additional standalone patterns outside loop */
    
    /* Direct bitfield address taking */
    volatile unsigned int *low_bits_ptr = (volatile unsigned int *)&bf1.low_bits;
    checksum ^= *low_bits_ptr;
    
    /* Complex memory indirection */
    volatile int **indirect_ptr = (volatile int **)&int_array[8];
    checksum += **indirect_ptr;
    
    /* Mixed-size operations */
    long mixed = (long)global_int * (short)global_short;
    checksum ^= (mixed >> 32) & 0xFFFFFFFF;
    
    printf("Checksum: %u\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
