#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0xEF;
volatile long global_long = 0x9ABCDEF012345678L;

/* Global arrays for memory access patterns */
volatile int int_array[32];
volatile short short_array[64];
volatile char char_array[128];

/* Bitfield structs for ZERO_EXTRACT/STRICT_LOW_PART generation */
struct bitfield_struct {
    unsigned int low8 : 8;
    unsigned int mid16 : 16;
    unsigned int high8 : 8;
    unsigned int extra : 24;
    unsigned int final : 8;
};

struct mixed_bitfields {
    unsigned short s_low : 4;
    unsigned short s_mid : 8;
    unsigned short s_high : 4;
    unsigned char c_field : 6;
    unsigned char c_rest : 2;
};

/* Global bitfield instances */
volatile struct bitfield_struct bf_global;
volatile struct mixed_bitfields mixed_bf;

/* Function to initialize data with non-constant values */
void initialize_data(int seed) {
    for (int i = 0; i < 32; i++) {
        int_array[i] = (seed + i * 3) ^ 0xDEADBEEF;
    }
    for (int i = 0; i < 64; i++) {
        short_array[i] = (short)((seed + i * 5) ^ 0xBEEF);
    }
    for (int i = 0; i < 128; i++) {
        char_array[i] = (char)((seed + i * 7) ^ 0xEF);
    }
    
    bf_global.low8 = seed & 0xFF;
    bf_global.mid16 = (seed * 3) & 0xFFFF;
    bf_global.high8 = (seed >> 8) & 0xFF;
    bf_global.extra = (seed * 5) & 0xFFFFFF;
    bf_global.final = (seed >> 16) & 0xFF;
    
    mixed_bf.s_low = seed & 0xF;
    mixed_bf.s_mid = (seed >> 4) & 0xFF;
    mixed_bf.s_high = (seed >> 12) & 0xF;
    mixed_bf.c_field = (seed >> 8) & 0x3F;
    mixed_bf.c_rest = (seed >> 14) & 0x3;
}

/* Complex function combining multiple patterns */
unsigned int process_bitfields(int index, int shift) {
    unsigned int result = 0;
    
    /* Pattern 1: ZERO_EXTRACT from memory with shifting */
    volatile int* volatile_ptr = &int_array[index % 32];
    result ^= ((*volatile_ptr >> shift) & 0x1F);  /* Likely ZERO_EXTRACT */
    
    /* Pattern 2: Type punning with different sizes - may generate SUBREG */
    volatile short* short_ptr = (volatile short*)&int_array[(index + 1) % 32];
    short temp_short = *short_ptr;  /* SUBREG may appear here */
    result += (unsigned int)temp_short;
    
    /* Pattern 3: Bitfield access through pointer */
    volatile unsigned int* bf_ptr = (volatile unsigned int*)&bf_global;
    result ^= (*bf_ptr & 0xFF00) >> 8;
    
    /* Pattern 4: Complex memory reference with array indexing */
    volatile int val = int_array[(index * 2 + shift) % 32];
    result += (val & 0xFFFF0000) >> 16;
    
    /* Pattern 5: Mixed bitfield operations */
    volatile struct mixed_bitfields* mptr = &mixed_bf;
    result ^= ((unsigned int)mptr->s_mid << 4) | mptr->s_low;
    
    return result;
}

/* Function with STRICT_LOW_PART patterns */
int strict_low_part_patterns(int idx) {
    int sum = 0;
    
    /* Access bitfield members individually - may generate STRICT_LOW_PART */
    sum += bf_global.low8;
    sum -= bf_global.mid16;
    sum ^= bf_global.high8 << 8;
    
    /* Type conversion with masking */
    volatile long long_val = global_long;
    short short_val = (short)(long_val & 0xFFFF);  /* Potential SUBREG */
    sum += short_val;
    
    /* Pointer casting between different types */
    volatile char* char_ptr = (volatile char*)&short_array[idx % 64];
    int char_as_int = *char_ptr;  /* SUBREG may appear */
    sum += char_as_int * 3;
    
    return sum;
}

