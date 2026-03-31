/* test_error_mark_node.c
 * This program contains various semantically invalid expressions designed
 * to trigger the error_mark_node return path in expr.cc during compilation.
 * The program will fail to compile, which is the intended behavior.
 */

/* Global scope error: invalid initializer with excess elements */
int global_arr[3] = {1, 2, 3, 4};  /* Requirement 1: excess initializers */

/* Macro to generate type mismatches */
#define BAD_ADD(a, b) (a + b)  /* Requirement 5: macro for malformed expressions */
#define CONCAT_INVALID(x, y) x##y##z  /* Requirement 5: token concatenation */

/* Function prototype */
void some_function(int param);

int main(void) {
    /* Valid code structure to ensure parser engagement */
    int valid_var = 42;
    printf("Starting...\n");  /* Valid statement for context */
    
    /* ---------------------------------------------------------------------
     * SEQUENCE OF MALFORMED EXPRESSIONS TO TRIGGER error_mark_node
     * --------------------------------------------------------------------- */
    
    /* 1. Type mismatch in binary operation (int + string literal) */
    /* Requirement 1: type mismatches, Requirement 2: function argument */
    printf("%d\n", BAD_ADD(5, "string"));  /* Macro expansion creates error */
    
    /* 2. Invalid operand combination: bitwise operator on floating-point */
    /* Requirement 1: invalid operand combinations */
    double d = 3.14;
    int bit_result = d | 2.5;  /* Bitwise OR on doubles */
    
    /* 3. Address-of operator on constant literal */
    /* Requirement 1: & on constants */
    int* p = &42;  /* Taking address of literal constant */
    
    /* 4. Scalar initialized with multiple values in braces */
    /* Requirement 1: invalid initializers, Requirement 2: variable declaration */
    int z = {5, 6};  /* Scalar with brace-enclosed list */
    
    /* 5. Undeclared identifier usage (out of scope) */
    /* Requirement 1: undeclared identifiers */
    {
        int inner_scope = 10;
    }
    int outer_ref = inner_scope;  /* Reference to variable from ended inner scope */
    
    /* 6. Misuse of GNU C statement expression */
    /* Requirement 4: misuse of GCC extensions */
    int stmt_expr = ({ 
        int a; 
        /* Missing return value - statement expression yields void */
    });
    
    /* 7. Invalid builtin function usage */
    /* Requirement 4: misuse of __builtin functions */
    int ctz_result = __builtin_ctz("hello");  /* String instead of integer */
    
    /* 8. Pointer arithmetic with incorrect types */
    /* Requirement 1: type mismatches in binary operations */
    float* fp = &valid_var;  /* Type mismatch in pointer assignment */
    float y = fp / 2;  /* Pointer arithmetic with float pointer */
    
    /* 9. Invalid token concatenation result */
    /* Requirement 5: token concatenation creating invalid identifier */
    int CONCAT_INVALID(foo, bar) = 10;  /* Attempts to create 'foobarz' variable */
    
    /* 10. Return statement with invalid expression */
    /* Requirement 2: return statement with error */
    return &main + 1;  /* Taking address of function and adding integer */
}

/* Additional function with errors in parameter */
void some_function(int param) {
    /* 11. Invalid expression in control flow condition */
    /* Requirement 2: errors in control flow */
    if (param + "invalid") {  /* int + string literal in condition */
        /* Empty */
    }
    
    /* 12. Array with too many initializers in local scope */
    /* Requirement 1: excess initializers */
    int local_arr[2] = {1, 2, 3, 4};
    
    /* 13. Valid statement to provide context */
    int ok = 5;  /* Requirement 3: mix valid and invalid code */
}
