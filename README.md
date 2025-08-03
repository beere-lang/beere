# Beere

Beere is a simple, fast, and low-level programming language, designed for systems programming with modern syntax and manual memory control.

> ⚙️ Inspired by C, Java, Go and C++, but with better readability, more safety, easier to develop projects, and optional object-oriented programming.

## Features (that beere will have in the **first** release)

- Fast native compilation to x86_64 Assembly  
- Manual memory management with safety checks  
- Simple and clean syntax
- Optional object-oriented features
- C interoperability (`extern "C" fn`)  
- Module system
- Full package manager system

## Downloading dependencies
```
beerec install [dependency]
```

```ts
import test as dep

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

## Status

> ⚠️ Beere is under active development.

## Development
> Created only by jerious1337.

## License
> BSD 3-Clause License. [Read more](https://github.com/beere-lang/beere?tab=BSD-3-Clause-1-ov-file)
