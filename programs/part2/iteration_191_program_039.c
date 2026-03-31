/* Test program to generate specific RTL patterns for coverage testing */
#include <stdio.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;

/* ========== ZERO_EXTRACT patterns ========== */
struct BitFieldStruct {
    unsigned int field1 : 4;
    unsigned int field2 : 8;
    unsigned int field3 : 12;
    unsigned int field4 : 8;
};

/* Function 1: Bit-field extraction that may generate ZERO_EXTRACT */
unsigned int extract_bitfield_1(volatile struct BitFieldStruct *bf) {
    /* Multiple bit-field accesses */
    unsigned int val = bf->field2;          /* 8-bit extract */
    val |= (bf->field3 << 8);               /* 12-bit extract shifted */
    return val;
}

/* Function 2: Manual bit extraction using shifts */
unsigned int extract_bitfield_2(volatile unsigned int *p) {
    /* This pattern often generates ZERO_EXTRACT in RTL */
    unsigned int val = *p;
    return ((val >> 4) & 0xF) |            /* Extract bits 4-7 */
           ((val >> 16) & 0xFF) << 8;      /* Extract bits 16-23, shift left */
}

/* ========== STRICT_LOW_PART patterns ========== */
/* Function 3: Writing to low part of register */
void write_low_part(volatile uint32_t *dest, uint16_t value) {
    /* This may generate STRICT_LOW_PART when writing 16-bit to 32-bit */
    *(volatile uint16_t *)dest = value;
}

/* Function 4: Byte-wise assignment to integer */
void write_byte_parts(volatile uint32_t *p) {
    /* Separate byte assignments */
    *(volatile uint8_t *)p = 0xAB;
    *((volatile uint8_t *)p + 1) = 0xCD;
}

/* ========== SUBREG patterns ========== */
union MixedTypes {
    int64_t full;
    int32_t half[2];
    int16_t quarter[4];
    int8_t bytes[8];
};

/* Function 5: Access through union - generates SUBREG */
int32_t access_via_subreg(volatile union MixedTypes *u) {
    /* Access smaller parts of larger type */
    u->quarter[1] = 0x1234;
    u->bytes[3] = 0x56;
    return u->half[0] + u->quarter[2];
}

/* Function 6: Pointer casting between types */
int32_t pointer_cast_subreg(volatile int64_t *big) {
    /* Cast to access part of larger object */
    int32_t val = *(volatile int32_t *)big;
    *(volatile int16_t *)((char *)big + 4) = 0x789A;
    return val;
}

/* ========== Complex MEM patterns ========== */
struct ComplexMem {
    int array[64];
    int padding[16];
    struct {
        int x, y, z;
    } point;
};

/* Function 7: Complex memory addressing */
int complex_memory_access(volatile struct ComplexMem *cm, 
                         volatile int idx1, 
                         volatile int idx2) {
    /* Multiple addressing modes */
    int val1 = cm->array[idx1 * 2 + idx2];
    int val2 = cm->array[(idx1 + cm->point.x) & 63];
    cm->point.y = val1 + val2;
    
    /* Pointer arithmetic */
    volatile int *ptr = &cm->array[0];
    ptr += (idx1 ^ idx2) & 15;
    return *ptr + cm->point.z;
}

/* Function 8: Loop with mixed patterns */
int mixed_pattern_loop(volatile int iterations) {
    union MixedTypes u;
    struct BitFieldStruct bf = {0};
    struct ComplexMem cm;
    volatile uint32_t reg = 0xDEADBEEF;
    
    int result = 0;
    
    for (volatile int i = 0; i < iterations && i < 10; i++) {
        /* Alternate between different patterns based on volatile flag */
        if (g_volatile_flag & 1) {
            /* ZERO_EXTRACT pattern */
            result += extract_bitfield_2(&reg);
            
            /* STRICT_LOW_PART pattern */
            write_low_part(&reg, i * 0x100 + 0xCD);
        } else {
            /* SUBREG pattern */
            result += access_via_subreg(&u);
            
            /* Complex MEM pattern */
            result += complex_memory_access(&cm, i, g_volatile_counter);
        }
        
        /* Toggle flag */
        g_volatile_flag ^= (result & 1);
        g_volatile_counter++;
    }
    
    return result;
}

/* Main function with control flow */
int main(void) {
    volatile int seed = 42;
    int total = 0;
    
    /* Initialize structures */
    union MixedTypes u;
    u.full = 0x0123456789ABCDEFULL;
    
    struct BitFieldStruct bf;
    bf.field1 = 0xA;
    bf.field2 = 0xBC;
    bf.field3 = 0xDEF;
    bf.field4 = 0x12;
    
    struct ComplexMem cm;
    for (int i = 0; i < 64; i++) {
        cm.array[i] = i * 3;
    }
    cm.point.x = 5;
    cm.point.y = 10;
    cm.point.z = 15;
    
    volatile uint32_t reg_var = 0x12345678;
    
    /* Execute pattern functions in sequence */
    total += extract_bitfield_1(&bf);
    total += extract_bitfield_2(&reg_var);
    
    write_low_part(&reg_var, 0xABCD);
    write_byte_parts(&reg_var);
    
    total += access_via_subreg(&u);
    total += pointer_cast_subreg(&u.full);
    
    total += complex_memory_access(&cm, seed, seed + 1);
    
    /* Loop with mixed patterns */
    total += mixed_pattern_loop(8);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total & 0xFF;  /* Return non-zero to indicate execution */
}
