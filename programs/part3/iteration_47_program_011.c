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
    unsigned int low_bits : 8;
    unsigned int mid_bits : 16;
    unsigned int high_bits : 8;
    volatile unsigned int full_word;
};

struct nested_bitfield {
    struct {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 8;
    } inner;
    unsigned int d : 16;
};

/* Global bitfield instances */
volatile struct bitfield_struct bf1;
volatile struct nested_bitfield bf2;

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* This should generate MEM_P with complex address */
    return int_array[(idx1 * 3 + idx2 * 7) & 31];
}

/* Function to force SUBREG generation through type conversions */
static short type_punning_conversion(int val) {
    /* Multiple type conversions to force SUBREG */
    char c = (char)(val >> 8);
    short s = (short)(c * 256 + (val & 0xFF));
    
    /* Pointer casting for SUBREG */
    volatile int* ptr = &global_int;
    short result = *(volatile short*)((char*)ptr + 1);
    
    return s + result;
}

/* Function for bitfield extraction patterns */
static unsigned int extract_bits(volatile unsigned int source, int shift, int width) {
    /* Explicit bit manipulation that may generate ZERO_EXTRACT */
    unsigned int mask = (1u << width) - 1;
    return (source >> shift) & mask;
}

/* Function to access bitfield members via pointers */
static int access_bitfield_ptr(struct bitfield_struct* bf, int which) {
    volatile unsigned int* ptr;
    
    switch (which & 3) {
        case 0:
            ptr = &bf->full_word;  /* Direct access */
            return *ptr;
        case 1:
            /* Taking address of bitfield - may generate complex RTL */
            return bf->low_bits + bf->mid_bits;
        case 2:
            /* Mixed bitfield and regular access */
            return (bf->high_bits << 16) | bf->low_bits;
        default:
            /* Pointer arithmetic with bitfield offset */
            return *(volatile int*)((char*)bf + sizeof(int));
    }
}

int main(int argc, char** argv) {
    int i, j;
    unsigned int checksum = 0;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < 32; i++) {
        int_array[i] = i * 0x1234567;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = i * 0xABCD;
    }
    for (i = 0; i < 128; i++) {
        char_array[i] = i * 0x37;
    }
    
    /* Initialize bitfields */
    bf1.low_bits = 0xAA;
    bf1.mid_bits = 0xBBCC;
    bf1.high_bits = 0xDD;
    bf1.full_word = 0xEEFFAABB;
    
    bf2.inner.a = 0x5;
    bf2.inner.b = 0xA;
    bf2.inner.c = 0xBC;
    bf2.d = 0xDEF0;
    
    /* Use command line arguments to create dynamic values */
    int base_shift = (argc > 1) ? atoi(argv[1]) % 16 : 3;
    int base_index = (argc > 2) ? atoi(argv[2]) % 8 : 5;
    
    /* Main loop with complex RTL generation patterns */
    for (i = 0; i < 100; i++) {
        int dynamic_shift = (base_shift + i) % 32;
        int dynamic_index = (base_index + i * 2) % 28;
        
        /* Pattern 1: ZERO_EXTRACT from memory with shifting */
        volatile int* mem_ptr = &int_array[dynamic_index & 31];
        unsigned int extracted = extract_bits(*mem_ptr, dynamic_shift, 8);
        checksum += extracted;
        
        /* Pattern 2: Complex memory addressing with SUBREG */
        if (i & 1) {
            /* Type punning that may generate SUBREG */
            short temp = type_punning_conversion(global_int + i);
            checksum += temp;
            
            /* Mixed size access to array */
            int val = short_array[dynamic_index & 63];
            val += char_array[(dynamic_index * 2) & 127];
            checksum += val;
        }
        
        /* Pattern 3: Bitfield struct access (potential STRICT_LOW_PART) */
        if (i & 2) {
            int bf_val = access_bitfield_ptr(&bf1, i);
            checksum += bf_val;
            
            /* Direct bitfield manipulation */
            bf2.d = (bf2.d + i) & 0xFFFF;
            checksum += bf2.d;
        }
        
        /* Pattern 4: Combined operation - extract from memory, convert, store */
        if (i & 4) {
            /* Complex one-liner combining multiple patterns */
            int combined = (*(volatile short*)(&int_array[i & 31]) >> (dynamic_shift & 7)) & 0xFF;
            combined += (bf1.mid_bits >> (dynamic_shift & 15)) & 0xFF;
            checksum += combined;
        }
        
        /* Pattern 5: Nested addressing with arithmetic */
        int idx1 = (i * 11) & 31;
        int idx2 = (i * 13) & 31;
        int complex_val = complex_address(idx1, idx2);
        checksum += complex_val;
        
        /* Pattern 6: Volatile bitfield extraction with masking */
        volatile unsigned int source = global_int ^ i;
        unsigned int masked = (source >> (i & 7)) & ((1 << ((i & 15) + 1)) - 1);
        checksum += masked;
    }
    
    /* Additional test: pointer to bitfield member */
    if (argc > 3) {
        struct bitfield_struct local_bf;
        local_bf.low_bits = 0x77;
        local_bf.mid_bits = 0x8899;
        local_bf.high_bits = 0xAA;
        local_bf.full_word = 0xBBCCDDEE;
        
        /* This may generate interesting RTL for bitfield address taking */
        volatile unsigned int* ptr_to_bitfield = &local_bf.full_word;
        for (int k = 0; k < 10; k++) {
            checksum += *(ptr_to_bitfield + k);
        }
    }
    
    printf("Final checksum: %u\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
