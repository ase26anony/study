/* Test file 1: Deeply nested structs and unions */
#ifndef DEEP_NESTED_STRUCTS_H
#define DEEP_NESTED_STRUCTS_H

/* Struct with multiple levels of nesting */
struct Level1 {
    struct Level2 {
        union Level3 {
            struct {
                int a;
                struct {
                    char b;
                    union {
                        short s;
                        long l;
                        struct {
                            float f;
                            double d;
                        } nested;
                    } u;
                } inner;
            } s;
            int arr[3][4][5];
        } u_level3;
        
        /* Bit-fields with complex layout */
        unsigned int bitfield1 : 4;
        unsigned int : 0;  /* Force alignment */
        unsigned int bitfield2 : 12;
    } level2;
    
    /* Array within struct */
    int matrix[2][3];
};

/* Anonymous struct/union nesting */
struct OuterAnonymous {
    struct {
        int x;
        union {
            char c;
            struct {
                int a, b;
            } pair;
        } data;
    };
    
    struct {
        /* Designated initializers in type context */
        struct Point {
            int x, y, z;
        } points[10];
    } geometry;
};

/* Union with struct containing arrays */
union ComplexUnion {
    struct {
        int (*callback)(int, char);
        void (*handlers[5])(struct Level1*);
    } funcs;
    
    struct {
        int data[10];
        struct {
            char buffer[256];
            int length;
        } metadata;
    } storage;
};

/* Struct with all delimiter types mixed */
struct AllDelimiters {
    /* Parentheses in function pointer */
    int (*compare)(const void*, const void*);
    
    /* Brackets for arrays */
    void* pointers[10][20];
    
    /* Braces for nested struct */
    struct {
        /* Nested parentheses in complex declaration */
        char* (*(*get_name)(void))[5];
        
        /* Array with designated initializer style */
        struct {
            int values[3];
        } items[2];
    } nested;
};

#endif /* DEEP_NESTED_STRUCTS_H */
