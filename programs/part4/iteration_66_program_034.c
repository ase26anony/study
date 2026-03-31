/* test_reload_coverage.c
 * Designed to trigger various reload types in GCC's reload pass
 * Specifically targets the switch cases in chain_reload_insns()
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex data structures to force address computations */
typedef struct {
    int data[8];
    int* ptr;
    int offset;
} InnerStruct;

typedef struct {
    InnerStruct inner[4];
    int matrix[4][4];
    volatile int* volatile_ptr;
} OuterStruct;

/* Global variables to increase register pressure */
volatile int global_index = 0;
volatile int global_offset = 0;
OuterStruct global_struct;

/* Function to prevent optimization */
static void use_value(int val) {
    asm volatile("" : : "r"(val) : "memory");
}

/* Test RELOAD_FOR_INPUT_ADDRESS */
void test_input_address(OuterStruct* os, int idx1, int idx2, int idx3) {
    /* Complex addressing: array[struct.member + index + offset] */
    int val;
    
    /* Force input address reload with multiple register components */
    asm volatile(
        "movl %[input], %[output]\n\t"
        : [output] "=r"(val)
        : [input] "m"(os->inner[idx1].data[idx2 + idx3 + global_offset])
        : "memory"
    );
    
    use_value(val);
    
    /* Nested structure access with volatile */
    val = os->inner[global_index].data[(idx1 << 2) + idx2];
    use_value(val);
}

/* Test RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(OuterStruct* os, int* indices, int count) {
    /* Complex output addressing with computed offsets */
    for (int i = 0; i < count; i++) {
        int offset = (indices[i] * 3) & 7;
        
        /* Force output address reload */
        asm volatile(
            "movl $42, %[output]\n\t"
            : [output] "=m"(os->inner[i].data[offset])
            :
            : "memory"
        );
        
        /* Mixed input/output with different addressing */
        int temp = os->matrix[i][offset];
        asm volatile(
            "addl $1, %[output]\n\t"
            : [output] "+m"(os->inner[global_index].data[i])
            : [input] "m"(os->matrix[offset][i])
            : "memory"
        );
        
        use_value(temp);
    }
}

/* Test RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_inpaddr_outaddr(OuterStruct* os, int base_idx) {
    volatile int* volatile ptr = &os->inner[0].data[0];
    int idx1 = base_idx + 1;
    int idx2 = base_idx + 2;
    
    /* Complex addressing in both input and output */
    asm volatile(
        "movl (%[in_addr]), %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, (%[out_addr])\n\t"
        :
        : [in_addr] "r"(&os->inner[idx1].data[idx2]),
          [out_addr] "r"(&os->matrix[idx2][idx1])
        : "eax", "memory"
    );
    
    /* Chain of address computations */
    int* addr1 = &os->inner[base_idx].data[global_index];
    int* addr2 = addr1 + (idx1 * idx2);
    
    asm volatile(
        "movl (%[addr]), %%ebx\n\t"
        : 
        : [addr] "r"(addr2)
        : "ebx", "memory"
    );
}

/* Test RELOAD_FOR_OPERAND_ADDRESS */
void helper_func(int* addr1, int* addr2, int* addr3) {
    /* Force address computations before call */
    asm volatile(
        "movl (%[a1]), %%ecx\n\t"
        "addl (%[a2]), %%ecx\n\t"
        "movl %%ecx, (%[a3])\n\t"
        :
        : [a1] "r"(addr1), [a2] "r"(addr2), [a3] "r"(addr3)
        : "ecx", "memory"
    );
}

void test_operand_address(OuterStruct* os, int i, int j, int k) {
    /* Complex address expressions as function arguments */
    helper_func(
        &os->inner[i].data[j + k],
        &os->matrix[j][k + global_offset],
        &os->inner[k].data[i * j]
    );
    
    /* More complex nested addressing */
    helper_func(
        &os->inner[global_index].data[(i << 2) | j],
        &os->matrix[k][(i + j) & 3],
        os->volatile_ptr + (i * j * k)
    );
}

/* Test RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
void test_other_address(OuterStruct* os, int* indices, int n) {
    int sum = 0;
    
    /* Loop with complex addressing that changes each iteration */
    for (int i = 0; i < n; i++) {
        int idx = indices[i] & 3;
        int offset = (indices[i] * 7) & 7;
        
        /* Mixed input/output with memory clobber */
        asm volatile(
            "movl (%[in]), %%edx\n\t"
            "imull %%edx, %%edx\n\t"
            "movl %%edx, (%[out])\n\t"
            "addl $1, (%[out2])\n\t"
            :
            : [in] "r"(&os->inner[idx].data[offset]),
              [out] "r"(&os->matrix[idx][offset]),
              [out2] "r"(&os->inner[offset].data[idx])
            : "edx", "memory"
        );
        
        /* Force spill/reload of address registers */
        sum += os->inner[idx].data[offset] + 
               os->matrix[offset][idx] + 
               os->inner[global_index].data[(i + offset) & 7];
    }
    
    use_value(sum);
}

/* Test mixed reload types in a single function */
void test_mixed_reloads(OuterStruct* os, int iter) {
    volatile int vi = iter;
    volatile int vj = iter * 2;
    volatile int vk = iter * 3;
    
    /* Multiple inline asm statements with different constraints */
    
    /* Output address reload */
    asm volatile(
        "movl $100, %[out]\n\t"
        : [out] "=m"(os->inner[vi].data[vj + vk])
        :
        : "memory"
    );
    
    /* Input address reload */
    int temp;
    asm volatile(
        "movl %[in], %[out]\n\t"
        : [out] "=r"(temp)
        : [in] "m"(os->matrix[vj][vk + global_offset])
        : "memory"
    );
    
    /* Mixed input/output with complex addressing */
    asm volatile(
        "movl (%[in1]), %%esi\n\t"
        "addl (%[in2]), %%esi\n\t"
        "movl %%esi, (%[out])\n\t"
        :
        : [in1] "r"(&os->inner[vi].data[vj]),
          [in2] "r"(&os->matrix[vk][vi]),
          [out] "r"(&os->inner[vj].data[vk])
        : "esi", "memory"
    );
    
    /* Operand address reload via function call */
    test_operand_address(os, vi, vj, vk);
}

/* Main driver */
int main() {
    /* Initialize test data */
    OuterStruct os;
    int indices[] = {0, 1, 2, 3, 0, 2, 1, 3};
    int* dynamic_array = malloc(64 * sizeof(int));
    
    /* Initialize structures */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            os.inner[i].data[j] = i * 8 + j;
        }
        for (int j = 0; j < 4; j++) {
            os.matrix[i][j] = i * 4 + j;
        }
    }
    os.volatile_ptr = dynamic_array;
    
    /* Run tests to trigger different reload types */
    for (int i = 0; i < 8; i++) {
        global_index = i & 3;
        global_offset = i & 7;
        
        test_input_address(&os, i & 3, (i + 1) & 3, (i + 2) & 3);
        test_output_address(&os, indices, 4);
        test_inpaddr_outaddr(&os, i & 3);
        test_operand_address(&os, i & 3, (i + 1) & 3, (i + 2) & 3);
        test_other_address(&os, indices, 8);
        test_mixed_reloads(&os, i);
    }
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            checksum ^= os.inner[i].data[j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(dynamic_array);
    return 0;
}
