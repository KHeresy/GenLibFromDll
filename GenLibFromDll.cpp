// Standard C++ Libraries
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

// Windows Library
#include <process.h>

namespace FS = std::filesystem;

inline std::vector<std::string> tokenize(const std::string& sInput)
{
	std::vector<std::string> vTokens;
	if (sInput.length() > 1)
	{
		std::istringstream isData(sInput);
		std::string sToken;
		while (isData >> sToken)
		{
			vTokens.push_back(sToken);
		}
	}
	return vTokens;
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cout << "Give a DLL file name" << std::endl;
		return -1;
	}

	FS::path sDllFile = argv[1];
	if (FS::exists(sDllFile))
	{
		FS::path pathFolder = sDllFile.parent_path();
		FS::path sDefFile = pathFolder / (sDllFile.stem().string() + ".def");

		// Run DumpBin
		std::cout << "Dump information for [" << sDllFile << "] to [" << sDefFile << "]" << std::endl;
		std::string sCommand = "DUMPBIN " + sDllFile.string() + " /EXPORTS /OUT:" + sDefFile.string();
		system(sCommand.c_str());

		// modify .def file
		if (FS::exists(sDefFile))
		{
			std::vector<std::string> vFunctionDef;

			std::ifstream isDefDump(sDefFile.string());
			if (isDefDump)
			{
				uint32_t uLinePos = 0;
				std::string sLine, sToken;
				bool bStartProcessFunctiuons = false;
				while (std::getline(isDefDump, sLine)) {
					++uLinePos;
					auto vTokens = tokenize(sLine);

					if (bStartProcessFunctiuons)
					{
						if (vTokens.size() == 1 && vTokens[0] == "Summary")
						{
							std::cout << " > Finish at line " << uLinePos << std::endl;
							break;
						}
						else if(vTokens.size() == 4)
						{
							// Basic format: 1    0 00001360 LiquidVR
							vFunctionDef.push_back(vTokens[3]);
						}
						else if (vTokens.size() > 5 )
						{
							// debug format: 1    0 0000F911 ??0Client@aiaa@nvidia@@QEAA@$$QEAV012@@Z = @ILT+59660(??0Client@aiaa@nvidia@@QEAA@$$QEAV012@@Z)
							vFunctionDef.push_back(vTokens[3] + " " + vTokens[4] + " " + vTokens[5]);
						}
					}
					else
					{
						if (vTokens.size() == 4)
						{
							if (vTokens[0] == "ordinal" && vTokens[1] == "hint" && vTokens[2] == "RVA" && vTokens[3] == "name") {
								std::cout << " > Found header at line " << uLinePos << std::endl;
								bStartProcessFunctiuons = true;
							}
						}
					}
				}
				isDefDump.close();
			}

			// write def file for generate lib file
			std::cout << "Find " << vFunctionDef.size() << " functions\n";
			std::cout << "Update file [" << sDefFile << "]" << std::endl;
			std::ofstream osDefFile(sDefFile.string());
			if (osDefFile)
			{
				osDefFile << "EXPORTS\n\n";

				for (const auto& rFunction : vFunctionDef)
				{
					osDefFile << rFunction << "\n";
				}
				osDefFile.close();
			}

			// generate lib file
			FS::path sLibFile = pathFolder / (sDllFile.stem().string() + ".lib");
			std::string sCmdGenLib = "LIB /DEF:" + sDefFile.string() + " /machine:X64 /OUT:" + sLibFile.string();
			system(sCmdGenLib.c_str());
		}
	}
	else
	{
		std::cerr << "File [" << sDllFile << "] not exist" << std::endl;
		return 1;
	}
}
