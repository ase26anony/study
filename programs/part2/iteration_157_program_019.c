%{
#include "test_types.h"
#include "gtype.h"
%}

struct GTY(()) marked_struct {
  int field;
  tree GTY((skip)) optional_tree_field;  // A GCC tree node, skipped by GC
};

%%
/* Grammar rules would follow in a .y file */
