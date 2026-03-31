#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variables to prevent constant folding */
volatile int global_int = 0x12345678;
volatile short global_short = 0xABCD;
volatile char global_char = 0x42;
volatile long global_long = 0x9876543210ABCDEF;

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

struct packed_fields {
    volatile unsigned short field1 : 4;
    volatile unsigned short field2 : 6;
    volatile unsigned short field3 : 5;
    volatile unsigned short field4 : 1;
};

/* Global struct instances */
struct bitfield_struct bf_global;
struct packed_fields pf_global;

/* Complex addressing struct */
struct nested_struct {
    int data[8];
    struct {
        short x;
        short y;
    } coords[4];
    volatile long timestamp;
};

struct nested_struct ns_global;

/* Function to create SUBREG patterns through type conversions */
static int type_punning_operations(volatile int *base, int offset) {
    /* Generate SUBREG through type conversions */
    char c_val = *(volatile char *)((char *)base + offset);
    short s_val = *(volatile short *)((char *)base + offset * 2);
    int i_val = *(volatile int *)((char *)base + offset * 4);
    
    /* Mix types to force SUBREG operations */
    return (int)c_val + (int)s_val + i_val;
}

/* Function to create ZERO_EXTRACT patterns */
static int bitfield_extraction(int index, int shift) {
    int result = 0;
    
    /* Pattern 1: Direct bitfield extraction (may generate ZERO_EXTRACT) */
    bf_global.full_word = global_int + index;
    result = (bf_global.full_word >> shift) & 0xFF;
    
    /* Pattern 2: Struct bitfield access via pointer */
    struct bitfield_struct *bf_ptr = &bf_global;
    volatile unsigned int *bitfield_ptr = &bf_ptr->full_word;
    result += (*bitfield_ptr >> (shift + 4)) & 0x0F;
    
    /* Pattern 3: Multiple bitfield extractions combined */
    pf_global.field1 = (global_short >> 0) & 0x0F;
    pf_global.field2 = (global_short >> 4) & 0x3F;
    pf_global.field3 = (global_short >> 10) & 0x1F;
    pf_global.field4 = (global_short >> 15) & 0x01;
    
    /* Access bitfields in different ways */
    result += pf_global.field1 + pf_global.field2 + pf_global.field3 + pf_global.field4;
    
    return result;
}

/* Function to create complex memory references */
static int complex_memory_access(int idx1, int idx2, int idx3) {
    int sum = 0;
    
    /* Pattern 1: Array indexing with variable offsets */
    sum += global_array[idx1 * 2 + idx2];
    sum += short_array[idx2 * 3 + idx3] * 2;
    sum += char_array[idx1 + idx2 * 4 + idx3 * 8];
    
    /* Pattern 2: Nested struct access with computed offsets */
    sum += ns_global.data[idx1 % 8];
    sum += ns_global.coords[idx2 % 4].x;
    sum += ns_global.coords[idx3 % 4].y;
    
    /* Pattern 3: Pointer arithmetic with type mixing */
    volatile int *ptr1 = &global_array[0] + idx1;
    volatile short *ptr2 = (volatile short *)ptr1 + idx2;
    volatile char *ptr3 = (volatile char *)ptr2 + idx3;
    
    sum += *ptr1 + *ptr2 + *ptr3;
    
    return sum;
}

/* Main function with loops and conditional execution */
int main(int argc, char *argv[]) {
    int i, j, k;
    int checksum = 0;
    
    /* Initialize global data */
    for (i = 0; i < 32; i++) {
        global_array[i] = i * 3 + 1;
    }
    for (i = 0; i < 64; i++) {
        short_array[i] = i * 5 - 2;
    }
    for (i = 0; i < 128; i++) {
        char_array[i] = i % 64;
    }
    
    /* Initialize structs */
    for (i = 0; i < 8; i++) {
        ns_global.data[i] = i * 100;
    }
    for (i = 0; i < 4; i++) {
        ns_global.coords[i].x = i * 10;
        ns_global.coords[i].y = i * 20;
    }
    ns_global.timestamp = 0x12345678;
    
    /* Use command line arguments to create dynamic indices */
    int base_idx = (argc > 1) ? atoi(argv[1]) % 16 : 5;
    int shift_base = (argc > 2) ? atoi(argv[2]) % 8 : 2;
    int loop_count = (argc > 3) ? atoi(argv[3]) % 10 + 5 : 8;
    
    /* Main loop combining all patterns */
    for (i = 0; i < loop_count; i++) {
        for (j = 0; j < 4; j++) {
            /* Dynamic indices based on loop variables and external input */
            int idx1 = (base_idx + i) % 16;
            int idx2 = (shift_base + j) % 8;
            int idx3 = (i * j) % 8;
            int shift = (i + j + shift_base) % 16;
            
            /* Combine all three patterns in complex expressions */
            
            /* 1. Bitfield extraction with type conversion */
            int temp1 = bitfield_extraction(idx1, shift);
            
            /* 2. Type punning with SUBREG patterns */
            int temp2 = type_punning_operations(&global_array[0], idx2);
            
            /* 3. Complex memory access */
            int temp3 = complex_memory_access(idx1, idx2, idx3);
            
            /* 4. Combined expression that may generate nested RTL */
            /* This combines bit extraction from memory with type conversion */
            volatile int combined = global_array[idx1];
            short extracted = (combined >> shift) & 0xFFFF;
            char converted = (char)extracted;
            int final_val = (int)converted + temp1 + temp2 + temp3;
            
            /* Conditional execution based on dynamic values */
            if ((i + j) % 3 == 0) {
                /* Different path with more complex bit manipulation */
                volatile long *long_ptr = &global_long;
                int from_long = (*long_ptr >> (shift * 2)) & 0xFFFFFFFF;
                final_val += from_long;
                
                /* Additional SUBREG pattern through pointer casting */
                short *as_short = (short *)&from_long;
                final_val += as_short[0] + as_short[1];
            } else if ((i + j) % 3 == 1) {
                /* Another path with struct bitfield access */
                struct packed_fields local_pf;
                local_pf.field1 = (global_char >> 0) & 0x0F;
                local_pf.field2 = (global_char >> 4) & 0x0F;
                final_val += local_pf.field1 * 10 + local_pf.field2;
            }
            
            checksum += final_val;
            
            /* Modify global variables to create dependencies */
            if (i % 2 == 0) {
                global_int ^= final_val;
                global_short += checksum & 0xFFFF;
            }
        }
    }
    
    /* Additional complex one-liner combining multiple patterns */
    /* This may generate ZERO_EXTRACT, SUBREG, and MEM_P in one expression */
    int complex_expr = (*(volatile short*)((char*)&global_array[base_idx] + shift_base) >> 
                       (checksum & 0x07)) & 0x1F;
    checksum += complex_expr;
    
    /* Another complex expression with nested operations */
    struct bitfield_struct * volatile bf_ptr_vol = &bf_global;
    int nested_expr = ((bf_ptr_vol->full_word >> 8) & 0xFF) + 
                     (short)(*(volatile int*)(&short_array[base_idx]));
    checksum += nested_expr;
    
    printf("Final checksum: %d\n", checksum);
    return checksum & 0xFF;
}
