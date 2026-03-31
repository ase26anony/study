#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0xEF;
volatile long global_long = 0x9ABCDEF012345678L;

/* Global array for memory access patterns */
volatile int global_array[32];
volatile short short_array[64];

/* Bitfield structs for ZERO_EXTRACT/STRICT_LOW_PART generation */
struct bitfield_struct {
    unsigned int low8 : 8;
    unsigned int mid16 : 16;
    unsigned int high8 : 8;
    unsigned int extra : 24;
    unsigned int last16 : 16;
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
volatile struct bitfield_struct bf_global;
volatile struct nested_bitfield nested_bf;

/* Function to create complex addressing modes */
static int complex_memory_access(int idx1, int idx2, int shift) {
    /* Force SUBREG through type punning with different sizes */
    volatile int *int_ptr = (volatile int *)&global_array[idx1];
    volatile short *short_ptr = (volatile short *)&global_array[idx2];
    
    /* This should generate SUBREG when accessing different parts */
    short temp_short = *short_ptr;
    int temp_int = *int_ptr;
    
    /* Combine with bit extraction (potential ZERO_EXTRACT) */
    int extracted = (temp_int >> shift) & 0xFF;
    
    /* More type punning for SUBREG */
    char as_char = (char)(temp_short & 0xFF);
    
    return extracted + as_char + temp_short;
}

/* Function to manipulate bitfields */
static unsigned int bitfield_operations(int mode) {
    unsigned int result = 0;
    
    /* Access bitfield members - may generate ZERO_EXTRACT/STRICT_LOW_PART */
    if (mode & 1) {
        /* Direct bitfield access */
        result += bf_global.low8;
        result += bf_global.mid16 << 8;
        
        /* Take address of bitfield member (complex case) */
        volatile unsigned int *ptr = (volatile unsigned int *)&bf_global.low8;
        result += *ptr;
    }
    
    if (mode & 2) {
        /* Nested bitfield access */
        result += nested_bf.inner.a;
        result += nested_bf.inner.b << 3;
        result += nested_bf.inner.c << 8;
        result += nested_bf.d << 16;
    }
    
    /* Explicit bit manipulation that may generate ZERO_EXTRACT */
    volatile unsigned int bits = global_int;
    if (mode & 4) {
        /* Multiple extractions */
        unsigned int chunk1 = (bits >> 0) & 0xF;
        unsigned int chunk2 = (bits >> 4) & 0xF;
        unsigned int chunk3 = (bits >> 8) & 0xFF;
        unsigned int chunk4 = (bits >> 16) & 0xFFFF;
        
        result += chunk1 + chunk2 + chunk3 + chunk4;
    }
    
    return result;
}

/* Function with combined patterns in single statements */
static int combined_patterns(int index, int shift) {
    int checksum = 0;
    
    /* Complex statement combining memory access, bit extraction, and type conversion */
    checksum += ((*(volatile short*)(&global_array[index]) >> shift) & 0x1F);
    
    /* Another combined pattern with pointer arithmetic */
    volatile char *char_ptr = (volatile char *)global_array;
    checksum += (int)((*(volatile int*)(char_ptr + index * 2) >> (shift * 2)) & 0xFF);
    
    /* Struct pointer with offset calculation */
    volatile struct bitfield_struct *bf_ptr = &bf_global;
    checksum += (int)((*(volatile unsigned int*)((char*)bf_ptr + index)) >> shift);
    
    return checksum;
}

int main(int argc, char **argv) {
    int i, j;
    unsigned long final_checksum = 0;
    
    /* Initialize arrays with non-zero values */
    for (i = 0; i < 32; i++) {
        global_array[i] = 0x1000 + i * 0x111;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = 0x200 + i * 0x22;
    }
    
    /* Initialize bitfields */
    bf_global.low8 = 0xAA;
    bf_global.mid16 = 0xBBBB;
    bf_global.high8 = 0xCC;
    bf_global.extra = 0xDDDDDD;
    bf_global.last16 = 0xEEEE;
    
    nested_bf.inner.a = 0x3;
    nested_bf.inner.b = 0x12;
    nested_bf.inner.c = 0xAB;
    nested_bf.d = 0xCDEF;
    
    /* Use command line arguments to create dynamic values */
    int base_shift = (argc > 1) ? atoi(argv[1]) % 16 : 3;
    int base_index = (argc > 2) ? atoi(argv[2]) % 28 : 5;
    
    /* Main loop with opaque control flow */
    for (i = 0; i < 100; i++) {
        int mode = i % 8;
        int shift = (base_shift + i) % 16;
        int idx1 = (base_index + i) % 24;
        int idx2 = (base_index + i * 2) % 24;
        
        /* Branch with external-dependent condition */
        if (argc > mode) {
            /* Pattern 1: Complex memory access with SUBREG potential */
            final_checksum += complex_memory_access(idx1, idx2, shift);
            
            /* Pattern 2: Bitfield operations */
            final_checksum += bitfield_operations(mode);
            
            /* Pattern 3: Combined patterns in single statements */
            final_checksum += combined_patterns(idx1, shift);
        } else {
            /* Alternative path with different patterns */
            volatile int *ptr = &global_array[i % 16];
            
            /* Type punning through union (may generate SUBREG) */
            union {
                int i;
                short s[2];
                char c[4];
            } converter;
            
            converter.i = *ptr;
            final_checksum += converter.s[0] + converter.s[1];
            final_checksum += converter.c[0] + converter.c[1] + converter.c[2] + converter.c[3];
            
            /* Explicit bit extraction from memory */
            volatile int mem_val = global_array[(i * 7) % 32];
            final_checksum += (mem_val >> (shift * 2)) & ((1 << (mode + 1)) - 1);
        }
        
        /* Additional memory reference with complex address calculation */
        volatile int *complex_ptr = &global_array[(idx1 * 3 + idx2 * 7 + i) % 32];
        final_checksum += *complex_ptr;
        
        /* Pointer to bitfield struct member with offset */
        volatile unsigned int *bf_member_ptr = (volatile unsigned int *)
            ((char*)&bf_global + (i % sizeof(struct bitfield_struct)));
        final_checksum += *bf_member_ptr;
    }
    
    /* Prevent dead code elimination */
    printf("Final checksum: %lu\n", final_checksum);
    
    return (int)(final_checksum & 0x7FFFFFFF);
}
