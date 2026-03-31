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

/* Bitfield structs to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int low_bits : 8;
    unsigned int mid_bits : 16;
    unsigned int high_bits : 8;
    volatile unsigned int padding;
};

struct nested_bitfield {
    struct {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 8;
    } inner;
    volatile unsigned int control;
};

/* Global bitfield instances */
volatile struct bitfield_struct g_bitfield = {0};
volatile struct nested_bitfield g_nested = {0};

/* Function to create complex addressing modes */
static int complex_memory_access(int index, int offset) {
    /* This should generate MEM with complex address */
    volatile int *ptr;
    
    /* Array access with variable index and offset - non-trivial addressing */
    ptr = &int_array[(index * 3 + offset) & 31];
    
    /* Combine with bitfield extraction */
    struct bitfield_struct local_bf;
    local_bf.low_bits = (*ptr >> 0) & 0xFF;
    local_bf.mid_bits = (*ptr >> 8) & 0xFFFF;
    local_bf.high_bits = (*ptr >> 24) & 0xFF;
    
    /* Return combined value */
    return local_bf.low_bits + local_bf.mid_bits + local_bf.high_bits;
}

/* Function to force SUBREG generation through type punning */
static short type_punning_operations(volatile int *source) {
    /* Multiple type conversions to force SUBREG */
    char c1, c2;
    short s1, s2;
    int temp;
    
    /* Access through different type pointers */
    c1 = *(volatile char *)source;
    s1 = *(volatile short *)source;
    
    /* Shift and mask operations that may generate ZERO_EXTRACT */
    temp = (*source >> 4) & 0x0F0F0F0F;
    
    /* More type conversions */
    c2 = (temp >> 16) & 0xFF;
    s2 = temp & 0xFFFF;
    
    /* Combine results */
    return (short)(c1 + c2 + s1 + s2);
}

/* Function for explicit bit manipulation */
static unsigned int bit_manipulation(volatile unsigned int value, int shift) {
    unsigned int result = 0;
    
    /* Multiple bitfield extractions */
    result |= (value >> shift) & 0x3F;          /* May generate ZERO_EXTRACT */
    result |= (value << (32 - shift)) & 0xC0;   /* Another extraction */
    
    /* Access specific bits using struct pointer */
    struct bitfield_struct *bf_ptr = (struct bitfield_struct *)&value;
    result |= bf_ptr->low_bits;
    
    return result;
}

int main(int argc, char **argv) {
    int i, j;
    unsigned int checksum = 0;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 32; i++) {
        int_array[i] = i * 0x01010101;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = i * 0x1111;
    }
    for (i = 0; i < 128; i++) {
        char_array[i] = i & 0x7F;
    }
    
    /* Use command line arguments for dynamic values */
    int base_index = (argc > 1) ? atoi(argv[1]) % 16 : 5;
    int shift_amount = (argc > 2) ? atoi(argv[2]) % 24 : 3;
    
    /* Loop with opaque control flow */
    for (i = 0; i < 100; i++) {
        volatile int loop_var = i;
        
        /* Condition based on external input to create different paths */
        if (loop_var % (base_index + 2) == 0) {
            /* Path 1: Complex memory access with bitfield extraction */
            checksum += complex_memory_access(loop_var, base_index);
            
            /* Additional bit manipulation */
            volatile int temp = int_array[loop_var & 31];
            checksum += bit_manipulation(temp, shift_amount + (loop_var & 7));
        } else if (loop_var % (shift_amount + 1) == 0) {
            /* Path 2: Type punning and SUBREG generation */
            checksum += type_punning_operations(&int_array[loop_var & 31]);
            
            /* Direct bitfield struct access */
            g_bitfield.low_bits = loop_var & 0xFF;
            g_bitfield.mid_bits = (loop_var * 3) & 0xFFFF;
            g_bitfield.high_bits = (loop_var >> 4) & 0xFF;
            
            /* Take address of bitfield member - may generate interesting RTL */
            volatile unsigned int *ptr = &g_bitfield.padding;
            *ptr = checksum & 0xFF;
        } else {
            /* Path 3: Mixed operations */
            
            /* Memory access with complex address calculation */
            volatile short *sptr = &short_array[(loop_var * 5 + base_index) & 63];
            volatile int val = *sptr;  /* Load short into int - may use SUBREG */
            
            /* Bit extraction from memory value */
            int extracted = (val >> (shift_amount & 7)) & 0x1F;  /* ZERO_EXTRACT candidate */
            
            /* Store with possible STRICT_LOW_PART */
            char_array[loop_var & 127] = extracted & 0xFF;
            
            checksum += extracted + val;
        }
        
        /* Nested bitfield operations */
        g_nested.inner.a = (checksum >> 0) & 0x0F;
        g_nested.inner.b = (checksum >> 4) & 0x0F;
        g_nested.inner.c = (checksum >> 8) & 0xFF;
        
        /* Access through pointer to force memory reference */
        volatile unsigned int *control_ptr = &g_nested.control;
        *control_ptr = *control_ptr ^ checksum;
        
        /* Additional complex expression combining multiple patterns */
        if (loop_var & 1) {
            /* Combine bit extraction, type conversion, and memory access */
            int combined = (*(volatile short*)(&int_array[(loop_var + base_index) & 31]) 
                          >> (shift_amount & 3)) 
                         & ((loop_var & 0x10) ? 0x0F : 0x1F);
            checksum += combined;
        }
    }
    
    /* Final computation using global variables */
    checksum ^= global_int;
    checksum ^= (global_short << 16);
    checksum ^= (global_char & 0xFF);
    checksum ^= (global_long & 0xFFFFFFFF);
    
    printf("Final checksum: %u (0x%08X)\n", checksum, checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
