/* Public TLS with default visibility - tests TREE_PUBLIC, DECL_VISIBILITY */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol - tests DECL_WEAK */
__thread int weak_tls_var __attribute__((weak));

/* Static TLS inside function - tests DECL_CONTEXT */
static void inner_function(void) {
    static __thread int local_tls = 100;  /* DECL_CONTEXT should be inner_function */
}

/* Function that returns address of TLS - forces address taking */
int* get_public_tls_addr(void) {
    return &public_tls;
}

/* Function that uses weak TLS */
int use_weak_tls(void) {
    if (&weak_tls_var) {
        return weak_tls_var;
    }
    return -1;
}

/* Function that uses local TLS */
void init_local_tls(void) {
    inner_function();  /* Triggers creation of local_tls */
}
