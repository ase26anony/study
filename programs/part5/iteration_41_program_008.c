/* test_resource_coverage.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent constant folding and dead code elimination */
volatile int g_seed = 42;

/* Function to generate pseudo-random values to prevent optimization */
static int get_pseudo_random(int max) {
    g_seed = (g_seed * 1103515245 + 12345) & 0x7fffffff;
    return g_seed % max;
}

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Bit-field structure for ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 3;
    unsigned int field4 : 17;
};

/* Function to test ZERO_EXTRACT patterns */
static int test_zero_extract(int iterations) {
    struct bitfield_struct bf = {0};
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: Bit-field extraction from structure */
        unsigned int val = get_pseudo_random(256);
        bf.field1 = val & 0x1F;        /* 5 bits */
        bf.field2 = (val >> 5) & 0x7F; /* 7 bits */
        
        /* This should generate ZERO_EXTRACT for comparison */
        if (bf.field1 == 10) {
            sum += 1;
        }
        
        /* Pattern 2: Explicit bit-field extraction */
        unsigned int x = get_pseudo_random(0xFFFF);
        
        /* Extract 8-bit field from position 4 */
        unsigned int extracted = (x >> 4) & 0xFF;
        sum += extracted;
        
        /* Pattern 3: Multiple bit-field operations */
        bf.field3 = (x >> 12) & 0x7;
        bf.field4 = (x >> 8) & 0x1FFFF;
        
        /* Combined extraction and arithmetic */
        unsigned int combined = (bf.field2 << 3) | bf.field3;
        sum += combined;
    }
    
    return sum;
}

/* ==================== STRICT_LOW_PART patterns ==================== */

/* Function to test STRICT_LOW_PART patterns */
static int test_strict_low_part(int iterations) {
    int sum = 0;
    volatile short vs;  /* volatile to prevent optimization */
    
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: Partial register updates with char/short */
        int temp = get_pseudo_random(0xFFFF);
        
        /* This assignment to short may generate STRICT_LOW_PART */
        short s = (short)(temp & 0xFFFF);
        sum += s;
        
        /* Pattern 2: Volatile write to partial register */
        vs = (short)(temp >> 8);
        sum += vs;
        
        /* Pattern 3: Pointer to volatile short */
        volatile short* ptr = &vs;
        *ptr = (short)(temp & 0xFF);
        sum += *ptr;
        
        /* Pattern 4: Array of shorts with partial updates */
        short arr[4] = {0};
        for (int j = 0; j < 4; j++) {
            arr[j] = (short)((temp >> (j * 4)) & 0xF);
            sum += arr[j];
        }
    }
    
    return sum;
}

/* ==================== SUBREG patterns ==================== */

/* Union for type-punning (SUBREG generation) */
union type_pun {
    uint32_t full;
    uint16_t halves[2];
    uint8_t bytes[4];
};

/* Function to test SUBREG patterns */
static int test_subreg(int iterations) {
    union type_pun u;
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: Union-based type punning */
        u.full = get_pseudo_random(0xFFFFFFFF);
        
        /* Access sub-parts through union - should generate SUBREG */
        sum += u.halves[0];
        sum += u.halves[1];
        sum += u.bytes[2];
        
        /* Pattern 2: Cast between different integer sizes */
        uint32_t val = get_pseudo_random(0xFFFFFFFF);
        uint16_t low = (uint16_t)(val & 0xFFFF);
        uint16_t high = (uint16_t)((val >> 16) & 0xFFFF);
        
        sum += low;
        sum += high;
        
        /* Pattern 3: SIMD-like operations on packed data */
        uint32_t packed = (low << 16) | high;
        uint8_t b0 = (packed >> 24) & 0xFF;
        uint8_t b1 = (packed >> 16) & 0xFF;
        
        sum += b0 + b1;
    }
    
    return sum;
}

/* ==================== Complex memory references ==================== */

/* Structure with mixed types for complex addressing */
struct mixed_data {
    int array[8];
    short shorts[16];
    char bytes[32];
};

/* Function combining all patterns with memory references */
static int test_complex_memory(int iterations) {
    struct mixed_data data;
    int sum = 0;
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < 8; i++) {
        data.array[i] = get_pseudo_random(0xFFFF);
    }
    for (int i = 0; i < 16; i++) {
        data.shorts[i] = (short)get_pseudo_random(0xFFFF);
    }
    
    for (int i = 0; i < iterations; i++) {
        /* Complex memory addressing with bit-field extraction */
        int idx = get_pseudo_random(8);
        
        /* Memory reference with bit-field extract */
        unsigned int val = data.array[idx];
        unsigned int field = (val >> 8) & 0xFF;  /* ZERO_EXTRACT from memory */
        sum += field;
        
        /* SUBREG access to memory */
        short* sptr = &data.shorts[idx * 2];
        sum += *sptr;  /* May involve SUBREG for memory access */
        
        /* Partial write to memory (potential STRICT_LOW_PART) */
        volatile short* vsptr = (volatile short*)&data.shorts[idx];
        *vsptr = (short)(val & 0xFFFF);
        sum += *vsptr;
        
        /* Union type-punning with memory */
        union type_pun* uptr = (union type_pun*)&data.array[idx % 4];
        uptr->halves[0] = (uint16_t)(val & 0xFFFF);
        sum += uptr->halves[1];
    }
    
    return sum;
}

/* ==================== Main test driver ==================== */

int main(int argc, char** argv) {
    int iterations = 1000;
    int total_sum = 0;
    
    /* Use command line argument or default */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    printf("Testing resource coverage patterns with %d iterations\n", iterations);
    
    /* Run all test patterns */
    total_sum += test_zero_extract(iterations);
    printf("Zero extract test completed\n");
    
    total_sum += test_strict_low_part(iterations);
    printf("Strict low part test completed\n");
    
    total_sum += test_subreg(iterations);
    printf("Subreg test completed\n");
    
    total_sum += test_complex_memory(iterations);
    printf("Complex memory test completed\n");
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: %d\n", total_sum);
    
    return total_sum & 0xFF;  /* Return non-constant result */
}
