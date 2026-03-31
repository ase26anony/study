/* reload_coverage.c - Complex program to trigger GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force no optimization on specific functions */
#define NOINLINE __attribute__((noinline, noipa))
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Global volatile variables to prevent constant propagation */
volatile int v1, v2, v3, v4, v5, v6, v7, v8;
volatile long vl1, vl2;
volatile double vd1, vd2;

/* Complex addressing structure */
struct MultiLevel {
    int *ptr1;
    int **ptr2;
    int ***ptr3;
    volatile int idx;
};

NOINLINE static int trigger_reloads(
    volatile int idx1, volatile int idx2, 
    volatile int idx3, volatile int idx4,
    volatile long offset1, volatile long offset2,
    volatile double scale1, volatile double scale2)
{
    /* Create high register pressure with many live values */
    int local1 = idx1 * 2;
    int local2 = idx2 + 3;
    int local3 = idx3 - 1;
    int local4 = idx4 * 4;
    double dlocal1 = scale1;
    double dlocal2 = scale2;
    double dlocal3 = scale1 * scale2;
    
    /* Multi-dimensional arrays for complex addressing */
    int arr3d[4][8][16];
    double darr2d[32][64];
    char carr[256];
    
    /* Pointer chains for multi-level indirection */
    int *ptr_arr[32];
    int **ptr_to_ptr[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        carr[i] = (char)(i + idx1);
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 64; j++) {
            darr2d[i][j] = (double)(i * j) * scale1;
        }
    }
    
    /* Complex addressing pattern 1: Multi-dimensional array with volatile indices */
    /* This should trigger RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
    int sum = 0;
    for (volatile int i = 0; i < 4; i++) {
        for (volatile int j = 0; j < 8; j++) {
            /* Complex address calculation that needs temporary registers */
            arr3d[i][j][idx1 + idx2 * idx3] = 
                carr[(i * 64 + j * 8 + idx4) & 255] * 
                (idx1 + idx2 - idx3);
            sum += arr3d[i][j][idx1 + idx2 * idx3];
        }
    }
    
    COMPILER_BARRIER();
    
    /* Complex addressing pattern 2: Pointer arithmetic with mixed types */
    /* This should trigger RELOAD_FOR_OUTPUT_ADDRESS */
    long *lptr = (long*)carr;
    int *iptr = (int*)&darr2d[0][0];
    double *dptr = darr2d[idx1 % 32];
    
    /* Mixed pointer arithmetic forcing address reloads */
    for (volatile int i = 0; i < 8; i++) {
        /* Complex address: base + index*scale + displacement */
        int *addr1 = &iptr[offset1 + i * (idx2 % 8)];
        double *addr2 = &dptr[offset2 + (idx3 + i) * 2];
        
        /* Force register pressure with mixed operations */
        *addr1 = (int)(*addr2 * scale2) + idx4;
        lptr[i] = (long)*addr1 * (idx1 + i);
    }
    
    COMPILER_BARRIER();
    
    /* Inline assembly to force specific reload types */
    /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
    int asm_result1, asm_result2;
    double asm_double;
    
    /* Assembly with memory operand constraints */
    __asm__ volatile(
        "movl %[input1], %%eax\n\t"
        "addl %[input2], %%eax\n\t"
        "movl %%eax, %[output1]\n\t"
        : [output1] "=r" (asm_result1)
        : [input1] "m" (arr3d[idx1 % 4][idx2 % 8][0]),
          [input2] "r" (idx3)
        : "%eax", "memory"
    );
    
    /* Assembly with output address reload */
    __asm__ volatile(
        "leaq %[addr], %%rax\n\t"
        "movq (%%rax), %%rbx\n\t"
        "addq %[offset], %%rbx\n\t"
        "movq %%rbx, %[result]\n\t"
        : [result] "=r" (asm_result2)
        : [addr] "m" (lptr),
          [offset] "r" (offset1)
        : "%rax", "%rbx", "memory"
    );
    
    COMPILER_BARRIER();
    
    /* Mixed register class pressure: integer and floating point */
    /* This should trigger RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS */
    double fp_sum = 0.0;
    for (volatile int i = 0; i < 16; i++) {
        /* Complex floating point computation with address calculation */
        double *fp_addr = &darr2d[(idx1 + i) % 32][(idx2 * 2 + idx3) % 64];
        double temp = *fp_addr * scale1 + (double)i * scale2;
        
        /* Convert to integer and back to force register moves */
        int int_temp = (int)temp;
        fp_sum += (double)int_temp * dlocal1 - dlocal2;
        
        /* Store with complex address */
        carr[(i * 13 + idx4) % 256] = (char)int_temp;
    }
    
    COMPILER_BARRIER();
    
    /* Multi-level pointer indirection for address reloads */
    int level1[128];
    int *level2[64];
    int **level3[32];
    
    for (int i = 0; i < 128; i++) {
        level1[i] = i + idx1;
    }
    
    for (int i = 0; i < 64; i++) {
        level2[i] = &level1[(i * 2 + idx2) % 128];
    }
    
    for (int i = 0; i < 32; i++) {
        level3[i] = &level2[(i * 3 + idx3) % 64];
    }
    
    /* Complex chain dereference - should need multiple address reloads */
    int chain_sum = 0;
    for (volatile int i = 0; i < 8; i++) {
        int ***ptr3 = &level3[(i + idx4) % 32];
        int **ptr2 = *ptr3;
        int *ptr1 = ptr2[(i * 5) % 64];
        chain_sum += *ptr1 * (i + 1);
    }
    
    /* Final computation mixing all types */
    double final_result = (double)sum * scale1 + 
                         (double)chain_sum * scale2 + 
                         fp_sum + 
                         (double)asm_result1 + 
                         (double)asm_result2;
    
    /* Force use of all local variables to maintain live ranges */
    local1 += (int)final_result;
    local2 += asm_result1;
    local3 += asm_result2;
    local4 += chain_sum;
    dlocal1 += final_result;
    dlocal2 += fp_sum;
    dlocal3 += scale1 * scale2;
    
    /* Return volatile sum to prevent optimization */
    return (int)(final_result + local1 + local2 + local3 + local4 + 
                 dlocal1 + dlocal2 + dlocal3 + idx1 + idx2);
}

