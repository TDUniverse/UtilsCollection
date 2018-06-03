
#include "JsonStthm.h"
#include <string>

// Experimental long/double parser
//#define STTHM_USE_CUSTOM_NUMERIC_PARSER

namespace JsonStthm
{
	//////////////////////////////
	// JsonValue::Iterator
	//////////////////////////////
	JsonValue::Iterator::Iterator(const JsonValue* pJson)
	{
		if (pJson != NULL && pJson->IsContainer())
		{
			m_pChild = pJson->m_oChilds.m_pFirst;
		}
		else
		{
			m_pChild = NULL;
		}
	}

	JsonValue::Iterator::Iterator(const Iterator& oIt)
	{
		m_pChild = oIt.m_pChild;
	}

	bool JsonValue::Iterator::IsValid() const
	{
		return m_pChild != NULL;
	}

	bool JsonValue::Iterator::operator!=(const Iterator& oIte) const
	{
		return m_pChild != oIte.m_pChild;
	}

	void JsonValue::Iterator::operator++()
	{
		if (m_pChild != NULL)
			m_pChild = m_pChild->m_pNext;
	}

	JsonValue* JsonValue::Iterator::operator*() const
	{
		return m_pChild;
	}

	JsonValue* JsonValue::Iterator::operator->() const
	{
		return m_pChild;
	}

	//////////////////////////////
	// JsonValue
	//////////////////////////////
	JsonValue JsonValue::INVALID = JsonValue::CreateConst();

	JsonValue::JsonValue()
		: m_bConst( false )
		, m_eType( E_TYPE_INVALID )
		, m_pName( NULL )
		, m_pNext( NULL )
	{
	}

	JsonValue::JsonValue(const JsonValue& oSource)
	{
		JsonValue();
		m_bConst = oSource.m_bConst;
		*this = oSource;
	}

	JsonValue::JsonValue(bool bValue)
	{
		JsonValue();
		*this = bValue;
	}

	JsonValue::JsonValue(const String& sValue)
	{
		JsonValue();
		*this = sValue;
	}

	JsonValue::JsonValue(const char* pValue)
	{
		JsonValue();
		*this = pValue;
	}

	JsonValue::JsonValue(long iValue)
	{
		JsonValue();
		*this = iValue;
	}

	JsonValue::JsonValue(double fValue)
	{
		JsonValue();
		*this = fValue;
	}

	JsonValue::~JsonValue()
	{
		StthmSafeFree(m_pName);
		Reset();
	}

	JsonValue JsonValue::CreateConst()
	{
		JsonValue oValue;
		oValue.m_bConst = true;
		return oValue;
	}

	void JsonValue::InitType(EType eType)
	{
		if (m_eType != eType)
		{
			Reset();

			m_eType = eType;
			switch (m_eType)
			{
			case E_TYPE_OBJECT:
			case E_TYPE_ARRAY:
				m_oChilds.m_pFirst = NULL;
				m_oChilds.m_pLast = NULL;
				break;
			case E_TYPE_STRING:
				m_pString = NULL;
				break;
			default:
				break;
			}
		}
	}

	JsonValue::EType JsonValue::GetType() const
	{
		return m_eType;
	}

	void JsonValue::Reset()
	{
		switch (m_eType)
		{
		case E_TYPE_OBJECT:
		case E_TYPE_ARRAY:
		{
			JsonValue* pChild = m_oChilds.m_pFirst;
			while (pChild != NULL)
			{
				JsonValue* pTemp = pChild->m_pNext;
				delete pChild;
				pChild = pTemp;
			}
			m_oChilds.m_pFirst = NULL;
			m_oChilds.m_pLast = NULL;
		}
		break;
		case E_TYPE_STRING:
			StthmSafeFree(m_pString);
			break;
		default:
			break;
		}
		m_eType = E_TYPE_INVALID;
	}

	void JsonValue::SetString(const char* pString)
	{
		StthmSafeFree(m_pString);
		if (NULL != pString)
		{
			size_t iLen  = sizeof(char) * (1 + strlen(pString));
			m_pString = (char*)StthmMalloc(iLen);
			memcpy(m_pString, pString, iLen);
		}
	}

