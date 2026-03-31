- `movl` is for 32-bit operations, but you're clobbering 64-bit registers (`rax`, `rbx`, etc.)
- On x86-64, this will zero-extend to 64 bits, which might not be what you want

### 2. **Redundant clobber list**
