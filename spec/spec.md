# Oryn Programming Language

Oryn is a fast, modern programming language, with beautiful syntax and functionality at heart.

> **Status:** draft. Sections marked **(draft)** were inferred from the example programs rather than stated explicitly, and need a decision before the compiler relies on them. See [Open questions](#open-questions).

---

## 1. Lexical structure

### 1.1 Source files
- UTF-8 text, extension `.oryn` **(draft)**.
- Whitespace (space, tab, newline) separates tokens and is otherwise insignificant. Indentation has no meaning.
- Statements end with `;`. Blocks use `{ }`.

### 1.2 Comments
```
// line comment
```
Block comments are not yet defined **(draft)**.

### 1.3 Identifiers
`[A-Za-z_][A-Za-z0-9_]*`. A lone `_` is the discard identifier (see out parameters).

### 1.4 Keywords
```
module  const  var  auto  fn  return  if  else  while  for
struct  packed  class  singleton  virtual  abstract  override
overload  apply  patch  extern  new  move  super  exc  raw_ptr
true  false  void  bool  string  int  uint  float
```
`this` is implicit inside methods and is not a keyword you write **(draft)**.

### 1.5 Literals
| Kind | Examples | Notes |
|------|----------|-------|
| Integer | `42`, `-64` | Type comes from context; with no context see [Open questions](#open-questions) |
| Float | `2.0`, `3.14159` | |
| Width-suffixed | `3.14159!64`, `7!32` | `!N` after a literal fixes its width, same notation as types |
| Bool | `true`, `false` | |
| String | `"shapes"` | Type `string` |

### 1.6 Operators and punctuation
```
+  -  *  /  %        arithmetic
== != < <= > >=      comparison
=  += -= *= /=       assignment
=>                   pure marker
->                   out argument / out parameter
<>                   inout argument / inout parameter
~                    "iterate over" in for-loops
&                    hard-block marker (prefix)
@                    shared reference (suffix on type)
exc@                 exclusive reference
*                    owning pointer (suffix on type)
!                    bit width (in types and literals); destructor prefix
:                    return type
.                    member access and module qualification
```

**Lexer notes**
- `!` has three roles: width (`int!32`), destructor name (`!Shape()`), and the `!=` operator. The lexer must tokenise `!=` first, and treat `!` directly attached to a type keyword or literal as a width.
- `&` as a prefix means hard-block. If binary `&` (bitwise and) is ever added, the parser must distinguish by position.
- `<>` is a single token (inout). In `raw_ptr<Circle>` the angle brackets are separate `<` and `>` tokens, so nested generics such as `raw_ptr<raw_ptr<T>>` need `>>` split by the parser **(draft)**.
- `*` is both multiplication and the owning-pointer suffix on a type. The parser resolves it by context: after a type name in a declaration it is a pointer.

---

## 2. Types

### 2.1 Primitive types
| Type | Meaning | Default value |
|------|---------|---------------|
| `int!N` | signed integer, N value bits | `0` |
| `uint!N` | unsigned integer, N value bits | `0` |
| `float!N` | IEEE float, N = 32 or 64 | `0.0` |
| `bool` | boolean | `false` |
| `string` | text | `""` |
| `void` | no value (return type only) | n/a |

`!N` is the number of **value bits**. Storage is rounded up to whole bytes.

| Type | Range | Storage |
|------|-------|---------|
| `uint!7` | 0..127 | 8 bits |
| `int!7` | -64..63 | 8 bits |
| `int!32` | -2^31..2^31-1 | 32 bits |

### 2.2 Conversions
- Widening conversions are implicit **(draft)**.
- **Narrowing needs an explicit cast**: `int!32(f)` where `f` is `float!64`.
- The result of `en(items)` yields an index of type `uint!64`, so `int!32(idx)` is required before mixing it with `int!32`.

### 2.3 Arrays and containers
| Type | Meaning |
|------|---------|
| `T[N]` | fixed-size array, N zeros by default |
| `T[]` | dynamic array, empty by default |

- Indexing `items[i]` is **bounds-checked**; out of range **traps**.
- `items.get(i, -> ok)` never traps: it returns the default value and sets `ok = false`.
- `items.push(x)` grows the container; the container owns its storage.
- `&items.at_unchecked(i)` skips the bounds check and must be hard-blocked.

### 2.4 Pointers and references
| Syntax | Meaning |
|--------|---------|
| `T*` | owning (unique) pointer |
| `T@` | shared reference, read-only |
| `T exc@` | exclusive reference, read-write |
| `raw_ptr<T>` | raw pointer, only dereferenceable in hard-blocked code |

See [Ownership and borrowing](#5-ownership-and-borrowing).

---

## 3. Declarations

### 3.1 Variables
Variables are declared with `var`, a type and a name.

`var int!32 count;`

A value can be given at the point of declaration.

`var int!32 count = 10;`

Variables can also have their type inferred with `auto`.

`var auto test = "hi!";`

**Forced initialisation:** every variable is initialised. With no initialiser it gets the type's default value.

```
var int!32 i;                // 0
var float!64 f;             // 0.0
var bool b;                 // false
var string s;               // ""
var int!32[4] fixed;        // four zeros
var int!32[] dyn;           // empty
var Circle c;               // default Circle (radius 1.0, name "circle")
var Circle* p;              // empty owning pointer
var Shape* sh;              // empty owning pointer to an abstract type
// var Shape sh;            // ERROR: cannot instantiate an abstract class
```

### 3.2 Constants and globals
Globals must be `const`. Mutable globals do not exist.

```
const int!32 scale = 3;
const auto label = "shapes";   // inferred as string
```

Pure functions may read `const` globals.

### 3.3 Modules
```
module geometry {
    // declarations
}
```
Names are qualified with `.`: `geometry.clamp(a, 0, 10)`.

---

## 4. Functions

General shape: `fn name(params): ReturnType body`.

### 4.1 Normal functions
Normal functions are impure. They may use block bodies, mutate state and call anything.

```
const int!32 count = 10;

fn calculate(int!32 value): int!32 {
    return value * count;
}
```

### 4.2 Flat functions
Flat functions have no braces. Their body is exactly one statement.

```
fn calculatedouble(int!32 value): int!32 return value * count * 2;
```

Also allowed, with the statement on the next line:

```
fn calculatedouble(int!32 value): int!32
    return value * count * 2;
```

A flat function returning `void` omits `return`: `fn bump(int!32 <> n): void n += 1;`

### 4.3 Pure functions
Pure functions put `=>` directly after the declaration. They can be flat or normal.

```
fn doubled(int!32 v): int!32 => v * 2;           // flat, pure

fn sum_to(int!32 n): int!32 => {                  // block, pure
    var int!32 total;
    for (var int!32 i = 1; i <= n; i += 1) {
        total += i;
    }
    return total;
}
```

**Purity rules (checked by the compiler):**
- May read: parameters, locals, `const` globals.
- May write: locals only.
- May call: other pure functions only.
- May not call hard-blocked (`&`) code or extern functions.

### 4.4 Overloading
Functions overload by **input type or count**, with no keyword.

```
fn half(float!64 v): float!64 => v / 2.0;
fn half(int!32 v): int!32 => v / 2;
```

The `overload` keyword is required only when you declare a function with the **same name, inputs and return type** as an existing one, typically from another module to replace or extend it. The original needs no keyword.

```
overload fn calculate(int!32 value, int!32 value2): float!64 => value * value2 * 1.04!64;
```

Out parameters are **not part of the signature** (see below), so an overload that differs only in outs still counts as "same inputs and return type" and needs `overload`.

### 4.5 Out parameters (`->`)
- Declared `bool -> ok`.
- Optional at the call site, write-only inside the function.
- Not part of the signature: there is exactly one `parse(string)`, and callers choose how many outs to take.
- If the caller omits an out, writes to it are discarded.

```
fn parse(string s, bool -> ok, int!32 -> consumed): int!32 { ... }

var int!32 a = parse("42");                 // no outs
var int!32 b = parse("42", -> ok);          // first out
var int!32 c = parse("42", -> ok, -> n);    // both outs
var int!32 d = parse("42", -> _, -> n);     // skip first, take second
```

Outs are positional. To take the second you must write `-> _` for the first.

### 4.6 Inout parameters (`<>`)
Read-write access to the caller's variable. Must be marked at the call site.

```
fn swap(int!32 <> a, int!32 <> b): void { ... }
swap(<> a, <> b);
```

### 4.7 Call-site aliasing rules
These are compile errors:
```
swap(<> a, <> a);                 // same variable passed twice, once as inout
parse("42", -> n, -> n);          // same variable passed to two outs
flow(<> items, <> items[0]);      // an element passed alongside its container
```

### 4.8 Parameters without a marker
A plain parameter (`int!32 v`, `string s`) is a **borrow**: the callee gets read access and copies if it stores the value. A parameter of owning-pointer type (`Circle* owned`) **takes ownership**; the caller must `move(...)` into it.

---

## 5. Ownership and borrowing

### 5.1 Owning pointers
```
var Circle* c = new Circle(2.0);   // unique owner
var Circle* d = move(c);           // c is now empty
// d is destroyed at end of scope
```
- `move(x)` transfers ownership and leaves `x` empty.
- An owning pointer is destroyed when it goes out of scope, unless moved.
- Passing to a function that takes `T*` requires `move`: `take(move(c));`.

### 5.2 References
```
var int!32 x = 5;
var int!32@ view = x;        // shared, read-only
var int!32 exc@ ref = x;     // exclusive, read-write
ref += 1;
```

### 5.3 Borrow rules
- Any number of shared borrows **or** one exclusive borrow of a value, never both.
- Borrows end at their **last use** (non-lexical): the borrow checker works on liveness, not scope.
- While a reference to an element is live, the container cannot be mutated.

```
var int!32@ first = items[0];   // borrows items
// items.push(2);                // ERROR: first is still live
var int!32 c = first + 1;        // last use of first
items.push(2);                   // fine now
```

### 5.4 Method receivers
- Pure methods (`=>`) borrow `this` **shared**.
- Impure methods borrow `this` **exclusively** for the call.
- `singleton` methods follow the same rule against the singleton's state.

### 5.5 Raw pointers
- `d.raw()` produces a `raw_ptr<T>`; creating one is safe.
- Dereferencing it or calling functions that take one needs hard-blocked code.

---

## 6. Hard-blocked code (`&`)

Anything that skips a safety guarantee is "hard-blocked" and must be visibly marked.

| Form | Meaning |
|------|---------|
| `&fn f(...) { ... }` | whole function body is a hard-blocked scope |
| `&{ ... }` | hard-blocked scope |
| `&call(...)` | one hard-blocked call |

```
&fn read_radius(raw_ptr<Circle> p): float!64 {
    return p.radius;
}

var float!64 rad = &read_radius(r);        // call to a hard-blocked fn

&{
    var float!64 again = r.radius;          // no extra markers needed inside
}
```

- Inside a hard-blocked scope, no further markers are needed.
- Calling a hard-blocked function from safe code requires the `&` marker.
- Extern (C++) functions are implicitly `&fn`.

---

## 7. User-defined types

### 7.1 Struct
Value type. Fields default-initialise. Methods may be pure or impure.

```
struct Point {
    var float!64 x;
    var float!64 y;

    fn length(): float!64 => sqrt(x * x + y * y);
    fn scale_by(float!64 k): void { x *= k; y *= k; }
}
```

### 7.2 Packed struct
Fields are bit-packed in declaration order. Field widths use `!N`; the struct is the sum of its bits.

```
packed struct Header {
    var uint!3 version;
    var uint!5 flags;
    var uint!16 length;    // 24 bits total
}
```
Endianness and padding to a byte boundary are undefined **(draft)**.

### 7.3 Class
Reference type with inheritance, constructors and virtual dispatch.

```
class Shape {
    var string name = "shape";

    Shape(string n) { name = n; }
    !Shape() { }                              // destructor, implicitly virtual

    abstract fn area(): float!64;             // no body; makes Shape abstract
    virtual fn describe(): string => name;    // overridable, has a default body
}

class Circle : Shape {
    var float!64 radius = 1.0;

    Circle() { super("circle"); }
    Circle(float!64 r) { super("circle"); radius = r; }

    override fn area(): float!64 => 3.14159!64 * radius * radius;
}
```

Rules:
- A class with any `abstract` method is abstract and cannot be instantiated.
- `super(...)` must be the first statement of a constructor.
- `override` is required when replacing a `virtual` or `abstract` method.
- Destructors are `!ClassName()` and are implicitly virtual.

### 7.4 Singleton
One global instance, accessed by name. State defaults like any other variable.

```
singleton Logger {
    var int!32 lines;
    fn log(string msg): void { lines += 1; }
    fn count(): int!32 => lines;
}

Logger.log("done");    // safe, no marker needed
```

---

## 8. Control flow

```
while (i < 10) { i += 1; }

for (var int!32 j = 0; j < 10; j += 1) { total += j; }

for (var auto (idx, v) ~ en(items)) { ... }    // en() from the standard library
for (var int!32 x ~ items) { total += x; }     // explicit loop variable type
for (var auto exc@ item ~ items) { item *= 2; } // mutate in place
```

- `~` iterates a container. The loop variable is a borrow of the element.
- `exc@` on the loop variable makes the iteration borrow `items` exclusively, so a function containing it must take `items` as `<>`.
- `if` / `else` use parenthesised conditions and braces. `else if` chains **(draft)**.

---

## 9. Patching and overrides across modules

Used to fix or extend code you do not own.

```
module fixes {
    overload fn Circle.area(): float!64 => 3.14159265358979!64 * radius * radius;
}

fn patched_scope(): void {
    apply fixes Circle area;       // scoped: only calls in this block see it
    var float!64 a = c.area();     // uses fixes.Circle.area
}

patch fixes Circle area;           // global: every call in the program sees it
```

- `overload fn` is mandatory here because the signature matches the original.
- `apply` is lexically scoped. `patch` is program-wide.
- Name lookup order **(draft)**: `apply` in nearest enclosing block, then `patch`, then the original.

---

## 10. C++ interop

```
extern "C++" {
    fn cpp_add(int!32 a, int!32 b): int!32;
}

var int!32 sum = &cpp_add(1, 2);    // extern calls need the & marker
```

- Declarations inside `extern "C++"` have no body.
- They are implicitly `&fn`.
- Type mapping to C++ (`int!32` to `int32_t`, and so on) is defined in the compiler's ABI notes **(draft)**.

---

## 11. Compiler reference

### 11.1 Suggested pipeline
1. **Lex** to tokens (handle `!`, `<>`, `->`, `=>`, `exc@` as described above).
2. **Parse** to an AST.
3. **Resolve names**: modules, overload sets, `apply`/`patch`.
4. **Type check**: widths, casts, overload resolution, abstract instantiation.
5. **Purity check**: for every `=>` function.
6. **Initialisation**: insert default values for every declaration.
7. **Borrow check**: liveness-based shared/exclusive rules, move tracking, call-site aliasing.
8. **Safety marks**: ensure `raw_ptr` use and unchecked or extern calls are inside `&`.
9. **Lower** to an IR and emit code.

### 11.2 Diagnostics the spec requires
| Condition | Result |
|-----------|--------|
| Narrowing without a cast | error |
| Instantiating an abstract class | error |
| Pure function reads non-const global, writes non-local, or calls impure | error |
| Same variable passed twice with an inout/out involved | error |
| Container mutated while a borrow into it is live | error |
| Use of a moved-from owning pointer | error (or empty-pointer trap) **(draft)** |
| Raw pointer dereference or extern call outside `&` | error |
| Same-signature function without `overload` in another module | error |
| `overload` with no matching original | error |
| Out-of-range index | runtime trap |

### 11.3 Grammar sketch (EBNF, draft)
```
program     = { module | decl } ;
module      = "module" IDENT "{" { decl } "}" ;
decl        = const_decl | func | type_decl | extern_block | overload | patch ;

const_decl  = "const" ( type | "auto" ) IDENT "=" expr ";" ;
var_decl    = "var"   ( type | "auto" ) IDENT [ "=" expr ] ";" ;

func        = [ "&" ] [ "virtual" | "abstract" | "override" | "overload" ]
              "fn" qname "(" [ params ] ")" ":" type body ;
body        = block | "=>" ( expr ";" | block ) | stmt | ";" ;
params      = param { "," param } ;
param       = type [ "->" | "<>" ] IDENT ;      (* "->" and "<>" sit between type and name *)

type        = base_type [ "!" INT ] { "*" | "@" | "exc@" | "[" [ INT ] "]" } ;
base_type   = "int" | "uint" | "float" | "bool" | "string" | "void"
            | "raw_ptr" "<" type ">" | qname ;

stmt        = var_decl | expr ";" | "return" [ expr ] ";" | if | while | for | block
            | "&" block | apply ;
for         = "for" "(" ( var_for | in_for ) ")" stmt ;
in_for      = "var" ( type | "auto" ) [ "exc@" ] binding "~" expr ;
call_arg    = expr | "->" ( IDENT | "_" ) | "<>" expr ;
apply       = "apply" qname IDENT IDENT ";" ;
patch       = "patch" qname IDENT IDENT ";" ;
```

---

## Open questions

Decisions to make before the compiler depends on them.

1. **Untyped integer literals.** What is the type of `var auto x = 3;`? Pick a default (likely `int!32`) or require a width suffix.
2. **Inference scope.** Is `auto` allowed everywhere, or only on locals and loop variables (not globals, fields or signatures)?
3. **Implicit widening.** Is `int!7` to `int!32` implicit? What about `int` to `float`?
4. **Reference syntax inconsistency.** Shared is `T@` (suffix on the type) while exclusive is `T exc@` (keyword before `@`). Consider `T@` and `T exc@` as intentional, or unify as `T@` / `T@exc`.
5. **Moved-from access.** Compile error, or runtime trap on an empty pointer?
6. **Block comments** and **string escapes** are not defined.
7. **Packed struct layout:** bit order, endianness and trailing padding.
8. **Generics.** `raw_ptr<T>` is the only generic form so far. User-defined generics need a syntax decision, and `>>` splitting in the parser.
9. **Error handling.** Only traps and `get(..., -> ok)` exist. No exceptions or result type is specified.
10. **Overload resolution order** across `apply`, `patch` and the original.
11. **Return type for void.** Always `: void`, or may it be omitted?
12. **Mutable struct methods vs pure.** Confirm that struct methods follow the same `=>` purity rule as free functions.
