/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimization */
#define NOINLINE __attribute__((noinline, optimize("O0")))
#define VOLATILE volatile

/* External function to prevent constant propagation */
extern int get_index(void);
extern void escape(void*);

/* Global volatile variables to prevent optimizations */
VOLATILE int global_index = 0;
VOLATILE void* global_ptr = NULL;

/* ========== ZERO_EXTRACT patterns ========== */
struct bitfield_s {
    unsigned int flag:3;
    unsigned int value:5;
    unsigned int extra:8;
    unsigned int pad:16;
} NOINLINE;

struct packed_bits {
    unsigned int a:1;
    unsigned int b:2;
    unsigned int c:3;
    unsigned int d:4;
    unsigned int e:5;
} NOINLINE;

NOINLINE static int test_zero_extract(void) {
    struct bitfield_s bf1 = {0};
    struct packed_bits pb = {0};
    VOLATILE unsigned int raw = 0xDEADBEEF;
    int result = 0;
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    bf1.flag = (raw >> 3) & 0x7;      /* Extract 3 bits */
    bf1.value = (raw >> 8) & 0x1F;    /* Extract 5 bits */
    bf1.extra = (raw >> 13) & 0xFF;   /* Extract 8 bits */
    
    /* Complex bitfield manipulation */
    pb.a = (bf1.flag & 0x1);
    pb.b = (bf1.value >> 1) & 0x3;
    pb.c = (bf1.extra >> 2) & 0x7;
    pb.d = (raw >> 16) & 0xF;
    pb.e = (raw >> 20) & 0x1F;
    
    /* Manual bit extraction that may compile to ZERO_EXTRACT */
    unsigned int mask = (1 << pb.c) - 1;
    unsigned int extracted = (raw >> pb.d) & mask;
    
    /* Mix with volatile to prevent optimization */
    result = bf1.flag + bf1.value + bf1.extra + pb.a + pb.b + pb.c + pb.d + pb.e + extracted;
    escape(&bf1);
    escape(&pb);
    
    return result & 0xFF;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE static int test_strict_low_part(void) {
    VOLATILE unsigned int wide_reg = 0x12345678;
    VOLATILE unsigned char byte_store;
    union {
        unsigned int full;
        unsigned char bytes[4];
    } u;
    int result = 0;
    
    /* Byte store into integer - may generate STRICT_LOW_PART */
    u.full = wide_reg;
    u.bytes[0] = 0xFF;                /* Low byte store */
    u.bytes[1] = (wide_reg >> 8) & 0xFF;
    
    /* Pointer cast for byte access */
    *(VOLATILE unsigned char*)&wide_reg = 0xAA;  /* Direct low-byte store */
    
    /* Arithmetic truncation preserving high bits */
    unsigned int temp = wide_reg;
    byte_store = temp & 0xFF;         /* Truncation to low byte */
    temp = (temp & ~0xFF) | byte_store; /* Recombine */
    
    /* Inline assembly forcing low-part register access */
    unsigned int asm_out;
    unsigned int asm_in = 0x87654321;
    asm volatile (
        "movb %b1, %b0\n\t"           /* %b modifier for low byte */
        : "=r"(asm_out)
        : "r"(asm_in)
        : "cc"
    );
    
    /* Multiple byte operations */
    for (int i = 0; i < 4; i++) {
        u.bytes[i] = (wide_reg >> (i * 8)) & 0xFF;
        result += u.bytes[i];
    }
    
    result += asm_out & 0xFF;
    escape(&wide_reg);
    escape(&u);
    
    return result & 0xFF;
}

/* ========== SUBREG patterns ========== */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE static int test_subreg(void) {
    VOLATILE v4si vec = {1, 2, 3, 4};
    VOLATILE v8hi short_vec = {10, 20, 30, 40, 50, 60, 70, 80};
    union {
        float f;
        int i;
        short s[2];
    } pun;
    int result = 0;
    
    /* Vector element extraction - often uses SUBREG */
    int elem0 = vec[0];
    int elem2 = vec[2];
    short selem = short_vec[3];
    
    /* Type punning between different sizes */
    pun.f = 3.14159f;
    pun.i = pun.i & 0xFFFF;           /* Truncate to low 16 bits */
    pun.s[0] = (short)(pun.i >> 8);   /* Extract byte as short */
    
    /* Mixed-size operations */
    unsigned long long big = 0x123456789ABCDEF0ULL;
    unsigned int low = (unsigned int)big;          /* SUBREG truncation */
    unsigned short lower = (unsigned short)low;    /* Another SUBREG */
    
    /* Cast chain forcing subregister accesses */
    double d = 2.71828;
    float f = (float)d;
    int i = *(int*)&f;                /* Bitcast float to int */
    short s = (short)i;
    
    /* Complex vector manipulation */
    v4si temp_vec;
    for (int j = 0; j < 4; j++) {
        temp_vec[j] = vec[j] + j;
        result += temp_vec[j];
    }
    
    result += elem0 + elem2 + selem + pun.s[0] + lower + s;
    escape(&vec);
    escape(&short_vec);
    escape(&pun);
    
    return result & 0xFF;
}

/* ========== Memory operand patterns ========== */
struct nested {
    int data[4];
    struct nested* next;
};

NOINLINE static int test_memory_operand(void) {
    VOLATILE int buffer[64];
    VOLATILE int*** triple_ptr;
    VOLATILE struct nested nodes[4];
    int result = 0;
    
    /* Initialize */
    for (int i = 0; i < 64; i++) {
        buffer[i] = i * 3;
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            nodes[i].data[j] = i * 10 + j;
        }
        if (i < 3) nodes[i].next = &nodes[i + 1];
        else nodes[i].next = &nodes[0];
    }
    
    /* Complex pointer chasing */
    int** ptr2 = (int**)malloc(sizeof(int*) * 4);
    for (int i = 0; i < 4; i++) {
        ptr2[i] = &buffer[i * 8];
    }
    triple_ptr = &ptr2;
    
    /* Multi-level dereferencing */
    VOLATILE int read1 = ***triple_ptr;                    /* Triple pointer deref */
    VOLATILE int read2 = *(*(*triple_ptr + 2) + 3);        /* Complex addressing */
    
    /* Volatile array indexing with non-constant index */
    VOLATILE int idx = global_index;
    VOLATILE int read3 = buffer[idx & 63];                 /* Volatile index */
    VOLATILE int read4 = buffer[(idx * 7) & 63];           /* Complex index */
    
    /* Structure field access with pointer chasing */
    struct nested* current = &nodes[0];
    for (int i = 0; i < 8; i++) {
        VOLATILE int field = current->data[i & 3];         /* Structure access */
        result += field;
        current = current->next;                           /* Pointer following */
    }
    
    /* Complex address calculation */
    VOLATILE int* addr = &buffer[0] + (idx * 3) % 64;
    VOLATILE int read5 = *addr;
    VOLATILE int read6 = addr[5];
    VOLATILE int read7 = addr[-2];
    
    result += read1 + read2 + read3 + read4 + read5 + read6 + read7;
    
    /* Force memory barriers */
    asm volatile("" ::: "memory");
    
    free(ptr2);
    escape(buffer);
    escape(nodes);
    
    return result & 0xFF;
}

/* ========== Main test driver ========== */
int main(void) {
    int total = 0;
    
    printf("Testing GCC resource tracking patterns...\n");
    
    /* Run all pattern tests */
    total += test_zero_extract();
    total += test_strict_low_part();
    total += test_subreg();
    total += test_memory_operand();
    
    /* Use result to prevent dead code elimination */
    printf("Result checksum: %d\n", total & 0xFF);
    
    /* Additional volatile operations to ensure execution */
    VOLATILE int dummy = total;
    asm volatile("" : "+r"(dummy));
    
    return dummy & 1;
}

/* Dummy external functions */
int get_index(void) {
    static int counter = 0;
    return counter++ & 0xF;
}

void escape(void* p) {
    asm volatile("" : : "r"(p) : "memory");
}
