## The challenge:

The compiler likely **cannot** move `a = 1` into the delay slot because:
- It's not safe on the fall-through path (would incorrectly set `a = 1`)
- The volatile access creates a compiler barrier
- The instruction has side effects (modifies `a`)

## Better delay slot candidate:

For a true delay slot candidate, you'd want an instruction that:
1. Executes safely on both branch paths
2. Doesn't depend on the branch condition
3. Isn't affected by volatile operations

Example of a good delay slot candidate:
