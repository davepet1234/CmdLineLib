/***********************************************************************

 CmdLine.c
 
 Author: David Petrovic
 GitHub: https://github.com/davepet1234/CmdLineLib

***********************************************************************/

// ### 
// Useful link: Command-Line Interfaces: Structure & Syntax
// https://dev.to/paulasantamaria/command-line-interfaces-structure-syntax-2533
// ###

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/ShellLib.h>
#include <Library/PrintLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib/BaseLibInternals.h>
#include <Library/UefiBootServicesTableLib.h>
#include "Protocol/EfiShellInterface.h"
#include "CmdLine.h"
#include "CmdLineInternal.h"

extern EFI_SHELL_INTERFACE *mEfiShellInterface;

#define DEBUG_MODE 0
#if DEBUG_MODE
#define TRACE(x) Print x
#else
#define TRACE(x)
#endif

// types
typedef enum {
    VAL_OK,
    VAL_STR_TRUNCATED,
    VAL_DEC_INVALID,
    VAL_HEX_INVALID,
    VAL_INT_INVALID,
    VAL_UINT8_TOO_BIG,
    VAL_UINT16_TOO_BIG,
    VAL_UINT32_TOO_BIG,
    VAL_OPT_INVALID,
    VAL_UNSUPPORTED_TYPE,
    VAL_UNSUPPORTED_SIZE,
    VAL_ERROR
} VALUE_STATUS;


// locals functions
STATIC VOID ValueError(IN VALUE_STATUS ValStatus, IN CONST CHAR16* SwStr, IN UINTN ParamNum, IN CONST CHAR16* ValString);
STATIC VALUE_STATUS ReturnValue(IN CONST CHAR16 *String, IN VALUE_TYPE ValueType, IN DATA *Data, OUT VALUE_RET_PTR ValueRetPtr);
STATIC VALUE_STATUS ProcessIntVal(IN UINTN Value, IN VALUE_SIZE ValSize, OUT VALUE_RET_PTR ValueRetPtr);
STATIC BOOLEAN GetEnumVal(IN ENUM_STR_ARRAY *EnumStrArray, IN CONST CHAR16 *Str, OUT UINTN *Value);
STATIC INTN EFIAPI StriCmp(IN CONST CHAR16 *FirstString, IN CONST CHAR16 *SecondString);
STATIC BOOLEAN HasHexPrefix(IN CONST CHAR16 *String);
STATIC BOOLEAN IsHexString(IN CONST CHAR16 *String);
STATIC BOOLEAN IsDecimalString(IN CONST CHAR16 *String);
STATIC CONST CHAR16* GetFileName(CONST CHAR16* PathName);
STATIC VOID TableError(IN UINTN i, IN CHAR16 *errStr);
STATIC BOOLEAN ArgNameDefined(IN CHAR16 *HelpStr);
STATIC UINTN GetArgName(IN CHAR16 *HelpStr, OUT CHAR16* ArgName, IN UINTN ArgNameSize, IN BOOLEAN Mandatory, IN CONST CHAR16 *DefaultArgName);
STATIC VOID ShowHelp(IN UINTN ManParamCount, IN PARAMETER_TABLE *ParamTable, IN SWITCH_TABLE *SwTable, IN CONST CHAR16 *ProgHelpStr, IN UINTN FuncOpt);

// globals
STATIC CONST CHAR16* CONST g_BreakSwStr1 = L"-b";
STATIC CONST CHAR16* CONST g_BreakSwStr2 = L"-break";
STATIC CONST CHAR16* CONST g_BreakSwStr = L"enable page break mode";

STATIC CONST CHAR16* CONST g_HelpSwStr1 = L"-h";
STATIC CONST CHAR16* CONST g_HelpSwStr2 = L"-help";
STATIC CONST CHAR16* CONST g_HelpSwStr = L"display this help and exit";

STATIC CONST CHAR16* CONST g_DefaultArgName = L"arg";

STATIC CONST CHAR16 *g_ProgName = NULL;


/**
 * SetProgName()
 *
 **/
VOID SetProgName(IN CONST CHAR16* ProgName)
{
    g_ProgName = ProgName;
}

