#pragma once
#include "CWeaponCSBaseVData.h"

class C_EconItemView {
public:
    SCHEMA_FIELD(uint32_t, CEconItemView, m_iAccountID); //public const nint m_iAccountID = 0x58; // uint32_t
    SCHEMA_FIELD(uint32_t, CEconItemView, m_iInventoryPosition);//public const nint m_iInventoryPosition = 0x5C; // uint32_t
    SCHEMA_FIELD(uint16_t, CEconItemView, m_iItemDefinitionIndex);//public const nint m_iItemDefinitionIndex = 0x38; // uint16_t
    SCHEMA_FIELD(int32_t, CEconItemView, m_iEntityQuality);//public const nint m_iEntityQuality = 0x3C; // int32_t
    SCHEMA_FIELD(uint32_t, CEconItemView, m_iEntityLevel);//public const nint m_iEntityLevel = 0x40; // uint32_t
    SCHEMA_FIELD(uint64_t, CEconItemView, m_iItemID);//public const nint m_iItemID = 0x48; // uint64_t
    SCHEMA_FIELD(uint32_t, CEconItemView, m_iItemIDHigh);//public const nint m_iItemIDHigh = 0x50; // uint32_t
    SCHEMA_FIELD(uint32_t, CEconItemView, m_iItemIDLow);//public const nint m_iItemIDLow = 0x54; // uint32_t
    SCHEMA_FIELD(bool, CEconItemView, m_bInitialized);//public const nint m_bInitialized = 0x68; // bool
    SCHEMA_FIELD(char, CEconItemView, m_szCustomName);//public const nint m_szCustomName = 0x130; // char[161]
    //MAYBE public const nint m_szCustomNameOverride = 0x1D1; // char[161]

    SCHEMA_FIELD(CAttributeList, CEconItemView, m_AttributeList);//public const nint m_AttributeList = 0x70; // CAttributeList
    SCHEMA_FIELD(CAttributeList, CEconItemView, m_NetworkedDynamicAttributes);//public const nint m_NetworkedDynamicAttributes = 0xD0; // CAttributeList

public:
    CCSWeaponBaseVData* GetCSWeaponDataFromItem();
    void SetAttributeValueByName(const char* name, float value);
    void SetAttributeValueIntByName(const char* name, int value);
};