/* Function creating complex addressing modes */
int complex_memory_access(int base, int offset1, int offset2) {
    int total = 0;
    
    /* Multi-dimensional array-like access with variable offsets */
    volatile int* ptr1 = &int_array[(base + offset1) % 32];
    volatile int* ptr2 = &int_array[(base + offset2) % 16];
    
    total += *ptr1;
    total -= *ptr2;
    
    /* Structure pointer arithmetic */
    volatile struct bitfield_struct* bf_arr = &bf_global;
    total += (int)((volatile char*)bf_arr)[offset1 % sizeof(struct bitfield_struct)];
    
    /* Pointer chain */
    volatile int** ptr_to_ptr = (volatile int**)&ptr1;
    total ^= **(ptr_to_ptr);
    
    return total;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    if (seed == 0) {
        seed = 12345;  /* Default seed */
    }
    
    initialize_data(seed);
    
    unsigned int checksum = 0;
    int loop_count = 100;
    
    /* Main loop combining all patterns */
    for (int i = 0; i < loop_count; i++) {
        int idx = (i + seed) % 32;
        int shift = (i * 7) % 16;
        
        /* Combine multiple patterns in single expressions */
        
        /* Pattern A: ZERO_EXTRACT from memory with type conversion */
        volatile int mem_val = int_array[idx];
        checksum += ((mem_val >> shift) & 0xFF);  /* ZERO_EXTRACT candidate */
        
        /* Pattern B: SUBREG through type punning */
        volatile short* sp = (volatile short*)&mem_val;
        short half_val = sp[1];  /* SUBREG candidate */
        checksum ^= (half_val & 0xFF) << 8;
        
        /* Pattern C: Bitfield extraction with pointer */
        volatile unsigned char* bf_byte = (volatile unsigned char*)&bf_global;
        checksum += bf_byte[(i % 4) + 1] * 3;
        
        /* Pattern D: Complex memory reference */
        int offset = (i * 11) % 28;
        volatile int* complex_ptr = &int_array[(idx + offset) % 32];
        checksum -= *complex_ptr & 0xFFFF;
        
        /* Pattern E: Mixed operations in one statement */
        /* This complex statement may generate nested RTL expressions */
        checksum ^= ((*(volatile short*)(&int_array[(idx + 3) % 32]) >> (shift % 8)) & 0x1F) 
                    + ((unsigned char)char_array[(idx * 2) % 128]);
        
        /* Call functions that generate specific patterns */
        checksum += process_bitfields(idx, shift);
        checksum ^= strict_low_part_patterns(idx);
        checksum += complex_memory_access(idx, shift, i % 8);
        
        /* Prevent loop unrolling from simplifying too much */
        if (checksum & 1) {
            shift = (shift + 1) % 16;
        }
    }
    
    /* Additional complex standalone expressions */
    
    /* ZERO_EXTRACT with volatile */
    volatile int extract_src = seed * 0x98765432;
    int extracted = (extract_src >> 5) & 0x1F;  /* Classic ZERO_EXTRACT pattern */
    checksum += extracted;
    
    /* STRICT_LOW_PART via bitfield assignment */
    struct {
        unsigned int low : 10;
        unsigned int high : 22;
    } local_bf;
    
    volatile unsigned int* local_bf_ptr = (volatile unsigned int*)&local_bf;
    *local_bf_ptr = checksum;  /* May generate STRICT_LOW_PART for bitfield stores */
    
    /* SUBREG through explicit size change */
    volatile long big_val = 0x123456789ABCDEF0L;
    int truncated = (int)big_val;  /* SUBREG candidate */
    checksum ^= truncated;
    
    /* Complex memory reference with pointer arithmetic */
    volatile int* dyn_ptr = int_array + (checksum % 16);
    for (int j = 0; j < 4; j++) {
        checksum += dyn_ptr[j * 3];  /* Non-constant stride access */
    }
    
    printf("Final checksum: %u\n", checksum);
    return 0;
}
