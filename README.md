# Beere Programming Language

Beere is a simple, fast, and low-level programming language, designed for systems programming with modern syntax and manual memory control.

> ⚙️ Inspired by C, Java, Go and C++, but with better readability, more safety, easier to develop projects, and optional object-oriented programming.

## Features (that beere will have in the **first** release)

- Fast native compilation to x86_64 Assembly  
- Manual memory management (Optional GC soon)
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

## Object Oriented
```ts
class Parent {
    public field_2: int = 10

    public Parent(n1: int) {
        this.field_2 = n1
    }
}

class Child extends Parent {
    public n1: int = 0

    public Child(n1: int) {
        super(n1)
        this.n1 = n1
    }
}

fn main() {
    let child: Child = new Child(10)
}
```

## C Language Interoperability
```rs
extern "C" fn foo(arg: string)

fn main() {
    // Strings structures in Beere are different from C, so,
    // you need to convert to C style strings to work on C methods
    foo("Teste".to_cstr())
}
```

## Building
To compile a Beere file:

```
beerec [entry-point-file] [dot-mod-file]
```
(Compiler will generate raw Assembly and link it automatically)

## Status

> ⚠️ Beere is under active development.

## Progress
> Check the `beerec/roadmap/TODO-LIST.md` file to more info.

## Development
> Created only by jerious1337.

## License
> BSD 3-Clause License. [Read more](https://github.com/beere-lang/beere?tab=BSD-3-Clause-1-ov-file)
