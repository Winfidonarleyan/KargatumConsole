/*
 * Copyright (C)
 */

#ifndef _MODULE_CREATOR_H_
#define _MODULE_CREATOR_H_

#include "Common.h"

class WH_COMMON_API ModuleCreator
{
public:
    static ModuleCreator* instance();

    void CreateModule(std::string const& moduleName, bool isWarheadModule = true);

private:
    bool CopyBaseModuleFiles();
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
    std::string GetCorrectModuleName(std::string str);
    std::string GetUpperText(std::string str);
    std::string GetHeadText(bool isSpecal = false);

    template<typename Format, typename... Args>
    inline void AddLineInText(std::string& text, Format&& fmt, Args&& ... args)
    {
        AddLineInText(text, Kargatum::StringFormat(std::forward<Format>(fmt), std::forward<Args>(args)...));
    }

    void CreateBaseArgs(std::string const& moduleName);
    void AddLineInText(std::string& text, std::string&& message);
    bool AddTextInFile(std::string const& path, std::string const& text);

    // Global variables
    std::string scriptName;
    std::string correctModuleName;
    std::string pathToModule;
    std::string pathToModuleSrc;
    std::string defineText;

    bool _IsWarheadModule = true;
};

#define sModule ModuleCreator::instance()

#endif // _MODULE_CREATOR_H_
