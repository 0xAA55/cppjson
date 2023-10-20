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

	class UnicodeDecodeError : public JsonDecodeError
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

	class JsonData;
	class JsonObject;
	class JsonArray;
	class JsonString;
	class JsonNumber;
	class JsonBoolean;
	class JsonNull;
	class JsonParser;

	template<typename T> using JsonPtr = std::shared_ptr<T>;
	using JsonDataPtr = JsonPtr<JsonData>;
	using JsonObjectPtr = JsonPtr<JsonObject>;
	using JsonArrayPtr = JsonPtr<JsonArray>;
	using JsonStringPtr = JsonPtr<JsonString>;
	using JsonNumberPtr = JsonPtr<JsonNumber>;
	using JsonBooleanPtr = JsonPtr<JsonBoolean>;
	using JsonNullPtr = JsonPtr<JsonNull>;

	class JsonData
	{
	protected:
		JsonDataType Type;
		size_t LineNo;
		size_t Column;

		JsonData(JsonDataType Type, size_t FromLineNo = 0, size_t FromColumn = 0);

		static void AddIndent(std::stringstream& ss, int indent, const std::string& indent_type);
		static JsonDataPtr ParseJson(JsonParser& jp);

	public:
		JsonData() = delete;
		JsonData(const JsonData& c) = default;

		JsonDataType GetType() const;
		virtual std::string ToString(int indent = 0, int cur_indent = 0, const std::string& indent_type = " ") const = 0;
		virtual JsonDataPtr Copy() const = 0;

		static JsonDataPtr ParseJson(const std::string& s);

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

		bool operator ==(const JsonData& c) const;
		bool operator !=(const JsonData& c) const;

		JsonDataPtr& operator [] (const std::string& Key);
		const JsonDataPtr& at(const std::string& Key) const;

		operator JsonString () const;

		virtual JsonDataPtr& operator [] (const JsonString& Key) = 0;
		virtual const JsonDataPtr& at(const JsonString& Key) const = 0;
		virtual JsonDataPtr& operator [] (size_t Index) = 0;
		virtual const JsonDataPtr& at(size_t Index) const = 0;
		virtual operator double() const = 0;

		inline operator int8_t() const { return int8_t(operator double()); }
		inline operator int16_t() const { return int16_t(operator double()); }
		inline operator int32_t() const { return int32_t(operator double()); }
		inline operator int64_t() const { return int64_t(operator double()); }
		inline operator uint8_t() const { return uint8_t(operator double()); }
		inline operator uint16_t() const { return uint16_t(operator double()); }
		inline operator uint32_t() const { return uint32_t(operator double()); }
		inline operator uint64_t() const { return uint64_t(operator double()); }
	};

	using JsonObjectParentType = std::map<JsonString, JsonDataPtr>;
	using JsonArrayParentType = std::vector<JsonDataPtr>;

	class JsonObject : public JsonData, public JsonObjectParentType
	{
	public:
		JsonObject(size_t FromLineNo = 0, size_t FromColumn = 0);
		JsonObject(const JsonObjectParentType& c, size_t FromLineNo, size_t FromColumn);
		JsonObject(const JsonObject& c) = default;

		virtual std::string ToString(int indent = 0, int cur_indent = 0, const std::string& indent_type = " ") const override;
		virtual JsonDataPtr Copy() const override;

		bool operator ==(const JsonObject& c) const;
		bool operator !=(const JsonObject& c) const;

		JsonDataPtr& operator [] (const std::string& Key);
		const JsonDataPtr& at (const std::string& Key) const;

		virtual JsonDataPtr& operator [] (const JsonString& Key) override;
		virtual const JsonDataPtr& at(const JsonString& Key) const override;
		virtual JsonDataPtr& operator [] (size_t Index) override;
		virtual const JsonDataPtr& at(size_t Index) const override;
		virtual operator double() const override;
	};

	class JsonArray : public JsonData, public JsonArrayParentType
	{
	public:
		JsonArray(size_t FromLineNo = 0, size_t FromColumn = 0);
		JsonArray(const JsonArrayParentType& c, size_t FromLineNo, size_t FromColumn);
		JsonArray(const JsonArray& c) = default;

		virtual std::string ToString(int indent = 0, int cur_indent = 0, const std::string& indent_type = " ") const override;
		virtual JsonDataPtr Copy() const override;

		bool operator ==(const JsonArray& c) const;
		bool operator !=(const JsonArray& c) const;

		JsonDataPtr& operator [] (const std::string& Key);
		const JsonDataPtr& at(const std::string& Key) const;

		virtual JsonDataPtr& operator [] (const JsonString& Key) override;
		virtual const JsonDataPtr& at(const JsonString& Key) const override;
		virtual JsonDataPtr& operator [] (size_t Index) override;
		virtual const JsonDataPtr& at(size_t Index) const override;
		virtual operator double() const override;
	};

	class JsonString : public JsonData, public std::string
	{
	public:
		JsonString(size_t FromLineNo = 0, size_t FromColumn = 0);
		JsonString(const std::string& Value, size_t FromLineNo, size_t FromColumn);
		JsonString(const JsonString& c) = default;

		virtual std::string ToString(int indent = 0, int cur_indent = 0, const std::string& indent_type = " ") const override;
		virtual JsonDataPtr Copy() const override;

		bool operator ==(const JsonString& c) const;
		bool operator !=(const JsonString& c) const;

		virtual JsonDataPtr& operator [] (const JsonString& Key) override;
		virtual const JsonDataPtr& at(const JsonString& Key) const override;
		virtual JsonDataPtr& operator [] (size_t Index) override;
		virtual const JsonDataPtr& at(size_t Index) const override;
		virtual operator double() const override;
	};

	class JsonNumber : public JsonData
	{
	public:
		double Value;

		JsonNumber(size_t FromLineNo = 0, size_t FromColumn = 0);
		JsonNumber(std::int32_t Value, size_t FromLineNo, size_t FromColumn);
		JsonNumber(std::int64_t Value, size_t FromLineNo, size_t FromColumn);
		JsonNumber(std::uint32_t Value, size_t FromLineNo, size_t FromColumn);
		JsonNumber(std::uint64_t Value, size_t FromLineNo, size_t FromColumn);
		JsonNumber(float Value, size_t FromLineNo, size_t FromColumn);
		JsonNumber(double Value, size_t FromLineNo, size_t FromColumn);
		JsonNumber(const JsonNumber& c) = default;

		virtual std::string ToString(int indent = 0, int cur_indent = 0, const std::string& indent_type = " ") const override;
		virtual JsonDataPtr Copy() const override;

		bool operator ==(const JsonNumber& c) const;
		bool operator !=(const JsonNumber& c) const;

		virtual JsonDataPtr& operator [] (const JsonString& Key) override;
		virtual const JsonDataPtr& at(const JsonString& Key) const override;
		virtual JsonDataPtr& operator [] (size_t Index) override;
		virtual const JsonDataPtr& at(size_t Index) const override;
		virtual operator double() const override;
	};

	class JsonBoolean : public JsonData
	{
	public:
		bool Value;

		JsonBoolean(size_t FromLineNo = 0, size_t FromColumn = 0);
		JsonBoolean(bool Value, size_t FromLineNo, size_t FromColumn);
		JsonBoolean(const JsonBoolean& c) = default;

		operator bool() const { return Value; }
		virtual std::string ToString(int indent = 0, int cur_indent = 0, const std::string& indent_type = " ") const override;
		virtual JsonDataPtr Copy() const override;

		bool operator ==(const JsonBoolean& c) const;
		bool operator !=(const JsonBoolean& c) const;

		virtual JsonDataPtr& operator [] (const JsonString& Key) override;
		virtual const JsonDataPtr& at(const JsonString& Key) const override;
		virtual JsonDataPtr& operator [] (size_t Index) override;
		virtual const JsonDataPtr& at(size_t Index) const override;
		virtual operator double() const override;
	};

	class JsonNull : public JsonData
	{
	public:
		JsonNull(size_t FromLineNo = 0, size_t FromColumn = 0);
		JsonNull(const JsonNull& c) = default;

		virtual std::string ToString(int indent = 0, int cur_indent = 0, const std::string& indent_type = " ") const override;
		virtual JsonDataPtr Copy() const override;

		bool operator ==(const JsonNull& c) const;
		bool operator !=(const JsonNull& c) const;

		virtual JsonDataPtr& operator [] (const JsonString& Key) override;
		virtual const JsonDataPtr& at(const JsonString& Key) const override;
		virtual JsonDataPtr& operator [] (size_t Index) override;
		virtual const JsonDataPtr& at(size_t Index) const override;
		virtual operator double() const override;
	};


	template<class T, class ... Args>
	JsonPtr<T> MakeJsonPtr(Args && ... args)
	{ return std::make_shared<T>(args...); }

	template<class ... Args>
	JsonPtr<JsonData> MakeJsonDataPtr(Args && ... args)
	{ return MakeJsonPtr<JsonData>(args...); }

	template<class ... Args>
	JsonPtr<JsonObject> MakeJsonObjectPtr(Args && ... args)
	{ return MakeJsonPtr<JsonObject>(args...); }

	template<class ... Args>
	JsonPtr<JsonArray> MakeJsonArrayPtr(Args && ... args)
	{ return MakeJsonPtr<JsonArray>(args...); }

	template<class ... Args>
	JsonPtr<JsonString> MakeJsonStringPtr(Args && ... args)
	{ return MakeJsonPtr<JsonString>(args...); }

	template<class ... Args>
	JsonPtr<JsonNumber> MakeJsonNumberPtr(Args && ... args)
	{ return MakeJsonPtr<JsonNumber>(args...); }

	template<class ... Args>
	JsonPtr<JsonBoolean> MakeJsonBooleanPtr(Args && ... args)
	{ return MakeJsonPtr<JsonBoolean>(args...); }

	template<class ... Args>
	JsonPtr<JsonNull> MakeJsonNullPtr(Args && ... args)
	{ return MakeJsonPtr<JsonNull>(args...); }

	JsonDataPtr ParseJsonFromString(const std::string& s);
	JsonDataPtr ParseJsonFromFile(const std::string& FilePath);

	JsonDataPtr Copy(JsonDataPtr Json);
	JsonDataPtr Copy(const JsonData& Json);
}


