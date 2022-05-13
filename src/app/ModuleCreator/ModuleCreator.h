/*
 * Copyright (C)
 */

#ifndef _MODULE_CREATOR_H_
#define _MODULE_CREATOR_H_

#include "Common.h"
#include "StringFormat.h"

class WH_COMMON_API ModuleCreator
{
public:
    static ModuleCreator* instance();

    void CreateModule(std::string const& moduleName, bool isWarheadModule = true);

private:
    bool CreateBaseFiles();
    bool CreateScriptLoader(std::string const& moduleName);
    bool CreateClassFiles(std::string const& moduleName);
    bool CreateSCFile(std::string const& moduleName);
    bool CreateConfigFile(std::string const& moduleName);
    bool CreateCmakeFile(std::string const& moduleName);

    std::string GetScriptsName(std::string str);
    std::string GetConfigFileName(std::string str);
    std::string GetScriptLoaderFileName(std::string str);
    std::string GetScriptCPPFileName(std::string str);
    std::string GetSCCPPFileName(std::string str);
    std::string GetScriptHFileName(std::string str);
    std::string GetCorrectModuleName(std::string str, bool sc = false);
    std::string GetUpperText(std::string str);
    std::string GetHeadText(bool isWarhead = true, bool isSpecal = false);

    bool CreateBaseArgs(std::string const& moduleName);
    bool AddTextInFile(std::string_view path, std::string_view text);

    // Global variables
    std::string scriptName;
    std::string correctModuleName;
    std::string correctSCModuleName;
    std::string pathToModule;
    std::string pathToModuleSrc;
    std::string defineText;

    bool _IsWarheadModule = true;
};

#define sModule ModuleCreator::instance()

#endif // _MODULE_CREATOR_H_
