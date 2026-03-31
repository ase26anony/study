/* gengtype_test_coverage.h
 * Complex type definitions to test gengtype's consume_balanced function
 * Specifically targets lines 341-352 in gengtype-parse.cc
 */

#ifndef GENGTYPE_TEST_COVERAGE_H
#define GENGTYPE_TEST_COVERAGE_H

/* 1. MACRO EXPANSIONS GENERATING BRACKETS */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_DECL(T, n) T[n]
#define NESTED_PTR(T) T (*(*))(T)
#define COMPLEX_MACRO(T) struct { T (*func)(T (*)(T)); }

/* 2. FUNCTION POINTERS WITH VARIED SIGNATURES */
/* Simple function pointer */
typedef void (*simple_func_ptr)(int);

/* Pointer to function returning pointer to function */
typedef int (*(*complex_func_ptr)(int (*)(char)))(double);

/* Function pointer with nested parentheses in parameters */
typedef void (*nested_param_func)(int (*callback)(char[10]), float);

/* 3. COMPLEX STRUCT WITH ALL BRACKET TYPES */
struct MasterStruct {
    /* Parentheses: function pointer members */
    void (*signal_handler)(int sig, void (*cleanup)(void));
    int (*(*get_processor)(void))(int);
    
    /* Brackets: multi-dimensional arrays */
    int matrix[3][4][5];
    char *string_array[10][20];
    
    /* Braces: nested anonymous struct/union */
    union {
        struct {
            unsigned int flag1:1;
            unsigned int flag2:3;
            unsigned int :4;  /* Unnamed bit-field */
            unsigned int flag3:8;
        } bits;
        unsigned int raw;
    } bitfield_container;
    
    /* Flexible array member */
    int flexible_array[];
};

/* 4. DEEPLY NESTED TYPE DEFINITIONS */
typedef struct Node {
    /* Array of function pointers */
    void (*(*callbacks[5])(int (*)(char)))(void);
    
    /* Nested struct with bit-fields */
    struct {
        union {
            struct {
                unsigned int a:2;
                unsigned int b:2;
                unsigned int c:4;
            } nested_bits;
            unsigned char byte;
        } inner_union;
        long data;
    } nested_struct;
    
    /* Pointer to array of pointers */
    int *(*(*complex_array_ptr)[10])[5];
    
    struct Node *next;
} Node_t;

/* 5. UNION WITH COMPLEX MEMBERS */
union MegaUnion {
    /* Function pointer with array parameter */
    int (*func_with_array)(int arr[10][20], void (*cb)(void));
    
    /* Struct containing anonymous union */
    struct {
        union {
            float f;
            double d;
        };
        int count;
    } container;
    
    /* Multi-level pointer */
    int ****quadruple_ptr;
};

/* 6. USING MACRO EXPANSIONS */
PTR_FUNC(int) *macro_func_ptr;
ARRAY_DECL(char *, 5) string_ptrs;
NESTED_PTR(double) crazy_ptr;

/* 7. ATTRIBUTE SYNTAX WITH PARENTHESES */
struct AlignedStruct {
    int data;
    char buffer[64];
} __attribute__((aligned(64), packed));

typedef int (*(*attributed_func_ptr)(int))
    __attribute__((nonnull(1), returns_nonnull));

/* 8. SINGLE HIGHLY COMPLEX DECLARATION (combining all bracket types) */
struct UltimateType {
    /* Complex function pointer declaration */
    void (*(*signal(int sig, void (*handler)(int)))(int, void (*)(void)))(char);
    
    /* Array of function pointers with complex signatures */
    int (*(*func_array[3])(int (*)(char[10]), float))(double);
    
    /* Nested anonymous struct/union with bit-fields */
    union {
        struct {
            unsigned int a:1;
            unsigned int b:2;
            unsigned int c:3;
            unsigned int :2;  /* Padding */
            unsigned int d:4;
        } bits;
        unsigned short all;
    } flags __attribute__((packed));
    
    /* Multi-dimensional array */
    unsigned char bitmap[8][8][3];
    
    /* Pointer to flexible array member in another struct */
    struct {
        int length;
        int data[];
    } *flex_struct_ptr;
    
    /* Macro-expanded type */
    COMPLEX_MACRO(long) macro_generated;
} __attribute__((aligned(128)));

/* 9. TYPEDEF WITH COMPLEX NESTING */
typedef struct {
    /* Function returning pointer to array */
    int (*(*get_matrix(void))[10][10])(void);
    
    /* Nested function pointers */
    void (*(*(*nested_fp)(int))(float))(double);
    
    /* Anonymous union with bit-fields */
    union {
        struct {
            unsigned int x:5;
            unsigned int y:5;
            unsigned int z:6;
        };
        unsigned int packed;
    } coordinates;
} ComplexTypedef;

/* 10. EXTERNAL DECLARATIONS WITH ATTRIBUTES */
extern struct MasterStruct *global_master
    __attribute__((visibility("default"), alias("master_alias")));

extern void (*(*exported_func_ptr)(int))
    __attribute__((weak, deprecated("use new_api instead")));

/* 11. ADDITIONAL EDGE CASES */
/* Empty struct (still has braces) */
struct Empty {};

/* Struct with only bit-fields */
struct OnlyBitfields {
    unsigned int a:1;
    unsigned int b:1;
    unsigned int :0;  /* Force alignment */
    unsigned int c:30;
};

/* Array of empty structs */
struct Empty empty_array[10];

/* Function pointer with no parameters */
typedef void (*nullary_func_ptr)(void);

/* 12. FINAL COMPLEX NESTING EXAMPLE */
typedef union {
    struct {
        /* Triple pointer with array */
        int **(*ptr_array[5])(void);
        
        /* Function pointer with struct parameter */
        void (*struct_func)(struct MasterStruct, struct UltimateType*);
        
        /* Nested anonymous struct */
        struct {
            int a;
            struct {
                short x;
                short y;
            } point;
        } data;
    } s;
    
    /* Array of function pointers returning function pointers */
    void (*(*(*fp_array[2])(int))(float))(char);
} FinalUnion;

#endif /* GENGTYPE_TEST_COVERAGE_H */
