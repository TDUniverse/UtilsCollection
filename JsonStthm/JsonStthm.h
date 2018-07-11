
#ifndef __JSON_STTHM_H__
#define __JSON_STTHM_H__

#include "JsonStthmConfig.h"
#include <string.h>

namespace JsonStthm
{
	typedef std::string String;

	class STTHM_API JsonValue
	{
	protected:

		template <typename T, int HeapSize = 1024>
		struct Buffer
		{
		public:
			Buffer(int iReserve = 8)
			{
				m_pData = m_pHeapData;
				m_iSize = m_iCapacity = 0;
				m_bUseHeap = true;
				Reserve(iReserve);
			}

			~Buffer()
			{
				if (!m_bUseHeap)
					StthmFree(m_pData);
			}

			Buffer<T>& operator +=(const T& oValue)
			{
				Push(oValue);
				return *this;
			}

			void Push(const T& oValue)
			{
				Resize(m_iSize + 1);
				if (m_bUseHeap)
					m_pHeapData[m_iSize - 1] = oValue; 
				else
					m_pData[m_iSize - 1] = oValue;
			}

			int Size() const
			{
				return m_iSize;
			}

			void Reserve(int iCapacity, bool bForceAlloc = false)
			{
				if (iCapacity != m_iCapacity)
				{
					if (!m_bUseHeap || iCapacity >= HeapSize || bForceAlloc)
					{
						T* pTemp = (T*)StthmMalloc(iCapacity * sizeof(T));
						if (NULL != m_pData)
						{
							memcpy(pTemp, m_pData, (m_iCapacity > iCapacity ? m_iCapacity : iCapacity) * sizeof(T));
							if (!m_bUseHeap)
								StthmFree(m_pData);
						}
						m_pData = pTemp;
						m_bUseHeap = false;
					}
					
					m_iCapacity = iCapacity;
				}
			}

			void Resize(int iSize)
			{
				int iNewCapacity = m_iCapacity > 0 ? m_iCapacity : 8;
				while (iSize > iNewCapacity) 
					iNewCapacity *= 2;
				if (iNewCapacity != m_iCapacity)
					Reserve(iNewCapacity);
				m_iSize = iSize;
			}

			const T* Data() const { return m_bUseHeap ? m_pHeapData : m_pData; }

			T* Take()
			{
				char* pTemp;
				if (m_bUseHeap)
				{
					pTemp = (T*)StthmMalloc(m_iSize * sizeof(T));
					memcpy(pTemp, m_pHeapData, m_iSize * sizeof(T));
				}
				else
				{
					pTemp = m_pData;
					m_pData = NULL;
				}
				m_iCapacity = 0;
				m_iSize = 0;
				m_bUseHeap = true;
				return pTemp;
			}

			void Clear()
			{
				m_iSize = 0;
			}
		protected:
			T		m_pHeapData[HeapSize];
			T*		m_pData;
			int		m_iSize;
			int		m_iCapacity;
			bool	m_bUseHeap;
		};

		typedef Buffer<char> CharBuffer;

		struct ParseInfos;
	public:
		enum EType
		{
			E_TYPE_INVALID = 0,	//null
			E_TYPE_OBJECT,		//JsonMembers
			E_TYPE_ARRAY,		//JsonArray
			E_TYPE_STRING,		//String
			E_TYPE_BOOLEAN,		//bool
			E_TYPE_INTEGER,		//long
			E_TYPE_FLOAT		//double
		};

		class Iterator
		{
		public:
			Iterator(const JsonValue* pJson);
			Iterator(const Iterator& oIt);

			bool IsValid() const;
			bool operator!=(const Iterator& oIte) const;
			void operator++();
			JsonValue* operator*() const;
			JsonValue* operator->() const;
		protected:
			JsonValue* m_pChild;
		};

