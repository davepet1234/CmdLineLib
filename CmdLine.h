/***********************************************************************

 CmdLine.h
 
 Author: David Petrovic
 GitHub: https://github.com/davepet1234/CmdLineLib

***********************************************************************/

#ifndef CMD_LINE_H
#define CMD_LINE_H
#ifdef __cplusplus
extern "C" {
#endif

#include <Uefi.h>
#include <Library/ShellLib.h>
#include "CmdLineInternal.h"

//-------------------------------------
// Parameter Table Macros
//-------------------------------------

/**
  PARAMTABLE_START - Begins the parameter table

  ArrayName     Defines name of parameter table
**/
#define PARAMTABLE_START(ArrayName) \
    PARAMETER_TABLE ArrayName[] = {

/**
  PARAMTABLE_STR - Adds string parameter to table

  ValueRetPtr   Ptr to CHAR16 string to hold value entered
  StrSize       Size of above string
  HelpStr       Ptr to CHAR16 help string for parameter
**/
#define PARAMTABLE_STR(ValueRetPtr, StrSize, HelpStr) \
    {VALTYPE_STRING, {.MaxStrSize=StrSize}, {.pChar16=ValueRetPtr}, HelpStr},

/**
  PARAMTABLE_DEC - Adds decimal parameter to table

  ValueRetPtr   Ptr to (UINTN|UINT8|UINT16|UINT32) to hold value entered
  HelpStr       Ptr to CHAR16 help string for parameter
**/
#define PARAMTABLE_DEC(ValueRetPtr, HelpStr) \
    {VALTYPE_DECIMAL, {.ValSize=SIZEN}, {.pUintn=ValueRetPtr}, HelpStr},
#define PARAMTABLE_DEC8(ValueRetPtr, HelpStr) \
    {VALTYPE_DECIMAL, {.ValSize=SIZE8}, {.pUint8=ValueRetPtr}, HelpStr},
#define PARAMTABLE_DEC16(ValueRetPtr, HelpStr) \
    {VALTYPE_DECIMAL, {.ValSize=SIZE16}, {.pUint16=ValueRetPtr}, HelpStr},
#define PARAMTABLE_DEC32(ValueRetPtr, HelpStr) \
    {VALTYPE_DECIMAL, {.ValSize=SIZE32}, {.pUint32=ValueRetPtr}, HelpStr},

/**
  PARAMTABLE_HEX - Adds hexidecimal parameter to table

  ValueRetPtr   Ptr to (UINTN|UINT8|UINT16|UINT32) to hold value entered
  HelpStr       Ptr to CHAR16 help string for parameter
**/
#define PARAMTABLE_HEX(ValueRetPtr, HelpStr) \
    {VALTYPE_HEXIDECIMAL, {.ValSize=SIZEN}, {.pUintn=ValueRetPtr}, HelpStr},
#define PARAMTABLE_HEX8(ValueRetPtr, HelpStr) \
    {VALTYPE_HEXIDECIMAL, {.ValSize=SIZE8}, {.pUint8=ValueRetPtr}, HelpStr},
#define PARAMTABLE_HEX16(ValueRetPtr, HelpStr) \
    {VALTYPE_HEXIDECIMAL, {.ValSize=SIZE16}, {.pUint16=ValueRetPtr}, HelpStr},
#define PARAMTABLE_HEX32(ValueRetPtr, HelpStr) \
    {VALTYPE_HEXIDECIMAL, {.ValSize=SIZE32}, {.pUint32=ValueRetPtr}, HelpStr},

/**
  PARAMTABLE_INT - Adds integer parameter (decimal or hex) to table

  ValueRetPtr   Ptr to (UINTN|UINT8|UINT16|UINT32) to hold value entered
  HelpStr       Ptr to CHAR16 help string for parameter
**/
#define PARAMTABLE_INT(ValueRetPtr, HelpStr) \
    {VALTYPE_INTEGER, {.ValSize=SIZEN}, {.pUintn=ValueRetPtr}, HelpStr},
#define PARAMTABLE_INT8(ValueRetPtr, HelpStr) \
    {VALTYPE_INTEGER, {.ValSize=SIZE8}, {.pUint8=ValueRetPtr}, HelpStr},
#define PARAMTABLE_INT16(ValueRetPtr, HelpStr) \
    {VALTYPE_INTEGER, {.ValSize=SIZE16}, {.pUint16=ValueRetPtr}, HelpStr},
#define PARAMTABLE_INT32(ValueRetPtr, HelpStr) \
    {VALTYPE_INTEGER, {.ValSize=SIZE32}, {.pUint32=ValueRetPtr}, HelpStr},

/**
  PARAMTABLE_ENUM - Adds enum parameter to table (string entry)

  ValueRetPtr   Ptr to enum to hold value entered
  EnumArray     Ptr to array defining enum value to string
  HelpStr       Ptr to CHAR16 help string for parameter
**/
#define PARAMTABLE_ENUM(ValueRetPtr, EnumArray, HelpStr) \
    {VALTYPE_ENUM, EnumArray, {.pEnum=ValueRetPtr}, HelpStr},

/**
  PARAMTABLE_END - Ends the parameter table
**/
#define PARAMTABLE_END \
    {VALTYPE_NONE,{0},{0},NULL}};



//-------------------------------------
// Switch Table Macros
//-------------------------------------

/**
  SWTABLE_START - Begins the switch table

  ArrayName     Defines name of switch table
**/
#define SWTABLE_START(ArrayName) \
    SWITCH_TABLE ArrayName[] = {

/**
  SWTABLE_OPT_FLAG - Adds a optional switch with no value (true/false) to table 

  SwStr1        Ptr to CHAR16 defining short switch name
  SwStr2        Ptr to CHAR16 defining long switch name
  ValueRetPtr   Ptr to BOOLEAN, set to TRUE if switch present
  HelpStr       Ptr to CHAR16 help string for switch
**/
#define SWTABLE_OPT_FLAG(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    {SwStr1, SwStr2, OPT_SW, VALTYPE_NONE, {0}, {.pBoolean=ValueRetPtr}, HelpStr},

/**
  SWTABLE_OPT_FLGVAL - Adds an optional switch with no value (has default value) to table

  SwStr1        Ptr to CHAR16 defining short switch name
  SwStr2        Ptr to CHAR16 defining long switch name
  ValueRetPtr   Ptr to UINTN, set to 'Value' if switch present
  Value         Value to set if switch present
  HelpStr       Ptr to CHAR16 help string for switch
**/
#define SWTABLE_OPT_FLGVAL(SwStr1, SwStr2, ValueRetPtr, Value, HelpStr) \
    {SwStr1, SwStr2, OPT_SW, VALTYPE_NONE, {.FlagValue=Value}, {.pUintn=ValueRetPtr}, HelpStr},

/**
  SWTABLE_OPT_STR - Adds an optional string switch to table
  SWTABLE_MAN_STR - Adds a mandatory string switch to table

  SwStr1        Ptr to CHAR16 defining short switch name
  SwStr2        Ptr to CHAR16 defining long switch name
  ValueRetPtr   Ptr to CHAR16 string to hold value entered
  StrSize       Size of above string
  HelpStr       Ptr to CHAR16 help string for parameter
**/
#define SWTABLE_OPT_STR(SwStr1, SwStr2, ValueRetPtr, StrSize, HelpStr) \
    { SwStr1, SwStr2, OPT_SW, VALTYPE_STRING, {.MaxStrSize=StrSize}, {.pChar16=ValueRetPtr}, HelpStr},
#define SWTABLE_MAN_STR(SwStr1, SwStr2, ValueRetPtr, StrSize, HelpStr) \
    { SwStr1, SwStr2, MAN_SW, VALTYPE_STRING, {.MaxStrSize=StrSize}, {.pChar16=ValueRetPtr}, HelpStr},


/**
  SWTABLE_OPT_DEC - Adds an optional decimal switch to table
  SWTABLE_MAN_DEC - Adds a mandatory decimal switch to table

  SwStr1        Ptr to CHAR16 defining short switch name
  SwStr2        Ptr to CHAR16 defining long switch name
  ValueRetPtr   Ptr to (UINTN|UINT8|UINT16|UINT32) to hold value entered
  HelpStr       Ptr to CHAR16 help string for parameter
**/
#define SWTABLE_OPT_DEC(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, OPT_SW, VALTYPE_DECIMAL, {.ValSize=SIZEN}, {.pUintn=ValueRetPtr}, HelpStr},
#define SWTABLE_OPT_DEC8(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, OPT_SW, VALTYPE_DECIMAL, {.ValSize=SIZE8}, {.pUint8=ValueRetPtr}, HelpStr},
#define SWTABLE_OPT_DEC16(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, OPT_SW, VALTYPE_DECIMAL, {.ValSize=SIZE16}, {.pUint16=ValueRetPtr}, HelpStr},
#define SWTABLE_OPT_DEC32(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, OPT_SW, VALTYPE_DECIMAL, {.ValSize=SIZE32}, {.pUint32=ValueRetPtr}, HelpStr},

#define SWTABLE_MAN_DEC(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, MAN_SW, VALTYPE_DECIMAL, {.ValSize=SIZEN}, {.pUintn=ValueRetPtr}, HelpStr},
#define SWTABLE_MAN_DEC8(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, MAN_SW, VALTYPE_DECIMAL, {.ValSize=SIZE8}, {.pUint8=ValueRetPtr}, HelpStr},
#define SWTABLE_MAN_DEC16(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, MAN_SW, VALTYPE_DECIMAL, {.ValSize=SIZE16}, {.pUint16=ValueRetPtr}, HelpStr},
#define SWTABLE_MAN_DEC32(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, MAN_SW, VALTYPE_DECIMAL, {.ValSize=SIZE32}, {.pUint32=ValueRetPtr}, HelpStr},

/**
  SWTABLE_OPT_HEC - Adds an optional hexidecimal switch to table
  SWTABLE_MAN_HEC - Adds a mandatory hexidecimal switch to table

  SwStr1        Ptr to CHAR16 defining short switch name
  SwStr2        Ptr to CHAR16 defining long switch name
  ValueRetPtr   Ptr to (UINTN|UINT8|UINT16|UINT32) to hold value entered
  HelpStr       Ptr to CHAR16 help string for parameter
**/
#define SWTABLE_OPT_HEX(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, OPT_SW, VALTYPE_HEXIDECIMAL, {.ValSize=SIZEN}, {.pUintn=ValueRetPtr}, HelpStr},
#define SWTABLE_OPT_HEX8(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, OPT_SW, VALTYPE_HEXIDECIMAL, {.ValSize=SIZE8}, {.pUint8=ValueRetPtr}, HelpStr},
#define SWTABLE_OPT_HEX16(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, OPT_SW, VALTYPE_HEXIDECIMAL, {.ValSize=SIZE16}, {.pUint16=ValueRetPtr}, HelpStr},
#define SWTABLE_OPT_HEX32(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, OPT_SW, VALTYPE_HEXIDECIMAL, {.ValSize=SIZE32}, {.pUint32=ValueRetPtr}, HelpStr},

#define SWTABLE_MAN_HEX(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, MAN_SW, VALTYPE_HEXIDECIMAL, {.ValSize=SIZEN}, {.pUintn=ValueRetPtr}, HelpStr},
#define SWTABLE_MAN_HEX8(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, MAN_SW, VALTYPE_HEXIDECIMAL, {.ValSize=SIZE8}, {.pUint8=ValueRetPtr}, HelpStr},
#define SWTABLE_MAN_HEX16(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, MAN_SW, VALTYPE_HEXIDECIMAL, {.ValSize=SIZE16}, {.pUint16=ValueRetPtr}, HelpStr},
#define SWTABLE_MAN_HEX32(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, MAN_SW, VALTYPE_HEXIDECIMAL, {.ValSize=SIZE32}, {.pUint32=ValueRetPtr}, HelpStr},

/**
  SWTABLE_OPT_INT - Adds an optional integer (decimal or hex) switch to table
  SWTABLE_MAN_INT - Adds a mandatory integer (decimal or hex) switch to table

  SwStr1        Ptr to CHAR16 defining short switch name
  SwStr2        Ptr to CHAR16 defining long switch name
  ValueRetPtr   Ptr to UINTN to hold value entered
  HelpStr       Ptr to CHAR16 help string for parameter
**/
#define SWTABLE_OPT_INT(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, OPT_SW, VALTYPE_INTEGER, {.ValSize=SIZEN}, {.pUintn=ValueRetPtr}, HelpStr},
#define SWTABLE_OPT_INT8(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, OPT_SW, VALTYPE_INTEGER, {.ValSize=SIZE8}, {.pUint8=ValueRetPtr}, HelpStr},
#define SWTABLE_OPT_INT16(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, OPT_SW, VALTYPE_INTEGER, {.ValSize=SIZE16}, {.pUint16=ValueRetPtr}, HelpStr},
#define SWTABLE_OPT_INT32(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, OPT_SW, VALTYPE_INTEGER, {.ValSize=SIZE32}, {.pUint32=ValueRetPtr}, HelpStr},

#define SWTABLE_MAN_INT(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, MAN_SW, VALTYPE_INTEGER, {.ValSize=SIZEN}, {.pUintn=ValueRetPtr}, HelpStr},
#define SWTABLE_MAN_INT8(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, MAN_SW, VALTYPE_INTEGER, {.ValSize=SIZE8}, {.pUint8=ValueRetPtr}, HelpStr},
#define SWTABLE_MAN_INT16(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, MAN_SW, VALTYPE_INTEGER, {.ValSize=SIZE16}, {.pUint16=ValueRetPtr}, HelpStr},
#define SWTABLE_MAN_INT32(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, MAN_SW, VALTYPE_INTEGER, {.ValSize=SIZE32}, {.pUint32=ValueRetPtr}, HelpStr},

/**
  SWTABLE_OPT_ENUM - Adds an optional enum switch to table (string entry)
  SWTABLE_MAN_ENUM - Adds a mandatory enum switch to table (string entry)

  SwStr1        Ptr to CHAR16 defining short switch name
  SwStr2        Ptr to CHAR16 defining long switch name
  ValueRetPtr   Ptr to enum to hold value entered
  EnumArray     Ptr to array defining enum value to string
  HelpStr       Ptr to CHAR16 help string for parameter
**/
#define SWTABLE_OPT_ENUM(SwStr1, SwStr2, ValueRetPtr, EnumArray, HelpStr) \
    { SwStr1, SwStr2, OPT_SW, VALTYPE_ENUM, {.EnumStrArray=EnumArray}, {.pEnum=(unsigned int *)ValueRetPtr}, HelpStr},
#define SWTABLE_MAN_ENUM(SwStr1, SwStr2, EnumArray, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, MAN_SW, VALTYPE_ENUM, {.EnumStrArray=EnumArray}, {.pEnum=(unsigned int *)ValueRetPtr}, HelpStr},

/**
  SWTABLE_END -Ends the switch table
**/
#define SWTABLE_END \
    {NULL,NULL,NO_SW,VALTYPE_NONE,{0},{0},NULL}};

//-------------------------------------
// Enum to String Table Macros
//-------------------------------------

/**
  ENUMSTR_START - Begins the Enum to String array

  ArrayName     Defines name of Enum to String array
**/
#define ENUMSTR_START(ArrayName) \
    ENUM_STR_ARRAY ArrayName[] = {

/**
  ENUMSTR_ENTRY - Adds a mapping to the Enum to String array

  Value         Enum value
  Str           Associated string
**/
#define ENUMSTR_ENTRY(Value, Str) \
    {Value, Str},

/**
  ENUMSTR_END - Ends the Enum to String array
**/
#define ENUMSTR_END \
    {0,NULL}};

//-------------------------------------
// Defines
//-------------------------------------

// Functional options
#define NO_OPT          0x0000
#define NO_HELP         0x0001
#define NO_BREAK        0x0002

//-------------------------------------
// Functions
//-------------------------------------

/**
  ParseCmdLine - Parses the command line
  
  ParamTable    Ptr to PARAMETER_TABLE defining the expected parameters
                If no parameters required set this to NULL
  ManParmCount  Number of manatory parameters required
  SwTable       Ptr to SWITCH_TABLE defining the expected switches
                If no switches required set this to NULL
  ProgHelpStr   Ptr to help string for program
  FuncOpt       Functional options (bit values to be ORed)
                    NO_OPT          no option, used on its own
                    NO_HELP         no command line help
                    FORCE_BREAK     no break option
  NumParams     Ptr to return the number of parameter entered (optional)
  
  Returns       SHELL_SUCCESS if all parameters/switches are valid
                SHELL_INVALID_PARAMETER if problem encountered with parameter/switches passed on cmd line
                SHELL_OUT_OF_RESOURCES if internal memory error
                SHELL_ABORTED if help displayed
**/
extern SHELL_STATUS ParseCmdLine(IN PARAMETER_TABLE *ParamTable, IN UINTN ManParmCount, IN SWITCH_TABLE *SwTable, IN CHAR16 *ProgHelpStr, IN UINT16 FuncOpt, OUT UINTN *NumParams);


/**
  SetProgName - Shell appication name is taken from cmd line parameters, this function allows it to be overriden

  ProgName      Name of shell app

  Returns       NA
**/
extern VOID SetProgName(IN CONST CHAR16* ProgName);

/**
  CheckProgAbort - Checks to see if Cntrl-C or ESC has been pressed

  PrintMsg      TRUE to print abort message

  Returns       TRUE if abort key pressed
**/
extern BOOLEAN CheckProgAbort(BOOLEAN PrintMsg);


#ifdef __cplusplus
}
#endif
#endif // CMD_LINE_H
