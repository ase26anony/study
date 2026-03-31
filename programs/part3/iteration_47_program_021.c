#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0x42;
volatile long global_long = 0x9876543210ABCDEF;

/* Global arrays for memory access patterns */
volatile int arr_int[32];
volatile short arr_short[64];
volatile char arr_char[128];

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
    /* This should generate MEM_P with complex address */
    return arr_int[idx1 * 3 + idx2 * 7 - 5];
}

/* Function to force SUBREG generation through type conversions */
static short type_punning_conversion(int val) {
    /* Multiple type conversions to force SUBREG operations */
    char c = (char)(val >> 8);
    short s = (short)(c * 3);
    int i = (int)(s + 0x100);
    short s2 = (short)(i & 0xFFFF);
    return s2;
}

/* Function for bitfield extraction patterns */
static unsigned extract_bits(volatile int *ptr, int shift, int mask) {
    /* This may generate ZERO_EXTRACT RTL */
    return (*ptr >> shift) & mask;
}

int main(int argc, char **argv) {
    int i, j;
    unsigned long checksum = 0;
    
    /* Initialize arrays with pseudo-random but deterministic values */
    for (i = 0; i < 32; i++) {
        arr_int[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    for (i = 0; i < 64; i++) {
        arr_short[i] = (i * 1664525 + 1013904223) & 0xFFFF;
    }
    for (i = 0; i < 128; i++) {
        arr_char[i] = (i * 134775813 + 1) & 0xFF;
    }
    
    /* Use command line arguments to create dynamic indices */
    int base_idx = (argc > 1) ? atoi(argv[1]) % 16 : 5;
    int shift_val = (argc > 2) ? atoi(argv[2]) % 8 : 3;
    int mask_val = (argc > 3) ? atoi(argv[3]) % 255 : 0x1F;
    
    /* Loop with opaque control flow to stress resource analysis */
    for (i = 0; i < 100; i++) {
        int condition = i & 0x3;  /* Vary operations based on i */
        
        if (condition == 0) {
            /* Pattern 1: Bitfield extraction from memory with type conversion */
            /* May generate: ZERO_EXTRACT + MEM_P + SUBREG */
            volatile int *ptr = &arr_int[(i + base_idx) % 32];
            unsigned bits = extract_bits(ptr, shift_val + (i & 0x7), mask_val);
            short converted = type_punning_conversion(bits);
            checksum += converted;
            
            /* Access bitfield struct member through pointer */
            unsigned int *bf_ptr = (unsigned int*)&bf_struct.a;
            unsigned bf_val = (*bf_ptr >> (i & 0x3)) & 0xF;
            checksum ^= bf_val;
        }
        else if (condition == 1) {
            /* Pattern 2: Complex memory addressing with bit manipulation */
            /* May generate: MEM_P with complex address + ZERO_EXTRACT */
            int idx1 = (i * 7) % 32;
            int idx2 = (i * 13) % 32;
            int mem_val = complex_address(idx1, idx2);
            
            /* Extract bits using shifts - may become ZERO_EXTRACT */
            int extracted = (mem_val >> (shift_val * 2)) & ((1 << mask_val) - 1);
            
            /* Cast to different type to force SUBREG */
            char c_val = (char)extracted;
            checksum += c_val * i;
        }
        else if (condition == 2) {
            /* Pattern 3: Direct struct bitfield access and pointer arithmetic */
            /* May generate: STRICT_LOW_PART or ZERO_EXTRACT */
            struct bitfield_struct local_bf;
            local_bf.a = i & 0xF;
            local_bf.b = (i * 3) & 0xFF;
            local_bf.c = (i * 5) & 0xFFF;
            local_bf.d = (i * 7) & 0xFF;
            
            /* Take address of bitfield member */
            volatile unsigned int *bf_elem = (volatile unsigned int*)&local_bf.a;
            checksum += *bf_elem;
            
            /* Access array through pointer with offset */
            volatile short *sptr = &arr_short[(i * 11 + base_idx) % 64];
            short sval = *sptr;
            
            /* Type conversion chain for SUBREG */
            int ival = (int)sval;
            long lval = (long)ival;
            short sval2 = (short)(lval & 0xFFFF);
            checksum += sval2;
        }
        else {
            /* Pattern 4: Mixed operations in single expression */
            /* Complex expression that combines multiple patterns */
            checksum += (
                (extract_bits(&global_int, i & 0x1F, 0xFF) << 8) |
                (type_punning_conversion(arr_int[i % 32]) & 0xFF)
            );
            
            /* Memory access with complex index calculation */
            int complex_idx = ((i * 17 + base_idx) ^ shift_val) % 128;
            char c = arr_char[complex_idx];
            
            /* Bit manipulation on the result */
            int rotated = (c << 4) | (c >> 4);
            checksum += rotated & 0xFF;
        }
        
        /* Occasionally modify global variables to create side effects */
        if ((i % 13) == 0) {
            global_int ^= checksum & 0xFFFF;
            global_short += i & 0xFF;
        }
    }
    
    /* Additional test: Nested bitfield extraction */
    {
        /* Create a volatile pointer to force memory access */
        volatile int *volatile_ptr = &global_int;
        
        /* Complex expression: extract bits, convert, mask */
        unsigned nested = (
            ((*volatile_ptr >> 16) & 0xFF) |          /* May be ZERO_EXTRACT */
            (((int)global_short << 8) & 0xFF00)       /* Type conversion for SUBREG */
        );
        
        /* Access through cast to different pointer type */
        short *as_short = (short*)&nested;
        checksum += as_short[0] + as_short[1];        /* May create SUBREG for byte access */
    }
    
    /* Final checksum output to prevent dead code elimination */
    printf("Checksum: %lu\n", checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}
