/* test_expr_error_mark.c
 * This program contains various constructs designed to trigger the
 * error_mark_node return path in expr.cc during compilation.
 * The program may not compile successfully - that's the point!
 */

/* ====== 1. Invalid operations on void ====== */
void void_func(void) {}

/* ====== 2. Misusing __builtin_va_arg ====== */
#include <stdarg.h>

/* ====== 3. Malformed compound literals ====== */
struct bad_struct {
    int valid_field;
};

/* ====== 4. Target-specific expansion failures ====== */
/* Vector types - may fail on targets without vector support */
typedef int v4si __attribute__((vector_size(16)));

/* Complex types for overflow builtins */
typedef _Complex float complex_float;

/* ====== 5. Transaction Memory constructs ====== */
#ifdef __TM__
/* Only define if TM is available */
#endif

int main(void) {
    /* Trigger 1: Invalid void operations in expression context */
    /* These should cause expansion failures */
    int x = (void)void_func();  /* Direct void cast in value context */
    
    /* Comma operator with void left side in value context */
    int y = (printf("hello"), 5);
    
    /* Void expression in conditional operator */
    int z = (1 ? (void)0 : 0);
    
    /* Nested void expression */
    int w = sizeof((void)void_func());
    
    /* Trigger 2: Misusing __builtin_va_arg */
    /* Create a va_list but use it incorrectly */
    va_list ap;
    /* This is invalid - using va_arg without proper initialization */
    /* The type mismatch and context should trigger expansion error */
    int bad_va = __builtin_va_arg(ap, float);
    
    /* Trigger 3: Malformed compound literals */
    /* Invalid designator */
    int *p = &(int){ .non_existent_field = 1 };
    
    /* Type mismatch in compound literal */
    struct bad_struct *bp = &(struct bad_struct){ .valid_field = "string" };
    
    /* Compound literal in invalid context */
    int *q = &(int){ 1 } + 1;  /* Taking address then arithmetic */
    
    /* Trigger 4: Target-specific expansion failures */
    /* Vector operations on potentially unsupported target */
    v4si v1, v2, v3;
    v3 = v1 + v2;  /* Vector addition */
    
    /* Overflow builtin with complex type */
    complex_float cf1, cf2;
    int overflow;
    /* This may fail expansion if complex overflow not supported */
    __builtin_add_overflow(cf1, cf2, &overflow);
    
    /* __builtin_constant_p with invalid expression */
    int constant_test = __builtin_constant_p((void)0);
    
    /* Trigger 5: Invalid address operations */
    /* Attempt to take address of non-lvalue */
    int *addr = &(x + y);
    
    /* Misaligned pointer in alignment context */
    char buffer[10];
    int *misaligned = (int*)&buffer[1];
    int *aligned = __builtin_assume_aligned(misaligned, 4);
    
    /* Trigger 6: Transaction Memory (if enabled) */
    /* This may fail expansion if TM not supported */
#ifdef __TM__
    __transaction_atomic {
        x = y + z;
    }
#endif
    
    /* Trigger 7: Nested invalid operations in complex expressions */
    /* Multiple layers of problematic constructs */
    int complex_error = (1 ? 
                        (__builtin_va_arg(ap, int) + (void)void_func()) : 
                        sizeof(&(int){ .invalid = 0 }));
    
    /* Invalid bitfield address attempt */
    struct {
        unsigned int bitfield : 3;
    } bf;
    /* Taking address of bitfield - invalid */
    unsigned int *bitaddr = (unsigned int*)&bf.bitfield;
    
    /* Trigger 8: Using __builtin_choose_expr with invalid types */
    /* The invalid branch may still be evaluated during expansion */
    int choose = __builtin_choose_expr(1, (void)0, 42);
    
    /* Trigger 9: Invalid pointer arithmetic with void* */
    void *vp = &x;
    void *vp2 = vp + 1;  /* Invalid arithmetic on void pointer */
    
    /* Trigger 10: Using statement expressions with errors */
    int stmt_expr = ({
        void_func();
        (void)void_func();  /* void expression as value */
        5;
    });
    
    return 0;
}
