/* test_reload_coverage.c
 * Designed to trigger various reload types in GCC's reload pass
 * Specifically targets the switch cases in chain_reload_insns()
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex data structures to force complex addressing */
typedef struct {
    int data[8];
    int* ptr;
    int offset;
} InnerStruct;

typedef struct {
    InnerStruct inner[4];
    int matrix[4][4];
    volatile int* volatile_index;
} OuterStruct;

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int* volatile_ptr = NULL;

/* Function to force RELOAD_FOR_OPERAND_ADDRESS */
void __attribute__((noinline)) 
use_complex_address(InnerStruct* addr) {
    asm volatile("" : : "r"(addr) : "memory");
    global_counter++;
}

/* Test RELOAD_FOR_INPUT_ADDRESS */
int __attribute__((noinline, optimize("O2")))
test_input_address(OuterStruct* os, int idx1, int idx2, int idx3) {
    int result = 0;
    
    /* Complex addressing that requires input address reload */
    for (int i = 0; i < 4; i++) {
        /* Multiple index computations in address */
        int complex_idx = (idx1 + i) * (idx2 + 1) + idx3;
        
        /* Inline asm with memory input using complex address */
        asm volatile(
            "movl (%[addr]), %%eax\n\t"
            "addl %%eax, %[res]\n\t"
            : [res] "+r" (result)
            : [addr] "r" (&os->inner[i].data[complex_idx & 7])
            : "eax", "memory"
        );
        
        /* Another complex access pattern */
        asm volatile(
            "movl (%[base],%[index],4), %%ebx\n\t"
            "addl %%ebx, %[res]\n\t"
            : [res] "+r" (result)
            : [base] "r" (os->matrix[0]),
              [index] "r" ((idx1 * i + idx2) & 3)
            : "ebx", "memory"
        );
    }
    
    return result;
}

/* Test RELOAD_FOR_OUTPUT_ADDRESS */
void __attribute__((noinline, optimize("O2")))
test_output_address(OuterStruct* os, int* indices, int count) {
    /* Complex output addressing patterns */
    for (int i = 0; i < count; i++) {
        int idx = indices[i];
        
        /* Output to memory with complex address computation */
        asm volatile(
            "movl %[val], (%[base],%[idx],4)\n\t"
            : 
            : [base] "r" (os->matrix[2]),
              [idx] "r" ((idx * 3 + i) & 3),
              [val] "r" (i * 100)
            : "memory"
        );
        
        /* Nested structure output with offset */
        int offset = os->inner[i].offset;
        asm volatile(
            "movl %[val], (%[ptr],%[off],4)\n\t"
            : 
            : [ptr] "r" (os->inner[idx & 3].ptr),
              [off] "r" (offset & 3),
              [val] "r" (global_counter)
            : "memory"
        );
    }
}

/* Test RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
int __attribute__((noinline, optimize("O3")))
test_mixed_address(OuterStruct* os, int idx) {
    int temp[4] = {0};
    int result = 0;
    
    /* Mixed input/output with addressing */
    for (int i = 0; i < 4; i++) {
        /* Complex input address */
        int* input_addr = &os->inner[(idx + i) & 3].data[i];
        
        /* Complex output address */
        int* output_addr = &temp[(i * 2) & 3];
        
        /* Inline asm using both addresses */
        asm volatile(
            "movl (%[in]), %%eax\n\t"
            "imull $2, %%eax\n\t"
            "movl %%eax, (%[out])\n\t"
            "addl %%eax, %[res]\n\t"
            : [res] "+r" (result)
            : [in] "r" (input_addr),
              [out] "r" (output_addr)
            : "eax", "memory"
        );
    }
    
    /* Force address of temp array to be used */
    use_complex_address((InnerStruct*)temp);
    
    return result;
}

/* Test RELOAD_FOR_OTHER_ADDRESS */
void __attribute__((noinline))
test_other_address(volatile int** ptr_array, int size) {
    /* Multiple volatile accesses with address computations */
    for (int i = 0; i < size; i++) {
        /* Force address reload for volatile access */
        volatile int* addr = ptr_array[i] + (i * global_counter);
        
        /* Multiple operations forcing different reload types */
        asm volatile(
            "movl (%[addr1]), %%ecx\n\t"
            "addl $1, %%ecx\n\t"
            "movl %%ecx, (%[addr2])\n\t"
            : 
            : [addr1] "r" (addr),
              [addr2] "r" (ptr_array[(i + 1) % size])
            : "ecx", "memory"
        );
    }
}

/* Test RELOAD_OTHER type */
int __attribute__((noinline, optimize("O2")))
test_other_reloads(int* data, int n) {
    int sum = 0;
    
    /* Pattern that might trigger RELOAD_OTHER */
    for (int i = 0; i < n; i++) {
        /* Multiple constraints forcing unusual reloads */
        asm volatile(
            "movl %[inc], %%edx\n\t"
            "addl %%edx, (%[data])\n\t"
            "movl (%[data]), %%edx\n\t"
            "addl %%edx, %[sum]\n\t"
            : [sum] "+r" (sum),
              [data] "+r" (&data[i])
            : [inc] "r" (i)
            : "edx", "memory"
        );
    }
    
    return sum;
}

/* Main driver function */
int main() {
    /* Initialize test data */
    OuterStruct os;
    int indices[8] = {0, 1, 2, 3, 0, 1, 2, 3};
    int data[16];
    volatile int* ptr_array[4];
    
    /* Initialize structures */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            os.inner[i].data[j] = i * 100 + j;
        }
        os.inner[i].ptr = &data[i * 4];
        os.inner[i].offset = i;
        
        for (int j = 0; j < 4; j++) {
            os.matrix[i][j] = i * 10 + j;
        }
        
        ptr_array[i] = &data[i * 4];
    }
    
    os.volatile_index = &global_counter;
    
    for (int i = 0; i < 16; i++) {
        data[i] = i;
    }
    
    /* Run tests to trigger different reload types */
    int result = 0;
    
    /* Test 1: Input address reloads */
    result += test_input_address(&os, 1, 2, 3);
    
    /* Test 2: Output address reloads */
    test_output_address(&os, indices, 4);
    
    /* Test 3: Mixed address reloads */
    result += test_mixed_address(&os, 2);
    
    /* Test 4: Other address reloads */
    test_other_address(ptr_array, 4);
    
    /* Test 5: Other reload types */
    result += test_other_reloads(data, 8);
    
    /* Use result to prevent dead code elimination */
    printf("Final result: %d\n", result);
    printf("Global counter: %d\n", global_counter);
    
    return result != 0 ? 0 : 1;
}
