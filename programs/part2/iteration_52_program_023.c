/* test_error_mark_node.c
 * This program contains various syntactically valid but semantically invalid
 * expressions designed to trigger the error_mark_node return path in expr.cc.
 * It mixes valid and invalid code to ensure the parser engages before hitting errors.
 */

#include <stdio.h>

/* Macro to generate type mismatches */
#define BAD_ADD(a, b) ((a) + (b))
#define BAD_BITWISE(a, b) ((a) | (b))

/* Token concatenation to create questionable identifier */
#define CONCAT(a, b) a ## b
int CONCAT(foo, bar); /* This is actually valid declaration */

/* Invalid global initializer (excess initializers) */
int global_arr[3] = {1, 2, 3, 4};  /* Should trigger error: excess initializers */

/* Function prototype */
void some_function(int x);

int main(void) {
    /* Some valid code to establish context */
    int valid_int = 42;
    float valid_float = 3.14f;
    printf("Starting program...\n");
    
    /* 1. Type mismatch in binary operation (int + string literal) */
    int x = 5 + "string";  /* Invalid: adding int and pointer */
    
    /* 2. Invalid operand combination: bitwise OR on floating-point */
    double d = 3.14 | 2.5;  /* Invalid: bitwise on doubles */
    
    /* 3. Address-of operator on constant literal */
    int* p = &42;  /* Invalid: cannot take address of literal */
    
    /* 4. Using macro to generate type mismatch */
    int y = BAD_ADD(valid_int, "text");  /* Expands to: valid_int + "text" */
    
    /* 5. Invalid initializer: scalar with multiple values in braces */
    int z = {5, 6};  /* Invalid: multiple values for scalar */
    
    /* 6. Undeclared identifier (out of scope reference) */
    {
        int inner = 10;
    }
    /* Attempt to use 'inner' outside its scope */
    int outer = inner * 2;  /* 'inner' undeclared here */
    
    /* 7. Misuse of GNU C statement expression */
    int stmt_expr = ({ int a; });  /* Missing value - returns void */
    
    /* 8. Invalid function argument expression */
    some_function(valid_float & valid_int);  /* Bitwise AND on float and int */
    
    /* 9. Misuse of __builtin with wrong argument type */
    int trailing_zeros = __builtin_ctz("hello");  /* String instead of integer */
    
    /* 10. Pointer arithmetic type mismatch */
    float* fp = &valid_float;
    int offset = fp / 2;  /* Invalid: pointer division */
    
    /* 11. Array initializer with wrong type in list */
    int arr[3] = {1, 2.5, 3};  /* 2.5 is double, not int */
    
    /* 12. Taking address of function in arithmetic */
    int func_addr = (int)(&main) + 1;  /* Actually valid in C, but questionable */
    
    /* 13. Using bitwise operator on function pointer */
    void (*func_ptr)(void) = (void(*)(void))(&main);
    int weird = (int)func_ptr | 0x1;  /* Mixing pointer and arithmetic */
    
    /* 14. Compound literal misuse */
    int* ptr = &(int){5} + 1;  /* Taking address of temporary */
    
    /* 15. Another valid statement to keep parser going */
    int last_valid = 100;
    
    /* 16. Return with invalid expression */
    return main + 1;  /* Adding to function pointer */
}

/* Function definition */
void some_function(int x) {
    /* 17. Invalid in function body: applying sizeof to incomplete type */
    int size = sizeof(struct undefined);  /* Undefined struct */
    
    /* 18. Shift operator with wrong types */
    float shift_result = 3.14f << 2;  /* Shift on float */
    
    /* 19. Conditional operator type mismatch */
    int cond = valid_int ? "yes" : "no";  /* Different types in branches */
}

/* Global scope error */
int* global_bad = &3;  /* Address of constant in global scope */

/* Array with invalid size expression */
extern int extern_array[sizeof("string") % 2.5];  /* Mod with double */
