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

/* Bitfield structs to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int field1 : 4;
    unsigned int field2 : 8;
    unsigned int field3 : 12;
    unsigned int field4 : 8;
};

struct mixed_bitfield {
    unsigned short low : 6;
    unsigned short mid : 5;
    unsigned short high : 5;
    unsigned char extra : 4;
};

/* Global bitfield instances */
volatile struct bitfield_struct bf1;
volatile struct mixed_bitfield bf2;

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* Force non-trivial addressing: array[idx1 + idx2 * 2] */
    return int_array[idx1 + idx2 * 2];
}

/* Function to extract bits using various methods */
static unsigned extract_bits(volatile unsigned int value, int shift, int width) {
    /* Multiple extraction patterns that may generate ZERO_EXTRACT */
    unsigned result = 0;
    
    /* Pattern 1: Direct shift and mask */
    result |= (value >> shift) & ((1 << width) - 1);
    
    /* Pattern 2: Via bitfield struct */
    struct local_bf {
        unsigned int data : 16;
        unsigned int rest : 16;
    };
    volatile struct local_bf *bf_ptr = (volatile struct local_bf *)&value;
    result |= bf_ptr->data;
    
    return result;
}

int main(int argc, char **argv) {
    /* Use argc to create runtime-variable indices/shifts */
    int base_idx = argc > 1 ? atoi(argv[1]) % 16 : 5;
    int shift_amount = argc > 2 ? atoi(argv[2]) % 8 : 3;
    int loop_count = argc > 3 ? atoi(argv[3]) % 10 + 5 : 8;
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 32; i++) {
        int_array[i] = i * 0x11111111;
    }
    for (int i = 0; i < 64; i++) {
        short_array[i] = i * 0x1111;
    }
    for (int i = 0; i < 128; i++) {
        char_array[i] = i * 0x11;
    }
    
    /* Initialize bitfields */
    bf1.field1 = 0xA;
    bf1.field2 = 0xBC;
    bf1.field3 = 0xDEF;
    bf1.field4 = 0x12;
    
    bf2.low = 0x1F;
    bf2.mid = 0x0A;
    bf2.high = 0x15;
    bf2.extra = 0x7;
    
    unsigned long checksum = 0;
    
    /* Main loop with complex RTL-generating patterns */
    for (int i = 0; i < loop_count; i++) {
        volatile int temp;
        
        /* PATTERN 1: ZERO_EXTRACT from memory with bitfield struct */
        /* Taking address of bitfield member may generate ZERO_EXTRACT */
        volatile unsigned int *bf_ptr = (volatile unsigned int*)&bf1.field2;
        temp = *bf_ptr;  /* Potential ZERO_EXTRACT of bitfield */
        checksum += temp;
        
        /* PATTERN 2: STRICT_LOW_PART via type punning */
        /* Cast between different-sized types may generate SUBREG/STRICT_LOW_PART */
        volatile short *short_ptr = (volatile short*)&global_int;
        temp = *short_ptr;  /* Reads only low 16 bits */
        checksum += temp;
        
        /* PATTERN 3: Complex memory access with non-constant address */
        /* This should generate MEM with complex address expression */
        int idx = (base_idx + i) & 31;
        temp = int_array[idx * 2 + (i % 4)];  /* Non-trivial addressing */
        checksum += temp;
        
        /* PATTERN 4: Combined bit extraction and memory access */
        /* Extract bits from memory location with type conversion */
        volatile int mem_val = int_array[i % 16];
        unsigned extracted = (mem_val >> shift_amount) & 0x3F;  /* May generate ZERO_EXTRACT */
        checksum += extracted;
        
        /* PATTERN 5: SUBREG generation via mixed-type operations */
        /* Operations on different integer sizes often generate SUBREG */
        volatile char c = char_array[i * 3 % 128];
        volatile short s = short_array[i * 2 % 64];
        volatile int combined = (c << 8) | s;  /* Mixing char and short */
        checksum += combined;
        
        /* PATTERN 6: Pointer arithmetic with type casting */
        /* Complex address calculation with pointer casting */
        volatile int *ptr = int_array + (i * 7) % 16;
        volatile char *cptr = (volatile char *)ptr;
        temp = cptr[1] + cptr[3];  /* Accesses different bytes of int */
        checksum += temp;
        
        /* PATTERN 7: Bitfield extraction from global with shift */
        /* Direct bit manipulation on volatile */
        temp = (global_int >> (i % 24)) & ((1 << ((i % 8) + 1)) - 1);
        checksum += temp;
        
        /* PATTERN 8: Nested extractions and memory accesses */
        /* Combine multiple patterns in one expression */
        temp = ((*(volatile short*)(&int_array[i % 8]) >> (shift_amount % 4)) & 0xF) 
               + (bf2.mid << 2);
        checksum += temp;
        
        /* Modify shift amount based on loop to create variation */
        shift_amount = (shift_amount + 1) % 7;
    }
    
    /* Additional complex patterns outside loop */
    
    /* Pattern with multiple SUBREGs: mixing 8, 16, 32, 64-bit types */
    volatile long long_result = global_long;
    volatile int int_result = (int)long_result;  /* Potential SUBREG */
    volatile short short_result = (short)int_result;  /* Another SUBREG */
    volatile char char_result = (char)short_result;  /* Another SUBREG */
    
    checksum += int_result + short_result + char_result;
    
    /* Complex bitfield extraction from struct pointer */
    struct bitfield_struct *bf_ptr = (struct bitfield_struct *)&global_int;
    checksum += bf_ptr->field1 + bf_ptr->field3;
    
    /* Final output to prevent dead code elimination */
    printf("Checksum: %lu\n", checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}
