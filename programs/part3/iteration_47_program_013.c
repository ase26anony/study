#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0xEF;
volatile long global_long = 0x9ABCDEF012345678L;

/* Global array for memory access patterns */
volatile int global_array[32] = {
    0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8,
    0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x10,
    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
    0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20
};

/* Struct with bitfields to encourage ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int low_bits : 4;
    unsigned int mid_bits : 8;
    unsigned int high_bits : 12;
    unsigned int pad : 8;
};

volatile struct bitfield_struct bf = {0x5, 0xAB, 0xDEF, 0x0};

/* Function to create complex addressing modes */
static inline int complex_address(int idx1, int idx2) {
    /* This should generate MEM with complex address */
    return global_array[(idx1 * 3 + idx2 * 7) & 31];
}

/* Function to force SUBREG through type conversions */
static inline short type_punning_int_to_short(volatile int *ptr) {
    /* Casting int* to short* should generate SUBREG in RTL */
    return *(volatile short *)ptr;
}

/* Function to extract bits using shift/mask (ZERO_EXTRACT pattern) */
static inline int extract_bits(volatile int value, int shift, int mask) {
    /* This may generate ZERO_EXTRACT RTL */
    return (value >> shift) & mask;
}

/* Function to manipulate bitfields (STRICT_LOW_PART pattern) */
static inline void manipulate_bitfield(struct bitfield_struct *s, int val) {
    /* Bitfield assignment may generate STRICT_LOW_PART */
    s->mid_bits = val & 0xFF;
    s->low_bits = (val >> 8) & 0xF;
}

int main(int argc, char **argv) {
    int i, j;
    int checksum = 0;
    
    /* Use command line arguments to get dynamic values */
    int base_shift = (argc > 1) ? atoi(argv[1]) % 16 : 3;
    int base_index = (argc > 2) ? atoi(argv[2]) % 16 : 5;
    
    /* Loop with opaque control flow to stress resource analysis */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 50; j++) {
            int temp = 0;
            
            /* Pattern 1: ZERO_EXTRACT through bit manipulation */
            if ((i + j) & 1) {
                /* Explicit bit extraction - may generate ZERO_EXTRACT */
                temp = extract_bits(global_int, (i + base_shift) & 31, 0x1F);
                checksum ^= temp;
            }
            
            /* Pattern 2: STRICT_LOW_PART through bitfield access */
            if ((i ^ j) & 2) {
                /* Take address of bitfield member */
                volatile unsigned int *p = (volatile unsigned int *)&bf.mid_bits;
                /* This may generate STRICT_LOW_PART in RTL */
                manipulate_bitfield((struct bitfield_struct *)&bf, i * j);
                checksum += *p;
            }
            
            /* Pattern 3: SUBREG through type punning */
            if ((i | j) & 4) {
                /* Type conversion between different sizes */
                short s = type_punning_int_to_short(&global_int);
                /* Further conversion to char */
                char c = (char)(s >> ((i & 3) * 2));
                checksum += c;
                
                /* Another SUBREG pattern: accessing partial register */
                volatile long *lp = &global_long;
                int partial = *(volatile int *)((char *)lp + (i & 3));
                checksum ^= partial;
            }
            
            /* Pattern 4: MEM with complex address calculation */
            if (j & 8) {
                /* Complex array indexing */
                int idx = complex_address(i & 15, j & 15);
                checksum += idx;
                
                /* Pointer arithmetic with non-constant offset */
                volatile int *ptr = &global_array[0] + ((i * 11 + j * 13) & 31);
                checksum ^= *ptr;
            }
            
            /* Combined pattern: Memory access with bit extraction */
            if ((i + j) % 3 == 0) {
                /* Access memory, extract bits, convert type */
                volatile int *mem_ptr = &global_array[(i + base_index) & 31];
                int val = *mem_ptr;
                /* Combined operation: memory → extract → type conversion */
                short extracted = (short)((val >> (j & 7)) & 0xFF);
                checksum += extracted;
            }
            
            /* Prevent compiler from optimizing everything away */
            asm volatile("" : "+r" (checksum));
        }
    }
    
    /* Additional complex expression combining multiple patterns */
    {
        /* Nested operations: memory → bitfield → type conversion */
        volatile struct bitfield_struct *bfp = &bf;
        volatile int *intp = (volatile int *)bfp;
        
        /* This statement may generate complex RTL with multiple patterns */
        int complex_val = (*(volatile short *)((char *)intp + 1) >> 2) & 0x3F;
        checksum += complex_val;
        
        /* Array of different types to force SUBREG */
        volatile char char_array[8] = {1, 2, 3, 4, 5, 6, 7, 8};
        volatile int *casted = (volatile int *)&char_array[i & 3];
        checksum ^= *casted & 0xFF;
    }
    
    printf("Final checksum: %d\n", checksum);
    return checksum & 0xFF;
}
