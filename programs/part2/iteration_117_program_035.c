/* main.c - Primary file with complex patterns to trigger target hooks */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Declare external weak function from helper.c */
extern void __attribute__((weak)) target_helper_init(void);
extern volatile int global_tm_var;

/* Transactional Memory global */
int __attribute__((transaction_safe)) tm_global = 0;

/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* AVX2 target-specific function */
__attribute__((target("avx2")))
static v4si avx2_vector_divide(v4si a, v4si b) {
    /* Complex vector division - may require helper functions */
    v4si mask = b != 0;
    v4si result = a / (b + (v4si){1,1,1,1}); /* Avoid division by zero */
    
    /* Mix with CPU feature check */
    if (__builtin_cpu_supports("avx512f")) {
        /* This built-in may require runtime initialization */
        __builtin_cpu_init();
    }
    
    return result & mask;
}

/* Function using PowerPC-style built-in (if compiled for PPC) */
static unsigned long read_special_reg(void) {
    unsigned long val = 0;
    
    /* These built-ins may require hidden helper functions */
#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    val = __builtin_ppc_mftb();
#elif defined(__arm__)
    asm volatile("mrc p15, 0, %0, c0, c0, 0" : "=r"(val));
#elif defined(__aarch64__)
    val = __builtin_aarch64_get_fpcr();
#endif
    
    return val;
}

/* OpenMP target region - may generate fallback helpers */
static void attempt_offload(void) {
    int n = 100;
    int host_array[100];
    
    for (int i = 0; i < n; i++) {
        host_array[i] = i;
    }
    
    /* Attempt offload to potentially unsupported device */
    #pragma omp target device(0) map(tofrom: host_array[0:n])
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        host_array[i] *= 2;
    }
    
    /* Use volatile to prevent optimization */
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += host_array[i];
    }
}

/* Transactional memory section */
static void transactional_operation(void) {
    /* This requires TM runtime helpers */
    __transaction_atomic {
        tm_global++;
        global_tm_var = tm_global;
        
        /* Nested complexity */
        if (tm_global % 2 == 0) {
            v4si vec1 = {1, 2, 3, 4};
            v4si vec2 = {5, 6, 7, 8};
            v4si result = avx2_vector_divide(vec1, vec2);
            
            /* Use result to prevent dead code elimination */
            volatile int check = result[0] + result[1];
        }
    }
}

/* Large stack usage to trigger stack protection helpers */
static void large_stack_function(void) {
    char large_buffer[4096];  /* Large stack frame */
    int another_buffer[512];
    
    /* Initialize with pattern */
    for (int i = 0; i < sizeof(large_buffer); i++) {
        large_buffer[i] = i % 256;
    }
    
    /* Use built-ins in complex way */
    unsigned long reg_val = read_special_reg();
    
    /* Mix with vector operations */
    v4sf float_vec = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf float_result = float_vec * float_vec + float_vec;
    
    /* Prevent optimization */
    volatile float fcheck = float_result[0];
    volatile unsigned long rcheck = reg_val;
    
    /* Call external weak function */
    if (target_helper_init) {
        target_helper_init();
    }
}

int main(void) {
    printf("Starting target hook triggering program...\n");
    
    /* Initialize CPU features - may generate built-in helpers */
    __builtin_cpu_init();
    
    /* Path 1: CPU feature-dependent vector operations */
    if (__builtin_cpu_supports("avx2")) {
        printf("AVX2 supported, performing vector operations\n");
        v4si a = {100, 200, 300, 400};
        v4si b = {5, 6, 7, 8};
        
        for (int i = 0; i < 10; i++) {
            v4si result = avx2_vector_divide(a, b);
            volatile int check = result[0] + result[3];
            a[0] += check;
        }
    }
    
    /* Path 2: Attempt OpenMP offload */
    printf("Attempting OpenMP offload...\n");
    attempt_offload();
    
    /* Path 3: Transactional memory operations */
    printf("Performing transactional memory operations...\n");
    for (int i = 0; i < 5; i++) {
        transactional_operation();
    }
    
    /* Path 4: Large stack usage with built-ins */
    printf("Using large stack frames with built-ins...\n");
    large_stack_function();
    
    /* Path 5: Architecture-specific built-ins */
    printf("Reading special registers...\n");
    unsigned long val = read_special_reg();
    printf("Special register value: 0x%lx\n", val);
    
    /* Final check to use all variables */
    volatile int final_check = tm_global + (int)val;
    
    printf("Program completed. TM global: %d\n", tm_global);
    return 0;
}
