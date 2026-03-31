#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimization */
#define NOINLINE __attribute__((noinline, optimize("O0")))
#define VOLATILE_VAR volatile

/* External function to prevent constant propagation */
extern int opaque(int x);
extern void* opaque_ptr(void* p);

/* Global volatile variables to force memory operations */
VOLATILE_VAR int global_index = 0;
VOLATILE_VAR char global_byte = 0;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Bitfield structure for ZERO_EXTRACT */
struct bitfield_pack {
    unsigned int flag:3;
    unsigned int value:5;
    unsigned int mode:4;
    unsigned int reserved:20;
} NOINLINE;

/* Another bitfield with different layout */
struct another_bitfield {
    unsigned int low:8;
    unsigned int mid:8;
    unsigned int high:8;
    unsigned int top:8;
} NOINLINE;

NOINLINE int test_zero_extract(void) {
    VOLATILE_VAR struct bitfield_pack bf1 = {0};
    VOLATILE_VAR struct another_bitfield bf2 = {0};
    VOLATILE_VAR unsigned int raw = 0xDEADBEEF;
    int result = 0;
    
    /* Direct bitfield assignments - may generate ZERO_EXTRACT */
    bf1.flag = 5;
    bf1.value = 20;
    bf1.mode = 9;
    
    bf2.low = 0xAA;
    bf2.mid = 0xBB;
    bf2.high = 0xCC;
    bf2.top = 0xDD;
    
    /* Bit extraction using shift and mask - may generate ZERO_EXTRACT */
    unsigned int extracted1 = (raw >> 3) & 0x1F;  /* Extract 5 bits at position 3 */
    unsigned int extracted2 = (raw >> 8) & 0xFF;  /* Extract 8 bits at position 8 */
    unsigned int extracted3 = (raw >> 16) & 0x7;  /* Extract 3 bits at position 16 */
    
    /* Combine bitfield accesses with arithmetic */
    result = bf1.flag + bf1.value * 2 + bf1.mode;
    result += bf2.low - bf2.mid + bf2.high ^ bf2.top;
    result += extracted1 + extracted2 * extracted3;
    
    /* Complex bitfield expression */
    bf1.value = (bf2.low & 0x0F) | ((bf2.mid >> 2) & 0x10);
    
    return result + opaque(result);
}

/* ==================== STRICT_LOW_PART patterns ==================== */

NOINLINE int test_strict_low_part(void) {
    VOLATILE_VAR int int_var = 0x12345678;
    VOLATILE_VAR long long_var = 0x9876543210ABCDEFLL;
    VOLATILE_VAR short short_var = 0;
    VOLATILE_VAR char char_var = 0;
    int result = 0;
    
    /* Union for type punning - may generate low-part accesses */
    union {
        int full;
        char bytes[4];
    } pun NOINLINE = {0};
    
    pun.full = int_var;
    
    /* Byte-sized stores into integers - may generate STRICT_LOW_PART */
    /* Force byte store to low part of register */
    *(VOLATILE_VAR unsigned char*)&int_var = 0xFF;
    global_byte = 0xAA;
    *(VOLATILE_VAR unsigned char*)&int_var = global_byte;
    
    /* Multiple byte operations */
    pun.bytes[0] = 0x11;
    pun.bytes[1] = 0x22;
    pun.bytes[2] = 0x33;
    pun.bytes[3] = 0x44;
    
    /* Truncation operations that preserve high bits */
    char_var = int_var & 0xFF;           /* Explicit truncation */
    short_var = long_var & 0xFFFF;       /* Truncate 64-bit to 16-bit */
    
    /* Inline assembly forcing low-byte register access */
    /* %b0 modifier accesses low byte of register on x86 */
    asm volatile (
        "movb %b1, %0\n\t"
        : "=r" (char_var)
        : "r" (int_var)
        : "cc"
    );
    
    /* Another inline asm with different constraint */
    int temp NOINLINE;
    asm volatile (
        "movw %w1, %w0\n\t"   /* %w accesses low word (16-bit) */
        : "=r" (temp)
        : "r" (int_var)
        : "cc"
    );
    
    result = pun.full + char_var + short_var + temp;
    return result + opaque(result);
}

/* ==================== SUBREG patterns ==================== */