/**
 * ParseCmdLine()
 * 
 **/
SHELL_STATUS ParseCmdLine(IN PARAMETER_TABLE* ParamTable, IN UINTN ManParamCount, IN SWITCH_TABLE* SwTable, IN CHAR16* ProgHelpStr, IN UINT16 FuncOpt, OUT UINTN* NumParams)
{
    SHELL_STATUS ShellStatus = SHELL_INVALID_PARAMETER;

    // reset number of actual parameters 
    if (NumParams) {
        *NumParams = 0;
    }

    // initialise switch present flags
    BOOLEAN SwPresent[MAX_SWITCH_ENTRIES] = { 0 };

    // get cmd line arguments
    UINTN Argc;
    CHAR16** Argv;
    if (gEfiShellParametersProtocol != NULL) {  // Check for UEFI Shell 2.0 protocols
        Argc = gEfiShellParametersProtocol->Argc;
        Argv = gEfiShellParametersProtocol->Argv;
    } else if  (mEfiShellInterface != NULL) {
        Argc = mEfiShellInterface->Argc;
        Argv = mEfiShellInterface->Argv;
    } else {
        ShellStatus = SHELL_UNSUPPORTED;
        goto Error_exit;
    }
    #if DEBUG_MODE
    {
        Print(L"Argc = %u\n", Argc);
        for (UINTN i = 0; i < Argc; i++) {
            Print(L"Argv[%u] = '%s'\n", i, Argv[i]);
        }
    }
    #endif

    // determine number of required parameters if any
    UINTN TableParamCount = 0;
    if (ParamTable) {
        while (ParamTable[TableParamCount].ValueType != VALTYPE_NONE) {
            TableParamCount++;
        }
    }
    // check manatory parameter count
    if (ManParamCount > TableParamCount) {
        ManParamCount = TableParamCount;
    }

    // use cmd line parameter for program name if non specified
    if (!g_ProgName) {
        g_ProgName = GetFileName(Argv[0]);
    }

    // check if break requested, ignoring all other options
    if (!(FuncOpt & NO_BREAK)) {
        ShellSetPageBreakMode(FALSE);
        for (UINTN i = 0; i < Argc; i++) {
            if (((StriCmp(Argv[i], g_BreakSwStr1) == 0) || (StriCmp(Argv[i], g_BreakSwStr2) == 0))) {
                ShellSetPageBreakMode(TRUE);
                break;
            }
        }
    }

    // check if help requested, ignoring all other options
    if (!(FuncOpt & NO_HELP)) {
        for (UINTN i = 0; i < Argc; i++) {
            if (((StriCmp(Argv[i], g_HelpSwStr1) == 0) || (StriCmp(Argv[i], g_HelpSwStr2) == 0))) {
                ShowHelp(ManParamCount, ParamTable, SwTable, ProgHelpStr, FuncOpt);
                ShellStatus = SHELL_ABORTED;
                goto Error_exit;
            }
        }
    }

    // parse cmd line arguments
    UINTN ParamCount = 0;
    UINTN ArgNum = 1;
    while (ArgNum < Argc) {
        // SWITCHES
        if ((Argv[ArgNum][0] == L'/') || (Argv[ArgNum][0] == L'-')) {
            if (((StriCmp(Argv[ArgNum], g_BreakSwStr1) == 0) || (StriCmp(Argv[ArgNum], g_BreakSwStr2) == 0))) {
                // ignore break switch as handled previously
                ArgNum++;
                continue;
            }
            UINTN i = 0;
            BOOLEAN found = FALSE;
            CHAR16* SwStr = NULL; // used to record switch name incase of no value
            while ((SwTable[i].SwitchNecessity != NO_SW)) {
                if ( (SwTable[i].SwStr1) && (StriCmp(Argv[ArgNum], SwTable[i].SwStr1) == 0) ) {
                    found = TRUE;
                    SwStr = SwTable[i].SwStr1;
                    break;
                }
                if ( (SwTable[i].SwStr2) && (StriCmp(Argv[ArgNum], SwTable[i].SwStr2) == 0) ) {
                    found = TRUE;
                    SwStr = SwTable[i].SwStr2;
                    break;
                }
                i++;
                if (i >= MAX_SWITCH_ENTRIES) {
                    TableError(i, L"Exceeded maximum switch count");
                    return SHELL_OUT_OF_RESOURCES;
                }
            }
            if (!found) {
                ShellPrintEx(-1, -1, L"%H%s%N: Unrecognised switch - '%H%s%N'\r\n", g_ProgName, Argv[ArgNum]);
                goto Error_exit;
            }
            if (SwPresent[i]) {
                ShellPrintEx(-1, -1, L"%H%s%N: Duplicate switch - '%H%s%N'\r\n", g_ProgName, SwStr);
                goto Error_exit;
            }
            SwPresent[i] = TRUE;
            if (SwTable[i].ValueType == VALTYPE_NONE) {
                if (SwTable[i].Data.FlagValue) {
                    // flag with predefined value
                    *(SwTable[i].ValueRetPtr.pUintn) = SwTable[i].Data.FlagValue;
                } else {
                    // true/false flag 
                    *(SwTable[i].ValueRetPtr.pBoolean) = TRUE;
                }
            } else {
                // read switch value
                if (ArgNum + 1 == Argc) {
                    ShellPrintEx(-1, -1, L"%H%s%N: Switch '%H%s%N' requires a value\r\n", g_ProgName, SwStr);
                    goto Error_exit;
                }
                ArgNum++;
                if ((Argv[ArgNum][0] == L'/') || (Argv[ArgNum][0] == L'-')) {
                    ShellPrintEx(-1, -1, L"%H%s%N: Switch '%H%s%N' requires a value\r\n", g_ProgName, SwStr);
                    goto Error_exit;
                }
                if (SwTable[i].ValueRetPtr.pVoid == NULL) {
                    TableError(i, L"Switch: Null 'RetValPtr'");
                    goto Error_exit;
                }
                VALUE_STATUS ValStatus = ReturnValue(Argv[ArgNum], SwTable[i].ValueType, &SwTable[i].Data, SwTable[i].ValueRetPtr);
                if (ValStatus != VAL_OK) {
                    ValueError(ValStatus, SwStr, 0, Argv[ArgNum]);
                    goto Error_exit;
                }
            }
        } else { // PARAMETERS
            if (ParamCount >= TableParamCount) {
                ShellPrintEx(-1, -1, L"%H%s%N: Too many parameters, only %u required\r\n", g_ProgName, TableParamCount);
                goto Error_exit;
            }
            if (ParamTable[ParamCount].ValueRetPtr.pVoid == (VOID *)NULL) {
                TableError(ParamCount, L"Parameter: Null 'RetValPtr'");
                goto Error_exit;
            }
            VALUE_STATUS ValStatus = ReturnValue(Argv[ArgNum], ParamTable[ParamCount].ValueType, &ParamTable[ParamCount].Data, ParamTable[ParamCount].ValueRetPtr);
            if (ValStatus != VAL_OK) {
                ValueError(ValStatus, NULL, ParamCount + 1, Argv[ArgNum]);
                goto Error_exit;
            }
            ParamCount++;
            if (NumParams) {
                *NumParams = ParamCount; // update number of actual parameters 
            }
        }
        // next cmd line argument
        ArgNum++;
    }

    // check parameter count
    if (ParamCount < ManParamCount) {
        ShellPrintEx(-1, -1, L"%H%s%N: Too few parameters, at least %u required\r\n", g_ProgName, ManParamCount);
        goto Error_exit;
    }

    // check mandatory switches
    UINTN i = 0;
    while (SwTable[i].SwitchNecessity != NO_SW) {
        if (SwTable[i].SwitchNecessity == MAN_SW && !SwPresent[i]) {
            ShellPrintEx(-1, -1, L"%H%s%N: Missing switch - '%H%s%N'\r\n", g_ProgName, SwTable[i].SwStr1);
            goto Error_exit;
        }
        i++;
    }

    ShellStatus = SHELL_SUCCESS;

Error_exit:

    return ShellStatus;
}

