/* Test file 1: Deeply nested structs and unions */
#ifndef DEEP_NESTED_STRUCTS_H
#define DEEP_NESTED_STRUCTS_H

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
        } u1;
    } nested;
};

/* Level 2: Anonymous structs and unions */
struct Level2 {
    struct {
        union {
            int x;
            struct {
                char y;
                struct {
                    unsigned z: 4;
                    unsigned w: 12;
                } bits;
            } s;
        };
        double arr[3][2];
    } anon;
    
    /* Array of nested structs */
    struct Complex {
        int (*callback)(void);
        union {
            void *ptr;
            struct {
                int a;
                int b;
            } pair;
        } data[5];
    } items[10];
};

/* Level 3: Mixed nesting with all delimiters */
struct Level3 {
    /* Function pointer inside struct */
    void (*func_ptr)(struct {
        int param1;
        char param2;
        struct {
            float nested_param;
        } inner_param;
    });
    
    /* 3D array */
    int matrix[2][3][4];
    
    /* Union with anonymous struct */
    union {
        struct {
            int a;
            int b;
        };
        struct {
            long c;
            long d;
        } named;
    } choice;
};

/* Designated initializer example */
struct WithDesignators {
    struct {
        int x;
        int y;
    } point;
    int values[5];
    union {
        char c;
        int i;
    } data;
};

#endif /* DEEP_NESTED_STRUCTS_H */
