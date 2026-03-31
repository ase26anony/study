#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0x42;
volatile long global_long = 0x9876543210ABCDEF;

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
    unsigned short a : 4;
    unsigned short b : 12;
    unsigned int c : 10;
    unsigned int d : 22;
};

/* Global bitfield structs */
volatile struct bitfield_struct bf1;
volatile struct mixed_bitfields bf2;

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* Force non-constant address computation */
    return (idx1 * 7 + idx2 * 13) & 31;
}

/* Function to extract bits in various ways */
static unsigned extract_bits(volatile unsigned *src, int shift, int width) {
    /* This may generate ZERO_EXTRACT */
    return (*src >> shift) & ((1 << width) - 1);
}

/* Function with type punning for SUBREG generation */
static short type_pun_int_to_short(volatile int *src) {
    /* Casting between types may generate SUBREG */
    return *(volatile short *)src;
}

static char type_pun_short_to_char(volatile short *src) {
    return *(volatile char *)src;
}

int main(int argc, char **argv) {
    int i, j;
    unsigned int checksum = 0;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 32; i++) {
        int_array[i] = i * 0x01010101;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = i * 0x1111;
    }
    for (i = 0; i < 128; i++) {
        char_array[i] = i;
    }
    
    /* Initialize bitfield structs */
    bf1.low8 = 0xAA;
    bf1.mid16 = 0xBBBB;
    bf1.high8 = 0xCC;
    bf1.extra = 0xDDDDDD;
    bf1.last16 = 0xEEEE;
    
    bf2.a = 0x5;
    bf2.b = 0xABC;
    bf2.c = 0x2FF;
    bf2.d = 0x3ABCDE;
    
    /* Use command line arguments to create runtime variability */
    int base_idx = (argc > 1) ? atoi(argv[1]) % 16 : 3;
    int shift_val = (argc > 2) ? atoi(argv[2]) % 24 : 7;
    int width_val = (argc > 3) ? (atoi(argv[3]) % 16) + 1 : 9;
    
    /* Main loop with complex operations */
    for (i = 0; i < 100; i++) {
        volatile int temp;
        
        /* Pattern 1: ZERO_EXTRACT from memory with variable shift/width */
        temp = extract_bits(&global_int, (shift_val + i) % 28, width_val);
        checksum += temp;
        
        /* Pattern 2: Direct bitfield access (may generate STRICT_LOW_PART) */
        checksum += bf1.mid16;
        checksum += bf2.b;
        
        /* Pattern 3: Type punning for SUBREG generation */
        if (i & 1) {
            short s = type_pun_int_to_short(&global_int);
            checksum += s;
        } else {
            char c = type_pun_short_to_char(&global_short);
            checksum += c;
        }
        
        /* Pattern 4: Complex memory addressing with array indexing */
        int idx1 = (base_idx + i) & 31;
        int idx2 = (i * 5) & 63;
        int addr_idx = complex_address(idx1, idx2);
        
        /* Access with non-trivial address computation */
        volatile int *mem_ptr = &int_array[addr_idx];
        checksum += *mem_ptr;
        
        /* Pattern 5: Combined operation - extract bits from memory location
           with type conversion (may create nested RTL) */
        int combined = (*(volatile short *)(&int_array[idx1]) >> (i % 8)) & 0x3F;
        checksum += combined;
        
        /* Pattern 6: Bitfield extraction from struct pointer */
        volatile struct bitfield_struct *bf_ptr = &bf1;
        unsigned int extracted = 0;
        
        /* Access different bitfields based on loop counter */
        switch (i % 4) {
            case 0:
                extracted = bf_ptr->low8;
                break;
            case 1:
                extracted = bf_ptr->mid16;
                break;
            case 2:
                extracted = bf_ptr->high8;
                break;
            case 3:
                extracted = bf_ptr->last16;
                break;
        }
        checksum += extracted;
        
        /* Pattern 7: Pointer arithmetic with different types */
        volatile char *char_ptr = (volatile char *)&global_long;
        char_ptr += i % 8;
        checksum += *char_ptr;
        
        /* Pattern 8: Multi-dimensional array access with variable indices */
        int matrix_idx = ((i * 11) % 8) * 8 + ((i * 7) % 8);
        volatile short *short_ptr = &short_array[matrix_idx];
        checksum += *short_ptr;
    }
    
    /* Additional complex statement combining multiple patterns */
    /* This single statement aims to create nested RTL expressions */
    int final_extract = ((*(volatile short *)(&int_array[base_idx]) >> shift_val) & 0xFF) + 
                        (bf1.low8 & 0x7F) +
                        (type_pun_int_to_short(&global_int) & 0xF);
    checksum += final_extract;
    
    /* Use argc to create conditional paths */
    if (argc > 4) {
        /* More complex bit manipulation when extra args provided */
        volatile long *long_ptr = (volatile long *)&int_array[0];
        checksum += (*long_ptr >> 16) & 0xFFFFFFFF;
    }
    
    printf("Checksum: %u\n", checksum);
    return checksum & 0xFF;
}
