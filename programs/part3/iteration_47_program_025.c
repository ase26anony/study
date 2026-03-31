#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0x42;
volatile long global_long = 0x9876543210ABCDEF;

/* Global arrays for memory access patterns */
volatile int arr_int[32];
volatile short arr_short[64];
volatile char arr_char[128];

/* Bitfield structs for ZERO_EXTRACT/STRICT_LOW_PART generation */
struct bitfield_struct {
    unsigned int low8 : 8;
    unsigned int mid16 : 16;
    unsigned int high8 : 8;
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
    /* This should generate MEM_P with non-trivial address */
    return arr_int[(idx1 * 3 + idx2 * 7) & 31];
}

/* Function to force SUBREG generation through type punning */
static short type_pun_int_to_short(volatile int *ptr) {
    /* Casting between different-sized types should generate SUBREG */
    return *(volatile short *)ptr;
}

/* Function for bitfield extraction patterns */
static unsigned extract_bits(volatile int value, int shift, int width) {
    /* Explicit bit manipulation for ZERO_EXTRACT */
    return (value >> shift) & ((1 << width) - 1);
}

int main(int argc, char **argv) {
    int i, j;
    unsigned int checksum = 0;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < 32; i++) {
        arr_int[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    for (i = 0; i < 64; i++) {
        arr_short[i] = (i * 1664525 + 1013904223) & 0xFFFF;
    }
    for (i = 0; i < 128; i++) {
        arr_char[i] = (i * 134775813 + 1) & 0xFF;
    }
    
    /* Initialize bitfields */
    bf1.low8 = 0xAA;
    bf1.mid16 = 0xBBBB;
    bf1.high8 = 0xCC;
    
    bf2.inner.a = 0x5;
    bf2.inner.b = 0xA;
    bf2.inner.c = 0xDE;
    bf2.d = 0xBEEF;
    
    /* Use command line arguments to create dynamic values */
    int base_idx = (argc > 1) ? atoi(argv[1]) % 16 : 3;
    int shift_val = (argc > 2) ? atoi(argv[2]) % 8 : 2;
    int width_val = (argc > 3) ? (atoi(argv[3]) % 16) + 1 : 5;
    
    /* Main loop combining all patterns */
    for (i = 0; i < 100; i++) {
        /* Pattern 1: Memory access with complex addressing */
        /* Should generate MEM_P(x) where x has complex address */
        int mem_val = complex_address(i, base_idx);
        checksum = (checksum * 31 + mem_val) & 0xFFFFFFFF;
        
        /* Pattern 2: Type punning for SUBREG generation */
        /* Cast between different-sized types */
        short subreg_val = type_pun_int_to_short(&arr_int[i & 31]);
        checksum ^= (subreg_val << (i & 15));
        
        /* Pattern 3: Direct bitfield access (potential ZERO_EXTRACT/STRICT_LOW_PART) */
        /* Access bitfield members through pointers */
        volatile unsigned int *bf_ptr;
        if (i & 1) {
            bf_ptr = (volatile unsigned int *)&bf1.low8;
        } else {
            bf_ptr = (volatile unsigned int *)&bf2.inner.a;
        }
        unsigned int bf_extract = *bf_ptr;
        checksum += bf_extract;
        
        /* Pattern 4: Explicit bit manipulation on memory */
        /* Combined memory access and bit extraction */
        int bitfield_from_mem = extract_bits(arr_int[(i + shift_val) & 31], 
                                            shift_val, width_val);
        checksum = checksum * 17 + bitfield_from_mem;
        
        /* Pattern 5: Mixed-size operations in expressions */
        /* Operations on different integer types force SUBREG */
        char char_val = arr_char[(i * 5) & 127];
        short short_val = arr_short[(i * 3) & 63];
        int mixed = (int)char_val * (int)short_val;
        checksum ^= mixed;
        
        /* Pattern 6: Complex expression with nested extractions */
        /* Multiple extractions from same value */
        volatile int multi_extract = global_int;
        int part1 = (multi_extract >> 0) & 0xFF;      /* Low 8 bits */
        int part2 = (multi_extract >> 8) & 0xFFFF;    /* Next 16 bits */
        int part3 = (multi_extract >> 24) & 0xFF;     /* High 8 bits */
        checksum += part1 + part2 * 3 + part3 * 7;
        
        /* Pattern 7: Pointer arithmetic with type conversion */
        /* Accesses memory with byte offset then converts */
        volatile char *byte_ptr = (volatile char *)&global_long;
        int byte_sum = 0;
        for (j = 0; j < 8; j++) {
            byte_sum += byte_ptr[(i + j) & 7];
        }
        checksum += byte_sum;
        
        /* Pattern 8: Conditional bitfield operations */
        /* Different paths create different RTL patterns */
        if (i & 0x4) {
            /* STRICT_LOW_PART pattern through assignment */
            volatile short *sptr = (volatile short *)&arr_int[i & 31];
            *sptr = (short)(checksum & 0xFFFF);
        } else {
            /* ZERO_EXTRACT pattern through read-modify-write */
            int temp = arr_int[i & 31];
            temp = (temp & ~(0xFF << 8)) | ((checksum & 0xFF) << 8);
            arr_int[i & 31] = temp;
        }
        
        /* Modify control variables to prevent optimization */
        base_idx = (base_idx * 13 + i) & 31;
        shift_val = (shift_val + 1) & 7;
        width_val = ((width_val * 9 + i) & 15) + 1;
    }
    
    /* Final computation to use all values */
    checksum = (checksum ^ global_int ^ (int)global_short ^ (int)global_char);
    
    printf("Checksum: %u\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
