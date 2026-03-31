/* reload_coverage.c - Program to trigger multiple reload types in GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force no optimization across function boundaries */
#define NO_OPT __attribute__((noinline, noipa, noclone))

/* Memory barrier to prevent reordering */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Volatile variables to prevent constant propagation */
static volatile int volatile_idx1;
static volatile int volatile_idx2;
static volatile int volatile_idx3;
static volatile int volatile_stride;
static volatile double volatile_scale;
static volatile long volatile_offset;

/* Complex addressing function that triggers multiple reload types */
NO_OPT static double trigger_reloads(int *base_arr, double *dbl_arr, 
                                     char *char_arr, int v1, int v2, 
                                     int v3, int stride, double scale, long offset) {
    /* Local variables to create register pressure */
    int local1 = v1 * 2;
    int local2 = v2 + v3;
    int local3 = v1 ^ v2;
    double local_d1 = scale * 2.0;
    double local_d2 = scale / 3.0;
    int local4 = v3 << 2;
    double local_d3 = local_d1 + local_d2;
    int local5 = local1 | local3;
    
    /* Multi-dimensional array simulation with complex addressing */
    int md_arr[10][20];
    double dbl_md_arr[5][10];
    
    /* Force register pressure with many live values */
    COMPILER_BARRIER();
    
    /* RELOAD_FOR_INPUT_ADDRESS: Complex address calculation */
    /* Base + index * scale + displacement with volatile components */
    int *addr1 = &base_arr[(v1 * stride + v2) * 3 + v3];
    COMPILER_BARRIER();
    
    /* RELOAD_FOR_INPUT: Value needs reloading */
    int val1 = *addr1 + local1;
    COMPILER_BARRIER();
    
    /* RELOAD_FOR_INPADDR_ADDRESS: Address of input address */
    int **addr_of_addr = &addr1;
    COMPILER_BARRIER();
    
    /* Multi-level pointer chain for address reloads */
    char *ptr1 = char_arr + offset;
    char *ptr2 = ptr1 + (v1 * sizeof(int));
    char *ptr3 = ptr2 + (v2 * sizeof(double));
    
    /* RELOAD_FOR_OUTPUT_ADDRESS: Complex store address */
    double *dbl_ptr = &dbl_arr[(v2 * 7 + v3) % 50];
    COMPILER_BARRIER();
    
    /* Inline assembly to force specific reload types */
    int asm_result1, asm_result2;
    double asm_dbl_result;
    
    /* RELOAD_FOR_OPERAND_ADDRESS: Address as assembly operand */
    __asm__ volatile (
        "movl (%[addr]), %[result]"
        : [result] "=r" (asm_result1)
        : [addr] "m" (*addr_of_addr)
        : "memory"
    );
    COMPILER_BARRIER();
    
    /* RELOAD_FOR_OUTADDR_ADDRESS: Output address reload */
    int *output_addr;
    __asm__ volatile (
        "leal (%[base], %[index], 4), %[out]"
        : [out] "=r" (output_addr)
        : [base] "r" (base_arr), [index] "r" (v1)
        : "cc"
    );
    COMPILER_BARRIER();
    
    /* Mixed register class pressure */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            /* Integer and FP operations interleaved */
            dbl_md_arr[i][j] = (double)md_arr[i][j] * scale;
            md_arr[i][j] = (int)(dbl_md_arr[i][j] * 2.0) + local1;
            
            /* Complex addressing with multiple volatile components */
            if (i + j < 15) {
                char *char_ptr = ptr3 + (i * stride + j) * sizeof(int);
                int char_val = *((int*)char_ptr);
                dbl_md_arr[i][j] += (double)char_val;
            }
        }
    }
    
    COMPILER_BARRIER();
    
    /* RELOAD_FOR_OTHER_ADDRESS: Other address calculations */
    long *long_ptr = (long*)((char*)base_arr + offset + v1 * 8);
    COMPILER_BARRIER();
    
    /* More inline assembly with constraints */
    __asm__ volatile (
        "movq %[in], %%rax\n\t"
        "addq $16, %%rax\n\t"
        "movq %%rax, %[out]"
        : [out] "=r" (asm_result2)
        : [in] "r" (long_ptr)
        : "rax", "cc"
    );
    
    /* RELOAD_OTHER: Miscellaneous reloads */
    /* Force spills with many simultaneous live values */
    double sum = 0.0;
    for (int i = 0; i < 10; i++) {
        int idx = (v1 + i * v2) % 100;
        sum += dbl_arr[idx] * (double)local2;
        sum -= (double)base_arr[(idx + v3) % 100];
        sum *= 1.0001;
        
        /* Keep many values live */
        local_d1 += 0.5;
        local_d2 -= 0.3;
        local_d3 = local_d1 * local_d2;
        local4 ^= idx;
        local5 += local4;
    }
    
    COMPILER_BARRIER();
    
    /* Final complex expression with many operands */
    double result = sum + local_d1 - local_d2 + local_d3 
                    + (double)asm_result1 + (double)asm_result2
                    + (double)(*output_addr) + (double)val1;
    
    return result * volatile_scale;
}

int main(void) {
    srand(time(NULL));
    
    /* Initialize volatile variables with random values */
    volatile_idx1 = rand() % 50;
    volatile_idx2 = rand() % 50;
    volatile_idx3 = rand() % 50;
    volatile_stride = 10 + rand() % 20;
    volatile_scale = 1.0 + (double)(rand() % 100) / 50.0;
    volatile_offset = rand() % 100;
    
    /* Allocate and initialize arrays */
    int *int_array = (int*)malloc(100 * sizeof(int));
    double *double_array = (double*)malloc(100 * sizeof(double));
    char *char_array = (char*)malloc(200 * sizeof(char));
    
    if (!int_array || !double_array || !char_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with random data */
    for (int i = 0; i < 100; i++) {
        int_array[i] = rand() % 1000;
        double_array[i] = (double)(rand() % 1000) / 10.0;
    }
    for (int i = 0; i < 200; i++) {
        char_array[i] = rand() % 256;
    }
    
    /* Call function with complex parameters */
    double result = trigger_reloads(
        int_array,
        double_array,
        char_array,
        volatile_idx1,
        volatile_idx2,
        volatile_idx3,
        volatile_stride,
        volatile_scale,
        volatile_offset
    );
    
    printf("Result: %f\n", result);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(char_array);
    
    return 0;
}
