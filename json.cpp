#include "json.hpp"
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstdio>
// #include <format>

namespace JsonLibrary
{
	JsonDecodeError::JsonDecodeError(size_t FromLineNo, size_t FromColumn, const std::string& what) noexcept :
		LineNo(FromLineNo),
		Column(FromColumn),
		std::runtime_error(what)
	{
	}

	size_t JsonDecodeError::GetLineNo() const
	{
		return LineNo;
	}
	size_t JsonDecodeError::GetColumn() const
	{
		return Column;
	}

	UnicodeEncodeError::UnicodeEncodeError(const std::string& what) noexcept :
		std::runtime_error(what)
	{
	}

	UnicodeDecodeError::UnicodeDecodeError(size_t FromLineNo, size_t FromColumn, const std::string& what) noexcept :
		LineNo(FromLineNo),
		Column(FromColumn),
		JsonDecodeError(FromLineNo, FromColumn, what)
	{
	}

	size_t UnicodeDecodeError::GetLineNo() const
	{
		return LineNo;
	}
	size_t UnicodeDecodeError::GetColumn() const
	{
		return Column;
	}

	WrongDataType::WrongDataType(size_t FromLineNo, size_t FromColumn, const std::string& what) noexcept :
		LineNo(FromLineNo),
		Column(FromColumn),
		std::invalid_argument(what)
	{
	}

	size_t WrongDataType::GetLineNo() const
	{
		return LineNo;
	}
	size_t WrongDataType::GetColumn() const
	{
		return Column;
	}

	template<typename NumType, int N>
	std::string HexN(NumType num)
	{
		char buf[N + 4];
		switch (N)
		{
		case 0x01: snprintf(buf, sizeof buf, "%01X", num); return buf;
		case 0x02: snprintf(buf, sizeof buf, "%02X", num); return buf;
		case 0x03: snprintf(buf, sizeof buf, "%03X", num); return buf;
		case 0x04: snprintf(buf, sizeof buf, "%04X", num); return buf;
		case 0x05: snprintf(buf, sizeof buf, "%05X", num); return buf;
		case 0x06: snprintf(buf, sizeof buf, "%06X", num); return buf;
		case 0x07: snprintf(buf, sizeof buf, "%07X", num); return buf;
		case 0x08: snprintf(buf, sizeof buf, "%08X", num); return buf;
		case 0x09: snprintf(buf, sizeof buf, "%09X", num); return buf;
		case 0x0A: snprintf(buf, sizeof buf, "%010X", num); return buf;
		case 0x0B: snprintf(buf, sizeof buf, "%011X", num); return buf;
		case 0x0C: snprintf(buf, sizeof buf, "%012X", num); return buf;
		case 0x0D: snprintf(buf, sizeof buf, "%013X", num); return buf;
		case 0x0E: snprintf(buf, sizeof buf, "%014X", num); return buf;
		case 0x0F: snprintf(buf, sizeof buf, "%015X", num); return buf;
		case 0x10: snprintf(buf, sizeof buf, "%016X", num); return buf;
		default: snprintf(buf, sizeof buf, "%X", num); return buf;
		}
	}

	template<typename NumType>
	std::string Hex2(NumType num)
	{
		return HexN<NumType, 2>(num);
	}

	template<typename NumType>
	std::string Hex4(NumType num)
	{
		return HexN<NumType, 4>(num);
	}

	class Utf8Parser
	{
	protected:
		const std::string& s;
		std::string::const_iterator it;
		size_t LineNo;
		size_t Column;

	public:
		Utf8Parser() = delete;
		Utf8Parser(const std::string& s):
			s(s),
			it(s.cbegin()),
			LineNo(1),
			Column(1)
		{
		}

		size_t GetUtf8CharLen(uint8_t FirstByte)
		{
			if ((FirstByte & 0xFE) == 0xFC)//1111110x
			{
				return 6;
			}
			else if ((FirstByte & 0xFC) == 0xF8)//111110xx
			{
				return 5;
			}
			else if ((FirstByte & 0xF8) == 0xF0)//11110xxx
			{
				return 4;
			}
			else if ((FirstByte & 0xF0) == 0xE0)//1110xxxx
			{
				return 3;
			}
			else if ((FirstByte & 0xE0) == 0xC0)//110xxxxx
			{
				return 2;
			}
			else if ((FirstByte & 0xC0) == 0x80)//10xxxxxx
			{
				std::stringstream ss;
				ss << "can't decode byte 0x" << std::hex << static_cast<uint32_t>(FirstByte) << ": invalid start byte";
				throw UnicodeDecodeError(LineNo, Column, ss.str());
			}
			else if ((FirstByte & 0x80) == 0x00)//0xxxxxxx
			{
				return 1;
			}
			else
			{
				std::stringstream ss;
				ss << "can't decode byte 0x" << std::hex << static_cast<uint32_t>(FirstByte) << ": invalid start byte";
				throw UnicodeDecodeError(LineNo, Column, ss.str());
			}
		}

