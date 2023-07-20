#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <stdexcept>
#include <cstdint>

namespace JsonLibrary
{
	using size_t = std::size_t;

	class JsonDecodeError : public std::runtime_error
	{
	protected:
		size_t LineNo;
		size_t Column;

	public:
		size_t GetLineNo() const;
		size_t GetColumn() const;
		JsonDecodeError(size_t FromLineNo, size_t FromColumn, const std::string& what) noexcept;
	};

	class UnicodeEncodeError : public std::runtime_error
	{
	public:
		UnicodeEncodeError(const std::string& what) noexcept;
	};

	class UnicodeDecodeError : public std::runtime_error
	{
	protected:
		size_t LineNo;
		size_t Column;

	public:
		size_t GetLineNo() const;
		size_t GetColumn() const;
		UnicodeDecodeError(size_t FromLineNo, size_t FromColumn, const std::string& what) noexcept;
	};

	class WrongDataType : public std::invalid_argument
	{
	protected:
		size_t LineNo;
		size_t Column;

	public:
		size_t GetLineNo() const;
		size_t GetColumn() const;
		WrongDataType(size_t FromLineNo, size_t FromColumn, const std::string& what) noexcept;
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
		size_t LineNo;
		size_t Column;

		JsonData(JsonDataType Type, size_t FromLineNo = 0, size_t FromColumn = 0);

		static void AddIndent(std::stringstream& ss, int indent, const std::string& indent_type);
		static std::unique_ptr<JsonData> ParseJson(JsonParser& jp);

	public:
		JsonData() = delete;
		JsonData(const JsonData& c) = delete;

		JsonDataType GetType() const;
		virtual std::string ToString(int indent = 0, int cur_indent = 0, const std::string& indent_type = " ") const = 0;

		static std::unique_ptr<JsonData> ParseJson(const std::string& s);

		size_t GetLineNo() const;
		size_t GetColumn() const;

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

	using JsonObjectParentType = std::map<JsonString, std::unique_ptr<JsonData>>;
	using JsonArrayParentType = std::vector<std::unique_ptr<JsonData>>;

	class JsonObject : public JsonData, public JsonObjectParentType
	{
	public:
		JsonObject(size_t FromLineNo = 0, size_t FromColumn = 0);
		JsonObject(const JsonObject& c);

		std::unique_ptr<JsonData>& operator [] (const std::string& key);

		virtual std::string ToString(int indent = 0, int cur_indent = 0, const std::string& indent_type = " ") const override;
	};

	class JsonArray : public JsonData, public JsonArrayParentType
	{
	public:
		JsonArray(size_t FromLineNo = 0, size_t FromColumn = 0);
		JsonArray(const JsonArray& c);

		virtual std::string ToString(int indent = 0, int cur_indent = 0, const std::string& indent_type = " ") const override;
	};

	class JsonString : public JsonData, public std::string
	{
	public:
		JsonString(size_t FromLineNo = 0, size_t FromColumn = 0);
		JsonString(const std::string& Value, size_t FromLineNo = 0, size_t FromColumn = 0);
		JsonString(const JsonString& c);

		virtual std::string ToString(int indent = 0, int cur_indent = 0, const std::string& indent_type = " ") const override;
	};

	class JsonNumber : public JsonData
	{
	public:
		double Value;

		JsonNumber(size_t FromLineNo = 0, size_t FromColumn = 0);
		JsonNumber(std::int32_t Value, size_t FromLineNo = 0, size_t FromColumn = 0);
		JsonNumber(std::int64_t Value, size_t FromLineNo = 0, size_t FromColumn = 0);
		JsonNumber(std::uint32_t Value, size_t FromLineNo = 0, size_t FromColumn = 0);
		JsonNumber(std::uint64_t Value, size_t FromLineNo = 0, size_t FromColumn = 0);
		JsonNumber(float Value, size_t FromLineNo = 0, size_t FromColumn = 0);
		JsonNumber(double Value, size_t FromLineNo = 0, size_t FromColumn = 0);
		JsonNumber(const JsonNumber& c);

		operator double() const { return Value; }
		virtual std::string ToString(int indent = 0, int cur_indent = 0, const std::string& indent_type = " ") const override;
	};

	class JsonBoolean : public JsonData
	{
	public:
		bool Value;

		JsonBoolean(size_t FromLineNo = 0, size_t FromColumn = 0);
		JsonBoolean(bool Value, size_t FromLineNo = 0, size_t FromColumn = 0);
		JsonBoolean(const JsonBoolean& c);

		operator bool() const { return Value; }
		virtual std::string ToString(int indent = 0, int cur_indent = 0, const std::string& indent_type = " ") const override;
	};

	class JsonNull : public JsonData
	{
	public:
		JsonNull(size_t FromLineNo = 0, size_t FromColumn = 0);
		JsonNull(const JsonNull& c);

		virtual std::string ToString(int indent = 0, int cur_indent = 0, const std::string& indent_type = " ") const override;
	};

	std::unique_ptr<JsonData> ParseJsonFromString(const std::string& s);
	std::unique_ptr<JsonData> ParseJsonFromFile(const std::string& FilePath);

	std::unique_ptr<JsonData> DuplicateJsonDataUnique(const JsonData& jd);
	std::shared_ptr<JsonData> DuplicateJsonDataShared(const JsonData& jd);
}


