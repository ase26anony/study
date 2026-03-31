  struct S {
      unsigned int bf : 4;  // bitfield
  } s;
  unsigned int *p = &s.bf;  // ERROR: cannot take address of bitfield