	void JsonValue::Write(String& sOutJson, int iIndent, bool bCompact)
	{
		if (m_eType == E_TYPE_OBJECT)
		{
			String sIndent(iIndent, '\t');
			String sIndent2(iIndent + 1, '\t');
			sOutJson += "{";
			JsonValue* pChild = m_oChilds.m_pFirst;
			bool bFirst = true;
			while (pChild != NULL)
			{
				if (!bFirst)
				{
					sOutJson += ",";
				}
				else
				{
					bFirst = false;
				}

				if (!bCompact)
				{
					sOutJson += "\n";
					sOutJson += sIndent2;
				}
				
				sOutJson += "\"";
				WriteStringEscaped(sOutJson, pChild->m_pName);
				sOutJson += bCompact ? "\":" : "\": ";

				pChild->Write(sOutJson, iIndent + 1, bCompact);
				pChild = pChild->m_pNext;
			}
			if (!bCompact)
			{
				sOutJson += "\n";
				sOutJson += sIndent;
			}
			sOutJson += "}";
		}
		else if (m_eType == E_TYPE_ARRAY)
		{
			String sIndent(iIndent, '\t');
			String sIndent2(iIndent + 1, '\t');
			sOutJson += "[";
			JsonValue* pChild = m_oChilds.m_pFirst;
			bool bFirst = true;
			while (pChild != NULL)
			{
				if (!bFirst)
				{
					sOutJson += ",";
				}
				else
				{
					bFirst = false;
				}

				if (!bCompact)
				{
					sOutJson += "\n";
					sOutJson += sIndent2;
				}

				pChild->Write(sOutJson, iIndent + 1, bCompact);
				pChild = pChild->m_pNext;
			}
			if (!bCompact)
			{
				sOutJson += "\n";
				sOutJson += sIndent;
			}
			sOutJson += "]";
		}
		else if (m_eType == E_TYPE_STRING)
		{
			sOutJson += "\"";
			WriteStringEscaped(sOutJson, m_pString);
			sOutJson += "\"";
		}
		else if (m_eType == E_TYPE_BOOLEAN)
		{
			if (m_bBoolean)
			{
				sOutJson += "true";
			}
			else
			{
				sOutJson += "false";
			}
		}
		else if (m_eType == E_TYPE_INTEGER)
		{
			char sBuffer[256];
			sprintf_s(sBuffer, 256, "%d", m_iInteger);
			sOutJson += sBuffer;
		}
		else if (m_eType == E_TYPE_FLOAT)
		{
			char sBuffer[256];
			sprintf_s(sBuffer, 256, "%.17Lg", m_fFloat);
			sOutJson += sBuffer;
		}
		else
		{
			sOutJson += "null";
		}
	}

	void JsonValue::WriteStringEscaped(String& sOutJson, const String& sInput)
	{
		const char* pString = sInput.c_str();
		while (*pString != '\0')
		{
			char cChar = *pString;
			if (cChar == '\n')
				sOutJson += "\\n";
			else if (cChar == '\r')
				sOutJson += "\\r";
			else if (cChar == '\t')
				sOutJson += "\\t";
			else if (cChar == '\b')
				sOutJson += "\\b";
			else if (cChar == '\f')
				sOutJson += "\\f";
			else if (cChar == '"')
				sOutJson += "\\\"";
			else if (cChar == '\\')
				sOutJson += "\\\\";
			else if ((unsigned char)cChar < 0x80)
				sOutJson += cChar;
			else
			{
				sOutJson += "\\u";
				unsigned int iChar = (unsigned char)cChar;

				if ((iChar & 0xF0) == 0xF0) // 4 bytes
				{
					iChar = ((((unsigned char)*(pString)) & 0x07) << 18) + ((((unsigned char)*(pString + 1)) & 0x3F) << 12) + ((((unsigned char)*(pString + 2)) & 0x3F) << 6) + ((((unsigned char)*(pString + 3)) & 0x3F));
					pString += 3;
				}
				else if ((iChar & 0xF0) == 0xE0) // 3 bytes
				{
					iChar = ((((unsigned char)*(pString)) & 0x0F) << 12) + ((((unsigned char)*(pString + 1)) & 0x3F) << 6) + ((((unsigned char)*(pString + 2)) & 0x3F));
					pString += 2;
				}
				else if ((iChar & 0xF0) == 0xC0) // 2 byte
				{
					iChar = ((((unsigned char)*(pString)) & 0x1F) << 6) + ((((unsigned char)*(pString + 1)) & 0x3F));
					pString += 1;
				}
				

				char sHexa[5];
				const char* const  pHexa = "0123456789ABCDEF";
				sHexa[0] = pHexa[(iChar >> 12) & 0x0f];
				sHexa[1] = pHexa[(iChar >> 8) & 0x0f];
				sHexa[2] = pHexa[(iChar >> 4) & 0x0f];
				sHexa[3] = pHexa[(iChar) & 0x0f];
				sHexa[4] = '\0';
				
				sOutJson += sHexa;
			}

			++pString;
		}
	}