int main(void) {
    srand(time(NULL));
    
    /* Initialize volatile variables with random values */
    v1 = rand() % 100;
    v2 = rand() % 100;
    v3 = rand() % 100;
    v4 = rand() % 100;
    v5 = rand() % 100;
    v6 = rand() % 100;
    v7 = rand() % 100;
    v8 = rand() % 100;
    vl1 = rand() % 1000;
    vl2 = rand() % 1000;
    vd1 = (double)(rand() % 100) / 10.0;
    vd2 = (double)(rand() % 100) / 10.0;
    
    /* Call the complex function multiple times with different args */
    int total = 0;
    for (int i = 0; i < 10; i++) {
        total += trigger_reloads(
            v1 + i, v2 - i, v3 * (i + 1), v4 + i * 2,
            vl1 + i * 100, vl2 - i * 50,
            vd1 + (double)i, vd2 - (double)i * 0.5
        );
    }
    
    printf("Result: %d\n", total);
    
    /* Additional test with different patterns */
    struct MultiLevel ml;
    int base_array[256];
    int *ptr_array[128];
    
    for (int i = 0; i < 256; i++) {
        base_array[i] = i * 2;
    }
    
    for (int i = 0; i < 128; i++) {
        ptr_array[i] = &base_array[(i * 3) % 256];
    }
    
    ml.ptr1 = &base_array[0];
    ml.ptr2 = &ptr_array[0];
    ml.ptr3 = (int***)&ml.ptr2;
    ml.idx = v1;
    
    /* Complex structure access */
    int struct_result = 0;
    for (volatile int i = 0; i < 16; i++) {
        struct_result += *(*ml.ptr2 + ml.idx + i) * (i + 1);
    }
    
    printf("Structure result: %d\n", struct_result);
    
    return 0;
}
