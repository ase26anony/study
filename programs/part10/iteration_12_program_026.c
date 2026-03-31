/* test_resources.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Prevent optimization */
#define NOINLINE __attribute__((noinline, optimize("O0")))
#define VOLATILE volatile

/* Opaque function to prevent constant propagation */
extern int get_index(void);
extern void* get_ptr(void);

/* Global volatile variables to force memory operations */
VOLATILE int g_index = 0;
VOLATILE void* g_ptr = NULL;

/* ========== ZERO_EXTRACT patterns ========== */
NOINLINE unsigned test_zero_extract(void) {
    /* Bitfield structure that may generate ZERO_EXTRACT */
    struct bitfields {
        unsigned int flag:3;
        unsigned int value:5;
        unsigned int mode:4;
        unsigned int pad:20;
    } bf;
    
    /* Union for bitfield access */
    union {
        struct bitfields bf;
        unsigned int raw;
    } u;
    
    VOLATILE unsigned int temp = 0x12345678;
    
    /* Bitfield assignments that may use ZERO_EXTRACT */
    u.raw = temp;
    u.bf.flag = (temp >> 5) & 0x7;      /* Extract bits 5-7 */
    u.bf.value = (temp >> 8) & 0x1F;    /* Extract bits 8-12 */
    u.bf.mode = (temp >> 13) & 0xF;     /* Extract bits 13-16 */
    
    /* Explicit bit extraction that may generate ZERO_EXTRACT */
    unsigned int extracted = 0;
    extracted |= ((temp >> 3) & 0x1F) << 10;   /* Extract and shift */
    extracted |= ((temp >> 10) & 0x7) << 5;    /* Another extraction */
    
    /* Complex bitfield manipulation */
    struct {
        unsigned int a:7;
        unsigned int b:9;
        unsigned int c:16;
    } bf2;
    
    bf2.a = (temp >> 1) & 0x7F;
    bf2.b = (temp >> 8) & 0x1FF;
    bf2.c = (temp >> 17) & 0xFFFF;
    
    return u.raw + extracted + bf2.c;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE unsigned test_strict_low_part(void) {
    VOLATILE unsigned int wide_reg = 0xDEADBEEF;
    VOLATILE unsigned char byte_reg = 0;
    
    /* Byte store into integer - may generate STRICT_LOW_PART */
    *(VOLATILE unsigned char*)&wide_reg = 0xFF;
    
    /* Union for byte access */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } u;
    
    u.full = 0x12345678;
    u.bytes[1] = 0xAA;  /* Modify only low part of the register */
    
    /* Truncation that preserves high bits */
    unsigned int temp = wide_reg;
    byte_reg = temp & 0xFF;  /* Access only low byte */
    
    /* Multiple byte operations */
    for (int i = 0; i < 4; i++) {
        u.bytes[i] = (temp >> (i * 8)) & 0xFF;
    }
    
    /* Inline assembly forcing low-part register access */
    unsigned int result;
    asm volatile (
        "movl %1, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=r" (byte_reg)
        : "r" (wide_reg)
        : "%eax"
    );
    
    /* Another inline asm with byte modifier */
    unsigned int in = 0x87654321;
    unsigned char out;
    asm volatile (
        "movb %b1, %0\n\t"
        : "=r" (out)
        : "r" (in)
    );
    
    return wide_reg + byte_reg + u.full + out;
}

/* ========== SUBREG patterns ========== */
NOINLINE unsigned test_subreg(void) {
    /* Vector extensions for SUBREG generation */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Vector element extraction - often uses SUBREG */
    int elem0 = vec_int[0];
    int elem2 = vec_int[2];
    short selem = vec_short[3];
    
    /* Type punning through casts */
    float f = 3.14159f;
    int i;
    /* Bitcast through union for strict aliasing */
    union {
        float f;
        int i;
    } pun;
    
    pun.f = f;
    i = pun.i;  /* This may generate SUBREG */
    
    /* Mixed size operations */
    short s = 1000;
    int extended = s;  /* Sign extension may use SUBREG */
    short truncated = extended;  /* Truncation may use SUBREG */
    
    /* Complex type mixing */
    long long ll = 0x1122334455667788LL;
    int lower = (int)ll;          /* Extract low 32 bits */
    int upper = (int)(ll >> 32);  /* Extract high 32 bits */
    
    /* Pointer casting for subreg access */
    char *ptr = (char*)&ll;
    int from_bytes = *(int*)(ptr + 2);  /* Misaligned access */
    
    return elem0 + elem2 + i + truncated + lower + from_bytes;
}

/* ========== Memory operand patterns ========== */
NOINLINE unsigned test_memory_operand(void) {
    /* Complex memory addressing */
    VOLATILE int array[100];
    VOLATILE int *ptr1 = array;
    VOLATILE int **ptr2 = &ptr1;
    VOLATILE int ***ptr3 = &ptr2;
    
    /* Initialize */
    for (int i = 0; i < 100; i++) {
        array[i] = i * 2;
    }
    
    /* Multi-level pointer dereference */
    int val1 = ***ptr3;
    int val2 = **(ptr2 + 1);  /* May cause complex addressing */
    
    /* Complex array indexing */
    VOLATILE int idx = get_index();
    int val3 = array[idx];
    int val4 = array[idx * 2 + 5];
    
    /* Structure with nested arrays */
    struct nested {
        int data[10][10];
        int *ptr;
    } s;
    
    /* Initialize structure */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            s.data[i][j] = i * 100 + j;
        }
    }
    
    /* Complex structure access */
    int val5 = s.data[3][4];
    int val6 = s.data[idx % 10][idx / 10];
    
    /* Pointer arithmetic with volatile */
    VOLATILE char *char_ptr = (char*)array;
    char_ptr += idx * sizeof(int);
    int val7 = *(int*)char_ptr;
    
    /* Memory barrier to prevent reordering */
    asm volatile ("" ::: "memory");
    
    return val1 + val2 + val3 + val4 + val5 + val6 + val7;
}

/* ========== Main test driver ========== */
int main(void) {
    unsigned total = 0;
    
    printf("Testing GCC resource tracking patterns...\n");
    
    /* Run all pattern tests */
    total += test_zero_extract();
    total += test_strict_low_part();
    total += test_subreg();
    total += test_memory_operand();
    
    printf("Result checksum: %u\n", total);
    
    /* Use result to prevent dead code elimination */
    VOLATILE unsigned sink = total;
    return sink > 0 ? 0 : 1;
}

/* External functions to prevent optimization */
int get_index(void) {
    return g_index;
}

void* get_ptr(void) {
    return g_ptr;
}
