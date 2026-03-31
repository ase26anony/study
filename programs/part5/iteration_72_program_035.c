#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Basic scalar type in container */
struct GTY(()) scalar_box {
    long GTY((skip)) value;
    double GTY((skip)) dvalue;
};

/* String type */
struct GTY(()) string_container {
    const char * GTY((skip)) name;
    char * GTY((skip)) buffer;
};

/* Structure type */
struct GTY(()) my_struct {
    int a;
    char * GTY((skip)) b;
    struct scalar_box GTY((skip)) box;
};

/* Union type */
union GTY(()) my_union {
    int i;
    float f;
    void * GTY((skip)) p;
    struct my_struct * GTY((skip)) s;
};

/* Pointer types within structures (recursive) */
struct GTY(()) list_node {
    struct list_node * GTY((skip)) next;
    struct list_node * GTY((skip)) prev;
    int data;
    union my_union GTY((skip)) u;
};

/* Array type definition */
typedef int GTY(()) int_array[10];
typedef struct list_node * GTY(()) node_ptr_array[5];

/* User-defined struct type via typedef */
typedef struct my_struct GTY(()) my_struct_t;

/* Callback (function pointer) type */
typedef void (*GTY(()) callback_fn)(int, const char *);
typedef int (*GTY(()) compare_fn)(const void *, const void *);

/* Nested structure with union */
struct GTY(()) complex_container {
    struct my_struct GTY((skip)) base;
    union {
        int GTY((skip)) int_val;
        struct list_node * GTY((skip)) node_ptr;
    } GTY((tag("0"))) choice;
    int_array GTY((skip)) numbers;
};

/* Simulating lang_struct pattern - often used in GCC internals */
struct GTY(()) tree_lang_specific {
    struct GTY(()) lang_type {
        unsigned int align : 8;
        unsigned int has_align : 1;
    } GTY((skip)) type;
    void * GTY((skip)) lang_specific;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_OPTIONS
struct GTY((chain_next ("next"), chain_prev ("prev"))) chained_node {
    struct chained_node *next;
    struct chained_node *prev;
    int value;
};
#else
struct chained_node {
    struct chained_node *next;
    struct chained_node *prev;
    int value;
};
#endif

/* Array with length specification */
struct GTY(()) variable_array {
    int GTY((skip)) length;
    int GTY((length ("%h.length"))) items[1];
};

/* Multiple GTY options */
struct GTY((skip, reorder, desc ("1"))) optioned_struct {
    int x;
    int y;
    void * GTY((skip)) opaque;
};

extern struct my_struct GTY(()) global_struct;
extern union my_union GTY(()) global_union;

#ifdef __cplusplus
}
#endif

#endif /* GTY_TEST_TYPES_H */
