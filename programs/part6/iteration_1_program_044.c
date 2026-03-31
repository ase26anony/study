// Before
int a = *p;
p = p + 1;

// After (with auto-increment)
int a = *p++;
