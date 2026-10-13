/*
 * Copyright (C) 2018-2023 Andrei Kopats
 * Copyright (C) 2023-...  Diego Iastrubni <diegoiast@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <QString>

namespace Qutepart {

/// Severity of a diagnostic, mirroring the LSP scale. Independent of the
/// ERROR_BIT / WARNING_BIT / INFO_BIT flags, which describe editor marks rather
/// than attributed diagnostics.
enum class DiagnosticSeverity { Error, Warning, Information, Hint };

/// One problem attributed to something that produced it.
///
/// `owner` is a logical name - "clangd", "clang-tidy", "build" - not a process
/// identity. A server that dies and restarts keeps its name, so clearing by owner
/// after a restart cannot orphan the previous run's marks.
///
/// `severity` comes before `message` so a diagnostic that only marks a line can
/// be written as `Diagnostic{"clangd", DiagnosticSeverity::Error}` and left to
/// default its message, rather than passing an empty string in the middle.
struct Diagnostic {
    QString owner;
    DiagnosticSeverity severity = DiagnosticSeverity::Error;
    QString message = {};
};

} // namespace Qutepart
