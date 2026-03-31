/* reload_coverage.c - Program to trigger GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force no optimization across function boundaries */
#define NO_OPT __attribute__((noinline, noipa, noclone))

/* Memory barrier to prevent reordering */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Volatile variables to prevent constant propagation */
static volatile int v1, v2, v3, v4, v5, v6, v7, v8;
static volatile long v9, v10;
static volatile double vd1, vd2;

/* Complex addressing structure */
struct MultiLevel {
    int *ptr1;
    int **ptr2;
    long *ptr3;
    double *dptr;
};

/* Function to trigger multiple reload types */
NO_OPT static long trigger_reloads(
    volatile int idx1, volatile int idx2, 
    volatile int idx3, volatile int idx4,
    volatile long lval1, volatile long lval2,
    volatile double dval1, volatile double dval2)
{
    /* Create high register pressure with many live values */
    int local1 = idx1 * 2;
    int local2 = idx2 + 7;
    int local3 = idx3 - 5;
    int local4 = idx4 * 3;
    long local5 = lval1;
    long local6 = lval2;
    double local7 = dval1;
    double local8 = dval2;
    
    /* Multi-dimensional arrays for complex addressing */
    int arr2d[10][20];
    double darr[50];
    char carr[100];
    long larr[30];
    
    /* Pointer chains */
    int *ptr_arr[15];
    long *lptr_arr[10];
    
    /* Force spills across register classes */
    COMPILER_BARRIER();
    
    /* Initialize arrays */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            /* Complex addressing: arr2d[volatile_idx][volatile_idx + offset] */
            arr2d[i][j] = i * j + idx1;
        }
    }
    
    for (int i = 0; i < 50; i++) {
        darr[i] = i * 0.5 + dval1;
    }
    
    COMPILER_BARRIER();
    
    /* RELOAD_FOR_INPUT_ADDRESS: Complex pointer arithmetic */
    int ***triple_ptr = (int***)&ptr_arr;
    int **double_ptr = *triple_ptr + idx1;
    
    /* Multi-level indirection with volatile indices */
    int result1 = *(*(arr2d + idx1) + idx2 + idx3);
    
    /* Mixed type pointer arithmetic */
    char *cptr = carr + idx1 * sizeof(int) + idx2;
    int *iptr = (int*)(cptr + idx3);
    
    COMPILER_BARRIER();
    
    /* RELOAD_FOR_OUTPUT_ADDRESS: Store with complex address */
    *(larr + idx1 * 2 + idx2) = local5 + local6;
    
    /* Different addressing modes */
    long *lptr1 = &larr[idx1];
    long *lptr2 = lptr1 + idx2 * sizeof(long) / sizeof(*lptr1);
    
    /* Inline assembly to force specific reload types */
    
    /* RELOAD_FOR_OPERAND_ADDRESS: Address as input to asm */
    int asm_input;
    __asm__ volatile (
        "movl %[addr], %%eax\n\t"
        "movl (%%eax), %[out]\n\t"
        : [out] "=r" (asm_input)
        : [addr] "m" (&arr2d[idx1][idx2])
        : "eax", "memory"
    );
    
    COMPILER_BARRIER();
    
    /* RELOAD_FOR_OUTADDR_ADDRESS: Address computation for output */
    long asm_output_addr;
    __asm__ volatile (
        "leaq %[in], %%rax\n\t"
        "addq $16, %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        : [out] "=r" (asm_output_addr)
        : [in] "m" (larr[idx3])
        : "rax"
    );
    
    /* RELOAD_FOR_INPADDR_ADDRESS: Address of input address */
    int **addr_of_addr = &iptr;
    int value_from_double_indirect = **addr_of_addr;
    
    COMPILER_BARRIER();
    
    /* Mixed integer/float operations to pressure different register files */
    double float_sum = 0.0;
    for (int i = 0; i < 30; i++) {
        /* Convert int to double, do computation, convert back */
        double temp = (double)larr[i];
        temp = temp * darr[i % 50] + local7;
        float_sum += temp;
        
        /* Store back through complex address */
        *(darr + (idx1 + i) % 50) = temp;
    }
    
    /* More complex addressing with structure */
    struct MultiLevel ml;
    ml.ptr1 = arr2d[idx1];
    ml.ptr2 = &ml.ptr1 + idx2;
    ml.ptr3 = larr + idx3 * 2;
    ml.dptr = darr + idx4;
    
    /* Access through structure with volatile offsets */
    int struct_result = *(ml.ptr1 + idx1) + **(ml.ptr2 + idx2);
    
    COMPILER_BARRIER();
    
    /* RELOAD_FOR_OTHER_ADDRESS: Other complex address computations */
    long *complex_addr = (long*)((char*)larr + idx1 * sizeof(long) + idx2 * 4);
    *complex_addr = struct_result + asm_input;
    
    /* Final computation using all values */
    long final_result = 
        (long)result1 + 
        (long)asm_input + 
        (long)value_from_double_indirect +
        (long)struct_result +
        (long)float_sum +
        asm_output_addr;
    
    COMPILER_BARRIER();
    
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
    v9 = rand() % 1000;
    v10 = rand() % 1000;
    vd1 = (double)rand() / RAND_MAX * 100.0;
    vd2 = (double)rand() / RAND_MAX * 100.0;
    
    /* Call function with volatile arguments */
    long result = trigger_reloads(v1, v2, v3, v4, v9, v10, vd1, vd2);
    
    printf("Result: %ld\n", result);
    
    /* Use result to prevent optimization */
    volatile long sink = result;
    
    return 0;
}