	void JsonValue::WriteString(String& sOutJson, bool bCompact)
	{
		Write(sOutJson, 0, bCompact);
	}

	bool JsonValue::WriteFile(const char* pFilename, bool bCompact)
	{
		FILE* pFile = fopen(pFilename, "w");
		if (NULL != pFile)
		{
			String sJson;
			WriteString(sJson, bCompact);
			bool bRet = fwrite(sJson.c_str(), sizeof(char), sJson.length(), pFile) == (sizeof(char) * sJson.length());
			fclose(pFile);
			return bRet;
		}
		return false;
	}

	int JsonValue::GetMemberCount() const
	{
		int iCount = 0;
		if (m_eType == E_TYPE_OBJECT || m_eType == E_TYPE_ARRAY)
		{
			JsonValue* pChild = m_oChilds.m_pFirst;
			while (pChild != NULL)
			{
				++iCount;
				pChild = pChild->m_pNext;
			}
		}

		return iCount;
	}

	const char* JsonValue::ToString() const
	{
		if (m_eType == E_TYPE_STRING)
			return m_pString;
		return NULL;
	}

	bool JsonValue::ToBoolean() const
	{
		if (m_eType == E_TYPE_BOOLEAN)
			return m_bBoolean;
		return false;
	}

	long JsonValue::ToInteger() const
	{
		if (m_eType == E_TYPE_INTEGER)
			return m_iInteger;
		else if (m_eType == E_TYPE_FLOAT)
			return (long)m_fFloat;
		return 0;
	}

	double JsonValue::ToFloat() const
	{
		if (m_eType == E_TYPE_FLOAT)
			return m_fFloat;
		else if (m_eType == E_TYPE_INTEGER)
			return (double)m_iInteger;
		return 0.0;
	}

	const JsonValue& JsonValue::operator[](const char* pName) const
	{
		if (m_eType == E_TYPE_OBJECT)
		{
			JsonValue* pChild = m_oChilds.m_pFirst;
			while (pChild != NULL)
			{
				if (strcmp(pChild->m_pName, pName) == 0)
					return *pChild;
				if (pChild->m_pNext == NULL)
					break;
				pChild = pChild->m_pNext;
			}
		}
		return JsonValue::INVALID;
	}

	JsonValue& JsonValue::operator[](const char* pName)
	{
		if (m_eType == E_TYPE_INVALID)
			InitType(E_TYPE_OBJECT);
		if (m_eType == E_TYPE_OBJECT)
		{
			JsonValue* pChild = m_oChilds.m_pFirst;
			while (pChild != NULL)
			{
				if (strcmp(pChild->m_pName, pName) == 0)
					return *pChild;
				if (pChild->m_pNext == NULL)
					break;
				pChild = pChild->m_pNext;
			}
			if (!m_bConst)
			{
				JsonValue* pNewMember = new JsonValue();
				pNewMember->m_pName = strdup(pName);
				
				if (NULL != m_oChilds.m_pLast)
					m_oChilds.m_pLast->m_pNext = pNewMember;
				else
					m_oChilds.m_pFirst = pNewMember;

				m_oChilds.m_pLast = pNewMember;
				return *pNewMember;
			}
		}
		return JsonValue::INVALID;
	}

