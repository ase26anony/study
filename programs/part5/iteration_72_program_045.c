#ifndef GTY_TYPES_H
#define GTY_TYPES_H

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
    const char * GTY((skip)) str;
    char * GTY((skip)) mutable_str;
};

/* Simple structure */
struct GTY(()) my_struct {
    int a;
    char * GTY((skip)) b;
    struct scalar_box GTY((skip)) box;
};

/* User-defined struct type via typedef */
typedef struct my_struct GTY(()) my_struct_t;

/* Union type */
union GTY(()) my_union {
    int i;
    float f;
    void * GTY((skip)) p;
    struct my_struct GTY((skip)) s;
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

/* Multi-dimensional array */
struct GTY(()) matrix {
    int GTY((skip)) data[3][3];
    float GTY((skip)) fdata[2][4];
};

/* Callback/function pointer type */
typedef void (*GTY(()) callback_fn)(int, void * GTY((skip)));
typedef int (*GTY(()) compare_fn)(const void *, const void *);

/* Structure containing callback */
struct GTY(()) callback_container {
    callback_fn GTY((skip)) handler;
    compare_fn GTY((skip)) compar;
    void * GTY((skip)) user_data;
};

/* Complex nested structure */
struct GTY(()) outer_struct {
    struct my_struct GTY((skip)) inner;
    union my_union GTY((skip)) choice;
    struct list_node * GTY((skip)) head;
    int_array GTY((skip)) numbers;
    struct matrix GTY((skip)) mat;
};

/* Self-referential union */
union GTY(()) tree_node {
    struct GTY(()) {
        int type;
        union tree_node * GTY((skip)) left;
        union tree_node * GTY((skip)) right;
    } binary;
    struct GTY(()) {
        int type;
        char * GTY((skip)) value;
    } leaf;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_OPTIONS
struct GTY((chain_next, chain_prev)) chained_struct {
    int id;
    struct chained_struct * GTY((skip)) next;
    struct chained_struct * GTY((skip)) prev;
};
#else
struct chained_struct {
    int id;
    struct chained_struct * next;
    struct chained_struct * prev;
};
#endif

/* Array with length specifier (simulated) */
struct GTY(()) variable_array {
    int length;
    int * GTY((skip)) data;  /* Would use (length) in real GCC */
};

/* For TYPE_LANG_STRUCT simulation - using a special naming pattern */
struct GTY(()) lang_decl {
    int lang_specific;
    void * GTY((skip)) lang_data;
};

struct GTY(()) lang_type {
    unsigned int lang_flag : 1;
    struct lang_decl * GTY((skip)) decl;
};

/* Opaque/undefined type forward declaration */
struct undefined_struct;
typedef struct undefined_struct GTY(()) undefined_t;

#ifdef __cplusplus
}
#endif

#endif /* GTY_TYPES_H */
