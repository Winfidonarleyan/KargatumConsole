/*
 * Copyright (C)
 */

#ifndef _MODULE_CREATOR_H_
#define _MODULE_CREATOR_H_

#include "Define.h"
#include "StringFormat.h"

class WH_COMMON_API ModuleCreator
{
public:
    static ModuleCreator* instance();

    void CreateModule(std::string_view moduleName);

private:
    bool CreateBaseDirs();
    bool CreateScriptLoader(std::string_view moduleName);
    bool CreateClassFiles(std::string_view moduleName);
    bool CreateSCFile(std::string_view moduleName);
    bool CreateConfigFile(std::string_view moduleName);

    std::string GetScriptsName(std::string_view moduleName);
    std::string GetConfigFileName(std::string_view moduleName);
    std::string GetScriptLoaderFileName(std::string_view moduleName);
    std::string GetScriptCPPFileName(std::string_view moduleName);
    std::string GetSCCPPFileName(std::string_view moduleName);
    std::string GetScriptHFileName(std::string_view moduleName);
    std::string GetCorrectModuleName(std::string_view moduleName, bool sc = false);
    std::string GetDefineText(std::string_view moduleName);
    std::string GetHeadText(bool isCmakeFile = false, bool isHeader = false);

    bool CreateBaseArgs(std::string_view moduleName);
    bool AddTextInFile(std::string_view path, std::string_view text);

    // Global variables
    std::string scriptName;
    std::string correctModuleName;
    std::string correctSCModuleName;
    std::string pathToModule;
    std::string pathToModuleSrc;
    std::string defineText;
};

#define sModule ModuleCreator::instance()

#endif // _MODULE_CREATOR_H_
