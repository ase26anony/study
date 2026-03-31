#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0xEF;
volatile long global_long = 0x9876543210ABCDEF;

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
    unsigned int last : 16;
};

struct mixed_bitfields {
    signed short s_field : 9;
    unsigned short u_field : 7;
    signed int i_field : 18;
    unsigned int pad : 6;
};

/* Global bitfield structs */
volatile struct bitfield_struct bf1;
volatile struct mixed_bitfields bf2;

/* Function to initialize arrays with non-trivial patterns */
void init_arrays(int seed) {
    for (int i = 0; i < 32; i++) {
        int_array[i] = (seed + i * 37) ^ 0xCAFEBABE;
    }
    for (int i = 0; i < 64; i++) {
        short_array[i] = (seed + i * 19) ^ 0xBEEF;
    }
    for (int i = 0; i < 128; i++) {
        char_array[i] = (seed + i * 7) ^ 0x55;
    }
}

/* Complex function combining multiple patterns */
int process_bitfields(int index, int shift, int mask) {
    int result = 0;
    
    /* Pattern 1: Direct bitfield access (may generate ZERO_EXTRACT) */
    result ^= bf1.low8;
    result ^= (bf1.mid16 << 3);
    
    /* Pattern 2: Bit extraction via shift/mask (ZERO_EXTRACT) */
    volatile int temp = global_int;
    result ^= (temp >> shift) & mask;
    result ^= (temp << (32 - shift)) & 0xFF;
    
    /* Pattern 3: Mixed-type access with casting (SUBREG) */
    volatile short s_temp = *(volatile short*)&global_int;
    result ^= s_temp;
    
    /* Pattern 4: Char access with pointer arithmetic (SUBREG + MEM) */
    volatile char* cptr = (volatile char*)&global_long;
    result ^= cptr[index % 8];
    
    return result;
}

/* Function with complex memory addressing */
int memory_access_patterns(int idx1, int idx2, int idx3) {
    int sum = 0;
    
    /* Pattern 1: Array access with variable offset (complex MEM address) */
    sum += int_array[idx1 * 3 + idx2];
    
    /* Pattern 2: Nested array access */
    sum += short_array[idx2 * 2 + idx3] * 2;
    
    /* Pattern 3: Pointer arithmetic with type conversion */
    volatile int* iptr = (volatile int*)((char*)int_array + idx3 * 4);
    sum ^= *iptr;
    
    /* Pattern 4: Struct pointer with offset calculation */
    volatile struct bitfield_struct* bf_ptr = &bf1;
    volatile unsigned int* field_ptr = (volatile unsigned int*)bf_ptr;
    sum += field_ptr[1];  /* Access second word */
    
    return sum;
}

int main(int argc, char** argv) {
    /* Use command-line arguments for dynamic values */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    int iterations = argc > 2 ? atoi(argv[2]) : 100;
    int base_shift = argc > 3 ? atoi(argv[3]) : 3;
    
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    /* Initialize data */
    init_arrays(seed);
    
    /* Initialize bitfield structs */
    bf1.low8 = 0xAA;
    bf1.mid16 = 0xBBCC;
    bf1.high8 = 0xDD;
    bf1.extra = 0xEEFF00;
    bf1.last = 0x1234;
    
    bf2.s_field = -256;
    bf2.u_field = 127;
    bf2.i_field = 131072;
    bf2.pad = 63;
    
    int checksum = 0;
    
    /* Main loop with complex operations */
    for (int i = 0; i < iterations; i++) {
        int idx = i;
        
        /* Combine multiple patterns in single expressions */
        
        /* Pattern A: Bitfield extraction from memory with type conversion */
        volatile struct mixed_bitfields* local_bf = &bf2;
        int val1 = local_bf->s_field + local_bf->u_field;
        checksum ^= val1;
        
        /* Pattern B: ZERO_EXTRACT from memory location with shifting */
        volatile int* mem_int = &int_array[i % 32];
        int val2 = (*mem_int >> (base_shift + (i % 5))) & ((1 << (8 + (i % 9))) - 1);
        checksum += val2;
        
        /* Pattern C: SUBREG through type punning with pointer */
        volatile short* short_ptr = (volatile short*)mem_int;
        short val3 = short_ptr[(i + 1) % 2];  /* May cross word boundary */
        checksum ^= (val3 << (i % 16));
        
        /* Pattern D: Complex memory address with array indexing */
        int val4 = int_array[(i * 7 + seed) % 32] + 
                   short_array[(i * 11 + seed) % 64] -
                   char_array[(i * 13 + seed) % 128];
        checksum += val4;
        
        /* Pattern E: Combined bitfield and memory access */
        if (i % 7 == 0) {
            /* Access bitfield through pointer with offset */
            volatile unsigned int* raw_bf = (volatile unsigned int*)&bf1;
            int field_idx = i % 3;
            unsigned int raw_val = raw_bf[field_idx];
            
            /* Extract specific bits (ZERO_EXTRACT pattern) */
            int extracted = (raw_val >> 8) & 0xFFFF;
            checksum ^= extracted;
            
            /* Type conversion through pointer (SUBREG pattern) */
            volatile char* byte_view = (volatile char*)&raw_val;
            checksum += byte_view[i % 4];
        }
        
        /* Pattern F: Memory indirect with calculation */
        volatile int** indirect = (volatile int**)&mem_int;
        if (i % 11 == 0) {
            int val5 = **indirect;
            checksum ^= (val5 << 3) | (val5 >> 29);  /* Rotation */
        }
        
        /* Call functions that generate specific patterns */
        if (i % 5 == 0) {
            checksum += process_bitfields(i, base_shift + (i % 8), 0xFF >> (i % 4));
        }
        
        if (i % 9 == 0) {
            checksum += memory_access_patterns(i % 8, (i + 3) % 8, (i * 2) % 8);
        }
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 13 == 0) {
            global_int ^= checksum;
            global_short += i;
        }
    }
    
    /* Final mixed-type operation combining everything */
    volatile long final_mix = global_long;
    volatile int* final_int_ptr = (volatile int*)&final_mix;
    volatile short* final_short_ptr = (volatile short*)&final_mix;
    volatile char* final_char_ptr = (volatile char*)&final_mix;
    
    checksum ^= final_int_ptr[0];
    checksum ^= final_int_ptr[1];
    checksum += final_short_ptr[2];
    checksum += final_char_ptr[5];
    
    /* Bitfield extraction from the final mixed value */
    struct {
        unsigned int a : 12;
        unsigned int b : 12;
        unsigned int c : 8;
    } final_bf;
    
    final_bf.a = checksum & 0xFFF;
    final_bf.b = (checksum >> 12) & 0xFFF;
    final_bf.c = (checksum >> 24) & 0xFF;
    
    /* One more memory access with complex addressing */
    volatile int* final_ptr = &int_array[(checksum ^ seed) % 32];
    checksum ^= *final_ptr;
    
    printf("Final checksum: %d (0x%08x)\n", checksum, checksum);
    return checksum & 0xFF;
}
