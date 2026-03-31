/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass (resource.cc lines 282-290). It creates
   operations that generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG,
   and complex MEM expressions in the RTL representation. */

#include <stdio.h>
#include <stdint.h>

/* Volatile variables to prevent optimization and create unpredictable branches */
volatile int v_flag1 = 1;
volatile int v_flag2 = 0;
volatile int v_counter = 0;

/* Global variables for memory access patterns */
unsigned int global_bitfield = 0xDEADBEEF;
unsigned int global_value = 0x12345678;
long long global_ll = 0x1122334455667788LL;
int global_array[256];

/* Structs for bit-field and memory access patterns */
struct BitFieldStruct {
    unsigned int low8 : 8;
    unsigned int mid16 : 16;
    unsigned int high8 : 8;
};

struct ComplexMem {
    int data[4][4];
    int padding;
};

union MixedSize {
    int32_t full;
    int16_t halves[2];
    int8_t bytes[4];
};

/* 1. ZERO_EXTRACT patterns - bit-field operations */
int zero_extract_pattern_1(volatile unsigned int *p) {
    /* This should generate ZERO_EXTRACT for the bit range */
    return (*p >> 8) & 0xFF;  /* Extract bits 8-15 */
}

int zero_extract_pattern_2(struct BitFieldStruct *bfs) {
    /* Bit-field access often generates ZERO_EXTRACT */
    int val = bfs->mid16;  /* 16-bit bit-field extract */
    return val * 2;
}

int zero_extract_pattern_3(volatile unsigned long *p) {
    /* Complex bit extraction */
    return ((*p & 0xF0F0F0F0) >> 4) | ((*p & 0x0F0F0F0F) << 4);
}

/* 2. STRICT_LOW_PART patterns - partial register writes */
void strict_low_part_pattern_1(volatile unsigned int *p, unsigned char v) {
    /* Writing only low byte */
    *p = (*p & ~0xFF) | v;  /* Preserve high bits, set low byte */
}

void strict_low_part_pattern_2(int32_t *x, int16_t v) {
    /* Writing to low 16 bits through pointer cast */
    *(int16_t*)x = v;  /* This may generate STRICT_LOW_PART */
}

void strict_low_part_pattern_3(volatile uint64_t *p, uint32_t v) {
    /* Writing low 32 bits of 64-bit value */
    *p = (*p & 0xFFFFFFFF00000000ULL) | v;
}

/* 3. SUBREG patterns - mixed size accesses */
int subreg_pattern_1(union MixedSize *u) {
    /* Access parts of larger type through smaller views */
    u->halves[0] = 0xABCD;
    u->halves[1] = 0x1234;
    return u->full;  /* Combines both halves */
}

int subreg_pattern_2(long long *ll, int index) {
    /* Access 32-bit portion of 64-bit value */
    int32_t *ptr = (int32_t*)ll;
    return ptr[index];  /* SUBREG for accessing part of larger reg */
}

void subreg_pattern_3(volatile double *d, float f) {
    /* Mixed floating point sizes */
    *(float*)d = f;  /* Write float into double */
}

/* 4. Complex MEM patterns - non-trivial addressing */
int complex_mem_pattern_1(struct ComplexMem *cm, int i, int j, int k) {
    /* Multi-dimensional array with computation */
    return cm->data[(i + j) & 3][k & 3];
}

int complex_mem_pattern_2(int *base, int idx1, int idx2, int idx3) {
    /* Complex addressing with multiple computations */
    return base[idx1 + idx2 * 8 + idx3 * 64];
}

int complex_mem_pattern_3(int (*matrix)[8], int row, int col) {
    /* Pointer to array type */
    return matrix[row][col] + matrix[col][row];
}

/* Main test function that combines all patterns */
int test_all_patterns(int iterations) {
    int result = 0;
    struct BitFieldStruct bfs = {0};
    union MixedSize ms;
    struct ComplexMem cm;
    volatile unsigned int mem_buffer[16];
    
    /* Initialize structures */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cm.data[i][j] = i * 4 + j;
        }
    }
    
    for (int i = 0; i < 16; i++) {
        mem_buffer[i] = i * 0x11111111;
    }
    
    for (int i = 0; i < iterations; i++) {
        /* Use volatile flags to create control flow */
        if (v_flag1) {
            /* ZERO_EXTRACT patterns */
            result ^= zero_extract_pattern_1(&global_bitfield);
            result += zero_extract_pattern_2(&bfs);
            bfs.low8 = result & 0xFF;  /* Bit-field write */
        }
        
        if (v_flag2 || (i % 3 == 0)) {
            /* STRICT_LOW_PART patterns */
            strict_low_part_pattern_1(&global_value, result & 0xFF);
            strict_low_part_pattern_2((int32_t*)&global_value, result & 0xFFFF);
        }
        
        /* SUBREG patterns - always execute */
        result += subreg_pattern_1(&ms);
        result += subreg_pattern_2(&global_ll, i & 1);
        
        /* Complex MEM patterns */
        if (i % 2 == 0) {
            result += complex_mem_pattern_1(&cm, i, i+1, i+2);
            result += complex_mem_pattern_2(global_array, 
                                          (i * 3) & 255, 
                                          (i * 5) & 255,
                                          (i * 7) & 255);
        }
        
        /* More complex memory with pointer arithmetic */
        int *ptr = &global_array[0];
        for (int j = 0; j < 4; j++) {
            ptr[j * 16] = result + j;
        }
        
        /* Update volatile counter */
        v_counter++;
        
        /* Mix in some direct bit manipulation */
        if (i & 1) {
            /* This may generate various RTL patterns */
            uint32_t temp = global_value;
            temp = (temp & 0xFFFF0000) | ((temp & 0x0000FFFF) << 8);
            global_value = temp;
        }
    }
    
    return result;
}

/* Helper functions to increase pass activity */
void helper_function_1(int x) {
    struct BitFieldStruct local_bfs = {0};
    local_bfs.mid16 = x & 0xFFFF;
    
    volatile unsigned int local_var = 0x87654321;
    int extracted = (local_var >> 12) & 0xFFF;  /* ZERO_EXTRACT */
    
    union MixedSize local_ms;
    local_ms.halves[0] = extracted;
}

int helper_function_2(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        /* Complex addressing */
        sum += arr[i * 2] + arr[i * 2 + 1];
        
        /* Partial write */
        if (i & 1) {
            *(int16_t*)(&arr[i]) = sum & 0xFFFF;
        }
    }
    return sum;
}

int main(void) {
    int final_result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    /* Call helper functions to create more RTL contexts */
    helper_function_1(0x1234);
    final_result += helper_function_2(global_array, 32);
    
    /* Main test with multiple iterations */
    final_result += test_all_patterns(100);
    
    /* Additional pattern combinations */
    for (int i = 0; i < 10; i++) {
        /* Direct union access for SUBREG */
        union MixedSize tmp;
        tmp.full = final_result + i;
        final_result ^= tmp.halves[i & 1];
        
        /* Memory with complex index */
        int idx = (i * 17) & 255;
        global_array[idx] = final_result;
        
        /* Bit manipulation for ZERO_EXTRACT */
        volatile unsigned int v = 0x89ABCDEF;
        final_result += (v >> (i * 2)) & 0xF;
    }
    
    /* Ensure all operations have observable effect */
    printf("Result: %d\n", final_result);
    return final_result & 0x7FFFFFFF;  /* Return non-negative */
}
