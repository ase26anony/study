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

/* Bitfield structs for ZERO_EXTRACT/STRICT_LOW_PART generation */
struct bitfield_struct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
};

struct mixed_bitfields {
    unsigned short low : 5;
    unsigned short mid : 6;
    unsigned short high : 5;
    unsigned char extra : 4;
};

/* Global bitfield instances */
volatile struct bitfield_struct bf_global;
volatile struct mixed_bitfields mixed_bf;

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* Force non-trivial addressing: arr[(idx1*idx2) % size] */
    return (idx1 * idx2) & 31;
}

/* Function to extract bits using various methods */
static unsigned extract_bits(volatile unsigned source, int start, int length) {
    /* Multiple extraction patterns that may generate ZERO_EXTRACT */
    unsigned mask = (1u << length) - 1;
    
    /* Pattern 1: Direct shift and mask */
    unsigned result1 = (source >> start) & mask;
    
    /* Pattern 2: Using bitfield struct (may generate different RTL) */
    struct local_bf {
        unsigned value;
    };
    union {
        struct local_bf bf;
        unsigned raw;
    } u;
    u.raw = source;
    
    /* Simulate bitfield access through pointer */
    volatile unsigned *ptr = &u.raw;
    unsigned result2 = ((*ptr) >> start) & mask;
    
    return result1 + result2;
}

int main(int argc, char **argv) {
    int i, j;
    unsigned long checksum = 0;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 32; i++) {
        arr_int[i] = i * 0x11111111;
    }
    for (i = 0; i < 64; i++) {
        arr_short[i] = i * 0x1111;
    }
    for (i = 0; i < 128; i++) {
        arr_char[i] = i * 0x11;
    }
    
    /* Initialize bitfields */
    bf_global.a = 0xF;
    bf_global.b = 0xAA;
    bf_global.c = 0xABC;
    bf_global.d = 0x77;
    
    mixed_bf.low = 0x1F;
    mixed_bf.mid = 0x2A;
    mixed_bf.high = 0x15;
    mixed_bf.extra = 0x7;
    
    /* Use command line arguments to create runtime variability */
    int base_idx = (argc > 1) ? atoi(argv[1]) % 16 : 3;
    int shift_amount = (argc > 2) ? atoi(argv[2]) % 8 : 2;
    
    /* Main loop combining all patterns */
    for (i = 0; i < 100; i++) {
        volatile int loop_var = i;
        
        /* PATTERN 1: Memory access with complex addressing */
        /* This should generate MEM_P(x) with non-trivial address */
        int idx = complex_address(loop_var, base_idx);
        volatile int mem_val = arr_int[idx + shift_amount];
        checksum += mem_val;
        
        /* PATTERN 2: Bitfield extraction from memory */
        /* May generate ZERO_EXTRACT from memory location */
        volatile short *short_ptr = (volatile short *)&arr_int[idx];
        unsigned short extracted = (*short_ptr >> shift_amount) & 0x1F;
        checksum += extracted;
        
        /* PATTERN 3: Type punning with different sizes - may generate SUBREG */
        /* Access different-sized views of the same memory */
        volatile char *char_view = (volatile char *)&arr_int[idx];
        for (j = 0; j < 4; j++) {
            volatile int char_as_int = char_view[j];  /* char to int conversion */
            checksum += char_as_int;
        }
        
        /* PATTERN 4: Direct bitfield struct access */
        /* May generate STRICT_LOW_PART or ZERO_EXTRACT */
        checksum += bf_global.b;
        checksum += bf_global.c;
        
        /* PATTERN 5: Mixed bitfield access with pointer */
        volatile unsigned short *bf_ptr = (volatile unsigned short *)&mixed_bf;
        unsigned short bf_bits = *bf_ptr;
        
        /* Extract different bit ranges - may generate multiple ZERO_EXTRACT */
        unsigned low_bits = (bf_bits >> 0) & 0x1F;
        unsigned mid_bits = (bf_bits >> 5) & 0x3F;
        unsigned high_bits = (bf_bits >> 11) & 0x1F;
        
        checksum += low_bits + mid_bits + high_bits;
        
        /* PATTERN 6: Complex expression combining multiple patterns */
        /* Memory access + bit extraction + type conversion */
        if (loop_var & 1) {
            /* Access as short, extract bits, convert to int */
            volatile int complex_val = 
                ((*(volatile short *)(&arr_char[loop_var * 2])) >> 
                 (shift_amount & 3)) & 0xF;
            checksum += complex_val;
        }
        
        /* PATTERN 7: Pointer arithmetic with struct */
        struct nested {
            int x;
            short y[4];
            char z;
        };
        
        volatile struct nested nested_arr[8];
        for (j = 0; j < 8; j++) {
            nested_arr[j].x = j * 100;
            nested_arr[j].y[0] = j * 10;
            nested_arr[j].z = j;
        }
        
        /* Complex struct member access with variable index */
        int struct_idx = (loop_var * base_idx) & 7;
        volatile short *y_ptr = &nested_arr[struct_idx].y[shift_amount & 3];
        checksum += *y_ptr;
        
        /* PATTERN 8: Inline assembly to force SUBREG usage */
        /* Access partial registers */
        {
            volatile int temp = loop_var * 0x1234;
            volatile short temp_short;
            
            /* Force conversion int -> short (may generate SUBREG) */
            temp_short = temp & 0xFFFF;
            checksum += temp_short;
            
            /* Force conversion short -> int (may generate SUBREG) */
            volatile int temp_int = temp_short;
            checksum += temp_int;
        }
    }
    
    /* Additional patterns outside loop */
    
    /* PATTERN 9: Extract from global with variable shift */
    volatile unsigned global_bits = (global_int >> (base_idx * 2)) & 0xFF;
    checksum += global_bits;
    
    /* PATTERN 10: Chain of extractions */
    volatile long long_val = global_long;
    volatile int int_val = (long_val >> 16) & 0xFFFFFFFF;
    volatile short short_val = (int_val >> 8) & 0xFFFF;
    volatile char char_val = (short_val >> 4) & 0xFF;
    
    checksum += char_val + short_val + int_val + (long_val & 0xFFFF);
    
    /* PATTERN 11: Pointer to bitfield member (tricky but may work) */
    volatile unsigned int *a_ptr = (volatile unsigned int *)&bf_global.a;
    checksum += *a_ptr & 0xF;
    
    printf("Checksum: %lu\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
