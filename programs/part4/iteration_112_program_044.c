#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Helper function for non-constant initializer */
int some_function(void) {
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
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Access and modify all TLS variables */
    tls_var1 = 1000 + thread_id;
    tls_var2 = 2000 + thread_id;
    tls_var3 = 3000 + thread_id;
    tls_var4 = 4000 + thread_id;
    tls_var5 = 5000 + thread_id;
    tls_init = 6000 + thread_id;
    
    /* Take addresses to ensure ODR-use */
    volatile int* addr1 = &tls_var1;
    volatile int* addr2 = &tls_var2;
    volatile int* addr3 = &tls_var3;
    volatile int* addr4 = &tls_var4;
    volatile int* addr5 = &tls_var5;
    volatile int* addr_init = &tls_init;
    
    /* Conditional access to prevent optimization */
    if (thread_id % 2 == 0) {
        *addr1 += 1;
        *addr3 += 1;
        *addr5 += 1;
    } else {
        *addr2 += 1;
        *addr4 += 1;
        *addr_init += 1;
    }
    
    /* Verify values are thread-local */
    printf("Thread %d: tls_var1=%d, tls_var2=%d, tls_var3=%d, "
           "tls_var4=%d, tls_var5=%d, tls_init=%d\n",
           thread_id, tls_var1, tls_var2, tls_var3, 
           tls_var4, tls_var5, tls_init);
    
    return NULL;
}

int main(void) {
    pthread_t threads[3];
    int thread_ids[3];
    
    /* Initialize non-constant TLS variable */
    tls_init = some_function();
    
    /* Initialize main thread's TLS values */
    tls_var1 = 1;
    tls_var2 = 2;
    tls_var3 = 3;
    tls_var4 = 4;
    tls_var5 = 5;
    
    /* Create multiple threads to force TLS emulation */
    for (int i = 0; i < 3; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Let threads run */
    sleep(1);
    
    /* Access TLS in main thread (different instance) */
    printf("Main thread before join: tls_var1=%d, tls_var2=%d, tls_var3=%d, "
           "tls_var4=%d, tls_var5=%d, tls_init=%d\n",
           tls_var1, tls_var2, tls_var3, tls_var4, tls_var5, tls_init);
    
    /* Join all threads */
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }
    
    /* Final access in main thread - should still have original values */
    printf("Main thread after join: tls_var1=%d, tls_var2=%d, tls_var3=%d, "
           "tls_var4=%d, tls_var5=%d, tls_init=%d\n",
           tls_var1, tls_var2, tls_var3, tls_var4, tls_var5, tls_init);
    
    /* Calculate checksum to prove all TLS machinery was used */
    int checksum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5 + tls_init;
    printf("TLS checksum in main thread: %d\n", checksum);
    
    /* Take addresses again to ensure ODR-use in main */
    volatile int* addrs[] = {&tls_var1, &tls_var2, &tls_var3, &tls_var4, &tls_var5, &tls_init};
    for (int i = 0; i < 6; i++) {
        *addrs[i] += i;  /* Modify to prevent dead store elimination */
    }
    
    return 0;
}