		static JsonValue	INVALID;
	public:
							JsonValue();
							JsonValue(const JsonValue& oSource);
							JsonValue(bool bValue);
							JsonValue(const String& sValue);
							JsonValue(const char* pValue);
							JsonValue(long iValue);
							JsonValue(double fValue);
							~JsonValue();

		void				InitType(EType eType);
		EType				GetType() const;

		int					ReadString(const char* pJson);
		int					ReadFile(const char* pFilename);

		void				WriteString(String& sOutJson, bool bCompact = false);
		bool				WriteFile(const char* pFilename, bool bCompact = false);

		bool				IsNull() const { return m_eType == E_TYPE_INVALID; }
		bool				IsObject() const { return m_eType == E_TYPE_OBJECT; }
		bool				IsArray() const { return m_eType == E_TYPE_ARRAY; }
		bool				IsBoolean() const { return m_eType == E_TYPE_BOOLEAN; }
		bool				IsString() const { return m_eType == E_TYPE_STRING; }
		bool				IsInteger() const { return m_eType == E_TYPE_INTEGER; }
		bool				IsFloat() const { return m_eType == E_TYPE_FLOAT; }

		bool				IsNumeric() const { return m_eType == E_TYPE_INTEGER || m_eType == E_TYPE_FLOAT; }
		bool				IsContainer() const { return m_eType == E_TYPE_ARRAY || m_eType == E_TYPE_OBJECT; }

		int					GetMemberCount() const;

		const char*			GetName() const { return m_pName; }

		const char*			ToString() const;
		bool				ToBoolean() const;
		long				ToInteger() const;
		double				ToFloat() const;

		const JsonValue&	operator [](const char* pName) const;
		JsonValue&			operator [](const char* pName);

		const JsonValue&	operator [](int iIndex) const;
		JsonValue&			operator [](int iIndex);
		
		JsonValue&			operator =(const JsonValue& oValue);
		JsonValue&			operator =(const String& sValue);
		JsonValue&			operator =(const char* pValue);
		JsonValue&			operator =(bool bValue);
		JsonValue&			operator =(long iValue);
		JsonValue&			operator =(double fValue);

		JsonValue&			operator +=(const JsonValue& oValue);

							operator const char*() const;
							operator bool() const;
							operator long() const;
							operator double() const;
	protected:
		static JsonValue	CreateConst();
		void				Reset();
		void				SetString(const char* pString);
		
		void				Write(String& sOutJson, int iIndent, bool bCompact);
		static void			WriteStringEscaped(String& sOutJson, const String& sInput);

		bool				m_bConst;
		EType				m_eType;
		char*				m_pName;
		JsonValue*			m_pNext;

		struct JsonChilds
		{
			JsonValue*		m_pFirst;
			JsonValue*		m_pLast;
		};
		union
		{
			JsonChilds		m_oChilds;
			char*			m_pString;
			bool			m_bBoolean;
			long			m_iInteger;
			double			m_fFloat;
		};

		const bool Parse(const char*& pString, CharBuffer& oTempBuffer);

		static inline bool	IsSpace(char cChar);
		static inline bool	IsDigit(char cChar);
		static inline bool	IsXDigit(char cChar);
		static inline int	CharToInt(char cChar);
		static inline void	SkipSpaces(const char*& pString);
		static inline bool	ReadSpecialChar(const char*& pString, CharBuffer& oTempBuffer);
		static inline bool	ReadStringValue(const char*& pString, CharBuffer& oTempBuffer);
		static inline bool	ReadStringValue(const char*& pString, JsonValue& oValue, CharBuffer& oTempBuffer);
		static inline bool	ReadNumericValue(const char*& pString, JsonValue& oValue);
		static inline bool	ReadObjectValue(const char*& pString, JsonValue& oValue, CharBuffer& oTempBuffer);
		static inline bool	ReadArrayValue(const char*& pString, JsonValue& oValue, CharBuffer& oTempBuffer);
	};
}

#endif // __JSON_STTHM_H__
