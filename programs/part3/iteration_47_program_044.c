#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0xEF;
volatile long global_long = 0x9ABCDEF012345678L;

/* Global arrays for memory access patterns */
volatile int arr_int[32];
volatile short arr_short[64];
volatile char arr_char[128];

/* Bitfield structs to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
};

struct mixed_bitfields {
    unsigned short low : 6;
    unsigned short high : 10;
    unsigned char extra : 4;
};

/* Global struct instances */
volatile struct bitfield_struct bf1;
volatile struct mixed_bitfields bf2;

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* Force non-constant address computation */
    return (idx1 * 3 + idx2 * 7) & 0x1F;
}

/* Function to extract bits in various ways */
static unsigned extract_bits(volatile unsigned value, int shift, int width) {
    /* Multiple extraction patterns that may generate ZERO_EXTRACT */
    unsigned result = 0;
    
    /* Pattern 1: Direct shift and mask */
    result |= (value >> shift) & ((1 << width) - 1);
    
    /* Pattern 2: Via bitfield struct */
    struct local_bf {
        unsigned field : 16;
    };
    volatile struct local_bf lbf;
    lbf.field = value;
    result ^= lbf.field;
    
    return result;
}

int main(int argc, char *argv[]) {
    int i, j;
    unsigned int checksum = 0;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 32; i++) {
        arr_int[i] = i * 0x13579BDF;
    }
    for (i = 0; i < 64; i++) {
        arr_short[i] = i * 0x2468;
    }
    for (i = 0; i < 128; i++) {
        arr_char[i] = i * 0x37;
    }
    
    /* Use command line arguments to create dynamic values */
    int base_idx = (argc > 1) ? atoi(argv[1]) % 16 : 5;
    int shift_val = (argc > 2) ? atoi(argv[2]) % 8 : 3;
    int width_val = (argc > 3) ? (atoi(argv[3]) % 12) + 1 : 7;
    
    /* Loop with opaque control flow */
    for (i = 0; i < 100; i++) {
        /* Dynamic condition based on external input */
        if ((i + base_idx) % 3 == 0) {
            /* Pattern 1: ZERO_EXTRACT from memory with complex address */
            int idx = complex_address(i, base_idx);
            
            /* Access memory with variable index - may generate MEM with complex address */
            volatile int mem_val = arr_int[idx];
            
            /* Extract bits - may generate ZERO_EXTRACT */
            unsigned extracted = extract_bits(mem_val, shift_val, width_val);
            checksum += extracted;
            
            /* Type conversion through pointer casting - may generate SUBREG */
            volatile short *short_ptr = (volatile short *)&mem_val;
            volatile short short_val = *short_ptr;
            checksum ^= short_val;
            
            /* Access bitfield through pointer */
            volatile unsigned *bf_ptr = (volatile unsigned *)&bf1.b;
            checksum += *bf_ptr;
        }
        
        if ((i + shift_val) % 5 == 0) {
            /* Pattern 2: STRICT_LOW_PART simulation through bitfield assignment */
            struct bitfield_struct local_bf;
            
            /* Assign to bitfield member - may generate STRICT_LOW_PART */
            local_bf.b = (global_int >> (i % 16)) & 0xFF;
            local_bf.c = (global_short * i) & 0xFFF;
            
            checksum += local_bf.b + local_bf.c;
            
            /* Mixed type access with pointer arithmetic */
            volatile char *char_ptr = arr_char + complex_address(i, shift_val);
            volatile int char_as_int = *(volatile int *)char_ptr;  /* Type punning */
            checksum ^= char_as_int & 0xFFFFFF;
        }
        
        if ((i + width_val) % 7 == 0) {
            /* Pattern 3: SUBREG generation through type conversions */
            
            /* Convert between different integer sizes */
            volatile long long_val = global_long + i;
            volatile int int_val = (volatile int)long_val;  /* Potential SUBREG */
            volatile short short_val = (volatile short)int_val;  /* Another potential SUBREG */
            volatile char char_val = (volatile char)short_val;
            
            checksum += int_val + short_val + char_val;
            
            /* Pointer-based type punning */
            volatile int *int_ptr = (volatile int *)&global_short;
            volatile int pun_val = *int_ptr;  /* May involve SUBREG due to size mismatch */
            checksum ^= pun_val;
        }
        
        /* Complex combined expression in single statement */
        if (i % 11 == 0) {
            /* This combines memory access, bit extraction, and type conversion */
            int idx1 = complex_address(i, base_idx);
            int idx2 = complex_address(i, shift_val);
            
            /* Complex expression that may generate nested RTL patterns */
            unsigned val = 
                ((*(volatile unsigned short *)(arr_char + idx1) >> (idx2 % 8)) & 0x3F) +
                ((*(volatile unsigned *)(&arr_short[idx2]) & 0x7FF) << 3) -
                (*(volatile char *)(&global_int + (i % 4)) & 0x1F);
            
            checksum += val;
        }
        
        /* Update bitfield structs */
        bf1.a = (checksum >> 0) & 0xF;
        bf1.b = (checksum >> 4) & 0xFF;
        bf1.c = (checksum >> 12) & 0xFFF;
        bf1.d = (checksum >> 24) & 0xFF;
        
        bf2.low = checksum & 0x3F;
        bf2.high = (checksum >> 6) & 0x3FF;
        bf2.extra = (checksum >> 16) & 0xF;
    }
    
    /* Final computation using all patterns one more time */
    for (j = 0; j < 10; j++) {
        /* Memory reference with complex address computation */
        int offset = complex_address(j, checksum & 0xF);
        volatile int *mem_ptr = arr_int + offset;
        volatile int mem_value = *mem_ptr;
        
        /* Bit extraction from the loaded value */
        unsigned bits = extract_bits(mem_value, j % 16, 8 + (j % 8));
        
        /* Type conversion chain */
        volatile long temp_long = mem_value + bits;
        volatile int temp_int = (volatile int)temp_long;
        volatile short temp_short = (volatile short)(temp_int >> (j % 16));
        
        checksum = (checksum * 0x1234567) + temp_short + (bits & 0xFF);
    }
    
    printf("Final checksum: %u\n", checksum);
    return checksum & 0x7F;  /* Return non-zero exit code based on result */
}
