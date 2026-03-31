// Initial access
int *p = &x;
int val = *p;  // This would match the mem_insn setup

// Then looking for
p++;  // Or similar increment to optimize
