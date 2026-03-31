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

/* Bitfield structs for ZERO_EXTRACT/STRICT_LOW_PART generation */
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

/* Global struct instances */
volatile struct bitfield_struct bf1;
volatile struct mixed_bitfields bf2;

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* Force non-trivial addressing: array[idx1 + idx2 * 2] */
    return int_array[idx1 + idx2 * 2];
}

/* Function to perform bitfield extraction with type conversion */
static short extract_and_convert(volatile struct bitfield_struct *s, int shift) {
    /* Combined operation: extract bits, shift, convert type */
    return (short)((s->field2 >> shift) & 0x3F);
}

int main(int argc, char **argv) {
    int i, j;
    int checksum = 0;
    
    /* Use command line arguments to get dynamic values */
    int base_idx = (argc > 1) ? atoi(argv[1]) % 16 : 5;
    int shift_amount = (argc > 2) ? atoi(argv[2]) % 8 : 3;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 32; i++) {
        int_array[i] = i * 0x11111111;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = i * 0x1111;
    }
    for (i = 0; i < 128; i++) {
        char_array[i] = i * 0x11;
    }
    
    /* Initialize bitfield structs */
    bf1.field1 = 0xA;
    bf1.field2 = 0xBC;
    bf1.field3 = 0xDEF;
    bf1.field4 = 0x12;
    
    bf2.low = 0x1F;
    bf2.mid = 0x2A;
    bf2.high = 0x15;
    bf2.extra = 0x7;
    
    /* Main loop with opaque control flow */
    for (i = 0; i < 100; i++) {
        int idx = (base_idx + i) % 28;
        int alt_idx = (i * 7) % 32;
        
        /* Pattern 1: ZERO_EXTRACT from memory with type conversion */
        if (i % 3 == 0) {
            /* Access bitfield through pointer - may generate ZERO_EXTRACT */
            volatile unsigned int *ptr = (volatile unsigned int*)&bf1;
            unsigned int temp = *ptr;
            
            /* Explicit bit extraction - likely ZERO_EXTRACT in RTL */
            int extracted = (temp >> (i % 16)) & ((1 << (8 + (i % 8))) - 1);
            checksum += extracted;
            
            /* Another ZERO_EXTRACT pattern with struct member */
            checksum += bf1.field2;
        }
        
        /* Pattern 2: STRICT_LOW_PART simulation through bitfield assignment */
        if (i % 5 == 1) {
            /* Modify partial bitfield */
            bf2.mid = (bf2.mid + i) & 0x3F;  /* Only 6 bits */
            checksum += bf2.mid;
            
            /* Access through different type pointer */
            volatile short *short_ptr = (volatile short*)&bf2;
            short low_part = *short_ptr & 0x7FF;  /* Extract low 11 bits */
            checksum += low_part;
        }
        
        /* Pattern 3: SUBREG generation through type punning */
        if (i % 4 == 2) {
            /* Type conversions that may generate SUBREG */
            long big_val = global_long + i;
            int truncated = (int)big_val;  /* Potential SUBREG */
            short further = (short)truncated;  /* Another potential SUBREG */
            char smallest = (char)further;  /* Yet another */
            
            checksum += smallest;
            
            /* Pointer casting between different sizes */
            volatile int *int_ptr = &global_int;
            volatile short *short_view = (volatile short*)int_ptr;
            short half = short_view[1];  /* Access high half of int */
            checksum += half;
        }
        
        /* Pattern 4: Complex memory addressing (MEM_P with address computation) */
        if (i % 6 == 3) {
            /* Array access with non-constant index computation */
            int val1 = int_array[idx * 2 + 1];
            int val2 = int_array[(idx ^ 0x0F) + alt_idx];
            
            /* Structure pointer with offset */
            volatile char *byte_ptr = (volatile char*)int_array;
            byte_ptr += idx * 3 + 1;
            char byte_val = *byte_ptr;
            
            checksum += val1 + val2 + byte_val;
        }
        
        /* Pattern 5: Combined operation - memory, extraction, conversion */
        if (i % 7 == 4) {
            /* Complex one-liner combining multiple patterns */
            int combined = ((*(volatile short*)(&int_array[alt_idx]) >> shift_amount) & 0x1F) 
                          + (char)(global_int >> ((i % 4) * 8));
            checksum += combined;
        }
        
        /* Pattern 6: Bitfield in memory with complex address */
        if (i % 8 == 5) {
            /* Create array of bitfield structs */
            static volatile struct bitfield_struct bf_array[8];
            static int init_done = 0;
            
            if (!init_done) {
                for (j = 0; j < 8; j++) {
                    bf_array[j].field1 = j;
                    bf_array[j].field2 = j * 0x11;
                    bf_array[j].field3 = j * 0x111;
                    bf_array[j].field4 = j * 0x1;
                }
                init_done = 1;
            }
            
            /* Access with computed index */
            int bf_idx = (idx >> 1) % 8;
            checksum += bf_array[bf_idx].field3;
            
            /* Take address of bitfield member */
            volatile unsigned int *field_ptr = &bf_array[bf_idx].field4;
            checksum += *field_ptr;
        }
    }
    
    /* Additional forced patterns outside loop */
    
    /* Force ZERO_EXTRACT with volatile */
    volatile int extract_src = 0xFEDCBA98;
    for (j = 0; j < 4; j++) {
        int bits = (extract_src >> (j * 8)) & 0xFF;
        checksum += bits;
    }
    
    /* Force SUBREG with mixed operations */
    {
        volatile long long big = 0x123456789ABCDEF0LL;
        volatile int *as_int = (volatile int*)&big;
        volatile short *as_short = (volatile short*)&big;
        volatile char *as_char = (volatile char*)&big;
        
        checksum += *as_int;
        checksum += as_short[1];
        checksum += as_char[3];
    }
    
    /* Print result to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}
