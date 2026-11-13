# Contributing

This guide covers how to set up, change, review and release the ICHU Alert Panel Embedded Application.

## Prerequisites

- [arduino-cli](https://arduino.github.io/arduino-cli/) 1.1 or later
- ESP32 Arduino core 3.3.4 (`esp32:esp32`)
- Libraries listed in `libraries.txt`
- Git

## Local setup

```sh
arduino-cli core install esp32:esp32@3.3.4
arduino-cli lib install ArduinoJson "LiquidCrystal I2C"
git config core.hooksPath .githooks
```

arduino-cli expects the main file to match its folder name, so compile a copy of the sources placed in a folder named `sketch`:

```sh
arduino-cli compile --fqbn esp32:esp32:esp32 --warnings all --output-dir build path/to/sketch
```

## Project structure and code quality

- One class per responsibility in PascalCase files with include guards: the Edge client (Wi-Fi, HTTP and NTP), the state rules without Arduino headers, and the outputs (LCD, LEDs and buzzer). `sketch.ino` only wires them together.
- Files stay at the repository root so that the project also opens in the Wokwi web editor.
- Configuration lives in `PanelConfig.h` as `constexpr` values, never as `#define` macros.
- `loop()` never waits on a timer: periodic work is scheduled with `millis()`, not `delay()`. The only blocking call is the HTTP GET, which stops after `kHttpTimeoutMs`.
- Comments are in English and short (1–2 lines) and explain why, not what.
- Every change must compile for `esp32:esp32:esp32` without warnings from the project files.
- Run the host test of the state rules (`tests/alert_status_test.cpp`, see the README) after any change to `AlertStatus`.

## Git Flow

Use main for release history and develop for integration. Do not commit directly to either branch.

| Branch | Start from | Pull request target |
| --- | --- | --- |
| feature/<short-description> | develop | develop |
| bugfix/<short-description> | develop | develop |
| release/vX.Y.Z | develop | main, then merge back into develop |
| hotfix/vX.Y.Z | main | main, then merge back into develop |

Use lowercase kebab-case branch descriptions. Keep branches focused and delete topic branches after they are merged.

Never open a pull request from `develop` to `main`; releases always go through `release/vX.Y.Z`. Do not use other branch prefixes, such as `fix/`, `docs/`, `refactor/`, `chore/`, or `audit/`.

~~~sh
git switch develop
git pull --ff-only origin develop
git switch -c feature/short-description
~~~

## Commit messages

Write commit messages in English using Conventional Commits:

~~~text
<type>(<scope>): <imperative summary>
~~~

Keep each commit focused and the summary concise, without a trailing period. Common types include feat, fix, docs, style, refactor, perf, test, build, ci, chore, and revert.

~~~text
feat(alerts): classify the active alert for the second LCD row
fix(connectivity): keep the last listing after a failed poll
docs: clarify local setup
~~~

Commit messages must also follow these rules:

- Start the summary with a lowercase letter, and keep the header at most 72 characters.
- Use only these scopes: `alerts`, `connectivity`, `display`, `indicators`, `simulation`, `config`.
- Use `feat` only for new functionality, `docs` for documentation and diagram changes, and `chore` for changelog, version, or maintenance changes.
- Do not use the default message of the GitHub web editor, such as `Update file.md`.

Invalid examples:

~~~text
(Docs): Add wiring table
Update sketch.ino
feat: add buzzer driver.
faet: add alert classifier
~~~

## Changelog

Record each logical code, configuration, architecture, or documentation change in CHANGELOG.md under [Unreleased]. Use the applicable Keep a Changelog category and describe the user-visible effect; do not list commit hashes.

## Pull requests

Open feature and bugfix pull requests against develop. Include the purpose, a concise summary, relevant checks, and screenshots or Wokwi evidence for changes that alter the panel output. Link related issues when applicable, address review feedback, and do not merge until the required review and checks are complete.

## Commit granularity

A commit is one complete logical change that can be described in a single sentence and leaves the project working.

| Too fine | Balanced | Too coarse |
| --- | --- | --- |
| One commit per file (`feat: add led driver`, `feat: add buzzer driver`, ...) | `feat(indicators): pulse the buzzer for 30 s on a new Critical alert` | `feat: implement the panel` (the whole firmware) |
| Several consecutive typo commits | `docs(readme): describe the wokwi wiring` | `docs: update readme` |

- Code: a feature branch usually has 3 to 8 commits of roughly 3 to 10 files each, split by layer or use case (for example state rules, network client, outputs, simulation).
- Documentation: one commit per complete artifact or section, not per paragraph.
- Group typo fixes with the section they belong to; a later round of fixes goes in a single `docs(<scope>): fix typos in <section>` commit.
- Before pushing a branch that only you use, squash local work-in-progress commits into logical commits. Never rewrite commits that are already published.
- The convention is enforced for commits created from 2026-11-07, when it was adopted; older history is kept unchanged.

## Enforcement

- After cloning, run once: `git config core.hooksPath .githooks`. The `commit-msg` hook rejects invalid messages locally.
- The `Git Flow checks` GitHub Actions workflow validates the branch name, the pull request target, and every commit message on pull requests to `main` and `develop`. Its `build` job compiles the firmware.
- Configure Git with your real name and the email linked to your GitHub account (`git config user.name` and `git config user.email`). `.mailmap` maps previously used alternative identities.
