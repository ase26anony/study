/* test_gengtype_coverage.h - Complex type definitions to test gengtype parser */
#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex Nested Type Definitions with all bracket types */
struct OuterStruct {
    /* Function pointer array with nested parentheses */
    int (*func_array[5])(void (*callback)(int, char));
    
    /* Nested struct with bit-fields (braces) */
    struct {
        unsigned int flags:4;
        unsigned int status:2;
        struct {
            int x:8;
            int y:8;
        } nested_bits;
    } bitfield_struct;
    
    /* Multi-dimensional array (brackets) */
    double matrix[3][4][2];
    
    /* Union containing function pointer */
    union {
        void (*func_ptr)(int);
        int (*array_func[2])(char);
    } func_union;
};

/* 2. Function Pointer Declarations with Varied Signatures */
/* Pointer to function returning pointer to function */
int (*(*complex_func_ptr)(int (*)(char[10])))(double);

/* Function returning pointer to array of function pointers */
int (*(*get_handler_array(void))[5])(int, int);

/* Nested function pointer type */
typedef void (*(*nested_fp_type)(int (*(*)(char))(float)))(double);

/* 3. Multi-dimensional Arrays and Flexible Array Members */
struct ArrayContainer {
    int multi_dim[2][3][4];
    char strings[5][256];
    
    /* Flexible array member */
    int flexible_array[];
};

/* Array of pointers to arrays */
int (*pointer_array[10])[20];

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
struct AnonymousContainer {
    /* Anonymous union */
    union {
        struct {
            unsigned int a:1;
            unsigned int b:3;
            unsigned int c:4;
        } bits;
        unsigned char byte;
    };
    
    /* Anonymous struct */
    struct {
        long x;
        long y;
    };
    
    /* Named struct with bit-fields inside union */
    union {
        struct {
            unsigned int flag1:1;
            unsigned int flag2:2;
            unsigned int flag3:5;
        } flags;
        unsigned int raw;
    } status;
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*(*)(T))(T)
#define ARRAY_TYPE(N, T) T (*)[N]
#define COMPLEX_PTR(T) T (*(*(*)(T (*)[5]))(T))[10]

/* Use the macros in declarations */
PTR_FUNC(int) macro_func_ptr;
ARRAY_TYPE(3, double) array_ptr;
COMPLEX_PTR(char) ultra_complex_ptr;

/* Struct using macro expansions */
struct MacroStruct {
    PTR_FUNC(void) handler;
    ARRAY_TYPE(4, int) matrix_ptr;
};

/* 6. Attribute Syntax with Parentheses */
/* Struct with alignment attribute */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    char padding[12];
} __attribute__((packed));

/* Function with attributes */
void __attribute__((noreturn)) 
__attribute__((format(printf, 1, 2))) 
attributed_func(const char *fmt, ...);

/* Variable with section attribute */
int __attribute__((section(".data"))) 
__attribute__((used)) global_var = 42;

/* 7. Include All Bracket Types in Single Declaration */
struct UltimateStruct {
    /* Complex function pointer declaration */
    void (*signal(int sig, void (*handler)(int)))(int);
    
    /* Array of function pointers with complex signature */
    int (*func_array[3])(int (*)(char[10]), void (*)(double));
    
    /* Nested union with bit-fields */
    union {
        struct {
            unsigned int a:1;
            unsigned int b:2;
            struct {
                unsigned int x:3;
                unsigned int y:4;
            } nested;
        } bits;
        unsigned long value;
    } flags;
    
    /* Multi-dimensional array */
    char buffer[2][256][128];
    
    /* Pointer to array of pointers */
    int *(*(*nested_ptr)[5])[10];
    
    /* Anonymous struct */
    struct {
        int x;
        int y;
        int z;
    };
} __attribute__((aligned(32)));

/* Additional complex type combinations */
typedef struct {
    /* Function returning pointer to function taking array */
    int (*(*get_processor(void))[5])(int[3]);
    
    /* Nested anonymous union */
    union {
        int (*simple_func)(void);
        struct {
            int (*complex_func)(int (*)(char));
            unsigned int bits:8;
        };
    } processor;
    
    /* Flexible array of function pointers */
    void (*dynamic_handlers[])(int, ...);
} ProcessorContainer;

/* Even more nesting */
struct RecursiveNesting {
    /* Pointer to self */
    struct RecursiveNesting *next;
    
    /* Array of pointers to functions returning pointers to similar structs */
    struct RecursiveNesting *(*generators[2])(int);
    
    /* Union containing struct with bit-fields and function pointer */
    union {
        struct {
            unsigned int depth:4;
            unsigned int type:3;
            void (*visit)(struct RecursiveNesting *);
        } info;
        unsigned long raw_info;
    };
    
    /* Multi-dimensional pointer array */
    int *(*(*ptr_matrix[2])[3])[4];
};

/* Test case with all bracket types in typedef */
typedef int (*(*(*UltraComplexType)(int (*(*)(char[5]))[3]))(double))[2];

/* Final test: struct with every possible bracket combination */
struct AllBrackets {
    int a;                                  /* default case */
    int b[10];                              /* '[' case */
    int c[5][7];                            /* nested '[' cases */
    void (*d)(int);                         /* '(' case */
    void (*e[3])(int, char);               /* '[' then '(' cases */
    struct {                                /* '{' case */
        union {                             /* nested '{' case */
            int x;
            long y;
        } u;
        int z:4;                           /* bit-field */
    } nested;
    int (*(*f)(int (*)(char)))(double);    /* multiple '(' cases */
} __attribute__((packed, aligned(8)));

#endif /* TEST_GENGTYPE_COVERAGE_H */