	const JsonValue& JsonValue::operator [](int iIndex) const
	{
		if (m_eType == E_TYPE_OBJECT || m_eType == E_TYPE_ARRAY)
		{
			JsonValue* pChild = m_oChilds.m_pFirst;
			int iCurrent = 0;
			while (pChild != NULL)
			{
				if (iCurrent++ == iIndex)
					return *pChild;
				pChild = pChild->m_pNext;
			}
		}
		return JsonValue::INVALID;
	}

	JsonValue& JsonValue::operator[](int iIndex)
	{
		if (m_eType == E_TYPE_INVALID)
			InitType(E_TYPE_ARRAY);
		if (m_eType == E_TYPE_OBJECT || m_eType == E_TYPE_ARRAY)
		{
			JsonValue* pChild = m_oChilds.m_pFirst;
			int iCurrent = 0;
			while (pChild != NULL)
			{
				if (iCurrent++ == iIndex)
					return *pChild;
				if (pChild->m_pNext == NULL)
					break;
				pChild = pChild->m_pNext;
			}
			if (m_eType == E_TYPE_ARRAY)
			{
				do
				{
					JsonValue* pNewChild = new JsonValue();

					if (NULL != m_oChilds.m_pLast)
						m_oChilds.m_pLast->m_pNext = pNewChild;
					else
						m_oChilds.m_pFirst = pNewChild;

					m_oChilds.m_pLast = pNewChild;
				}
				while (iCurrent++ != iIndex);
				return *m_oChilds.m_pLast;
			}
		}
		return JsonValue::INVALID;
	}

	JsonValue& JsonValue::operator =(const JsonValue& oValue)
	{
		if (oValue.m_pName != NULL)
		{
			m_pName = strdup(oValue.m_pName);
		}

		if (oValue.m_eType == E_TYPE_OBJECT || oValue.m_eType == E_TYPE_ARRAY)
		{
			InitType(oValue.m_eType);
			
			JsonValue* pSourceChild = oValue.m_oChilds.m_pFirst;;
			while (pSourceChild != NULL)
			{
				JsonValue* pNewChild = new JsonValue(*pSourceChild);
				if (NULL != m_oChilds.m_pLast)
					m_oChilds.m_pLast->m_pNext = pNewChild;
				else
					m_oChilds.m_pFirst = pNewChild;

				m_oChilds.m_pLast = pNewChild;

				pSourceChild = pSourceChild->m_pNext;
			}
		}
		else if (oValue.m_eType == E_TYPE_BOOLEAN)
		{
			*this = (bool)oValue;
		}
		else if (oValue.m_eType == E_TYPE_STRING)
		{
			*this = (String)oValue;
		}
		else if (oValue.m_eType == E_TYPE_INTEGER)
		{
			*this = (long)oValue;
		}
		else if (oValue.m_eType == E_TYPE_FLOAT)
		{
			*this = (double)oValue;
		}
		return *this;
	}

	JsonValue& JsonValue::operator =(const String& sValue)
	{
		if (!m_bConst)
		{
			InitType(E_TYPE_STRING);
			SetString(sValue.c_str());
		}
		return *this;
	}

	JsonValue& JsonValue::operator =(const char* pValue)
	{
		if (!m_bConst)
		{
			if (NULL != pValue)
			{
				InitType(E_TYPE_STRING);
				SetString(pValue);
			}
			else
			{
				InitType(E_TYPE_INVALID);
			}
		}
		return *this;
	}

	JsonValue& JsonValue::operator =(bool bValue)
	{
		if (!m_bConst)
		{
			InitType(E_TYPE_BOOLEAN);
			m_bBoolean = bValue;
		}
		return *this;
	}

	JsonValue& JsonValue::operator =(long iValue)
	{
		if (!m_bConst)
		{
			InitType(E_TYPE_INTEGER);
			m_iInteger = iValue;
		}
		return *this;
	}

	JsonValue& JsonValue::operator =(double fValue)
	{
		if (!m_bConst)
		{
			InitType(E_TYPE_FLOAT);
			m_fFloat = fValue;
		}
		return *this;
	}

