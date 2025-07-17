<div align="center">
  <picture>
    <source srcset="https://raw.githubusercontent.com/Adheroad/Crox/main/crox_flipped.svg" type="image/svg+xml" />
    <img src="https://raw.githubusercontent.com/Adheroad/Crox/main/crox_flipped.png" alt="Crox Logo" width="24%" height="25%" />
  </picture>
  <picture>
    <source srcset="https://raw.githubusercontent.com/Adheroad/Crox/main/crox.svg" type="image/svg+xml" />
    <img src="https://raw.githubusercontent.com/Adheroad/Crox/main/crox.png" alt="Crox Logo" width="24%" height="25%" />
  </picture>
   <picture>
    <source srcset="https://raw.githubusercontent.com/Adheroad/Crox/main/crox_flipped.svg" type="image/svg+xml" />
    <img src="https://raw.githubusercontent.com/Adheroad/Crox/main/crox_flipped.png" alt="Crox Logo" width="24%" height="25%" />
  </picture>
  <picture>
    <source srcset="https://raw.githubusercontent.com/Adheroad/Crox/main/crox.svg" type="image/svg+xml" />
    <img src="https://raw.githubusercontent.com/Adheroad/Crox/main/crox.png" alt="Crox Logo" width="24%" height="25%" />
  </picture>
    
</div>

# Crox

**A tiny Lisp-like language in C**

_“A terribly useless language to solve none of your problems.”_  
~Anshul Dhawan

---

## Table of Contents

1. [Philosophy and Motives](#philosophy-and-motives)
2. [Prerequisites](#prerequisites)
3. [Installation & Usage](#installation--usage)
   - [Clone & Build](#clone--build)
   - [Demo](#demo)
4. [Terminology](#terminology)
5. [Project Layout](#project-layout)
6. [Future updates](#future-updates)
7. [Contributing](#contributing)

---

## Philosophy and Motives

Crox was developed for a variety of reasons—some stupid, some pathetic, and some astoundingly genius. To display only the golden side would be to betray the core principles of the creator’s life and the project’s raison d’être. Integrity is valued above all. Collaboration follows. Service comes third.
With this attempt, the creator not only wanted to learn more about languages but also to provide a frame for what can be done outside the boundaries of what one has been taught or told to learn.
A full list of misguided, pathetic, and occasionally brilliant motivations follows:

1. To embark on a “real big project” and understand how languages work.
2. To create everything from scratch—you must first simulate the universe (because why not?).
3. As a tribute to loved ones.
4. To let a stranger know this language was written with love. The sole reason it exists is because it was written for you.
5. To chase the dream of a “perfect” language (spoiler: failure was spectacular).

---

## Prerequisites

Before invoking Crox, one must satisfy the following prerequisites:

- **C Compiler**
- **GNU Readline** (for optional line‑editing capabilities)
- **make** (build tool)

as it has been tested on Fedora linux this command was necessary:

```bash
sudo dnf install gcc readline-devel make
```

---

## Installation & Usage

### Clone & Build:

1. Clone the repo:
   `git clone https://github.com/Adheroad/Crox.git`
2. Get inside the folder
   `cd Crox`
3. Build and run the interpreter:
   `make run`

### Demo:

```
$ make run
Crox -> 1 + 2 ^ 3 ^ 2 // infix notation
513

Crox -> [+ 2 3 4] //S-Expression inside [] following polish notation
9

Crox -> {1 2 3 4} //Q-Expression inside {} Code as data follows polish notation
{1 2 3 4}

Crox -> join {1 2} {3 4}
{1 2 3 4}

Crox -> [* 3 [eval {+1 2}]]
9

Crox -> { 1 2 [* 3 4]}
{1 2 [* 3 4]}

Crox -> join {1 2} [head [init [join {[eval {+ 1 2 3 4}]} {2 3}]]]
{1 2 2}
```

---

## Terminology

`ari` is the core datatype (an `Atomic Runtime Instance` for public). It represents all Lisp values.
`Crox` is a funny name whose origins are forgotten. but it still stands out and is why remains as is.

---

## Project Layout

### A walk around the codebase

The code is divided into three subfolders: include, lib, and src.

- lib/ contains mpc.h and mpc.c from [orangeduck](https://github.com/orangeduck/mpc). Thank you. These files are under the BSD license, so they were used with creative freedom.
- include/ contains all header files. All declared functions live here. Walking through this folder will give the reader a sense of Crox’s capabilities.
- src/ provides implementations for those headers.

##### Breakdown of source files:

1. `repl.c` — repl.c contains main logic
2. `ari.h` / `ari.c` — defines the ari datatype and its operations
3. `eval.c` / `eval.h` — evaluation logic for expressions
4. `parser.c` / `parser.h` — defines and cleans up language parsers
5. `io.c` / `io.h` — handles reading and printing

---

## Future Updates

- [x] basic repl
- [x] Parsers and error handling
- [x] Infix notation
- [x] Polish notation
- [x] S-Expressions
- [x] Q-Expressions (code as data)
- [ ] variables
- [ ] functions
- [ ] conditionals
- [ ] strings
- [ ] many more

---

## Contributing

Spot an error? Feel free to open an issue or submit a pull request.
Disagree with a design choice? Feedback is welcome even without a proposed fix.
