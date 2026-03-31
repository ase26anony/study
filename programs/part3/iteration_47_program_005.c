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
    int a : 4;
    int b : 8;
    int c : 12;
    int d : 8;
};

struct mixed_bitfields {
    short low : 5;
    short mid : 6;
    short high : 5;
    int wide : 20;
    char tiny : 3;
};

/* Global struct instances */
volatile struct bitfield_struct bf1;
volatile struct mixed_bitfields bf2;

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* Force non-constant address computation */
    volatile int *ptr;
    
    /* Array indexing with variable offset */
    ptr = &int_array[(idx1 * 7 + idx2 * 3) % 32];
    
    /* Access through pointer with arithmetic */
    return *(ptr + ((idx1 ^ idx2) & 3));
}

/* Function to generate ZERO_EXTRACT patterns */
static int extract_bits(volatile int source, int shift, int mask) {
    /* Explicit bit extraction - may generate ZERO_EXTRACT */
    return (source >> shift) & mask;
}

/* Function to generate SUBREG patterns through type punning */
static short type_pun_int_to_short(volatile int val) {
    /* Cast between different-sized types - may generate SUBREG */
    volatile short *sp = (volatile short*)&val;
    return sp[(val >> 16) & 1];  /* Choose based on value */
}

int main(int argc, char **argv) {
    int i, j;
    int checksum = 0;
    
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
    
    /* Use command line arguments to get dynamic values */
    int base_shift = (argc > 1) ? atoi(argv[1]) % 16 : 3;
    int base_index = (argc > 2) ? atoi(argv[2]) % 8 : 5;
    
    /* Pattern 1: Bitfield operations that may generate ZERO_EXTRACT/STRICT_LOW_PART */
    for (i = 0; i < 16; i++) {
        volatile struct bitfield_struct local_bf;
        
        /* Direct bitfield assignment */
        local_bf.a = (global_int >> (i * 2)) & 0xF;
        local_bf.b = (global_short + i) & 0xFF;
        local_bf.c = extract_bits(global_long, i, 0xFFF);
        
        /* Take address of bitfield member - may generate complex RTL */
        volatile int *bp = (volatile int*)&local_bf.b;
        checksum += *bp;
        
        /* Update global bitfield */
        bf1.c = local_bf.a + local_bf.b;
        checksum += bf1.c;
    }
    
    /* Pattern 2: Mixed bitfield access with type conversions */
    for (i = 0; i < 8; i++) {
        /* Access bitfields through different type views */
        bf2.low = global_char & 0x1F;
        bf2.mid = (global_short >> (i * 2)) & 0x3F;
        bf2.high = (global_int >> (24 - i * 3)) & 0x1F;
        
        /* Type punning on bitfield struct */
        volatile int *ip = (volatile int*)&bf2;
        checksum += (*ip >> 8) & 0xFFFF;  /* Extract middle bits */
    }
    
    /* Pattern 3: Memory accesses with complex addressing */
    for (i = 0; i < 12; i++) {
        for (j = 0; j < 8; j++) {
            /* Complex array indexing */
            int idx = (i * 11 + j * 7 + base_index) & 31;
            volatile int *mem_ptr = &int_array[idx];
            
            /* Nested memory access with bit extraction */
            int val = (*mem_ptr >> (base_shift + j)) & 0xFF;
            
            /* Further memory access using computed value */
            checksum += short_array[val & 63];
            
            /* Pointer arithmetic with type conversion */
            volatile char *cp = (volatile char*)mem_ptr + j;
            checksum += *cp;
        }
    }
    
    /* Pattern 4: SUBREG generation through type mixing */
    for (i = 0; i < 20; i++) {
        /* Mix types in expressions */
        short s_val = type_pun_int_to_short(global_int + i);
        char c_val = (char)(s_val >> (i & 7));
        
        /* Cast between types in memory access */
        volatile int *int_ptr = (volatile int*)&short_array[i * 2];
        checksum += *(volatile short*)int_ptr;  /* SUBREG likely here */
        
        /* Combine with bit extraction */
        checksum += (c_val << 8) | (s_val & 0xFF);
    }
    
    /* Pattern 5: Combined operation in single statement */
    /* This may generate nested RTL with ZERO_EXTRACT, SUBREG, and MEM */
    for (i = 0; i < 10; i++) {
        /* Complex one-liner combining multiple patterns */
        int combined = 
            ((*(volatile short*)((volatile char*)&global_long + i) >> 
              (base_shift & 7)) & 0x1F) +
            (type_pun_int_to_short(complex_address(i, base_index)) & 0xFF);
        
        checksum += combined;
        
        /* Another combined pattern with bitfield */
        struct mixed_bitfields temp;
        temp.wide = complex_address(i, i + 2);
        checksum += (temp.wide >> 4) & 0xFFF;
    }
    
    /* Pattern 6: Conditional access to different RTL patterns */
    for (i = 0; i < 32; i++) {
        if (checksum & (1 << (i & 7))) {
            /* Bit extraction path */
            checksum += extract_bits(int_array[i], i & 15, 0xFF);
        } else {
            /* Type punning path */
            checksum += type_pun_int_to_short(short_array[i * 2]);
        }
        
        /* Always do a complex memory access */
        checksum += char_array[(checksum + i) & 127];
    }
    
    printf("Final checksum: %d\n", checksum);
    return checksum & 0xFF;
}
