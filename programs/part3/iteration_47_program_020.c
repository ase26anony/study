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
    unsigned int mid12 : 12;
    unsigned int high12 : 12;
    volatile unsigned int padding;
};

struct mixed_bitfields {
    volatile unsigned short part1 : 4;
    volatile unsigned short part2 : 6;
    volatile unsigned short part3 : 5;
    unsigned char byte_field;
};

/* Global bitfield instances */
struct bitfield_struct bf1;
struct mixed_bitfields bf2;

/* Function to create complex addressing modes */
static int complex_memory_access(int index, int offset) {
    /* Force SUBREG through type conversions */
    short temp_short = *(volatile short*)(&int_array[index]);
    char temp_char = *(volatile char*)(&global_long);
    
    /* Combine with bit extraction */
    int extracted = (temp_short >> (offset & 7)) & 0x1F;
    
    /* More type conversions */
    long temp_long = *(volatile long*)(&short_array[index * 2]);
    int converted = (int)(temp_long & 0xFFFF);
    
    return extracted + converted + temp_char;
}

/* Function to manipulate bitfields */
static unsigned int bitfield_operations(int shift) {
    unsigned int result = 0;
    
    /* Access bitfield members - may generate ZERO_EXTRACT */
    result |= bf1.low8;
    result |= (bf1.mid12 << 8);
    
    /* Pointer to bitfield member */
    volatile unsigned int* ptr = &bf1.padding;
    result ^= *ptr;
    
    /* Bit manipulation on volatile */
    volatile unsigned int v = 0x87654321;
    result += (v >> shift) & 0xFF;  /* Potential ZERO_EXTRACT */
    
    /* Access through casted pointer */
    unsigned short* short_ptr = (unsigned short*)&bf2;
    result += *short_ptr;
    
    return result;
}

int main(int argc, char *argv[]) {
    int i, j;
    unsigned int checksum = 0;
    
    /* Initialize arrays with non-constant values */
    for (i = 0; i < 32; i++) {
        int_array[i] = (i * 0x12345) ^ 0xFEDCBA;
        short_array[i] = (i * 0xABCD) & 0xFFFF;
        char_array[i] = (i * 0x37) & 0xFF;
    }
    
    /* Initialize bitfields */
    bf1.low8 = 0xAA;
    bf1.mid12 = 0xBBB;
    bf1.high12 = 0xCCC;
    bf1.padding = 0xDEADBEEF;
    
    bf2.part1 = 0x7;
    bf2.part2 = 0x2A;
    bf2.part3 = 0x1F;
    bf2.byte_field = 0x88;
    
    /* Use command line arguments to create runtime variability */
    int base_index = (argc > 1) ? atoi(argv[1]) % 16 : 5;
    int shift_amount = (argc > 2) ? atoi(argv[2]) % 24 : 3;
    
    /* Main loop with complex operations */
    for (i = 0; i < 100; i++) {
        /* Vary parameters based on loop counter */
        int idx = (base_index + i) % 28;
        int shift = (shift_amount + i) % 16;
        
        /* Pattern 1: Memory access with complex addressing */
        if (i & 1) {
            /* Array indexing with computation */
            volatile int* mem_ptr = &int_array[idx + (i % 4)];
            checksum += *mem_ptr;
            
            /* Pointer arithmetic */
            volatile short* short_ptr = short_array + idx * 2;
            checksum += *short_ptr;
        }
        
        /* Pattern 2: Bitfield operations */
        if (i & 2) {
            /* Direct bitfield access */
            checksum += bf1.mid12;
            
            /* Bit extraction from memory */
            volatile int temp = int_array[idx];
            checksum += (temp >> shift) & 0x3FF;  /* ZERO_EXTRACT candidate */
            
            /* Type punning with different sizes */
            short temp_short = *(volatile short*)(&int_array[idx]);
            checksum += temp_short;  /* SUBREG candidate */
        }
        
        /* Pattern 3: Combined operations in single statements */
        if (i & 4) {
            /* Complex expression combining multiple patterns */
            int val = (*(volatile short*)(&int_array[idx]) >> (shift & 7)) & 0x1F;
            val += (*(volatile char*)(&global_long) & 0x7F);
            checksum += val;
        }
        
        /* Pattern 4: Function calls with complex operations */
        checksum += complex_memory_access(idx, shift);
        checksum += bitfield_operations(shift);
        
        /* Pattern 5: Structure member access via computed pointer */
        struct bitfield_struct* bf_ptr = &bf1;
        if (idx & 1) {
            checksum += bf_ptr->low8;
            /* Take address of bitfield member */
            volatile unsigned int* pad_ptr = &bf_ptr->padding;
            checksum ^= *pad_ptr;
        }
        
        /* Prevent compiler from optimizing away the loop */
        asm volatile("" : "+r" (checksum));
    }
    
    /* Additional complex patterns outside loop */
    
    /* STRICT_LOW_PART candidate through bitfield assignment */
    {
        struct {
            volatile unsigned int full : 32;
        } s;
        s.full = checksum & 0xFFFF;  /* May generate STRICT_LOW_PART */
        checksum += s.full;
    }
    
    /* More type conversions for SUBREG */
    {
        long big_val = global_long;
        int small_val = (int)big_val;
        short smaller_val = (short)small_val;
        char tiny_val = (char)smaller_val;
        checksum += tiny_val;
    }
    
    /* Complex memory expression */
    {
        /* Array access with variable index computation */
        int complex_idx = (checksum ^ base_index) % 32;
        volatile int* addr = &int_array[complex_idx + (shift_amount % 4)];
        
        /* Dereference with bit manipulation */
        int mem_val = *addr;
        checksum += (mem_val >> 8) & 0xFF;
        
        /* Cast to different pointer type */
        short* short_addr = (short*)addr;
        checksum += *short_addr;
    }
    
    printf("Final checksum: %u\n", checksum);
    return checksum & 0xFF;
}
