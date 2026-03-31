/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdint.h>

/* Structure with bit-fields to generate ZERO_EXTRACT and STRICT_LOW_PART */
struct bitfield_struct {
    volatile unsigned int field1 : 1;
    volatile unsigned int field2 : 3;
    volatile unsigned int field3 : 12;
    volatile unsigned int field4 : 16;
    volatile unsigned int padding : 0; /* Force alignment */
};

/* Union for accessing same memory as different types */
union data_union {
    volatile uint64_t full;
    struct {
        volatile uint32_t low;
        volatile uint32_t high;
    } parts;
    volatile double as_double;
};

/* Function to force complex addressing modes */
static inline uint32_t complex_index(volatile uint32_t *arr, int idx, int stride) {
    return arr[idx * stride + 3]; /* Non-trivial addressing */
}

/* Function with inline assembly to force specific register usage */
static inline uint32_t asm_low_part(uint64_t val) {
    uint32_t result;
    /* Assembly that operates on low 32 bits of 64-bit value */
    __asm__ volatile (
        "movl %1, %0\n\t"
        "addl $0x1234, %0"
        : "=r" (result)
        : "r" ((uint32_t)val)  /* Cast forces low part extraction */
        : "cc"
    );
    return result;
}

int main(int argc, char *argv[]) {
    volatile struct bitfield_struct bf = {0};
    volatile union data_union data;
    volatile uint64_t big_val = 0x123456789ABCDEF0ULL;
    volatile double dbl_val = 3.141592653589793;
    
    /* Array for complex memory accesses */
    #define ARRAY_SIZE 128
    volatile uint32_t mem_array[ARRAY_SIZE];
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        mem_array[i] = i * 3;
    }
    
    /* Loop counter with volatile to prevent optimization */
    volatile int iterations = (argc > 1) ? 10 : 5;
    uint32_t accumulator = 0;
    
    /* Main loop with mixed operations */
    for (volatile int i = 0; i < iterations; i++) {
        /* 1. Bit-field operations (potential ZERO_EXTRACT/STRICT_LOW_PART) */
        bf.field1 = (i & 1);                    /* Single bit assignment */
        bf.field2 = (bf.field3 >> 4) & 0x7;     /* Bit-field extraction */
        bf.field3 = bf.field3 + bf.field4;      /* Multi-bit field operation */
        
        /* Manual bit extraction using masks (forces ZERO_EXTRACT-like patterns) */
        uint32_t extracted = (bf.field3 >> 2) & 0x3FF;  /* Extract 10 bits */
        bf.field4 = (bf.field4 & ~0xFF) | (extracted & 0xFF); /* Partial update */
        
        /* 2. Multi-word operations (potential SUBREG generation) */
        data.full = big_val;
        data.parts.low += data.parts.high;      /* Mix high/low parts */
        data.parts.high ^= data.parts.low;
        
        /* Double precision operations on 32-bit targets need multiple registers */
        dbl_val = dbl_val * 2.0 - 1.0;
        data.as_double = dbl_val + 0.5;
        
        /* 64-bit arithmetic on potential 32-bit target */
        big_val = big_val * 3 + 0x1000;
        
        /* 3. Complex memory addressing with bit-field derived index */
        int idx = (bf.field2 * 7 + i) % (ARRAY_SIZE / 4);
        uint32_t mem_val = complex_index(mem_array, idx, 4);
        
        /* Modify memory with bit-field controlled mask */
        mem_array[idx * 4 + 3] = mem_val | (1 << bf.field1);
        
        /* 4. Inline assembly forcing low-part operations */
        uint32_t asm_result = asm_low_part(data.full);
        
        /* 5. Conditional branching based on bit-field and multi-word results */
        if ((bf.field3 & 1) || (data.parts.low > data.parts.high)) {
            /* Branch 1: More bit manipulations */
            bf.field4 = (bf.field4 << 1) | bf.field1;
            mem_array[i % ARRAY_SIZE] ^= asm_result;
        } else {
            /* Branch 2: Different operations */
            bf.field3 = bf.field3 & 0x7FF;      /* Strict low 11 bits */
            big_val = (big_val >> 16) | (big_val << 48); /* 64-bit rotate */
        }
        
        /* Switch based on extracted bits */
        switch (extracted & 0x3) {
            case 0:
                dbl_val += 1.0;
                break;
            case 1:
                data.parts.low += mem_val;
                break;
            case 2:
                bf.field2 = (bf.field2 + 1) & 0x7;
                break;
            default:
                /* Force memory store with complex address */
                volatile uint32_t *ptr = &mem_array[(i * 13) % ARRAY_SIZE];
                *ptr = *ptr + bf.field3;
                break;
        }
        
        /* Accumulate results to prevent elimination */
        accumulator += bf.field3 + data.parts.low + (uint32_t)big_val;
        accumulator ^= mem_array[i % ARRAY_SIZE];
    }
    
    /* Final computation using all variables */
    uint32_t result = accumulator;
    result += bf.field1 + (bf.field2 << 8) + (bf.field3 << 16);
    result += (uint32_t)(data.full >> 32) + (uint32_t)data.full;
    result += (uint32_t)dbl_val;
    
    /* Access array with wrap-around to force more complex addressing */
    for (int i = 0; i < 8; i++) {
        result += mem_array[(result + i) % ARRAY_SIZE];
    }
    
    /* Return result to prevent dead code elimination */
    return (int)(result % 1000);
}

/* Additional function to create more RTL patterns during compilation */
static void __attribute__((noinline)) 
additional_patterns(volatile uint64_t *p1, volatile struct bitfield_struct *p2) {
    /* Force memory-to-memory operations with bit-field extraction */
    uint32_t temp = p2->field3;
    p2->field4 = (p2->field4 & 0xFFFF0000) | (temp & 0xFFFF);
    
    /* 64-bit operation that might be split */
    *p1 = (*p1 & 0xFFFFFFFF) | ((uint64_t)temp << 32);
    
    /* Pointer arithmetic with bit-field offset */
    volatile uint32_t *arr_ptr = (volatile uint32_t *)p2;
    arr_ptr[1] = arr_ptr[0] + (p2->field2 << p2->field1);
}

/* Force compiler to generate this function's code */
void __attribute__((constructor)) init_func(void) {
    volatile uint64_t dummy = 0;
    volatile struct bitfield_struct dummy_bf = {0};
    additional_patterns(&dummy, &dummy_bf);
}
