### Unary Expression Parser

| Feature               | Implemented | Notes                      |
|---------------------|-------------|----------------------------|
| Plus `+x`           | ✅           | Directly passes through    |
| Minus `-x`          | ✅           | `UnaryOp::kMinus`          |
| Increment/Decrement `++x / --x` | ✅           | Can extend to prefix/postfix |
| Logical Not `!x`    | ✅           | `UnaryOp::kLogicalNot`     |
| Bitwise Not `~x`    | ✅           | `UnaryOp::kBitNot`         |
| Address Of `&x`     | ✅           | `UnaryOp::kAddrOf`         |
| Dereference `*x`    | ✅           | `UnaryOp::kDeref`          |
| Parenthesized Expression `(expr)` | ✅           | Calls back to primary      |
| Type Cast `(T)x`    | ✅           | Calls `parseType()`        |
| sizeof / alignof    | ⚙️ TODO     | Similar handling to cast   |