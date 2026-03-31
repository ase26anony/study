Original:                    Optimized:
  BRANCH (insn)               BRANCH (insn) 
  JUMP to X (trial)    →      INSTRUCTION X (moved here)
  ... (other code)            ... (other code)
X:                           X: (now unreachable or removed)
  INSTRUCTION (next_trial)    ...
