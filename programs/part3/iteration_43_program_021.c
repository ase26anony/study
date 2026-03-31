/* gcc -O2 -m32 -fno-strict-aliasing -funroll-loops -fdump-rtl-all -o coverage_test coverage_test.c */

#include <stdint.h>
#include <stdlib.h>

/* Function A: ZERO_EXTRACT and MEM patterns */
static void __attribute__((noinline)) 
pattern_zero_extract_mem(volatile int iter) {
    /* Struct with volatile bit-fields for ZERO_EXTRACT */
    struct S { 
        volatile unsigned int f1:5;
        volatile unsigned int f2:7;
        volatile unsigned int f3:9;
    } s;
    
    /* Complex array access for MEM pattern */
    int arr[10][10];
    volatile int *ptr = (volatile int*)arr;
    
    /* ZERO_EXTRACT: Assignment to volatile bit-field */
    s.f1 = iter & 0x1F;
    s.f2 = (iter >> 5) & 0x7F;
    s.f3 = (iter >> 12) & 0x1FF;
    
    /* MEM: Complex addressing with pointer arithmetic */
    int idx = iter % 100;
    volatile int v = *(ptr + idx + (iter % 3) * 10);
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(s.f1), "r"(v) : "memory");
}

/* Function B: STRICT_LOW_PART and SUBREG patterns */
static void __attribute__((noinline))
pattern_strict_low_part_subreg(volatile int iter) {
    volatile uint32_t var32 = iter;
    volatile uint16_t var16 = iter & 0xFFFF;
    volatile uint8_t var8 = iter & 0xFF;
    
    /* STRICT_LOW_PART: Inline assembly modifying byte-sized part */
    asm volatile (
        "addb $1, %0\n\t"
        "subb $1, %0"
        : "=q"(var8) 
        : "0"(var8) 
        : "cc"
    );
    
    /* Another STRICT_LOW_PART with short */
    asm volatile (
        "addw $1, %0\n\t"
        "subw $1, %0"
        : "=r"(var16)
        : "0"(var16)
        : "cc"
    );
    
    /* SUBREG: Type punning with mixed-size accesses */
    uint32_t combined = (iter << 16) | (iter & 0xFFFF);
    uint16_t *ps = (uint16_t*)&combined;
    *ps = var16;  /* SUBREG store */
    
    /* More SUBREG: Access 32-bit as bytes */
    uint8_t *pb = (uint8_t*)&combined;
    pb[1] = var8;
    pb[3] = var8 ^ 0xFF;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(combined), "r"(var32) : "memory");
}

/* Function C: Mixed patterns with ternary operator */
static void __attribute__((noinline))
pattern_mixed_complex(volatile int iter) {
    /* Volatile struct with bit-fields */
    struct T {
        volatile unsigned int flag:1;
        volatile unsigned int value:15;
        volatile unsigned int pad:16;
    } t;
    
    /* Array for MEM patterns */
    volatile int buffer[64];
    for (int i = 0; i < 64; i++) {
        buffer[i] = iter + i;
    }
    
    /* Complex expression with ternary selecting address */
    volatile int *addr = (iter & 1) ? 
        (volatile int*)&t.value :  /* Bit-field address */
        &buffer[iter % 64];        /* Array element address */
    
    /* ZERO_EXTRACT pattern on selected address */
    if (addr == (volatile int*)&t.value) {
        t.value = iter & 0x7FFF;  /* ZERO_EXTRACT store */
    } else {
        *addr = iter;  /* MEM store */
    }
    
    /* Additional SUBREG pattern */
    uint64_t big_val = (uint64_t)iter * 0x10001ULL;
    uint32_t *p32 = (uint32_t*)&big_val;
    p32[0] ^= p32[1];  /* SUBREG access on 64-bit */
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(t.flag), "r"(big_val), "r"(*addr) : "memory");
}

/* Helper function to create more complex MEM addressing */
static void __attribute__((noinline))
complex_mem_addressing(volatile int iter) {
    /* Multi-dimensional array with complex indexing */
    int matrix[8][8][8];
    volatile int sum = 0;
    
    /* Nested loops create complex MEM RTL with addressing modes */
    for (int i = 0; i < (iter % 4) + 1; i++) {
        for (int j = 0; j < (iter % 3) + 1; j++) {
            for (int k = 0; k < (iter % 2) + 1; k++) {
                /* Complex addressing: matrix[i][j][k] */
                sum += matrix[i][j][k];
                matrix[i][j][k] = iter + i + j + k;
            }
        }
    }
    
    /* Pointer chain for MEM */
    int *p1 = &matrix[0][0][0];
    int **p2 = &p1;
    int ***p3 = &p2;
    
    volatile int val = ***p3;
    asm volatile("" : : "r"(sum), "r"(val) : "memory");
}

int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    volatile int dummy_sum = 0;
    
    /* Initialize some volatile data */
    volatile int counter = 0;
    volatile int toggle = 0;
    
    for (volatile int i = 0; i < iterations; i++) {
        /* Call pattern functions with volatile arguments */
        pattern_zero_extract_mem(i ^ counter);
        pattern_strict_low_part_subreg(i + counter);
        pattern_mixed_complex(i * 3 + counter);
        complex_mem_addressing(i % 16);
        
        /* Update volatile state to prevent optimization */
        counter += (i & 1) ? 1 : -1;
        toggle ^= 1;
        
        /* Accumulate to dummy sum to prevent dead code elimination */
        dummy_sum += counter + toggle;
    }
    
    /* Use dummy_sum to prevent optimization */
    asm volatile("" : : "r"(dummy_sum) : "memory");
    
    return 0;
}
