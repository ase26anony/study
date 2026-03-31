/* test_resource_coverage.c */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Prevent dead code elimination */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT patterns ========== */
struct BitFieldStruct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 3;
    unsigned int field4 : 17;
};

/* Function to trigger ZERO_EXTRACT from bit-field operations */
unsigned int test_zero_extract(struct BitFieldStruct *bfs, unsigned int *arr, int n) {
    unsigned int sum = 0;
    
    /* Bit-field extraction from structure */
    for (int i = 0; i < n; i++) {
        /* These generate ZERO_EXTRACT for bit-field reads */
        sum += bfs[i].field1;
        sum += bfs[i].field2 << 3;
        sum += (bfs[i].field3 & 0x7) * 2;
        
        /* Complex bit-field comparison */
        if (bfs[i].field4 > 1000) {
            sum += bfs[i].field4;
        }
    }
    
    /* Explicit bit-field extraction from integers */
    for (int i = 0; i < n; i++) {
        /* This should generate ZERO_EXTRACT: (x >> shift) & mask */
        unsigned int val = arr[i];
        unsigned int low_bits = (val >> 0) & 0x1F;      /* bits 0-4 */
        unsigned int mid_bits = (val >> 8) & 0xFF;      /* bits 8-15 */
        unsigned int high_bits = (val >> 16) & 0x7FFF;  /* bits 16-30 */
        
        sum += low_bits + mid_bits + high_bits;
    }
    
    /* Bit-field assignment - may generate ZERO_EXTRACT in SET_DEST */
    for (int i = 0; i < n; i++) {
        bfs[i].field1 = (arr[i] & 0x1F);
        bfs[i].field2 = ((arr[i] >> 5) & 0x7F);
    }
    
    return sum;
}

/* ========== STRICT_LOW_PART patterns ========== */
short test_strict_low_part(short *sarr, char *carr, int n) {
    short total = 0;
    
    /* Partial register updates through pointers */
    for (int i = 0; i < n; i++) {
        /* Writing to short pointer - may generate STRICT_LOW_PART */
        sarr[i] = (short)(g_volatile_seed + i);
        
        /* Reading back and using */
        total += sarr[i];
    }
    
    /* char operations that may promote to int then write back low part */
    for (int i = 0; i < n; i++) {
        /* char assignment with arithmetic */
        char temp = (char)(g_volatile_seed * i);
        carr[i] = temp + 1;  /* May generate STRICT_LOW_PART for byte store */
        
        /* Use result to prevent elimination */
        total += carr[i];
    }
    
    /* volatile short pointer - forces partial store */
    volatile short *vsptr = sarr;
    for (int i = 0; i < n; i += 2) {
        *vsptr = (short)(i * 3);
        vsptr++;
    }
    
    return total;
}

/* Inline assembly for STRICT_LOW_PART (x86 specific) */
#ifdef __x86_64__
void test_strict_low_part_asm(short *sptr, char *cptr) {
    short sval;
    char cval;
    
    /* Byte register operation */
    __asm__ volatile (
        "movb %1, %0\n\t"
        : "=q"(cval)   /* q constraint = a, b, c, or d register */
        : "r"((char)g_volatile_seed)
        : "cc"
    );
    *cptr = cval;
    
    /* Word register operation */
    __asm__ volatile (
        "movw %1, %0\n\t"
        : "=r"(sval)   /* word operation in general register */
        : "r"((short)(g_volatile_seed + 1))
        : "cc"
    );
    *sptr = sval;
}
#endif

/* ========== SUBREG patterns ========== */
union TypePunningUnion {
    uint32_t full;
    uint16_t halves[2];
    uint8_t bytes[4];
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

int test_subreg(union TypePunningUnion *unions, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Initialize with pattern */
        unions[i].full = g_volatile_seed + i * 0x100;
        
        /* Access through different views - generates SUBREG */
        sum += unions[i].halves[0];      /* low 16 bits */
        sum += unions[i].halves[1];      /* high 16 bits */
        sum += unions[i].bytes[1] << 8;  /* byte 1 */
        
        /* Cast between types of different sizes */
        uint16_t low_part = (uint16_t)(unions[i].full & 0xFFFF);
        uint16_t high_part = (uint16_t)((unions[i].full >> 16) & 0xFFFF);
        
        sum += low_part + high_part;
        
        /* Structure member access */
        unions[i].parts.low = (uint16_t)(sum & 0xFFFF);
        unions[i].parts.high = (uint16_t)((sum >> 16) & 0xFFFF);
    }
    
