/* test_expr_error_mark.c
 * This program contains various constructs designed to trigger
 * error_mark_node returns in expr.cc during RTL expansion.
 * The goal is compile-time failure, not runtime execution.
 */

/* ====== 1. Invalid void expressions ====== */
void void_func(void) {}

/* ====== 2. Variadic function for va_arg misuse ====== */
#include <stdarg.h>
void variadic_func(int n, ...) {
    va_list ap;
    va_start(ap, n);
    /* Correct usage inside variadic function */
    int x = va_arg(ap, int);
    va_end(ap);
}

/* ====== 3. Struct for compound literal errors ====== */
struct mystruct {
    int valid_field;
    int another_field;
};

/* ====== 4. Vector type for unsupported target testing ====== */
typedef int v4si __attribute__((vector_size(16)));

/* ====== 5. Transactional memory test ====== */
#ifdef __TM__
int tm_var = 0;
#endif

/* ====== Main function with various error triggers ====== */
int main() {
    /* Trigger 1a: Using void expression where value is required */
    /* This should fail during expansion */
    int x = (void)void_func();
    
    /* Trigger 1b: Comma operator with void left side in value context */
    int y = (printf("hello"), 5);
    
    /* Trigger 1c: sizeof with void expression */
    size_t sz = sizeof((void)0);
    
    /* Trigger 1d: Conditional operator with void */
    int z = (1 ? (void)0 : 0);
    
    /* Trigger 2a: Misusing __builtin_va_arg outside variadic context */
    /* This is particularly likely to trigger error_mark_node */
    va_list fake_ap;
    float f = __builtin_va_arg(fake_ap, float);
    
    /* Trigger 2b: Type mismatch in va_arg */
    /* Create a scenario where promoted type doesn't match */
    variadic_func(1, 42);  // int passed
    /* But in another context, try to read wrong type */
    {
        va_list ap2;
        /* This might fail during expansion if type checking is deferred */
        double d = __builtin_va_arg(ap2, double);
    }
    
    /* Trigger 3a: Compound literal with invalid designator */
    int *p = &(int){ .non_existent = 1 };
    
    /* Trigger 3b: Complex compound literal misuse */
    /* Taking address of compound literal in non-lvalue context */
    struct mystruct *sp = &(struct mystruct){ 
        .valid_field = 1,
        .another_field = 2 
    };
    
    /* Trigger 3c: Nested invalid compound literal */
    int *q = &(int){ (void)void_func() };
    
    /* Trigger 4a: Vector operations on potentially unsupported target */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;  /* Might fail expansion on non-vector targets */
    
    /* Trigger 4b: Vector extract with invalid index */
    int elem = c[10];  /* Out of bounds - might fail during expansion */
    
    /* Trigger 5: Transactional memory if supported */
    #ifdef __TM__
    __transaction_atomic {
        tm_var = 42;
    }
    #endif
    
    /* Trigger 6: Builtin overflow with potentially unsupported type */
    /* Try with long double which might not be supported */
    long double ld1 = 1.0e100L;
    long double ld2 = 2.0e100L;
    int overflow;
    /* This builtin might fail during expansion for long double */
    __builtin_add_overflow(ld1, ld2, &ld1);
    
    /* Trigger 7: Complex numbers in overflow builtins */
    _Complex double cd1 = 1.0 + 2.0i;
    _Complex double cd2 = 3.0 + 4.0i;
    /* This should definitely fail during expansion */
    __builtin_add_overflow(cd1, cd2, &cd1);
    
    /* Trigger 8: Misaligned pointer operations */
    char buffer[100];
    int *misaligned = (int*)(buffer + 1);  /* Misaligned by 1 byte */
    /* Force assume aligned on misaligned pointer */
    int *forced = __builtin_assume_aligned(misaligned, 16);
    *forced = 42;  /* Might fail during expansion on strict-align targets */
    
    /* Trigger 9: Nested invalid operations in complex expressions */
    /* Invalid void expression inside __builtin_constant_p */
    int is_const = __builtin_constant_p((void)void_func());
    
    /* Trigger 10: Bit-field address taking in complex context */
    struct bitfield_struct {
        unsigned int field:4;
    } bfs = {0};
    
    /* Attempt to take address of bit-field in complex expression */
    unsigned int *bf_ptr = (unsigned int*)&bfs.field;
    
    /* Trigger 11: Invalid pointer arithmetic with non-lvalue */
    int *ptr_arr = (int[]){1, 2, 3};
    int *weird_ptr = &ptr_arr[(void)void_func()];
    
    /* Trigger 12: Using __builtin_choose_expr with invalid types */
    int choice = __builtin_choose_expr(1, (void)0, 42);
    
    /* Trigger 13: Invalid switch case values (constant but invalid) */
    switch (x) {
        case (void)void_func():  /* Invalid case expression */
            break;
        default:
            break;
    }
    
    /* Trigger 14: Array indexing with void expression */
    int arr[10];
    int val = arr[(void)void_func()];
    
    /* Trigger 15: Nested in macro-like builtins */
    /* __builtin_offsetof with invalid member */
    size_t off = __builtin_offsetof(struct mystruct, non_existent);
    
    return 0;
}
