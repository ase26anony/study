#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0xEF;
volatile long global_long = 0x9ABCDEF012345678L;

/* Global arrays for memory access patterns */
volatile int arr_int[32];
volatile short arr_short[64];
volatile char arr_char[128];

/* Bitfield structs for ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int field1 : 4;
    unsigned int field2 : 8;
    unsigned int field3 : 12;
    unsigned int field4 : 8;
};

struct mixed_bitfields {
    unsigned short low : 5;
    unsigned short mid : 6;
    unsigned short high : 5;
    unsigned char extra : 4;
};

/* Global bitfield instances */
volatile struct bitfield_struct bf_global;
volatile struct mixed_bitfields mixed_bf;

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* Force non-constant address computation */
    volatile int *ptr;
    
    /* Array indexing with variable offset - may generate MEM with complex address */
    ptr = &arr_int[(idx1 * 7 + idx2 * 3) & 31];
    return *ptr;
}

/* Function to extract bits using various methods */
static unsigned extract_bits(volatile unsigned src, int shift, int width) {
    /* Multiple extraction patterns that may generate ZERO_EXTRACT */
    unsigned result = 0;
    
    /* Pattern 1: Direct shift and mask */
    result |= (src >> shift) & ((1 << width) - 1);
    
    /* Pattern 2: Multiple extractions combined */
    result |= (src >> (shift + 4)) & 0xF;
    
    return result;
}

/* Function to force SUBREG appearances through type conversions */
static int type_punning_operations(int val) {
    volatile int int_var = val;
    volatile short short_var;
    volatile char char_var;
    
    /* Type conversions that may generate SUBREG */
    short_var = *(volatile short*)&int_var;  /* Likely SUBREG */
    char_var = *(volatile char*)&int_var;    /* Likely SUBREG */
    
    /* Mix types in computation */
    return int_var + short_var * 3 + char_var * 5;
}

int main(int argc, char **argv) {
    int i, j;
    unsigned long checksum = 0;
    
    /* Initialize arrays with pseudo-random data */
    for (i = 0; i < 32; i++) {
        arr_int[i] = i * 0x12345 + 0x6789;
    }
    for (i = 0; i < 64; i++) {
        arr_short[i] = i * 0xABCD + 0x1234;
    }
    for (i = 0; i < 128; i++) {
        arr_char[i] = i * 0x37 + 0x42;
    }
    
    /* Initialize bitfields */
    bf_global.field1 = 0xA;
    bf_global.field2 = 0xBC;
    bf_global.field3 = 0xDEF;
    bf_global.field4 = 0x12;
    
    mixed_bf.low = 0x1F;
    mixed_bf.mid = 0x2A;
    mixed_bf.high = 0x15;
    mixed_bf.extra = 0x7;
    
    /* Use command line arguments to create runtime variability */
    int base_shift = (argc > 1) ? atoi(argv[1]) % 16 : 3;
    int base_index = (argc > 2) ? atoi(argv[2]) % 32 : 7;
    
    /* Main loop with complex operations */
    for (i = 0; i < 100; i++) {
        volatile int temp;
        volatile short stemp;
        volatile char ctemp;
        
        /* Pattern 1: Memory access with complex addressing */
        /* May generate MEM with address that needs marking */
        temp = complex_address(i, base_index);
        checksum += temp;
        
        /* Pattern 2: Bitfield extraction from memory */
        /* May generate ZERO_EXTRACT */
        if (i & 1) {
            /* Access bitfield through pointer - may create ZERO_EXTRACT */
            volatile unsigned *bf_ptr = (volatile unsigned*)&bf_global;
            unsigned extracted = (*bf_ptr >> (i % 16)) & 0xFF;
            checksum += extracted;
            
            /* Another extraction pattern */
            extracted = (global_int >> ((i + base_shift) % 24)) & 0x3FF;
            checksum += extracted;
        }
        
        /* Pattern 3: Type punning and SUBREG generation */
        /* Force conversions between different-sized types */
        stemp = *(volatile short*)&arr_int[i & 31];  /* Likely SUBREG */
        checksum += stemp;
        
        ctemp = *(volatile char*)&arr_short[i & 63]; /* Likely SUBREG */
        checksum += ctemp;
        
        /* Pattern 4: Combined operation - memory access with bit extraction */
        /* Complex expression that may generate nested RTL */
        if (i & 2) {
            /* Access memory, extract bits, convert type */
            int val = arr_int[(i * 3) & 31];
            short sval = (val >> (i % 8)) & 0x7FFF;  /* Shift and mask */
            checksum += *(volatile char*)&sval;      /* Type punning */
        }
        
        /* Pattern 5: Struct member access via pointer arithmetic */
        /* Creates complex address computation */
        volatile struct mixed_bitfields *mb_ptr = &mixed_bf;
        volatile unsigned char *byte_ptr = (volatile unsigned char*)mb_ptr;
        
        /* Access with offset - non-trivial address */
        checksum += byte_ptr[(i % 4)];
        
        /* Pattern 6: Explicit bit manipulation on volatile */
        /* May generate ZERO_EXTRACT for bitfield-like operations */
        volatile unsigned bitfield_like = global_int;
        unsigned bits = (bitfield_like >> 8) & 0xF;   /* 4-bit extract */
        bits |= (bitfield_like >> 16) & 0xFF;         /* 8-bit extract */
        bits |= (bitfield_like >> 24) & 0x7;          /* 3-bit extract */
        checksum += bits;
        
        /* Pattern 7: Mixed-size operations in expression */
        /* Forces SUBREG for size conversions */
        long ltemp = global_long;
        int itemp = ltemp & 0xFFFFFFFF;  /* Truncation */
        short stemps = itemp >> 16;      /* Another conversion */
        checksum += stemps + (itemp & 0xFFFF);
        
        /* Pattern 8: Conditional extraction based on runtime value */
        /* Prevents constant folding */
        int shift_amount = (i + base_shift) % 28;
        int width = 1 + (i % 7);
        checksum += extract_bits(global_int, shift_amount, width);
        
        /* Pattern 9: More type punning */
        checksum += type_punning_operations(i);
        
        /* Pattern 10: Array access with complex index computation */
        /* MEM with non-simple address */
        int idx1 = (i * 5 + base_index) & 31;
        int idx2 = (i * 7 + base_shift) & 31;
        checksum += arr_int[idx1] + arr_short[idx2 * 2];
    }
    
    /* Prevent dead code elimination */
    printf("Checksum: %lu\n", checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}