	JsonValue& JsonValue::operator +=(const JsonValue& oValue)
	{
		if (m_eType == E_TYPE_ARRAY)
		{
			JsonValue* pNewValue = new JsonValue(oValue);
			
			if (NULL != m_oChilds.m_pLast)
				m_oChilds.m_pLast->m_pNext = pNewValue;
			else
				m_oChilds.m_pFirst = pNewValue;
			
			m_oChilds.m_pLast = pNewValue;
		}
		else if (m_eType == E_TYPE_STRING)
		{
			if (oValue.IsString())
			{
				String sValue = m_pString;
				sValue += (String)oValue;
				SetString(sValue.c_str());
			}
		}
		return *this;
	}

	JsonValue::operator const char*() const
	{
		return ToString();
	}

	JsonValue::operator bool() const
	{
		return ToBoolean();
	}

	JsonValue::operator long() const
	{
		return ToInteger();
	}

	JsonValue::operator double() const
	{
		return ToFloat();
	}

	//Reading

	bool JsonValue::IsSpace(char cChar) {
		return cChar == ' ' || (cChar >= '\t' && cChar <= '\r');
	}

	bool JsonValue::IsDigit(char cChar)
	{
		return (cChar >= '0' && cChar <= '9');
	}

	bool JsonValue::IsXDigit(char cChar)
	{
		return (cChar >= '0' && cChar <= '9') || ((cChar & ~' ') >= 'A' && (cChar & ~' ') <= 'F') || ((cChar & ~' ') >= 'a' && (cChar & ~' ') <= 'f');
	}

	int	JsonValue::CharToInt(char cChar)
	{
		if (cChar <= '9')
			return cChar - '0';
		else
			return (cChar & ~' ') - 'A' + 10;
	}

	void JsonValue::SkipSpaces(const char*& pString)
	{
		while (IsSpace(*pString)) ++pString;
	}

	bool JsonValue::ReadSpecialChar(const char*& pString, CharBuffer& oTempBuffer)
	{
		if (*pString == 'n') oTempBuffer += '\n';
		else if (*pString == 'r') oTempBuffer += '\r';
		else if (*pString == 't') oTempBuffer += '\t';
		else if (*pString == 'b') oTempBuffer += '\b';
		else if (*pString == 'f') oTempBuffer += '\f';
		else if (*pString == '"') oTempBuffer += '"';
		else if (*pString == '\\') oTempBuffer += '\\';
		else if (*pString == 'u')
		{
			int iChar = 0;
			for (int i = 0; i < 4; ++i)
			{
				if (IsXDigit(*++pString))
					iChar = iChar * 16 + CharToInt((unsigned char)*pString);
				else
					return false;
			}
			if (iChar < 0x0080)
			{
				oTempBuffer += (char)iChar;
			}
			else if (iChar >= 0x80 && iChar < 0x800)
			{
				oTempBuffer += (char)(0xC0 | (iChar >> 6));
				oTempBuffer += (char)(0x80 | (iChar & 0x3F));
			}
			else if (iChar >= 0x800 && iChar < 0x7FFF)
			{
				oTempBuffer += (char)(0xE0 | (iChar >> 12));
				oTempBuffer += (char)(0x80 | ((iChar >> 6) & 0x3F));
				oTempBuffer += (char)(0x80 | (iChar & 0x3F));
			}
			else if (iChar >= 0x8000 && iChar < 0x7FFFF)
			{
				oTempBuffer += (char)(0xF0 | (iChar >> 18));
				oTempBuffer += (char)(0xE0 | ((iChar >> 12) & 0x3F));
				oTempBuffer += (char)(0x80 | ((iChar >> 6) & 0x3F));
				oTempBuffer += (char)(0x80 | (iChar & 0x3F));
			}
			else
			{
				return false;
			}
			return true;
		}
		else
			return false;

		return true;
	}

