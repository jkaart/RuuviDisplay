# RuuviDisplay Project - Agent Guidelines

## 1. Purpose

This repository contains the RuuviDisplay firmware for the LilyGO T5-47 e-paper device.

The project is an ESP32 PlatformIO project using the Arduino framework.

These rules apply to coding agents working in this repository.

---

## 2. Core Rules

### Investigate before implementing

Do not immediately start modifying code for a non-trivial task.

Before making changes:

1. Understand the actual requirement.
2. Inspect the existing implementation and project structure.
3. Search for existing functionality that can be reused.
4. Check existing PlatformIO dependencies.
5. Check whether Arduino or ESP32 already provides the required functionality.
6. Check for a suitable maintained library when appropriate.
7. Identify important compatibility or hardware constraints.

Prefer the smallest change that solves the actual problem.

### Reuse before reimplementing

Prefer solutions in this order:

1. Existing project code
2. Arduino / ESP32 functionality
3. Existing project dependencies
4. Mature external libraries
5. Custom implementation

Do not add a dependency or custom implementation merely because it is convenient.

Before adding a new dependency, verify that it is compatible with the project's ESP32, Arduino framework, and PlatformIO setup.

### Do not make major architectural decisions silently

When several reasonable implementations exist, compare them before making a substantial architectural change.

For non-trivial changes involving:

- new dependencies
- hardware interaction
- networking
- persistence
- protocols
- architecture
- substantial refactoring

first investigate and state the recommended approach before implementing it.

For simple, local, low-risk fixes, this does not require a separate planning step.

### Do not restore deleted files without explicit instruction

A file deleted by the user is an intentional change unless the user explicitly states otherwise.

Do not restore deleted files from Git history, previous commits, `old/`, or other locations merely because the file appears to be missing or was previously used by the project.

---

## 3. Project Architecture

Keep the project modular.

Prefer separating:

- application logic
- data parsing and processing
- display logic
- networking
- configuration
- hardware-specific code

Do not introduce unnecessary coupling between unrelated components.

Keep platform-specific code isolated where practical so that hardware-independent logic can be tested without the device.

Do not perform unrelated refactoring while implementing a feature or fixing a bug.

---

## 4. `old/` Directory

The `old/` directory contains reference material from the previous ESP32WeatherDisplay project.

It is reference material only.

### Rules

- Do not modify anything inside `old/`.
- Do not treat code in `old/` as part of the current implementation.
- Do not automatically copy the old architecture into the current project.
- Do not add dependencies solely because they exist in `old/platformio.ini`.
- Use the old project only to understand hardware, APIs, or previously working implementation patterns.
- Prefer the current project's architecture and dependencies over the old implementation.

Before reusing code from `old/`, determine that the functionality is actually required by the current project.

---

## 5. Implementation Workflow

For non-trivial work, use this workflow.

### Step 1 - Understand

Determine:

- what the user actually needs
- what existing code is relevant
- what constraints exist
- what must remain compatible

### Step 2 - Investigate

Inspect the repository, relevant source files, dependencies, and APIs.

Do not modify source code merely to experiment during investigation unless necessary for diagnosis.

### Step 3 - Plan

Choose the smallest appropriate implementation.

For substantial architectural, dependency, networking, persistence, or hardware changes, present the approach before implementation.

### Step 4 - Implement

Make the required changes.

Prefer small, focused changes over broad rewrites.

Reuse existing code where appropriate.

### Step 5 - Verify

After implementation:

1. build the affected PlatformIO environment
2. run relevant tests
3. fix errors introduced by the change
4. verify that unrelated project functionality was not broken

### Step 6 - Report

Report:

- what changed
- why the approach was chosen
- what was tested
- whether the relevant build/test commands passed
- any remaining limitations

---

## 6. Build and Test Failures

A diagnosis is not a completed fix.

When a build, test, compile, or linker error occurs:

1. Read the actual error output.
2. Inspect the relevant source files and build configuration.
3. Identify the smallest reasonable fix.
4. Implement the fix.
5. Re-run the command that originally failed.
6. If the first fix fails, continue investigating.
7. Try the next reasonable solution instead of stopping.
8. Do not rewrite unrelated code to work around a localized problem.
9. Do not replace the project's test framework or build system unless required.

Do not stop after identifying the cause.

For example:

> "The linker error occurs because `timezone_table.cpp` is not linked."

is a diagnosis, not a completed task.

The task is complete only when the failed build/test has been re-run and its result verified.

If a genuine external blocker prevents completion, clearly report:

- what was attempted
- what failed
- what remains unresolved
- what evidence supports the conclusion

---

## 7. Testing

### Tests must not be resurrected

Tests are not mandatory project files.

If a test file is removed by the user, do not restore, recreate, copy, or retrieve that test from Git history unless the user explicitly asks for it to be restored.

Do not treat deleted tests as missing functionality.

Only create new tests when the task explicitly requires adding a test or when the user explicitly asks for testing to be added.

Tests belong in:

```text
test/
```

Prefer unit tests for hardware-independent logic.

When practical, code such as:

- data parsing
- calculations
- formatting
- configuration handling
- validation
- lookup tables

should be structured so that it can be tested independently of the ESP32 hardware.

Hardware-dependent functionality such as display hardware, GPIO, Wi-Fi hardware, deep sleep, and other device-specific behavior may require target-specific testing.

Do not create tests that merely exercise code without checking meaningful behavior.

A test should verify a defined input, state, or condition and an expected result.

Use the existing project's test infrastructure rather than inventing a separate test mechanism unless there is a clear reason.

### Native test shared source

Native tests may test production source files from `src/`.

PlatformIO does not include `src/` source files in a test build by default.

When a native test needs to use an existing production implementation from `src/`, do not copy the production `.cpp` file into `test/`.

Instead, configure the native test environment appropriately, using PlatformIO's `test_build_src = yes` when suitable.

The test must use the actual production source file so that the test verifies the same implementation used by the firmware.

Do not create duplicate copies or test-only implementations of production source files merely to resolve linker errors.

---

## 8. PlatformIO

The project uses PlatformIO with the Arduino framework.

Common commands:

```bash
source ~/.platformio/penv/bin/activate

# Build
pio run

# Build a specific environment
pio run -e <environment>

# Upload
pio run -t upload

# Serial monitor
pio run -t monitor

# Run tests
pio test

# Run tests for one environment
pio test -e <environment>

# Clean build artifacts
pio run -t clean
```

When debugging a failing PlatformIO build or test, use verbose output when necessary:

```bash
pio run -vvv
pio test -vvv
```

Always use the actual environments defined in `platformio.ini`. Do not invent environment names.

---

## 9. Current Hardware

Primary target:

- LilyGO T5-47
- ESP32-WROVER-class device
- 4.7" ED047TC1 e-paper display
- 960 × 540 resolution
- EPDiy for e-paper control
- WiFiManager for network configuration

The repository may also contain support or configuration for related ESP32 / ESP32-S3 hardware.

Hardware-specific assumptions must be verified against the current project configuration before changing them.

---

## 10. Important Project Locations

```text
src/        Current application source code
include/    Project headers
lib/        Project-specific libraries
test/       Tests
old/        Reference implementation - do not modify
platformio.ini  PlatformIO environments and dependencies
```

Create new project files in the appropriate current-project directory.

Do not put new implementation code into `old/`.

---

## 11. Final Rule

The goal is not merely to produce code.

The goal is to leave the project in a working, verifiable state.

Prefer:

- understanding before editing
- reuse before reimplementation
- small changes before rewrites
- tests before assumptions
- verified results before declaring completion