    /* SIMD-like operations using unions */
    for (int i = 0; i < n - 1; i++) {
        /* Pack two 16-bit values into 32-bit */
        uint32_t packed = (unions[i].halves[1] << 16) | unions[i+1].halves[0];
        unions[i].full = packed;
        
        /* Extract and swap bytes */
        uint8_t temp = unions[i].bytes[0];
        unions[i].bytes[0] = unions[i].bytes[3];
        unions[i].bytes[3] = temp;
        
        sum += unions[i].full;
    }
    
    return sum;
}

/* ========== Complex memory references ========== */
struct ComplexStruct {
    int data[16];
    struct BitFieldStruct bfs;
    union TypePunningUnion uni;
    short svalues[8];
    char cvalues[16];
};

int test_complex_memory(struct ComplexStruct *cs, int index) {
    int result = 0;
    
    /* Array access with index calculation */
    for (int i = 0; i < 8; i++) {
        /* Complex addressing: base + index * scale */
        result += cs->data[i * 2 + index];
        
        /* Bit-field in structure member */
        cs->bfs.field1 = (cs->data[i] & 0x1F);
        result += cs->bfs.field1;
        
        /* SUBREG access through union in structure */
        result += cs->uni.halves[i % 2];
        
        /* STRICT_LOW_PART through structure member */
        cs->svalues[i] = (short)(result & 0xFFFF);
        cs->cvalues[i] = (char)(result & 0xFF);
    }
    
    /* Pointer arithmetic with type punning */
    short *sptr = cs->svalues;
    for (int i = 0; i < 8; i++) {
        /* Memory reference with pointer increment */
        result += *sptr++;
        
        /* Cast pointer to different type */
        char *cptr = (char *)&cs->data[i];
        result += cptr[0] + cptr[1];
    }
    
    return result;
}

/* ========== Main test driver ========== */
int main(int argc, char **argv) {
    /* Use argc to prevent constant folding */
    int size = (argc > 1) ? 64 : 32;
    if (size < 10) size = 64;
    
    /* Initialize test data */
    struct BitFieldStruct bfs_array[64];
    unsigned int int_array[64];
    short short_array[64];
    char char_array[64];
    union TypePunningUnion union_array[64];
    struct ComplexStruct cs;
    
    /* Initialize with non-constant patterns */
    for (int i = 0; i < 64; i++) {
        int_array[i] = g_volatile_seed + i * 3;
        short_array[i] = (short)(g_volatile_seed + i * 5);
        char_array[i] = (char)(g_volatile_seed + i * 7);
        
        bfs_array[i].field1 = (i & 0x1F);
        bfs_array[i].field2 = ((i * 2) & 0x7F);
        bfs_array[i].field3 = ((i * 3) & 0x07);
        bfs_array[i].field4 = (i * 100);
        
        union_array[i].full = g_volatile_seed + i * 0x123;
    }
    
    memset(&cs, 0, sizeof(cs));
    for (int i = 0; i < 16; i++) {
        cs.data[i] = g_volatile_seed + i * 11;
    }
    
    /* Run all tests */
    unsigned int total = 0;
    
    total += test_zero_extract(bfs_array, int_array, size % 64);
    total += test_strict_low_part(short_array, char_array, size % 64);
    total += test_subreg(union_array, size % 64);
    total += test_complex_memory(&cs, size % 8);
    
#ifdef __x86_64__
    test_strict_low_part_asm(&short_array[0], &char_array[0]);
    total += short_array[0] + char_array[0];
#endif
    
    /* Use result to prevent dead code elimination */
    printf("Result checksum: %u\n", total);
    
    /* Return non-zero if tests seem to have run */
    return (total > 0) ? 0 : 1;
}
