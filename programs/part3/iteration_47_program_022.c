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
    unsigned int mid8 : 8;
    unsigned int high16 : 16;
};

struct nested_bitfield {
    struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 8;
    } inner;
    unsigned int d : 16;
};

/* Global bitfield instances */
volatile struct bitfield_struct global_bf = {0};
volatile struct nested_bitfield global_nested_bf = {0};

/* Function to create complex addressing modes */
static int complex_memory_access(int idx1, int idx2, int shift) {
    /* Combine multiple indices for non-trivial addressing */
    int array_idx = (idx1 * 3 + idx2 * 7) & 31;
    
    /* Memory access with complex address - should generate MEM_P patterns */
    int val = int_array[array_idx];
    
    /* Additional level of indirection */
    volatile int *ptr = &int_array[(array_idx + shift) & 31];
    val += *ptr;
    
    return val;
}

/* Function to force SUBREG generation through type conversions */
static short type_punning_operations(int value, int offset) {
    /* Multiple type conversions to force SUBREG */
    char c = (char)(value >> offset);
    short s = (short)(value * 2);
    
    /* Pointer casting between different types */
    volatile short *short_ptr = (volatile short *)&global_int;
    s += *short_ptr;
    
    /* Access partial register through pointer */
    volatile char *char_ptr = (volatile char *)&global_long;
    c += char_ptr[offset & 7];
    
    return (short)(s + c);
}

/* Function to create ZERO_EXTRACT patterns */
static unsigned int bitfield_extraction(int base, int shift, int mask) {
    unsigned int result = 0;
    
    /* Explicit bit manipulation - may generate ZERO_EXTRACT */
    result = (global_int >> shift) & mask;
    
    /* Bitfield struct access */
    volatile struct bitfield_struct *bf_ptr = &global_bf;
    result ^= bf_ptr->high16;
    
    /* Nested bitfield access with pointer */
    volatile struct nested_bitfield *nbf_ptr = &global_nested_bf;
    result |= (nbf_ptr->inner.c << 8);
    
    /* More bit manipulation with volatile */
    result ^= (base >> 4) & 0xFF;
    
    return result;
}

int main(int argc, char **argv) {
    int i, j;
    unsigned int checksum = 0;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < 32; i++) {
        int_array[i] = i * 0x13579BDF;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = i * 0x2468;
    }
    for (i = 0; i < 128; i++) {
        char_array[i] = i ^ 0x5A;
    }
    
    /* Initialize bitfields */
    global_bf.low8 = 0xAA;
    global_bf.mid8 = 0xBB;
    global_bf.high16 = 0xCCDD;
    
    global_nested_bf.inner.a = 0x3;
    global_nested_bf.inner.b = 0x12;
    global_nested_bf.inner.c = 0x89;
    global_nested_bf.d = 0xFEDC;
    
    /* Use command line arguments to create dynamic values */
    int base_shift = (argc > 1) ? atoi(argv[1]) % 16 : 3;
    int base_mask = (argc > 2) ? atoi(argv[2]) % 256 : 0x1F;
    int loop_count = (argc > 3) ? atoi(argv[3]) % 100 + 10 : 25;
    
    /* Main loop combining all patterns */
    for (i = 0; i < loop_count; i++) {
        for (j = 0; j < 4; j++) {
            int dynamic_idx = (i * 17 + j * 23) & 31;
            int dynamic_shift = (base_shift + i + j) & 15;
            int dynamic_mask = (base_mask + i * 5) & 0xFF;
            
            /* Pattern 1: Complex memory access with addressing modes */
            int mem_val = complex_memory_access(i, j, dynamic_shift);
            checksum += mem_val;
            
            /* Pattern 2: Type punning for SUBREG generation */
            short subreg_val = type_punning_operations(mem_val, dynamic_shift);
            checksum += subreg_val;
            
            /* Pattern 3: Bitfield extraction for ZERO_EXTRACT/STRICT_LOW_PART */
            unsigned int extract_val = bitfield_extraction(mem_val, dynamic_shift, dynamic_mask);
            checksum ^= extract_val;
            
            /* Pattern 4: Combined operation in single statement */
            /* This may generate nested RTL expressions */
            int combined = (*(volatile short*)(&short_array[dynamic_idx * 2]) >> dynamic_shift) & dynamic_mask;
            combined += (int)((global_bf.mid8 << 8) | global_bf.low8);
            checksum += combined;
            
            /* Pattern 5: Struct member access via pointer with offset */
            volatile char *byte_ptr = (volatile char*)&global_bf;
            char byte_val = byte_ptr[(i + j) & 3];
            checksum += byte_val;
            
            /* Pattern 6: Mixed-size operations */
            long temp = global_long;
            int truncated = (int)(temp >> (dynamic_shift * 4));
            checksum += truncated;
        }
        
        /* Conditional branching to create different control flow paths */
        if (i & 1) {
            /* Access different bitfield on odd iterations */
            checksum += global_nested_bf.inner.b;
        } else {
            /* Access different memory location on even iterations */
            checksum += int_array[i & 31];
        }
        
        /* Modify global variables to create dependencies */
        global_int ^= checksum;
        global_short += (short)(checksum & 0xFFFF);
    }
    
    /* Final computation to use all results */
    checksum = (checksum & 0xFFFFFFFF);
    
    printf("Result checksum: %u (0x%08X)\n", checksum, checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}
