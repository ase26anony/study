/* test.c - Program designed to trigger error_mark_node returns in expr.cc */

#include <stdio.h>

/* Macro that generates type mismatches */
#define BAD_ADD(a, b) ((a) + (b))
/* Macro with token concatenation creating invalid usage */
#define MAKE_BAD_EXPR(type, val) type##_bad = val

/* Invalid global initializer (excess initializers) */
int global_arr[3] = {1, 2, 3, 4};  /* line 251 candidate: excess initializers */

/* Function prototype */
void test_func(int x);

int main(void) {
    /* Some valid code for context */
    int valid = 42;
    printf("Starting...\n");
    
    /* 1. Type mismatch in binary operation (int + string literal) */
    int x = 5 + "string";  /* line 251 candidate: invalid operands to binary + */
    
    /* 2. Invalid operand combination (bitwise on float) */
    double d = 3.14;
    double result = d | 2.5;  /* line 251 candidate: invalid operands to binary | */
    
    /* 3. Valid statement to maintain structure */
    int ok = 5;
    
    /* 4. Address-of operator on constant */
    int* p = &42;  /* line 251 candidate: lvalue required as unary & operand */
    
    /* 5. Using macro to generate type mismatch */
    int y = BAD_ADD(10, "text");  /* Expands to: 10 + "text" */
    
    /* 6. Invalid initializer for scalar */
    int z = {5, 6};  /* line 251 candidate: scalar initialized with multiple values */
    
    /* 7. Misuse of GNU C statement expression */
    int w = ({ int a; a; });  /* Missing return value in statement expression */
    
    /* 8. Invalid builtin usage */
    int bits = __builtin_ctz("hello");  /* line 251 candidate: invalid argument type */
    
    /* 9. Out-of-scope identifier attempt */
    {
        int inner = 99;
    }
    /* inner = 100; */  /* Would be error but commented - keeping as example */
    
    /* 10. Function argument with invalid expression */
    test_func(main + 1);  /* line 251 candidate: taking address of function in expression */
    
    /* 11. Pointer arithmetic with incorrect types */
    float f = &valid / 2;  /* line 251 candidate: invalid operands to binary / */
    
    return 0;
}

void test_func(int x) {
    /* 12. Undeclared identifier (without C23) */
    /* undeclared_var = x; */  /* Would be error but commented */
    
    /* 13. Array with invalid initializer */
    char arr[2] = "too long";  /* line 251 candidate: initializer string too long */
}
