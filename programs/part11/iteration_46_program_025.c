/* test_resource_marking.c
 * Designed to trigger specific RTL patterns in GCC's resource.cc:
 * - ZERO_EXTRACT and STRICT_LOW_PART in SET_DEST
 * - SUBREG in SET_DEST  
 * - MEM_P with complex addressing
 */

#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep computations in registers */
#define KEEP_REGISTER __attribute__((noinline))

/* Test 1: Bitfield operations for ZERO_EXTRACT/STRICT_LOW_PART */
KEEP_REGISTER
static int test_bitfield_ops(void) {
    volatile struct {
        unsigned int field4 : 4;    /* Likely ZERO_EXTRACT */
        unsigned int field8 : 8;    /* For sub-byte extraction */
        unsigned int field16 : 16;  /* For word extraction */
    } bf = {0};
    
    register int a = 0x12345678;
    register int b = 0x9ABCDEF0;
    register int c = 0x55555555;
    
    /* Multiple bitfield assignments with complex expressions */
    bf.field4 = (a & 0xF) + (b & 0x7);          /* Should generate ZERO_EXTRACT */
    bf.field8 = ((a >> 8) & 0xFF) ^ ((b >> 4) & 0xFF);
    bf.field16 = (c & 0xFFFF) | ((a >> 16) & 0xFFFF);
    
    /* Use __builtin_popcount on sub-word data */
    int popcnt = __builtin_popcount((unsigned char)bf.field8);
    bf.field4 = popcnt & 0xF;
    
    return bf.field4 + bf.field8 + bf.field16;
}

/* Test 2: SUBREG patterns through type narrowing */
KEEP_REGISTER  
static int test_subreg_patterns(void) {
    volatile short vs1, vs2;
    volatile char vc;
    
    register int r1 = 0x12345678;
    register int r2 = 0x9ABCDEF0;
    register long rl = 0x1122334455667788ULL;
    
    /* Explicit narrowing casts - should create SUBREG in SET_DEST */
    vs1 = (short)r1;                           /* int -> short SUBREG */
    vs2 = (short)(r1 + r2);                    /* Arithmetic then narrowing */
    vc = (char)((r1 & 0xFF) + (r2 & 0xFF));    /* Byte operation */
    
    /* Implicit narrowing through arithmetic overflow */
    char c1 = 100, c2 = 100;
    volatile char result = c1 + c2;            /* char + char -> char truncation */
    
    /* Pointer-based SUBREG */
    int arr[4] = {1, 2, 3, 4};
    volatile short *ps = (volatile short *)arr;
    ps[1] = (short)r1;                         /* Store through short pointer */
    
    return vs1 + vs2 + vc + result + ps[1];
}

/* Test 3: Complex memory addressing for MEM_P(x) path */
KEEP_REGISTER
static int test_complex_addressing(void) {
    int arr[256] = {0};  /* Large enough to prevent simple addressing */
    register int sum = 0;
    
    /* Multi-dimensional style indexing */
    for (int i = 0; i < 16; i++) {
        /* Complex index calculation - forces non-trivial address */
        int idx = (i * 17 + 23) & 0xFF;        /* Non-linear addressing */
        register int val = i * 0x11111111;
        
        /* Store with complex address computation */
        arr[idx] = val;                        /* Should create MEM with complex XEXP */
        
        /* Even more complex: nested array with struct */
        int *ptr = arr + idx;
        ptr[1] = val >> 1;                     /* Pointer arithmetic */
    }
    
    /* Struct with array member */
    struct {
        int header;
        int data[32];
        int footer;
    } s = {0};
    
    register int rval = 0x87654321;
    for (int i = 0; i < 8; i++) {
        /* Access through struct pointer with offset */
        int *dptr = s.data;
        dptr[i * 3 + 2] = rval + i;           /* Complex struct+array indexing */
    }
    
    /* Compute checksum to prevent elimination */
    for (int i = 0; i < 256; i++) {
        sum += arr[i];
    }
    sum += s.data[5];
    
    return sum;
}

/* Test 4: Combined patterns in single assignments */
KEEP_REGISTER
static int test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct {
        unsigned int flags : 12;
        short values[8];
        unsigned int status : 4;
    } combined = {0};
    
    register int a = 0x12345678;
    register int b = 0x9ABCDEF0;
    
    /* Combined: bitfield store with computation */
    combined.flags = ((a & 0xFFF) ^ (b & 0xFFF)) | 0x1;
    
    /* Combined: array store with narrowing and complex index */
    for (int i = 0; i < 8; i++) {
        register int temp = a + i * b;
        /* Narrowing store to array with bitfield in same struct */
        combined.values[(i * 5 + 3) & 7] = (short)temp;
    }
    
    /* Final bitfield store based on array contents */
    combined.status = (combined.values[3] & 0xF) + (combined.values[5] & 0xF);
    
    /* Use inline assembly for direct RTL influence */
    int dummy = 0;
    asm volatile (
        "# Force complex MEM store\n"
        : "=m" (combined.values[2])  /* Memory output constraint */
        : "r" (a)                     /* Register input */
        : "memory"
    );
    
    return combined.flags + combined.status + combined.values[0] + dummy;
}

/* Test 5: Direct inline assembly for specific RTL patterns */
KEEP_REGISTER
static int test_asm_patterns(void) {
    int array[64] = {0};
    register int r1 = 0x11111111;
    register int r2 = 0x22222222;
    
    /* Inline asm with complex memory addressing */
    int idx = (r1 & 0x3F);
    asm volatile (
        "# Complex addressing pattern\n"
        : "=m" (array[idx * 2 + 1])   /* Non-simple memory output */
        : "r" (r2)                    /* Register input */
        : "memory"
    );
    
    /* Another asm with potential SUBREG */
    volatile short vs;
    asm volatile (
        "# Narrow store pattern\n"
        : "=m" (vs)                   /* Short memory output */
        : "r" (r1)                    /* Full register input */
        : "memory"
    );
    
    return array[1] + vs;
}

int main(void) {
    int checksum = 0;
    
    printf("Testing RTL pattern coverage for resource.cc lines 282-290\n");
    
    /* Execute all tests */
    checksum += test_bitfield_ops();      /* ZERO_EXTRACT/STRICT_LOW_PART */
    checksum += test_subreg_patterns();   /* SUBREG in SET_DEST */
    checksum += test_complex_addressing(); /* MEM_P with complex XEXP */
    checksum += test_combined_patterns(); /* Combined patterns */
    checksum += test_asm_patterns();      /* Inline assembly patterns */
    
    printf("Final checksum: %d\n", checksum);
    printf("(Non-zero checksum indicates all tests executed)\n");
    
    return checksum != 0 ? 0 : 1;
}
