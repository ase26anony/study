/* test_resource_marking.c - Test for uncovered lines in mark_set_resources */

#include <stdint.h>
#include <stdlib.h>

/* Global struct with bitfield to ensure memory storage */
struct BitfieldStruct {
    unsigned int data;
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
    unsigned int field4 : 16;
};

volatile struct BitfieldStruct global_bitfield = {0};

/* Function to force bitfield assignment through pointer */
void set_bitfield_via_pointer(struct BitfieldStruct *ptr, int idx, unsigned int value) {
    /* Use switch to create control flow that's hard to optimize away */
    switch (idx & 3) {
        case 0:
            ptr->field1 = value & 0x7;
            /* Memory barrier to prevent reordering */
            asm volatile("" : : : "memory");
            break;
        case 1:
            ptr->field2 = value & 0x1F;
            asm volatile("" : : : "memory");
            break;
        case 2:
            ptr->field3 = value & 0xFF;
            asm volatile("" : : : "memory");
            break;
        case 3:
            ptr->field4 = value & 0xFFFF;
            asm volatile("" : : : "memory");
            break;
    }
}

/* Function with inline assembly that might generate STRICT_LOW_PART */
void partial_register_ops(void) {
    volatile uint8_t byte_var = 0;
    volatile uint16_t word_var = 0;
    volatile uint32_t dword_var = 0;
    
    /* Multiple inline asm statements with overlapping resources */
    
    /* This might generate STRICT_LOW_PART for byte access */
    __asm__ volatile(
        "movb $0x42, %0\n\t"
        : "=Q" (byte_var)
        :
        : "memory"
    );
    
    /* Another asm that reads/writes same location */
    __asm__ volatile(
        "incb %0\n\t"
        : "+Q" (byte_var)
        :
        : "cc", "memory"
    );
    
    /* Word access that might interact with scheduling */
    __asm__ volatile(
        "movw $0x1234, %0\n\t"
        : "=r" (word_var)
        :
        : "memory"
    );
    
    /* Complex asm with multiple constraints */
    uint32_t input = 0x89ABCDEF;
    __asm__ volatile(
        "movl %1, %%eax\n\t"
        "andl $0xFF, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=Q" (byte_var)
        : "r" (input)
        : "eax", "memory"
    );
}

/* Function using atomic operations on bitfields */
void atomic_bitfield_ops(struct BitfieldStruct *ptr) {
    /* Atomic operations on bitfields may generate ZERO_EXTRACT patterns */
    __sync_fetch_and_or(&ptr->field1, 1);
    __sync_fetch_and_and(&ptr->field2, 0x1E);
    __sync_fetch_and_xor(&ptr->field3, 0x55);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
}

/* Function with loop containing bitfield assignments */
void loop_with_bitfields(struct BitfieldStruct *ptr, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Mix different bitfield assignments */
        ptr->field1 = (i * 3) & 0x7;
        ptr->field2 = (i * 5) & 0x1F;
        
        /* Volatile function call to prevent loop unrolling */
        asm volatile("" : : : "memory");
        
        /* Conditional assignment based on i */
        if (i & 1) {
            ptr->field3 = (i * 7) & 0xFF;
        } else {
            ptr->field4 = (i * 11) & 0xFFFF;
        }
    }
}

/* Main function with unpredictable control flow */
int main(int argc, char *argv[]) {
    struct BitfieldStruct local_bitfield = {0};
    struct BitfieldStruct *heap_bitfield = malloc(sizeof(struct BitfieldStruct));
    
    if (!heap_bitfield) return 1;
    
    /* Use argc to create unpredictable control flow */
    int mode = argc > 1 ? atoi(argv[1]) : 0;
    
    /* Mix different operations to create complex RTL patterns */
    
    /* 1. Direct bitfield assignment to global (volatile ensures memory access) */
    global_bitfield.field1 = 1;
    global_bitfield.field2 = 2;
    
    /* 2. Bitfield assignment via pointer */
    set_bitfield_via_pointer(&local_bitfield, mode, 0x55);
    set_bitfield_via_pointer(heap_bitfield, mode + 1, 0xAA);
    
    /* 3. Partial register operations */
    partial_register_ops();
    
    /* 4. Atomic operations on bitfields */
    atomic_bitfield_ops(&local_bitfield);
    atomic_bitfield_ops(heap_bitfield);
    
    /* 5. Loop with bitfields - use argc to determine iterations */
    loop_with_bitfields(&local_bitfield, argc > 2 ? atoi(argv[2]) : 10);
    
    /* 6. Complex inline asm with memory constraints */
    uint32_t temp = 0xDEADBEEF;
    __asm__ volatile(
        "movl %1, %%eax\n\t"
        "shrl $3, %%eax\n\t"
        "andl $0x7, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (heap_bitfield->field1)
        : "r" (temp)
        : "eax", "memory"
    );
    
    /* 7. Mixed-size accesses to same memory area */
    volatile uint32_t *as_int = (volatile uint32_t*)&local_bitfield;
    volatile uint8_t *as_byte = (volatile uint8_t*)&local_bitfield;
    
    __asm__ volatile(
        "movl (%1), %%eax\n\t"
        "orb $0x1, %%al\n\t"
        "movb %%al, (%0)\n\t"
        : 
        : "r" (as_byte), "r" (as_int)
        : "eax", "memory"
    );
    
    /* Use the results to prevent dead code elimination */
    if (local_bitfield.field1 != 0 || heap_bitfield->field2 != 0) {
        asm volatile("" : : "r" (local_bitfield.data), "r" (heap_bitfield->data) : "memory");
    }
    
    free(heap_bitfield);
    
    return (global_bitfield.field1 + local_bitfield.field2) & 0xFF;
}
