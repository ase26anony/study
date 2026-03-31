/* test_expr_error.c
 * This program contains syntactically valid but semantically invalid expressions
 * designed to trigger the error_mark_node return path in expr.cc during compilation.
 * The program will not compile successfully.
 */

/* Global scope errors */
#define BAD_MACRO(x, y) (x & y)  /* Bitwise operation on potentially incompatible types */

/* Misuse of GNU extensions */
#define BAD_STMT_EXPR(x) ({ x; })  /* Missing return value in statement expression */

/* Invalid macro through token concatenation */
#define CONCAT(a, b) a##b
#define MAKE_BAD_EXPR CONCAT(undeclared, _var)  /* Creates undeclared identifier */

int global = 5 + "string";  /* Type mismatch in global initializer (line 251 candidate) */

/* Function with multiple error patterns */
void problematic_function(void) {
    /* Valid code for context */
    int valid_int = 42;
    float valid_float = 3.14f;
    
    /* 1. Type mismatch in binary operation */
    int type_mismatch = valid_int + "hello";  /* int + string literal */
    
    /* 2. Invalid operand combination - bitwise on float */
    float bitwise_float = valid_float | 2.5;  /* bitwise OR on floating point */
    
    /* 3. Address-of operator on constant */
    int* bad_ptr = &42;  /* Taking address of literal */
    
    /* 4. Undeclared identifier from macro expansion */
    int bad_id = MAKE_BAD_EXPR;  /* Expands to undeclared_var */
    
    /* 5. Invalid initializer - scalar with multiple values */
    int bad_init = {5, 6, 7};  /* Multiple values for scalar */
    
    /* 6. Misuse of GNU statement expression */
    int stmt_expr = BAD_STMT_EXPR(valid_int);  /* Missing return value */
    
    /* 7. Invalid builtin usage */
    int builtin_bad = __builtin_ctz("string");  /* String argument to bit builtin */
    
    /* 8. Out of scope reference attempt */
    if (1) {
        int inner_scope = 10;
    }
    int scope_error = inner_scope;  /* inner_scope no longer in scope */
    
    /* 9. Array initializer with wrong count */
    int arr[3] = {1, 2, 3, 4, 5};  /* Too many initializers */
    
    /* 10. Macro with type mismatch */
    int macro_bad = BAD_MACRO(valid_int, "text");  /* int & string */
}

/* Another function with different context errors */
int bad_return_context(int x) {
    /* Valid statement for context */
    int y = x * 2;
    
    /* Return with invalid expression */
    return main + 1;  /* Taking address of function in expression */
}

/* Control flow with errors */
void control_flow_errors(void) {
    /* Error in if condition */
    if (5 / "string") {  /* Division with string */
        /* Error in loop body */
        for (int i = 0; i < 10; i++) {
            int loop_error = i + &i;  /* int + pointer */
        }
    }
    
    /* Error in while condition */
    while (&"constant") {  /* Address of string literal as condition */
        break;
    }
}

/* Function call with bad arguments */
void function_call_errors(void) {
    /* Assuming printf is available, but with bad arguments */
    /* printf("%d", 5 + "str"); */  /* Would need stdio.h */
    
    /* Simulated function call error */
    extern void some_func(int);
    some_func(3.14 | 2.71);  /* Bitwise OR on floats as argument */
}

int main(void) {
    /* Start with valid code */
    int ok_var = 100;
    ok_var = ok_var + 1;  /* Valid operation */
    
    /* Insert various errors in main context */
    
    /* 1. Invalid binary operation */
    float bad_math = &ok_var / 2;  /* Pointer arithmetic type mismatch */
    
    /* 2. Macro-generated error */
    int macro_result = BAD_MACRO(3.14, 2.5);  /* Bitwise on floats via macro */
    
    /* 3. Statement expression misuse */
    int stmt_result = ({ 
        int a = 5; 
        float b = 3.14;
        a + b;  /* Mixed type addition in statement expr */
    });
    
    /* 4. Invalid initializer */
    int bad_scalar_init = {1, 2};  /* Multiple values for scalar */
    
    /* 5. Call function with errors */
    problematic_function();
    
    /* 6. Use invalid return value */
    int ret_val = bad_return_context(10);
    
    /* 7. Control flow with errors */
    control_flow_errors();
    
    /* 8. Function call errors */
    function_call_errors();
    
    /* End with valid code */
    return 0;
}

/* Additional global error */
int* global_bad_ptr = &"global_string";  /* Address of string literal in global */
