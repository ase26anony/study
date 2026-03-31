/* Test program to generate specific RTL patterns for resource.cc coverage */
#include <stdio.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;

/* ===== ZERO_EXTRACT patterns ===== */
struct BitFieldStruct {
    unsigned int field1 : 8;
    unsigned int field2 : 16;
    unsigned int field3 : 8;
};

/* Function 1: Bit-field extraction that should generate ZERO_EXTRACT */
unsigned int extract_bitfield(struct BitFieldStruct *bfs) {
    /* Multiple bit-field accesses */
    unsigned int val1 = bfs->field1;
    unsigned int val2 = bfs->field2;
    unsigned int val3 = bfs->field3;
    
    /* Complex extraction with shifting */
    return (val1 << 16) | (val2 & 0xFFFF) | ((val3 & 0xF) << 24);
}

/* Function 2: Manual bit extraction using shifts */
unsigned int manual_extract(volatile unsigned int *data) {
    /* This should generate ZERO_EXTRACT for the mask operation */
    unsigned int temp = *data;
    return ((temp >> 8) & 0xFF) | ((temp >> 16) & 0xFF00);
}

/* ===== STRICT_LOW_PART patterns ===== */
/* Function 3: Writing to low part of register */
void write_low_part(volatile unsigned int *dest, unsigned char value) {
    /* Writing only low byte - may generate STRICT_LOW_PART */
    *dest = (*dest & ~0xFF) | (value & 0xFF);
}

/* Function 4: Using smaller types to write partial data */
void partial_write(int32_t *target) {
    /* Cast to smaller type to write partial data */
    int16_t *half_ptr = (int16_t *)target;
    half_ptr[0] = 0x1234;  /* Write to low 16 bits */
    half_ptr[1] = 0x5678;  /* Write to high 16 bits */
}

/* ===== SUBREG patterns ===== */
/* Function 5: Union for type punning - should generate SUBREG */
union TypePunningUnion {
    int64_t full;
    struct {
        int32_t low;
        int32_t high;
    } parts;
    int16_t quarters[4];
};

int32_t access_via_subreg(union TypePunningUnion *u) {
    /* Access different parts through different types */
    u->parts.low = 0xDEADBEEF;
    u->quarters[2] = 0xCAFE;
    
    /* Return a mix of parts */
    return u->parts.low + (int32_t)u->quarters[2];
}

/* Function 6: Mixed-size operations */
int64_t mixed_size_ops(int64_t a, int32_t b) {
    /* Operations mixing different sizes */
    int32_t temp = (int32_t)a + b;
    return (int64_t)temp * 2;
}

/* ===== Complex MEM patterns ===== */
/* Function 7: Complex memory addressing */
struct ComplexStruct {
    int array[256];
    int padding[64];
    struct BitFieldStruct bitfields[16];
};

int complex_memory_access(struct ComplexStruct *cs, int idx1, int idx2) {
    /* Complex addressing with multiple calculations */
    int *ptr1 = &cs->array[idx1 * 2];
    int *ptr2 = &cs->array[idx2 * 4];
    
    /* Even more complex: array of structs with bitfields */
    int bitfield_idx = (idx1 + idx2) % 16;
    unsigned int bf_val = cs->bitfields[bitfield_idx].field2;
    
    /* Return with address arithmetic */
    return *ptr1 + *ptr2 + (int)bf_val;
}

/* Function 8: Pointer arithmetic in loop */
int pointer_arithmetic_sum(int *base, int n) {
    int sum = 0;
    int *end = base + n;
    
    /* Complex addressing mode in loop */
    for (int *p = base; p < end; p += 2) {
        sum += *p;
        /* Add some offset access */
        if (p + 1 < end) {
            sum += *(p + 1);
        }
    }
    return sum;
}

/* ===== Main function combining all patterns ===== */
int main(void) {
    int result = 0;
    
    /* Initialize test data */
    struct BitFieldStruct bfs = {0xAA, 0xBBBB, 0xCC};
    union TypePunningUnion u;
    struct ComplexStruct cs;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        cs.array[i] = i * 3;
    }
    for (int i = 0; i < 16; i++) {
        cs.bitfields[i].field1 = i;
        cs.bitfields[i].field2 = i * 100;
        cs.bitfields[i].field3 = i * 10;
    }
    
    /* Use volatile to control flow */
    volatile int iterations = 10;
    
    for (int i = 0; i < iterations; i++) {
        g_volatile_counter++;
        
        /* Mix different patterns based on volatile condition */
        if (g_volatile_flag) {
            /* ZERO_EXTRACT patterns */
            result ^= extract_bitfield(&bfs);
            result ^= manual_extract(&g_volatile_counter);
            
            /* Update bitfields for next iteration */
            bfs.field1 = (bfs.field1 + 1) & 0xFF;
            bfs.field2 = (bfs.field2 + 100) & 0xFFFF;
        } else {
            /* STRICT_LOW_PART patterns */
            unsigned char byte_val = (result & 0xFF);
            write_low_part((volatile unsigned int *)&result, byte_val);
            
            int32_t target = result;
            partial_write(&target);
            result += target;
        }
        
        /* SUBREG patterns (always execute) */
        result += access_via_subreg(&u);
        result += (int)mixed_size_ops(result, i);
        
        /* Complex MEM patterns */
        int idx1 = (result + i) % 128;
        int idx2 = (result * 2) % 64;
        result += complex_memory_access(&cs, idx1, idx2);
        
        /* More complex memory access */
        int base_idx = (result % 200);
        result += pointer_arithmetic_sum(&cs.array[base_idx], 20);
        
        /* Toggle volatile flag occasionally */
        if ((i % 3) == 0) {
            g_volatile_flag = !g_volatile_flag;
        }
    }
    
    /* Ensure all operations contribute to output */
    printf("Final result: %d\n", result);
    return result;
}
