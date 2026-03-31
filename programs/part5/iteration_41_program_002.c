/* test_resource_patterns.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent constant propagation */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT patterns ========== */
struct BitFieldStruct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 3;
    unsigned int field4 : 17;
};

/* Function to force ZERO_EXTRACT in various contexts */
unsigned int test_zero_extract(struct BitFieldStruct *bfs, unsigned int *arr, int n) {
    unsigned int sum = 0;
    
    /* Pattern 1: Bit-field extraction from struct */
    for (int i = 0; i < n; i++) {
        /* Multiple bit-field accesses that should generate ZERO_EXTRACT */
        sum += bfs[i].field1;
        sum += bfs[i].field2 << 3;
        sum += (bfs[i].field3 & 0x3) * 2;
    }
    
    /* Pattern 2: Explicit bit-field extraction from integers */
    for (int i = 0; i < n; i++) {
        /* These should generate ZERO_EXTRACT RTL */
        unsigned int val = arr[i];
        sum += (val >> 8) & 0xFF;        /* Extract byte 1 */
        sum += (val >> 16) & 0x3F;       /* Extract bits 16-21 */
        sum += (val & 0x1F) << 2;        /* Extract low 5 bits, shift */
    }
    
    /* Pattern 3: Bit-field comparison */
    for (int i = 0; i < n; i++) {
        if (bfs[i].field1 == 3) {
            sum += 100;
        }
        if ((bfs[i].field2 & 0x20) != 0) {
            sum += 200;
        }
    }
    
    /* Pattern 4: Combined operations with bit-fields */
    for (int i = 0; i < n; i++) {
        /* Complex expression that might generate multiple ZERO_EXTRACT */
        unsigned int temp = (bfs[i].field1 << bfs[i].field3) | 
                           (bfs[i].field2 >> (7 - bfs[i].field3));
        sum += temp & 0xFF;
    }
    
    return sum;
}

/* ========== STRICT_LOW_PART patterns ========== */
short test_strict_low_part(short *sarr, char *carr, int n) {
    short total = 0;
    
    /* Pattern 1: Partial register updates through pointers */
    for (int i = 0; i < n; i++) {
        /* Writing to short pointer - may generate STRICT_LOW_PART */
        volatile short *vs = &sarr[i];
        *vs = (short)(g_volatile_seed + i);
        total += *vs;
    }
    
    /* Pattern 2: char operations that get promoted */
    for (int i = 0; i < n; i++) {
        /* char operations in registers with partial writes back */
        char c = carr[i];
        c = (c + 1) & 0x7F;  /* Keep in 7-bit range */
        carr[i] = c;         /* Partial write of low 8 bits */
        total += c;
    }
    
    /* Pattern 3: Mixed-size operations */
    for (int i = 0; i < n; i++) {
        int temp = sarr[i];      /* Load 16-bit into 32-bit register */
        temp = temp * 3 + 7;     /* Operate in full register */
        sarr[i] = (short)temp;   /* Store back only low 16 bits - STRICT_LOW_PART */
        total += (short)temp;
    }
    
    /* Pattern 4: Inline assembly for byte register access (x86 specific) */
    for (int i = 0; i < n && i < 4; i++) {
        unsigned char byte_val;
        /* This asm should generate STRICT_LOW_PART for partial reg update */
        __asm__ volatile (
            "movb %1, %0\n\t"
            : "=q" (byte_val)    /* q = a, b, c, d registers (byte-accessible) */
            : "r" ((unsigned char)(g_volatile_seed + i))
            : /* no clobber */
        );
        total += byte_val;
    }
    
    return total;
}

