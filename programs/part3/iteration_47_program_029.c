#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0xEF;
volatile long global_long = 0x9ABCDEF0;

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
    unsigned short s1 : 4;
    unsigned short s2 : 12;
    unsigned int i1 : 10;
    unsigned int i2 : 22;
};

/* Global bitfield instances */
volatile struct bitfield_struct bf_global;
volatile struct mixed_bitfields mixed_bf;

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* Array access with non-constant offset - may generate MEM with complex address */
    return int_array[idx1 * 3 + idx2 * 7];
}

/* Function to force SUBREG generation through type conversions */
static short type_punning_conversion(int val) {
    /* Multiple type conversions to force SUBREG operations */
    char c = (char)(val >> 8);
    short s = (short)(c * 256 + (val & 0xFF));
    int i = (int)s * 2;
    /* Cast through volatile pointer to prevent optimization */
    return *(volatile short*)(&i);
}

/* Function for bitfield extraction patterns */
static unsigned extract_bits(volatile int* ptr, int shift, int width) {
    /* Explicit bit extraction - may generate ZERO_EXTRACT */
    return (*ptr >> shift) & ((1 << width) - 1);
}

int main(int argc, char **argv) {
    int i, j;
    unsigned int checksum = 0;
    
    /* Initialize arrays with pseudo-random but deterministic values */
    for (i = 0; i < 32; i++) {
        int_array[i] = i * 0x12345 + 0x6789;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = i * 0x2345 + 0x6789;
    }
    for (i = 0; i < 128; i++) {
        char_array[i] = i * 0x23 + 0x45;
    }
    
    /* Initialize bitfield structs */
    bf_global.low8 = 0xAA;
    bf_global.mid16 = 0xBBCC;
    bf_global.high8 = 0xDD;
    bf_global.extra = 0xEEFF00;
    bf_global.last16 = 0x1122;
    
    mixed_bf.s1 = 0x5;
    mixed_bf.s2 = 0xABC;
    mixed_bf.i1 = 0x1FF;
    mixed_bf.i2 = 0x2ABCDE;
    
    /* Use command line arguments to create dynamic indices */
    int base_idx = (argc > 1) ? atoi(argv[1]) % 16 : 5;
    int shift_val = (argc > 2) ? atoi(argv[2]) % 24 : 3;
    int width_val = (argc > 3) ? (atoi(argv[3]) % 16) + 1 : 8;
    
    /* Main loop with complex RTL generation patterns */
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 4; j++) {
            int dynamic_idx = base_idx + i * 2 + j * 3;
            
            /* PATTERN 1: ZERO_EXTRACT from memory with complex addressing */
            /* Access bitfield through pointer - may generate ZERO_EXTRACT */
            volatile unsigned int *bf_ptr = (volatile unsigned int*)&bf_global;
            unsigned int extracted = (*bf_ptr >> (shift_val + i)) & ((1 << width_val) - 1);
            checksum ^= extracted;
            
            /* PATTERN 2: STRICT_LOW_PART through bitfield assignment */
            /* Bitfield assignment may generate STRICT_LOW_PART */
            if (i & 1) {
                struct bitfield_struct local_bf;
                volatile struct bitfield_struct *bfp = &local_bf;
                /* Complex assignment to bitfield member */
                bfp->mid16 = (short_array[dynamic_idx % 64] >> j) & 0xFFFF;
                checksum += bfp->mid16;
            }
            
            /* PATTERN 3: SUBREG through type punning and casting */
            /* Multiple type conversions to force SUBREG generation */
            int temp_int = int_array[dynamic_idx % 32];
            short temp_short = type_punning_conversion(temp_int);
            char temp_char = (char)(temp_short >> 4);
            /* Cast through different pointer types */
            int reconstructed = *(volatile int*)(&temp_char) * 0x10101;
            checksum += reconstructed & 0xFF;
            
            /* PATTERN 4: Complex memory reference with addressing mode */
            /* Array access with computation in index - forces complex MEM address */
            volatile int *mem_ptr = &int_array[(dynamic_idx * 7 + j * 11) % 32];
            int mem_val = *mem_ptr;
            
            /* Combine with bit extraction */
            int combined = (mem_val >> (j * 2)) & 0xF;
            checksum ^= combined << (i * 4);
            
            /* PATTERN 5: Mixed operations in single expression */
            /* Complex expression combining multiple patterns */
            int complex_val = (
                (*(volatile short*)(&int_array[(i + j) % 32]) >> shift_val) & 0x1F
            ) + (
                (char_array[(dynamic_idx * 3) % 128] << j) & 0xFF
            );
            checksum += complex_val;
            
            /* PATTERN 6: Access bitfield through pointer with offset */
            /* May generate ZERO_EXTRACT for bitfield access */
            if (j & 1) {
                volatile unsigned short *mixed_ptr = 
                    (volatile unsigned short*)&mixed_bf;
                unsigned short mixed_val = mixed_ptr[1];  /* Access specific bitfield area */
                checksum ^= (mixed_val << 8) | (mixed_val >> 8);
            }
        }
    }
    
    /* Additional patterns outside loops */
    
    /* Pointer arithmetic with type conversion */
    volatile char *char_ptr = char_array + base_idx * 2;
    volatile int *aliased_int = (volatile int*)char_ptr;
    int aliased_val = *aliased_int & 0xFFFFFF;  /* May involve SUBREG */
    checksum += aliased_val;
    
    /* Nested bit extraction from memory */
    volatile long *long_ptr = (volatile long*)&int_array[0];
    long long_val = *long_ptr;
    for (i = 0; i < 4; i++) {
        /* Multiple ZERO_EXTRACT operations */
        int chunk = (long_val >> (i * 8)) & 0xFF;
        checksum ^= chunk * (i + 1);
    }
    
    /* Final output to prevent dead code elimination */
    printf("Checksum: 0x%08X\n", checksum & 0xFFFFFFFF);
    
    return (checksum & 0xFF);
}