/**
 * Function: ValueError
 *
 **/
STATIC VOID ValueError(IN VALUE_STATUS ValStatus, IN CONST CHAR16 *SwStr, IN UINTN ParamNum, IN CONST CHAR16 *ValString)
{
    if (ValStatus == VAL_OK || !ValString) {
        return;
    }
    if (!SwStr && !ParamNum) {
        return;
    }
    CHAR16* ErrorStr;
    switch (ValStatus) {
        case VAL_STR_TRUNCATED:  ErrorStr = L"has its string truncated"; break;
        case VAL_DEC_INVALID:    ErrorStr = L"has invalid decimal value"; break;
        case VAL_HEX_INVALID:    ErrorStr = L"has invalid hex value"; break;
        case VAL_INT_INVALID:    ErrorStr = L"has invalid integer value"; break;
        case VAL_UINT8_TOO_BIG:  ErrorStr = L"has too large a number (8-bit)"; break;
        case VAL_UINT16_TOO_BIG: ErrorStr = L"has too large a number (16-bit)"; break;
        case VAL_UINT32_TOO_BIG: ErrorStr = L"has too large a number (32-bit)"; break;
        case VAL_OPT_INVALID:    ErrorStr = L"has invalid option"; break;
        default:                 ErrorStr = L"UNDEFINED ERROR"; break;
    }
    if (SwStr) {
        ShellPrintEx(-1, -1, L"%H%s%N: Switch '%H%s%N' %s - '%H%s%N'\r\n", g_ProgName, SwStr, ErrorStr, ValString);
    } else {
        ShellPrintEx(-1, -1, L"%H%s%N: Parameter '%H%u%N' %s - '%H%s%N'\r\n", g_ProgName, ParamNum, ErrorStr, ValString);
    }
}

