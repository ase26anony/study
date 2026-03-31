/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass (resource.cc lines 282-290). It creates code
   that should generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and
   complex MEM expressions in the RTL representation. */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create unpredictable control flow */
volatile int v_flag1 = 1;
volatile int v_flag2 = 0;
volatile int v_counter = 0;

/* Global variables for memory access patterns */
volatile unsigned int global_bitfield = 0xDEADBEEF;
int global_array[256];
long long global_ll = 0x123456789ABCDEF0LL;

/* Struct with bit-fields for ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int low8 : 8;
    unsigned int mid8 : 8;
    unsigned int high16 : 16;
};

/* Union for SUBREG patterns */
union mixed_types {
    int32_t i32;
    int16_t i16[2];
    int8_t i8[4];
};

/* Struct for complex memory addressing */
struct nested_array {
    int data[10][10];
    int padding[5];
};

/* 1. ZERO_EXTRACT patterns */
int extract_bits_volatile(volatile unsigned int *p) {
    /* Should generate ZERO_EXTRACT: extract bits 8-15 */
    return (*p >> 8) & 0xFF;
}

int extract_bitfield_struct(struct bitfield_struct *s) {
    /* Accessing bit-fields often creates ZERO_EXTRACT */
    unsigned int val = s->mid8;  /* bits 8-15 */
    return val + s->low8;
}

/* 2. STRICT_LOW_PART patterns */
void set_low_part_direct(volatile unsigned int *p, unsigned char v) {
    /* Writing only low byte - may generate STRICT_LOW_PART */
    *p = (*p & ~0xFF) | v;
}

void set_low_part_cast(int32_t *x, int16_t v) {
    /* Cast assignment to partial register */
    *(int16_t*)x = v;
}

/* 3. SUBREG patterns */
int32_t subreg_via_union(union mixed_types *u) {
    /* Mixed size accesses via union */
    u->i16[0] = 0x1234;
    u->i8[2] = 0xAB;
    return u->i32;
}

int32_t subreg_pointer_cast(long long *ll) {
    /* Access part of larger type */
    int32_t result = *(int32_t*)ll;
    *(int16_t*)((char*)ll + 2) = 0x5678;
    return result;
}

/* 4. Complex MEM patterns */
int complex_mem_access(struct nested_array *arr, int i, int j, int k) {
    /* Multi-dimensional array with offset calculation */
    return arr->data[i + k][j * 2] + arr->data[k][i];
}

int pointer_arithmetic_mem(int *base, int idx1, int idx2, int idx3) {
    /* Complex addressing mode with multiple indices */
    return base[idx1 + idx2 * 4 + idx3 * 8] + 
           base[idx1 * 2 - idx3];
}

/* Main function that combines all patterns with control flow */
int process_patterns(int iteration) {
    int result = 0;
    struct bitfield_struct bfs = {0};
    union mixed_types mt;
    struct nested_array na;
    
    /* Initialize data */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            na.data[i][j] = i * 100 + j + iteration;
        }
    }
    
    /* Control flow based on volatile variables */
    if (v_flag1) {
        /* ZERO_EXTRACT patterns */
        result += extract_bits_volatile(&global_bitfield);
        bfs.low8 = iteration & 0xFF;
        bfs.mid8 = (iteration >> 8) & 0xFF;
        result += extract_bitfield_struct(&bfs);
    }
    
    if (v_flag2 || (iteration % 3 == 0)) {
        /* STRICT_LOW_PART patterns */
        set_low_part_direct(&global_bitfield, iteration & 0xFF);
        set_low_part_cast((int32_t*)&global_ll, iteration & 0xFFFF);
    }
    
    /* SUBREG patterns - always executed */
    result += subreg_via_union(&mt);
    result += subreg_pointer_cast(&global_ll);
    
    /* Complex MEM patterns with loop-dependent indices */
    int idx = (iteration * 7) % 10;
    result += complex_mem_access(&na, idx, (idx + 3) % 10, (idx * 2) % 10);
    
    /* More complex memory addressing */
    result += pointer_arithmetic_mem(global_array, 
                                     iteration % 64,
                                     (iteration * 3) % 64,
                                     (iteration * 5) % 64);
    
    return result;
}

int main() {
    int total_result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    /* Loop with volatile counter to create complex control flow */
    for (v_counter = 0; v_counter < 100; v_counter++) {
        /* Change volatile flags periodically */
        v_flag1 = (v_counter % 7) < 4;
        v_flag2 = (v_counter % 11) > 5;
        
        /* Process all patterns */
        int iter_result = process_patterns(v_counter);
        
        /* Use result to affect next iteration */
        global_array[v_counter % 256] = iter_result;
        global_bitfield ^= iter_result;
        
        total_result += iter_result;
        
        /* Occasionally modify the long long in different ways */
        if (v_counter % 13 == 0) {
            union mixed_types *u = (union mixed_types*)&global_ll;
            u->i16[1] = v_counter & 0x7FFF;
        }
    }
    
    /* Final computation using all patterns one more time */
    total_result += extract_bits_volatile(&global_bitfield);
    set_low_part_direct(&global_bitfield, total_result & 0xFF);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", total_result);
    
    return total_result & 0xFF;
}
