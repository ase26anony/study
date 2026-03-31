/* test_resource_patterns.c */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimization */
#define NOINLINE __attribute__((noinline, optimize("O0")))
#define VOLATILE_VAR volatile

/* External function to prevent constant propagation */
extern int opaque_int(void);
extern void* opaque_ptr(void);

/* ========== ZERO_EXTRACT patterns ========== */
struct bitfield_packed {
    unsigned int flag:3;
    unsigned int value:5;
    unsigned int mode:4;
    unsigned int status:8;
    unsigned int reserved:12;
} __attribute__((packed));

struct nested_bitfield {
    struct {
        unsigned int low:8;
        unsigned int high:8;
    } bytes;
    unsigned int combined:16;
};

NOINLINE static unsigned int test_zero_extract(void) {
    VOLATILE_VAR struct bitfield_packed bf;
    VOLATILE_VAR unsigned int result = 0;
    
    /* Initialize with opaque values */
    unsigned int seed = opaque_int();
    bf.flag = (seed >> 0) & 0x7;
    bf.value = (seed >> 3) & 0x1F;
    bf.mode = (seed >> 8) & 0xF;
    bf.status = (seed >> 12) & 0xFF;
    
    /* Bitfield extraction that may generate ZERO_EXTRACT */
    result |= (unsigned int)bf.flag;
    result |= (unsigned int)bf.value << 3;
    result |= (unsigned int)bf.mode << 8;
    result |= (unsigned int)bf.status << 12;
    
    /* Explicit bit masking/extraction */
    VOLATILE_VAR unsigned int raw = opaque_int();
    unsigned int extracted = (raw >> 5) & 0x3FF;  /* 10-bit extraction */
    result ^= extracted;
    
    /* Nested bitfield access */
    VOLATILE_VAR struct nested_bitfield nested;
    nested.bytes.low = raw & 0xFF;
    nested.bytes.high = (raw >> 8) & 0xFF;
    nested.combined = nested.bytes.low | (nested.bytes.high << 8);
    
    result += nested.combined;
    
    /* Multiple extractions with different widths */
    for (VOLATILE_VAR int i = 0; i < 4; i++) {
        unsigned int temp = opaque_int();
        unsigned int field = (temp >> (i * 3)) & 0x7;  /* 3-bit fields */
        result += field * (i + 1);
    }
    
    return result;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE static unsigned int test_strict_low_part(void) {
    VOLATILE_VAR unsigned int wide_reg = opaque_int();
    VOLATILE_VAR unsigned int result = 0;
    
    /* Byte-sized store into integer - may generate STRICT_LOW_PART */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } u;
    
    u.full = wide_reg;
    
    /* Modify individual bytes */
    for (VOLATILE_VAR int i = 0; i < 4; i++) {
        u.bytes[i] = (u.bytes[i] + i) & 0xFF;
    }
    result = u.full;
    
    /* Pointer-based byte access */
    VOLATILE_VAR unsigned int target = opaque_int();
    unsigned char *byte_ptr = (unsigned char*)&target;
    
    /* Force byte stores that might preserve high bits */
    byte_ptr[0] = (target + 1) & 0xFF;
    byte_ptr[1] = (target + 2) & 0xFF;
    byte_ptr[2] = (target + 3) & 0xFF;
    byte_ptr[3] = (target + 4) & 0xFF;
    
    result ^= target;
    
    /* Inline assembly for explicit low-part register access */
    unsigned int asm_in = opaque_int();
    unsigned int asm_out;
    
    /* x86-specific: %b0 modifier accesses low byte */
    asm volatile (
        "movb %b1, %b0\n\t"
        : "=r"(asm_out)
        : "r"(asm_in)
        : "cc"
    );
    
    result += asm_out;
    
    /* Truncation operations in context */
    VOLATILE_VAR unsigned int source = opaque_int();
    unsigned char truncated = source & 0xFF;  /* Explicit truncation */
    
    /* Use truncated value in way that might need high bits preserved */
    unsigned int extended = truncated;
    extended = (extended << 8) | truncated;
    
    result += extended;
    
    return result;
}