		static std::string EncodeUnicode(int Unicode)
		{
			char buf[8] = { 0 };
			char* ch = buf;
			if (Unicode < 0)
			{
				std::stringstream ss;
				ss << "can't encode 0x" << std::hex << static_cast<uint32_t>(Unicode) << ": invalid code point";
				throw UnicodeEncodeError(ss.str());
			}

			if (Unicode >= 0x4000000)
			{
				*ch++ = 0xFC | ((Unicode >> 30) & 0x01);
				*ch++ = 0x80 | ((Unicode >> 24) & 0x3F);
				*ch++ = 0x80 | ((Unicode >> 18) & 0x3F);
				*ch++ = 0x80 | ((Unicode >> 12) & 0x3F);
				*ch++ = 0x80 | ((Unicode >> 6) & 0x3F);
				*ch++ = 0x80 | (Unicode & 0x3F);
			}
			else if (Unicode >= 0x200000)
			{
				*ch++ = 0xF8 | ((Unicode >> 24) & 0x03);
				*ch++ = 0x80 | ((Unicode >> 18) & 0x3F);
				*ch++ = 0x80 | ((Unicode >> 12) & 0x3F);
				*ch++ = 0x80 | ((Unicode >> 6) & 0x3F);
				*ch++ = 0x80 | (Unicode & 0x3F);
			}
			else if (Unicode >= 0x10000)
			{
				*ch++ = 0xF0 | ((Unicode >> 18) & 0x07);
				*ch++ = 0x80 | ((Unicode >> 12) & 0x3F);
				*ch++ = 0x80 | ((Unicode >> 6) & 0x3F);
				*ch++ = 0x80 | (Unicode & 0x3F);
			}
			else if (Unicode >= 0x0800)
			{
				*ch++ = 0xE0 | ((Unicode >> 12) & 0x0F);
				*ch++ = 0x80 | ((Unicode >> 6) & 0x3F);
				*ch++ = 0x80 | (Unicode & 0x3F);
			}
			else if (Unicode >= 0x0080)
			{
				*ch++ = 0xC0 | ((Unicode >> 6) & 0x1F);
				*ch++ = 0x80 | (Unicode & 0x3F);
			}
			else
			{
				*ch++ = (char)Unicode;
			}
			return buf;
		}

		int PeekChar(std::string::const_iterator * next = nullptr)
		{
			std::string::const_iterator cur = it;
			size_t bytes;
			uint32_t ret;

			if ((cur[0] & 0xFE) == 0xFC)//1111110x
			{
				ret =
					(((uint32_t)cur[0] & 0x01) << 30) |
					(((uint32_t)cur[1] & 0x3F) << 24) |
					(((uint32_t)cur[2] & 0x3F) << 18) |
					(((uint32_t)cur[3] & 0x3F) << 12) |
					(((uint32_t)cur[4] & 0x3F) << 6) |
					(((uint32_t)cur[5] & 0x3F) << 0);
				bytes = 6;
			}
			else if ((cur[0] & 0xFC) == 0xF8)//111110xx
			{
				ret =
					(((uint32_t)cur[0] & 0x03) << 24) |
					(((uint32_t)cur[1] & 0x3F) << 18) |
					(((uint32_t)cur[2] & 0x3F) << 12) |
					(((uint32_t)cur[3] & 0x3F) << 6) |
					(((uint32_t)cur[4] & 0x3F) << 0);
				bytes = 5;
			}
			else if ((cur[0] & 0xF8) == 0xF0)//11110xxx
			{
				ret =
					(((uint32_t)cur[0] & 0x07) << 18) |
					(((uint32_t)cur[1] & 0x3F) << 12) |
					(((uint32_t)cur[2] & 0x3F) << 6) |
					(((uint32_t)cur[3] & 0x3F) << 0);
				bytes = 4;
			}
			else if ((cur[0] & 0xF0) == 0xE0)//1110xxxx
			{
				ret =
					(((uint32_t)cur[0] & 0x0F) << 12) |
					(((uint32_t)cur[1] & 0x3F) << 6) |
					(((uint32_t)cur[2] & 0x3F) << 0);
				bytes = 3;
			}
			else if ((cur[0] & 0xE0) == 0xC0)//110xxxxx
			{
				ret =
					(((uint32_t)cur[0] & 0x1F) << 6) |
					(((uint32_t)cur[1] & 0x3F) << 0);
				bytes = 2;
			}
			else if ((cur[0] & 0xC0) == 0x80)//10xxxxxx
			{
				std::stringstream ss;
				ss << "can't decode byte 0x" << Hex2(cur[0]) << ": invalid start byte";
				throw UnicodeDecodeError(LineNo, Column, ss.str());
			}
			else if ((cur[0] & 0x80) == 0x00)//0xxxxxxx
			{
				ret = cur[0] & 0x7F;
				bytes = 1;
			}
			else
			{
				std::stringstream ss;
				ss << "can't decode byte " << Hex2(cur[0]) << ": invalid byte";
				throw UnicodeDecodeError(LineNo, Column, ss.str());
			}

			if (next) *next = cur + bytes;
			return static_cast<int>(ret);
		}

