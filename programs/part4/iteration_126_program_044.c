#ifndef TEST_GTY_PARSER_H
#define TEST_GTY_PARSER_H

#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointer) */
struct GTY(()) StructWithParens {
  int value;
  /* This will trigger case '(': consume_balanced('(', ')') */
  int (*callback)(int, char*);
  unsigned int bits: (sizeof(int) * 8 - 1);  /* More parentheses */
};

/* Test case 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
  /* This will trigger case '[': consume_balanced('[', ']') */
  int fixed_array[10];
  char variable_array[];
  int (*array_of_pointers[5])(void);
  int multi_dim[3][4];
};

/* Test case 3: Struct with braces (nested anonymous union) */
struct GTY(()) StructWithBraces {
  int id;
  /* This will trigger case '{': consume_balanced('{', '}') */
  union {
    int as_int;
    float as_float;
    void* as_ptr;
  } data;
  struct {
    int x;
    int y;
  } point;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
  /* Parentheses in function pointer type */
  void (*init_func)(struct ComplexType*);
  
  /* Brackets in array declaration */
  struct StructWithParens* items[20];
  
  /* Braces in nested struct definition */
  struct {
    /* Nested parentheses in bitfield */
    unsigned int flags: (8);
    
    /* Nested brackets in flexible array member */
    char name[];
  } header;
  
  /* Multi-dimensional array with parentheses in size expression */
  double matrix[3][(sizeof(double) * 8)];
};

/* Test case 5: Pointer to array of function pointers */
typedef int GTY(()) (*func_ptr_t)(int);
struct GTY(()) Container {
  /* Complex type: pointer to array of function pointers */
  func_ptr_t (*get_callbacks(void))[10];
  
  /* Anonymous struct with all bracket types */
  struct {
    int (*compare)(const void*, const void*);
    char keys[][50];
    union {
      long l;
      double d;
    } value;
  } GTY((skip)) metadata;  /* GTY annotation with parentheses */
};

/* Test case 6: Even more complex nested case */
struct GTY(()) SuperComplex {
  /* Array of pointers to functions returning pointers to arrays */
  char* (*(*func_table[5])(int))[];
  
  /* Struct containing union containing struct... */
  struct {
    union {
      struct {
        int (*handler)(int, char*[]);
        void* data[100];
      } s;
      long long big_num;
    } u;
  } nested;
};

#endif /* TEST_GTY_PARSER_H */
