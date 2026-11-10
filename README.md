# CmdLineLib

EDK2 library for parsing the command line when writing EFI-Shell applications.

This is not a buildable application but a module that can be added to your project. For a complete application and an example of usage please refer to the [CmdLineDemo](https://github.com/davepet1234/CmdLineDemo) project.

## Adding to a project

Add the sources to your application's `.inf`:

    [Sources]
      CmdLineLib/CmdLine.c
      CmdLineLib/CmdLine.h
      CmdLineLib/CmdLineInternal.h

    [LibraryClasses]
      UefiLib
      ShellCEntryLib
      ShellLib

## Quick start

```c
#include "CmdLineLib/CmdLine.h"

CHAR16  FileName[64] = L"";
UINT8   Level        = 0;
BOOLEAN Verbose      = FALSE;
BOOLEAN LevelPresent;

PARAMTABLE_START(ParamTable)
PARAMTABLE_STR(FileName, 64,  L"[file]file to process")
PARAMTABLE_END

SWTABLE_START(SwitchTable)
SWTABLE_OPT_FLAG(     L"-v", L"-verbose", &Verbose,                       L"verbose output")
SWTABLE_OPT_DEC8_FLGD(L"-l", L"-level",   &LevelPresent, &Level,          L"[num]level (0-255)")
SWTABLE_END

INTN EFIAPI ShellAppMain(IN UINTN Argc, IN CHAR16 **Argv)
{
    UINTN ParamCount;
    SHELL_STATUS Status = ParseCmdLine(ParamTable, 1, SwitchTable, L"My application", NO_OPT, &ParamCount);
    if (Status != SHELL_SUCCESS) {
        return Status;  // error already printed, or SHELL_ABORTED if help was shown
    }
    ...
}
```

Help text may start with `[name]` to name the argument in the generated help, e.g. `L"[num]level (0-255)"` shows as `-l, -level num`.

## Command Line Structure

    command [parameters] [switches]

### Parameters

Parameters are identified by their position on the command line.

    command param1 param2 param3

The number of mandatory parameters is passed to `ParseCmdLine()`; an error is returned if fewer are entered.

| Macro               | Description                                  |
| ------------------- | -------------------------------------------- |
| `PARAMTABLE_STR`    | String (CHAR16)                              |
| `PARAMTABLE_STR8`   | 8-bit string (CHAR8, Latin-1)                |
| `PARAMTABLE_DEC`    | Decimal number                               |
| `PARAMTABLE_HEX`    | Hexadecimal number (`0x` prefix optional)    |
| `PARAMTABLE_INT`    | Integer: decimal, or hex with `0x` prefix    |
| `PARAMTABLE_ENUM`   | Enum (string entry)                          |

### Switches

Switches are named, not positional, and can have a short and/or long name. Each switch is either optional (`OPT`) or mandatory (`MAN`).

    command -f name.txt
    command -file name.txt
    command -verbose

| Macro                            | Description                              |
| -------------------------------- | ---------------------------------------- |
| `SWTABLE_OPT_FLAG` / `MAN_FLAG`     | Flag, sets a BOOLEAN to TRUE          |
| `SWTABLE_OPT_FLGVAL` / `MAN_FLGVAL` | Flag, sets a UINTN to a non-zero value |
| `SWTABLE_OPT_STR` / `MAN_STR`       | String (CHAR16)                       |
| `SWTABLE_OPT_STR8` / `MAN_STR8`     | 8-bit string (CHAR8, Latin-1)         |
| `SWTABLE_OPT_DEC` / `MAN_DEC`       | Decimal number                        |
| `SWTABLE_OPT_HEX` / `MAN_HEX`       | Hexadecimal number                    |
| `SWTABLE_OPT_INT` / `MAN_INT`       | Integer (decimal or hex)              |
| `SWTABLE_OPT_ENUM` / `MAN_ENUM`     | Enum (string entry)                   |

- Numbers are unsigned and default to UINTN. Append `8`, `16` or `32` (e.g. `SWTABLE_OPT_DEC16`) for UINT8/UINT16/UINT32; values that don't fit are rejected.
- Append `_FLGD` (e.g. `SWTABLE_OPT_HEX_FLGD`) to also get a BOOLEAN set when the switch is present.
- Switch values cannot start with `-` or `/`.
- A switch table can hold up to 30 entries (`MAX_SWITCH_ENTRIES`).

### Enums

```c
ENUMSTR_START(ColourStrs)
ENUMSTR_ENTRY(RED,  L"red")
ENUMSTR_ENTRY(BLUE, L"blue")
ENUMSTR_END

SWTABLE_OPT_ENUM(L"-c", L"-colour", &Colour, ColourStrs, L"[val]colour")
```

Enum strings are matched case-insensitively; valid options are listed in the help.

### Built-in switches

| Switch          | Description                                | Disable with |
| --------------- | ------------------------------------------ | ------------ |
| `-h`, `-help`   | Display help and return `SHELL_ABORTED`    | `NO_HELP`    |
| `-b`, `-break`  | Enable shell page break                    | `NO_BREAK`   |

## Functions

| Function                    | Description                                                |
| --------------------------- | ---------------------------------------------------------- |
| `ParseCmdLine()`            | Parse the command line against the tables                  |
| `SetProgName()`             | Override the program name used in messages                 |
| `CheckProgAbort()`          | Return TRUE if ESC has been pressed                        |
| `WaitKeyPress()`            | Wait for any key, or a key from a list                     |
| `WaitKeyPressWithTimeout()` | As above, returns `EFI_TIMEOUT` after a timeout (ms)       |
| `StringInput()`             | Read a string from the keyboard                            |
| `DecimalInput()`            | Read a decimal number from the keyboard                    |
| `HexadecimalInput()`        | Read a hexadecimal number from the keyboard                |
| `IntegerInput()`            | Read a decimal or `0x` hex number from the keyboard        |

The input functions return `EFI_ABORTED` if ESC is pressed. See `CmdLine.h` for full parameter details.
