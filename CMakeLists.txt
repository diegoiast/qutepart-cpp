cmake_minimum_required(VERSION 3.8.0)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
option(QUTEPART_ENABLE_DOCS "Should we build documentation for qutepart" OFF)
option(QUTEPART_USE_MOLD "Should we link using mold?" ON)

project(qutepart)

include(FetchContent)
include(cmake/CPM.cmake)
include(cmake/mold-linker.cmake)

cpmaddpackage(
  NAME
  Format.cmake
  VERSION 1.8.2
  GITHUB_REPOSITORY
  TheLartians/Format.cmake
  OPTIONS
  "FORMAT_SKIP_CMAKE NO"
  "FORMAT_SKIP_CLANG NO"
  "CMAKE_FORMAT_EXCLUDE cmake/CPM.cmake")

if(QUTEPART_ENABLE_DOCS)
  add_subdirectory("docs")
endif()

set(QUTEPART_VERSION_MAJOR 0)
set(QUTEPART_VERSION_MINOR 1)
set(QUTEPART_VERSION_PATCH 0)
set(QUTEPART_VERSION
    ${QUTEPART_VERSION_MAJOR}.${QUTEPART_VERSION_MINOR}.${QUTEPART_VERSION_PATCH}
)
# set(QUTEPART_VERSION ${QUTEPART_VERSION} PARENT_SCOPE)

# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the QtWidgets library
find_package(Qt6 COMPONENTS Core Widgets Test)

# Populate a CMake variable with the sources
set(qutepart_SRCS
    qutepart-syntax-files.qrc
    qutepart-theme-data.qrc
    src/qutepart.cpp
    src/lines.cpp
    src/char_iterator.cpp
    src/text_block_utils.cpp
    src/text_block_flags.cpp
    src/bracket_highlighter.cpp
    src/side_areas.cpp
    src/completer.cpp
    src/theme.cpp
    src/html_delegate.cpp
    src/hl_factory.cpp
    src/hl/context.cpp
    src/hl/language.cpp
    src/hl/loader.cpp
    src/hl/rules.cpp
    src/hl/syntax_highlighter.cpp
    src/hl/style.cpp
    src/hl/context_stack.cpp
    src/hl/context_switcher.cpp
    src/hl/text_block_user_data.cpp
    src/hl/text_to_match.cpp
    src/hl/match_result.cpp
    src/hl/language_db_generated.cpp
    src/hl/language_db.cpp
    src/hl/text_type.cpp
    src/indent/indenter.cpp
    src/indent/indent_funcs.cpp
    src/indent/alg_impl.cpp
    src/indent/alg_lisp.cpp
    src/indent/alg_xml.cpp
    src/indent/alg_scheme.cpp
    src/indent/alg_python.cpp
    src/indent/alg_ruby.cpp
    src/indent/alg_cstyle.cpp)

include_directories(include/qutepart)
include_directories(src)
include_directories(test)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Tell CMake to create the qutepart library
add_library(qutepart ${qutepart_SRCS} include/qutepart/qutepart.h)

# Use the Widgets module from Qt
target_link_libraries(qutepart Qt::Core Qt::Widgets)

if(MSVC)
  target_compile_options(qutepart PRIVATE /W4)
else()
  target_compile_options(qutepart PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

# Build example executable
add_executable(editor example/editor.cpp)
target_link_libraries(editor Qt::Core Qt::Widgets qutepart)

# Install only library, not binaries
install(TARGETS qutepart DESTINATION lib)
install(FILES include/qutepart/hl_factory.h include/qutepart/qutepart.h
        DESTINATION include/qutepart)

# Tests
if(NOT DEFINED DISABLE_QUTEPART_TESTS)
  enable_testing()

  function(indent_test name)
    set(full_name test_indenter_${name})
    add_executable(${full_name} test/indenter/base_indenter_test.cpp
                                test/indenter/${full_name}.cpp)
    target_link_libraries(${full_name} Qt::Test Qt::Core Qt::Widgets qutepart)
    add_test(NAME ${name} COMMAND ${full_name})
  endfunction()

  indent_test(normal)
  indent_test(python)
  indent_test(ruby)
  indent_test(cstyle)
  indent_test(xml)
  indent_test(scheme)
  indent_test(lisp)
  # indent_test(haskel)

  function(qpart_test name)
    set(full_name test_${name})
    add_executable(${full_name} test/${full_name}.cpp)
    target_link_libraries(${full_name} Qt::Test Qt::Core Qt::Widgets qutepart)
    add_test(NAME ${name} COMMAND ${full_name})
  endfunction()

  qpart_test(char_iterator)
  qpart_test(text_block_utils)
  qpart_test(line_iterator)
  qpart_test(api)
  qpart_test(line)
  qpart_test(lines)
  qpart_test(line_edit_operations)
  qpart_test(edit_operations)
  qpart_test(bracket_highlighter)
endif()
