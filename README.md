# Lurje Programming Language
Lurje is a compiled programming language designed with a slightly more modern syntax. The Lurje compiler is implemented in C and generates assembly code.

## 🚀 Features
Modern and readable syntax inspired by classic languages

Compiled to assembly

Compiler written in C

## 🔧 Status
This project is in early development.

## 📄 Syntax
```rust
fn foo(bar: int): int

fn main(): void
{
    let x: int = 0

    while (x < 100)
    {
       x++
    }

    switch (x)
    {
        case 100:
            x = 100
            break
        case 0:
            break
        default:
            x = 9
            break
    }

    const y: int = foo(10) // comment test
}

fn foo(bar: int): int
{
    return bar + 1 // bar + 1
}
```
