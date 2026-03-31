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
    unsigned int low_bits : 8;
    unsigned int mid_bits : 12;
    unsigned int high_bits : 12;
    volatile unsigned int padding;
};

struct mixed_bitfields {
    volatile unsigned short part1 : 4;
    volatile unsigned short part2 : 6;
    volatile unsigned short part3 : 6;
};

/* Global bitfield instances */
struct bitfield_struct global_bf;
struct mixed_bitfields global_mixed_bf;

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* Force non-constant address computation */
    volatile int* ptr;
    
    /* Array indexing with variable offset - may generate MEM with complex address */
    int val1 = int_array[idx1 * 3 + idx2];
    
    /* Pointer arithmetic that can't be simplified */
    ptr = (volatile int*)&short_array[idx1];
    int val2 = *(volatile int*)ptr;  /* Type punning - may generate SUBREG */
    
    /* Nested array access */
    int val3 = char_array[int_array[idx2] & 0x7F];
    
    return val1 + val2 + val3;
}

/* Function to manipulate bitfields and generate ZERO_EXTRACT */
static unsigned int bitfield_ops(int shift, int mask) {
    unsigned int result = 0;
    
    /* Direct bitfield access - may generate ZERO_EXTRACT */
    result |= global_bf.low_bits;
    
    /* Bitfield through pointer - may generate STRICT_LOW_PART */
    struct bitfield_struct* bf_ptr = &global_bf;
    result |= bf_ptr->mid_bits << 4;
    
    /* Explicit bit extraction using shifts - may generate ZERO_EXTRACT */
    result |= (global_int >> shift) & mask;
    
    /* Mixed size bitfield access */
    result |= global_mixed_bf.part2;
    
    /* Bit extraction from memory with type conversion */
    volatile short temp = short_array[shift & 0x3F];
    result |= ((unsigned int)temp >> 4) & 0x0F;  /* May generate ZERO_EXTRACT of MEM */
    
    return result;
}

/* Function to generate SUBREG patterns through type conversions */
static int subreg_patterns(int index) {
    int result = 0;
    
    /* Type punning through pointers - likely generates SUBREG */
    result += *(volatile char*)&global_int;
    
    /* Mixed size operations */
    volatile short s = global_short;
    result += (int)s * 2;  /* Conversion short->int may use SUBREG */
    
    /* Access partial register through pointer arithmetic */
    volatile char* byte_ptr = (volatile char*)&global_long + (index & 0x3);
    result += *byte_ptr;
    
    /* Load with different sized access */
    result += (int)global_char;  /* char->int conversion */
    
    /* Complex expression with mixed types */
    result += (int)(*(volatile short*)((char*)int_array + index));
    
    return result;
}

int main(int argc, char** argv) {
    int i, j;
    unsigned int checksum = 0;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < 32; i++) {
        int_array[i] = i * 0x12345 + 0x6789;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = i * 0xABCD + 0x1234;
    }
    for (i = 0; i < 128; i++) {
        char_array[i] = i * 0x37 + 0x42;
    }
    
    /* Initialize bitfields */
    global_bf.low_bits = 0xAA;
    global_bf.mid_bits = 0xBBB;
    global_bf.high_bits = 0xCCC;
    global_bf.padding = 0xDDDDDDDD;
    
    global_mixed_bf.part1 = 0x5;
    global_mixed_bf.part2 = 0x15;
    global_mixed_bf.part3 = 0x25;
    
    /* Use command line arguments to create dynamic values */
    int base_shift = (argc > 1) ? atoi(argv[1]) % 16 : 3;
    int base_index = (argc > 2) ? atoi(argv[2]) % 8 : 5;
    
    /* Main loop combining all patterns */
    for (i = 0; i < 100; i++) {
        int dynamic_shift = (base_shift + i) % 24;
        int dynamic_mask = 0xFF >> (i % 8);
        int idx1 = (base_index + i) % 16;
        int idx2 = (base_index * 2 + i) % 16;
        
        /* Combine multiple patterns in single expressions */
        
        /* Pattern 1: Bitfield extraction from memory with type conversion */
        /* May generate: ZERO_EXTRACT of MEM, followed by SUBREG conversion */
        unsigned int val1 = ((unsigned int)short_array[idx1 * 2] >> dynamic_shift) & dynamic_mask;
        checksum += val1;
        
        /* Pattern 2: Complex memory access with bitfield extraction */
        /* May generate: MEM with complex address, then ZERO_EXTRACT */
        volatile struct bitfield_struct* dyn_bf_ptr = 
            (volatile struct bitfield_struct*)&int_array[idx1];
        checksum += dyn_bf_ptr->low_bits;
        
        /* Pattern 3: Type punning with partial access */
        /* May generate: SUBREG of MEM */
        checksum += *(volatile short*)((char*)&global_long + (i & 0x3));
        
        /* Pattern 4: Nested operations - memory access with shift and mask */
        /* May generate: ZERO_EXTRACT of SUBREG of MEM */
        int temp_idx = (idx1 + idx2) & 0x1F;
        checksum += (int_array[temp_idx] >> (dynamic_shift * 2)) & 0x3FF;
        
        /* Pattern 5: Bitfield struct member through pointer with offset */
        /* May generate complex addressing for MEM */
        checksum += ((struct bitfield_struct*)&char_array[i % 64])->mid_bits;
        
        /* Call functions that generate specific patterns */
        checksum += bitfield_ops(dynamic_shift, dynamic_mask);
        checksum += subreg_patterns(idx1);
        checksum += complex_address(idx1, idx2);
        
        /* Conditional to create different control flow paths */
        if (i & 0x1) {
            /* Different access pattern for odd iterations */
            checksum += (global_int >> (dynamic_shift + 4)) & (dynamic_mask << 1);
        } else {
            /* Different pattern for even iterations */
            checksum += *(volatile int*)((short*)&global_bf + 1);
        }
    }
    
    /* Additional complex standalone expressions */
    
    /* Combined bitfield and memory access */
    checksum += ((global_bf.high_bits << 16) | 
                 (*(volatile short*)&global_mixed_bf));
    
    /* Pointer chain with type conversions */
    checksum += *(volatile int*)((volatile char*)int_array + 
                                 (*(volatile short*)&global_short & 0x7F));
    
    /* Final output to prevent dead code elimination */
    printf("Checksum: 0x%08X\n", checksum & 0xFFFFFFFF);
    
    return (checksum & 0xFF);
}
