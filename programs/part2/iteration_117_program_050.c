/* main.c - Primary file with multiple techniques to trigger target hooks */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Declare external weak function from helper.c */
extern void weak_helper_function(void) __attribute__((weak));

/* Global variable for transactional memory */
int global_counter = 0;

/* Vector type using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function using AVX2 target attribute */
__attribute__((target("avx2")))
void avx2_vector_operations(void) {
    /* Complex vector operations that might need runtime helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    
    /* Complex expression with multiple operations */
    v4si result = (a * b) + (c / (a + 1));
    
    /* Use result to prevent optimization */
    volatile v4si volatile_result = result;
    (void)volatile_result;
}

/* Function using AVX512 check */
void check_and_use_avx512(void) {
    /* Initialize CPU features */
    __builtin_cpu_init();
    
    /* Check for AVX512 support - may generate helper functions */
    if (__builtin_cpu_supports("avx512f")) {
        printf("AVX512F is supported\n");
        
        /* Use vector extensions with complex operations */
        v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
        v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
        
        /* Trigonometric approximation using Taylor series
           This complex operation might need helper functions */
        v4sf angle = vec1 * 0.0174533f;  /* degrees to radians */
        v4sf sin_approx = angle - (angle*angle*angle)/6.0f 
                         + (angle*angle*angle*angle*angle)/120.0f;
        
        volatile v4sf volatile_sin = sin_approx;
        (void)volatile_sin;
    } else {
        printf("AVX512F not supported\n");
    }
}

/* Function with transactional memory */
void transactional_operation(void) {
    /* Transactional memory block - requires runtime support */
    __transaction_atomic {
        global_counter++;
        
        /* Nested memory access in transaction */
        int temp = global_counter;
        global_counter = temp * 2;
    }
}

/* OpenMP target region */
void attempt_offload(void) {
    int n = 100;
    int data[n];
    
    /* Initialize data */
    for (int i = 0; i < n; i++) {
        data[i] = i;
    }
    
    /* Attempt to offload - will likely generate fallback functions */
    #pragma omp target map(tofrom: data[0:n]) device(0)
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        data[i] = data[i] * 2 + 1;
    }
    
    /* Use result to prevent optimization */
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += data[i];
    }
    (void)sum;
}

/* Large stack usage for stack protector */
void large_stack_function(void) {
    char large_buffer[4096];  /* Large stack allocation */
    int another_buffer[512];
    
    /* Fill buffers to prevent optimization */
    memset(large_buffer, 0xAA, sizeof(large_buffer));
    for (int i = 0; i < 512; i++) {
        another_buffer[i] = i * 3;
    }
    
    /* Complex operations on buffers */
    for (int i = 0; i < 512 && i < 4096; i++) {
        large_buffer[i] = (char)(another_buffer[i] & 0xFF);
    }
    
    volatile char vol = large_buffer[100];
    (void)vol;
}

int main(void) {
    printf("Starting target hook triggering program\n");
    
    /* 1. Check and use CPU features */
    check_and_use_avx512();
    
    /* 2. Use AVX2 vector operations */
    avx2_vector_operations();
    
    /* 3. Call weak helper function (if linked) */
    if (weak_helper_function) {
        weak_helper_function();
    } else {
        printf("Weak helper not available\n");
    }
    
    /* 4. Perform transactional memory operation */
    transactional_operation();
    printf("Global counter after transaction: %d\n", global_counter);
    
    /* 5. Attempt OpenMP offload */
    attempt_offload();
    
    /* 6. Use large stack */
    large_stack_function();
    
    /* 7. Additional ARM-specific check if compiled for ARM */
    #ifdef __arm__
    {
        /* ARM system register access - may need helper */
        unsigned int reg_value;
        __asm__ volatile ("mrc p15, 0, %0, c0, c0, 0" : "=r"(reg_value));
        printf("ARM MIDR: 0x%08x\n", reg_value);
    }
    #elif defined(__aarch64__)
    {
        /* AArch64 system register */
        unsigned long fpcr = __builtin_aarch64_get_fpcr();
        printf("AArch64 FPCR: 0x%016lx\n", fpcr);
    }
    #elif defined(__powerpc__) || defined(__PPC__)
    {
        /* PowerPC special register */
        double d = 3.14159;
        long long ll;
        memcpy(&ll, &d, sizeof(double));
        __builtin_ppc_mtfsf(0xFF, ll);
    }
    #endif
    
    /* 8. Complex expression with multiple builtins */
    {
        int x = 23;
        int y = 42;
        /* Use __builtin_expect to influence branch prediction */
        if (__builtin_expect(x < y, 1)) {
            /* Use __builtin_popcount */
            int popcnt = __builtin_popcount(x ^ y);
            printf("Population count of XOR: %d\n", popcnt);
        }
    }
    
    printf("Program completed\n");
    return 0;
}
