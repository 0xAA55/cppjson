#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <stdexcept>

namespace JsonLibrary
{
	class JsonDecodeError : public std::runtime_error
	{
	protected:
		size_t LineNo;
		size_t Column;

	public:
		JsonDecodeError(size_t LineNo, size_t Column, const std::string& what) noexcept;
	};

	class UnicodeEncodeError : public std::runtime_error
	{
	public:
		UnicodeEncodeError(const std::string& what) noexcept;
	};

	class UnicodeDecodeError : public std::runtime_error
	{
	public:
		UnicodeDecodeError(const std::string& what) noexcept;
	};

	class WrongDataType : public std::invalid_argument
	{
	public:
		WrongDataType(const std::string& what) noexcept;
	};

	enum class JsonDataType
	{
		Unknown,
		Object,
		Array,
		String,
		Number,
		Boolean,
		Null
	};

	class JsonObject;
	class JsonArray;
	class JsonString;
	class JsonNumber;
	class JsonBoolean;
	class JsonParser;

	class JsonData
	{
	protected:
		JsonDataType Type;
		JsonData(JsonDataType Type);

		static void AddIndent(std::stringstream& ss, int indent, const std::string& indent_type);
		static std::unique_ptr<JsonData> ParseJson(JsonParser& jp);

	public:
		JsonData() = delete;
		JsonData(const JsonData& c) = delete;

		JsonDataType GetType() const;
		virtual std::string ToString(int indent = 0, int cur_indent = 0, const std::string& indent_type = " ") const = 0;

		static std::unique_ptr<JsonData> ParseJson(const std::string& s);

		JsonObject& AsJsonObject();
		JsonArray& AsJsonArray();
		JsonString& AsJsonString();
		JsonNumber& AsJsonNumber();
		JsonBoolean& AsJsonBoolean();

		const JsonObject& AsJsonObject() const;
		const JsonArray& AsJsonArray() const;
		const JsonString& AsJsonString() const;
		const JsonNumber& AsJsonNumber() const;
		const JsonBoolean& AsJsonBoolean() const;

		bool IsNull() const;
	};

	using JsonObjectParentType = std::map<std::string, std::unique_ptr<JsonData>>;
	using JsonArrayParentType = std::vector<std::unique_ptr<JsonData>>;

	class JsonObject : public JsonData, public JsonObjectParentType
	{
	public:
		JsonObject();
		JsonObject(const JsonObject& c);

		virtual std::string ToString(int indent = 0, int cur_indent = 0, const std::string& indent_type = " ") const override;
	};

	class JsonArray : public JsonData, public JsonArrayParentType
	{
	public:
		JsonArray();
		JsonArray(const JsonArray& c);

		virtual std::string ToString(int indent = 0, int cur_indent = 0, const std::string& indent_type = " ") const override;
	};

	class JsonString : public JsonData, public std::string
	{
	public:
		JsonString();
		JsonString(const std::string& Value);
		JsonString(const JsonString& c);

		virtual std::string ToString(int indent = 0, int cur_indent = 0, const std::string& indent_type = " ") const override;
	};

	class JsonNumber : public JsonData
	{
	public:
		double Value;

		JsonNumber();
		JsonNumber(double Value);
		JsonNumber(const JsonNumber& c);

		operator double() const { return Value; }
		virtual std::string ToString(int indent = 0, int cur_indent = 0, const std::string& indent_type = " ") const override;
	};

	class JsonBoolean : public JsonData
	{
	public:
		bool Value;

		JsonBoolean();
		JsonBoolean(bool Value);
		JsonBoolean(const JsonBoolean& c);

		operator bool() const { return Value; }
		virtual std::string ToString(int indent = 0, int cur_indent = 0, const std::string& indent_type = " ") const override;
	};

	class JsonNull : public JsonData
	{
	public:
		JsonNull();
		JsonNull(const JsonNull& c);

		virtual std::string ToString(int indent = 0, int cur_indent = 0, const std::string& indent_type = " ") const override;
	};

	std::unique_ptr<JsonData> ParseJsonFromString(const std::string& s);
	std::unique_ptr<JsonData> ParseJsonFromFile(const std::string& FilePath);
}


