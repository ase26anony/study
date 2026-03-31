/* test_targhooks.c - Designed to trigger GCC's internal tree initialization
   for compiler-generated declarations with specific flags:
   TREE_STATIC=1, TREE_PUBLIC=1, DECL_EXTERNAL=1, TREE_USED=1,
   TREE_THIS_VOLATILE=1, TREE_NOTHROW=1, DECL_ARTIFICIAL=1,
   DECL_IGNORED_P=1, DECL_VISIBILITY_SPECIFIED=1, DECL_VISIBILITY=VISIBILITY_HIDDEN
   
   Compile with: gcc -O3 -ftls-model=initial-exec -fPIC -fnon-call-exceptions \
                   -fvisibility=hidden -fdump-tree-all -o test_targhooks test_targhooks.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

/* External function to force external linkage context */
extern int external_helper(void) __attribute__((weak));
int external_helper(void) { return rand() % 100; }

/* Volatile variable to prevent optimization */
volatile int volatile_trigger = 0;

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;
static int init_tls(void) __attribute__((noinline));
static int init_tls(void) {
    return external_helper() + 1;
}

/* Function with nothrow attribute to provide context for TREE_NOTHROW */
static void process_data(void) __attribute__((nothrow));

/* Complex static initializer with side effects */
static int complex_init = (printf("Initializing complex static...\n"), 42);

/* Nested function using setjmp within a statement expression */
#define NESTED_SETJMP(buf, val) \
    __extension__ ({ \
        int __result = 0; \
        void __nested_fn(int x) { \
            if (x > 0) { \
                longjmp(buf, x); \
            } \
        } \
        __nested_fn(val); \
        __result; \
    })

void process_data(void) {
    jmp_buf env;
    volatile int counter = 0;
    
    /* Force TLS initialization with dynamic initializer */
    tls_var = init_tls();
    
    if (setjmp(env) == 0) {
        /* Use nested function with non-local jump */
        NESTED_SETJMP(env, volatile_trigger);
        
        /* Access TLS variable in loop to ensure usage */
        for (int i = 0; i < 10; i++) {
            counter += tls_var + i;
        }
        
        /* Simulate error case with longjmp */
        if (counter > 100) {
            longjmp(env, 1);
        }
    } else {
        printf("longjmp executed\n");
    }
    
    /* Use complex static initializer */
    counter += complex_init;
    
    /* Prevent dead code elimination */
    volatile_trigger = counter;
}

/* Multi-file simulation using weak symbols */
extern void __attribute__((weak)) possibly_external_function(int x);
void __attribute__((weak)) possibly_external_function(int x) {
    printf("Weak function called with %d\n", x);
}

/* Main function with observable I/O */
int main(void) {
    /* Setup */
    srand(42);
    printf("Starting test...\n");
    
    /* Execute the key construct multiple times */
    for (int i = 0; i < 3; i++) {
        process_data();
        possibly_external_function(i);
        
        /* Modify volatile to change execution path */
        volatile_trigger = rand() % 10;
    }
    
    /* Final observable output */
    printf("Final TLS value: %d\n", tls_var);
    printf("Complex static: %d\n", complex_init);
    printf("Volatile trigger: %d\n", volatile_trigger);
    
    return 0;
}

/* Additional file simulation - this would normally be in separate file */
#ifdef SIMULATE_MULTIFILE
/* Force DECL_EXTERNAL handling */
int external_helper(void);
int external_helper(void) {
    return rand() % 50 + 50;
}
#endif
