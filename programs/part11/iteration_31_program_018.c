/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to potentially generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bit_packed {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int value : 12;
    unsigned int : 0;  /* Force alignment boundary */
    unsigned int data  : 16;
} __attribute__((packed));

/* Union for accessing same memory as different types */
union mixed_access {
    struct bit_packed bits;
    unsigned int word;
    unsigned short half[2];
};

/* Complex structure with nested bit-fields */
struct complex_bits {
    unsigned char a : 2;
    unsigned char b : 3;
    unsigned char c : 3;
    unsigned short d : 9;
    unsigned int e : 7;
    unsigned int f : 25;
};

/* Function to force generation of SUBREG patterns for multi-word operations */
static long long process_multiword(volatile long long a, volatile long long b) {
    /* Operations that may generate SUBREG on 32-bit targets */
    long long result = a + b;
    result = result ^ (a << 3);
    result = result | (b >> 5);
    
    /* Comparison that might split into high/low parts */
    if ((a & 0xFFFFFFFF) > (b & 0xFFFFFFFF)) {
        result = result & ~0xFF;
    }
    
    return result;
}

/* Function with complex memory addressing */
static void process_array(volatile int *arr, int size, int stride) {
    volatile int temp;
    
    /* Complex addressing mode that might generate MEM with complex XEXP */
    for (int i = 0; i < size; i += stride) {
        /* Non-trivial index calculation */
        int idx = (i * 7 + 3) % size;
        
        /* Read-modify-write with bit manipulation */
        temp = arr[idx];
        temp = (temp & 0xFFFF0000) | ((temp & 0xFFFF) ^ 0x55AA);
        arr[idx] = temp;
        
        /* Access with pointer arithmetic */
        *(arr + idx + 1) = *(arr + idx) * 2;
    }
}

/* Function using inline assembly to suggest specific register usage */
static unsigned int asm_bit_ops(unsigned int x, unsigned int y) {
    unsigned int result;
    
    /* Inline asm that might influence RTL generation */
    __asm__ volatile (
        "andl %1, %0\n\t"
        "rorl $3, %0\n\t"
        : "=r" (result)
        : "r" (x), "0" (y)
        : "cc"
    );
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile struct bit_packed bp = {0};
    volatile union mixed_access ma = {0};
    volatile struct complex_bits cb = {0};
    volatile long long ll_var1 = 0x123456789ABCDEF0LL;
    volatile long long ll_var2 = 0xFEDCBA9876543210LL;
    volatile double dbl_var = 3.141592653589793;
    
    /* Array with volatile elements for complex memory access */
    #define ARRAY_SIZE 64
    volatile int data_array[ARRAY_SIZE];
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data_array[i] = i * 3 + 1;
    }
    
    /* Use argc to control loop iterations (prevents optimization) */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    int sum = 0;
    
    /* Main loop with mixed operations */
    for (volatile int i = 0; i < iterations; i++) {
        /* 1. Bit-field operations (potential ZERO_EXTRACT/STRICT_LOW_PART) */
        bp.flag1 = i & 1;
        bp.flag2 = (i >> 1) & 0x7;
        bp.value = (bp.value + i) & 0xFFF;
        bp.data = bp.data ^ (i << 4);
        
        /* Access through union with type punning */
        ma.word = (ma.word << 1) | bp.flag1;
        ma.bits.value = (ma.bits.value + ma.half[0]) & 0xFFF;
        
        /* Complex bit-field structure operations */
        cb.a = (cb.a + 1) & 0x3;
        cb.b = (cb.b ^ cb.a) & 0x7;
        cb.c = (cb.c | cb.b) & 0x7;
        cb.d = (cb.d + i) & 0x1FF;
        cb.e = (cb.e << 1) | (cb.d & 1);
        cb.f = (cb.f ^ (i << 8)) & 0x1FFFFFF;
        
        /* 2. Multi-word operations (potential SUBREG generation) */
        ll_var1 = process_multiword(ll_var1, ll_var2 + i);
        ll_var2 = ll_var2 - (ll_var1 >> 16);
        
        /* Double precision operations (may use multiple registers) */
        dbl_var = dbl_var * 1.01 + (i & 0xF);
        
        /* 3. Complex memory addressing */
        process_array(data_array, ARRAY_SIZE, 3 + (i & 3));
        
        /* 4. Conditional logic based on bit-field results */
        if (bp.flag1) {
            /* When flag1 is set, use different operations */
            ll_var1 = ll_var1 | 0x5555555555555555LL;
            data_array[i % ARRAY_SIZE] = asm_bit_ops(data_array[i % ARRAY_SIZE], i);
        } else {
            /* When flag1 is clear */
            ll_var1 = ll_var1 & 0xAAAAAAAAAAAAAAAALL;
            
            /* Access specific bits using shift/mask (more ZERO_EXTRACT potential) */
            unsigned int temp = data_array[(i * 2) % ARRAY_SIZE];
            unsigned int low_bits = temp & 0xF;
            unsigned int high_bits = (temp >> 28) & 0xF;
            data_array[(i * 2) % ARRAY_SIZE] = (high_bits << 4) | low_bits;
        }
        
        /* Conditional based on comparison of long long parts */
        if ((ll_var1 & 0xFFFFFFFF) < (ll_var2 & 0xFFFFFFFF)) {
            /* Swap high and low parts using bit operations */
            unsigned long long mask_low = ll_var1 & 0xFFFFFFFF;
            unsigned long long mask_high = ll_var1 >> 32;
            ll_var1 = (mask_low << 32) | mask_high;
        }
        
        /* Accumulate results to prevent optimization */
        sum += bp.value + (ll_var1 & 0xFF) + data_array[i % ARRAY_SIZE];
        
        /* Force memory barrier */
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Additional operations to ensure all constructs are used */
    
    /* Pointer casting for potentially unaligned access */
    char *byte_ptr = (char *)&ll_var1;
    for (int j = 0; j < 8; j++) {
        sum += byte_ptr[j];
    }
    
    /* Mixed-type access through pointers */
    unsigned short *short_ptr = (unsigned short *)&ma;
    for (int j = 0; j < sizeof(ma)/sizeof(unsigned short); j++) {
        sum += short_ptr[j];
    }
    
    /* Final aggregation and output */
    printf("Result: %d\n", sum);
    
    /* Return value based on computation */
    return (sum > 0) ? 0 : 1;
}
