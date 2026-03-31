/* Combined test program to trigger compiler-generated artificial symbols */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#ifdef __cplusplus
#include <typeinfo>
#include <exception>

/* Pattern 4A: Static local variable in inline function (C++ only) */
inline int get_static_counter() {
    static int counter = 0;  // May generate guard variable
    return ++counter;
}

/* Pattern 4B: Exception handling (C++ only) */
class Base {
public:
    virtual ~Base() {}
    virtual void foo() { printf("Base::foo\n"); }
};

class Derived : public Base {
public:
    virtual void foo() override { printf("Derived::foo\n"); }
};

void test_exceptions() {
    try {
        Base* b = new Derived();
        // Pattern 4C: RTTI usage
        Derived* d = dynamic_cast<Derived*>(b);
        if (d) {
            const std::type_info& ti = typeid(*d);
            printf("Type: %s\n", ti.name());
        }
        b->foo();
        delete b;
        throw std::runtime_error("test exception");
    } catch (const std::exception& e) {
        printf("Caught: %s\n", e.what());
    }
}
#endif

/* Pattern 1A: AddressSanitizer triggers */
void asan_stack_overflow() {
    int arr[10];
    /* Deliberate overflow - ASan will detect this */
    arr[15] = 42;  // Out-of-bounds write
}

void asan_use_after_return() {
    int* ptr;
    {
        int local = 42;
        ptr = &local;  // Taking address of local
    }
    // Use after scope - ASan will detect if enabled
    // printf("%d\n", *ptr);  // Commented to avoid crash without ASan
}

/* Pattern 1B: ThreadSanitizer triggers */
int shared_var = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* thread_func(void* arg) {
    for (int i = 0; i < 1000; i++) {
        // Data race - TSan will detect this
        shared_var++;
    }
    return NULL;
}

void tsan_data_race() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread_func, NULL);
    pthread_create(&t2, NULL, thread_func, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Shared var (racy): %d\n", shared_var);
}

void tsan_data_race_fixed() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread_func, NULL);
    pthread_create(&t2, NULL, thread_func, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    // Reset with mutex for clean output
    pthread_mutex_lock(&mutex);
    shared_var = 0;
    pthread_mutex_unlock(&mutex);
}

/* Pattern 2: OpenMP parallelization */
#ifdef _OPENMP
#include <omp.h>
void openmp_reduction() {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 1000; ++i) {
        sum += i;
    }
    printf("OpenMP reduction sum: %d\n", sum);
    
    // Thread-private variables
    #pragma omp parallel
    {
        int private_var = omp_get_thread_num();
        #pragma omp critical
        {
            printf("Thread %d private var: %d\n", 
                   omp_get_thread_num(), private_var);
        }
    }
}
#endif

/* Pattern 3: Complex inline assembly */
void inline_assembly_test() {
    int x = 42, y = 100, result;
    
    // Extended asm with multiple clobbers
    asm volatile (
        "mov %1, %%eax\n\t"
        "add %2, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "=r" (result)
        : "r" (x), "r" (y)
        : "%eax", "%ebx", "memory", "cc"
    );
    
    printf("Assembly result: %d\n", result);
    
    // Memory barrier
    asm volatile ("" ::: "memory");
}

/* Pattern 1C: Profile-guided optimization triggers */
void pgo_hot_loop() {
    volatile int counter = 0;
    for (int i = 0; i < 10000; i++) {
        for (int j = 0; j < 10000; j++) {
            counter += i * j;
        }
    }
    printf("PGO loop completed: %d\n", counter);
}

/* Pattern: Builtin usage */
void builtin_test() {
    // Use GCC builtins that may generate internal symbols
    int x = __builtin_popcount(0xFF);
    printf("Popcount: %d\n", x);
    
    // Memory builtins
    char src[100], dst[100];
    __builtin_memcpy(dst, src, sizeof(src));
    __builtin_memset(src, 0, sizeof(src));
}

/* Main orchestrator */
int main(int argc, char** argv) {
    int run_sanitizer_tests = 0;
    
    // Check if we should run sanitizer-triggering code
    if (argc > 1) {
        run_sanitizer_tests = atoi(argv[1]);
    }
    
    printf("=== Starting compiler internal symbol test ===\n");
    
    // Always run safe tests
    printf("\n1. Running inline assembly test...\n");
    inline_assembly_test();
    
    printf("\n2. Running builtin test...\n");
    builtin_test();
    
#ifdef _OPENMP
    printf("\n3. Running OpenMP tests...\n");
    openmp_reduction();
#endif
    
#ifdef __cplusplus
    printf("\n4. Running C++ specific tests...\n");
    printf("Static counter: %d\n", get_static_counter());
    test_exceptions();
#endif
    
    printf("\n5. Running PGO hot loop...\n");
    pgo_hot_loop();
    
    printf("\n6. Running ThreadSanitizer pattern (fixed version)...\n");
    tsan_data_race_fixed();
    
    // Only run actual sanitizer triggers if explicitly requested
    if (run_sanitizer_tests) {
        printf("\n7. Running sanitizer-triggering patterns...\n");
        printf("WARNING: These may crash without sanitizers enabled!\n");
        
        // These will be detected by sanitizers if enabled
        asan_stack_overflow();
        tsan_data_race();
    } else {
        printf("\n7. Skipping sanitizer-triggering patterns\n");
        printf("   (run with './program 1' to enable them)\n");
    }
    
    printf("\n=== Test completed ===\n");
    return 0;
}
