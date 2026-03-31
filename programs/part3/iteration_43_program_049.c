/* resource_coverage.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource tracking logic (mark_referenced_resources in resource.cc).
 * The goal is to cover lines 282-290 that handle ZERO_EXTRACT,
 * STRICT_LOW_PART, SUBREG, and MEM expressions.
 *
 * Compile with: gcc -O2 -m32 -fno-strict-aliasing -c resource_coverage.c
 * Additional flags for debugging: -fdump-rtl-all -dP -da
 */

#include <stdint.h>
#include <string.h>

/* Force functions to not be inlined to ensure their RTL is examined */
#define NOINLINE __attribute__((noinline))

/* Function A: Focus on ZERO_EXTRACT and MEM patterns */
NOINLINE static void func_a(volatile int *counter) {
    /* Struct with volatile bit-field to generate ZERO_EXTRACT */
    struct bitfield_struct {
        volatile unsigned int field1 : 5;
        volatile unsigned int field2 : 3;
        volatile unsigned int field3 : 8;
    } bs;
    
    /* Array with complex addressing for MEM patterns */
    volatile int arr[10][10];
    
    /* Initialize to prevent constant propagation */
    int i = *counter % 10;
    int j = (*counter + 1) % 10;
    
    /* ZERO_EXTRACT: Assignment to volatile bit-field */
    bs.field1 = i & 0x1F;
    bs.field2 = j & 0x07;
    
    /* MEM: Complex array access with pointer arithmetic */
    volatile int *ptr = &arr[i][j];
    *ptr = bs.field1 + bs.field2;
    
    /* More complex MEM addressing */
    volatile int val = arr[(i + j) % 10][(i * j) % 10];
    bs.field3 = val & 0xFF;
    
    /* Update counter to prevent dead code elimination */
    *counter += bs.field1 + bs.field2 + bs.field3;
}

/* Function B: Focus on STRICT_LOW_PART and SUBREG patterns */
NOINLINE static void func_b(volatile int *counter) {
    volatile short s_val = *counter & 0xFFFF;
    volatile char c_val = *counter & 0xFF;
    
    /* STRICT_LOW_PART: Inline assembly modifying only part of register */
    /* Using 'q' constraint for byte-addressable register */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(c_val) 
        : "0"(c_val)
        : "cc"
    );
    
    /* Another STRICT_LOW_PART pattern with different operation */
    asm volatile (
        "orb $0x10, %0\n\t"
        : "=q"(c_val)
        : "0"(c_val)
        : "cc"
    );
    
    /* SUBREG: Type punning through pointer casts */
    int int_val = *counter;
    
    /* Access lower 16 bits through short pointer (SUBREG in RTL) */
    short *short_ptr = (short *)&int_val;
    *short_ptr = s_val;
    
    /* Access individual bytes (more SUBREG patterns) */
    char *char_ptr = (char *)&int_val;
    char_ptr[1] = c_val;
    char_ptr[3] = (*counter >> 8) & 0xFF;
    
    /* Mixed-size operations to encourage SUBREG usage */
    long long ll_val = (long long)int_val * (long long)s_val;
    int_val = (int)(ll_val & 0xFFFFFFFF);
    
    /* Update counter */
    *counter = int_val + c_val;
}

/* Function C: Complex expression mixing multiple patterns */
NOINLINE static void func_c(volatile int *counter, volatile int *sum) {
    /* Struct with bit-fields for ZERO_EXTRACT */
    struct mixed_struct {
        volatile unsigned int flags : 4;
        volatile unsigned int value : 12;
        volatile unsigned int pad : 16;
    } ms;
    
    /* Array for MEM patterns */
    static volatile int data[256];
    
    /* Initialize based on counter */
    int idx = *counter & 0xFF;
    
    /* Complex addressing mode for MEM */
    volatile int *elem = &data[idx * 2 % 256];
    
    /* Ternary operator selecting different addresses */
    volatile int *ptr = (idx & 1) ? &data[0] : elem;
    
    /* ZERO_EXTRACT: Bit-field assignment */
    ms.flags = idx & 0x0F;
    ms.value = (*ptr) & 0x0FFF;
    
    /* SUBREG: Access through different-sized pointer */
    short *short_view = (short *)ptr;
    short_view[1] = ms.value;
    
    /* MEM: Store with complex address calculation */
    data[(ms.value + idx) % 256] = ms.flags;
    
    /* Update sum to prevent elimination */
    *sum += ms.value + *elem;
    *counter = idx + 1;
}

/* Function D: Additional patterns with loops to increase RTL complexity */
NOINLINE static void func_d(volatile int *counter) {
    volatile char buffer[64];
    volatile int *int_ptr = (volatile int *)buffer;
    
    /* Initialize buffer */
    for (int i = 0; i < 16; i++) {
        int_ptr[i] = *counter + i;
    }
    
    /* Mixed-size accesses causing SUBREG */
    volatile short *short_ptr = (volatile short *)buffer;
    for (int i = 0; i < 32; i++) {
        short_ptr[i] = (short)(short_ptr[i] + 1);
    }
    
    /* Bit-field in local struct for ZERO_EXTRACT */
    struct {
        volatile unsigned int a : 2;
        volatile unsigned int b : 6;
        volatile unsigned int c : 24;
    } local_bf;
    
    local_bf.a = (*counter >> 0) & 0x3;
    local_bf.b = (*counter >> 2) & 0x3F;
    local_bf.c = (*counter >> 8) & 0xFFFFFF;
    
    /* STRICT_LOW_PART with different register constraint */
    unsigned char byte_val = local_bf.b;
    asm volatile (
        "subb $1, %0\n\t"
        : "=r"(byte_val)
        : "0"(byte_val)
        : "cc"
    );
    
    /* Update counter */
    *counter = local_bf.c + byte_val + buffer[0];
}

/* Main function that drives all patterns */
int main(int argc, char *argv[]) {
    volatile int counter = 0;
    volatile int sum = 0;
    
    /* Use argc to bound loops for compilation analysis */
    int iterations = (argc > 1) ? 10 : 5;
    
    /* Initialize some global data */
    static volatile int global_array[100];
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * i;
    }
    
    /* Main loop calling pattern functions */
    for (volatile int i = 0; i < iterations; i++) {
        func_a(&counter);
        func_b(&counter);
        func_c(&counter, &sum);
        func_d(&counter);
        
        /* Complex MEM addressing with global array */
        volatile int *ptr = &global_array[counter % 100];
        volatile int val = *(ptr + (i % 10));
        sum += val;
        
        /* Additional SUBREG pattern via union */
        union {
            int i;
            short s[2];
            char c[4];
        } u;
        u.i = counter;
        u.s[0] = (short)(u.s[0] + u.s[1]);
        u.c[2] = u.c[0] ^ u.c[1];
        counter = u.i + 1;
    }
    
    /* Final dummy operation to prevent elimination */
    asm volatile ("" : : "r"(sum), "r"(counter) : "memory");
    
    return sum != 0; /* Non-deterministic return to prevent optimization */
}
