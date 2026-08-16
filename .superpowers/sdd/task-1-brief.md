### Task 1: CMake app skeleton

**Files:**
- Create: `CMakeLists.txt`
- Create: `src/main.cpp`
- Create: `src/app/MainWindow.h`
- Create: `src/app/MainWindow.cpp`
- Modify: `README.md` (build block already present; keep in sync if generator text changes)

**Interfaces:**
- Consumes: Qt 5.15 Widgets via `QT_DIR`
- Produces: executable target `3DEditMax`; `MainWindow` constructor `explicit MainWindow(QWidget* parent = 0)`

- [ ] **Step 1: Write CMakeLists.txt**

```cmake
cmake_minimum_required(VERSION 3.16)
project(3DEditMax LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)

if(DEFINED ENV{QT_DIR})
  string(STRIP "$ENV{QT_DIR}" _qt_dir)
  string(REGEX REPLACE "[\\\\/]+$" "" _qt_dir "${_qt_dir}")
  get_filename_component(_qt_name "${_qt_dir}" NAME)
  if(_qt_name STREQUAL "bin")
    get_filename_component(_qt_dir "${_qt_dir}" DIRECTORY)
  endif()
  list(PREPEND CMAKE_PREFIX_PATH "${_qt_dir}")
  message(STATUS "Qt prefix from QT_DIR: ${_qt_dir}")
endif()

find_package(Qt5 5.15 REQUIRED COMPONENTS Widgets)

add_executable(3DEditMax
  src/main.cpp
  src/app/MainWindow.h
  src/app/MainWindow.cpp
)

target_include_directories(3DEditMax PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(3DEditMax PRIVATE Qt5::Widgets)

enable_testing()
add_executable(3deditmax_tests
  tests/test_main.cpp
)
target_include_directories(3deditmax_tests PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  "${CMAKE_SOURCE_DIR}/tests"
)
add_test(NAME 3deditmax_tests COMMAND 3deditmax_tests)
```

- [ ] **Step 2: Write a failing test runner so CTest has a target**

`tests/test_harness.h`:

```cpp
#pragma once
#include <cstdio>
#include <cstdlib>

inline int g_failures = 0; /* C++11: use a .cpp counter instead if MSVC rejects inline vars 鈥?see Step 2b */

#define TEST_CHECK(cond) do { \
  if (!(cond)) { \
    std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    ++g_test_failures; \
  } \
} while (0)
```

C++11 has no inline variables. Use this instead:

`tests/test_harness.h`:

```cpp
#pragma once
#include <cstdio>

int& testFailureCount();

#define TEST_CHECK(cond) do { \
  if (!(cond)) { \
    std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    ++testFailureCount(); \
  } \
} while (0)
```

`tests/test_main.cpp` (Task 1: one dummy check that passes, so the harness exists; real fails come in Task 3):

```cpp
#include "test_harness.h"

int& testFailureCount() {
  static int n = 0;
  return n;
}

int main() {
  TEST_CHECK(1 == 1);
  if (testFailureCount() != 0) {
    return 1;
  }
  return 0;
}
```

- [ ] **Step 3: Write MainWindow and main**

`src/app/MainWindow.h`:

```cpp
#pragma once
#include <QMainWindow>

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget* parent = 0);
};
```

`src/app/MainWindow.cpp`:

```cpp
#include "app/MainWindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
  setWindowTitle(QString::fromUtf8("3DEditMax"));
  resize(1280, 720);
}
```

`src/main.cpp`:

```cpp
#include "app/MainWindow.h"
#include <QApplication>

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  MainWindow w;
  w.show();
  return app.exec();
}
```

- [ ] **Step 4: Configure, build, run CTest**

```bat
set QT_DIR=D:\soft\qt5152\5.15.2\msvc2019_64
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build -C Debug --output-on-failure
```

Expected: configure finds Qt5, `3DEditMax.exe` and `3deditmax_tests.exe` build, CTest PASS. Run `build\3DEditMax.exe` and confirm an empty 1280脳720 window titled `3DEditMax`.

If `QT_DIR` on this machine differs, use the kit root that contains `lib\cmake\Qt5`.

- [ ] **Step 5: Commit and push**

```bash
git add CMakeLists.txt src/main.cpp src/app/MainWindow.h src/app/MainWindow.cpp tests/test_harness.h tests/test_main.cpp
git commit -m "Add CMake Qt skeleton and CTest harness."
git push origin main
```

---

