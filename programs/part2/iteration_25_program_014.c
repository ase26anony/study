/* Test file for gengtype parser - deeply nested structs and unions */

/* Level 1: Basic nested struct */
struct Level1 {
    int a;
    struct {
        char b;
        union {
            short c;
            long d;
            struct {
                float e;
                double f;
            } inner;
        } u;
    } nested;
};

/* Level 2: Array within nested struct */
struct ComplexArray {
    int matrix[3][4][5];
    struct {
        char *strings[10];
        struct {
            void *pointers[7];
            union {
                int (*func_ptrs[5])(void);
                struct {
                    float data[20];
                } data_block;
            } u_array;
        } deep;
    } container;
};

/* Level 3: Bit-fields and anonymous unions */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    struct {
        union {
            struct {
                int : 4;  /* unnamed bit-field */
                signed int value : 12;
            } bits;
            unsigned short raw;
        } anon_union;
        int regular_field;
    } inner_struct;
    int trailing_array[];
};

/* Level 4: Designated initializers in type context */
struct WithInitializer {
    int x;
    struct {
        int a;
        int b;
        struct {
            char c;
            double d;
        } point;
    } nested = { .a = 1, .b = 2, .point = { .c = 'X', .d = 3.14 } };
    int arr[2][3] = { [0] = {1, 2, 3}, {4, 5, 6} };
};

/* Level 5: Function pointer inside nested struct */
struct WithFunctionPointer {
    int (*comparator)(const void *, const void *);
    struct {
        void (*setup)(struct WithFunctionPointer *);
        int (*process)(int, char **);
        struct {
            float (*transform)(float);
            double (*complex_op)(double, double (*)(double));
        } ops;
    } funcs;
};

/* Level 6: Mixed nested types with all delimiters */
struct UltimateNest {
    /* Parentheses in function pointer */
    int (*(*callback_provider)(void))[10];
    
    /* Brackets in array */
    struct {
        char data[5][7][9];
        union {
            /* Braces in initializer (in type context) */
            int matrix[2][2] = {{1, 2}, {3, 4}};
            long big_array[100];
        } storage;
    } container;
    
    /* Nested struct with bit-fields */
    struct {
        unsigned : 8;
        unsigned mode : 4;
        struct {
            int : 0;  /* force alignment */
            signed count : 16;
        } counter;
    } flags;
};

/* Level 7: Forward declaration with complex nested type */
struct ForwardDecl;
struct HasForwardRef {
    struct ForwardDecl *next;
    struct {
        struct ForwardDecl **array[5];
        void (*handler)(struct ForwardDecl *);
    } links;
};

/* Level 8: Union containing struct containing union... */
union DeepUnion {
    struct {
        union {
            struct {
                int a;
                union {
                    char b;
                    struct {
                        short c;
                        long d;
                    } pair;
                } inner;
            } data;
            float f;
        } choice;
        double dbl;
    } variant;
    void *ptr;
};

/* Level 9: Array of structs containing arrays of unions */
struct ArrayOfArrays {
    struct Element {
        union Value {
            int i;
            float f;
            char *str;
            struct {
                int x, y;
            } coord;
        } values[8];
        int count;
    } elements[16];
    
    struct {
        struct Element *ptr_array[4];
        union Value (*getter[2])(int);
    } meta;
};

/* Level 10: The ultimate challenge - all delimiters deeply nested */
struct FinalBoss {
    /* (((( )))) */
    int (*(*(*deep_func_ptr)(int (*)(char)))(float))[5];
    
    /* [[[[ ]]]] */
    struct {
        int four_d_array[2][3][4][5];
        struct {
            char ***string_matrix[7][8];
        } string_data;
    } arrays;
    
    /* {{{{ }}}} */
    union {
        struct {
            struct {
                union {
                    struct {
                        int deepest;
                    } level5;
                } level4;
            } level3;
        } level2;
    } level1 = { .level2 = { .level3 = { .level4 = { .level5 = { .deepest = 42 } } } } };
    
    /* Mix: ({[ ]}) pattern in declarations */
    struct Mixed {
        void (*func_array[3])(int[2], struct { char c; });
        int (*(*complex)[10])(char);
    } mixed = {
        .func_array = { NULL, NULL, NULL },
        .complex = NULL
    };
};
