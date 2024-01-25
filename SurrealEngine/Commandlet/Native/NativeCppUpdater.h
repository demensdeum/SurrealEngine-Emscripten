#pragma once

#include <filesystem>

class DebuggerApp;
class JsonValue;

class NativeCppUpdater
{
public:
	NativeCppUpdater(DebuggerApp* console) : Console(console) { }

	void Run();

private:
	struct NativeFunctionDecl
	{
		std::string args;
		std::vector<std::string> games;
	};

	struct NativeFunction
	{
		std::string name;
		std::vector<NativeFunctionDecl> decls;
		std::vector<std::pair<std::string, int>> versionIndex;
	};

	struct NativeProperty
	{
		std::string name;
		std::string type;
		std::vector<std::string> games;
	};

	struct NativeClass
	{
		std::string name;
		std::string package;
		std::vector<NativeFunction> funcs;
		std::vector<NativeProperty> props;

		NativeFunction& AddUniqueNativeFunction(const std::string& funcName);
		NativeProperty& AddUniqueNativeProperty(const std::string& propName);

		void ParseClassFunction(const std::string& funcName, const JsonValue& json, const std::string& version);
	};

	bool FindSourceCode();
	void ParseJsonFiles();
	void ParseClassNatives(const std::string& className, const std::string& packageName, const JsonValue& json, const std::string& version);
	void ParseClassProperties(const std::string& className, const std::string& packageName, const JsonValue& json, const std::string& version);
	void ParseGameNatives(const JsonValue& json, const std::string& version);
	void ParseGameProperties(const JsonValue& json, const std::string& version);

	NativeClass& AddUniqueNativeClass(const std::string& className);

	std::string UpdateBlock(const std::string& args, const std::string& block, const std::string& whitespaceprefix);

	DebuggerApp* Console = nullptr;
	std::filesystem::path SourceBasePath;
	std::vector<NativeClass> Classes;
};
