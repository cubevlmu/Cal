## 🧩 `parseAssignExpr()` Function Requirements Table

| **Category**         | **Feature**       | **Syntax Example**                          | **AST Output**                                                                 | **Required?**     | **Remarks / Implementation Points**       |                            |
| -------------------- | ----------------- | ------------------------------------------- | ------------------------------------------------------------------------------ | ----------------- | ----------------------------------------- | -------------------------- |
| **1️⃣ Core Logic**      | Call Lower-Level Expression Parser | `a + b * c`                                 | Directly return the lower-level expression result                                                | ✅ Required        | Usually starts from `parseLogicalOrExpr()` |                            |
|                      | Detect Assignment Operators      | `=, +=, -=, *=, /=, %=, <<=, >>=, &=, \|=, ^=` | Check TokenType                                                                 | ✅ Required        | Use `isAssignOperator()` helper function  |                            |
| **2️⃣ Simple Assignment** | Normal Assignment | `a = 10`                                    | `BinaryExpr('=', Ident(a), IntLit(10))`                                        | ✅ Required        | Right-associative, allows chaining        |                            |
|                      | Chained Assignment| `a = b = 1`                                 | Nested `BinaryExpr`                                                              | ✅ Required        | `a = (b = 1)`                             |                            |
| **3️⃣ Compound Assignment** | Arithmetic Compound Assignment | `a += 10`, `b -= 2`, `c *= 3`, `d /= 4` | `BinaryExpr('+=', Ident(a), IntLit(10))`                                       | ✅ Required        | Semantically equivalent to `a = a + 10`   |                            |
|                      | Bitwise Compound Assignment      | `a <<= 1`, `b >>= 2`, `c &= 3`, `d \|= 4`, `e ^= 5` | Same as above                                                                  | ✅ Required        | Processed uniformly with arithmetic assignments |                            |
| **4️⃣ Error Detection**  | LValue Validation | `10 = a;`                                   | ❌ Error                                                                        | ✅ Required        | Left side must be an LValue (variable, field, array element) |                            |
|                      | Missing RValue    | `a = ;`                                     | ❌ Error                                                                        | ✅ Required        | Error if right-hand side is empty          |                            |
|                      | Unsupported Assignment Operator | `a **= 2`                                   | ❌ Error                                                                        | ✅ Required        | Error if `isAssignOperator()` doesn't recognize |                            |
| **5️⃣ Operator Precedence** | Correct Right Associativity | `a = b = c` → `a = (b = c)`           | AST Nesting                                                                      | ✅ Required        | Implemented via recursive self-call       |                            |
|                      | No Conflict with Logical/Arithmetic | `a = b + c * d`                       | `BinaryExpr('=', Ident(a), BinaryExpr('+', b, BinaryExpr('*', c, d)))`          | ✅ Required        | Parse left side starting from logical OR  |                            |
| **6️⃣ Error Recovery (Optional)** | Recover to Semicolon/Newline | `a =` + `<EOF>`                       | ❌ Error and Recover                                                              | ⚙️ Optional       | Improves error friendliness               |                            |
| **7️⃣ Extended Features (Optional)** | Support Destructuring Assignment | `(x, y) = func();`                    | `DestructuringAssignExpr`                                                        | 🚫 Not Required Now | Can be extended in the future             |                            |
|                      | Support Compound Expressions     | `a = b = c + d`                       | Nested BinaryExpr                                                                | ✅ Required        | Already included in right-associative semantics |                            |
| **8️⃣ Debugging & Logging (Optional)** | Print Parse Trace | —                                       | —                                                                              | ⚙️ Optional       | Helps locate issues during debugging      |                            |

---

## ✅ Core Responsibilities Summary (Concise Version)

| **Category** | **Responsibility**                              |
| ------------ | ----------------------------------------------- |
| **Syntax Level** | Top of the expression tree, handles all assignment-related operations |
| **Associativity** | Right-associative (`a = b = c` → `a = (b = c)`)    |
| **Sub-Parsing** | Calls next level `parseLogicalOrExpr()` to get left operand |
| **Error Detection** | Checks LValue validity and RValue existence          |
| **Semantic Output** | Generates `BinaryExpr` or `AssignExpr` node     |
| **Extension Points** | Can support destructuring assignment, ternary expressions, chained assignment |

---

## 💡 Suggested AST Node Design for Future Reference

| **Node Type**              | **Description**                       | **Field Example**                      |
| -------------------------- | ------------------------------------ | -------------------------------------- |
| `BinaryExpr`               | Generic binary operator (includes assignment) | `{ op, lhs, rhs }`                     |
| `AssignExpr` *(Optional)*    | Dedicated assignment node, distinct from generic binary operations | `{ target, value, compoundOp }`        |
| `LValueExpr` *(Abstract Base Class)* | All assignable expressions (Ident, Field, ArrayElem) | —                                    |