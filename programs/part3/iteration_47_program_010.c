#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0xEF;
volatile long global_long = 0x9ABCDEF0;

/* Global arrays for memory access patterns */
volatile int global_array[32];
volatile short short_array[64];
volatile char char_array[128];

/* Bitfield structs for ZERO_EXTRACT/STRICT_LOW_PART generation */
struct bitfield_struct {
    unsigned int low8 : 8;
    unsigned int mid16 : 16;
    unsigned int high8 : 8;
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
volatile struct bitfield_struct bf1;
volatile struct nested_bitfield bf2;

/* Function to create complex addressing modes */
static int complex_address(int idx1, int idx2) {
    /* This should generate MEM with complex address */
    return global_array[(idx1 * 3 + idx2 * 7) & 31];
}

/* Function to force SUBREG generation through type punning */
static short type_pun_int_to_short(int val) {
    /* Casting between types should generate SUBREG */
    volatile int temp = val;
    return *(volatile short*)&temp;
}

/* Function to extract bits using shift/mask (ZERO_EXTRACT pattern) */
static int extract_bits(volatile int src, int shift, int mask) {
    /* This may generate ZERO_EXTRACT in RTL */
    return (src >> shift) & mask;
}

/* Function to access bitfield via pointer */
static int access_bitfield_ptr(volatile struct bitfield_struct *bf, int which) {
    int result = 0;
    volatile unsigned int *ptr;
    
    /* Different pointer accesses to force various RTL patterns */
    if (which == 0) {
        /* Access via pointer to bitfield member - may generate ZERO_EXTRACT */
        ptr = (volatile unsigned int*)&bf->low8;
        result = *ptr & 0xFF;
    } else if (which == 1) {
        /* Another bitfield access pattern */
        ptr = (volatile unsigned int*)&bf->mid16;
        result = (*ptr >> 8) & 0xFFFF;
    } else {
        /* Cast to char pointer for SUBREG generation */
        volatile char *cptr = (volatile char*)&bf->high8;
        result = *cptr;
    }
    
    return result;
}

int main(int argc, char **argv) {
    int i, j;
    int checksum = 0;
    
    /* Initialize arrays with pseudo-random but deterministic values */
    for (i = 0; i < 32; i++) {
        global_array[i] = i * 0x12345 + 0x6789;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = i * 0x37 - 0xABCD;
    }
    for (i = 0; i < 128; i++) {
        char_array[i] = i ^ 0x55;
    }
    
    /* Initialize bitfields */
    bf1.low8 = 0xAA;
    bf1.mid16 = 0xBBCC;
    bf1.high8 = 0xDD;
    
    bf2.inner.a = 0x3;
    bf2.inner.b = 0x12;
    bf2.inner.c = 0xAB;
    bf2.d = 0xCDEF;
    
    /* Use command line arguments to create dynamic values */
    int base_idx = (argc > 1) ? atoi(argv[1]) % 16 : 5;
    int shift_val = (argc > 2) ? atoi(argv[2]) % 8 : 3;
    int mask_val = (argc > 3) ? atoi(argv[3]) % 255 : 0x1F;
    
    /* Main loop with complex operations */
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 4; j++) {
            int idx = (base_idx + i * 2 + j * 3) & 31;
            
            /* Pattern 1: Memory access with complex addressing */
            int mem_val = complex_address(i, j);
            checksum ^= mem_val;
            
            /* Pattern 2: Type punning for SUBREG generation */
            short short_val = type_pun_int_to_short(mem_val + i);
            checksum += short_val;
            
            /* Pattern 3: Bit extraction (ZERO_EXTRACT) */
            int extracted = extract_bits(global_int, shift_val + j, mask_val);
            checksum ^= (extracted << (j * 4));
            
            /* Pattern 4: Bitfield access via pointer */
            int bf_val = access_bitfield_ptr(&bf1, j % 3);
            checksum += bf_val * (i + 1);
            
            /* Pattern 5: Nested bitfield with partial access */
            volatile unsigned int *nptr = (volatile unsigned int*)&bf2.inner.b;
            int nested_bf = (*nptr >> 3) & 0x1F;  /* Extract 5-bit field */
            checksum ^= nested_bf;
            
            /* Pattern 6: Mixed-size memory access */
            volatile char *char_ptr = &char_array[idx * 2];
            volatile short *short_ptr = &short_array[idx];
            
            /* Combine different sized accesses */
            int mixed = (*char_ptr << 8) | (*short_ptr & 0xFF);
            checksum += mixed;
            
            /* Pattern 7: Pointer arithmetic with type conversion */
            volatile int *int_ptr = (volatile int*)((char*)global_array + idx * sizeof(int));
            int ptr_math_val = *int_ptr;
            checksum ^= ptr_math_val;
            
            /* Pattern 8: Direct struct member access with offset */
            checksum += ((volatile char*)&bf1)[j % sizeof(bf1)];
        }
        
        /* Modify shift value to create different extraction patterns */
        shift_val = (shift_val + 1) % 8;
    }
    
    /* Additional complex one-liner combining multiple patterns */
    /* This may generate nested RTL expressions */
    int complex_result = ((*(volatile short*)((char*)&global_int + 1) >> shift_val) & mask_val) 
                         + ((volatile struct bitfield_struct*)&global_long)->low8;
    checksum += complex_result;
    
    /* Force volatile writes to ensure all operations are preserved */
    global_int = checksum;
    global_short = checksum & 0xFFFF;
    global_char = checksum & 0xFF;
    
    printf("Final checksum: %d (0x%08x)\n", checksum, checksum);
    
    return checksum & 0xFF;
}
