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

/* Struct with bitfields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
} volatile bf_struct;

/* Another struct for type punning */
struct mixed_struct {
    char c;
    short s;
    int i;
    long l;
} volatile mixed;

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* This should generate MEM with complex address */
    return int_array[(idx1 * 3 + idx2 * 7) & 31];
}

/* Function to force SUBREG generation through type conversions */
static short type_punning_conversion(int val) {
    /* Multiple type conversions to generate SUBREGs */
    char c = (char)(val & 0xFF);
    short s = (short)((val >> 8) & 0xFFFF);
    int i = (int)(s * 2 + c);
    return (short)(i & 0xFFFF);
}

int main(int argc, char **argv) {
    int i, j;
    int checksum = 0;
    
    /* Initialize arrays with pseudo-random but deterministic values */
    for (i = 0; i < 32; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = (short)((i * 214013 + 2531011) & 0x7FFF);
    }
    for (i = 0; i < 128; i++) {
        char_array[i] = (char)((i * 16381 + 15823) & 0x7F);
    }
    
    /* Use command line arguments to get dynamic values */
    int base_idx = (argc > 1) ? atoi(argv[1]) % 16 : 5;
    int shift_amount = (argc > 2) ? atoi(argv[2]) % 8 : 3;
    int loop_count = (argc > 3) ? atoi(argv[3]) % 100 + 10 : 20;
    
    /* Pattern 1: Bitfield operations that may generate ZERO_EXTRACT */
    for (i = 0; i < loop_count; i++) {
        /* Direct bitfield access - may generate ZERO_EXTRACT */
        unsigned int bits = bf_struct.b;
        checksum ^= bits << (i & 3);
        
        /* Explicit bit manipulation on volatile - may generate ZERO_EXTRACT */
        int extracted = (global_int >> (shift_amount + i)) & ((1 << 8) - 1);
        checksum += extracted * i;
        
        /* Combined bitfield and memory access */
        int val = (int_array[i & 31] >> (i % 16)) & 0xF;
        checksum |= val << (i % 16);
    }
    
    /* Pattern 2: Type punning and casting to generate SUBREG */
    for (i = 0; i < loop_count; i++) {
        /* Type punning through pointer casting */
        short s_val = *(volatile short*)((char*)&global_int + (i & 2));
        checksum += s_val * 3;
        
        /* Multiple type conversions */
        long temp = global_long + i;
        int int_part = (int)(temp & 0xFFFFFFFF);
        short short_part = (short)(int_part >> 16);
        char char_part = (char)(short_part & 0xFF);
        
        /* This chain of conversions should generate SUBREGs */
        checksum += type_punning_conversion(int_part + char_part);
        
        /* Access struct members of different types */
        mixed.i = int_array[i & 15];
        mixed.s = short_array[i & 31];
        checksum ^= *(volatile int*)(&mixed.s);  /* Type punning */
    }
    
    /* Pattern 3: Complex memory addressing modes */
    for (i = 0; i < loop_count; i++) {
        for (j = 0; j < 4; j++) {
            /* Array access with complex index calculation */
            int idx = (base_idx + i * 7 + j * 11) & 31;
            checksum += complex_address(i, j);
            
            /* Pointer arithmetic with multiple variables */
            volatile char *ptr = char_array + (i * 3 + j * 5) & 127;
            checksum += *ptr * (i + j);
            
            /* Nested array access */
            checksum += short_array[int_array[idx] & 63] & 0xFF;
        }
    }
    
    /* Pattern 4: Combined operations in single statements */
    /* These complex expressions may generate nested RTL patterns */
    for (i = 0; i < loop_count; i++) {
        /* Extract bits from memory location and assign to different type */
        int combined = (*(volatile short*)((char*)&int_array[i & 31] + 1) >> (i % 8)) & 0x1F;
        checksum ^= combined;
        
        /* Bitfield extraction from struct pointer */
        struct bitfield_struct *bf_ptr = (struct bitfield_struct*)&int_array[0];
        int field_val = ((volatile struct bitfield_struct*)bf_ptr)->c;
        checksum += field_val * i;
        
        /* Memory access with bit extraction in one expression */
        checksum += ((int_array[(i * 13) & 31] >> 4) & 0xFF) | 
                   ((short_array[(i * 17) & 63] << 8) & 0xFF00);
    }
    
    /* Pattern 5: Conditional execution based on external input */
    /* This creates different control flow paths */
    if (argc > 1) {
        /* More aggressive bitfield operations */
        for (i = 0; i < loop_count; i++) {
            /* STRICT_LOW_PART might be generated here */
            unsigned int mask = (1 << (8 + (i % 4))) - 1;
            int masked_val = global_int & mask;
            checksum += masked_val;
            
            /* Access different bit ranges based on condition */
            if (i % 3 == 0) {
                checksum += (global_long >> 16) & 0xFFFF;
            } else if (i % 3 == 1) {
                checksum += (global_long >> 32) & 0xFFFFFFFF;
            } else {
                checksum += (global_long >> 48) & 0xFFFF;
            }
        }
    }
    
    /* Final output to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}
