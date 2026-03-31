#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0xEF;
volatile long global_long = 0x9876543210ABCDEF;

/* Global arrays for memory access patterns */
volatile int arr_int[32];
volatile short arr_short[64];
volatile char arr_char[128];

/* Bitfield structs to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
} volatile bf = {0x5, 0xAB, 0xDEF, 0x42};

struct mixed_bitfield {
    unsigned short low : 6;
    unsigned short high : 10;
    unsigned char byte1 : 4;
    unsigned char byte2 : 4;
} volatile mixed_bf = {0x1F, 0x2A5, 0x9, 0x3};

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* This should generate MEM with complex address */
    return arr_int[idx1 * 3 + idx2 * 7 - 5];
}

/* Function to force SUBREG generation through type punning */
static short type_pun_int_to_short(volatile int *ptr) {
    /* Casting between different-sized types often generates SUBREG */
    return *(volatile short *)ptr;
}

/* Function to extract bits in various ways */
static unsigned extract_bits(volatile unsigned val, int shift, int width) {
    /* Explicit bit extraction - may generate ZERO_EXTRACT */
    return (val >> shift) & ((1 << width) - 1);
}

int main(int argc, char *argv[]) {
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
        arr_char[i] = (i * 214013 + 2531011) & 0xFF;
    }
    
    /* Use command line arguments to create dynamic values */
    int base_idx = (argc > 1) ? atoi(argv[1]) % 16 : 3;
    int shift_amount = (argc > 2) ? atoi(argv[2]) % 8 : 2;
    int loop_count = (argc > 3) ? atoi(argv[3]) % 10 + 5 : 8;
    
    /* Main loop with complex operations */
    for (i = 0; i < loop_count; i++) {
        /* Pattern 1: Bitfield access that may generate ZERO_EXTRACT/STRICT_LOW_PART */
        unsigned int bitfield_val;
        
        /* Access bitfield through pointer - may generate ZERO_EXTRACT */
        bitfield_val = *(volatile unsigned int*)&bf.b;
        checksum ^= bitfield_val << (i & 0xF);
        
        /* Mixed bitfield access */
        bitfield_val = mixed_bf.high;
        checksum += bitfield_val * i;
        
        /* Pattern 2: Explicit bit manipulation on volatile */
        volatile unsigned int temp = global_int;
        unsigned extracted = extract_bits(temp, shift_amount + i, 5);
        checksum |= extracted;
        
        /* Pattern 3: Type punning for SUBREG generation */
        short converted = type_pun_int_to_short(&global_int);
        checksum += converted;
        
        /* More type punning with different sizes */
        char small_val = *(volatile char*)&global_short;
        checksum ^= small_val << 8;
        
        /* Pattern 4: Complex memory addressing */
        int mem_val = complex_address(base_idx + i, i * 2);
        checksum += mem_val;
        
        /* Array access with variable index computation */
        int idx = (base_idx * i + shift_amount) & 0x1F;
        volatile int *ptr = &arr_int[idx];
        checksum ^= *ptr;
        
        /* Pattern 5: Combined operation - memory access with bit extraction */
        volatile short *short_ptr = &arr_short[(i * 7) & 0x3F];
        unsigned combined = (*short_ptr >> (i & 0x7)) & 0x1F;
        checksum += combined * 3;
        
        /* Pattern 6: Struct pointer with offset */
        struct bitfield_struct *bf_ptr = (struct bitfield_struct*)&global_int;
        checksum ^= bf_ptr->a;  /* May generate ZERO_EXTRACT */
        
        /* Pattern 7: Inline assembly to force partial register access */
        /* This often generates SUBREG in RTL */
        unsigned int asm_input = checksum;
        unsigned short asm_output;
        __asm__ volatile (
            "movw %w1, %0"
            : "=r" (asm_output)
            : "r" (asm_input)
            : /* no clobber */
        );
        checksum += asm_output;
        
        /* Pattern 8: Pointer arithmetic with different types */
        volatile char *char_ptr = (volatile char*)arr_int;
        char_ptr += i * sizeof(int) + shift_amount;
        checksum += *char_ptr;
    }
    
    /* Additional complex expressions outside loop */
    
    /* Nested bit extraction from memory */
    volatile int *int_ptr = &arr_int[base_idx];
    unsigned nested_extract = (*(volatile short*)int_ptr >> shift_amount) & 0xFF;
    checksum ^= nested_extract;
    
    /* Access bitfield through double indirection */
    struct bitfield_struct **bf_pp = &bf_ptr;
    checksum += (*bf_pp)->c;
    
    /* Mixed-size operations */
    long long_result = global_long;
    int truncated = (int)long_result;  /* May involve SUBREG */
    checksum += truncated & 0xFFFF;
    
    /* Final output to prevent dead code elimination */
    printf("Checksum: 0x%08X\n", checksum);
    
    return checksum & 0xFF;
}