		void UpdateLineNoColumn(int cur_char)
		{
			if (cur_char == '\n')
			{
				LineNo += 1;
				Column = 0;
			}
			Column += 1;
		}

		int GetChar()
		{
			int ch = PeekChar(&it);
			UpdateLineNoColumn(ch);
			return ch;
		}

		bool End() const { return it == s.cend(); }
		size_t GetLineNo() const { return LineNo; }
		size_t GetColumn() const { return Column; }
	};

	class JsonParser : public Utf8Parser
	{
	public:
		JsonParser() = delete;
		JsonParser(const std::string& s) : Utf8Parser(s)
		{
		}

		void SkipSpaces()
		{
			std::string::const_iterator n;
			if (End()) return;
			for (;;)
			{
				int ch = PeekChar(&n);
				if (isspace(ch))
				{
					UpdateLineNoColumn(ch);
					it = n;
					if (End()) break;
				}
				else break;
			}
		}

		void SkipUntilChar(int Char)
		{
			while (GetChar() != Char) if (End()) break;
		}

		void SkipSpacesAndComments()
		{
			std::string::const_iterator n;
			std::stringstream ss;

			for (;;)
			{
				SkipSpaces();
				if (End()) return;
				int ch = PeekChar(&n);
				if (ch != '/') return;
				UpdateLineNoColumn(ch);
				it = n;
				int next = GetChar();
				switch (next)
				{
				case '/': // Single line comment
					SkipUntilChar('\n');
					continue;
				case '*': /* Multiline comment */
					for (;;)
					{
						SkipUntilChar('*');
						if (End()) throw JsonDecodeError(LineNo, Column, "Expected */");
						ch = PeekChar(&n);
						if (ch == '/')
						{
							UpdateLineNoColumn(ch);
							it = n;
							break;
						}
					}
					continue;
				default:
					ss << "Unexpected " << EncodeUnicode(next);
					throw JsonDecodeError(LineNo, Column, ss.str());
				}
			}
		}

		std::string ParseString()
		{
			std::stringstream ss;
			for (;;)
			{
				int ch = GetChar();
				if (ch >= 0 && ch < 0x20) throw JsonDecodeError(LineNo, Column, "Invalid control character");
				switch (ch)
				{
				case '\\':
					if (1)
					{
						int ch2 = GetChar();
						std::stringstream buf;
						switch (ch2)
						{
						case '"': ss << '"'; break;
						case '\\': ss << '\\'; break;
						case '/': ss << '/'; break;
						case 'b': ss << '\b'; break;
						case 'f': ss << '\f'; break;
						case 'n': ss << '\n'; break;
						case 'r': ss << '\r'; break;
						case 't': ss << '\t'; break;
						case 'u':
							for (int i = 0; i < 4; i++)
							{
								int c = GetChar();
								if (!isxdigit(c)) throw JsonDecodeError(LineNo, Column, "Invalid \\escape");
								buf << static_cast<char>(c);
							}
							ss << EncodeUnicode(std::stoi(buf.str(), nullptr, 16));
							break;
						default:
							throw JsonDecodeError(LineNo, Column, "Invalid \\escape");
						}
					}
					break;
				default:
					ss << EncodeUnicode(ch);
				case '"':
					break;
				}
				if (ch == '"') break;
			}
			return ss.str();
		}

		JsonString ParseJsonString(size_t FromLineNo, size_t FromColumn)
		{
			return JsonString(ParseString(), FromLineNo, FromColumn);
		}

		JsonStringPtr ParseJsonStringPtr(size_t FromLineNo, size_t FromColumn)
		{
			return MakeJsonStringPtr(ParseString(), FromLineNo, FromColumn);
		}

		bool SkipDigits()
		{
			std::string::const_iterator n;
			bool SkippedDigit = false;
			for (;;)
			{
				int ch = PeekChar(&n);
				if (isdigit(ch))
				{
					SkippedDigit = true;
					UpdateLineNoColumn(ch);
					it = n;
				}
				else
				{
					break;
				}
			}
			return SkippedDigit;
		}

		double ParseNumber(char FirstChar)
		{
			std::stringstream ss;
			std::string::const_iterator n, si, ei;
			ss << FirstChar;

			si = it;

			bool isMinus = (FirstChar == '-');
			bool sd = SkipDigits();
			if (isMinus && !sd) throw JsonDecodeError(LineNo, Column, "Expected digit");

			int next = PeekChar(&n);
			if (next == '.')
			{
				UpdateLineNoColumn(next);
				it = n;
				sd = SkipDigits();
				if (!sd) throw JsonDecodeError(LineNo, Column, "Expected digit");
				next = PeekChar(&n);
			}
			if (next == 'e' || next == 'E')
			{
				UpdateLineNoColumn(next);
				it = n;
				next = PeekChar(&n);
				if (next == '-')
				{
					UpdateLineNoColumn(next);
					it = n;
				}
				sd = SkipDigits();
				if (!sd) throw JsonDecodeError(LineNo, Column, "Expected digit");
			}

			ei = it;
			for (auto i = si; i != ei; i++) ss << *i;
			return std::stod(ss.str());
		}

