/* test_expr_error.c
 * This program contains various constructs designed to trigger
 * error_mark_node returns in expr.cc during compilation.
 * The program may not compile successfully - that's the point.
 */

#include <stdarg.h>
#include <stdio.h>

/* Helper functions */
void void_func(void) { printf("void\n"); }
int int_func(int x) { return x * 2; }

/* For vector type testing */
typedef int v4si __attribute__((vector_size(16)));

/* For variadic testing */
void variadic_func(int count, ...) {
    va_list ap;
    va_start(ap, count);
    /* Intentionally empty - just for setup */
    va_end(ap);
}

int main(void) {
    /* 1. Invalid operations on void expressions */
    /* This should trigger error during expansion */
    int x = (void)void_func();  /* Direct void cast in value context */
    
    /* Comma operator with void left side in value context */
    int y = (printf("test"), 5);
    
    /* Void expression in conditional */
    int z = (void_func() ? 1 : 2);
    
    /* 2. Misusing __builtin_va_arg */
    /* Create a va_list but use it incorrectly */
    va_list ap;
    /* Using va_arg without proper initialization */
    float f = __builtin_va_arg(ap, float);
    
    /* Type mismatch in va_arg */
    variadic_func(3, 1, 2, 3);
    /* This would be wrong if we had access to ap here */
    
    /* 3. Malformed compound literals */
    /* Non-existent field designator */
    struct S { int a; int b; };
    int *p = &(struct S){ .c = 1 };  /* .c doesn't exist */
    
    /* Taking address in invalid context */
    int *q = &(int){ &main };  /* Type mismatch in initializer */
    
    /* 4. Target-specific expansion failures */
    /* Vector operations on potentially unsupported targets */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;  /* May fail expansion on non-vector targets */
    
    /* Overflow builtins with complex types */
    _Complex double c1 = 1.0 + 2.0i;
    _Complex double c2 = 3.0 + 4.0i;
    /* This might fail if overflow detection not supported for complex */
    int overflow = __builtin_add_overflow_p(c1, c2, (_Complex double)0);
    
    /* 5. Transaction Memory constructs (if supported) */
    #ifdef __GNUC__
    __transaction_atomic {
        x = x + 1;
    }
    #endif
    
    /* 6. Nested invalid operations */
    /* Void expression inside sizeof */
    size_t s = sizeof((void)void_func());
    
    /* Void expression as builtin argument */
    int is_const = __builtin_constant_p((void)0);
    
    /* Invalid address operations */
    int arr[5];
    /* Taking address of bit-field-like expression */
    int *r = &(arr[0]++);  /* Not an lvalue after increment */
    
    /* 7. Misaligned pointer operations */
    char buffer[100];
    int *misaligned = (int*)(buffer + 1);
    /* Force assumption of alignment */
    int *aligned = __builtin_assume_aligned(misaligned, 16);
    *aligned = 42;  /* Potential misaligned access */
    
    /* 8. Complex expression with multiple errors */
    /* This creates a deep expression tree with errors at leaves */
    int complex_result = 
        (*(int*)((void)void_func(), &x)) +  /* Invalid comma with void */
        __builtin_va_arg(ap, struct S) +    /* Wrong va_arg type */
        ((v4si){1,2,3,4}[5]) +              /* Vector out of bounds */
        (int)(&(int){.non_exist=5});        /* Invalid compound literal */
    
    /* 9. Using __builtin_choose_expr with invalid types */
    int choice = __builtin_choose_expr(1, (void)0, 5);
    
    /* 10. Invalid pointer arithmetic */
    void *vp = &x;
    int *ip = vp + 5;  /* Pointer arithmetic on void* */
    
    /* 11. Attempt to modify string literal through pointer */
    char *str = "hello" + 2;
    *str = 'x';  /* Modification of string literal */
    
    /* 12. Invalid use of offsetof */
    #include <stddef.h>
    size_t off = offsetof(struct S, c);  /* Non-existent member */
    
    /* 13. Register variable in invalid context */
    register int reg_var asm ("ax");
    /* Taking address of register variable */
    int *reg_addr = &reg_var;
    
    /* 14. Invalid flexible array member access */
    struct Flex {
        int count;
        int data[];  /* Flexible array member */
    };
    struct Flex *flex = 0;
    int flex_elem = flex->data[0];  /* Null pointer dereference */
    
    /* 15. __builtin_constant_p with side effects */
    int side_effect = __builtin_constant_p(x++);
    
    return 0;
}
