/* reload_coverage.c - Complex program to trigger GCC's reload pass edge cases */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define FORCE_SPILL __attribute__((noinline, noipa))

/* Complex addressing structure */
struct MultiLevel {
    int *level1;
    long **level2;
    double ***level3;
};

/* Force register pressure with many live values */
FORCE_SPILL 
double trigger_reloads(
    volatile int v1, volatile int v2, volatile int v3,
    volatile long v4, volatile int v5, volatile int v6,
    int *ext_arr1, double *ext_arr2, struct MultiLevel *ml)
{
    /* Create high register pressure with many locals */
    int local_arr1[100];
    double local_arr2[50];
    int local_arr3[75];
    double local_arr4[60];
    
    /* Various pointer types for different addressing modes */
    char *char_ptr;
    int *int_ptr;
    long *long_ptr;
    double *double_ptr;
    
    /* Intermediate computation results */
    int sum_int = 0;
    double sum_double = 0.0;
    long product_long = 1;
    
    /* Force complex address calculations */
    char_ptr = (char *)local_arr1 + v1 * sizeof(int) + v2;
    int_ptr = (int *)(char_ptr + v3);
    long_ptr = (long *)(int_ptr + v4);
    
    /* Memory barrier to prevent optimization */
    __asm__ volatile("" : : : "memory");
    
    /* RELOAD_FOR_INPUT_ADDRESS: Complex array indexing */
    for (volatile int i = 0; i < v5 % 10; i++) {
        /* Multi-dimensional access simulation */
        int idx1 = (v1 + i) % 100;
        int idx2 = (v2 * i) % 100;
        int idx3 = (v3 + idx1 * idx2) % 100;
        
        /* Complex address calculation requiring temporary register */
        local_arr1[idx1 + idx2 * v6 + idx3 / (v5 + 1)] = 
            ext_arr1[idx1] + ext_arr1[idx2] * (v4 % 100);
    }
    
    __asm__ volatile("" : : : "memory");
    
    /* RELOAD_FOR_OUTPUT_ADDRESS: Store with complex address */
    for (volatile int j = 0; j < v2 % 8; j++) {
        /* Even more complex addressing */
        double *target = &local_arr2[
            (v1 * j + v3 * (j + 1)) % 50 +
            (v4 * (v5 + j)) % 50
        ];
        
        /* Mixed-type computation */
        *target = (double)local_arr1[
            (v6 * j + v2) % 100
        ] * 3.14159 / (j + 1);
        
        /* Force floating-point register pressure */
        sum_double += *target * (j + 1);
    }
    
    /* Inline assembly to force specific reload types */
    
    /* RELOAD_FOR_OPERAND_ADDRESS: Address as assembly operand */
    __asm__ volatile(
        "movq %[addr], %%rax\n\t"
        "addl $1, (%%rax)"
        : 
        : [addr] "r" (&local_arr1[v1 % 100])
        : "rax", "memory", "cc"
    );
    
    /* RELOAD_FOR_OUTADDR_ADDRESS: Output address reload */
    int output1, output2;
    __asm__ volatile(
        "leaq %[in], %%rax\n\t"
        "movl (%%rax), %%ebx\n\t"
        "movl %%ebx, %[out1]\n\t"
        "addl $100, %%ebx\n\t"
        "movl %%ebx, %[out2]"
        : [out1] "=r" (output1), [out2] "=r" (output2)
        : [in] "m" (local_arr3[v2 % 75])
        : "rax", "rbx", "cc"
    );
    
    /* RELOAD_FOR_INPADDR_ADDRESS: Input address with offset */
    int input_addr_result;
    __asm__ volatile(
        "movq %[base], %%rcx\n\t"
        "addq %[offset], %%rcx\n\t"
        "movl (%%rcx), %%edx\n\t"
        "movl %%edx, %[result]"
        : [result] "=r" (input_addr_result)
        : [base] "r" (local_arr1), 
          [offset] "r" ((v3 * sizeof(int)) + v4)
        : "rcx", "rdx", "cc"
    );
    
    /* Multi-level pointer chasing - triggers various address reloads */
    if (ml && ml->level2 && ml->level3) {
        /* Complex pointer arithmetic */
        long **level2_ptr = ml->level2 + v1;
        double ***level3_ptr = ml->level3 + v2;
        
        if (*level2_ptr && **level3_ptr) {
            /* Mixed-type memory access */
            sum_int += **(int **)level2_ptr;
            sum_double += ***level3_ptr;
        }
    }
    
    /* Force integer register pressure */
    for (volatile int k = 0; k < v6 % 20; k++) {
        /* Complex expression with many intermediates */
        int temp1 = local_arr1[(v1 + k) % 100];
        int temp2 = local_arr3[(v2 * k) % 75];
        int temp3 = ext_arr1[(v3 + k) % 100];
        
        /* Chain of computations keeping values live */
        product_long *= (temp1 + temp2) * (temp3 - k);
        sum_int += temp1 * temp2 + temp3 / (k + 1);
        
        /* Convert to double and back */
        double temp_d = (double)temp1 * (double)temp2;
        local_arr4[k % 60] = temp_d;
        sum_int += (int)temp_d;
    }
    
    /* RELOAD_FOR_OTHER_ADDRESS: Complex address in return */
    double *return_ptr = &local_arr2[v4 % 50] + v5;
    
    /* Mix everything in return to keep values live */
    return sum_double + (double)sum_int + (double)product_long + *return_ptr;
}

/* Helper to initialize multi-level structure */
void init_multi_level(struct MultiLevel *ml) {
    static int data1[100];
    static long *data2[50];
    static double **data3[25];
    static double row3[25][10];
    
    for (int i = 0; i < 100; i++) data1[i] = rand() % 1000;
    
    ml->level1 = data1;
    ml->level2 = data2;
    ml->level3 = data3;
    
    for (int i = 0; i < 50; i++) {
        data2[i] = (long *)&data1[i * 2];
    }
    
    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < 10; j++) {
            row3[i][j] = (double)(rand() % 1000) / 10.0;
        }
        data3[i] = (double **)&row3[i];
    }
}

int main() {
    srand(time(NULL));
    
    /* External arrays to prevent optimization */
    static int ext_arr1[200];
    static double ext_arr2[100];
    struct MultiLevel ml;
    
    /* Initialize with random values */
    for (int i = 0; i < 200; i++) ext_arr1[i] = rand() % 1000;
    for (int i = 0; i < 100; i++) ext_arr2[i] = (double)(rand() % 1000) / 10.0;
    
    init_multi_level(&ml);
    
    /* Volatile variables to prevent constant propagation */
    volatile int v1 = rand() % 100;
    volatile int v2 = rand() % 100;
    volatile int v3 = rand() % 100;
    volatile long v4 = rand() % 100;
    volatile int v5 = rand() % 100;
    volatile int v6 = rand() % 100;
    
    /* Call the reload-intensive function multiple times */
    double total = 0.0;
    for (int iter = 0; iter < 3; iter++) {
        total += trigger_reloads(
            v1 + iter, v2 + iter, v3 + iter,
            v4 + iter, v5 + iter, v6 + iter,
            ext_arr1, ext_arr2, &ml
        );
        
        /* Modify volatiles to change addressing patterns */
        v1 = (v1 * 13 + 17) % 100;
        v2 = (v2 * 7 + 23) % 100;
    }
    
    printf("Result: %f\n", total);
    
    /* Use results to prevent dead code elimination */
    if (total > 1000000.0) {
        printf("Unexpected large result\n");
    }
    
    return 0;
}
