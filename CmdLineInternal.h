/***********************************************************************

 CmdLineInternal.h
 
 Author: David Petrovic
 GitHub: https://github.com/davepet1234/CmdLineLib

***********************************************************************/

#ifndef CMD_LINE_INTERNAL_H
#define CMD_LINE_INTERNAL_H
#ifdef __cplusplus
extern "C" {
#endif

#include <Uefi.h>

// Types
typedef enum { NO_SW, OPT_SW, MAN_SW, HELP_SW } SWITCH_NECESSITY;
typedef enum { VALTYPE_NONE, VALTYPE_STRING, VALTYPE_DECIMAL, VALTYPE_HEXIDECIMAL, VALTYPE_INTEGER, VALTYPE_ENUM } VALUE_TYPE;
typedef enum { SIZEN, SIZE8, SIZE16, SIZE32} VALUE_SIZE;
typedef enum { NO_VALUE, OPT_VALUE, MAN_VALUE } VALUE_NECESSITY;

// Struct to hold mapping of enum value to string for use with enum parameters and switches
typedef struct {
    UINTN Value;
    UINT16 *Str;
} ENUM_STR_ARRAY;

// Misc data used for both parameters and switches
typedef union {
    ENUM_STR_ARRAY *EnumStrArray;
    UINTN MaxStrSize;
    UINTN FlagValue;
    VALUE_SIZE ValSize;
} DATA;

// Ptr to return value
typedef union {
    BOOLEAN *pBoolean;
    UINTN *pUintn;
    UINT8* pUint8;
    UINT16* pUint16;
    UINT32* pUint32;
    UINT16* pUint64;
    CHAR16 *pChar16;
    unsigned int *pEnum;
    VOID *pVoid;
} VALUE_RET_PTR;


//---------------------------
// Parameter table
//---------------------------
typedef struct {
    VALUE_TYPE ValueType;
    DATA Data;
    VALUE_RET_PTR ValueRetPtr;
    CHAR16 *HelpStr;
} PARAMETER_TABLE;

//  generic parameter table entry
#define PARAMTABLE_ENTRY(ValueType, Data, ValueRetPtr, HelpStr) {ValueType, {Data}, {.pVoid=ValueRetPtr}, HelpStr},


//---------------------------
// Switch table
//---------------------------
#define MAX_SWITCH_ENTRIES  30

typedef struct {
    CHAR16 *SwStr1; // short switch
    CHAR16 *SwStr2; // long switch
    SWITCH_NECESSITY SwitchNecessity;
    VALUE_TYPE ValueType;
    DATA Data;
    BOOLEAN *PresentPtr;
    VALUE_RET_PTR ValueRetPtr;
    CHAR16 *HelpStr;
} SWITCH_TABLE;


// generic switch table entry
#define SWTABLE_ENTRY(SwStr1, SwStr2, SwitchNeccessity, ValueType, EnumArray, PresentPtr, ValueRetPtr, HelpStr) \
        { SwStr1, SwStr2, SwitchNeccessity, ValueType, EnumArray, PresentPtr, {.pVoid=ValueRetPtr}, HelpStr },


#ifdef __cplusplus
}
#endif
#endif // CMD_LINE_INTERNAL_H