	bool JsonValue::ReadStringValue(const char*& pString, CharBuffer& oTempBuffer)
	{
		oTempBuffer.Clear();
		while (*pString != 0)
		{
			if (*pString == '\\')
			{
				++pString;
				if (!ReadSpecialChar(pString, oTempBuffer))
					return false;
				++pString;
			}
			else if (*pString == '"')
			{
				oTempBuffer += '\0';
				++pString;
				return true;
			}
			else if ((unsigned int)*pString < ' ' || *pString == '\x7F')
			{
				return false;
			}
			else
			{
				oTempBuffer += *pString;
				++pString;
			}
		}
		return false;
	}

	bool JsonValue::ReadStringValue(const char*& pString, JsonValue& oValue, CharBuffer& oTempBuffer)
	{
		if (ReadStringValue(pString, oTempBuffer))
		{
			oValue.InitType(E_TYPE_STRING);
			oValue.m_pString = oTempBuffer.Take();
			//oValue = oTempBuffer.Data();//Use Take();
			return true;
		}
		return false;
	}

	bool JsonValue::ReadNumericValue(const char*& pString, JsonValue& oValue)
	{
	#ifdef STTHM_USE_CUSTOM_NUMERIC_PARSER
		static double const c_pExpTable[] = {
			1e5, 1e4, 1e3, 1e2, 10, 1,
			0.1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6,
			1e-7, 1e-8, 1e-9, 1e-10, 1e-11, 1e-12,
			1e-13, 1e-14, 1e-15, 1e-16, 1e-17
		};
		static double const* c_pExpLookup = &c_pExpTable[5];

		bool bNeg = false;
		if (*pString == '-')
		{
			++pString;
			bNeg = true;
		}

		uint64_t lValue = 0;
		int iNegFract = 0;

		while (IsDigit(*pString))
			lValue = lValue * 10 + (*pString++ & 0xF);

		if (*pString == '.')
		{
			const char* pStart = ++pString;

			while (IsDigit(*pString))
				lValue = lValue * 10 + (*pString++ & 0xF);

			iNegFract = (int)(pString - pStart);

			if (*pString == 'e' || *pString == 'E')
			{
				++pString;

				bool bNegExp = false;
				if (*pString == '+')
				{
					++pString;
				}
				else if (*pString == '-')
				{
					++pString;
					bNegExp = true;
				}

				uint64_t iExpValue = 0;
				while (IsDigit(*pString))
					lValue = lValue * 10 + (*pString++ & 0xF);

				iNegFract += bNegExp ? (int)iExpValue : -(int)iExpValue;
			}
			oValue = (bNeg ? -1.0 : 1.0) * lValue * c_pExpLookup[iNegFract];
		}
		else
		{
			//TODO manage E/e for long?
			oValue = (long)(bNeg ? -lValue : lValue);
		}
		return true;
	#else //STTHM_USE_CUSTOM_NUMERIC_PARSER
		char* pEndDouble;
		char* pEndLong;
		double fValue = strtod( pString, &pEndDouble );
		long iValue = strtol( pString, &pEndLong, 10 );
		if( pEndDouble > pEndLong )
		{
			pString = pEndDouble;
			oValue = fValue;
		}
		else
		{
			pString = pEndLong;
			oValue = iValue;
		}
		
		return true;
	#endif // !STTHM_USE_CUSTOM_NUMERIC_PARSER
	}

	bool JsonValue::ReadObjectValue(const char*& pString, JsonValue& oValue, CharBuffer& oTempBuffer)
	{
		oValue.InitType(JsonValue::E_TYPE_OBJECT);

		SkipSpaces( pString );

		if( *pString == '}' )
		{
			++pString;
			return true;
		}

		while (*pString != 0)
		{
			SkipSpaces(pString);

			// Read member name
			if (*pString != '"' || !ReadStringValue(++pString, oTempBuffer))
				return false;

			JsonValue* pNewMember = new JsonValue();
			pNewMember->m_pName = oTempBuffer.Take();

			SkipSpaces(pString);

			if (*pString != ':')
				return false;
			
			++pString;

			SkipSpaces(pString);

			if (!pNewMember->Parse(pString, oTempBuffer))
			{
				delete pNewMember;
				return false;
			}

			if (oValue.m_oChilds.m_pFirst == NULL)
			{
				oValue.m_oChilds.m_pFirst = pNewMember;
			}
			else
			{
				oValue.m_oChilds.m_pLast->m_pNext = pNewMember;
			}
			oValue.m_oChilds.m_pLast = pNewMember;

			SkipSpaces(pString);

			if (*pString == '}')
			{
				++pString;
				return true;
			}
			else if (*pString != ',')
			{
				return false;
			}
			++pString;
		}
		return false;
	}

