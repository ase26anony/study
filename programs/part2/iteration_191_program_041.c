/* Test program to generate specific RTL patterns for coverage testing */
#include <stdio.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;

/* Global variables for memory access patterns */
unsigned int g_bitfield_source = 0xDEADBEEF;
int g_array[256];
long long g_large_value = 0x123456789ABCDEF0LL;

/* Structs for bit-field and subreg patterns */
struct BitFieldStruct {
    unsigned int low8 : 8;
    unsigned int mid16 : 16;
    unsigned int high8 : 8;
};

struct MixedSizeStruct {
    int32_t full;
    int16_t half[2];
    int8_t quarter[4];
};

/* Union for SUBREG patterns */
union SubregUnion {
    int64_t dword;
    int32_t words[2];
    int16_t halfwords[4];
    int8_t bytes[8];
};

/* ========== ZERO_EXTRACT patterns ========== */
int extract_bitfield_8_16(volatile unsigned int *p) {
    /* Should generate ZERO_EXTRACT: extract bits 8-23 */
    return (*p >> 8) & 0xFFFF;
}

int extract_bitfield_4_12(struct BitFieldStruct *bfs) {
    /* Access bit-field member - may generate ZERO_EXTRACT */
    int val = bfs->mid16;
    /* Force another extract pattern */
    return (val >> 4) & 0xFF;
}

/* ========== STRICT_LOW_PART patterns ========== */
void set_low_16_part(volatile uint32_t *p, uint16_t value) {
    /* Write only low 16 bits */
    *p = (*p & 0xFFFF0000) | value;
}

void set_low_8_part_via_cast(int32_t *p, int8_t value) {
    /* Cast to smaller type assignment */
    *(int8_t*)p = value;
}

/* ========== SUBREG patterns ========== */
int32_t access_via_subreg(union SubregUnion *u) {
    /* Access parts of larger type through smaller views */
    u->halfwords[1] = 0x1234;
    u->bytes[3] = 0xAB;
    return u->words[0];
}

int32_t mixed_size_operations(int64_t large_val) {
    /* Operations that require mode changes */
    int32_t low_part = (int32_t)large_val;
    int16_t very_low = (int16_t)large_val;
    return low_part + very_low;
}

/* ========== Complex MEM patterns ========== */
int complex_memory_access(int *base, int idx1, int idx2, int idx3) {
    /* Complex addressing mode with multiple computations */
    return base[(idx1 * 3 + idx2 * 7) % 256] + 
           base[(idx2 * 5 + idx3 * 11) % 256];
}

int struct_memory_access(struct MixedSizeStruct *s, int index) {
    /* Access struct members with offset computation */
    s->half[index % 2] = index;
    s->quarter[(index * 3) % 4] = index & 0xFF;
    return s->full + s->half[0];
}

/* ========== Combined function with control flow ========== */
int combined_operations(int iteration) {
    int result = 0;
    union SubregUnion u;
    struct BitFieldStruct bfs = {0};
    struct MixedSizeStruct mss = {0};
    
    /* Initialize */
    u.dword = g_large_value + iteration;
    bfs.mid16 = (iteration * 7) & 0xFFFF;
    mss.full = iteration * 11;
    
    /* Complex control flow based on volatile */
    if (g_volatile_flag & 1) {
        /* ZERO_EXTRACT pattern */
        result += extract_bitfield_8_16(&g_bitfield_source);
        result += extract_bitfield_4_12(&bfs);
    }
    
    if (g_volatile_flag & 2) {
        /* STRICT_LOW_PART patterns */
        uint32_t temp = result;
        set_low_16_part(&temp, iteration & 0xFFFF);
        set_low_8_part_via_cast(&result, iteration & 0xFF);
        result ^= temp;
    }
    
    if (iteration % 3 == 0) {
        /* SUBREG patterns */
        result += access_via_subreg(&u);
        result += mixed_size_operations(g_large_value + iteration);
    }
    
    if (iteration % 5 == 0) {
        /* Complex MEM patterns */
        result += complex_memory_access(g_array, 
                                       iteration, 
                                       iteration * 2, 
                                       iteration * 3);
        result += struct_memory_access(&mss, iteration);
    }
    
    /* Array access with complex index */
    g_array[iteration % 256] = result;
    
    return result;
}

/* ========== Main function ========== */
int main() {
    int total = 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 3 + 7;
    }
    
    /* Perform multiple iterations with different patterns */
    for (int i = 0; i < 100; i++) {
        g_volatile_counter++;
        
        /* Vary the volatile flag to change control flow */
        g_volatile_flag = (g_volatile_flag * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call combined operations */
        int res = combined_operations(i);
        
        /* Mix results to prevent dead code elimination */
        total = (total * 31 + res) & 0xFFFFFF;
        
        /* Occasionally call individual pattern functions directly */
        if (i % 13 == 0) {
            uint32_t temp = total;
            set_low_16_part(&temp, i & 0xFFFF);
            total ^= temp;
        }
        
        if (i % 17 == 0) {
            union SubregUnion u;
            u.dword = g_large_value + i;
            total += access_via_subreg(&u);
        }
    }
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", total);
    
    /* Additional forced patterns before exit */
    struct BitFieldStruct final_bfs;
    final_bfs.low8 = total & 0xFF;
    final_bfs.mid16 = (total >> 8) & 0xFFFF;
    final_bfs.high8 = (total >> 24) & 0xFF;
    
    volatile int final_check = extract_bitfield_8_16(&g_bitfield_source) +
                              final_bfs.mid16;
    
    return total & 0xFF;
}
