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

/* Bitfield structs to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int low8 : 8;
    unsigned int mid8 : 8;
    unsigned int high8 : 8;
    unsigned int top8 : 8;
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
volatile struct bitfield_struct g_bf = {0};
volatile struct nested_bitfield g_nbf = {0};

/* Function to create complex addressing modes */
static int complex_memory_access(int idx1, int idx2, int shift) {
    /* Force SUBREG through type conversions */
    short s_val = *(volatile short*)(&int_array[idx1]);
    char c_val = *(volatile char*)(&short_array[idx2]);
    
    /* Combine with bit extraction (potential ZERO_EXTRACT) */
    int extracted = (s_val >> shift) & 0x1F;
    
    /* More type conversions causing SUBREG */
    long l_val = (long)c_val * (long)extracted;
    
    return (int)(l_val & 0xFF);
}

/* Function to manipulate bitfields with pointer access */
static int bitfield_operations(int offset) {
    volatile struct bitfield_struct *bf_ptr = &g_bf;
    volatile struct nested_bitfield *nbf_ptr = &g_nbf;
    
    /* Access bitfield members through pointers - may generate ZERO_EXTRACT */
    int val1 = bf_ptr->low8;
    int val2 = bf_ptr->mid8;
    
    /* Nested bitfield access */
    int val3 = nbf_ptr->inner.a;
    int val4 = nbf_ptr->inner.c;
    
    /* Explicit bit manipulation on volatile */
    volatile unsigned int bits = global_int;
    int extracted_bits = (bits >> (offset & 0x1F)) & 0xFF;  /* Potential ZERO_EXTRACT */
    
    return val1 + val2 + val3 + val4 + extracted_bits;
}

int main(int argc, char **argv) {
    int i, j;
    int checksum = 0;
    
    /* Initialize arrays with non-constant values */
    for (i = 0; i < 32; i++) {
        int_array[i] = i * 3 + argc;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = i * 5 + argc;
    }
    for (i = 0; i < 128; i++) {
        char_array[i] = i * 7 + argc;
    }
    
    /* Initialize bitfields */
    g_bf.low8 = argc & 0xFF;
    g_bf.mid8 = (argc >> 8) & 0xFF;
    g_bf.high8 = (argc * 3) & 0xFF;
    g_bf.top8 = (argc * 5) & 0xFF;
    
    g_nbf.inner.a = argc & 0x7;
    g_nbf.inner.b = (argc >> 3) & 0x1F;
    g_nbf.inner.c = (argc * 2) & 0xFF;
    g_nbf.d = (argc * 4) & 0xFFFF;
    
    /* Main loop with complex operations */
    for (i = 0; i < (argc > 1 ? atoi(argv[1]) % 16 : 8); i++) {
        for (j = 0; j < (argc > 2 ? atoi(argv[2]) % 8 : 4); j++) {
            /* Pattern 1: Memory access with complex addressing */
            /* This creates MEM_P(x) with non-trivial address */
            int idx = (i * 17 + j * 23 + argc) & 0x1F;
            volatile int *mem_ptr = &int_array[idx];
            int mem_val = *mem_ptr;
            
            /* Pattern 2: Pointer casting causing SUBREG */
            short short_val = *(volatile short*)(mem_ptr);
            char char_val = *(volatile char*)(&short_array[idx]);
            
            /* Pattern 3: Bit extraction from memory (ZERO_EXTRACT) */
            int shift = (i + j + argc) & 0x7;
            int extracted = (mem_val >> shift) & ((1 << (shift + 1)) - 1);
            
            /* Pattern 4: Complex expression combining everything */
            /* This may generate nested RTL with ZERO_EXTRACT/SUBREG/MEM */
            int complex_val = (*(volatile short*)(&int_array[(i + j) & 0x1F]) >> 
                              (j & 0x7)) & 0x3F;
            
            /* Pattern 5: Bitfield operations */
            int bf_val = bitfield_operations(i + j);
            
            /* Pattern 6: More type punning with different sizes */
            long long_val = (long)short_val * (long)char_val;
            int truncated = (int)(long_val & 0xFFFF);  /* Potential SUBREG */
            
            /* Combine all values into checksum */
            checksum += mem_val + short_val + char_val + extracted + 
                       complex_val + bf_val + truncated;
            
            /* Additional complex memory access pattern */
            checksum += complex_memory_access(i & 0x1F, j & 0x3F, shift);
            
            /* Access struct through computed pointer (complex address) */
            volatile struct bitfield_struct *dynamic_bf = &g_bf + (i & 0x1);
            checksum += dynamic_bf->low8 + dynamic_bf->mid8;
        }
    }
    
    /* Final bit manipulation on accumulated checksum */
    /* This may generate additional ZERO_EXTRACT patterns */
    checksum = (checksum >> 4) & 0x0FFFFFFF;
    checksum ^= (global_int >> 8) & 0xFF;
    checksum ^= (global_short << 4) & 0xFF0;
    
    /* Use argv to influence final result */
    if (argc > 3) {
        checksum += atoi(argv[3]);
    }
    
    printf("Result: %d\n", checksum);
    return checksum & 0x7F;  /* Return non-zero exit code */
}
