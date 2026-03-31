/* test_expr_error.c - Trigger error_mark_node in expr.cc expansion */

/* Invalid void expressions in value contexts */
void void_func(void) {}

/* Vector type that might fail expansion on some targets */
typedef int v4si __attribute__((vector_size(16)));

/* Variadic function for va_arg misuse */
void variadic_func(int n, ...) {
    __builtin_va_list ap;
    __builtin_va_start(ap, n);
    /* Deliberate misuse below */
}

int main(void) {
    /* 1. Invalid void expressions in value contexts */
    /* These should trigger error during expansion */
    int x = (void)void_func();  /* Casting void to value */
    
    /* Comma operator with void left side in value context */
    int y = (void_func(), 5);
    
    /* sizeof with void expression - might pass parsing */
    int z = sizeof((void)0);
    
    /* 2. Misusing __builtin_va_arg outside proper context */
    __builtin_va_list ap;
    /* Using va_arg without va_start - invalid context */
    float f = __builtin_va_arg(ap, float);
    
    /* 3. Malformed compound literals */
    /* Non-existent field designator */
    struct S { int a; } *p = &(struct S){ .b = 1 };  /* .b doesn't exist */
    
    /* Taking address in invalid context */
    int *ptr = &(int){ &(int){0} };  /* Nested compound literal address */
    
    /* 4. Target-specific expansion failures */
    /* Vector operations might fail on non-vector targets */
    v4si v1, v2, v3;
    v3 = v1 + v2;  /* Vector addition */
    
    /* Overflow builtins with potentially unsupported types */
    int overflow;
    __builtin_add_overflow(1.5L, 2.5L, &overflow);  /* long double */
    
    /* 5. Complex nested invalid operations */
    /* Invalid void expression inside conditional */
    int w = (1 ? (void)0 : 0);
    
    /* Invalid void in __builtin_constant_p */
    int b = __builtin_constant_p((void)0);
    
    /* 6. Transactional Memory (if supported) */
    /* This might fail expansion if TM not supported */
    __transaction_atomic {
        x = 5;
    }
    
    /* 7. Misaligned pointer operations */
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);
    int *aligned = __builtin_assume_aligned(misaligned, 4);
    
    /* 8. Bit-field address attempt */
    struct BitField {
        unsigned int field:4;
    } bf;
    unsigned int *bf_ptr = &bf.field;  /* Taking address of bit-field */
    
    /* 9. Invalid offsetof usage */
    /* offsetof with bit-field - invalid per C standard */
    size_t off = __builtin_offsetof(struct BitField, field);
    
    /* 10. Complex expression with multiple issues */
    int complex_expr = (void_func(), 
                       __builtin_va_arg(ap, double),
                       sizeof((void)0));
    
    return 0;
}
