#ifndef TYPE_ZOO_H
#define TYPE_ZOO_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* ========== SCALAR TYPES ========== */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern long long global_longlong;
extern _Bool global_bool;

/* ========== STRING TYPE ========== */
extern char *global_string;

/* ========== STRUCT TYPES ========== */
struct SimpleStruct {
    int a;
    float b;
    char c;
};

struct ComplexStruct {
    int *ptr;
    double data[4];
    struct SimpleStruct nested;
};

/* ========== USER STRUCT TYPES (typedef'd) ========== */
typedef struct {
    int x;
    int y;
    char name[32];
} Point;

typedef struct Node {
    int value;
    struct Node *next;
} ListNode;

/* ========== UNION TYPES ========== */
union DataUnion {
    int i;
    float f;
    char str[16];
};

typedef union {
    long long ll;
    double d;
    void *ptr;
} BigUnion;

/* ========== POINTER TYPES ========== */
extern int *int_ptr;
extern int **int_double_ptr;
extern struct SimpleStruct *struct_ptr;
extern union DataUnion *union_ptr;
extern void (*func_ptr)(void);

/* ========== ARRAY TYPES ========== */
extern int int_array[10];
extern struct SimpleStruct struct_array[5];
extern int *ptr_array[8];
extern double multi_dim_array[3][4][5];

/* ========== CALLBACK TYPES ========== */
typedef int (*Comparator)(const void *, const void *);
typedef void (*EventHandler)(int event_id, void *data);

/* ========== LANGUAGE-SPECIFIC STRUCTS ========== */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

#ifdef __GNUC__
struct __attribute__((transparent_union)) TransparentUnion {
    int *ptr;
    long *lptr;
};
#endif

/* Function declarations */
void init_types(void);
void use_all_types(void);
int compute_checksum(void);

#endif /* TYPE_ZOO_H */
