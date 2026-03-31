/* test_types.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY macros */
#ifndef GTY
#define GTY(x) __attribute__((gty x))
#endif

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Various scalar types */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef int GTY(()) scalar_int;
typedef double GTY(()) scalar_double;

/* TYPE_STRUCT: Basic struct with various members */
struct GTY(()) basic_struct {
    int GTY(()) id;
    char GTY(()) name[32];
    double GTY(()) value;
    
    /* Anonymous struct member */
    struct GTY(()) {
        int GTY(()) x;
        int GTY(()) y;
    } position;
    
    /* Bit fields */
    unsigned GTY(()) flags : 4;
    unsigned GTY(()) status : 2;
};

/* TYPE_STRUCT: More complex nested struct */
struct GTY(()) complex_struct {
    struct GTY(()) basic_struct *GTY((skip)) inner;
    struct GTY(()) complex_struct *GTY((chain_next)) next;
    struct GTY(()) complex_struct *GTY((chain_prev)) prev;
    
    /* Array within struct */
    color_t GTY(()) colors[5];
    
    /* Flexible array member */
    int GTY(()) data[];
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    int GTY(()) user_id;
    void *GTY((skip)) user_data;
};

/* TYPE_UNION: Basic union */
union GTY(()) basic_union {
    int GTY(()) int_val;
    double GTY(()) double_val;
    char *GTY(()) string_val;
};

/* TYPE_UNION: Tagged union within struct */
struct GTY(()) tagged_union_container {
    enum GTY(()) union_type {
        INT_TYPE,
        DOUBLE_TYPE,
        STRING_TYPE
    } type;
    
    union GTY(()) {
        int GTY(()) i;
        double GTY(()) d;
        char *GTY((length("strlen($)"))) s;
    } value;
};

/* TYPE_POINTER: Various pointer types */
typedef struct GTY(()) basic_struct *GTY(()) basic_struct_ptr;
typedef union GTY(()) basic_union *GTY(()) basic_union_ptr;
typedef void (*GTY(()) callback_func)(int, void*);

/* TYPE_ARRAY: Various array types */
typedef int GTY(()) int_array[10];
typedef struct GTY(()) basic_struct GTY(()) struct_array[5];
typedef int GTY(()) multi_dim_array[3][4][5];

/* TYPE_STRING: String types */
typedef char *GTY((length("strlen($)"))) gty_string;
typedef const char *GTY((length("strlen($)"))) const_gty_string;

/* TYPE_CALLBACK: Callback function pointer type */
typedef void (*GTY(()) event_callback)(void *GTY(()), int GTY(()));

/* Struct containing callback */
struct GTY(()) callback_container {
    event_callback GTY(()) callback;
    void *GTY(()) user_data;
    int GTY(()) callback_id;
};

/* Chain of structures for testing chain_next/chain_prev */
struct GTY(()) chain_struct {
    int GTY(()) id;
    char GTY(()) data[64];
    struct GTY(()) chain_struct *GTY((chain_next)) next;
    struct GTY(()) chain_struct *GTY((chain_prev)) prev;
};

/* Structure with desc (discriminator) field */
struct GTY(()) desc_struct {
    enum GTY(()) desc_type {
        DESC_A,
        DESC_B,
        DESC_C
    } GTY((desc("%0.type"))) type;
    
    union GTY(()) {
        struct GTY(()) {
            int GTY(()) a;
            int GTY(()) b;
        } desc_a;
        struct GTY(()) {
            double GTY(()) x;
            double GTY(()) y;
        } desc_b;
        struct GTY(()) {
            char *GTY((length("strlen($)"))) text;
        } desc_c;
    } GTY((desc("%0.type"))) u;
};

#endif /* TEST_TYPES_H */
