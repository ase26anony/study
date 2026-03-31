#include <stdio.h>
#include <stdint.h>

/* Packed structure with bit-fields to encourage ZERO_EXTRACT */
struct __attribute__((packed)) packed_data {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
};

/* Volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile int result = 0;

int main(void) {
    /* Mixed volatile and non-volatile variables */
    volatile struct packed_data pdata;
    volatile int array[16];
    int temp = 0;
    int i, j;
    
    /* Initialize array with pattern */
    for (i = 0; i < 16; i++) {
        array[i] = i * 3 + 1;
    }
    
    /* Main loop with complex operations */
    for (i = 0; i < 100; i++) {
        /* 1. Arithmetic operation that may create SUBREG in RTL */
        temp = i * 7 + global_counter;
        
        /* 2. Bit-field assignment - potential for ZERO_EXTRACT/STRICT_LOW_PART */
        /* Access bit-fields in packed structure */
        pdata.a = (temp >> 0) & 0xF;    /* First 4 bits */
        pdata.b = (temp >> 4) & 0xFF;   /* Next 8 bits */
        pdata.c = (temp >> 12) & 0xFFF; /* Next 12 bits */
        
        /* 3. Complex memory addressing with pointer arithmetic */
        /* Compute index with bit manipulation */
        int idx = ((i & 0x3) << 2) | (temp & 0x3);
        
        /* Array access with computed index - creates complex MEM address */
        int val = array[idx];
        
        /* 4. Bitwise operation on memory result */
        /* Mask specific bits - may generate ZERO_EXTRACT as destination */
        val &= 0x00FF00FF;  /* Clear alternating bytes */
        
        /* 5. Update array element with bit manipulation */
        /* This creates a store with potential ZERO_EXTRACT destination */
        array[idx] = (array[idx] & 0xFF00FF00) | (val & 0x00FF00FF);
        
        /* 6. More bit-field manipulation */
        /* Combined read-modify-write on bit-fields */
        pdata.d = (pdata.d + 1) & 0x7F;
        
        /* 7. Complex addressing with multiple operations */
        /* Pointer dereference with arithmetic */
        volatile int* ptr = &array[0];
        ptr += (i % 8);
        *ptr = *ptr ^ temp;  /* XOR operation through pointer */
        
        /* 8. Accumulate results to prevent optimization */
        result += pdata.a + pdata.b + pdata.c + pdata.d;
        result += array[i % 16];
        
        /* Update global counter with bit manipulation */
        global_counter = (global_counter + 1) & 0x3FF;
    }
    
    /* Additional operations to ensure coverage */
    {
        /* Nested bit-field operations */
        struct __attribute__((packed)) {
            unsigned int x : 6;
            unsigned int y : 10;
            unsigned int z : 16;
        } local_packed;
        
        volatile int* volatile_ptr;
        
        /* Multiple levels of indirection and bit manipulation */
        for (j = 0; j < 50; j++) {
            local_packed.x = (j * 3) & 0x3F;
            local_packed.y = (local_packed.y + local_packed.x) & 0x3FF;
            local_packed.z = (local_packed.z << 1) | (local_packed.y & 1);
            
            /* Complex array indexing */
            volatile_ptr = &array[(local_packed.x + local_packed.y) % 16];
            *volatile_ptr = *volatile_ptr + local_packed.z;
            
            result += local_packed.z;
        }
    }
    
    /* Print result to ensure all operations are observable */
    printf("Result: %d\n", result);
    
    return 0;
}
