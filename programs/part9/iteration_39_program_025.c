Entry
  ↓
  i₀ = 0
  ↓
Loop Header: i₁ = φ(i₀, i₂)  ← Phi node merges initial and updated values
  ↓
  if (i₁ < n)
  ↓ (true)
  if (i₁ == 0)  ← This uses the Phi node value i₁
  ↓ (true)
  // body
  ↓
  i₂ = i₁ + 1   ← New SSA name for incremented value
  ↓
  (back to Loop Header)
