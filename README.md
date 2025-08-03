# Beere

Beere is a simple, fast, and low-level programming language, designed for systems programming with modern syntax and manual memory control.

> ⚙️ Inspired by C, Java, Go and C++, but with better readability, more safety, easier to develop projects, and optional object-oriented programming.

## Features (that beere will have in the **first** release)

- Fast native compilation to x86_64 Assembly  
- Manual memory management with safety checks  
- Simple and clean syntax
- Optional object-oriented features
- C interoperability (`extern "C" fn`)
- Optimizations (inspired in LLVM)
- Module system
- Full package manager system

## Downloading Dependencies
```
beerec install [dependency]
```

```ts
import [dependency] as dep

fn main() {
    dep.foo(10)
}
```

## Hello World

```rs
fn main() {
    print("Hello, World!")
}
```

## Building
To compile a Beere file:

```
beerec [entry-point-file] [dot-mod-file]
```
(Compiler will generate raw Assembly and link it automatically)

## C Language Interoperability
```rs
extern "C" fn foo(arg: string)

fn main() {
    // Strings structures in Beere are different from C, so,
    // you need to convert to C style strings to work on C methods
    foo("Teste".to_cstr())
}
```

## Status

> ⚠️ Beere is under active development.

## Development
> Created only by jerious1337.

## License
> BSD 3-Clause License. [Read more](https://github.com/beere-lang/beere?tab=BSD-3-Clause-1-ov-file)
