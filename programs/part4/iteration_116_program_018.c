/* test_resource_cc.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * resource.cc file (lines 282-290) by generating RTL patterns containing
 * ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM expressions.
 * 
 * Compilation recommendations:
 *   For RTL analysis: gcc -O1 -fdump-rtl-all -fdump-rtl-expand test_resource_cc.c
 *   For scheduling:   gcc -O2 -fschedule-insns test_resource_cc.c
 *   For BPF target:   clang -target bpf -O2 -S -emit-llvm test_resource_cc.c
 */

#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT/STRICT_LOW_PART */
struct __attribute__((packed)) bitfield_struct {
    volatile uint32_t full;
    struct {
        uint32_t low : 8;
        uint32_t mid : 12;
        uint32_t high : 10;
        uint32_t flags : 2;
    } parts;
};

/* Global variables to create complex addressing modes */
static volatile int global_array[32];
static volatile struct bitfield_struct global_bf;

/* Helper to prevent optimization */
static volatile int sink;

int main(void) {
    /* Local variables with volatile to force memory operations */
    volatile struct bitfield_struct local_bf;
    volatile int local_array[16];
    volatile int *ptr_array[8];
    
    /* Initialize */
    for (int i = 0; i < 16; i++) {
        local_array[i] = i * 3;
    }
    
    for (int i = 0; i < 8; i++) {
        ptr_array[i] = &local_array[i * 2];
    }
    
    local_bf.full = 0;
    global_bf.full = 0xFFFFFFFF;
    
    int result = 0;
    
    /* Main loop to generate complex RTL patterns */
    for (int i = 0; i < 100; i++) {
        /* 1. Arithmetic creating SUBREG potential */
        int base = i * 7 + 3;
        
        /* 2. Bit-field assignments for ZERO_EXTRACT/STRICT_LOW_PART */
        /* Direct bit-field assignment */
        local_bf.parts.low = base & 0xFF;           /* May generate ZERO_EXTRACT */
        local_bf.parts.mid = (base >> 3) & 0xFFF;   /* May generate ZERO_EXTRACT */
        
        /* Bitwise operation on full field with partial update */
        local_bf.full = (local_bf.full & ~0xFF) | (base & 0xFF);
        
        /* 3. Complex memory addressing for MEM_P(x) path */
        /* Array access with computed index */
        int idx = (i * 13 + 7) & 0xF;
        local_array[idx] += base;
        
        /* Pointer arithmetic with dereference */
        volatile int *ptr = &local_array[0] + ((i * 5) & 0x7);
        *ptr ^= 0x55;
        
        /* Two-level pointer dereference */
        volatile int **pptr = &ptr_array[(i * 3) & 0x7];
        **pptr += 1;
        
        /* 4. Global variable access with offset */
        global_array[(i * 11) & 0x1F] = local_bf.parts.low;
        
        /* 5. Mixed operations to create dependencies */
        int temp = local_bf.full;
        temp = (temp + 1) & 0xFFFF;
        
        /* Bit manipulation on the result */
        temp = (temp & 0xFF00FF00) | ((temp & 0x00FF00FF) << 8);
        
        /* Store back through complex address */
        *((volatile uint8_t*)&local_bf.full + (i & 0x3)) = temp & 0xFF;
        
        /* Accumulate result to prevent optimization */
        result += local_bf.full + local_array[idx] + *ptr + **pptr;
        
        /* Force memory barrier */
        sink = result;
    }
    
    /* Additional bit-field operations outside loop */
    global_bf.parts.flags = (result >> 16) & 0x3;  /* STRICT_LOW_PART potential */
    
    /* Complex addressing with bit-field component */
    global_array[global_bf.parts.low & 0x1F] = result & 0xFF;
    
    /* Final computation using all variables */
    int final_result = 0;
    for (int i = 0; i < 16; i++) {
        final_result ^= local_array[i];
    }
    final_result ^= global_bf.full;
    final_result ^= result;
    
    printf("Result: %d\n", final_result);
    return final_result & 0xFF;
}
