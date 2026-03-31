/* test_targhooks.c - Comprehensive test for triggering artificial symbol creation */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

/* Pattern 1: Sanitizer triggers (will only create symbols when compiled with sanitizers) */
#ifdef __SANITIZE_ADDRESS__
void asan_trigger_stack_overflow(void) {
    volatile int arr[10];
    /* Potential stack buffer overflow - ASan will instrument this */
    arr[15] = 42;  /* Out of bounds write */
}

void asan_trigger_use_after_return(void) {
    static volatile int* dangling_ptr;
    {
        int local = 42;
        dangling_ptr = &local;  /* Pointer to local that will go out of scope */
    }
    /* Use after return - ASan will detect if enabled */
    if (dangling_ptr) {
        volatile int x = *dangling_ptr;
        (void)x;
    }
}
#endif

/* Pattern 2: ThreadSanitizer data race */
static volatile int shared_counter = 0;

void* tsan_thread_func(void* arg) {
    for (int i = 0; i < 1000; ++i) {
        shared_counter++;  /* Data race - TSan will instrument */
    }
    return NULL;
}

void tsan_trigger_data_race(void) {
    pthread_t thread1, thread2;
    pthread_create(&thread1, NULL, tsan_thread_func, NULL);
    pthread_create(&thread2, NULL, tsan_thread_func, NULL);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    printf("Shared counter (racy): %d\n", shared_counter);
}

/* Pattern 3: OpenMP artificial symbols */
void omp_trigger_reduction(void) {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 1000; ++i) {
        sum += i;
    }
    printf("OpenMP reduction sum: %d\n", sum);
}

/* Pattern 4: Complex inline assembly */
void asm_trigger_clobbers(void) {
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
    printf("Assembly result: %d\n", y);
}

/* Pattern 5: C++ features (compiled as C++ when using g++) */
#ifdef __cplusplus
class Base {
public:
    virtual ~Base() {}
    virtual void foo() { printf("Base::foo\n"); }
};

class Derived : public Base {
public:
    virtual void foo() override { printf("Derived::foo\n"); }
};

void cpp_trigger_rtti(void) {
    Base* b = new Derived();
    
    /* Use RTTI - generates typeinfo symbols */
    if (Derived* d = dynamic_cast<Derived*>(b)) {
        d->foo();
    }
    
    /* Exception handling - generates personality routine */
    try {
        throw 42;
    } catch (int e) {
        printf("Caught exception: %d\n", e);
    }
    
    delete b;
}

/* Static local in inline function */
inline int static_local_counter() {
    static int counter = 0;  /* May generate guard variable */
    return ++counter;
}
#endif

/* Pattern 6: Profile-guided optimization counters */
void pgo_trigger_counters(void) {
    /* Loop-heavy code for PGO instrumentation */
    volatile int result = 0;
    for (int i = 0; i < 10000; ++i) {
        for (int j = 0; j < 1000; ++j) {
            result += i * j;
        }
    }
    printf("PGO loop result: %d\n", result);
}

/* Main orchestrator */
int main(int argc, char** argv) {
    int run_sanitizer_tests = 0;
    
    /* Check if we should run sanitizer-triggering code */
    if (argc > 1 && strcmp(argv[1], "--safe") != 0) {
        run_sanitizer_tests = 1;
    }
    
    printf("=== Starting artificial symbol trigger tests ===\n");
    
    /* Always run these (safe) */
    omp_trigger_reduction();
    asm_trigger_clobbers();
    pgo_trigger_counters();
    
    /* ThreadSanitizer test (safe to run, just data race) */
    tsan_trigger_data_race();
    
    #ifdef __cplusplus
    cpp_trigger_rtti();
    #endif
    
    /* Conditionally run sanitizer tests */
    if (run_sanitizer_tests) {
        #ifdef __SANITIZE_ADDRESS__
        printf("\nRunning ASan tests...\n");
        asan_trigger_stack_overflow();
        asan_trigger_use_after_return();
        #endif
    } else {
        printf("\nSkipping sanitizer tests (use --unsafe to run them)\n");
    }
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
