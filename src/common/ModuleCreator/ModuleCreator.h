/*
 * Copyright (C)
 */

#ifndef _MODULE_CREATOR_H_
#define _MODULE_CREATOR_H_

#include "Common.h"

class K_COMMON_API ModuleCreator
{
public:
    static ModuleCreator* instance();

    void CreateModule(std::string const& moduleName);

private:
    bool CreateScriptLoader(std::string const& moduleName);
    bool CopyBaseModuleFiles();
    
    std::string GetScriptName(std::string str);
    std::string GetScriptLoaderFileName(std::string str);
    std::string GetCorrectModuleName(std::string str);
    std::string GetUpperText(std::string str);
    std::string GetHeadText();

    template<typename Format, typename... Args>
    inline void AddLineInText(std::string& text, Format&& fmt, Args&& ... args)
    {
        AddLineInText(text, Kargatum::StringFormat(std::forward<Format>(fmt), std::forward<Args>(args)...));
    }

    void CreateBaseArgs(std::string const& moduleName);
    void AddLineInText(std::string& text, std::string&& message);

    // Global variables
    std::string scriptName;
    std::string correctModuleName;
    std::string pathToModule;
    std::string defineText;
};

#define sModule ModuleCreator::instance()

#endif // _MODULE_CREATOR_H_
