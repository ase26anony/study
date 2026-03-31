/* test_error_mark_node.c
 * This program contains various syntactically valid but semantically invalid
 * expressions designed to trigger the error_mark_node return path in expr.cc.
 * The program will fail to compile during semantic analysis.
 */

/* Global scope errors */
#define BAD_MACRO(x, y) (x & y)  /* Bitwise operation on potentially incompatible types */

/* Invalid global initializer - excess initializers */
int global_arr[3] = {1, 2, 3, 4, 5};

/* Another macro that concatenates tokens to create problematic expressions */
#define CONCAT_EXPR(a, b) a##b + b##a

/* Function prototype */
void some_function(int param);

int main(void) {
    /* 1. Valid code for context */
    int valid_var = 42;
    printf("Starting program...\n");
    
    /* 2. Type mismatch in binary operation - int + string literal */
    int type_mismatch = 10 + "hello";
    
    /* 3. Invalid operand combination - bitwise OR on floating point */
    double floating_bitwise = 3.14159 | 2.71828;
    
    /* 4. Address-of operator on constant */
    int* bad_pointer = &42;
    
    /* 5. Using macro with type mismatch */
    int macro_result = BAD_MACRO(3.14, "text");
    
    /* 6. Another valid statement for context */
    int another_valid = valid_var * 2;
    
    /* 7. Invalid initializer - scalar with multiple values in braces */
    int bad_init = {5, 6, 7};
    
    /* 8. Undeclared identifier (out of scope reference) */
    {
        int inner_scope = 99;
    }
    int scope_error = inner_scope * 2;  /* inner_scope no longer in scope */
    
    /* 9. Misuse of GNU C statement expression */
    int stmt_expr_error = ({ 
        int a; 
        /* Missing return value - just a declaration */
    });
    
    /* 10. Invalid function argument */
    some_function(5 + "string");
    
    /* 11. Misuse of __builtin function */
    int builtin_error = __builtin_ctz("not_an_int");
    
    /* 12. Invalid array subscript */
    char* ptr = "test";
    float bad_subscript = ptr[3.14];
    
    /* 13. Using concatenated macro to create weird expression */
    int concat_result = CONCAT_EXPR(valid, _var);
    
    /* 14. Taking address of a function in arithmetic */
    void* func_addr = &main + 1;
    
    /* 15. Invalid conditional expression types */
    int cond_expr = (valid_var > 10) ? "yes" : 3.14;
    
    /* 16. Another valid statement */
    printf("Valid statement here\n");
    
    /* 17. Invalid compound literal */
    int* bad_literal = &(int){1, 2, 3};
    
    /* 18. Return with invalid expression */
    return main + valid_var;  /* Adding function pointer to integer */
}

void some_function(int param) {
    /* 19. Invalid expression in function body */
    double local_error = param / "text";
    
    /* 20. Control flow with invalid condition */
    if (&param + 10) {  /* Pointer arithmetic in condition */
        printf("This shouldn't compile\n");
    }
    
    /* 21. Loop with invalid increment */
    for (int i = 0; i < 10; i += "step") {
        /* Invalid increment expression */
    }
}

/* 22. Global variable with invalid initializer using address-of */
int* global_bad = &"constant";

/* 23. Invalid designated initializer */
struct Point {
    int x;
    int y;
};

struct Point bad_point = {
    .x = "not_a_number",  /* Type mismatch */
    .y = 3.14 | 2.5       /* Bitwise on float */
};
