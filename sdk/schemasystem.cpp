#include "schemasystem.h"
#include "utils.hpp"
#include <cstring>

void CSchemaSystemTypeScope::FindDeclaredClass(SchemaClassInfoData_t*& pClassInfo, const char* pszClassName)
{
#if defined _WIN32 && _M_X64
    CallVFunc<void, 2, SchemaClassInfoData_t*&, const char*>(this, pClassInfo, pszClassName);
#else
    pClassInfo = CallVFunc<SchemaClassInfoData_t*, 2, const char*>(this, pszClassName);
#endif
}

CSchemaSystemTypeScope* CSchemaSystem::FindTypeScopeForModule(const char* szpModuleName)
{
    return CallVFunc<CSchemaSystemTypeScope*, 13, const char*, void*>(this, szpModuleName, nullptr);
}

CSchemaSystemTypeScope* CSchemaSystem::GetServerTypeScope()
{
    static CSchemaSystemTypeScope* pServerTypeScope = FindTypeScopeForModule(WIN_LINUX("server.dll", "libserver.so"));
    //static CSchemaSystemTypeScope* pServerTypeScope = FindTypeScopeForModule(WIN_LINUX("client.dll", "libserver.so"));
    
    return pServerTypeScope;
}

CSchemaSystemTypeScope* CSchemaSystem::GetClientTypeScope()
{
    static CSchemaSystemTypeScope* pClientTypeScope = FindTypeScopeForModule(WIN_LINUX("client.dll", "libclient.so"));

    return pClientTypeScope;
}

int32_t CSchemaSystem::GetClientOffset(const char* pszClassName, const char* pszPropName)
{
    SchemaClassInfoData_t* pClassInfo = nullptr;
    GetClientTypeScope()->FindDeclaredClass(pClassInfo, pszClassName);
    if (pClassInfo)
    {
        for (int i = 0; i < pClassInfo->m_iFieldsCount; i++)
        {
            auto& pFieldData = pClassInfo->m_pFieldsData[i];

            if (std::strcmp(pFieldData.m_pszName, pszPropName) == 0)
            {
                return pFieldData.m_iOffset;
            }
        }
    }

    return -1;
}


int32_t CSchemaSystem::GetServerOffset(const char* pszClassName, const char* pszPropName)
{
    SchemaClassInfoData_t* pClassInfo = nullptr;
    GetServerTypeScope()->FindDeclaredClass(pClassInfo, pszClassName);
    if (pClassInfo)
    {
        for (int i = 0; i < pClassInfo->m_iFieldsCount; i++)
        {
            auto& pFieldData = pClassInfo->m_pFieldsData[i];

            if (std::strcmp(pFieldData.m_pszName, pszPropName) == 0)
            {
                return pFieldData.m_iOffset;
            }
        }
    }

    return -1;
}
