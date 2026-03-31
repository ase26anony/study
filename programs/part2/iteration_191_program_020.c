/* resource_patterns.c - Generate RTL patterns for resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;

/* ===== ZERO_EXTRACT patterns ===== */

/* Pattern 1: Bit-field extraction using shift and mask */
int extract_bits_ze1(volatile unsigned int *p) {
    /* Should generate ZERO_EXTRACT: extract bits 8-15 */
    return (*p >> 8) & 0xFF;
}

/* Pattern 2: Struct with bit-fields */
struct BitFieldStruct {
    unsigned int field1 : 4;
    unsigned int field2 : 8;
    unsigned int field3 : 12;
    unsigned int field4 : 8;
};

int extract_bits_ze2(struct BitFieldStruct *bfs) {
    /* Taking address of bit-field member may generate ZERO_EXTRACT */
    unsigned int val = bfs->field2;
    return val + bfs->field3;
}

/* Pattern 3: Multiple extractions in sequence */
unsigned int extract_bits_ze3(volatile unsigned long long *p) {
    unsigned long long val = *p;
    unsigned int part1 = (val >> 0) & 0xFF;
    unsigned int part2 = (val >> 8) & 0xFF;
    unsigned int part3 = (val >> 16) & 0xFF;
    unsigned int part4 = (val >> 24) & 0xFF;
    return part1 + part2 + part3 + part4;
}

/* ===== STRICT_LOW_PART patterns ===== */

/* Pattern 1: Writing to low byte of integer */
void set_low_byte_slp1(volatile unsigned int *p, unsigned char v) {
    /* May generate STRICT_LOW_PART for low-byte write */
    *p = (*p & ~0xFF) | v;
}

/* Pattern 2: Cast and assignment to smaller type */
int set_low_part_slp2(void) {
    int32_t x = 0x12345678;
    /* Writing to 16-bit portion */
    *(int16_t*)&x = 0xABCD;
    return x;
}

/* Pattern 3: Inline assembly for low-part write (x86 specific) */
void set_low_part_asm(volatile unsigned int *p) {
    /* Assembly that writes only low 16 bits */
    __asm__ volatile (
        "movw %0, %%ax\n\t"
        "movw %%ax, %1\n\t"
        : 
        : "r"((unsigned short)0x1234), "m"(*p)
        : "ax"
    );
}

/* ===== SUBREG patterns ===== */

/* Pattern 1: Union for type aliasing */
union SubregUnion {
    int32_t full;
    int16_t halves[2];
    int8_t bytes[4];
};

int subreg_access_union(void) {
    union SubregUnion u;
    u.full = 0;
    /* Access through smaller type */
    u.halves[0] = 0x1234;
    u.bytes[2] = 0xAB;
    return u.full;
}

/* Pattern 2: Pointer casting between different sizes */
int subreg_pointer_cast(void) {
    long long ll = 0x1122334455667788ULL;
    /* Cast to access part of larger type */
    int i = *(int*)&ll;
    short s = *(short*)((char*)&ll + 2);
    return i + s;
}

/* Pattern 3: Mixed-size operations */
int subreg_mixed_ops(int32_t a, int16_t b) {
    /* Operations mixing different sizes */
    int32_t result = a + b;  /* b may need SUBREG promotion */
    return result;
}

/* ===== Complex MEM patterns ===== */

/* Pattern 1: Array with complex indexing */
int mem_complex_index(int *base, int index1, int index2) {
    /* Complex address calculation */
    return base[index1 + index2 * 4];
}

/* Pattern 2: Struct with array access */
struct MemStruct {
    int arr[100];
    int pad;
    int other[50];
};

int mem_struct_access(struct MemStruct *ms, int i, int j) {
    /* Multiple struct field accesses */
    int val1 = ms->arr[i];
    int val2 = ms->other[j];
    ms->pad = val1 + val2;
    return ms->pad;
}

/* Pattern 3: Pointer arithmetic with multiple bases */
int mem_pointer_arithmetic(int *base1, int *base2, int offset) {
    int *p1 = base1 + offset;
    int *p2 = base2 + (offset * 2);
    return *p1 + *p2;
}

/* ===== Combined patterns in complex control flow ===== */

int combined_patterns(volatile int flag) {
    int result = 0;
    static int data[256];
    struct BitFieldStruct bfs = {0};
    union SubregUnion u;
    
    /* Initialize data */
    for (int i = 0; i < 256; i++) {
        data[i] = i * 3;
    }
    
    /* Complex control flow with multiple patterns */
    for (int i = 0; i < 100; i++) {
        if (flag & 0x1) {
            /* ZERO_EXTRACT pattern */
            volatile unsigned int *p = (volatile unsigned int*)&data[i % 256];
            result += extract_bits_ze1(p);
        }
        
        if (flag & 0x2) {
            /* STRICT_LOW_PART pattern */
            set_low_byte_slp1((volatile unsigned int*)&data[(i + 1) % 256], 
                             (unsigned char)i);
        }
        
        if (flag & 0x4) {
            /* SUBREG pattern */
            u.full = data[i % 256];
            u.halves[0] = i;
            result += u.full;
        }
        
        if (flag & 0x8) {
            /* Complex MEM pattern */
            result += mem_complex_index(data, i % 64, (i / 64) % 4);
        }
        
        /* Mix operations to create complex RTL */
        flag = (flag * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    return result;
}

/* ===== Main function with multiple passes ===== */

int main(void) {
    int final_result = 0;
    
    /* Initialize test data */
    int test_array[1024];
    struct MemStruct ms;
    volatile unsigned int volatile_data = 0x89ABCDEF;
    
    for (int i = 0; i < 1024; i++) {
        test_array[i] = i * 7;
    }
    
    /* Call pattern functions in sequence */
    final_result += extract_bits_ze1(&volatile_data);
    
    struct BitFieldStruct bfs = {1, 255, 4095, 127};
    final_result += extract_bits_ze2(&bfs);
    
    volatile unsigned long long big_val = 0xFEDCBA9876543210ULL;
    final_result += extract_bits_ze3(&big_val);
    
    set_low_byte_slp1(&volatile_data, 0x42);
    final_result += set_low_part_slp2();
    
    final_result += subreg_access_union();
    final_result += subreg_pointer_cast();
    final_result += subreg_mixed_ops(1000, 500);
    
    final_result += mem_complex_index(test_array, 10, 20);
    final_result += mem_struct_access(&ms, 5, 10);
    final_result += mem_pointer_arithmetic(test_array, test_array + 512, 100);
    
    /* Combined patterns with volatile control */
    for (int i = 0; i < 10; i++) {
        g_volatile_counter++;
        final_result += combined_patterns(g_volatile_counter | g_volatile_flag);
        
        /* Alternate between different flag patterns */
        if (i % 3 == 0) {
            set_low_part_asm(&volatile_data);
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Final result: %d\n", final_result);
    
    return final_result & 0xFF;
}
