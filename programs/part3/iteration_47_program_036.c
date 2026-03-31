#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0xEF;
volatile long global_long = 0x9876543210ABCDEF;

/* Global arrays for memory access patterns */
volatile int int_array[32];
volatile short short_array[64];
volatile char char_array[128];

/* Bitfield structs for ZERO_EXTRACT/STRICT_LOW_PART generation */
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
    } inner;
    volatile short separator;
    unsigned int wide_field : 24;
};

/* Global bitfield instances */
volatile struct bitfield_struct bf1;
volatile struct nested_bitfield bf2;

/* Function to create complex addressing modes */
static int complex_memory_access(int index, int offset) {
    /* Create MEM_P with complex address expression */
    volatile int *ptr;
    
    /* Array indexing with variable offset - forces complex address */
    ptr = &int_array[(index * 3 + offset) % 32];
    
    /* Additional pointer arithmetic */
    ptr = (volatile int *)((char *)ptr + (offset & 3));
    
    return *ptr;
}

/* Function to generate SUBREG patterns through type punning */
static short generate_subreg(volatile int *source, int shift) {
    /* Type conversions that may generate SUBREG */
    volatile short temp;
    
    /* Direct type punning */
    temp = *(volatile short *)((char *)source + shift);
    
    /* Additional conversion */
    return (short)((*source >> shift) & 0xFFFF);
}

/* Function for bitfield extraction patterns */
static unsigned int extract_bits(volatile unsigned int value, int pos, int width) {
    /* Explicit bit manipulation that may generate ZERO_EXTRACT */
    unsigned int mask = (1u << width) - 1;
    return (value >> pos) & mask;
}

int main(int argc, char *argv[]) {
    int i, j;
    unsigned int checksum = 0;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < 32; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = (i * 1664525 + 1013904223) & 0x7FFF;
    }
    for (i = 0; i < 128; i++) {
        char_array[i] = (i * 214013 + 2531011) & 0x7F;
    }
    
    /* Use command line arguments to create dynamic values */
    int base_index = (argc > 1) ? atoi(argv[1]) % 16 : 5;
    int shift_amount = (argc > 2) ? atoi(argv[2]) % 8 : 3;
    int loop_count = (argc > 3) ? atoi(argv[3]) % 20 + 10 : 15;
    
    /* Main loop combining all patterns */
    for (i = 0; i < loop_count; i++) {
        volatile int temp_int;
        volatile short temp_short;
        volatile char temp_char;
        
        /* Pattern 1: Memory access with complex addressing (triggers MEM_P) */
        temp_int = complex_memory_access(i, base_index);
        checksum += temp_int;
        
        /* Pattern 2: SUBREG generation through type conversions */
        temp_short = generate_subreg(&global_int, (i + shift_amount) % 16);
        checksum += temp_short;
        
        /* Pattern 3: Direct bitfield access (may generate ZERO_EXTRACT/STRICT_LOW_PART) */
        bf1.low_bits = (i * 7) & 0xFF;
        bf1.mid_bits = (i * 13) & 0xFFF;
        bf1.high_bits = (i * 17) & 0xFFF;
        
        /* Access bitfield through pointer (encourages ZERO_EXTRACT) */
        volatile unsigned int *bf_ptr = (volatile unsigned int *)&bf1.low_bits;
        checksum += *bf_ptr;
        
        /* Pattern 4: Explicit bit extraction (ZERO_EXTRACT candidate) */
        unsigned int extracted = extract_bits(global_int ^ i, 
                                            (i + shift_amount) % 24, 
                                            4 + (i % 4));
        checksum += extracted;
        
        /* Pattern 5: Nested bitfield with volatile separation */
        bf2.inner.a = i & 0xF;
        bf2.inner.b = (i >> 1) & 0xF;
        bf2.inner.c = (i >> 2) & 0xF;
        bf2.inner.d = (i >> 3) & 0xF;
        bf2.wide_field = (i * 0x12345) & 0xFFFFFF;
        
        /* Access through different type pointer */
        volatile unsigned char *char_ptr = (volatile unsigned char *)&bf2.inner;
        checksum += char_ptr[(i + 1) % 4];
        
        /* Pattern 6: Combined operation - memory access with bit extraction */
        if (i & 1) {
            /* Access short array, convert to int with bit manipulation */
            volatile short *sptr = &short_array[(i * 2 + base_index) % 64];
            int combined = ((*sptr << 8) | char_array[i % 128]) & 0x3FF;
            checksum += combined;
        }
        
        /* Pattern 7: Pointer arithmetic with type punning */
        volatile long *long_ptr = (volatile long *)&int_array[i % 16];
        temp_char = *(volatile char *)((char *)long_ptr + (i & 3));
        checksum += temp_char;
        
        /* Pattern 8: Conditional bitfield operation */
        if (i % 3 == 0) {
            /* STRICT_LOW_PART candidate - partial register update */
            volatile struct {
                unsigned int part1 : 16;
                unsigned int part2 : 16;
            } split;
            
            split.part1 = checksum & 0xFFFF;
            split.part2 = (checksum >> 16) & 0xFFFF;
            
            /* Mix the parts */
            unsigned int mixed = (split.part2 << 8) | (split.part1 >> 8);
            checksum ^= mixed;
        }
        
        /* Pattern 9: Array access with bitfield-like extraction */
        int array_index = (i * 11 + base_index) % 32;
        volatile int *int_ptr = &int_array[array_index];
        
        /* Extract specific bits from memory location */
        unsigned int mem_bits = (*int_ptr >> (shift_amount + (i % 4))) & 
                               ((1 << (4 + (i % 8))) - 1);
        checksum += mem_bits;
    }
    
    /* Final mixing to prevent optimization */
    checksum = (checksum >> 16) | (checksum << 16);
    checksum ^= global_int ^ global_short ^ global_char;
    
    printf("Result: %u\n", checksum);
    return checksum & 0xFF;
}
