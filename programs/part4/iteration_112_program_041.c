#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Helper function for non-constant initializer */
static int get_initial_value(void) {
    return 42;
}

/* 1. Extern TLS with hidden visibility */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS variable */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS (implicitly extern) */
__thread int tls_var3 __attribute__((used));

/* 4. Static TLS with DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_var4;
#else
/* Simulate similar attribute on non-Windows */
__thread int tls_var4 __attribute__((visibility("default"), used));
#endif

/* 5. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* 6. Another TLS with explicit visibility */
__thread int tls_var5 __attribute__((visibility("protected"), used));

/* Define the extern TLS variable */
__thread int tls_var1 = 100;

/* Thread function that accesses all TLS variables */
static void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Force ODR-use: take addresses of TLS variables */
    volatile int* addrs[] = {
        &tls_var1,
        &tls_var2,
        &tls_var3,
        &tls_var4,
        &tls_var5,
        &tls_init
    };
    
    /* Prevent optimization from removing TLS accesses */
    if (thread_id % 2 == 0) {
        /* Write thread-specific values */
        tls_var1 = 1000 + thread_id;
        tls_var2 = 2000 + thread_id;
        tls_var3 = 3000 + thread_id;
        tls_var4 = 4000 + thread_id;
        tls_var5 = 5000 + thread_id;
        tls_init = 6000 + thread_id;
    } else {
        /* Alternative write pattern */
        tls_var1 = 100 + thread_id;
        tls_var2 = 200 + thread_id;
        tls_var3 = 300 + thread_id;
        tls_var4 = 400 + thread_id;
        tls_var5 = 500 + thread_id;
        tls_init = 600 + thread_id;
    }
    
    /* Read back and verify (creates data dependency) */
    int sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5 + tls_init;
    
    /* Use the addresses to prevent optimization */
    for (int i = 0; i < 6; i++) {
        sum += (addrs[i] != NULL) ? 1 : 0;
    }
    
    int* result = malloc(sizeof(int));
    *result = sum;
    return result;
}

/* Function that returns address of TLS variable (forces emulation) */
static volatile int* get_tls_addr(int idx) {
    switch (idx) {
        case 0: return &tls_var1;
        case 1: return &tls_var2;
        case 2: return &tls_var3;
        case 3: return &tls_var4;
        case 4: return &tls_var5;
        case 5: return &tls_init;
        default: return NULL;
    }
}

int main(void) {
    /* Initialize TLS with non-constant value */
    tls_init = get_initial_value();
    
    pthread_t threads[3];
    int thread_ids[3];
    void* thread_results[3];
    
    /* Create multiple threads to force TLS emulation */
    for (int i = 0; i < 3; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Main thread also accesses TLS variables */
    tls_var1 = 999;
    tls_var2 = 888;
    tls_var3 = 777;
    tls_var4 = 666;
    tls_var5 = 555;
    
    /* Force address-taking in main thread too */
    volatile int* main_addrs[6];
    for (int i = 0; i < 6; i++) {
        main_addrs[i] = get_tls_addr(i);
    }
    
    /* Join all threads */
    int total_sum = 0;
    for (int i = 0; i < 3; i++) {
        if (pthread_join(threads[i], &thread_results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
        total_sum += *(int*)thread_results[i];
        free(thread_results[i]);
    }
    
    /* Add main thread's TLS values */
    total_sum += tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5 + tls_init;
    
    /* Use the addresses to prevent dead code elimination */
    for (int i = 0; i < 6; i++) {
        if (main_addrs[i]) {
            total_sum += 1;
        }
    }
    
    printf("TLS test completed. Checksum: %d\n", total_sum);
    printf("If checksum is non-zero, TLS machinery was invoked.\n");
    
    return 0;
}
