/* Test file 1: Deeply nested structs and unions */
#ifndef NESTED_STRUCTS_H
#define NESTED_STRUCTS_H

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
            } inner_inner;
        } u;
    } nested;
};

/* Level 2: Array within nested struct */
struct ComplexArray {
    int matrix[3][4][5];
    struct {
        char *strings[10];
        union {
            int (*func_ptrs[5])(void);
            void (*void_ptrs[3])(int, char);
        } func_union;
    } data;
};

/* Level 3: Anonymous structs and unions */
struct AnonymousNest {
    struct {
        union {
            struct {
                int depth:4;
                int width:4;
            } bits;
            long long value;
        };
        char name[20];
    };
    int (*callback)(struct AnonymousNest *self);
};

/* Level 4: Mixed delimiters in single declaration */
struct MixedDelimiters {
    /* Array of function pointers returning pointers to arrays */
    int (*(*callbacks[3])(int))[5];
    
    /* Nested struct with array and bitfields */
    struct {
        unsigned flags:8;
        int counters[2][3];
        union {
            char str[10];
            void *ptr;
        } variant;
    } state;
    
    /* Function pointer with complex signature */
    void (*(*signal_handler)(int sig, void *data))(void);
};

/* Level 5: Designated initializers (in type definition context) */
struct WithInitializer {
    int x;
    struct {
        int a;
        int b;
        int c;
    } coords;
    int arr[2][3];
} default_instance = {
    .x = 42,
    .coords = { .a = 1, .b = 2, .c = 3 },
    .arr = { {1, 2, 3}, {4, 5, 6} }
};

#endif /* NESTED_STRUCTS_H */
