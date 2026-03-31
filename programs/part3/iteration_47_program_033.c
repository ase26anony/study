#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
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
    unsigned int mid_bits : 16;
    unsigned int high_bits : 8;
    volatile unsigned int full_word;
};

struct mixed_bitfields {
    volatile unsigned short part1 : 4;
    volatile unsigned short part2 : 12;
    volatile unsigned int combined : 24;
};

/* Global bitfield instances */
volatile struct bitfield_struct bf1;
volatile struct mixed_bitfields bf2;

/* Function to create complex addressing modes */
static int complex_memory_access(int index, int offset) {
    /* Create MEM_P with complex address: array + variable index + offset */
    volatile int *ptr = (volatile int*)&global_array[index + offset];
    volatile short *sptr = (volatile short*)&short_array[index * 2];
    volatile char *cptr = (volatile char*)&char_array[index + offset * 3];
    
    /* Combine multiple memory accesses with type conversions */
    int val1 = *ptr;                     /* Direct memory access */
    short val2 = *sptr;                  /* SUBREG may appear here */
    char val3 = *cptr;                   /* Another SUBREG candidate */
    
    /* Bit manipulation on memory values */
    int extracted = (val1 >> (offset & 0x7)) & 0xFF;  /* ZERO_EXTRACT pattern */
    int combined = (val2 << 8) | (val3 & 0xFF);
    
    return extracted + combined;
}

/* Function to force SUBREG generation through type punning */
static long type_punning_operations(volatile void *data, int size) {
    long result = 0;
    
    switch (size & 3) {
        case 0:
            /* Access as char - likely generates SUBREG */
            result = *(volatile char*)data;
            break;
        case 1:
            /* Access as short - SUBREG */
            result = *(volatile short*)data;
            break;
        case 2:
            /* Access as int - may involve SUBREG on 64-bit */
            result = *(volatile int*)data;
            break;
        case 3:
            /* Access as long */
            result = *(volatile long*)data;
            break;
    }
    
    /* Additional bitfield extraction */
    result = (result >> 4) & 0x0F0F0F0F;  /* ZERO_EXTRACT pattern */
    
    return result;
}

/* Function to create ZERO_EXTRACT patterns from bitfields */
static unsigned int bitfield_extraction(int mode) {
    unsigned int result = 0;
    
    /* Direct bitfield access - may generate ZERO_EXTRACT */
    if (mode & 1) {
        result |= bf1.low_bits;
        result |= (bf1.mid_bits << 8);
    }
    
    /* Explicit bit manipulation on volatile */
    if (mode & 2) {
        volatile unsigned int temp = bf1.full_word;
        /* Multiple extractions */
        unsigned int bits_8_15 = (temp >> 8) & 0xFF;    /* ZERO_EXTRACT */
        unsigned int bits_16_23 = (temp >> 16) & 0xFF;  /* ZERO_EXTRACT */
        result ^= (bits_8_15 << 16) | bits_16_23;
    }
    
    /* Mixed bitfield operations */
    if (mode & 4) {
        result |= bf2.part1;
        result |= (bf2.part2 << 4);
        /* Extract specific bits from combined field */
        unsigned int middle_bits = (bf2.combined >> 8) & 0xFF;  /* ZERO_EXTRACT */
        result ^= middle_bits;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int i, j;
    unsigned long checksum = 0;
    
    /* Initialize global data */
    for (i = 0; i < 32; i++) {
        global_array[i] = i * 0x01010101;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = i * 0x1111;
    }
    for (i = 0; i < 128; i++) {
        char_array[i] = i & 0xFF;
    }
    
    bf1.low_bits = 0xAA;
    bf1.mid_bits = 0xBBCC;
    bf1.high_bits = 0xDD;
    bf1.full_word = 0xDEADBEEF;
    
    bf2.part1 = 0x5;
    bf2.part2 = 0xABC;
    bf2.combined = 0x123456;
    
    /* Use command line arguments to create dynamic indices */
    int base_index = argc > 1 ? atoi(argv[1]) % 16 : 5;
    int shift_amount = argc > 2 ? atoi(argv[2]) % 8 : 3;
    
    /* Main loop combining all patterns */
    for (i = 0; i < 100; i++) {
        /* Vary parameters to create different RTL patterns */
        int idx = (base_index + i) % 28;
        int mode = i % 8;
        
        /* Pattern 1: Complex memory access with SUBREG and ZERO_EXTRACT */
        int mem_val = complex_memory_access(idx, shift_amount);
        checksum += mem_val;
        
        /* Pattern 2: Type punning operations forcing SUBREG */
        volatile int *target = NULL;
        if (i & 1) {
            target = (volatile int*)&global_int;
        } else if (i & 2) {
            target = (volatile int*)&global_short;  /* Different size */
        } else {
            target = (volatile int*)&global_char;   /* Different size */
        }
        
        long pun_result = type_punning_operations(target, i);
        checksum += pun_result;
        
        /* Pattern 3: Bitfield extraction for ZERO_EXTRACT/STRICT_LOW_PART */
        unsigned int bf_result = bitfield_extraction(mode);
        checksum += bf_result;
        
        /* Pattern 4: Combined operation in single statement */
        /* Extract bits from memory, convert type, mask bits */
        if (i & 4) {
            /* Complex expression combining multiple patterns */
            int combined = ((*(volatile short*)(&short_array[idx * 2]) >> shift_amount) & 0x1F)
                          + ((*(volatile char*)(&char_array[idx + 10]) << 3) & 0xF8);
            checksum += combined;
        }
        
        /* Pattern 5: Pointer arithmetic with complex addressing */
        volatile int *complex_ptr = &global_array[(idx * 3 + shift_amount) % 32];
        volatile short *subreg_ptr = (volatile short*)complex_ptr;
        int ptr_val = *complex_ptr + *subreg_ptr;  /* Mixed size accesses */
        checksum += ptr_val;
    }
    
    /* Additional test: Nested bitfield in struct pointer */
    struct {
        volatile unsigned int header : 8;
        volatile unsigned int data : 24;
    } nested_bf;
    
    nested_bf.header = 0x12;
    nested_bf.data = 0x345678;
    
    volatile unsigned int *bf_ptr = (volatile unsigned int*)&nested_bf;
    unsigned int nested_extract = (*bf_ptr >> 4) & 0x0FFFFFF0;  /* ZERO_EXTRACT */
    checksum += nested_extract;
    
    printf("Final checksum: %lu\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