/* ========== SUBREG patterns ========== */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE static unsigned int test_subreg(void) {
    VOLATILE_VAR unsigned int result = 0;
    
    /* Vector operations with element extraction */
    v4si vec = { opaque_int(), opaque_int(), opaque_int(), opaque_int() };
    
    /* Extract individual elements - may use SUBREG */
    result += vec[0];
    result += vec[1];
    result += vec[2];
    result += vec[3];
    
    /* Type punning between different sizes */
    VOLATILE_VAR float f = (float)opaque_int() / 1000.0f;
    VOLATILE_VAR int i;
    
    /* Bitcast through union - forces subregister access */
    union {
        float f;
        int i;
        short s[2];
    } pun;
    
    pun.f = f;
    result += pun.i;
    
    /* Access halves of integer */
    pun.i = opaque_int();
    result += pun.s[0];
    result += pun.s[1];
    
    /* Mixed-size vector operations */
    v8hi short_vec = { 1, 2, 3, 4, 5, 6, 7, 8 };
    for (VOLATILE_VAR int idx = 0; idx < 8; idx++) {
        short_vec[idx] = (opaque_int() >> idx) & 0xFFFF;
    }
    
    /* Convert between vector types */
    v4si converted;
    memcpy(&converted, &short_vec, sizeof(short_vec));
    result += converted[0];
    
    /* Explicit casts between different integer sizes */
    VOLATILE_VAR long long ll = (long long)opaque_int() * opaque_int();
    int truncated_int = (int)ll;  /* SUBREG from 64-bit to 32-bit */
    short truncated_short = (short)truncated_int;  /* Another SUBREG */
    
    result += truncated_short;
    
    return result;
}

/* ========== Complex Memory Operand patterns ========== */
struct node {
    int value;
    struct node *next;
    int data[3];
};

NOINLINE static unsigned int test_memory_operand(void) {
    VOLATILE_VAR unsigned int result = 0;
    
    /* Create complex memory access patterns */
    struct node nodes[4];
    VOLATILE_VAR struct node *volatile current;
    
    /* Initialize linked structure */
    for (int i = 0; i < 4; i++) {
        nodes[i].value = opaque_int();
        nodes[i].next = (i < 3) ? &nodes[i + 1] : NULL;
        for (int j = 0; j < 3; j++) {
            nodes[i].data[j] = opaque_int();
        }
    }
    
    /* Complex pointer chasing */
    current = &nodes[0];
    while (current) {
        result += current->value;
        
        /* Nested array access with volatile index */
        VOLATILE_VAR int idx = opaque_int() & 0x3;
        if (idx < 3) {
            result += current->data[idx];
        }
        
        /* Multiple indirections */
        if (current->next && current->next->next) {
            result += current->next->next->value;
        }
        
        current = current->next;
    }
    
    /* Multi-level pointer dereferencing */
    int ***triple_ptr = (int***)opaque_ptr();
    if (triple_ptr && *triple_ptr && **triple_ptr) {
        result += ***triple_ptr;
    }
    
    /* Volatile array with complex indexing */
    VOLATILE_VAR int array[100];
    for (VOLATILE_VAR int i = 0; i < 100; i++) {
        array[i] = opaque_int();
    }
    
    VOLATILE_VAR int index1 = opaque_int() % 100;
    VOLATILE_VAR int index2 = opaque_int() % 100;
    VOLATILE_VAR int index3 = opaque_int() % 100;
    
    /* Complex address calculation */
    result += array[index1] + array[index2] * array[index3];
    
    /* Structure field with bitfield */
    struct {
        int header;
        struct bitfield_packed bf;
        int footer;
    } wrapped;
    
    wrapped.header = opaque_int();
    wrapped.bf.value = (opaque_int() >> 2) & 0x1F;
    wrapped.footer = opaque_int();
    
    result += wrapped.header + wrapped.footer;
    
    /* Memory barrier to prevent reordering */
    asm volatile ("" ::: "memory");
    
    return result;
}

/* Opaque functions to prevent optimization */
int opaque_int(void) {
    static VOLATILE_VAR int counter = 0;
    return ++counter;
}

void* opaque_ptr(void) {
    static VOLATILE_VAR char buffer[256];
    static VOLATILE_VAR int offset = 0;
    offset = (offset + 37) % 256;
    return buffer + offset;
}

/* Main function to drive all tests */
int main(void) {
    unsigned int total = 0;
    
    printf("Testing resource pattern coverage...\n");
    
    /* Run each test multiple times with different data */
    for (VOLATILE_VAR int iteration = 0; iteration < 3; iteration++) {
        total += test_zero_extract();
        total += test_strict_low_part();
        total += test_subreg();
        total += test_memory_operand();
    }
    
    printf("Result checksum: %u\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total == 0) {
        printf("Unexpected zero result\n");
    }
    
    return (int)(total & 0x7FFFFFFF);
}
