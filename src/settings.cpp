#include "containerManager.h"
#include "settings.h"
#include "utility.h"

namespace {
	struct SwapData {
		bool hasError = false;
		bool hasBatData = false;
		std::string name;
		std::string changesError;
		std::string conditionsError;
		std::string conditionsPluginTypeError;
		std::vector<std::string> missingOldField;
		std::vector<std::string> missingNewField;
		std::vector<std::string> badStringField;
		std::vector<std::string> badStringFormat;
		std::vector<std::string> missingForm;
	};

	void ReadReport(SwapData a_report) {
		if (!a_report.hasError) return;

		_loggerInfo("{} has errors:", a_report.name);
		if (a_report.hasBatData) {
			_loggerInfo("Swap has bad data.");
		}

		if (!a_report.missingOldField.empty()) {
			_loggerInfo("Missing item for \"add\" field:");
			for (auto it : a_report.missingOldField) {
				_loggerInfo("    >{}", it);
			}
			_loggerInfo("");
		}

		if (!a_report.missingNewField.empty()) {
			_loggerInfo("Missing item for \"remove\" field:");
			for (auto it : a_report.missingNewField) {
				_loggerInfo("    >{}", it);
			}
			_loggerInfo("");
		}

		if (!a_report.missingNewField.empty()) {
			_loggerInfo("The following fields are not strings when they should be:");
			for (auto it : a_report.badStringField) {
				_loggerInfo("    >{}", it);
			}
			_loggerInfo("");
		}

		if (!a_report.missingNewField.empty()) {
			_loggerInfo("The following fields are not formatted as they should be:");
			for (auto it : a_report.badStringFormat) {
				_loggerInfo("    >{}", it);
			}
			_loggerInfo("");
		}

		if (!a_report.changesError.empty()) {
			_loggerInfo("Changes error.");
		}

		if (!a_report.conditionsError.empty()) {
			_loggerInfo("The condition field is unreadable, but present.");
		}

		if (!a_report.conditionsPluginTypeError.empty()) {
			_loggerInfo("The plugins field is unreadable, but present.");
		}

		if (!a_report.missingForm.empty()) {
			_loggerInfo("The following fields specify a form in a plugin that is loaded, but the form doesn't exist:");
			for (auto it : a_report.missingForm) {
				_loggerInfo("    >{}", it);
			}
			_loggerInfo("");
		}
	}
}

