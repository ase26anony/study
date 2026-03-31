/* test_error_mark_node.c
 * Contains syntactically valid but semantically invalid expressions
 * to trigger error_mark_node returns in GCC's expr.cc
 */

/* Global scope errors */
#define BAD_MACRO(x, y) (x & y)
#define CONCAT(a, b) a##b

/* Invalid global initializer - excess initializers */
int global_arr[3] = {1, 2, 3, 4, 5};

/* Function prototype */
void some_function(int x);

/* Another macro that will be misused */
#define BAD_CAST(type, expr) ((type)(expr))

int main(void) {
    /* Valid code for context */
    int valid_int = 42;
    float valid_float = 3.14f;
    
    /* 1. Type mismatch in binary operation */
    int type_mismatch = valid_int + "string literal";
    
    /* Valid statement to separate errors */
    int another_valid = 100;
    
    /* 2. Invalid operand combination - bitwise on float */
    double bitwise_float = 3.14159 | 2.71828;
    
    /* 3. Address-of operator on constant */
    int* addr_of_const = &42;
    
    /* 4. Misuse of GCC statement expression */
    int bad_stmt_expr = ({
        int a;
        /* Missing return value - just a declaration */
        float b;
    });
    
    /* 5. Invalid initializer - scalar with multiple values */
    int bad_scalar_init = {5, 6, 7};
    
    /* 6. Using undeclared identifier (out of scope) */
    {
        int inner_scope = 99;
    }
    int use_out_of_scope = inner_scope * 2;
    
    /* 7. Macro-generated type mismatch */
    int macro_error = BAD_MACRO(valid_int, "text");
    
    /* 8. Invalid builtin usage */
    int builtin_error = __builtin_ctz("not an integer");
    
    /* 9. Bad cast through macro */
    float* bad_pointer_cast = BAD_CAST(float*, valid_int);
    
    /* 10. Invalid array subscript */
    char* ptr = "hello";
    float bad_subscript = ptr[valid_float];
    
    /* 11. Invalid conditional expression types */
    int bad_cond = (valid_int > "string") ? 1 : 2.0;
    
    /* 12. Taking address of function in expression */
    int func_addr_error = (int)(&main + 1);
    
    /* 13. Invalid compound literal */
    int* bad_literal = &(int){1, 2, 3};
    
    /* 14. Token concatenation creating invalid identifier */
    int CONCAT(123, 456) = 789;  /* Creates identifier 123456 */
    
    /* 15. Invalid shift operation */
    double shift_error = valid_float << 2;
    
    /* 16. Return statement with invalid expression */
    return &valid_int + "string";
}

/* Function with invalid expressions in parameters */
void some_function(int x) {
    /* 17. Invalid argument in function call */
    printf("%d", 5 + "str");
    
    /* 18. Invalid switch case expression */
    switch(x + "invalid") {
        case 1: break;
        default: break;
    }
    
    /* 19. Invalid sizeof operand */
    int bad_sizeof = sizeof(main + 1);
    
    /* 20. Invalid pointer arithmetic */
    int* ptr = &x;
    float* bad_ptr_arith = ptr / 2;
}

/* Global function with invalid return */
int* bad_global_func(void) {
    /* 21. Returning address of local through invalid expression */
    int local = 10;
    return &local + "text";
}

/* Struct with invalid initializer */
struct BadStruct {
    int a;
    float b;
};

/* 22. Invalid struct initializer */
struct BadStruct bs = {1, 2.0, "extra", 4};

/* 23. Invalid designator */
int designator_error[5] = {[1.5] = 10};

/* 24. Invalid complex expression with multiple errors */
void complex_error(void) {
    int x = 5;
    /* Multiple type mismatches in one expression */
    int y = (x + "str") * (&x - 3.14) | sizeof("text" + x);
}

/* 25. Invalid goto label expression */
void goto_error(void) {
    void* label = &&error_label;
    int z = label + 10;
error_label:
    return;
}
