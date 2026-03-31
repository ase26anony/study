// test_targhooks.c
// Compile with: gcc -O2 -fsanitize=address -fprofile-generate -fopenmp -pthread test_targhooks.c -o test_program

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Pattern 1: AddressSanitizer triggers
void asan_stack_overflow() {
    int arr[10];
    // Deliberate overflow when ASan is enabled
    arr[15] = 42;  // Will be detected by ASan if enabled
}

int* use_after_return_helper() {
    int local = 42;
    return &local;  // Use-after-return warning
}

void asan_use_after_return() {
    int* ptr = use_after_return_helper();
    // Access would be caught by ASan
    (void)ptr;
}

// Pattern 2: ThreadSanitizer data race
int shared_var = 0;

void* thread_func(void* arg) {
    for (int i = 0; i < 1000; i++) {
        shared_var++;  // Data race when compiled with TSan
    }
    return NULL;
}

void trigger_data_race() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread_func, NULL);
    pthread_create(&t2, NULL, thread_func, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
}

// Pattern 3: OpenMP artificial symbols
void openmp_reduction() {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 1000; ++i) {
        sum += i;
    }
    printf("OpenMP reduction sum: %d\n", sum);
}

// Pattern 4: Complex inline assembly
void inline_assembly_test() {
    int x = 42, y = 0;
    // Complex asm with multiple clobbers
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (y)
        : "r" (x)
        : "%eax", "%ebx", "memory", "cc"
    );
    printf("Inline assembly result: %d\n", y);
}

// Pattern 5: Static local variables (C++ style guard variables)
#ifdef __cplusplus
// C++ version for static initialization guards
struct ComplexType {
    ComplexType() { printf("ComplexType constructed\n"); }
    ~ComplexType() { printf("ComplexType destroyed\n"); }
    int data[100];
};

void static_local_test() {
    static ComplexType obj;  // May generate guard variable
    obj.data[0] = 42;
}
#else
// C version - still can generate internal symbols
void static_local_test_c() {
    static int counter = 0;  // Static initialization
    static int large_array[1000];  // Large static array
    counter++;
    large_array[counter % 1000] = counter;
}
#endif

// Pattern 6: Profile-guided optimization triggers
void pgo_hot_function() {
    volatile int result = 0;
    // Hot loop for PGO instrumentation
    for (int i = 0; i < 10000; i++) {
        result += i * i;
        if (i % 1000 == 0) {
            result /= 2;
        }
    }
    printf("PGO function result: %d\n", result);
}

// Pattern 7: Exception handling (C++ only)
#ifdef __cplusplus
#include <exception>
void exception_test() {
    try {
        throw std::runtime_error("Test exception");
    } catch (const std::exception& e) {
        printf("Caught exception: %s\n", e.what());
    }
}
#endif

// Pattern 8: Vectorization and optimization
void vectorization_test() {
    int arr[1024];
    int sum = 0;
    
    // Loop that may be vectorized
    for (int i = 0; i < 1024; i++) {
        arr[i] = i * 2;
    }
    
    // Another loop that may be optimized
    for (int i = 0; i < 1024; i++) {
        sum += arr[i];
    }
    
    printf("Vectorization test sum: %d\n", sum);
}

int main(int argc, char** argv) {
    printf("Starting test program for targhooks coverage\n");
    
    // Only trigger sanitizer errors if explicitly requested
    int trigger_errors = 0;
    if (argc > 1 && argv[1][0] == '1') {
        trigger_errors = 1;
    }
    
    // Always run safe patterns
    openmp_reduction();
    inline_assembly_test();
    vectorization_test();
    pgo_hot_function();
    
    #ifdef __cplusplus
    static_local_test();
    exception_test();
    #else
    static_local_test_c();
    #endif
    
    // Conditionally trigger sanitizer patterns
    if (trigger_errors) {
        printf("\nTriggering sanitizer patterns...\n");
        asan_stack_overflow();
        trigger_data_race();
    } else {
        printf("\nRunning in safe mode (no sanitizer triggers)\n");
    }
    
    printf("\nTest program completed successfully\n");
    return 0;
}
