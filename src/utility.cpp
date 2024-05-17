#include "utility.h"

namespace Utility {
	bool IsModPresent(std::string a_modName) {
		auto* found = RE::TESDataHandler::GetSingleton()->LookupLoadedLightModByName(a_modName);
		if (!found) { found = RE::TESDataHandler::GetSingleton()->LookupLoadedModByName(a_modName); }
		if (found) return true;
		return false;
	}

	bool IsHex(std::string const& s) {
		return s.compare(0, 2, "0x") == 0
			&& s.size() > 2
			&& s.find_first_not_of("0123456789abcdefABCDEF", 2) == std::string::npos;
	}

	RE::FormID StringToFormID(std::string a_str) {
		RE::FormID result;
		std::istringstream ss{ a_str };
		ss >> std::hex >> result;
		return result;
	}
}