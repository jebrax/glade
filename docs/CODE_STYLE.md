# Code style

## Naming

- Class, struct, enum, enum class, and namespace names use `CapitalisedCase`.
- Methods and class or struct members use `camelCase`, including static methods
  and static members.
- Method, function, and lambda parameters use `snake_case`.
- Functions and variables that do not belong to a class or struct use
  `snake_case`. This includes local variables inside methods.
- If a method parameter name clashes with a member name, append an underscore
  to the parameter name, for example `value_`.
- Named fixed numeric and string constants, and enum or enum class values, use
  `ALL_CAPS_CASE`. This rule takes precedence over the variable/member naming
  rules. A local `const` variable whose value is computed at runtime follows
  the ordinary local-variable rule.

```cpp
constexpr int MAX_PLAYERS = 4;
constexpr char GAME_TITLE[] = "Beat 'em up";

enum class PlayerState {
  IDLE,
  WALKING
};

class Player {
public:
  void setValue(int value_) {
    value = value_;
  }

private:
  int value = 0;
};

void handle_input() {
  const auto key_code = ::read_key();
  ::process_key(key_code);
}
```

Prefix accesses to variables and functions belonging to the global namespace
with `::`, including project-defined globals.

```cpp
#include <cstdio>

void print_message() {
  ::printf("Foo");
}
```

## Control blocks

Separate `if` statements, loops, and similar control blocks from surrounding
statements with one blank line before and after the block. Do not add a blank
line immediately inside its braces. Keep connected clauses together:
`} else {`, `} catch (...) {`, and `} while (...);`.

An `if` statement written entirely on one line does not need surrounding blank
lines unless another rule requires them, such as return spacing or semantic
grouping.

```cpp
prepare();

if (ready) {
  run();
} else {
  wait();
}

finish();
```

## Return statements

Put one blank line before a `return` statement when its immediate enclosing
block contains more than two nonblank lines. Exclude the block's braces from
the count, but include the `return` itself. This also applies to function bodies.

```cpp
if (foo) {
  bar();
  auto ret = baz();

  return ret;
}

if (foo) {
  auto ret = baz();
  return ret;
}

if (foo) {
  return ret;
}
```

## Declarations and definitions

Keep one blank line between adjacent method declarations or definitions unless
both are one-liners. A method is a one-liner when its signature and body are on
the same line; a bodyless declaration on one line also counts as a one-liner.

Apply the same spacing rule to adjacent class, enum, enum class, and namespace
declarations or definitions.

```cpp
class Example {
public:
  void start();
  void stop();

  int value() {
    return 0;
  }

  int minimum() { return 0; }
  int maximum() { return 10; }
};
```

## Semantic grouping

Group lines by their meaning and separate groups with one blank line. Keep
closely related operations together, using blank lines to mark a change in
purpose.

## Namespaces

Namespace membership does not add indentation.

Keep one blank line between a namespace's opening brace and its content, unless
its first immediate child is another namespace. Keep one blank line between
the end of its content and its closing brace, unless its last immediate child
is another namespace. These are exceptions to the general rule against blank
lines immediately inside braces.

```cpp
namespace Foo {

class Bar {
};

}

namespace Foo2 {
namespace Foo3 {

class Example {
public:
  int method1() {
    return 0;
  }
};

}
}
```