/* ========== SUBREG patterns ========== */
union TypePun {
    uint32_t full;
    uint16_t halves[2];
    uint8_t bytes[4];
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

int test_subreg(union TypePun *unions, int *arr, int n) {
    int sum = 0;
    
    /* Pattern 1: Union-based type punning */
    for (int i = 0; i < n; i++) {
        unions[i].full = arr[i];
        
        /* Access sub-parts - should generate SUBREG */
        sum += unions[i].halves[0];  /* Low 16 bits */
        sum += unions[i].halves[1];  /* High 16 bits */
        sum += unions[i].bytes[2];   /* Third byte */
    }
    
    /* Pattern 2: Casting between different integer sizes */
    for (int i = 0; i < n; i++) {
        uint32_t val = arr[i];
        
        /* Multiple SUBREG operations */
        uint16_t low16 = (uint16_t)val;           /* Truncate to 16 bits */
        uint16_t high16 = (uint16_t)(val >> 16);  /* Extract high 16 bits */
        uint8_t low8 = (uint8_t)val;              /* Truncate to 8 bits */
        
        sum += low16 + high16 + low8;
    }
    
    /* Pattern 3: Packed structure simulation */
    for (int i = 0; i < n; i++) {
        /* Simulate packed data in a register */
        uint32_t packed = (arr[i] & 0xFFFF) | ((arr[i + 1] & 0xFFFF) << 16);
        
        /* Extract parts - should use SUBREG */
        uint16_t part1 = packed & 0xFFFF;
        uint16_t part2 = (packed >> 16) & 0xFFFF;
        
        sum += part1 + part2;
    }
    
    /* Pattern 4: Memory access with SUBREG */
    for (int i = 0; i < n; i++) {
        /* Complex addressing with type conversion */
        int *ptr = &arr[i];
        short *sptr = (short *)ptr;  /* Cast pointer to different type */
        
        /* Access through cast pointer - may involve SUBREG */
        sum += sptr[0] + sptr[1];    /* Access as two shorts */
    }
    
    return sum;
}

/* ========== Combined patterns with memory references ========== */
int test_combined_patterns(struct BitFieldStruct *bfs, short *sarr, 
                          union TypePun *unions, int *arr, int n) {
    int total = 0;
    
    /* Mix different patterns in loops to prevent optimization */
    for (int i = 0; i < n; i++) {
        /* ZERO_EXTRACT pattern with memory */
        unsigned int val = arr[i];
        int extracted = (val >> bfs[i].field1) & ((1 << bfs[i].field2) - 1);
        
        /* STRICT_LOW_PART pattern */
        short temp_short = sarr[i];
        temp_short = (temp_short * 3 + extracted) & 0x7FFF;
        sarr[i] = temp_short;
        
        /* SUBREG pattern */
        unions[i].full = val;
        int subpart = unions[i].parts.low + unions[i].parts.high;
        
        /* Complex memory addressing */
        total += extracted + temp_short + subpart + arr[(i + 1) % n];
    }
    
    return total;
}

/* ========== Main test driver ========== */
int main(int argc, char **argv) {
    const int N = 100;
    int checksum = 0;
    
    /* Initialize test data with non-constant values */
    struct BitFieldStruct *bfs = malloc(N * sizeof(struct BitFieldStruct));
    short *sarr = malloc(N * sizeof(short));
    union TypePun *unions = malloc(N * sizeof(union TypePun));
    int *arr = malloc(N * sizeof(int));
    
    /* Use volatile to prevent compile-time optimization */
    volatile int seed = g_volatile_seed;
    
    for (int i = 0; i < N; i++) {
        /* Initialize with pattern that's not trivially optimizable */
        bfs[i].field1 = (i + seed) & 0x1F;
        bfs[i].field2 = (i * 3 + seed) & 0x7F;
        bfs[i].field3 = (i + seed * 2) & 0x7;
        bfs[i].field4 = (i * 5 + seed * 3) & 0x1FFFF;
        
        sarr[i] = (short)((i * 7 + seed) & 0x7FFF);
        arr[i] = (i * 11 + seed * 5) ^ 0x12345678;
    }
    
    /* Run individual pattern tests */
    checksum += test_zero_extract(bfs, arr, N);
    checksum += test_strict_low_part(sarr, (char *)arr, N);
    checksum += test_subreg(unions, arr, N);
    
    /* Run combined test */
    checksum += test_combined_patterns(bfs, sarr, unions, arr, N);
    
    /* Use results to affect return value */
    printf("Resource pattern test checksum: %d\n", checksum);
    
    free(bfs);
    free(sarr);
    free(unions);
    free(arr);
    
    return (checksum & 0xFF) == 0 ? 0 : 1;
}
