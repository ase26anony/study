/* Combined test program to trigger artificial symbol generation in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/* Pattern 1: AddressSanitizer triggers */
#ifdef __SANITIZE_ADDRESS__
void asan_stack_overflow() {
    int arr[10];
    /* Deliberate overflow when ASan is enabled */
    arr[15] = 42;
}

void asan_use_after_return() {
    static int* dangling_ptr;
    {
        int local = 42;
        dangling_ptr = &local;
    }
    /* Use after scope - only dangerous with ASan */
    if (dangling_ptr) {
        *dangling_ptr = 43;
    }
}
#endif

/* Pattern 2: ThreadSanitizer triggers */
volatile int shared_counter = 0;

void* thread_func(void* arg) {
    for (int i = 0; i < 1000; i++) {
        shared_counter++;  /* Data race when unsynchronized */
    }
    return NULL;
}

void trigger_data_race() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread_func, NULL);
    pthread_create(&t2, NULL, thread_func, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Shared counter (racy): %d\n", shared_counter);
}

/* Pattern 3: OpenMP artificial symbols */
#ifdef _OPENMP
#include <omp.h>
void openmp_reduction() {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 1000; ++i) {
        sum += i;
    }
    printf("OpenMP reduction sum: %d\n", sum);
}

void openmp_private_vars() {
    int x = 0;
    #pragma omp parallel private(x)
    {
        x = omp_get_thread_num();
        #pragma omp critical
        printf("Thread %d: x = %d\n", omp_get_thread_num(), x);
    }
}
#endif

/* Pattern 4: Complex inline assembly */
void inline_asm_test() {
    int x = 42, y = 0;
    
    /* Complex asm with multiple clobbers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (y)
        : "r" (x)
        : "%eax", "%ebx", "memory", "cc"
    );
    
    printf("Inline asm result: %d -> %d\n", x, y);
}

/* Pattern 5: C++ features (compiled as C++ if needed) */
#ifdef __cplusplus
#include <typeinfo>

class Base {
public:
    virtual ~Base() {}
    virtual void foo() { printf("Base::foo\n"); }
};

class Derived : public Base {
public:
    void foo() override { printf("Derived::foo\n"); }
};

void cpp_rtti_test() {
    Base* b = new Derived();
    
    /* Triggers typeinfo generation */
    if (Derived* d = dynamic_cast<Derived*>(b)) {
        d->foo();
    }
    
    /* Exception handling personality routine */
    try {
        throw 42;
    } catch (int e) {
        printf("Caught exception: %d\n", e);
    }
    
    delete b;
}

/* Static local with potential guard variable */
inline int static_local_func() {
    static int counter = 0;
    return ++counter;
}
#endif

/* Pattern 6: Profile-guided optimization triggers */
void pgo_hot_loop() {
    volatile int result = 0;
    for (int i = 0; i < 10000; i++) {
        result += i * i;
    }
    printf("PGO loop result: %d\n", result);
}

/* Main orchestrator */
int main(int argc, char** argv) {
    int run_sanitizer_tests = 0;
    
    /* Check if we should run sanitizer-specific tests */
    if (argc > 1) {
        run_sanitizer_tests = 1;
    }
    
    printf("=== Starting artificial symbol generation tests ===\n");
    
    /* Always run safe tests */
    printf("\n1. Running OpenMP tests...\n");
    #ifdef _OPENMP
    openmp_reduction();
    openmp_private_vars();
    #else
    printf("OpenMP not supported\n");
    #endif
    
    printf("\n2. Running inline assembly test...\n");
    inline_asm_test();
    
    printf("\n3. Running PGO hot loop...\n");
    pgo_hot_loop();
    
    printf("\n4. Running thread race test...\n");
    trigger_data_race();
    
    /* C++ specific tests */
    #ifdef __cplusplus
    printf("\n5. Running C++ RTTI and exception tests...\n");
    cpp_rtti_test();
    printf("Static local calls: %d\n", static_local_func());
    #endif
    
    /* Sanitizer tests - only run if explicitly requested */
    if (run_sanitizer_tests) {
        printf("\n6. Running sanitizer-specific tests...\n");
        #ifdef __SANITIZE_ADDRESS__
        asan_stack_overflow();
        asan_use_after_return();
        #else
        printf("Sanitizers not enabled at compile time\n");
        #endif
    } else {
        printf("\n6. Sanitizer tests skipped (run with any argument to enable)\n");
    }
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
