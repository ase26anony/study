/* test_error_mark_node.c - Program to trigger error_mark_node returns in expr.cc */

/* Global scope errors */
#define BAD_MACRO(x, y) (x & y)  /* Bitwise operator macro */
#define CONCAT(a, b) a##b  /* Token concatenation */

int global = 5 + "string";  /* Type mismatch in global initializer */

/* Invalid global initializer with brace-enclosed list */
int bad_global = {10, 20};

/* Function prototype */
void some_function(int x);

/* Another macro that will be misused */
#define STATEMENT_EXPR({ int tmp = 5; tmp; })

int main(void) {
    /* Valid code for context */
    int valid_var = 42;
    printf("Starting program...\n");
    
    /* 1. Type mismatch in binary operation */
    int type_mismatch = valid_var + "hello";  /* int + string literal */
    
    /* 2. Invalid operand combination - bitwise on float */
    float f = 3.14;
    double d = 2.71;
    int bitwise_float = f | d;  /* Bitwise OR on floating-point types */
    
    /* 3. Address-of operator on constant */
    int* bad_ptr = &42;  /* Taking address of literal */
    
    /* 4. Undeclared identifier usage (out of scope) */
    {
        int inner_scope = 100;
    }
    int use_out_of_scope = inner_scope * 2;  /* inner_scope no longer in scope */
    
    /* 5. Invalid initializer - excess elements */
    int array[3] = {1, 2, 3, 4, 5};  /* Too many initializers */
    
    /* 6. Scalar with multiple initializers */
    int scalar = {7, 8, 9};  /* Multiple values for scalar */
    
    /* 7. Misuse of GCC statement expression */
    int stmt_expr = ({ int a; });  /* Missing value - just declaration */
    
    /* 8. Invalid pointer arithmetic */
    int x = 10;
    float ptr_arith = &x / 2;  /* Pointer in division */
    
    /* 9. Misuse of builtin function */
    int builtin_misuse = __builtin_ctz("string");  /* String argument to ctz */
    
    /* 10. Macro-generated type error */
    int macro_error = BAD_MACRO(3.14, "text");  /* Float and string to bitwise & */
    
    /* 11. Invalid token concatenation attempt */
    int CONCAT(123, 456) = 789;  /* Creates invalid identifier 123456 */
    
    /* 12. Function address in arithmetic */
    int func_arithmetic = (int)main + 100;  /* Function pointer arithmetic */
    
    /* 13. Invalid conditional expression types */
    int cond = (valid_var > "string") ? 1 : 2.0;  /* Mixed types in branches */
    
    /* 14. Compound literal misuse */
    int* bad_compound = &(int){1, 2};  /* Multiple values in compound literal */
    
    /* 15. Invalid switch case expression */
    switch(valid_var + "string") {  /* Type mismatch in switch expression */
        case 1: break;
        default: break;
    }
    
    /* 16. Return statement with invalid expression */
    return main + 1;  /* Function address arithmetic */
}

/* Function with invalid parameter initializer */
void some_function(int x = 5) {  /* Default arguments not valid in C */
    /* More errors inside function */
    int local_error = x + "inside";  /* Type mismatch */
    
    /* Invalid sizeof usage */
    int size = sizeof("string" + 5);  /* sizeof on invalid expression */
}
