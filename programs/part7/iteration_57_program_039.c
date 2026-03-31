But the clobber list includes 64-bit registers (rax, rbx, etc.). This is inconsistent.

### 2. **Unnecessary register clobbers**
The clobber list includes registers that aren't actually modified:
- `rax`, `rbx`, `rcx`, `rdx` are listed but the `movl` instruction doesn't necessarily use them
- This forces the compiler to save/restore these registers unnecessarily

### 3. **Memory clobber may be excessive**
`"memory"` clobber tells the compiler that memory may be modified, which forces all memory values to be reloaded. This is likely unnecessary for a simple load operation.

### 4. **Missing input constraint**
The `complex_index` calculation isn't shown, but if it involves computation, it should be properly constrained.

## Better alternatives:

### Option 1: Let the compiler handle it (recommended)