/**
 * Function: ReturnValue
 * 
 **/
STATIC VALUE_STATUS ReturnValue(IN CONST CHAR16 *String, IN VALUE_TYPE ValueType, IN DATA *Data, OUT VALUE_RET_PTR ValueRetPtr)
{
    UINTN Value;
    VALUE_STATUS ValStatus;
    
    if ( !String || (!ValueRetPtr.pVoid) ) {
        return VAL_ERROR;
    }

    switch (ValueType) {
    case VALTYPE_STRING:
        StrnCpyS(ValueRetPtr.pChar16, Data->MaxStrSize, String, Data->MaxStrSize-1);
        if (StrLen(String) > Data->MaxStrSize-1) {
            return VAL_STR_TRUNCATED;
        }
        break;
    case VALTYPE_DECIMAL:
        if (!IsDecimalString(String)) {
            return VAL_DEC_INVALID;
        }
        Value = StrDecimalToUintn(String);
        ValStatus = ProcessIntVal(Value, Data->ValSize, ValueRetPtr);
        if (ValStatus != VAL_OK) {
            return ValStatus;
        }
        break;                
    case VALTYPE_HEXIDECIMAL:
        if (!IsHexString(String)) {
            return VAL_HEX_INVALID;
        }
        Value = StrHexToUintn(String);
        ValStatus = ProcessIntVal(Value, Data->ValSize, ValueRetPtr);
        if (ValStatus != VAL_OK) {
            return ValStatus;
        }
        break;
    case VALTYPE_INTEGER:
        if (HasHexPrefix(String)) {
            if (!IsHexString(String)) {
                return VAL_INT_INVALID; // hex
            }
            Value = StrHexToUintn(String);
        } else if (IsDecimalString(String)) {
            Value = StrDecimalToUintn(String);
        } else {
            return VAL_INT_INVALID; // decimal
        }
        ValStatus = ProcessIntVal(Value, Data->ValSize, ValueRetPtr);
        if (ValStatus != VAL_OK) {
            return ValStatus;
        }
        break;
    case VALTYPE_ENUM:
        if(GetEnumVal(Data->EnumStrArray, String, &Value)) {
            *ValueRetPtr.pEnum = (unsigned int)Value;
        } else {
            return VAL_OPT_INVALID;
        }
        break;
    default:
        return VAL_UNSUPPORTED_TYPE;
    }
    
    return VAL_OK;
}

