/* resource_patterns.c - Generate RTL patterns for resource.cc coverage */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;

/* ===== ZERO_EXTRACT patterns ===== */
struct BitFieldStruct {
    unsigned int field1 : 8;
    unsigned int field2 : 16;
    unsigned int field3 : 8;
};

/* Function 1: Bit-field extraction that may generate ZERO_EXTRACT */
unsigned int extract_bitfield_1(struct BitFieldStruct *bfs) {
    /* Direct bit-field access */
    unsigned int val = bfs->field2;  /* 16-bit extraction from 32-bit */
    
    /* Complex extraction with shifting */
    volatile unsigned int *p = (volatile unsigned int*)bfs;
    unsigned int combined = (*p >> 8) & 0xFFFF;  /* Should generate ZERO_EXTRACT */
    
    return val + (combined & 0xFF);
}

/* Function 2: Manual bit extraction */
unsigned int extract_bitfield_2(volatile unsigned int *arr, int idx) {
    /* Multiple extractions to increase chances */
    unsigned int x = arr[idx];
    unsigned int y = (x >> 4) & 0xF;      /* 4-bit extract */
    unsigned int z = (x >> 16) & 0xFF;    /* 8-bit extract from upper half */
    unsigned int w = (x >> 24) & 0x7;     /* 3-bit extract from top */
    
    return y + z + w;
}

/* ===== STRICT_LOW_PART patterns ===== */
/* Function 3: Writing to low parts of variables */
void write_low_part_1(volatile unsigned int *dest, unsigned char value) {
    /* Clear low byte and set new value */
    *dest = (*dest & ~0xFF) | value;  /* May generate STRICT_LOW_PART */
}

/* Function 4: Mixed-size assignments */
void write_low_part_2(int32_t *p32, int16_t *p16) {
    /* Writing 16-bit to what might be part of 32-bit */
    *p16 = 0x1234;
    
    /* Follow with 32-bit write to same location */
    if (g_volatile_flag) {
        *p32 = 0xABCD1234;
    }
}

/* ===== SUBREG patterns ===== */
/* Function 5: Union-based type punning */
union MixedTypes {
    int64_t i64;
    int32_t i32[2];
    int16_t i16[4];
    int8_t  i8[8];
};

int32_t subreg_access_1(union MixedTypes *u) {
    /* Access different views of same storage */
    u->i16[1] = 0x5678;      /* Write 16-bit to middle of 64-bit */
    u->i8[3] = 0x9A;         /* Write 8-bit to another position */
    
    /* Read back through different type */
    return u->i32[0] + u->i32[1];
}

/* Function 6: Pointer casting for subreg */
int32_t subreg_access_2(int64_t *pll) {
    /* Cast between different pointer sizes */
    int32_t *p32 = (int32_t*)pll;
    int16_t *p16 = (int16_t*)pll;
    
    p16[1] = 0xDEAD;         /* Write to middle of 64-bit */
    p32[0] = p32[0] + 1;     /* Modify through 32-bit view */
    
    return *p32;
}

/* ===== Complex MEM patterns ===== */
/* Function 7: Complex addressing modes */
struct NestedStruct {
    int matrix[10][10];
    int padding[20];
};

int complex_mem_1(struct NestedStruct *ns, int i, int j, int k) {
    /* Multiple index calculations */
    int idx1 = i * j + k;
    int idx2 = (i << 2) + (j * 3);
    
    /* Complex addressing with multiple operations */
    return ns->matrix[idx1 % 10][idx2 % 10] + 
           ns->padding[(i + j * k) % 20];
}

/* Function 8: Pointer arithmetic with struct */
int complex_mem_2(int *base, int offset1, int offset2, int scale) {
    /* Complex address calculation */
    int *addr = base + (offset1 * scale) / sizeof(int);
    addr = addr + offset2;
    
    /* Conditional access */
    if (g_volatile_flag & 1) {
        return *addr + addr[scale];
    } else {
        return addr[-offset1];
    }
}

/* ===== Combined function with control flow ===== */
/* Function 9: Mix all patterns in one function */
unsigned int combined_patterns(struct BitFieldStruct *bfs, 
                               union MixedTypes *u,
                               struct NestedStruct *ns,
                               int *array) {
    unsigned int result = 0;
    volatile int i;
    
    for (i = 0; i < 3; i++) {
        /* ZERO_EXTRACT patterns */
        if (g_volatile_flag & (1 << i)) {
            result += extract_bitfield_1(bfs);
            result += (result >> (i * 4)) & 0xF;  /* Another extract */
        }
        
        /* STRICT_LOW_PART patterns */
        if (g_volatile_counter++ & 1) {
            write_low_part_1((volatile unsigned int*)&result, i);
        }
        
        /* SUBREG patterns */
        if (i % 2 == 0) {
            result += subreg_access_1(u);
            u->i16[i % 4] = result & 0xFFFF;  /* Low-part write */
        }
        
        /* Complex MEM patterns */
        int idx = (result + i) % 100;
        result += complex_mem_1(ns, idx % 10, (idx / 10) % 10, i);
        
        /* More complex addressing */
        result += complex_mem_2(array, result & 0xF, (result >> 4) & 0xF, i + 1);
    }
    
    return result;
}

/* Main function to drive everything */
int main() {
    /* Initialize test data */
    struct BitFieldStruct bfs = {0xAA, 0xBBCC, 0xDD};
    union MixedTypes u;
    struct NestedStruct ns;
    int array[100];
    
    /* Initialize with pattern */
    for (int i = 0; i < 100; i++) {
        array[i] = i * 3 + 1;
        if (i < 100) {
            ns.matrix[i / 10][i % 10] = i * 2;
            ns.padding[i % 20] = i * 5;
        }
    }
    
    u.i64 = 0x0123456789ABCDEFull;
    
    /* Call pattern functions multiple times */
    unsigned int final_result = 0;
    
    for (int iter = 0; iter < 5; iter++) {
        g_volatile_flag = iter + 1;
        
        /* Individual pattern tests */
        final_result ^= extract_bitfield_1(&bfs);
        final_result += extract_bitfield_2((volatile unsigned int*)array, iter % 10);
        
        write_low_part_1((volatile unsigned int*)&final_result, iter & 0xFF);
        
        final_result += subreg_access_1(&u);
        final_result += subreg_access_2(&u.i64);
        
        final_result += complex_mem_1(&ns, iter, iter * 2, iter * 3);
        final_result += complex_mem_2(array, iter, iter + 1, iter + 2);
        
        /* Combined test */
        final_result += combined_patterns(&bfs, &u, &ns, array);
        
        /* Modify bitfields for next iteration */
        bfs.field2 = (bfs.field2 * 3 + 1) & 0xFFFF;
        u.i16[iter % 4] = final_result & 0xFFFF;
    }
    
    /* Use result to prevent optimization */
    printf("Result: 0x%08X\n", final_result);
    
    return (int)(final_result & 0x7FFFFFFF);
}
