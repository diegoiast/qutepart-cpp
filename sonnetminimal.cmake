CPMAddPackage(
    NAME sonnet_src
    GITHUB_REPOSITORY KDE/sonnet
    GIT_TAG v6.8.0
    DOWNLOAD_ONLY YES
)

set(GEN ${CMAKE_CURRENT_BINARY_DIR}/sonnet_gen)
file(MAKE_DIRECTORY ${GEN})

set(FAKE_SONNET_DIR ${GEN}/Sonnet)
file(MAKE_DIRECTORY ${FAKE_SONNET_DIR})

set(LOGGING_HELPER [[
inline QDebug operator+(QDebug dbg, const char* msg) {
    dbg << msg;
    return dbg;
}
]])

file(WRITE ${GEN}/sonnetcore_export.h [[
#pragma once
// auto generated file - do not modify!!!
#define SONNETCORE_EXPORT
#define SONNETCORE_NO_EXPORT
]])

file(WRITE ${GEN}/sonnetui_export.h [[
#pragma once
#define SONNETUI_EXPORT
#define SONNETUI_NO_EXPORT
// auto generated file - do not modify!!!
#include <QLoggingCategory>
#include <QDebug>
]])

file(WRITE ${GEN}/core_debug.h
"#pragma once\n"
"#include <QLoggingCategory>\n"
"#include <QDebug>\n"
"// auto generated file - do not modify!!!\n"
"${LOGGING_HELPER}\n"
"Q_DECLARE_LOGGING_CATEGORY(SONNET_LOG_CORE)\n"
)

file(WRITE ${GEN}/ui_debug.h [[
#pragma once
#include <QLoggingCategory>
#include <QDebug>
// auto generated file - do not modify!!!
Q_DECLARE_LOGGING_CATEGORY(SONNET_LOG_UI)
]])

file(WRITE ${GEN}/hunspelldebug.h [[
#pragma once
#include <QLoggingCategory>
#include <QDebug>
Q_DECLARE_LOGGING_CATEGORY(SONNET_HUNSPELL)
]])

file(WRITE ${GEN}/logging_categories.cpp [[
#include "core_debug.h"
#include "ui_debug.h"
#include "hunspelldebug.h"
// auto generated file - do not modify!!!
Q_LOGGING_CATEGORY(SONNET_LOG_CORE, "sonnet.core")
Q_LOGGING_CATEGORY(SONNET_LOG_UI, "sonnet.ui")
Q_LOGGING_CATEGORY(SONNET_HUNSPELL, "sonnet.hunspell")
]])

file(WRITE ${FAKE_SONNET_DIR}/Speller
"#pragma once\n"
"// auto generated file - do not modify!!! \n"
"#include \"${sonnet_src_SOURCE_DIR}/src/core/speller.h\"\n"
)

file(WRITE ${FAKE_SONNET_DIR}/Highlighter
"#pragma once\n"
"// auto generated file - do not modify!!! \n"
"#include \"${sonnet_src_SOURCE_DIR}/src/ui/highlighter.h\"\n"
)

add_library(sonnet-minimal STATIC
    ${sonnet_src_SOURCE_DIR}/src/core/backgroundchecker.cpp
    ${sonnet_src_SOURCE_DIR}/src/core/guesslanguage.cpp
    ${sonnet_src_SOURCE_DIR}/src/core/client.cpp
    ${sonnet_src_SOURCE_DIR}/src/core/languagefilter.cpp
    ${sonnet_src_SOURCE_DIR}/src/core/loader.cpp
    ${sonnet_src_SOURCE_DIR}/src/core/settings.cpp
    ${sonnet_src_SOURCE_DIR}/src/core/settingsimpl.cpp
    ${sonnet_src_SOURCE_DIR}/src/core/speller.cpp
    ${sonnet_src_SOURCE_DIR}/src/core/spellerplugin.cpp
    ${sonnet_src_SOURCE_DIR}/src/core/tokenizer.cpp
    ${sonnet_src_SOURCE_DIR}/src/core/textbreaks.cpp
    ${sonnet_src_SOURCE_DIR}/src/plugins/hunspell/hunspellclient.cpp
    ${sonnet_src_SOURCE_DIR}/src/plugins/hunspell/hunspelldict.cpp
    ${sonnet_src_SOURCE_DIR}/src/ui/highlighter.cpp
    ${GEN}/logging_categories.cpp
)

target_include_directories(sonnet-minimal
    PUBLIC
        ${GEN}
        ${FAKE_SONNET_DIR}
    PRIVATE
        ${sonnet_src_SOURCE_DIR}/src/core
        ${sonnet_src_SOURCE_DIR}/src/ui
        ${sonnet_src_SOURCE_DIR}/src/plugins/hunspell
)

target_compile_definitions(sonnet-minimal PRIVATE
    SONNET_STATIC
    INSTALLATION_PLUGIN_PATH="sonnet/plugins"
)

target_link_libraries(sonnet-minimal
    PUBLIC Qt6::Widgets
    PRIVATE hunspell_minimal
)

add_library(sonnet::minimal ALIAS sonnet-minimal)
