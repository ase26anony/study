/* reload_coverage.c - Program to trigger multiple reload types in GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force no optimization across function boundaries */
#define NO_OPT __attribute__((noinline, noipa))

/* Memory barrier to prevent reordering */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Volatile variables to prevent constant propagation */
static volatile int v1, v2, v3, v4, v5, v6, v7, v8;
static volatile long vl1, vl2;
static volatile double vd1, vd2;

/* Complex addressing structure */
struct nested {
    int *ptr1;
    int **ptr2;
    long offset;
};

NO_OPT static long trigger_reloads(
    volatile int idx1, volatile int idx2, 
    volatile int idx3, volatile int idx4,
    volatile long lval1, volatile long lval2,
    volatile double dval1, volatile double dval2)
{
    /* Create high register pressure with many live values */
    int local1 = idx1 * 2;
    int local2 = idx2 + idx3;
    int local3 = idx4 ^ 0xABCD;
    long local4 = lval1;
    long local5 = lval2;
    double local6 = dval1;
    double local7 = dval2;
    
    /* Multi-dimensional arrays for complex addressing */
    int arr3d[4][8][16];
    double darr[64][32];
    char carr[128][256];
    
    /* Pointer chains */
    int *ptr_a = &local1;
    int **ptr_b = &ptr_a;
    int ***ptr_c = &ptr_b;
    
    struct nested nested1, nested2;
    nested1.ptr1 = &local2;
    nested1.ptr2 = &ptr_a;
    nested1.offset = lval1;
    
    COMPILER_BARRIER();
    
    /* ========== RELOAD_FOR_INPUT_ADDRESS ========== */
    /* Complex address calculation requiring temporary register */
    int sum1 = 0;
    for (volatile int i = 0; i < 4; i++) {
        /* Multi-level array access with volatile indices */
        sum1 += arr3d[idx1 % 4][(idx2 + i) % 8][(idx3 * i) % 16];
        
        /* Pointer arithmetic with mixed types */
        char *cptr = (char*)carr[idx1 % 128] + idx2 * sizeof(int) + idx3;
        sum1 += *((int*)cptr);
    }
    
    COMPILER_BARRIER();
    
    /* ========== RELOAD_FOR_OUTPUT_ADDRESS ========== */
    /* Store through complex computed address */
    double *dptr;
    for (volatile int i = 0; i < 8; i++) {
        /* Complex address for store operation */
        dptr = &darr[(idx1 + i) % 64][(idx2 * i) % 32];
        *dptr = local6 * (i + 1) + local7;
        
        /* Chain dereference with offset */
        *(nested1.ptr2) = &arr3d[i % 4][0][0];
        **nested1.ptr2 = local1 + i;
    }
    
    COMPILER_BARRIER();
    
    /* ========== RELOAD_FOR_OPERAND_ADDRESS ========== */
    /* Inline assembly with memory operand constraints */
    long asm_result1, asm_result2;
    
    /* Assembly that takes complex address as input */
    __asm__ volatile (
        "movq (%[input]), %[output]\n\t"
        "addq $1, %[output]"
        : [output] "=&r" (asm_result1)
        : [input] "r" (&arr3d[idx1 % 4][idx2 % 8][0])
        : "memory"
    );
    
    /* Another assembly with different constraints */
    __asm__ volatile (
        "lea (%[base], %[index], 4), %[out]\n\t"
        "movq (%[out]), %[out]"
        : [out] "=&r" (asm_result2)
        : [base] "r" (arr3d[0][0]),
          [index] "r" ((long)(idx3 * 16 + idx4))
        : "memory"
    );
    
    COMPILER_BARRIER();
    
    /* ========== RELOAD_FOR_INPADDR_ADDRESS ========== */
    /* Address of address computation */
    int **addr_of_addr = &ptr_a;
    for (volatile int i = 0; i < 3; i++) {
        /* Complex addressing mode */
        int *temp = *addr_of_addr + idx1 * i + idx2;
        sum1 += *temp;
        
        /* Change the pointer target */
        addr_of_addr = (i & 1) ? &ptr_a : &nested1.ptr1;
    }
    
    COMPILER_BARRIER();
    
    /* ========== RELOAD_FOR_OUTADDR_ADDRESS ========== */
    /* Compute address for output then use it */
    int *out_addr;
    __asm__ volatile (
        "lea (%[base], %[idx], 4), %[addr]"
        : [addr] "=r" (out_addr)
        : [base] "r" (arr3d[0][0]),
          [idx] "r" ((long)(idx1 + idx2))
    );
    
    /* Store through computed address */
    *out_addr = local1 + local2;
    
    COMPILER_BARRIER();
    
    /* ========== RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS ========== */
    /* Mixed register class pressure */
    double fp_sum = 0.0;
    for (volatile int i = 0; i < 32; i++) {
        /* Integer to float conversion */
        fp_sum += (double)arr3d[0][i % 8][i % 16];
        
        /* Float operations */
        darr[i % 64][(i * 2) % 32] = fp_sum * dval1;
        
        /* More complex addressing */
        int *int_ptr = (int*)((char*)carr[i % 128] + idx3 * sizeof(int));
        *int_ptr = (int)(fp_sum * 100.0);
    }
    
    COMPILER_BARRIER();
    
    /* ========== RELOAD_FOR_OPADDR_ADDR ========== */
    /* Operand address of operand address */
    long complex_result = 0;
    for (volatile int i = 0; i < 4; i++) {
        /* Triple pointer dereference */
        complex_result += ***ptr_c;
        
        /* Update pointer chain */
        *ptr_a = idx1 + i;
        ptr_a = &arr3d[i % 4][0][0];
        ptr_b = &ptr_a;
        ptr_c = &ptr_b;
    }
    
    COMPILER_BARRIER();
    
    /* Final computation mixing all types */
    long final_result = (long)sum1 + (long)fp_sum + asm_result1 + asm_result2 + complex_result;
    
    /* Force use of all local variables */
    final_result += local1 + local2 + local3 + local4 + local5 + (long)local6 + (long)local7;
    
    /* Access through all arrays */
    final_result += arr3d[0][0][0] + (long)darr[0][0] + carr[0][0];
    
    return final_result;
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
    vd1 = (double)(rand() % 1000) / 10.0;
    vd2 = (double)(rand() % 1000) / 10.0;
    
    /* Call the function multiple times with different arguments */
    long total = 0;
    for (int i = 0; i < 10; i++) {
        total += trigger_reloads(
            v1 + i, v2 - i, v3 * (i + 1), v4 ^ i,
            vl1 + i * 10, vl2 - i * 5,
            vd1 + i, vd2 - i
        );
        
        /* Modify volatiles to prevent optimization */
        COMPILER_BARRIER();
        v1 += 1;
        v2 -= 1;
        vd1 += 0.5;
    }
    
    printf("Result: %ld\n", total);
    
    /* Dump reload information if compiled with -fdump-rtl-reload */
    #ifdef __GNUC__
    __asm__ volatile("# Reload coverage test complete");
    #endif
    
    return 0;
}