		JsonNumber ParseJsonNumber(char FirstChar, size_t FromLineNo, size_t FromColumn)
		{
			return JsonNumber(ParseNumber(FirstChar), FromLineNo, FromColumn);
		}

		JsonNumberPtr ParseJsonNumberUniquePtr(char FirstChar, size_t FromLineNo, size_t FromColumn)
		{
			return MakeJsonNumberPtr(ParseNumber(FirstChar), FromLineNo, FromColumn);
		}

		void ParseTrue()
		{
			JsonDecodeError ex = JsonDecodeError(LineNo, Column, "Error when decoding true");
			int r = GetChar();
			int u = GetChar();
			int e = GetChar();
			if (r != 'r' || u != 'u' || e != 'e') throw ex;
		}

		void ParseFalse()
		{
			JsonDecodeError ex = JsonDecodeError(LineNo, Column, "Error when decoding false");
			int a = GetChar();
			int l = GetChar();
			int s = GetChar();
			int e = GetChar();
			if (a != 'a' || l != 'l' || s != 's' || e != 'e') throw ex;
		}

		void ParseNull()
		{
			JsonDecodeError ex = JsonDecodeError(LineNo, Column, "Error when decoding null");
			int u = GetChar();
			int l1 = GetChar();
			int l2 = GetChar();
			if (u != 'u' || l1 != 'l' || l2 != 'l') throw ex;
		}
	};

	JsonData::JsonData(JsonDataType Type, size_t FromLineNo, size_t FromColumn) :
		Type(Type),
		LineNo(FromLineNo),
		Column(FromColumn)
	{
	}

	JsonDataType JsonData::GetType() const
	{
		return Type;
	}

	void JsonData::AddIndent(std::stringstream& ss, int indent, const std::string& indent_type)
	{
		for (int i = 0; i < indent; i++) ss << indent_type;
	}

	JsonDataPtr JsonData::ParseJson(JsonParser& jp)
	{
		jp.SkipSpacesAndComments();
		if (jp.End()) return nullptr;

		size_t CurLineNo = jp.GetLineNo();
		size_t CurColumn = jp.GetColumn();
		int cur = jp.GetChar();
		switch (cur)
		{
		case '{':
			if(1)
			{
				jp.SkipSpacesAndComments();
				auto ret = MakeJsonObjectPtr(CurLineNo, CurColumn);
				if (jp.PeekChar() == '}')
				{
					jp.GetChar();
					return ret;
				}
				for(;;)
				{
					jp.SkipSpacesAndComments();
					if (jp.GetChar() != '"') throw JsonDecodeError(jp.GetLineNo(), jp.GetColumn(), "Key name must be string");
					auto Key = jp.ParseJsonString(jp.GetLineNo(), jp.GetColumn());
					jp.SkipSpacesAndComments();
					if (jp.GetChar() != ':') throw JsonDecodeError(jp.GetLineNo(), jp.GetColumn(), "No ':' found");
					jp.SkipSpacesAndComments();
					(*ret)[Key] = ParseJson(jp);
					jp.SkipSpacesAndComments();
					auto comma = jp.GetChar();
					if (comma == '}') break;
					if (comma == ',') continue;
					std::stringstream ss;
					ss << "Unexpected '" << jp.EncodeUnicode(comma) << "'";
					throw JsonDecodeError(jp.GetLineNo(), jp.GetColumn(), ss.str());
				}
				return ret;
			}
		case '[':
			if (1)
			{
				jp.SkipSpacesAndComments();
				auto ret = MakeJsonArrayPtr(CurLineNo, CurColumn);
				if (jp.PeekChar() == ']')
				{
					jp.GetChar();
					return ret;
				}
				for (;;)
				{
					jp.SkipSpacesAndComments();
					ret->push_back(ParseJson(jp));
					jp.SkipSpacesAndComments();
					auto comma = jp.GetChar();
					if (comma == ']') break;
					if (comma == ',') continue;
					std::stringstream ss;
					ss << "Unexpected '" << jp.EncodeUnicode(comma) << "'";
					throw JsonDecodeError(jp.GetLineNo(), jp.GetColumn(), ss.str());
				}
				return ret;
			}
		case '"':
			return jp.ParseJsonStringPtr(CurLineNo, CurColumn);
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case '-':
			return jp.ParseJsonNumberUniquePtr(cur, CurLineNo, CurColumn);
		case 't':
			jp.ParseTrue();
			return MakeJsonBooleanPtr(true, CurLineNo, CurColumn);
		case 'f':
			jp.ParseFalse();
			return MakeJsonBooleanPtr(false, CurLineNo, CurColumn);
		case 'n':
			jp.ParseNull();
			return MakeJsonNullPtr(CurLineNo, CurColumn);
			break;
		}

		std::stringstream ss;
		ss << "Unexpected '" << jp.EncodeUnicode(cur) << "'";
		throw JsonDecodeError(CurLineNo, CurColumn, ss.str());
		return nullptr;
	}

