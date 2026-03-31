#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0xEF;
volatile long global_long = 0x9ABCDEF012345678L;

/* Global arrays for memory access patterns */
volatile int global_array[32];
volatile short short_array[64];
volatile char char_array[128];

/* Bitfield structs to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int low_bits : 8;
    unsigned int mid_bits : 12;
    unsigned int high_bits : 12;
    volatile unsigned int padding;
};

struct nested_bitfield {
    struct {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } nibbles;
    volatile short separator;
    struct {
        unsigned int x : 10;
        unsigned int y : 10;
        unsigned int z : 12;
    } chunks;
};

/* Global bitfield instances */
volatile struct bitfield_struct bf1;
volatile struct nested_bitfield bf2;

/* Function to create complex addressing modes */
static int complex_memory_access(int index, int offset) {
    /* Create SUBREG patterns through type conversions */
    short s_val = *(volatile short*)(&global_array[index % 32]);
    char c_val = *(volatile char*)(&short_array[(index + offset) % 64]);
    
    /* Combine with bit manipulation for ZERO_EXTRACT */
    int extracted = (s_val >> (offset & 0x7)) & 0x1F;
    extracted |= (c_val << 8) & 0xF00;
    
    return extracted;
}

/* Function to manipulate bitfields directly */
static unsigned int bitfield_operations(int mode) {
    unsigned int result = 0;
    
    /* Access bitfield members - may generate ZERO_EXTRACT */
    result |= bf1.low_bits;
    result |= (bf1.mid_bits << 8);
    result |= (bf1.high_bits << 20);
    
    /* Complex bitfield access based on mode */
    if (mode & 1) {
        /* Take address of bitfield member */
        volatile unsigned int* ptr = &bf1.low_bits;
        result ^= *ptr;
    }
    
    /* Nested bitfield access */
    result |= bf2.nibbles.a;
    result |= (bf2.nibbles.b << 4);
    result |= (bf2.nibbles.c << 8);
    result |= (bf2.nibbles.d << 12);
    
    /* More complex extraction */
    if (mode & 2) {
        unsigned int temp = bf2.chunks.x;
        result ^= (temp << 16);
        temp = bf2.chunks.y;
        result ^= (temp << 20);
    }
    
    return result;
}

/* Function with explicit bit manipulation */
static long explicit_bit_ops(long value, int shift) {
    /* Operations that may generate ZERO_EXTRACT */
    long masked = value & 0x00000000FFFFFFFFL;
    long shifted = (value >> shift) & 0xFF;
    long combined = (masked << 16) | shifted;
    
    /* Type conversions for SUBREG */
    int as_int = (int)combined;
    short as_short = (short)(combined >> 8);
    char as_char = (char)(combined >> 16);
    
    /* Combine all types */
    return (long)as_int + (as_short << 8) + (as_char << 16);
}

int main(int argc, char *argv[]) {
    /* Initialize arrays and bitfields */
    for (int i = 0; i < 32; i++) {
        global_array[i] = i * 0x11111111;
    }
    
    for (int i = 0; i < 64; i++) {
        short_array[i] = i * 0x1111;
    }
    
    for (int i = 0; i < 128; i++) {
        char_array[i] = i;
    }
    
    /* Initialize bitfields */
    bf1.low_bits = 0xAA;
    bf1.mid_bits = 0xBBB;
    bf1.high_bits = 0xCCC;
    bf1.padding = 0xDDDDDDDD;
    
    bf2.nibbles.a = 0x1;
    bf2.nibbles.b = 0x2;
    bf2.nibbles.c = 0x3;
    bf2.nibbles.d = 0x4;
    bf2.separator = 0xEEEE;
    bf2.chunks.x = 0x1FF;
    bf2.chunks.y = 0x2FF;
    bf2.chunks.z = 0x3FF;
    
    /* Use command line arguments for dynamic values */
    int base_index = argc > 1 ? atoi(argv[1]) % 16 : 5;
    int base_shift = argc > 2 ? atoi(argv[2]) % 8 : 3;
    int iterations = argc > 3 ? atoi(argv[3]) % 100 + 10 : 20;
    
    unsigned int checksum = 0;
    
    /* Main loop with complex operations */
    for (int i = 0; i < iterations; i++) {
        int idx = (base_index + i) & 0x1F;
        int shift = (base_shift + i) & 0x7;
        
        /* Pattern 1: Memory access with complex addressing */
        volatile int* mem_ptr = &global_array[idx];
        volatile short* short_ptr = (volatile short*)mem_ptr;
        volatile char* char_ptr = (volatile char*)(&short_array[idx * 2]);
        
        /* May generate MEM_P with complex address */
        int val1 = *mem_ptr;
        short val2 = *short_ptr;
        char val3 = *char_ptr;
        
        /* Combine with bit extraction */
        int extracted1 = (val1 >> shift) & 0xFF;
        int extracted2 = (val2 >> (shift / 2)) & 0x3F;
        
        /* Pattern 2: Bitfield operations */
        unsigned int bf_result = bitfield_operations(i);
        
        /* Pattern 3: Explicit bit manipulation with type conversions */
        long bit_result = explicit_bit_ops(global_long + i, shift);
        
        /* Pattern 4: Complex single statement combining multiple patterns */
        /* This may generate nested RTL with ZERO_EXTRACT, SUBREG, and MEM_P */
        int complex_val = (*(volatile short*)(&global_array[(idx + shift) % 32]) >> 
                          (shift & 0x3)) & 0x7F;
        
        /* Cast to different types for SUBREG */
        short as_short = (short)complex_val;
        char as_char = (char)(complex_val >> 4);
        
        /* More complex: bitfield from memory */
        struct bitfield_struct* bf_ptr = (struct bitfield_struct*)&global_array[idx % 24];
        unsigned int bf_from_mem = bf_ptr->low_bits | 
                                  (bf_ptr->mid_bits << 8) | 
                                  (bf_ptr->high_bits << 20);
        
        /* Update checksum with all results */
        checksum ^= val1;
        checksum += val2;
        checksum ^= val3 << 8;
        checksum += extracted1;
        checksum ^= extracted2 << 4;
        checksum += bf_result;
        checksum ^= (unsigned int)bit_result;
        checksum += complex_val;
        checksum ^= as_short;
        checksum += as_char;
        checksum ^= bf_from_mem;
        
        /* Modify globals to create dependencies */
        global_int ^= checksum;
        global_short += i;
        global_char ^= as_char;
    }
    
    /* Final complex expression combining everything */
    unsigned int final_result = 
        (checksum & 0xFF) |
        ((global_int & 0xFF00) << 8) |
        ((unsigned int)global_short << 16) |
        ((unsigned int)global_char << 24);
    
    /* Access bitfield through pointer for final ZERO_EXTRACT possibility */
    volatile unsigned int* low_bits_ptr = &bf1.low_bits;
    final_result ^= *low_bits_ptr;
    
    /* Memory access with pointer arithmetic */
    volatile int* final_ptr = &global_array[final_result % 32];
    final_result += *final_ptr;
    
    printf("Result: 0x%08X\n", final_result);
    return (int)(final_result & 0x7FFFFFFF);
}