/**
 * Function: ProcessIntVal
 *
 **/
STATIC VALUE_STATUS ProcessIntVal(IN UINTN Value, IN VALUE_SIZE ValSize, OUT VALUE_RET_PTR ValueRetPtr)
{
    switch (ValSize) {
    case SIZEN:
        *ValueRetPtr.pUintn = Value;
        break;
    case SIZE8:
        if (Value > MAX_UINT8) {
            return VAL_UINT8_TOO_BIG;
        }
        *ValueRetPtr.pUint8 = (UINT8)Value;
        break;
    case SIZE16:
        if (Value > MAX_UINT16) {
            return VAL_UINT16_TOO_BIG;
        }
        *ValueRetPtr.pUint16 = (UINT16)Value;
        break;
    case SIZE32:
        if (Value > MAX_UINT32) {
            return VAL_UINT32_TOO_BIG;
        }
        *ValueRetPtr.pUint32 = (UINT32)Value;
        break;
    default:
        ShellPrintEx(-1, -1, L"ERROR: Invalid decimal size\r\n");
        return VAL_UNSUPPORTED_SIZE;
    }
    return VAL_OK;
}

/**
 * Function: GetEnumVal
 * 
 **/
STATIC BOOLEAN GetEnumVal(IN ENUM_STR_ARRAY *EnumStrArray, IN CONST CHAR16 *Str, OUT UINTN *Value)
{
    BOOLEAN found = FALSE;
    UINTN i = 0;
    while (EnumStrArray[i].Str) {
        if (StriCmp(Str, EnumStrArray[i].Str) == 0) {
            if (Value) {
                *Value = EnumStrArray[i].Value;
            }
            found = TRUE;
            break;
        }
        i++;
    }
    return found;
}

/**
 * Function: StriCmp
 *
 **/
STATIC INTN EFIAPI StriCmp(IN CONST CHAR16* FirstString, IN CONST CHAR16* SecondString)
{
    CHAR16  UpperFirstString;
    CHAR16  UpperSecondString;

    UpperFirstString = CharToUpper(*FirstString);
    UpperSecondString = CharToUpper(*SecondString);
    while ((*FirstString != L'\0') && (*SecondString != L'\0') && (UpperFirstString == UpperSecondString)) {
        FirstString++;
        SecondString++;
        UpperFirstString = CharToUpper(*FirstString);
        UpperSecondString = CharToUpper(*SecondString);
    }

    return UpperFirstString - UpperSecondString;
}

/**
 * Function: HasHexPrefix
 *
 **/
STATIC BOOLEAN HasHexPrefix(IN CONST CHAR16* String)
{
    BOOLEAN LeadingZero = FALSE;

    while ((*String == L' ') || (*String == L'\t')) {
        String++;
    }
    while (*String == L'0') {
        LeadingZero = TRUE;
        String++;
    }
    if (CharToUpper(*String) == L'X') {
        if (!LeadingZero) {
            return FALSE;
        }
    }
    else {
        return FALSE;
    }
    return TRUE;
}

/**
 * Function: IsHexString
 *
 **/
STATIC BOOLEAN IsHexString(IN CONST CHAR16* String)
{
    BOOLEAN LeadingZero = FALSE;

    while ((*String == L' ') || (*String == L'\t')) {
        String++;
    }
    while (*String == L'0') {
        LeadingZero = TRUE;
        String++;
    }
    if (CharToUpper(*String) == L'X') {
        if (!LeadingZero) {
            return FALSE;
        }
        String++; // Skip the 'X'
    }
    while (InternalIsHexaDecimalDigitCharacter(*String)) {
        String++;
    }
    return *String == L'\0' ? TRUE : FALSE;
}

/**
 * Function: IsDecimalString
 *
 **/
STATIC BOOLEAN IsDecimalString(IN CONST CHAR16* String)
{
    while ((*String == L' ') || (*String == L'\t')) {
        String++;
    }
    while (InternalIsDecimalDigitCharacter(*String)) {
        String++;
    }
    return *String == L'\0' ? TRUE : FALSE;
}

