/* main.c - Primary file with multiple coverage techniques */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Declare external weak function from helper.c */
extern void weak_constructor_func(void) __attribute__((weak));

/* Global variable for transactional memory */
volatile int global_counter = 0;

/* Vector type using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function using AVX2 target attribute */
__attribute__((target("avx2")))
v4si vector_division_avx2(v4si a, v4si b) {
    /* Complex vector operation that may need helper */
    v4si result;
    
    /* Force potential helper generation with division */
    for (int i = 0; i < 4; i++) {
        /* Division by variable may trigger helper call */
        result[i] = a[i] / (b[i] + 1);
    }
    
    /* Mix with CPU feature check */
    if (__builtin_cpu_supports("avx512f")) {
        __builtin_cpu_init();
        /* Additional complex operation */
        result = result + (a & b);
    }
    
    return result;
}

/* Function with ARM-specific built-in (will be compiled on ARM) */
#ifdef __arm__
__attribute__((target("arch=armv8-a+crc")))
unsigned int arm_coprocessor_access(void) {
    /* Access coprocessor - may need helper */
    unsigned int value;
    value = __builtin_arm_mrc(15, 0, 0, 0, 0);
    return value;
}
#endif

/* Function with PowerPC specific built-in */
#ifdef __powerpc__
void ppc_special_register(void) {
    /* Manipulate FPSCR - may need helper */
    __builtin_ppc_mtfsf(0xFF, 0x12345678);
}
#endif

/* Transactional memory function */
void transactional_increment(void) {
    /* Use GCC transactional memory */
    __transaction_atomic {
        global_counter++;
        
        /* Complex operation inside transaction */
        v4si vec1 = {1, 2, 3, 4};
        v4si vec2 = {5, 6, 7, 8};
        v4si vec3 = vector_division_avx2(vec1, vec2);
        
        /* Use result to prevent optimization */
        asm volatile("" : "+r"(vec3));
    }
}

/* OpenMP target region */
void attempt_offload(void) {
    int n = 1000;
    int *array = (int*)malloc(n * sizeof(int));
    
    if (!array) return;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        array[i] = i;
    }
    
    /* Attempt offload - may generate fallback helpers */
    #pragma omp target map(tofrom: array[0:n]) device(0)
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            array[i] *= 2;
        }
    }
    
    /* Use result */
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += array[i];
    }
    
    free(array);
}

/* Large stack usage for stack protector */
void large_stack_function(void) {
    char large_buffer[4096];  /* Large stack for protector */
    volatile int *ptr = (volatile int*)large_buffer;
    
    /* Initialize with pattern */
    for (int i = 0; i < 1024; i++) {
        ptr[i] = i * 3;
    }
    
    /* Call weak function if available */
    if (weak_constructor_func) {
        weak_constructor_func();
    }
    
    /* Use buffer to prevent optimization */
    asm volatile("" : "+m"(large_buffer));
}

int main(void) {
    printf("Starting target hook coverage test...\n");
    
    /* 1. CPU feature detection and vector operations */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        v4si a = {10, 20, 30, 40};
        v4si b = {2, 3, 4, 5};
        v4si result = vector_division_avx2(a, b);
        
        printf("Vector result: %d %d %d %d\n", 
               result[0], result[1], result[2], result[3]);
    }
    
    /* 2. Architecture-specific built-ins */
    #ifdef __arm__
    printf("ARM coprocessor: %u\n", arm_coprocessor_access());
    #endif
    
    #ifdef __powerpc__
    ppc_special_register();
    printf("PowerPC FPSCR modified\n");
    #endif
    
    /* 3. Transactional memory */
    for (int i = 0; i < 10; i++) {
        transactional_increment();
    }
    printf("Global counter after transactions: %d\n", global_counter);
    
    /* 4. OpenMP offload attempt */
    attempt_offload();
    printf("OpenMP offload attempted\n");
    
    /* 5. Large stack with protector */
    large_stack_function();
    printf("Large stack function completed\n");
    
    /* 6. Complex expression with multiple built-ins */
    volatile int complex_check = 0;
    if (__builtin_cpu_supports("sse4.2") && 
        __builtin_cpu_supports("popcnt")) {
        complex_check = __builtin_popcount(0xFFFFFFFF);
    }
    printf("Complex check result: %d\n", complex_check);
    
    return 0;
}
