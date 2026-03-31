#ifndef TEST_GTY_H
#define TEST_GTY_H

#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointer) */
struct GTY(()) StructWithParens {
  int value;
  /* This will trigger case '(': */
  int (*callback)(int, char*);
  /* More parentheses in bitfield */
  unsigned int bits: (sizeof(int) * 8 - 1);
};

/* Test case 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
  /* This will trigger case '[': */
  int fixed_array[10];
  char variable_array[];
  /* Multi-dimensional array */
  double matrix[5][5];
};

/* Test case 3: Struct with braces (nested anonymous union) */
struct GTY(()) StructWithBraces {
  int tag;
  /* This will trigger case '{': */
  union {
    int as_int;
    float as_float;
    char* as_string;
  } data;
  /* Another nested struct */
  struct {
    int x;
    int y;
  } point;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
  /* Parentheses in function pointer */
  void (*init_func)(struct ComplexType*);
  
  /* Brackets in array declaration */
  struct StructWithParens* items[100];
  
  /* Braces for nested anonymous struct */
  struct {
    /* Nested parentheses */
    int (*comparator)(const void*, const void*);
    /* Nested brackets */
    int scores[];
  } metadata;
  
  /* Function pointer returning pointer to array */
  int (*(*complex_func)(int))[10];
};

/* Test case 5: Even more complex nested case */
typedef struct GTY(()) TreeNode {
  int value;
  /* Self-referential pointer with array */
  struct TreeNode* GTY((length("%h.child_count"))) children[];
  
  /* Union with all bracket types */
  union {
    /* Parentheses in function pointer type */
    void (*action)(void);
    /* Brackets in array */
    int params[5];
    /* Braces for anonymous struct */
    struct {
      int start;
      int end;
    } range;
  } extra;
  
  /* Bitfield with parentheses */
  unsigned int child_count: (sizeof(size_t) * 8);
} TreeNode;

#endif /* TEST_GTY_H */