/**
 * Function: GetFileName
 *
 **/
STATIC CONST CHAR16* GetFileName(CONST CHAR16* PathName)
{
    UINTN i = StrLen(PathName);
    while (i) {
        if (PathName[i] == L'\\') {
            return &PathName[i+1];
        }
        i--;
    }
    return PathName;
}

/**
 * TableError()
 * 
 **/
STATIC VOID TableError(IN UINTN i, IN CHAR16 *errStr)
{
    ShellPrintEx(-1, -1, L"TBLERR(%d): %s\n", i, errStr);
}

/**
 * ArgNameDefined()
 * 
 **/
STATIC BOOLEAN ArgNameDefined(IN CHAR16 *HelpStr)
{
    return HelpStr[0] == L'[' ? TRUE:FALSE;
}

/**
 * GetArgName()
 * 
 **/
STATIC UINTN GetArgName(IN CHAR16 *HelpStr, OUT CHAR16* ArgName, IN UINTN ArgNameSize, IN BOOLEAN Mandatory, IN CONST CHAR16 *DefaultArgName)
{
    UINTN HelpStartIdx = 0;             // start of help text
    CONST CHAR16 *ArgPtr = NULL;
    UINTN ArgLen = 0;

    if (ArgNameDefined(HelpStr)) {
        UINTN i = 1;  // skip start brace
        while (TRUE) {
            CHAR16 c = HelpStr[i];
            if (c == L']') {
                // valid argument name found
                ArgPtr = &HelpStr[1];   // char after '['
                ArgLen = i-1;
                break;
            }
            if (c == L'\0') {
                // no end brace for arg name
                break;
            }
            i++;
        }
        HelpStartIdx = i+1;
    }
    if (!ArgPtr && DefaultArgName) {
        // no argument name specified so use default
        ArgPtr = DefaultArgName;
        ArgLen = StrLen(DefaultArgName);
    }
    if (ArgPtr) {
        if (Mandatory) {
            StrnCpyS(ArgName, ArgNameSize, ArgPtr, (ArgLen >= ArgNameSize) ? ArgNameSize-1 : ArgLen);
        } else {
            // optional argument
            ArgName[0] = L'[';
            ArgName[1] = L'\0';
            StrnCatS(ArgName, ArgNameSize-1, ArgPtr, (ArgLen >= ArgNameSize-3) ? ArgNameSize-3 : ArgLen);
            StrCatS(ArgName, ArgNameSize, L"]");
        }
    } else {
        // no argument name specified or default given
        ArgName[0] = L'\0';
    }

    return HelpStartIdx;
}

/**
 * Function: ShowHelp
 * 
 **/

#define ARG_NAME_SIZE   24
#define PAD_SIZE        20