/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE int test_subreg(void) {
    VOLATILE_VAR v4si vec_int = {1, 2, 3, 4};
    VOLATILE_VAR v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    VOLATILE_VAR v4sf vec_float = {1.0f, 2.0f, 3.0f, 4.0f};
    VOLATILE_VAR int scalar_int = 0;
    VOLATILE_VAR short scalar_short = 0;
    VOLATILE_VAR float scalar_float = 0.0f;
    int result = 0;
    
    /* Extract elements from vectors - may generate SUBREG */
    scalar_int = vec_int[0] + vec_int[2];    /* Access specific elements */
    scalar_short = vec_short[3] - vec_short[5];
    scalar_float = vec_float[1] * vec_float[3];
    
    /* Type punning between int and float - may generate SUBREG */
    union {
        float f;
        int i;
    } float_int NOINLINE;
    
    float_int.f = scalar_float;
    result = float_int.i + opaque(float_int.i);
    
    /* Cast between different integer sizes */
    long long big_int = 0x123456789ABCDEF0LL;
    int small_int = (int)big_int;           /* Truncation cast */
    short smaller = (short)small_int;       /* Another truncation */
    
    /* Mix types in expressions */
    result += scalar_int + small_int * smaller;
    result += (int)scalar_float;
    
    /* Vector element manipulation */
    vec_int[1] = result;
    vec_short[2] = result & 0xFFFF;
    
    /* Complex vector operation */
    v4si temp_vec = vec_int + (v4si){result, 0, result, 0};
    result += temp_vec[0] + temp_vec[2];
    
    return result + opaque(result);
}

/* ==================== Memory operand patterns ==================== */

/* Complex nested structure for memory access patterns */
struct level3 {
    int data[4];
    char padding[12];
};

struct level2 {
    struct level3 *l3;
    int values[8];
    struct level3 direct_l3;
};

struct level1 {
    struct level2 *l2_array[4];
    struct level2 direct_l2;
    volatile int index;
};

NOINLINE int test_memory_operand(void) {
    /* Allocate complex memory structure */
    struct level1 *l1 = (struct level1*)malloc(sizeof(struct level1));
    if (!l1) return 0;
    
    /* Initialize pointers */
    for (int i = 0; i < 4; i++) {
        l1->l2_array[i] = (struct level2*)malloc(sizeof(struct level2));
        if (l1->l2_array[i]) {
            l1->l2_array[i]->l3 = (struct level3*)malloc(sizeof(struct level3));
        }
    }
    
    VOLATILE_VAR int idx1 = global_index & 3;  /* Force non-constant index */
    VOLATILE_VAR int idx2 = opaque(idx1) & 7;
    VOLATILE_VAR int idx3 = opaque(idx2) & 3;
    
    int result = 0;
    
    /* Complex multi-level pointer dereferencing */
    if (l1->l2_array[idx1] && l1->l2_array[idx1]->l3) {
        /* Triple pointer dereference */
        result = l1->l2_array[idx1]->l3->data[idx3];
        
        /* Update through complex path */
        l1->l2_array[idx1]->l3->data[idx3] = result + 1;
        
        /* Another level */
        l1->l2_array[idx1]->values[idx2] = result * 2;
    }
    
    /* Direct structure access with volatile index */
    l1->direct_l2.direct_l3.data[l1->index & 3] = result;
    l1->direct_l2.values[l1->index & 7] = result + 100;
    
    /* Array indexing with non-constant offset */
    VOLATILE_VAR int* dynamic_array = (int*)malloc(64 * sizeof(int));
    if (dynamic_array) {
        for (VOLATILE_VAR int i = 0; i < 64; i++) {
            dynamic_array[i] = i * i;
        }
        
        /* Complex addressing mode */
        result += dynamic_array[idx1 * 8 + idx2];
        result += dynamic_array[idx3 * 16 + opaque(idx1) & 15];
        
        free(dynamic_array);
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        if (l1->l2_array[i]) {
            if (l1->l2_array[i]->l3) free(l1->l2_array[i]->l3);
            free(l1->l2_array[i]);
        }
    }
    free(l1);
    
    return result + opaque(result);
}

/* ==================== Main function ==================== */

/* Opaque function to prevent optimization */
int opaque(int x) {
    static volatile int counter = 0;
    counter++;
    return x ^ counter;
}

void* opaque_ptr(void* p) {
    static volatile intptr_t mask = 0x00FF00FF;
    return (void*)((intptr_t)p ^ mask);
}

int main(void) {
    int total = 0;
    
    printf("Starting resource pattern tests...\n");
    
    /* Run all pattern tests */
    total += test_zero_extract();
    printf("Zero extract test completed\n");
    
    total += test_strict_low_part();
    printf("Strict low part test completed\n");
    
    total += test_subreg();
    printf("Subreg test completed\n");
    
    total += test_memory_operand();
    printf("Memory operand test completed\n");
    
    printf("Final checksum: %d\n", total);
    
    /* Use the result to prevent dead code elimination */
    if (total > 1000000) {
        printf("Unexpected large result\n");
    }
    
    return total & 0xFF;
}
