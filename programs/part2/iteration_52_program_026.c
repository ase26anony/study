/* test_error_mark_node.c
 * This program contains various semantically invalid expressions designed
 * to trigger the error_mark_node return path in expr.cc during compilation.
 * The program will fail to compile - that's the intended behavior.
 */

/* 1. Invalid initializer - scalar with multiple values in braces */
int global_bad = {1, 2, 3};  /* Should trigger error in global context */

/* 2. Macro generating type mismatches */
#define BAD_ADD(a, b) ((a) + (b))
#define CONCAT_INVALID(a, b) a##b##_nonexistent

/* 3. Function prototype */
void some_function(int x);

int main(void) {
    /* Some valid code for context */
    int valid_var = 42;
    float valid_float = 3.14f;
    
    /* 4. Type mismatch in binary operation - int + string literal */
    int x = 5 + "string";  /* Invalid: adding int and pointer */
    
    /* 5. Invalid operand combination - bitwise on float */
    double d = 3.14 | 2.5;  /* Invalid: bitwise OR on floating point */
    
    /* 6. Address-of operator on constant */
    int* p = &42;  /* Invalid: cannot take address of literal */
    
    /* 7. Using macro with type mismatch */
    int y = BAD_ADD(10, "text");  /* Expands to: 10 + "text" */
    
    /* 8. Invalid initializer - excess initializers */
    int arr[3] = {1, 2, 3, 4, 5};  /* Too many initializers */
    
    /* 9. Statement expression misuse (GCC extension) */
    int z = ({ 
        int a; 
        /* Missing return value - just a declaration */
    });  /* Invalid: statement expression without value */
    
    /* 10. Builtin function with wrong argument type */
    int bits = __builtin_ctz("hello");  /* Invalid: string instead of int */
    
    /* 11. Invalid in return statement - address of function with arithmetic */
    /* Note: This would be after some valid code */
    if (valid_var > 0) {
        return main + 1;  /* Invalid: taking address of main function */
    }
    
    /* 12. Token concatenation creating invalid identifier */
    /* This creates identifier "var_nonexistent" which is undeclared */
    int CONCAT_INVALID(var, _nonexistent) = 5;
    
    /* 13. Scope violation attempt */
    {
        int inner_scope = 100;
    }
    /* Try to use out-of-scope variable */
    int outer = inner_scope * 2;  /* inner_scope not visible here */
    
    /* 14. Pointer arithmetic with wrong types */
    float* fp = &valid_float;
    int* ip = (int*)fp;
    float result = fp / ip;  /* Invalid: pointer division */
    
    /* 15. Array with invalid designator (C99 feature misused) */
    int arr2[5] = { [6] = 10 };  /* Invalid: designator out of bounds */
    
    /* More macro misuse */
    printf("%d", BAD_ADD(valid_var, "text"));  /* In function call argument */
    
    return 0;
}

/* 16. Invalid global with function address arithmetic */
int* global_ptr = (int*)main + 10;  /* Function pointer arithmetic */

/* 17. Complex expression with multiple errors */
void another_func(void) {
    /* Nested invalid expressions */
    int a = ({ 
        int x = 5 & 3.14;  /* Bitwise AND with float */
        x + "text";        /* Add int and string */
    });
    
    /* Invalid conditional expression */
    int b = (1 ? 5 : "string");  /* Types don't match in ternary */
    
    /* Misuse of offsetof with non-aggregate type */
    /* Note: offsetof requires struct/union type */
    size_t off = __builtin_offsetof(int, phantom);  /* Invalid: int has no members */
}

/* 18. Invalid function argument type */
void test_call(void) {
    some_function(5 + "text");  /* Invalid argument expression */
}
