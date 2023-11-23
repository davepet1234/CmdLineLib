# CmdLineLib

EDK2 library for parsing the command line when writing EFI-Shell applications.

This this is not a buildable application but a module that can be added to your project. For a complete application and an example of usage please refer to the [CmdLineDemo](https://github.com/davepet1234/CmdLineDemo) project.


## Command Line Structure

The command line structure is as follows:

    command [parameters] [switches]

### Parameters

Parameters allow you to send data to the application and are identified by their position on the command line.

    command param1 param2 param3

As parameters may or not be required, the number of mandatory parameters must be specified. An error is returned if number of parameters entered is less than the mandatory number.

 Supported parameters types:

| Parameter Type | Description                     |
| -------------- | ------------------------------- |
| STR            | String                          |
| DEC            | Decimal number                  |
| HEX            | Hexidecimal number              |
| INT            | Integer number (decimal or hex) |
| ENUM           | Enum (string entry)             |

All numbers are unsigned and defaut to UINTN. You can also specify type size, so either 8, 16 or 32, which relate to UINT8, UINT16 and UINT32 values respectively.

 ### Switches

Switches are not position dependant as they are named and can have a short or long version.

    command -f name.txt

    command -file name.txt

Switches that don't require a value are called flags and are boolean (TRUE or FALSE). 

    command -verbose

Switches can be specified as being either optional or mandatory. An error is returned if a mandatory switch is not specified.

Support switch types:

| Switch Type     | Description                                       |
| ----------------| ------------------------------------------------- |
| OPT_FLAG        | Optional switch with no value (true/false)        |
| OPT_FLGVAL      | Optional switch with no value (has default value) |
| SWTABLE_OPT_STR | Optional string switch                            |
| MAN_STR         | Mandatory string switch                           |
| OPT_DEC         | Optional decimal switch                           |
| MAN_DEC         | Mandatory decimal switch                          |
| OPT_HEC         | Optional hexidecimal switch                       |
| MAN_HEC         | Mandatory hexidecimal switch                      |
| OPT_INT         | Optional integer (decimal or hex) switch          |
| MAN_INT         | Mandatory integer (decimal or hex) switch         |
| OPT_ENUM        | Optional enum switch (string entry)               |
| MAN_ENUM        | Mandatory enum switch (string entry)              |

All numbers are unsigned and defaut to UINTN. You can also specify type size, so either 8, 16 or 32, which relate to UINT8, UINT16 and UINT32 values respectively.

