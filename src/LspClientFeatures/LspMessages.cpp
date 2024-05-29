// Part of the NPP LSP client
// Created on 24.05.2024 by JamesNiklasson
// Contains definitions of LSP messages and JSON conversion code

#include "LspMessages.h"
#include "Logging.h"
#include "..\nlohmann\json.hpp"
using json = nlohmann::json;

#include <string>
#include <variant>
#include <vector>

void to_json(json& j, const WorkspaceFolder& workspaceFolder) {
	j = json{
		{"uri", workspaceFolder.uri},
		{"name", workspaceFolder.name}
	};
}

void to_json(json& j, const MarkupKind& markupKind) {
	if (markupKind == PlainText) {
		j = json("plaintext");
	} else if (markupKind == Markdown) {
		j = json("markdown");
	}
}

void to_json(json& j, const InsertTextMode& insertTextMode) {
	j = json((int)insertTextMode);
}

void to_json(json& j, const CompletionItemKind& completionItemKind) {
	j = json((int)completionItemKind);
}

void to_json(json& j, const PositionEncodingKind& positionEncodingKind) {
	if (positionEncodingKind == utf_8) {
		j = json("utf-8");
	} else if (positionEncodingKind == utf_16) {
		j = json("utf-16");
	} else if (positionEncodingKind == utf_32) {
		j = json("utf-32");
	}
}

void to_json(json& j, const InitializeParams& initializeParams) {
	j = json{
		{"processId", initializeParams.processId},
		{"clientInfo", {
			{"name", initializeParams.clientInfo.name},
			{"version", initializeParams.clientInfo.version}
		}},
		{"locale", initializeParams.locale},
		{"capabilities", {
			{"workspace", {
				{"workspaceFolders", initializeParams.capabilities.workspace.workspaceFolders},
				{"fileOperations", {
					{"dynamicRegistration", initializeParams.capabilities.workspace.fileOperations.dynamicRegistration},
					{"didCreate", initializeParams.capabilities.workspace.fileOperations.didCreate},
					{"willCreate", initializeParams.capabilities.workspace.fileOperations.willCreate},
					{"didRename", initializeParams.capabilities.workspace.fileOperations.didRename},
					{"willRename", initializeParams.capabilities.workspace.fileOperations.willRename},
					{"didDelete", initializeParams.capabilities.workspace.fileOperations.didDelete},
					{"willDelete", initializeParams.capabilities.workspace.fileOperations.willDelete}
				}}
			}},
			{"textDocument", {
				{"synchronization", {
					{"dynamicRegistration", initializeParams.capabilities.textDocument.synchronization.dynamicRegistration},
					{"willSave", initializeParams.capabilities.textDocument.synchronization.willSave},
					{"willSaveWaitUntil", initializeParams.capabilities.textDocument.synchronization.willSaveWaitUntil},
					{"didSave", initializeParams.capabilities.textDocument.synchronization.didSave}
				}},
				{"completion", {
					{"dynamicRegistration", initializeParams.capabilities.textDocument.completion.dynamicRegistration},
					{"completionItem", {
						{"snippetSupport", initializeParams.capabilities.textDocument.completion.completionItem.snippetSupport},
						{"commitCharactersSupport", initializeParams.capabilities.textDocument.completion.completionItem.commitCharactersSupport},
						{"documentationFormat", initializeParams.capabilities.textDocument.completion.completionItem.documentationFormat},
						{"deprecatedSupport", initializeParams.capabilities.textDocument.completion.completionItem.deprecatedSupport},
						{"preselectSupport", initializeParams.capabilities.textDocument.completion.completionItem.preselectSupport},
						{"insertReplaceSupport", initializeParams.capabilities.textDocument.completion.completionItem.insertReplaceSupport},
						{"resolveSupport", {
							{"properties", initializeParams.capabilities.textDocument.completion.completionItem.resolveSupport.properties}
						}},
						{"insertTextModeSupport", {
							{"valueSet", initializeParams.capabilities.textDocument.completion.completionItem.insertTextModeSupport.valueSet}
						}},
						{"labelDetailsSupport", initializeParams.capabilities.textDocument.completion.completionItem.labelDetailsSupport}
					}},
					{"completionItemKind", {
						{"valueSet", initializeParams.capabilities.textDocument.completion.completionItemKind.valueSet}
					}},
					{"contextSupport", initializeParams.capabilities.textDocument.completion.contextSupport},
					{"insertTextMode", initializeParams.capabilities.textDocument.completion.insertTextMode},
					{"completionList", {
						{"itemDefaults", initializeParams.capabilities.textDocument.completion.completionList.itemDefaults}
					}}
				}}
			}},
			{"window", {
				{"workDoneProgress", initializeParams.capabilities.window.workDoneProgress},
				{"showMessage", {
					{"messageActionItem", {
						{"additionalPropertiesSupport", initializeParams.capabilities.window.showMessage.messageActionItem.additionalPropertiesSupport}
					}}
				}},
				{"showDocument", {
					{"support", initializeParams.capabilities.window.showDocument.support}
				}}
			}},
			{"general", {
				{"staleRequestSupport", {
					{"cancel", initializeParams.capabilities.general.staleRequestSupport.cancel},
					{"retryOnContentModified", initializeParams.capabilities.general.staleRequestSupport.retryOnContentModified}
				}}
			}}
		}},
		{"workspaceFolders", initializeParams.workspaceFolders}
	};
}

void to_json(json& j, const MessageParams& messageParams) {
	if (std::holds_alternative<InitializeParams>(messageParams)) {
		j = json(std::get<InitializeParams>(messageParams));
	}
}

void to_json(json& j, const MessageId& id) {
	if (std::holds_alternative<int>(id)) {
		j = json(std::get<int>(id));
	} else if (std::holds_alternative<std::string>(id)) {
		j = json(std::get<std::string>(id));
	} else {
		//throw a huge tantrum, but because the show must go on:
		logWrite("literally in shambles right now, a message ID that was neither int nor string was serialized as \"\"");
		j = json("");
	}
}

void to_json(json& j, const RequestMessage& requestMessage) {
	j = json{
		{"jsonrpc", requestMessage.jsonrpc},
		{"id", requestMessage.id},
		{"method", requestMessage.method},
		{"messageParams", requestMessage.messageParams}
	};
}