	static std::string EscapeToUxxxx(int CodePoint)
	{
		std::stringstream ss;
		if (CodePoint < 0)
		{
			ss << "can't encode 0x" << std::hex << static_cast<uint32_t>(CodePoint) << ": invalid code point";
			throw UnicodeEncodeError(ss.str());
		}
		if (CodePoint > 0x10000)
		{
			// Process UTF-16 surrogate pair
			int first = CodePoint >> 10;
			int second = CodePoint & 0x3F;
			ss << "\\u" << EscapeToUxxxx(0xD800 | first);
			ss << "\\u" << EscapeToUxxxx(0xDC00 | second);
		}
		else
		{
			ss << "\\u" << Hex4(CodePoint);
		}
		return ss.str();
	}

	static std::string EscapeJsonString(const std::string& s)
	{
		std::stringstream ss;

		Utf8Parser jp(s);

		while (!jp.End())
		{
			int ch = jp.GetChar();
			if (ch < 0x20)
			{
				switch (ch)
				{
				case '\b': ss << "\\b"; break;
				case '\f': ss << "\\f"; break;
				case '\n': ss << "\\n"; break;
				case '\r': ss << "\\r"; break;
				case '\t': ss << "\\t"; break;
				default:
					ss << EscapeToUxxxx(ch);
					break;
				}
			}
			else if (ch >= 0x7f)
			{
				ss << EscapeToUxxxx(ch);
			}
			else
			{
				ss << Utf8Parser::EncodeUnicode(ch);
			}
		}
		return ss.str();
	}

	JsonObject::JsonObject(size_t FromLineNo, size_t FromColumn) :
		JsonData(JsonDataType::Object, FromLineNo, FromColumn)
	{
	}

	JsonObject::JsonObject(const JsonObjectParentType& c, size_t FromLineNo, size_t FromColumn) :
		JsonObjectParentType(c),
		JsonData(JsonDataType::Object, FromLineNo, FromColumn)
	{
	}

	std::string JsonObject::ToString(int indent, int cur_indent, const std::string& indent_type) const
	{
		std::stringstream ss;
		ss << "{";
		if (indent) ss << std::endl;
		cur_indent += indent;
		for (auto it = cbegin(); it != cend();)
		{
			AddIndent(ss, cur_indent, indent_type);
			ss << "\"" << EscapeJsonString(it->first) << "\":";
			if (indent) ss << " ";
			ss << it->second->ToString(indent, cur_indent, indent_type);
			it++;
			if (it != cend()) ss << ",";
			if (indent) ss << std::endl;
		}
		cur_indent -= indent;
		AddIndent(ss, cur_indent, indent_type);
		ss << "}";
		return ss.str();
	}

	JsonArray::JsonArray(size_t FromLineNo, size_t FromColumn) :
		JsonData(JsonDataType::Array, FromLineNo, FromColumn)
	{
	}

	JsonArray::JsonArray(const JsonArrayParentType& c, size_t FromLineNo, size_t FromColumn) :
		JsonData(JsonDataType::Array, FromLineNo, FromColumn),
		JsonArrayParentType(c)
	{
	}

	std::string JsonArray::ToString(int indent, int cur_indent, const std::string& indent_type) const
	{
		std::stringstream ss;
		ss << "[";
		if (indent) ss << std::endl;
		cur_indent += indent;
		for (size_t i = 0; i < size();)
		{
			AddIndent(ss, cur_indent, indent_type);
			ss << JsonArrayParentType::operator[](i)->ToString(indent, cur_indent, indent_type);
			i++;
			if (i < size()) ss << ",";
			if (indent) ss << std::endl;
		}
		cur_indent -= indent;
		AddIndent(ss, cur_indent, indent_type);
		ss << "]";
		return ss.str();
	}

	JsonString::JsonString(size_t FromLineNo, size_t FromColumn) :
		JsonData(JsonDataType::String, FromLineNo, FromColumn)
	{
	}

	JsonString::JsonString(const std::string& Value, size_t FromLineNo, size_t FromColumn) :
		JsonData(JsonDataType::String, FromLineNo, FromColumn),
		std::string(Value)
	{
	}

	std::string JsonString::ToString(int indent, int cur_indent, const std::string& indent_type) const
	{
		std::stringstream ss;
		ss << "\"" << EscapeJsonString(*this) << "\"";
		return ss.str();
	}

	JsonNumber::JsonNumber(size_t FromLineNo, size_t FromColumn) :
		JsonData(JsonDataType::Number, FromLineNo, FromColumn),
		Value(0)
	{
	}