	bool JsonValue::ReadArrayValue(const char*& pString, JsonValue& oValue, CharBuffer& oTempBuffer)
	{
		oValue.InitType(JsonValue::E_TYPE_ARRAY);

		SkipSpaces( pString );
		if( *pString == ']' )
		{
			++pString;
			return true;
		}

		while (*pString != 0)
		{
			SkipSpaces(pString);

			JsonValue* pNewValue = new JsonValue();
			
			if (!pNewValue->Parse(pString, oTempBuffer))
			{
				delete pNewValue;
				return false;
			}

			if (oValue.m_oChilds.m_pFirst == NULL)
			{
				oValue.m_oChilds.m_pFirst = pNewValue;
			}
			else
			{
				oValue.m_oChilds.m_pLast->m_pNext = pNewValue;
			}
			oValue.m_oChilds.m_pLast = pNewValue;

			SkipSpaces(pString);

			if (*pString == ']')
			{
				++pString;
				return true;
			}
			else if (*pString != ',')
			{
				return false;
			}
			++pString;
		}
		return false;
	}

	int JsonValue::ReadString(const char* pJson)
	{
		if (pJson != NULL)
		{
			CharBuffer oTempBuffer;
			Reset();
			const char* pEnd = pJson;
			if (!Parse(pEnd, oTempBuffer))
			{
				int iLine = 1;
				int iReturn = 1;
				while (pJson != pEnd)
				{
					if (*pJson == '\n')
						++iLine;
					else if (*pJson == '\r')
						++iReturn;
					++pJson;
				}
				if (iReturn > iLine)
					iLine = iReturn;
				return iLine;
			}
			return 0;
		}
		return -1;
	}

	const bool JsonValue::Parse(const char*& pString, CharBuffer& oTempBuffer)
	{
		bool bOk = pString != NULL && *pString != 0;
		while (*pString != 0 && bOk)
		{
			while (IsSpace(*pString)) ++pString;
			if (*pString == '"')
			{
				++pString;
				if (!ReadStringValue(pString, *this, oTempBuffer))
					bOk = false;
				break;
			}
			else if (IsDigit(*pString) || *pString == '-')
			{
				if (!ReadNumericValue(pString, *this))
					bOk = false;
				break;
			}
			else if (pString[0] == 't' && pString[1] == 'r' && pString[2] == 'u' && pString[3] == 'e')
			{
				pString += 4;
				*this = true;
				break;
			}
			else if (pString[0] == 'f' && pString[1] == 'a' && pString[2] == 'l' && pString[3] == 's' && pString[4] == 'e')
			{
				pString += 5;
				*this = false;
				break;
			}
			else if (pString[0] == 'n' && pString[1] == 'u' && pString[2] == 'l' && pString[3] == 'l')
			{
				pString += 4;
				InitType(E_TYPE_INVALID);
				break;
			}
			else if (*pString == '{')
			{
				++pString;
				if (!ReadObjectValue(pString, *this, oTempBuffer))
				{
					bOk = false;
				}
				break;
			}
			else if (*pString == '[')
			{
				++pString;
				if (!ReadArrayValue(pString, *this, oTempBuffer))
					bOk = false;
				break;
			}
			else
			{
				//Error
				bOk = false;
				break;
			}
		}
		return bOk;
	}

	int JsonValue::ReadFile(const char* pFilename)
	{
		FILE* pFile = fopen(pFilename, "r");
		if (NULL != pFile)
		{
			Reset();

			fseek(pFile, 0, SEEK_END);
			long iSize = ftell(pFile);
			fseek(pFile, 0, SEEK_SET);

			char* pString = new char[iSize / sizeof(char)];
			fread(pString, 1, iSize, pFile);
			fclose(pFile);

			int iLine = ReadString(pString);

			delete[] pString;
			return iLine;
		}
		return -1;
	}
}