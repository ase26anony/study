#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0xEF;
volatile long global_long = 0x9ABCDEF0;

/* Global arrays for memory access patterns */
volatile int arr_int[16];
volatile short arr_short[32];
volatile char arr_char[64];

/* Bitfield structs to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
};

struct mixed_bitfields {
    unsigned short low : 6;
    unsigned short mid : 5;
    unsigned short high : 5;
    unsigned char extra : 4;
    unsigned int padding : 12;
};

/* Global bitfield instances */
volatile struct bitfield_struct bf1;
volatile struct mixed_bitfields bf2;

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* This should generate MEM with complex address */
    return arr_int[(idx1 * 3 + idx2 * 7) & 0xF];
}

/* Function to force SUBREG through type conversions */
static short type_punning_conversion(int value) {
    /* Multiple type conversions to force SUBREG */
    char c = (char)(value >> 8);
    short s = (short)(c * 3);
    int i = (int)s + (value & 0xFF);
    return (short)(i & 0xFFFF);
}

/* Function for bitfield extraction patterns */
static unsigned extract_bits(volatile int *ptr, int shift, int mask) {
    /* Explicit bit manipulation - may generate ZERO_EXTRACT */
    return ((*ptr) >> shift) & mask;
}

/* Function using struct bitfield address taking */
static int bitfield_address_ops(void) {
    int result = 0;
    
    /* Take address of bitfield member - may generate complex RTL */
    volatile unsigned int *p1 = (volatile unsigned int*)&bf1.b;
    volatile unsigned short *p2 = (volatile unsigned short*)&bf2.mid;
    
    result = *p1 + (*p2 << 4);
    
    /* More bitfield access */
    result |= (bf1.a << 16);
    result |= (bf2.high << 24);
    
    return result;
}

int main(int argc, char **argv) {
    int i, j;
    unsigned long checksum = 0;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < 16; i++) {
        arr_int[i] = (i * 0x12345 + 0x6789) & 0xFFFF;
    }
    for (i = 0; i < 32; i++) {
        arr_short[i] = (i * 0xABCD + 0x1234) & 0xFFFF;
    }
    for (i = 0; i < 64; i++) {
        arr_char[i] = (i * 0x37 + 0x5A) & 0xFF;
    }
    
    /* Initialize bitfields */
    bf1.a = (argc > 1) ? (argv[1][0] & 0xF) : 5;
    bf1.b = (argc > 2) ? (argv[2][0] & 0xFF) : 0xAB;
    bf1.c = (argc > 3) ? (atoi(argv[3]) & 0xFFF) : 0x123;
    bf1.d = (argc > 4) ? (argv[4][0] & 0xFF) : 0xCD;
    
    bf2.low = (argc > 5) ? (argv[5][0] & 0x3F) : 0x12;
    bf2.mid = (argc > 6) ? (argv[6][0] & 0x1F) : 0x0A;
    bf2.high = (argc > 7) ? (argv[7][0] & 0x1F) : 0x15;
    bf2.extra = (argc > 8) ? (argv[8][0] & 0x0F) : 0x8;
    bf2.padding = 0xABC;
    
    /* Use command line arguments to create dynamic indices */
    int base_idx = (argc > 0) ? (argv[0][0] & 0x7) : 0;
    int shift_var = (argc > 1) ? (argv[1][0] & 0x7) : 3;
    int mask_var = (argc > 2) ? (argv[2][0] & 0x1F) : 0x1F;
    
    /* Main loop combining all patterns */
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 4; j++) {
            /* Pattern 1: Memory access with complex addressing */
            int idx1 = (base_idx + i) & 0xF;
            int idx2 = (base_idx + j * 2) & 0xF;
            
            /* Complex array access - should generate MEM with address expression */
            volatile int mem_val = arr_int[(idx1 * 5 + idx2 * 3) & 0xF];
            checksum += mem_val;
            
            /* Pattern 2: Bitfield extraction from memory */
            /* This may generate ZERO_EXTRACT */
            unsigned extracted = extract_bits(&mem_val, shift_var + j, mask_var);
            checksum += extracted;
            
            /* Pattern 3: Type punning with pointer casting - may generate SUBREG */
            volatile short *short_ptr = (volatile short*)&mem_val;
            volatile short short_val = *short_ptr;
            checksum += short_val;
            
            /* Pattern 4: More complex type conversion chain */
            short converted = type_punning_conversion(mem_val + i);
            checksum += converted;
            
            /* Pattern 5: Bitfield struct operations */
            int bf_result = bitfield_address_ops();
            checksum += bf_result;
            
            /* Pattern 6: Direct bitfield access with shifting */
            /* May generate STRICT_LOW_PART or ZERO_EXTRACT */
            int bf_bits = (bf1.b << 4) | (bf2.mid << 12);
            checksum += bf_bits;
            
            /* Pattern 7: Mixed-size memory accesses */
            /* Access different sized elements from arrays */
            volatile char char_val = arr_char[(i * 8 + j) & 0x3F];
            volatile short short_arr_val = arr_short[(i * 4 + j) & 0x1F];
            
            /* Combine with type conversion - may generate SUBREG */
            int combined = (int)char_val + ((int)short_arr_val << 8);
            checksum += combined;
            
            /* Pattern 8: Complex expression combining everything */
            /* Memory access + bit extraction + type conversion */
            int complex_expr = ((arr_int[idx1] >> (j * 2)) & 0x3) + 
                              (*(volatile short*)((char*)arr_char + i * 4) & 0xFF);
            checksum += complex_expr;
        }
        
        /* Modify shift and mask based on loop to create variation */
        shift_var = (shift_var + 1) & 0x7;
        mask_var = (mask_var << 1) | 0x1;
        if (mask_var > 0x3F) mask_var = 0x1F;
    }
    
    /* Additional test: Nested bitfield operations in conditional */
    if (argc > 3) {
        /* Access bitfield through pointer with offset */
        volatile char *byte_ptr = (volatile char*)&bf1;
        for (int k = 0; k < 4; k++) {
            /* May generate SUBREG for byte access */
            checksum += byte_ptr[k];
        }
    }
    
    /* Final checksum output */
    printf("Checksum: %lu\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