	JsonNumber::JsonNumber(std::int32_t Value, size_t FromLineNo, size_t FromColumn) :
		JsonData(JsonDataType::Number, FromLineNo, FromColumn),
		Value(static_cast<double>(Value))
	{
	}
	JsonNumber::JsonNumber(std::int64_t Value, size_t FromLineNo, size_t FromColumn) :
		JsonData(JsonDataType::Number, FromLineNo, FromColumn),
		Value(static_cast<double>(Value))
	{
	}
	JsonNumber::JsonNumber(std::uint32_t Value, size_t FromLineNo, size_t FromColumn) :
		JsonData(JsonDataType::Number, FromLineNo, FromColumn),
		Value(static_cast<double>(Value))
	{
	}
	JsonNumber::JsonNumber(std::uint64_t Value, size_t FromLineNo, size_t FromColumn) :
		JsonData(JsonDataType::Number, FromLineNo, FromColumn),
		Value(static_cast<double>(Value))
	{
	}
	JsonNumber::JsonNumber(float Value, size_t FromLineNo, size_t FromColumn) :
		JsonData(JsonDataType::Number, FromLineNo, FromColumn),
		Value(static_cast<double>(Value))
	{
	}
	JsonNumber::JsonNumber(double Value, size_t FromLineNo, size_t FromColumn) :
		JsonData(JsonDataType::Number, FromLineNo, FromColumn),
		Value(Value)
	{
	}

	std::string JsonNumber::ToString(int indent, int cur_indent, const std::string& indent_type) const
	{
		char buf[256];
		char* chr = buf;
		char* chw = buf;
		char* pt = nullptr;
		bool DecimalNonZero = false;

		snprintf(buf, sizeof buf, "%lf", Value);
		for (; *chr; chr ++)
		{
			// 去除加号
			if (*chr != '+')
			{
				*chw = *chr;
				if (!pt)
				{
					// 找到小数点
					if (*chw == '.') pt = chw;
				}
				else
				{
					// 找到后看小数点后面是不是全都是零
					if (*chw != '0') DecimalNonZero = true;
				}
				chw++;
			}
		}
		// 小数点后面全都是零，从小数点这里截断字符串
		if (pt && !DecimalNonZero) *pt = 0;
		else *chw = 0;
		return buf;
	}

	JsonBoolean::JsonBoolean(size_t FromLineNo, size_t FromColumn) :
		JsonData(JsonDataType::Boolean, FromLineNo, FromColumn),
		Value(0)
	{
	}

	JsonBoolean::JsonBoolean(bool Value, size_t FromLineNo, size_t FromColumn) :
		JsonData(JsonDataType::Boolean, FromLineNo, FromColumn),
		Value(Value)
	{
	}

	std::string JsonBoolean::ToString(int indent, int cur_indent, const std::string& indent_type) const
	{
		if (Value) return "true";
		else return "false";
	}

	JsonNull::JsonNull(size_t FromLineNo, size_t FromColumn) :
		JsonData(JsonDataType::Null, FromLineNo, FromColumn)
	{
	}

	std::string JsonNull::ToString(int indent, int cur_indent, const std::string& indent_type) const
	{
		return "null";
	}

	size_t JsonData::GetLineNo() const
	{
		return LineNo;
	}

	size_t JsonData::GetColumn() const
	{
		return Column;
	}

	JsonObject& JsonData::AsJsonObject()
	{
		auto p = dynamic_cast<JsonObject*>(this);
		if (p) return *p; else throw WrongDataType(LineNo, Column, "Not a JSON object.");
	}

	JsonArray& JsonData::AsJsonArray()
	{
		auto p = dynamic_cast<JsonArray*>(this);
		if (p) return *p; else throw WrongDataType(LineNo, Column, "Not a JSON array.");
	}

	JsonString& JsonData::AsJsonString()
	{
		auto p = dynamic_cast<JsonString*>(this);
		if (p) return *p; else throw WrongDataType(LineNo, Column, "Not a JSON string.");
	}

	JsonNumber& JsonData::AsJsonNumber()
	{
		auto p = dynamic_cast<JsonNumber*>(this);
		if (p) return *p; else throw WrongDataType(LineNo, Column, "Not a JSON number.");
	}

	JsonBoolean& JsonData::AsJsonBoolean()
	{
		auto p = dynamic_cast<JsonBoolean*>(this);
		if (p) return *p; else throw WrongDataType(LineNo, Column, "Not a JSON boolean.");
	}

	const JsonObject& JsonData::AsJsonObject() const
	{
		auto p = dynamic_cast<const JsonObject*>(this);
		if (p) return *p; else throw WrongDataType(LineNo, Column, "Not a JSON object.");
	}

	const JsonArray& JsonData::AsJsonArray() const
	{
		auto p = dynamic_cast<const JsonArray*>(this);
		if (p) return *p; else throw WrongDataType(LineNo, Column, "Not a JSON array.");
	}

	const JsonString& JsonData::AsJsonString() const
	{
		auto p = dynamic_cast<const JsonString*>(this);
		if (p) return *p; else throw WrongDataType(LineNo, Column, "Not a JSON string.");
	}

	const JsonNumber& JsonData::AsJsonNumber() const
	{
		auto p = dynamic_cast<const JsonNumber*>(this);
		if (p) return *p; else throw WrongDataType(LineNo, Column, "Not a JSON number.");
	}

	const JsonBoolean& JsonData::AsJsonBoolean() const
	{
		auto p = dynamic_cast<const JsonBoolean*>(this);
		if (p) return *p; else throw WrongDataType(LineNo, Column, "Not a JSON boolean.");
	}

