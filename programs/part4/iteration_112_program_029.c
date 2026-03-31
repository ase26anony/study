/* test_tls_emulation.c - Program to trigger TLS declaration cloning logic */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

/* Helper function for non-constant initializer */
static int get_initial_value(void) {
    return 42;
}

/* 1. Extern TLS with hidden visibility - will have DECL_VISIBILITY_SPECIFIED set */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS - will have DECL_WEAK set */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS - may become DECL_COMMON */
__thread int tls_var3 __attribute__((used));

/* 4. TLS with DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_var4;
#else
/* Simulate with visibility attribute */
__thread int tls_var4 __attribute__((visibility("default"), used));
#endif

/* 5. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* 6. Static TLS - will have different TREE_PUBLIC/DECL_EXTERNAL attributes */
static __thread int tls_static __attribute__((used));

/* Global array to collect results from threads */
static int thread_results[4][6] = {0};
static volatile int result_index = 0;

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    int local_idx = __sync_fetch_and_add(&result_index, 1);
    
    /* Force ODR-use by taking addresses */
    int* p1 = &tls_var1;
    int* p2 = &tls_var2;
    int* p3 = &tls_var3;
    int* p4 = &tls_var4;
    int* p5 = &tls_init;
    int* p6 = &tls_static;
    
    /* Prevent optimization by using volatile */
    volatile int* vp1 = p1;
    volatile int* vp2 = p2;
    volatile int* vp3 = p3;
    volatile int* vp4 = p4;
    volatile int* vp5 = p5;
    volatile int* vp6 = p6;
    
    /* Write thread-specific values to TLS variables */
    tls_var1 = 1000 + thread_id * 10 + 1;
    tls_var2 = 1000 + thread_id * 10 + 2;
    tls_var3 = 1000 + thread_id * 10 + 3;
    tls_var4 = 1000 + thread_id * 10 + 4;
    tls_init = 1000 + thread_id * 10 + 5;
    tls_static = 1000 + thread_id * 10 + 6;
    
    /* Read back and store results */
    thread_results[local_idx][0] = tls_var1;
    thread_results[local_idx][1] = tls_var2;
    thread_results[local_idx][2] = tls_var3;
    thread_results[local_idx][3] = tls_var4;
    thread_results[local_idx][4] = tls_init;
    thread_results[local_idx][5] = tls_static;
    
    /* Conditional access to prevent dead code elimination */
    if (thread_id % 2 == 0) {
        /* Use the pointers to force address-taking */
        *p1 += 1;
        *p3 += 1;
        *p5 += 1;
    } else {
        *p2 += 1;
        *p4 += 1;
        *p6 += 1;
    }
    
    return NULL;
}

/* Function that returns address of TLS variable - forces TLS machinery */
int* get_tls_address(int which) {
    switch (which) {
        case 0: return &tls_var1;
        case 1: return &tls_var2;
        case 2: return &tls_var3;
        case 3: return &tls_var4;
        case 4: return &tls_init;
        case 5: return &tls_static;
        default: return NULL;
    }
}

/* Define the extern TLS variable */
__thread int tls_var1 = 1;

int main(void) {
    pthread_t threads[4];
    int thread_ids[4];
    int i, j;
    
    /* Initialize TLS with non-constant value */
    tls_init = get_initial_value();
    
    /* Access TLS variables in main thread first */
    tls_var1 = 999;
    tls_var2 = 998;
    tls_var3 = 997;
    tls_var4 = 996;
    tls_static = 995;
    
    /* Force address-taking in main thread too */
    int* addrs[6];
    for (i = 0; i < 6; i++) {
        addrs[i] = get_tls_address(i);
    }
    
    /* Create multiple threads to force TLS emulation */
    for (i = 0; i < 4; i++) {
        thread_ids[i] = i + 1;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Wait for all threads */
    for (i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    
    /* Verify thread-local behavior */
    printf("TLS Test Results:\n");
    printf("Main thread TLS values: %d, %d, %d, %d, %d, %d\n",
           tls_var1, tls_var2, tls_var3, tls_var4, tls_init, tls_static);
    
    printf("\nThread results:\n");
    for (i = 0; i < 4; i++) {
        printf("Thread %d: ", i + 1);
        for (j = 0; j < 6; j++) {
            printf("%d ", thread_results[i][j]);
        }
        printf("\n");
    }
    
    /* Calculate checksum to prove all TLS accesses happened */
    int checksum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init + tls_static;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 6; j++) {
            checksum += thread_results[i][j];
        }
    }
    
    printf("\nChecksum: %d (expected non-zero)\n", checksum);
    
    /* Final verification - main thread values should be unchanged */
    if (tls_var1 == 999 && tls_static == 995) {
        printf("TLS isolation verified: main thread values preserved\n");
    }
    
    return 0;
}
