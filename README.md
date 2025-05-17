# mnxdom

Document object model for the MNX music interchange format. It is compatible with the C++17 standard.

- compatible with the C++17 standard and higher. (Currently tested with C++23.)
- uses [nlohmann\_json](https://github.com/nlohmann/json) as its JSON parser.
- allows structured access to MNX objects, without the need of quoted strings.
- validates against the MNX schema using [json-schema-validator](https://github.com/pboettch/json-schema-validator).

### Documentation

[MNX Document Model for C++17](https://rpatters1.github.io/mnxdom/)

Here is a simple example of code that creates the [Hello World](https://w3c.github.io/mnx/docs/mnx-reference/examples/hello-world/) MNX JSON file.

```cpp
// create new document
auto doc = mnx::Document(); // automatically creates required child nodes
// global
auto globalMeasure = doc.global().measures().append();
// required fields are supplied when objects are created
globalMeasure.create_barline(mnx::BarlineType::Regular);
globalMeasure.create_time(4, mnx::TimeSignatureUnit::Quarter);
// parts
auto part = doc.parts().append();
auto measure = part.create_measures().append();
measure.create_clefs().append(mnx::ClefSign::GClef, -2);
auto event = measure.sequences().append<mnx::sequence::Event>(mnx::NoteValueBase::Whole);
event.create_notes().append(mnx::NoteStep::C, 4);
// save to file
doc.save("hello_world.json", 4); // indent with 4 spaces
```

### Setup Instructions

Include the top header in your source file.

```cpp
#include "mnxdom.h"
```

Add the libary to your project with `FetchContent`:

```cmake
include(FetchContent)

FetchContent_Declare(
  musx
  GIT_REPOSITORY https://github.com/rpatters1/mnxdom
  GIT_TAG main # Replace with the desired commit hash, tag, or branch
)
FetchContent_MakeAvailable(musx)

# Also add somewhere:
target_link_libraries(project PRIVATE mnxdom) # replace "project" with your actual project name
```

### To Run the Tests

You need `cmake` to build and run the tests. From the directory containing this repository, configure the build directory:

```bash
cmake -S . build
```

To build the tests:

```bash
cmake --build build
```

To run the tests:

```bash
ctest --test-dir build
```

### To Build the Documentation

You need [Doxygen](https://doxygen.nl/index.html) to create the documentation. You can download and run the installer from the Doxygen site. Package managers can also install it. Once you have Doxygen you can build the documentation as follows.

```bash
cd docs
doxygen Doxyfile
```

You will then find an html website in `./docs/generated/html`. Use a web browser to view the site from your hard drive. Opening any of the `.html` pages gives you access to the entire site.