STATIC VOID ShowHelp(IN UINTN ManParamCount, IN PARAMETER_TABLE *ParamTable, IN SWITCH_TABLE *SwTable, IN CONST CHAR16 *ProgHelpStr, IN UINTN FuncOpt)
{
    CHAR16 ArgName[ARG_NAME_SIZE];
    UINTN HelpIdx;

    if (FuncOpt & NO_HELP) {
        return;
    }

    // initialise padding string
    CHAR16 pad[PAD_SIZE];
    for (UINTN i=0; i<PAD_SIZE; i++) {
        pad[i] = L' ';
    }
    pad[PAD_SIZE-1] = L'\0';

    // program description
    ShellPrintEx(-1, -1, L"\n");
    
    if (ProgHelpStr) {
        ShellPrintEx(-1, -1, L"%s\n\n", ProgHelpStr);
    }

    // usage
    if (g_ProgName) {
        ShellPrintEx(-1, -1, L"Usage: %s", g_ProgName);
    } else {
        ShellPrintEx(-1, -1, L"Usage: ");
    }
    UINTN i = 0;
    while (ParamTable[i].ValueType != VALTYPE_NONE) {
        GetArgName(ParamTable[i].HelpStr, ArgName, ARG_NAME_SIZE, (i+1 <= ManParamCount), g_DefaultArgName);
        ShellPrintEx(-1, -1, L" %s", ArgName);
        i++;
    }
    ShellPrintEx(-1, -1, L" [options]\n");

    // Parameter help
    if (ParamTable) {
        ShellPrintEx(-1, -1, L"\n Parameters:\n");
        i = 0;
        while (ParamTable[i].ValueType != VALTYPE_NONE) {
            HelpIdx = GetArgName(ParamTable[i].HelpStr, ArgName, ARG_NAME_SIZE, (i+1 <= ManParamCount), g_DefaultArgName);
            // get rid of spaces below ###
            ShellPrintEx(-1, -1, L"  %s%s     %s\n", ArgName, &pad[StrLen(ArgName)], &ParamTable[i].HelpStr[HelpIdx]);
            i++;
        }
    }
    // Switch help
    ShellPrintEx(-1, -1, L"\n Options:\n");
    if (SwTable) {
        i = 0;
        while (SwTable[i].SwitchNecessity != NO_SW) {
            HelpIdx = GetArgName(SwTable[i].HelpStr, ArgName, ARG_NAME_SIZE, TRUE, (SwTable[i].ValueType == VALTYPE_NONE) ? NULL : g_DefaultArgName);
            CHAR16 SeperatorChar = L',';
            CHAR16 *SwStr1 = SwTable[i].SwStr1;
            CHAR16 *SwStr2 = SwTable[i].SwStr2;
            if (!SwStr1) {
                SwStr1 = &pad[(PAD_SIZE-1)-2]; // 2 spaces for short switch
                SeperatorChar = L' ';
            }
            if (!SwStr2) {
                SwStr2 = &pad[PAD_SIZE-1]; // null str
                SeperatorChar = L' ';
            }
            UINTN TotalLen = StrLen(SwStr2) + StrLen(ArgName);
            CHAR16 *PadStr = (TotalLen > PAD_SIZE-1) ? &pad[(PAD_SIZE-1)-1] : &pad[TotalLen];
            ShellPrintEx(-1, -1, L"  %s%c %s %s%s%s", SwStr1, SeperatorChar, SwStr2, ArgName, PadStr, &SwTable[i].HelpStr[HelpIdx]);
            if (SwTable[i].ValueType == VALTYPE_ENUM) {
                // print all valid options for enum switches
                ShellPrintEx(-1, -1, L" (");
                UINTN j = 0;
                while (SwTable[i].Data.EnumStrArray[j].Str) {
                    ShellPrintEx(-1, -1, L"%s", SwTable[i].Data.EnumStrArray[j].Str);
                    j++;
                    if (SwTable[i].Data.EnumStrArray[j].Str) {
                        ShellPrintEx(-1, -1, L"|");
                    }
                }
                ShellPrintEx(-1, -1, L")");
            }
            ShellPrintEx(-1, -1, L"\n");
            i++;
        }
    }
    // break switch
    ShellPrintEx(-1, -1, L"  %s, %s %s%s\n", g_BreakSwStr1, g_BreakSwStr2, &pad[StrLen(g_BreakSwStr2)], g_BreakSwStr);
    // help switch
    ShellPrintEx(-1, -1, L"  %s, %s %s%s\n\n", g_HelpSwStr1, g_HelpSwStr2, &pad[StrLen(g_HelpSwStr2)], g_HelpSwStr);
}

/**
 * Function: CheckProgAbort
 * 
 **/
BOOLEAN CheckProgAbort(BOOLEAN PrintMsg)
{
    EFI_INPUT_KEY key;
    BOOLEAN abort = FALSE;
    while (!EFI_ERROR(gST->ConIn->ReadKeyStroke(gST->ConIn, &key))) {
        //Print(L"scancode=%04X char=%04X\n", key.ScanCode, key.UnicodeChar);
        if ( (key.ScanCode == 0x17) && (key.UnicodeChar== 0x00) ) { // ESC key
            abort = TRUE;
            if (PrintMsg) {
                ShellPrintEx(-1, -1, L"%H%s%N: User Aborted!\r\n", g_ProgName);
            }
            break;
        }
    }
    return abort;
}

