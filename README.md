# tmv
A C89 standard compliant, single header, nostdlib (no C Standard Library) squarified tree map viewer (TMV).

It uses the paper "Squarified Treemaps" algorythm by Mark Bruls, Kees Huizing, and Jarke J. van Wijk.

For more information please look at the "tmv.h" file or take a look at the "examples" or "tests" folder.

> [!WARNING]
> THIS PROJECT IS A WORK IN PROGRESS! ANYTHING CAN CHANGE AT ANY MOMENT WITHOUT ANY NOTICE! USE THIS PROJECT AT YOUR OWN RISK!

## Quick Start

Download or clone tmv.h and include it in your project.

```C
#include "tmv.h"

int main() {

    /* Define a output buffer for output rects */
    tmv_treemap_rect rects[TMV_MAX_RECTS];
    int rect_count = 0;

    /*
    Expected squarified treemap output if width = 100 and height = 100:

    id: 1, x:  0, y:  0, width: 50, height: 50
    id: 2, x:  0, y: 50, width: 50, height: 50
    id: 3, x: 50, y:  0, width: 50, height: 50
    id: 4, x: 50, y: 50, width: 50, height: 50
    id: 5, x:  0, y:  0, width: 25, height: 25  <- child1
    id: 6, x:  0, y: 25, width: 25, height: 25  <- child2
    id: 7, x: 25, y:  0, width: 25, height: 25  <- child3
    id: 8, x: 25, y: 25, width: 25, height: 25  <- child4
    */
    tmv_treemap_item child1 = {5, 2.5, 0, 0};
    tmv_treemap_item child2 = {6, 2.5, 0, 0};
    tmv_treemap_item child3 = {7, 2.5, 0, 0};
    tmv_treemap_item child4 = {8, 2.5, 0, 0};
    tmv_treemap_item children[4];

    tmv_treemap_item items[] = {
        {1, 10.0, 0, 4},
        {2, 10.0, 0, 0},
        {3, 10.0, 0, 0},
        {4, 10.0, 0, 0}};

    children[0] = child1;
    children[1] = child2;
    children[2] = child3;
    children[3] = child4;

    items[0].children = children;

    /* Build squarified recursive treemap view */
    tmv_squarify(
        items,                 /* List of treemap items                             */
        TMV_ARRAY_SIZE(items), /* Size of top level items                           */
        0, 0,                  /* Treemap view area start                           */
        100, 100,              /* Treemap view area width and height                */
        rects,                 /* The output buffer for rectangular shapes computed */
        &rect_count            /* The number of rectangular shapes computed.        */
    );

    /* Afterwards you can iterate through the rects */

    return 0;
}
```

## Run Example: nostdlib, freestsanding

In this repo you will find the "examples/tmv_win32_nostdlib.c" with the corresponding "build.bat" file which
creates an executable only linked to "kernel32" and is not using the C standard library and executes the program afterwards.

## "nostdlib" Motivation & Purpose

nostdlib is a lightweight, minimalistic approach to C development that removes dependencies on the standard library. The motivation behind this project is to provide developers with greater control over their code by eliminating unnecessary overhead, reducing binary size, and enabling deployment in resource-constrained environments.

Many modern development environments rely heavily on the standard library, which, while convenient, introduces unnecessary bloat, security risks, and unpredictable dependencies. nostdlib aims to give developers fine-grained control over memory management, execution flow, and system calls by working directly with the underlying platform.

### Benefits

#### Minimal overhead
By removing the standard library, nostdlib significantly reduces runtime overhead, allowing for faster execution and smaller binary sizes.

#### Increased security
Standard libraries often include unnecessary functions that increase the attack surface of an application. nostdlib mitigates security risks by removing unused and potentially vulnerable components.

#### Reduced binary size
Without linking to the standard library, binaries are smaller, making them ideal for embedded systems, bootloaders, and operating systems where storage is limited.

#### Enhanced performance
Direct control over system calls and memory management leads to performance gains by eliminating abstraction layers imposed by standard libraries.

#### Better portability
By relying only on fundamental system interfaces, nostdlib allows for easier porting across different platforms without worrying about standard library availability.
