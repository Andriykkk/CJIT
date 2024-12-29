# Cjit: A Just-In-Time Compiled C-Like Language

I am trying to create a language similar to C but with the added Just-In-Time (JIT) compilation. I'll try to concentrate on perfomance, but first of all I'm trying to create something that at least will work.

## Roadmap
- [X] Lexer (created not full lexer, but it has quite good lexer that easily to expand)
- [ ] Parser (working right now)
- [ ] Syntax Checker (second pass throught ast)
- [ ] Intermediate Representation (IR)
- [ ] JIT Execution

## Example
Currently, Cjit supports only simple declarations like the following(without brackets):
```bash
int x = 1 + 2 * 3.43 / 54;
```

### Example of Generated AST:
```bash
Variable Declaration: Var: x
        Binary: +
        Left operand:
                Literal: Int: 1
        Right operand:
                Binary: /
                Left operand:
                        Binary: *
                        Left operand:
                                Literal: Int: 2
                        Right operand:
                                Literal: Float: 3.43
                Right operand:
                        Literal: Int: 54
```