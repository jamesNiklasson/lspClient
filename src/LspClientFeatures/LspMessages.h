// Part of the NPP LSP client
// Created on 24.05.2024 by JamesNiklasson
// Contains definitions of LSP messages
// generative AI (GPT3.5) was used to help generate these definitions from the LSP spec

#ifndef LSPMESSAGES_H
#define LSPMESSAGES_H

#include <string>
#include <variant>
#include <vector>

#include "..\nlohmann\json.hpp"
using json = nlohmann::json;

typedef struct {
	std::string uri;
	std::string name;
} WorkspaceFolder;

typedef enum {
	PlainText,
	Markdown
} MarkupKind;

typedef enum {
	AsIs = 1,
	AdjustIndentation = 2
} InsertTextMode;

typedef enum {
	Text = 1,
	Method = 2,
	Function = 3,
	Constructor = 4,
	Field = 5,
	Variable = 6,
	Class = 7,
	Interface = 8,
	Module = 9,
	Property = 10,
	Unit = 11,
	Value = 12,
	Enum = 13,
	Keyword = 14,
	Snippet = 15,
	Color = 16,
	File = 17,
	Reference = 18,
	Folder = 19,
	EnumMember = 20,
	Constant = 21,
	Struct = 22,
	Event = 23,
	Operator = 24,
	TypeParameter = 25
} CompletionItemKind;

typedef enum {
	utf_8,
	utf_16,
	utf_32
} PositionEncodingKind;

//InitializeParams
typedef struct {
	int processId;
	
	struct {
		std::string name;
		std::string version;
	} clientInfo;
	
	std::string locale;
	
	//ClientCapabilities
	struct {
		//Workspace capabilities
		struct {
			bool workspaceFolders;
			
			struct {
				bool dynamicRegistration;
				bool didCreate;
				bool willCreate;
				bool didRename;
				bool willRename;
				bool didDelete;
				bool willDelete;
			} fileOperations;
		} workspace;
		
		//TextDocumentClientCapabilities
		struct {
			//TextDocumentSyncClientCapabilities
			struct {
				bool dynamicRegistration;
				bool willSave;
				bool willSaveWaitUntil;
				bool didSave;
			} synchronization;
			
			//CompletionClientCapabilities
			struct {
				bool dynamicRegistration;
				
				struct {
					bool snippetSupport;
					bool commitCharactersSupport;
					std::vector<MarkupKind> documentationFormat;
					bool deprecatedSupport;
					bool preselectSupport;					
					bool insertReplaceSupport;
					
					struct {
						std::vector<std::string> properties;
					} resolveSupport;
					
					struct {
						std::vector<InsertTextMode> valueSet;
					} insertTextModeSupport;
					
					bool labelDetailsSupport;
				} completionItem;

				struct {
					std::vector<CompletionItemKind> valueSet;
				} completionItemKind;

				bool contextSupport;
				InsertTextMode insertTextMode;

				struct {
					std::vector<std::string> itemDefaults;
				} completionList;
			} completion;
		} textDocument;
		
		//Window capabilities
		struct {
			bool workDoneProgress;
			
			struct  {
				struct {
					bool additionalPropertiesSupport;
				} messageActionItem;
			} showMessage;
			
			struct  {
				bool support;
			} showDocument;
		} window;
		
		//General capabilities
		struct {
			struct {
				bool cancel;
				std::vector<std::string> retryOnContentModified;
			} staleRequestSupport;
			
			std::vector<PositionEncodingKind> positionEncodings;
		} general;
	} capabilities;
	
	std::vector<WorkspaceFolder> workspaceFolders;
} InitializeParams;


//this is hands-down the weirdest fucking fix I've ver seen, but it works :)
typedef struct {
	int id;
} AlternativeMessageId;

typedef std::variant<InitializeParams> MessageParams;
typedef std::variant<AlternativeMessageId, int, std::string> MessageId;

typedef struct {
	std::string jsonrpc;
	MessageId id;
	std::string method;
	MessageParams messageParams;
} RequestMessage;

void to_json(json& j, const MessageId& id);
void to_json(json& j, const WorkspaceFolder& workspaceFolder);
void to_json(json& j, const MarkupKind& markupKind);
void to_json(json& j, const InsertTextMode& insertTextMode);
void to_json(json& j, const CompletionItemKind& completionItemKind);
void to_json(json& j, const PositionEncodingKind& positionEncodingKind);
void to_json(json& j, const InitializeParams& initializeParams);
void to_json(json& j, const MessageParams& messageParams);
void to_json(json& j, const RequestMessage& requestMessage);

#endif