	bool JsonData::IsNull() const
	{
		auto p = dynamic_cast<const JsonNull*>(this);
		return p ? true : false;
	}

	JsonDataPtr JsonData::ParseJson(const std::string& s)
	{
		JsonParser jp(s);
		auto ret = ParseJson(jp);
		jp.SkipSpacesAndComments();
		if (!jp.End()) throw JsonDecodeError(jp.GetLineNo(), jp.GetColumn(), "Unexpected extra data");
		return ret;
	}

	bool JsonData::operator ==(const JsonData& c) const
	{
		if (Type != c.Type) return false;
		switch (c.GetType())
		{
		case JsonDataType::Object:
			return AsJsonObject() == c.AsJsonObject();
		case JsonDataType::Array:
			return AsJsonArray() == c.AsJsonArray();
		case JsonDataType::String:
			return AsJsonString() == AsJsonString();
		case JsonDataType::Number:
			return AsJsonNumber() == AsJsonNumber();
		case JsonDataType::Boolean:
			return AsJsonBoolean() == AsJsonBoolean();
		case JsonDataType::Null:
			return true; // 都是 true
		default:
			throw WrongDataType(c.GetLineNo(), c.GetColumn(), "Unknown JSON data type.");
			return false;
		}
	}

	bool JsonData::operator !=(const JsonData& c) const
	{
		return ! operator==(c);
	}

	bool JsonObject::operator ==(const JsonObject& c) const
	{
		// 数量得一致
		if (size() != c.size()) return false;

		for (auto& kv : *this)
		{
			auto& key = kv.first;

			// 每个 key 都得找得到
			if (!c.contains(key)) return false;

			// 每个 Item 得匹配
			if (*kv.second != *c.at(key)) return false;
		}

		return true;
	}

	bool JsonObject::operator !=(const JsonObject& c) const
	{
		return !operator==(c);
	}

	bool JsonArray::operator ==(const JsonArray& c) const
	{
		if (size() != c.size()) return false;

		for (size_t i = 0; i < size(); i++)
		{
			if (*at(i) != *c.at(i)) return false;
		}

		return true;
	}

	bool JsonArray::operator !=(const JsonArray& c) const
	{
		return !operator==(c);
	}

	bool JsonString::operator ==(const JsonString& c) const
	{
		return static_cast<const std::string&>(*this) == static_cast<const std::string&>(c);
	}
	bool JsonString::operator !=(const JsonString& c) const
	{
		return !operator==(c);
	}

	bool JsonNumber::operator ==(const JsonNumber& c) const
	{
		return Value == c.Value;
	}
	bool JsonNumber::operator !=(const JsonNumber& c) const
	{
		return !operator==(c);
	}

	bool JsonBoolean::operator ==(const JsonBoolean& c) const
	{
		return Value == c.Value;
	}
	bool JsonBoolean::operator !=(const JsonBoolean& c) const
	{
		return !operator==(c);
	}

	bool JsonNull::operator ==(const JsonNull& c) const
	{
		return true;
	}
	bool JsonNull::operator !=(const JsonNull& c) const
	{
		return !operator==(c);
	}

	JsonDataPtr& JsonData::operator [] (const std::string& key)
	{
		return operator [](JsonString(key, 0, 0));
	}

	const JsonDataPtr& JsonData::at(const std::string& key) const
	{
		return at(JsonString(key, 0, 0));
	}

	JsonData::operator JsonString () const
	{
		return JsonString(ToString(), LineNo, Column);
	}

	JsonDataPtr& JsonObject::operator [] (const std::string& key)
	{
		return JsonObjectParentType::operator[](JsonString(key, 0, 0));
	}

	const JsonDataPtr& JsonObject::at(const std::string& key) const
	{
		return JsonObjectParentType::at(JsonString(key, 0, 0));
	}

	JsonDataPtr& JsonObject::operator [] (const JsonString& Key)
	{
		return JsonObjectParentType::operator[](Key);
	}

	const JsonDataPtr& JsonObject::at(const JsonString& Key) const
	{
		return JsonObjectParentType::at(Key);
	}

	JsonDataPtr& JsonObject::operator [] (size_t Index)
	{
		char buf[128];
		snprintf(buf, sizeof buf, "%zu", Index);
		return operator [](buf);
	}

	const JsonDataPtr& JsonObject::at(size_t Index) const
	{
		char buf[128];
		snprintf(buf, sizeof buf, "%zu", Index);
		return at(buf);
	}

	JsonObject::operator double() const
	{
		throw WrongDataType(LineNo, Column, "Not a JSON number.");
	}

	JsonDataPtr& JsonArray::operator [] (const JsonString& Key)
	{
		return operator[](size_t(std::stoull(Key)));
	}

	JsonDataPtr& JsonArray::operator [] (const std::string& Key)
	{
		return operator[](size_t(std::stoull(Key)));
	}

	const JsonDataPtr& JsonArray::at(const std::string& Key) const
	{
		return at(size_t(std::stoull(Key)));
	}