namespace Settings {
	void ReadConfig(Json::Value a_config, std::string a_reportName, SwapData* a_report) {
		if (!a_config.isObject()) return;
		auto& rules = a_config["rules"];
		if (!rules.isArray()) return;

		for (auto& data : rules) {
			if (!data.isObject()) {
				a_report->hasBatData = true;
				a_report->hasError = true;
				continue;
			}

			auto& friendlyName = data["friendlyName"];
			auto& conditions = data["conditions"];
			auto& changes = data["changes"];

			if (!friendlyName || !friendlyName.isString()) {
				a_report->hasBatData = true;
				a_report->hasError = true;
				continue;
			}
			auto friendlyNameString = friendlyName.asString();

			if (!changes || !changes.isArray()) {
				a_report->changesError = friendlyNameString;
				a_report->hasError = true;
				continue;
			}

			bool conditionsAreValid = true;
			std::string ruleName = friendlyName.asString(); ruleName += a_reportName;
			std::vector<std::string> validLocationKeywords;
			std::vector<RE::BGSLocation*> validLocationIdentifiers;
			std::vector<RE::TESObjectCONT*> validContainers;
			std::vector<RE::FormID> validReferences;

			if (conditions) {
				if (!conditions.isObject()) {
					a_report->conditionsError = friendlyNameString;
					a_report->hasError = true;
					conditionsAreValid = false;
					continue;
				}

				//Plugins check.
				//Will not read the rest of the config if at least one plugin is not present.
				auto& plugins = conditions["plugins"];
				if (plugins) {
					if (!plugins.isArray()) {
						a_report->conditionsPluginTypeError = friendlyNameString;
						a_report->hasError = true;
						conditionsAreValid = false;
						continue;
					}

					for (auto& plugin : plugins) {
						if (!plugin.isString()) {
							a_report->conditionsPluginTypeError = friendlyNameString;
							a_report->hasError = true;
							conditionsAreValid = false;
							continue;
						}

						if (!Utility::IsModPresent(plugin.asString())) {
							conditionsAreValid = false;
						}
					}
				} //End of plugins check

				//Container check.
				auto& containerField = conditions["containers"];
				if (containerField && containerField.isArray()) {
					for (auto& identifier : containerField) {
						if (!identifier.isString()) {
							std::string name = friendlyNameString; name += " -> containers";
							a_report->badStringField.push_back(name);
							a_report->hasError = true;
							conditionsAreValid = false;
							continue;
						}

						auto components = clib_util::string::split(identifier.asString(), "|"sv);
						if (components.size() != 2) {
							std::string name = friendlyNameString; name += " -> containers -> "; name += identifier.asString();
							a_report->badStringFormat.push_back(name);
							a_report->hasError = true;
							conditionsAreValid = false;
							continue;
						}
						auto* container = Utility::GetObjectFromMod<RE::TESObjectCONT>(components.at(0), components.at(1));
						if (!container) {
							if (Utility::IsModPresent(components.at(1))) {
								std::string name = friendlyNameString; name += " -> containers -> "; name += identifier.asString();
								a_report->missingForm.push_back(name);
								a_report->hasError = true;
								conditionsAreValid = false;
							}
							continue;
						}
						validContainers.push_back(container);
					}
				}

				//Location check.
				//If a location is null, it will not error.
				auto& locations = conditions["locations"];
				if (locations && locations.isArray()) {
					for (auto& identifier : locations) {
						if (!identifier.isString()) {
							std::string name = friendlyNameString; name += " -> locations";
							a_report->badStringField.push_back(name);
							a_report->hasError = true;
							conditionsAreValid = false;
							continue;
						}

						auto components = clib_util::string::split(identifier.asString(), "|"sv);
						if (components.size() != 2) {
							std::string name = friendlyNameString; name += " -> locations -> "; name += identifier.asString();
							a_report->badStringFormat.push_back(name);
							a_report->hasError = true;
							conditionsAreValid = false;
							continue;
						}
						auto* location = Utility::GetObjectFromMod<RE::BGSLocation>(components.at(0), components.at(1));
						if (!location) {
							if (Utility::IsModPresent(components.at(1))) {
								std::string name = friendlyNameString; name += " -> locations -> "; name += identifier.asString();
								a_report->missingForm.push_back(name);
								a_report->hasError = true;
								conditionsAreValid = false;
							}
							continue;
						}
						validLocationIdentifiers.push_back(location);
					}
				}

				//Location keywords.
				//No verification, for KID reasons.
				auto& locationKeywords = conditions["locationKeywords"];
				if (locationKeywords && locationKeywords.isArray()) {
					for (auto& identifier : locationKeywords) {
						if (!identifier.isString()) {
							std::string name = friendlyNameString; name += " -> locationKeywords";
							a_report->badStringField.push_back(name);
							a_report->hasError = true;
							conditionsAreValid = false;
							continue;
						}
						validLocationKeywords.push_back(identifier.asString());
					}
				}

				//Reference check.
				auto& referencesField = conditions["references"];
				if (referencesField && referencesField.isArray()) {
					for (auto& identifier : referencesField) {
						if (!identifier.isString()) {
							std::string name = friendlyNameString; name += " -> references";
							a_report->badStringField.push_back(name);
							a_report->hasError = true;
							conditionsAreValid = false;
							continue;
						}

						auto components = clib_util::string::split(identifier.asString(), "|"sv);
						if (components.size() != 2) {
							std::string name = friendlyNameString; name += " -> references -> "; name += identifier.asString();
							a_report->badStringFormat.push_back(name);
							a_report->hasError = true;
							conditionsAreValid = false;
							continue;
						}

						auto* file = RE::TESDataHandler::GetSingleton()->LookupModByName(components.at(1));
						if (!file) continue;
						validReferences.push_back(Utility::ParseFormID(identifier.asString()));
					}
				}
			} //End of conditions check
			if (!conditionsAreValid) continue;

			//Changes tracking here.
			for (auto& change : changes) {
				ContainerManager::SwapRule newRule;
				newRule.validLocations = validLocationIdentifiers;
				newRule.locationKeywords = validLocationKeywords;
				newRule.container = validContainers;
				newRule.references = validReferences;
				newRule.ruleName = ruleName;
				bool changesAreValid = true;

				auto& oldId = change["remove"];
				if (oldId) {
					if (!oldId.isString()) {
						std::string name = friendlyNameString; name += " -> remove";
						a_report->badStringField.push_back(name);
						a_report->hasError = true;
						changesAreValid = false;
						continue;
					}

					auto oldIdString = oldId.asString();
					auto components = clib_util::string::split(oldIdString, "|"sv);
					if (components.size() != 2) {
						std::string name = friendlyNameString; name += " -> remove -> "; name += oldIdString;
						a_report->badStringFormat.push_back(name);
						a_report->hasError = true;
						changesAreValid = false;
						continue;
					}

					auto* oldChangeForm = Utility::GetFormFromMod(components.at(0), components.at(1));
					if (!oldChangeForm || !oldChangeForm->IsBoundObject()) {
						std::string name = friendlyNameString; name += " -> remove -> "; name += oldIdString;
						a_report->missingOldField.push_back(name);
						a_report->hasError = true;
						changesAreValid = false;
						continue;
					}
					newRule.oldForm = static_cast<RE::TESBoundObject*>(oldChangeForm);
				} //Old object check.

				auto& newId = change["add"];
				if (newId) {
					if (!newId.isArray()) {
						std::string name = friendlyNameString; name += " -> add";
						a_report->badStringField.push_back(name);
						a_report->hasError = true;
						changesAreValid = false;
						continue;
					}

					for (auto& addition : newId) {
						if (!addition.isString()) {
							std::string name = friendlyNameString; name += " -> add";
							a_report->badStringField.push_back(name);
							a_report->hasError = true;
							changesAreValid = false;
							continue;
						}
						auto newIdString = addition.asString();
						auto newComponents = clib_util::string::split(newIdString, "|"sv);
						if (newComponents.size() != 2) {
							std::string name = friendlyNameString; name += " -> add -> "; name += newIdString;
							a_report->badStringFormat.push_back(name);
							a_report->hasError = true;
							changesAreValid = false;
							continue;
						}

						auto* newChangeForm = Utility::GetFormFromMod(newComponents.at(0), newComponents.at(1));
						if (!newChangeForm || !newChangeForm->IsBoundObject()) continue;
						auto* newForm = static_cast<RE::TESBoundObject*>(newChangeForm);
						newRule.newForm.push_back(newForm);
					}
				} //New ID Check
				if (!changesAreValid) continue;

				auto& removeKeyword = change["removeByKeyword"];
				if (removeKeyword) {
					if (!removeKeyword.isString()) {
						std::string name = friendlyNameString; name += " -> removeKeyword";
						a_report->badStringField.push_back(name);
						a_report->hasError = true;
						changesAreValid = false;
						continue;
					}

					newRule.removeKeyword = removeKeyword.asString();
				}

				auto& countField = change["count"];
				if (countField && countField.isInt()) {
					newRule.count = countField.asInt();
				}
				else {
					newRule.count = 1;
				}
				if (!(oldId || newId || removeKeyword)) continue;
				ContainerManager::ContainerManager::GetSingleton()->CreateSwapRule(newRule);
			} //End of changes check
		} //End of data loop
	} //End of read config

	bool ReadSettings() {
		std::vector<std::string> configFiles = std::vector<std::string>();
		try {
			configFiles = clib_util::distribution::get_configs(configPath, ""sv, ".json"sv);
		}
		catch (std::exception e) { //Directory not existing, for example?
			SKSE::log::warn("WARNING - caught exception {} while attempting to fetch configs.", e.what());
			return false;
		}
		if (configFiles.empty()) return true;

		std::vector<SwapData> reports;
		for (const auto& config : configFiles) {
			try {
				std::ifstream rawJSON(config);
				Json::Reader  JSONReader;
				Json::Value   JSONFile;
				JSONReader.parse(rawJSON, JSONFile);

				SwapData  report;
				std::string configName = config.substr(config.rfind("/") + 1, config.length() - 1);
				report.name = configName;
				ReadConfig(JSONFile, configName, &report);
				reports.push_back(report);
			}
			catch (std::exception e) { //Unlikely to be thrown, unless there are some weird characters involved.
				SKSE::log::warn("WARNING - caught exception {} while reading a file.", e.what());
			}
		}

		for (auto& report : reports) {
			ReadReport(report);
		}
		return true;
	}
}