/* test_error_mark_node.c
 * Contains syntactically valid but semantically invalid expressions
 * to trigger error_mark_node returns in GCC's expr.cc
 */

/* Global scope errors */
int global_bad = 5 + "string";  /* Type mismatch in global initializer */

/* Macro to generate bad expressions */
#define BAD_BINARY(a, b) (a | b)
#define BAD_ADDRESS(x) (&x)
#define BAD_INITIALIZER {1, 2, 3, 4, 5}

/* Function with various expression errors */
void problematic_function(void) {
    /* Valid code for context */
    int valid_var = 42;
    float valid_float = 3.14f;
    
    /* 1. Type mismatches in binary operations */
    int type_mismatch = valid_var + "hello";  /* int + string literal */
    float float_bitwise = valid_float | 0x0F;  /* float with bitwise OR */
    
    /* 2. Invalid operand combinations */
    int* bad_addr1 = &42;  /* Address of literal */
    int* bad_addr2 = &(valid_var + 1);  /* Address of temporary */
    
    /* 3. Statement expression misuse (GCC extension) */
    int bad_stmt_expr = ({ 
        int a; 
        /* Missing return value - just declaration */
    });
    
    /* 4. Builtin function misuse */
    int bad_builtin = __builtin_ctz("string");  /* Wrong argument type */
    
    /* 5. Array initializer with wrong count */
    int bad_array[3] = BAD_INITIALIZER;  /* Too many initializers */
}

/* Another function with scope issues */
void scope_problems(void) {
    {
        int inner_var = 100;
    }
    /* Use variable from inner scope */
    int use_out_of_scope = inner_var * 2;  /* Undeclared here */
}

/* Main function with control flow errors */
int main(void) {
    /* Some valid code first */
    int ok1 = 10;
    float ok2 = 2.5f;
    
    /* 6. Invalid expressions in control flow conditions */
    if (ok1 + "text") {  /* Type mismatch in condition */
        /* 7. Invalid return expression */
        return &main + 1;  /* Pointer arithmetic on function address */
    }
    
    /* 8. While loop with bad condition */
    while (ok2 & 0xF) {  /* Float with bitwise AND */
        /* 9. Invalid assignment */
        ok1 = ({ float f; });  /* Statement expr without value */
        break;
    }
    
    /* 10. For loop with bad increment */
    for (int i = 0; i < 5; i += "a") {  /* int += string */
        /* 11. Function call with bad arguments */
        printf("%d", 5 | 3.14);  /* Mixed types in binary op */
    }
    
    /* 12. Switch with bad case expression */
    switch (ok1) {
        case 1 + "bad":  /* Invalid case expression */
            break;
        default:
            /* 13. Compound literal misuse */
            int* bad_ptr = &(int){1, 2};  /* Too many initializers */
    }
    
    /* 14. Using macro-generated bad expression */
    int macro_bad = BAD_BINARY(3.14, "text");
    
    /* 15. Comma operator with type mismatch */
    int bad_comma = (ok1, "string", 3.14);
    
    /* 16. Conditional operator type mismatch */
    int bad_conditional = ok1 ? "yes" : 3.14;
    
    /* 17. Offsetof misuse */
    struct S { int a; char b; };
    size_t bad_offset = __builtin_offsetof(struct S, c);  /* Nonexistent member */
    
    /* 18. Alignof misuse */
    int bad_align = __alignof__(ok1 + "string");  /* Invalid expression */
    
    /* 19. Complex number misuse (C99) */
    double _Complex z = 3.14 + "i";  /* Invalid complex initializer */
    
    /* 20. Designated initializer error */
    int bad_designated[5] = { [6] = 1 };  /* Out of bounds */
    
    return 0;
}

/* Additional global with bad initializer */
int extra_global[] = {1, {2, 3}, 4};  /* Nested braces incorrectly */