	const JsonDataPtr& JsonArray::at(const JsonString& Key) const
	{
		return at(size_t(std::stoull(Key)));
	}

	JsonDataPtr& JsonArray::operator [] (size_t Index)
	{
		return JsonArrayParentType::operator[](Index);
	}

	const JsonDataPtr& JsonArray::at(size_t Index) const
	{
		return JsonArrayParentType::at(Index);
	}

	JsonArray::operator double() const
	{
		throw WrongDataType(LineNo, Column, "Not a JSON number.");
	}

	JsonDataPtr& JsonString::operator [] (const JsonString& Key)
	{
		throw WrongDataType(LineNo, Column, "Not a JSON object.");
	}

	const JsonDataPtr& JsonString::at(const JsonString& Key) const
	{
		throw WrongDataType(LineNo, Column, "Not a JSON object.");
	}

	JsonDataPtr& JsonString::operator [] (size_t Index)
	{
		throw WrongDataType(LineNo, Column, "Not a JSON array.");
	}

	const JsonDataPtr& JsonString::at(size_t Index) const
	{
		throw WrongDataType(LineNo, Column, "Not a JSON array.");
	}

	JsonString::operator double() const
	{
		return std::stod(*this);
	}

	JsonDataPtr& JsonNumber::operator [] (const JsonString& Key)
	{
		throw WrongDataType(LineNo, Column, "Not a JSON object.");
	}

	const JsonDataPtr& JsonNumber::at(const JsonString& Key) const
	{
		throw WrongDataType(LineNo, Column, "Not a JSON object.");
	}

	JsonDataPtr& JsonNumber::operator [] (size_t Index)
	{
		throw WrongDataType(LineNo, Column, "Not a JSON array.");
	}

	const JsonDataPtr& JsonNumber::at(size_t Index) const
	{
		throw WrongDataType(LineNo, Column, "Not a JSON array.");
	}

	JsonNumber::operator double() const
	{
		return Value;
	}

	JsonDataPtr& JsonBoolean::operator [] (const JsonString& Key)
	{
		throw WrongDataType(LineNo, Column, "Not a JSON object.");
	}

	const JsonDataPtr& JsonBoolean::at(const JsonString& Key) const
	{
		throw WrongDataType(LineNo, Column, "Not a JSON object.");
	}

	JsonDataPtr& JsonBoolean::operator [] (size_t Index)
	{
		throw WrongDataType(LineNo, Column, "Not a JSON array.");
	}

	const JsonDataPtr& JsonBoolean::at(size_t Index) const
	{
		throw WrongDataType(LineNo, Column, "Not a JSON array.");
	}

	JsonBoolean::operator double() const
	{
		return Value ? 1.0 : 0.0;
	}

	JsonDataPtr& JsonNull::operator [] (const JsonString& Key)
	{
		throw WrongDataType(LineNo, Column, "Not a JSON object.");
	}

	const JsonDataPtr& JsonNull::at(const JsonString& Key) const
	{
		throw WrongDataType(LineNo, Column, "Not a JSON object.");
	}

	JsonDataPtr& JsonNull::operator [] (size_t Index)
	{
		throw WrongDataType(LineNo, Column, "Not a JSON array.");
	}

	const JsonDataPtr& JsonNull::at(size_t Index) const
	{
		throw WrongDataType(LineNo, Column, "Not a JSON array.");
	}

	JsonNull::operator double() const
	{
		throw WrongDataType(LineNo, Column, "Not a JSON number.");
	}

	JsonDataPtr ParseJson(const std::string& s)
	{
		return JsonData::ParseJson(s);
	}

	JsonDataPtr ParseJsonFromString(const std::string& s)
	{
		return JsonData::ParseJson(s);
	}

	JsonDataPtr ParseJsonFromFile(const std::string& FilePath)
	{
		std::ifstream ifs(FilePath);
		std::stringstream ss;
		if (ifs.fail())
		{
			ss << "Could not read `" << FilePath << "`";
			throw JsonDecodeError(0, 0, ss.str());
		}
		ss << ifs.rdbuf();
		return JsonData::ParseJson(ss.str());
	}

	JsonDataPtr Copy(JsonDataPtr Json)
	{
		return Copy(*Json);
	}

	JsonDataPtr Copy(const JsonData& Json)
	{
		return Json.Copy();
	}

	JsonDataPtr JsonObject::Copy() const
	{
		return MakeJsonObjectPtr(*this);
	}

	JsonDataPtr JsonArray::Copy() const
	{
		return MakeJsonArrayPtr(*this);
	}

	JsonDataPtr JsonString::Copy() const
	{
		return MakeJsonStringPtr(*this);
	}

	JsonDataPtr JsonNumber::Copy() const
	{
		return MakeJsonNumberPtr(*this);
	}

	JsonDataPtr JsonBoolean::Copy() const
	{
		return MakeJsonBooleanPtr(*this);
	}

	JsonDataPtr JsonNull::Copy() const
	{
		return MakeJsonNullPtr(*this);
	}
}

