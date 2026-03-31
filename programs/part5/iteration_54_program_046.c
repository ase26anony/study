Original:        Proposed:
  JUMP L1          next_trial  ← moved to delay slot
  (delay slot)     JUMP L1
L1:              L1:
  next_trial       (rest of code)
  (rest of code)
