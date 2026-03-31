/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * resource.cc file, specifically lines 282-290 in mark_set_resources().
 * The goal is to create RTL patterns where:
 * 1. A SET has a destination that is ZERO_EXTRACT or STRICT_LOW_PART
 * 2. That destination ultimately references memory (MEM_P(x) is true)
 * 3. The pattern survives optimization to be seen by resource tracking passes
 */

#include <stdint.h>
#include <stdlib.h>

/* ========== ZERO_EXTRACT patterns (bitfield assignments) ========== */

/* Global struct with bitfield - ensures memory storage */
struct GlobalBitfield {
    unsigned int data;
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
    unsigned int field4 : 16;
} global_bf;

/* Volatile bitfield struct - forces memory access */
volatile struct VolatileBitfield {
    unsigned int status : 1;
    unsigned int mode : 2;
    unsigned int flags : 4;
} volatile_bf;

/* Function that takes pointer to bitfield struct
 * This ensures the bitfield is in memory, not registers */
void set_bitfield_via_pointer(struct GlobalBitfield *ptr, int val) {
    /* Multiple bitfield assignments increase chance of pattern preservation */
    ptr->field1 = val & 0x7;
    ptr->field2 = (val >> 3) & 0x1F;
    ptr->field3 = (val >> 8) & 0xFF;
    
    /* Compiler barrier to prevent merging */
    asm volatile("" : : : "memory");
    
    ptr->field4 = (val >> 16) & 0xFFFF;
}

/* Complex bitfield manipulation with control flow */
void conditional_bitfield_set(int condition, struct GlobalBitfield *ptr) {
    if (condition) {
        ptr->field1 = 1;
        ptr->field2 = 2;
    } else {
        ptr->field1 = 3;
        ptr->field2 = 4;
    }
    
    /* Loop with bitfield assignment */
    for (int i = 0; i < 3; i++) {
        ptr->field3 = i & 0x7;
        /* External function call prevents optimization */
        rand();
    }
}

/* ========== STRICT_LOW_PART patterns (partial register assignments) ========== */

/* Function using inline assembly with byte constraints */
void partial_register_ops(void) {
    /* Using 'Q' constraint for byte-addressable register */
    unsigned char byte1, byte2;
    
    /* Multiple asm statements that might generate STRICT_LOW_PART */
    asm volatile(
        "movb $0xAA, %0\n\t"
        : "=Q" (byte1)
        :
        : "memory"
    );
    
    asm volatile(
        "movb $0x55, %0\n\t"
        : "=Q" (byte2)
        :
        : "memory"
    );
    
    /* Force memory storage of results */
    volatile char storage[2];
    storage[0] = byte1;
    storage[1] = byte2;
}

/* Mixed byte and word operations that might create partial register patterns */
void mixed_size_operations(void) {
    struct MixedSizes {
        char c;
        short s;
        int i;
    } ms;
    
    /* These assignments might generate different partial register patterns */
    ms.c = 0x12;
    ms.s = 0x3456;
    ms.i = 0x789ABCDE;
    
    /* Take address to force memory storage */
    volatile struct MixedSizes *vptr = &ms;
    (void)vptr;
}

/* ========== Combined patterns with memory references ========== */

/* Function that combines bitfields and inline assembly */
void combined_patterns(struct GlobalBitfield *ptr) {
    /* Start with bitfield assignment (potential ZERO_EXTRACT) */
    ptr->field1 = 5;
    
    /* Inline assembly that reads/writes memory */
    unsigned int temp;
    asm volatile(
        "movl %1, %%eax\n\t"
        "andl $0x7, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (ptr->field2)  /* Memory output constraint */
        : "r" (ptr->data)     /* Register input */
        : "eax", "memory"
    );
    
    /* Another bitfield assignment */
    ptr->field3 = temp & 0xFF;
}

/* Atomic operations on bitfields - may generate complex RTL */
void atomic_bitfield_ops(void) {
    /* Use __sync builtins on bitfields */
    unsigned int sync_var = 0;
    
    /* Create a bitfield-like operation atomically */
    __sync_fetch_and_or(&sync_var, 0x07);  /* Sets lower 3 bits */
    __sync_fetch_and_and(&sync_var, ~0x38); /* Clears bits 3-5 */
    __sync_fetch_and_xor(&sync_var, 0x40); /* Toggles bit 6 */
    
    /* Store result in global bitfield */
    global_bf.field1 = sync_var & 0x7;
}

/* ========== Main function with complex control flow ========== */

int main(int argc, char **argv) {
    /* Use argc to create unpredictable control flow */
    int mode = argc > 1 ? atoi(argv[1]) : 0;
    
    /* Initialize global structures */
    global_bf.data = 0x12345678;
    volatile_bf.status = 0;
    volatile_bf.mode = 1;
    
    /* Local struct with bitfield - take address to force memory */
    struct GlobalBitfield local_bf;
    struct GlobalBitfield *ptr = &local_bf;
    
    /* Call functions with different patterns based on mode */
    switch (mode & 0x3) {
        case 0:
            /* Pure bitfield operations */
            set_bitfield_via_pointer(&global_bf, 0xABCDEF);
            set_bitfield_via_pointer(ptr, 0x123456);
            break;
            
        case 1:
            /* Partial register operations */
            partial_register_ops();
            mixed_size_operations();
            break;
            
        case 2:
            /* Combined patterns */
            combined_patterns(&global_bf);
            atomic_bitfield_ops();
            break;
            
        case 3:
            /* All patterns mixed */
            conditional_bitfield_set(mode, &global_bf);
            partial_register_ops();
            combined_patterns(ptr);
            break;
    }
    
    /* Additional loop with bitfield assignments */
    for (int i = 0; i < 10; i++) {
        /* Volatile access prevents optimization */
        volatile_bf.flags = i & 0xF;
        
        /* Conditional bitfield assignment */
        if (i % 2) {
            ptr->field1 = i & 0x7;
        } else {
            ptr->field2 = i & 0x1F;
        }
        
        /* Compiler barrier every few iterations */
        if (i % 3 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    /* Final volatile store to ensure all operations complete */
    volatile int finish = 1;
    (void)finish;
    
    return 0;
}
