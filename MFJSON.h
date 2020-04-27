#pragma once
#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <assert.h>
#include <string.h>
#include <math.h>
//#include <windows.h>

namespace MFJSON
{


	typedef long long IFI64;
	typedef int IFI32;

#pragma pack(push)
#pragma pack(4)

	struct String
	{

		int nBuf;
		int nHash;
		bool isEmpty() const
		{
			return nBuf == -1;
		}
		static const String& EMPTY()
		{
			static String empty = { -1,0 };
			return empty;
		}
	};

	struct Array
	{
		Array()
		{

		}
		Array(int sidx, int bidx, int sbidx)
		{
			assert(sidx < 256 && sidx>=0 && bidx < 256 && bidx>=0 && sbidx < 65536 && sbidx>=0);
			set(sidx, bidx, sbidx);
		}
		int nBuf;
		int nLen;

		void set(int sidx, int bidx, int sbidx)
		{
			nBuf = sidx << 24 | bidx << 16 | sbidx;
		}
	};

	template<typename NODE>
	struct KeyValueT
	{
		int skey;
		NODE value;
	};
#pragma pack(pop)


	
	//class Buffer
	//{
	//public:
	//	Buffer(int nSize)
	//		:m_pBuf(nullptr),
	//		m_nSize(0)
	//	{
	//		reserve(nSize);
	//	}
	//	~Buffer()
	//	{
	//		if (m_pBuf)
	//			delete[] m_pBuf;
	//	}


	//	void reserve(int nSize)
	//	{
	//		if (m_nSize >= nSize)
	//			return;
	//		char* pNewBuf = new char[nSize];
	//		if (m_nSize)
	//		{
	//			memcpy(pNewBuf, m_pBuf, m_nSize );
	//			delete[] m_pBuf;
	//		}
	//		m_pBuf = pNewBuf;
	//	}
	//	inline operator char*()
	//	{
	//		return m_pBuf;
	//	}
	//	inline int size()
	//	{
	//		return m_nSize;
	//	}
	//private:
	//	char* m_pBuf;
	//	int m_nSize;


	//};

	class StringStream
	{

	public:
		inline void seek_to_next_token()
		{
			while (*m_pPointer == ' ' ||
				*m_pPointer == '\t' ||
				*m_pPointer == '\r' ||
				*m_pPointer == '\n')
			{
				++m_pPointer;
			}
		}
		inline void seek(int nSize)
		{
			m_pPointer += nSize;
		}

		inline char token()
		{
			return *m_pPointer;
		}
		inline bool next_is(const char* s, int nLen)
		{
			auto s_end = s + nLen;
			while (s < s_end&&m_pPointer < m_pBuffEnd && (*s) == *m_pPointer)
			{

				s++;
				m_pPointer++;
			}
			return s == s_end;
		}
		inline bool seek_next_is(const char* s, int nLen)
		{
			auto s_end = s + nLen;
			auto pPointer = m_pPointer;
			while (s < s_end&&pPointer < m_pBuffEnd && (*s) == *pPointer)
			{

				s++;
				pPointer++;
			}
			return s == s_end;
		}

		char* m_pBuf;
		char* m_pBuffEnd;
		char* m_pPointer;
	};



	class Doc;

	enum NodeType : char
	{
		NT_NULL,
		NT_BOOL,
		NT_STRING,
		NT_NUMBER,
		NT_MAP,
		NT_ARRAY,

	};

	enum NodeFlag:char
	{
		NF_NONE,
		NF_DOUBLE,
		NF_FLOAT,
		NF_INT32,
		NF_INT64,
		NF_SHORT_STRING,
		NF_LONG_STRING,
		NF_L8_STRING,
	};

	struct Flag
	{
		char placeholder[3];
		char flag;
	};
	enum
	{
		SHORT_STRING_LEN = 128
	};
	
	class Doc;
	template<typename T>
	const char* parse_generic(Doc& doc, const char*& sUTF8, T& o);


#pragma pack(push)
#pragma pack(4)
	struct LongString
	{
		int nidx;
		int nlen;
	};
	class Node
	{
	private:
		Node()
			:m_TF(0), m_int64(0)
		{

		}

		const char* parse(Doc& doc, const char*& sUTF8);

		const char* parseNumber(Doc& doc, const char*& sUTF8);
		const char* parseString(Doc& doc, const char*& sUTF8);
		const char* parseArray(Doc& doc, const char*& sUTF8);
		const char* parseMap(Doc& doc, const char*& sUTF8);



	public:



		void setBool(bool b)
		{
			
		}
		void setNull()
		{
			if (m_Type != NT_NULL)
			{
				m_Type = NT_NULL;
			}
		}
		union
		{
			int m_TF;
			union
			{
				NodeType m_Type;
				Flag m_Flag;
			};
		};
	
		union 
		{
			bool m_b;
			double m_df;
			float m_f;
			IFI64 m_int64;
			int m_int32;
			String m_ss;
			LongString m_ls;
			char m_l8s[8];
			Array m_array;
			//std::vector<int,Node>
		};

		friend class Accessor;
		friend class Doc;
		template<typename T>
		friend const char* parse_generic(Doc& doc, const char*& sUTF8, T& o);

	};
	typedef KeyValueT<Node> KeyValue;

#pragma pack(pop)
	class StringHash
	{
	public:
		size_t operator ()(const String& s) const
		{
			return s.nHash;
		}
	};

	class ShortStringPool;

	class StringEqual
	{
	public:
		StringEqual(const StringEqual& o)
			:pool(o.pool), lasthash(0)
		{

		}
		StringEqual(ShortStringPool& d) :pool(d), lasthash(0)
		{

		}

		bool operator ()(const String& a, const String& b) const;
		//static int m_scounternum;

	private:
		ShortStringPool& pool;
		mutable int lasthash;
		mutable bool lastresult;
	};

	
	class StringBlock
	{
	public:
		enum { BLOCK_SIZE = 1024 * 32 };

		StringBlock()
			:m_alloc_ptr(0)
		{

		}
		int alloc(int nsize)
		{
			if (m_alloc_ptr + nsize <= BLOCK_SIZE)
			{
				int nowptr = m_alloc_ptr;
				m_alloc_ptr += nsize;
				return nowptr;

			}
			else
				return -1;
		}

		inline char* get(int nOffset)
		{
			return m_buf + nOffset;
		}
		inline int cur_alloc_ptr()
		{
			return m_alloc_ptr;
		}

	private:
		char m_buf[BLOCK_SIZE];
		int m_alloc_ptr;
	};
	
	inline size_t hash_value(const MFJSON::String& h)
	{
		return h.nHash;
	}

	inline int rs_hash(const char* pSrc, int nLen)
	{
		int a = 63689;
		int b = 378551;
		int hash = 0;
		for (int i = 0; i < nLen; i++)
		{
			hash = hash * a + pSrc[i];
			a *= b;
		}
		return hash;
	}

	inline size_t copy_and_hash(char* pDst, const char* pSrc, int nLen)
	{
		size_t a = 63689;
		size_t b = 378551;
		size_t hash = 0;
		for (int i = 0; i < nLen; i++)
		{
			hash = hash * a + pSrc[i];
			a *= b;
			pDst[i] = pSrc[i];
		}
		return hash;
	}

	class StringBuilder
	{
	public:
		StringBuilder()
			:m_write_pos(StringBlock::BLOCK_SIZE)
			, m_size(0)
		{
		}
		~StringBuilder()
		{
			clear();
		}
		void clear()
		{
			for (auto& p:m_buff)
			{
				delete p;
			}
			m_buff.clear();
			m_size = 0;
			m_write_pos = StringBlock::BLOCK_SIZE;
		}



		inline void append(const char* s, int len)
		{
			m_size += len;
			char* lastblock;
			do 
			{
			
				int nleftsize = StringBlock::BLOCK_SIZE - m_write_pos;
				if (nleftsize == 0)
				{
					m_buff.push_back(new char[StringBlock::BLOCK_SIZE]);
					m_write_pos = 0;
					nleftsize = StringBlock::BLOCK_SIZE;
					lastblock = m_buff.back();
				}
				else
				{
					lastblock = m_buff.back();

				}
				int nwritesize = len < nleftsize ? len : nleftsize;
				
				memcpy(lastblock + m_write_pos, s, nwritesize);
				m_write_pos += nwritesize;
				len -= nwritesize;
				s += len;

			} while (len);
			//return;
			//do
			////{
			//	int nSubWritePos = m_write_pos%StringBlock::BLOCK_SIZE;
			//	int nWriteIdx = m_write_pos / StringBlock::BLOCK_SIZE;
			//	int nCanWriteSize;
			//	if (nWriteIdx>=m_buff.size())
			//	{
			//		m_buff.push_back(new StringBlock());
			//		nCanWriteSize = StringBlock::BLOCK_SIZE;
			//	}
			//	else
			//	{
			//		nCanWriteSize = StringBlock::BLOCK_SIZE - nSubWritePos;
			//	}
			//	int nWriteSize = len < nCanWriteSize ? len : nCanWriteSize;
			//	char* pdst = m_buff[nWriteIdx]->get(nSubWritePos);
			//	for (int i = 0; i < nWriteSize; i ++ )
			//	{
			//		pdst[i] = s[i];
			//	}
			//	//memcpy(m_buff[nWriteIdx]->get(nSubWritePos), s, nWriteSize);
			//	s += nWriteSize;
			//	len -= nWriteSize;
			//	m_write_pos += nWriteSize;
			////} while (len);

		}

		std::string toString()
		{
			std::string s(m_size, 0);
			int nSubWritePos = m_size%StringBlock::BLOCK_SIZE;
			int nWriteIdx = m_size / StringBlock::BLOCK_SIZE;
			for (int i = 0; i < nWriteIdx; i ++)
			{
				memcpy(&s[i*StringBlock::BLOCK_SIZE], m_buff[i], StringBlock::BLOCK_SIZE);
			}
			memcpy(&s[nWriteIdx*StringBlock::BLOCK_SIZE], m_buff[nWriteIdx], nSubWritePos);
			return s;
		}
	protected:
		std::vector<char*> m_buff;
		int m_write_pos;
		int m_size;
	};

	class Accessor;
	template<class T>
	T array_get(Accessor& acc, int n);

	template<class T>
	void accessor_generic_setter(Accessor& acc, const T& val);

	template<class T>
	void accessor_generic_getter(Accessor& acc, T& val);


	class Accessor
	{
	public:

		//class ArrayAccessor;

		template<typename T>
		class iteratorT
		{

		public:
			iteratorT(Accessor* acc, int idx)
				:m_accessor(acc),m_idx(idx)
			{

			}
			iteratorT& operator ++()
			{
				m_idx++;
				return *this;
			}

			bool operator == (const iteratorT& o)
			{
				return o.m_idx == m_idx;
			}
			bool operator != (const iteratorT& o)
			{
				return o.m_idx != m_idx;
			}

			T operator *();


			int m_idx;

			Accessor* m_accessor;
			friend class Accessor;
		};



		template<typename T>
		class ArrayAccessor
		{
			ArrayAccessor(Accessor& acc)
				:m_accessor(acc)
			{

			}
		public:
			typedef iteratorT<T> iterator;
			iterator begin()
			{
				return iterator(&m_accessor, 0);

			}
			iterator end()
			{
				return iterator(&m_accessor, m_accessor.m_node->m_array.nLen);
			}
		private:
			Accessor& m_accessor;
			friend class Accessor;
		};

	public:
		Accessor(Doc& doc, Node* nd);

		inline bool isNull() const
		{
			return m_node || m_node->m_Type == NT_NULL;
		}

		inline bool isInt() const
		{
			return m_node && m_node->m_Type == NT_NUMBER && (m_node->m_Flag.flag == NF_INT32 || m_node->m_Flag.flag == NF_INT64);
		}
		inline bool isInt32() const
		{
			return m_node && m_node->m_Type == NT_NUMBER && (m_node->m_Flag.flag == NF_INT32);
		}
		inline bool isInt64() const
		{
			return m_node && m_node->m_Type == NT_NUMBER && (m_node->m_Flag.flag == NF_INT64);
		}
		inline bool isDouble() const
		{
			return m_node && m_node->m_Type == NT_NUMBER && m_node->m_Flag.flag == NF_DOUBLE;
		}
		inline bool isFloat() const
		{
			return m_node && m_node->m_Type == NT_NUMBER && m_node->m_Flag.flag == NF_FLOAT;
		}
		inline bool isBool() const
		{
			return m_node && m_node->m_Type == NT_BOOL;
		}
		inline bool isString() const
		{
			return m_node && m_node->m_Type == NT_STRING;
		}
		inline bool isArray() const
		{
			return m_node && m_node->m_Type == NT_ARRAY;
		}
		inline bool isMap() const
		{
			return m_node && m_node->m_Type == NT_MAP;
		}

		inline double getDouble() const
		{
			if (m_node&&m_node->m_Type == NT_NUMBER)
			{

				if (m_node->m_Flag.flag == NF_DOUBLE)
				{
					return m_node->m_df;
				}
				else if (m_node->m_Flag.flag == NF_FLOAT)
				{
					return m_node->m_f;
				}
				else if (m_node->m_Flag.flag == NF_INT32)
				{
					return m_node->m_int32;
				}
				else
				{
					return (double)m_node->m_int64;
				}

			}
			return 0.0;
		}
		inline  float getFloat() const
		{
			if (m_node&&m_node->m_Type == NT_NUMBER)
			{

				if (m_node->m_Flag.flag == NF_DOUBLE)
				{
					return (float)m_node->m_df;
				}
				else if (m_node->m_Flag.flag == NF_FLOAT)
				{
					return m_node->m_f;
				}
				else if (m_node->m_Flag.flag == NF_INT32)
				{
					return (float)m_node->m_int32;
				}
				else
				{
					return (float)m_node->m_int64;
				}

			}
			return 0.0f;
		}

		inline int getInt() const
		{
			if (m_node&&m_node->m_Type == NT_NUMBER)
			{

				if (m_node->m_Flag.flag == NF_INT32)
					return m_node->m_int32;
				else if (m_node->m_Flag.flag == NF_INT64)
					return (int)m_node->m_int64;
				else if (m_node->m_Flag.flag == NF_FLOAT)
					return (int)m_node->m_f;
				else
					return (int)m_node->m_df;
			}
			return 0;
		}

		inline IFI64 getInt64() const
		{
			if (m_node&&m_node->m_Type == NT_NUMBER)
			{

				if (m_node->m_Flag.flag == NF_INT64)
					return m_node->m_int64;
				else if (m_node->m_Flag.flag == NF_INT32)
					return m_node->m_int32;
				else if (m_node->m_Flag.flag == NF_FLOAT)
					return (IFI64)m_node->m_f;
				else
					return (IFI64)m_node->m_df;
			}
			return 0;
		}
		inline const char* getString() const;
		int getMapLength() const
		{
			if (m_node && m_node->m_Type == NT_MAP)
				return m_node->m_array.nLen;
			else
				return 0;
		}
		int getArrayLength() const
		{
			if (m_node && m_node->m_Type == NT_ARRAY)
				return m_node->m_array.nLen;
			else
				return 0;
		}
		Accessor getMapValue(const char* key) const;
		std::pair<const char*, Accessor> getMapKeyValue(int i) const;

		Accessor getArrayValue(int i) const;


		Accessor operator[](const char* s)
		{
			return getMapValue(s);
		}
		Accessor operator[](int idx)
		{
			return getArrayValue(idx);
		}

		ArrayAccessor<Accessor> array()
		{
			return ArrayAccessor<Accessor>(*this);
		}
		ArrayAccessor<std::pair<const char*, Accessor>> map()
		{
			return ArrayAccessor<std::pair<const char*, Accessor>>(*this);
		}

		template<typename T>
		void get(T& o)
		{
			accessor_generic_getter(*this, o);
		}


		std::string dump(bool bVisibleFormat = true);


		void setNull()
		{
			m_node->m_Type = NT_NULL;
		}

		void set(double df)
		{
			m_node->m_Type = NT_NUMBER;
			m_node->m_Flag.flag = NF_DOUBLE;
			m_node->m_df = df;
		}
		void set(float f)
		{
			m_node->m_Type = NT_NUMBER;
			m_node->m_Flag.flag = NF_FLOAT;
			m_node->m_f = f;
		}
		void set(int i)
		{
			m_node->m_Type = NT_NUMBER;
			m_node->m_Flag.flag = NF_INT32;
			m_node->m_int32 = i;
		}
		void set(IFI64 i)
		{
			m_node->m_Type = NT_NUMBER;
			m_node->m_Flag.flag = NF_INT64;
			m_node->m_int64 = i;
		}

		void set(bool b)
		{
			m_node->m_Type = NT_BOOL;
			m_node->m_b = b;
		}

		void set(const char* s, int len=-1);

		template<typename T>
		void set(const T& t)
		{
			accessor_generic_setter(*this, t);
		}




		void arrayBegin();
		Accessor arrayPush();
		void arrayEnd();
	
		void mapBegin();
		Accessor mapPush(const char* key);
		Accessor mapPush(const String& key);

		void mapEnd();

private:
		void dumpInternal(bool bVisibleFormat, std::string& tb);
		Doc& m_doc;
		Node* m_node;

	};

	template<>
	inline Accessor array_get(Accessor& acc, int n)
	{
		return acc.getArrayValue(n);
	}
	template<>
	inline std::pair<const char*, Accessor> array_get(Accessor& acc, int n)
	{
		return acc.getMapKeyValue(n);
	}

	template<typename T>
	const char* object_value_setter(Doc& doc, T& o, const char*& sUTF8, const String& sKey);

	typedef std::vector<StringBlock*> StringBlockList;

	class ShortStringPool
	{
	public:
		ShortStringPool() :m_StringEqual(*this),
			m_ShortStringLookupMap(128, m_StringHash, m_StringEqual)
		{
			m_string_blocks.push_back(new StringBlock());

		}

		~ShortStringPool()
		{
			for (auto& b : m_string_blocks)
			{
				delete b;
			}
		}
	public:

		//static ShortStringPool& InstanceForObjRead()
		//{
		//	static ShortStringPool instance;
		//	return instance;
		//}
		//
		inline const String& findShortString(const char* s, int nlen)
		{
			m_temp_short_string = s;
			m_temp_short_string_len = nlen;
			String stemp;
			stemp.nBuf = -1;
			stemp.nHash = rs_hash(s, nlen);
			auto it = m_ShortStringLookupMap.find(stemp);
			if (it != m_ShortStringLookupMap.end())
			{
				m_temp_short_string = NULL;
				return *it;
			}
			else
				return String::EMPTY();
		}

		inline const String& addNewShortString(const char* s, int nlen)
		{
			int sb = m_string_blocks.back()->alloc(nlen + 2);
			if (sb == -1)
			{
				m_string_blocks.push_back(new StringBlock());
				sb = m_string_blocks.back()->alloc(nlen + 2);
			}
			auto pdst = m_string_blocks.back()->get(sb);
			//stemp.nHash = copy_and_hash(pdst, s, nlen);
			memcpy(pdst+1, s, nlen);
			pdst[nlen+1] = 0;
			pdst[0] = nlen;
			String stemp;
			stemp.nHash = rs_hash(s, nlen);;
			stemp.nBuf = ((int)m_string_blocks.size() - 1) << 16 | sb;
			auto ir = m_ShortStringLookupMap.insert(stemp);

			return  (*ir.first);
		}

		inline const String& addShortString(const char* s, int nlen)
		{

			auto& stemp = findShortString(s, nlen);
			if(!stemp.isEmpty())
				return stemp;
			else
				return addNewShortString(s,nlen);

			
		}
		const char* getString(int nBuf)
		{
			int nb = nBuf >> 16;
			int sb = nBuf & 0xffff;
			return m_string_blocks[nb]->get(sb)+1;
		}
		const char* getString(int nBuf,int& len)
		{
			int nb = nBuf >> 16;
			int sb = nBuf & 0xffff;
			auto p = m_string_blocks[nb]->get(sb);
			len = p[0];
			return p + 1;
		}

	private:
		StringEqual m_StringEqual;

		typedef std::unordered_set<String, StringHash, StringEqual> ShortStringListLookList;
		ShortStringListLookList m_ShortStringLookupMap;

		//char m_TempShortString[SHORT_STRING_LEN];
		const char* m_temp_short_string;
		int m_temp_short_string_len;
		StringBlockList m_string_blocks;
		StringHash m_StringHash;
		friend class StringEqual;

	};


	class Doc
	{

	public:
		Doc()
			:			
			m_array_block_stack_idx(-1), m_map_block_stack_idx(-1), m_root_accessor(*this,&m_Root), m_pErrorPtr(NULL)
			//short_string_pool_intance(ShortStringPool::Instance())
			
		{
			addLongString(new char[4]);
		}
		~Doc()
		{
			//for (auto& v : m_ShortStringLookupMap)
			//{
			//	printf("%s ", getString(v.nBuf));
			//}


			for (auto& bs:m_array_block_stack)
			{
				for (auto& b:*bs)
				{
					delete b;
				}
				delete bs;
			}
			for (auto& bs : m_map_block_stack)
			{
				for (auto& b : *bs)
				{
					delete b;
				}
				delete bs;
			}
			for (auto& s : m_long_string_list)
			{
				delete[] s;
			}
		}

		bool load(const char* sUTF8)
		{
			if (m_pErrorPtr = m_Root.parse(*this, sUTF8))
			{
				return false;
			}
			return true;
		}
		std::string errorDesc(const char* sUTF8)
		{
			if (m_pErrorPtr)
			{
				auto p = sUTF8;
				int nLine = 1;
				int nCol = 1;
				while (*p && p != m_pErrorPtr)
				{
					if (*p == '\r')
					{
						if (*(p + 1) == '\n')
							p++;
						nLine++;
						nCol = 0;
					}
					else if (*p == '\n')
					{
						nLine++;
						nCol = 0;
					}
					nCol++;
					p++;

				}
				char buf[64];
				snprintf(buf, sizeof(buf), "Error@Line:%d Col:%d", nLine, nCol);
				return buf;
			}
			return "";
		}
		Accessor operator[](const char* s)
		{
			return m_root_accessor.getMapValue(s);
		}
		Accessor operator[](int idx)
		{
			return m_root_accessor.getArrayValue(idx);
		}

		std::string dump(bool bVisibleFormat = false)
		{
			return m_root_accessor.dump(bVisibleFormat);
		}


		Accessor& root()
		{
			return m_root_accessor;
		}

		template<typename T>
		bool load(const char* sUTF8, T& o)
		{			
			m_pErrorPtr =  parse_generic(*this, sUTF8, o);
			if (m_pErrorPtr)
				return false;
			else
				return true;
		}
		ShortStringPool& getStringPool()
		{
			return short_string_pool_intance;
		}

		Node& getTempNode()
		{
			return m_tempNode;
		}
	private:

		inline const String& addShortString(const char* s, int nlen)
		{
			return short_string_pool_intance.addShortString(s, nlen);
		}

		inline const char* getString(int nBuf)
		{
			return short_string_pool_intance.getString(nBuf);
		}
		

		int addLongString(char* s)
		{
			m_long_string_list.push_back(s);
			return (int)m_long_string_list.size() - 1;
		}



		Array array_begin()
		{
			m_array_block_stack_idx++;
			if (m_array_block_stack_idx >= m_array_block_stack.size())
			{
				m_array_block_stack.push_back(new StringBlockList());
				m_array_block_stack_cur_len.push_back(0);
			}
			m_cur_array_blocks = m_array_block_stack[m_array_block_stack_idx];
			m_array_block_stack_cur_len[m_array_block_stack_idx] = 0;
			if (m_cur_array_blocks->size() == 0)
			{
				m_cur_array_blocks->push_back(new StringBlock());
			}
			
			return Array(m_array_block_stack_idx,(int)m_cur_array_blocks->size()-1, m_cur_array_blocks->back()->cur_alloc_ptr() );
			
		}

		Node& array_push()
		{
			int sb = m_cur_array_blocks->back()->alloc(sizeof(Node));
			if (sb == -1)
			{
				m_cur_array_blocks->push_back(new StringBlock());
				sb = m_cur_array_blocks->back()->alloc(sizeof(Node));
			}
			m_array_block_stack_cur_len[m_array_block_stack_idx]++;
			return *(Node*)m_cur_array_blocks->back()->get(sb);
		}

		enum {
			ARRAY_ONE_BLOCK_COUNT = StringBlock::BLOCK_SIZE / sizeof(Node),
			MAP_ONE_BLOCK_COUNT = StringBlock::BLOCK_SIZE / sizeof(KeyValue),

		};
		
		
		Node& array_get(int nBuf, int i)
		{
			int sidx = nBuf >> 24;
			int bidx = (nBuf & 0xff0000) >> 16;
			int sbidx = (nBuf & 0xffff);

			int absibid = sbidx/ sizeof(Node) + i;
			sbidx = absibid%ARRAY_ONE_BLOCK_COUNT;
			bidx += absibid / ARRAY_ONE_BLOCK_COUNT;

			auto blocks = m_array_block_stack[sidx];
			auto block = (*blocks)[bidx];
			return *(Node*)block->get(sbidx*sizeof(Node));
		}

		int array_end()
		{
			int len = m_array_block_stack_cur_len[m_array_block_stack_idx];

			m_array_block_stack_idx--;
			if (m_array_block_stack_idx < 0)
				m_cur_array_blocks = NULL;
			else
				m_cur_array_blocks = m_array_block_stack[m_array_block_stack_idx];

			//if (m_array_block_stack_idx < 0)
			//{
			//	DebugBreak();
			//}
			//assert(m_array_block_stack_idx >= 0);
			return len;
		}
		Array map_begin()
		{
			m_map_block_stack_idx++;
			if (m_map_block_stack_idx >= m_map_block_stack.size())
			{
				m_map_block_stack.push_back(new StringBlockList());
				m_map_block_stack_cur_len.push_back(0);
			}
			m_cur_map_blocks = m_map_block_stack[m_map_block_stack_idx];
			m_map_block_stack_cur_len[m_map_block_stack_idx] = 0;
			if (m_cur_map_blocks->size() == 0)
			{
				m_cur_map_blocks->push_back(new StringBlock());
			}
			return Array(m_map_block_stack_idx,(int)m_cur_map_blocks->size()-1, m_cur_map_blocks->back()->cur_alloc_ptr());

		}

		KeyValue& map_push()
		{
			int sb = m_cur_map_blocks->back()->alloc(sizeof(KeyValue));
			if (sb == -1)
			{
				m_cur_map_blocks->push_back(new StringBlock());
				sb = m_cur_map_blocks->back()->alloc(sizeof(KeyValue));
			}
			m_map_block_stack_cur_len[m_map_block_stack_idx]++;
			return *(KeyValue*)m_cur_map_blocks->back()->get(sb);
		}
		KeyValue& map_get(int nBuf, int i)
		{
			int sidx = nBuf >> 24;
			int bidx = (nBuf & 0xff0000) >> 16;
			int sbidx = (nBuf & 0xffff);

			int absibid = sbidx / sizeof(KeyValue) + i;
			sbidx = absibid%MAP_ONE_BLOCK_COUNT;
			bidx += absibid / MAP_ONE_BLOCK_COUNT;

			auto blocks = m_map_block_stack[sidx];
			auto block = (*blocks)[bidx];
			return *(KeyValue*)block->get(sbidx* sizeof(KeyValue));
		}
		int map_end()
		{
			int len = m_map_block_stack_cur_len[m_map_block_stack_idx];

			m_map_block_stack_idx--;
			if (m_map_block_stack_idx < 0)
				m_cur_map_blocks = NULL;
			else
				m_cur_map_blocks = m_map_block_stack[m_map_block_stack_idx];

			return len;
		}

		
		//ShortStringPool m_short
		//std::vector<String> m_ShortStringList;

		ShortStringPool short_string_pool_intance;
		
		StringBlockList* m_cur_array_blocks;
		std::vector<StringBlockList*> m_array_block_stack;
		std::vector<int> m_array_block_stack_cur_len;

		int m_array_block_stack_idx;

		StringBlockList* m_cur_map_blocks;
		std::vector<StringBlockList*> m_map_block_stack;
		std::vector<int> m_map_block_stack_cur_len;

		int m_map_block_stack_idx;
		Accessor m_root_accessor;
		std::vector<char*> m_long_string_list;
		StringBuilder m_string_builder;

		const char* m_pErrorPtr;

		Node m_Root;
		Node m_tempNode;
		friend class Node;
		friend class Accessor;
		template<typename T>
		friend const char* parse_generic(Doc& doc, const char*& sUTF8, T& o);
		template<typename TFUN >
		friend const char* parse_map(Doc& doc, const char*& sUTF8, TFUN parseFun);
		const char* parseString(const char*& str, String& ss, int &ls, IFI64* l8s);
		const char* parseKeyString(const char*& str, String& ss);


		template<typename T>
		friend const char* object_value_setter(Doc& doc, std::map<std::string, T>& o, const char*& sUTF8, const String& sKey);
		//int allocString(const char* p, const char* )

	};



	inline Accessor::Accessor(Doc& doc, Node* nd)
		:m_doc(doc),m_node(nd)
	{

	}

	inline void Accessor::set(const char* s, int len )
	{
		m_node->m_Type = NT_STRING;
		int nlen = len==-1?(int)strlen(s): len;
		if (nlen < 8)
		{
			m_node->m_int64 = *(IFI64*)(s);
			m_node->m_Flag.flag = NF_L8_STRING;
		}
		else if (nlen < SHORT_STRING_LEN)
		{
			m_node->m_ss = m_doc.addShortString(s, nlen);
			m_node->m_Flag.flag = NF_SHORT_STRING;
		}
		else
		{
			char* pnew = new char[nlen + 1];
			memcpy(pnew, s, nlen);
			pnew[nlen] = 0;
			m_node->m_ls.nidx = m_doc.addLongString(pnew);
			m_node->m_ls.nlen = nlen;
			m_node->m_Flag.flag = NF_LONG_STRING;
		}
	}

	inline void Accessor::arrayBegin()
	{
		m_node->m_Type = NT_ARRAY;
		m_node->m_array = m_doc.array_begin();
	}

	inline Accessor Accessor::arrayPush()
	{
		return Accessor(m_doc, &m_doc.array_push());
	}

	inline void Accessor::arrayEnd()
	{
		m_node->m_array.nLen = m_doc.array_end();
	}

	inline void Accessor::mapBegin()
	{
		m_node->m_Type = NT_MAP;
		m_node->m_array = m_doc.map_begin();
	}

	inline Accessor Accessor::mapPush(const char* s)
	{
		auto& kv = m_doc.map_push();
		kv.skey = m_doc.addShortString(s, (int)strlen(s)).nBuf;
		return Accessor(m_doc, &kv.value);
		//return Accessor(m_doc, &m_doc.array_push());
	}


	inline Accessor Accessor::mapPush(const String& key)
	{
		auto& kv = m_doc.map_push();
		kv.skey = key.nBuf;
		return Accessor(m_doc, &kv.value);

	}


	inline void Accessor::mapEnd()
	{
		m_node->m_array.nLen = m_doc.map_end();
	}

	inline const char* Accessor::getString() const
	{
		if (m_node->m_Flag.flag == NF_LONG_STRING)
			return m_doc.m_long_string_list[m_node->m_ls.nidx];
		else if (m_node->m_Flag.flag == NF_L8_STRING)
			return m_node->m_l8s;
		else if (m_node->m_Flag.flag == NF_SHORT_STRING)
			return m_doc.getString(m_node->m_ss.nBuf);
		else
			return NULL;
	}

	inline Accessor Accessor::getMapValue(const char* key) const
	{
		for (int i = 0; i < m_node->m_array.nLen; i++)
		{
			auto& kv = m_doc.map_get(m_node->m_array.nBuf, i);
			if (strcmp(m_doc.getString(kv.skey), key) == 0)
			{
				return Accessor(m_doc, &kv.value);
			}
		}
		return Accessor(m_doc, nullptr);
	}
	inline std::pair<const char*, Accessor> Accessor::getMapKeyValue(int i) const
	{
		auto& kv = m_doc.map_get(m_node->m_array.nBuf, i);
		return std::make_pair(m_doc.getString(kv.skey), Accessor(m_doc, &kv.value));
	}
	inline MFJSON::Accessor MFJSON::Accessor::getArrayValue(int i) const
	{
		if ( i < m_node->m_array.nLen)
		{
			auto& v = m_doc.array_get(m_node->m_array.nBuf, i);
			return Accessor(m_doc, &v);		
		}
		return Accessor(m_doc, nullptr);
	}

	inline std::string Accessor::dump(bool bVisibleFormat /* = true */)
	{
		m_doc.m_string_builder.clear();
		std::string tb;
		dumpInternal(bVisibleFormat,tb);
		return m_doc.m_string_builder.toString();
	}
#pragma warning(push)
#pragma warning(disable:4996)

	const int values[] =
	{
		1000000000,
		100000000,
		10000000,
		1000000,
		100000,
		10000,
		1000,
		100,
		10,
	};
	const  char numbers[] = "00010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979899";
	inline char* int1002buf(int n, char* p)
	{
		n <<= 1;
		if (n > 19)
		{
			*p++ = numbers[n];
		}
		*p++ = numbers[n + 1];
		return p;
	}

	inline char* int100002buf(int n, char* p)
	{
		int high = (n / 100) << 1;
		int low = (n % 100) << 1;
	
		if (high)
		{
			if (high > 19)
				*p++ = numbers[high];
			*p++ = numbers[high+1];
			*p++ = numbers[low];
		}
		else
		{
			if (low > 19)
				*p++ = numbers[low];
		}
		*p++ = numbers[low + 1];

		return p;
	}

	inline char* int100002bufptr(int n, char* p)
	{
		int high = (n / 100) << 1;
		int low = (n % 100) << 1;

		if (high)
		{
			if (high > 19)
			{
				*p++ = numbers[high];
				*p++ = '.';
				*p++ = numbers[high + 1];

			}
			else
			{
				*p++ = numbers[high + 1];
				*p++ = '.';
			}
			
			*p++ = numbers[low];
			*p++ = numbers[low+1];
		}
		else
		{
			if (low > 19)
			{
				*p++ = numbers[low];
				*p++ = '.';
				*p++ = numbers[low + 1];

			}
			else
			{
				*p++ = numbers[low + 1];
				*p++ = '.';
			}
		}

		return p;
	}

	inline char* int100002buf2(int n, char* p)
	{
		int high = (n / 100)<<1;
		int low = (n % 100)<<1;


		*p++ = numbers[high];
		*p++ = numbers[high + 1];
		*p++ = numbers[low];
		*p++ = numbers[low + 1];

		return p;
	}
	inline char* int180buf(int n, char* p)
	{
		int high = n / 10000;
		int low = n % 10000;
		p = int100002buf(high, p);
		return  int100002buf2(low, p);
	}
	inline char* int180bufptr(int n, char* p)
	{
		int high = n / 10000;
		int low = n % 10000;
		p = int100002bufptr(high, p);
		return  int100002buf2(low, p);
	}

	inline char* int180buf2(unsigned int n, char* p)
	{
		int high = n / 10000;
		int low = n % 10000;
		p = int100002buf2(high, p);
		return  int100002buf2(low, p);
	}
	inline char* int160buf2(IFI64 n, char* p)
	{
		unsigned int high = (unsigned int)(n / 100000000);
		unsigned int low = n % 100000000;
		p = int180buf2(high, p);
		return  int180buf2(low, p);
	}
	inline char* int32buf(int n, char* p)
	{
		if (n < 0)
		{
			n = -n;
			*p++ = '-';
		}
		if (n < 10000)
		{
			return int100002buf(n, p);
		}
		else if (n < 100000000)
		{
			return int180buf(n, p);
		}
		else
		{
			int high = n / 100000000;
			int low = n % 100000000;
			p = int100002buf(high, p);
			return int180buf2(low, p);
		}
	}
	inline char* int32bufptr(int n, char* p)
	{

		if (n < 10000)
		{
			return int100002bufptr(n, p);
		}
		else if (n < 100000000)
		{
			return int180bufptr(n, p);
		}
		else
		{
			int high = n / 100000000;
			int low = n % 100000000;
			p = int100002bufptr(high, p);
			return int180buf2(low, p);
		}
	}

	inline char* int32buf2(int n, char* p)
	{
		if (n < 10000)
		{
			return int100002buf2(n, p);
		}
		else if (n < 100000000)
		{
			return int180buf2(n, p);
		}
		else
		{
			int high = n / 100000000;
			int low = n % 100000000;
			p = int100002buf(high, p);
			return int180buf2(low, p);
		}
	}


	inline char* int64buf(IFI64 n, char* p)
	{
		if (n < 0)
		{
			*p++ = '-';
			n = -n;;
		}
		IFI64 hightpart = n / 100000000;
		IFI64 lowpart = n % 100000000;
		if (hightpart)
		{
			p = int32buf((int)hightpart, p);
			return  int180buf2((unsigned int)lowpart, p);
		}
		else
		{
			return int32buf((unsigned int)lowpart, p) ;
		}
		
	}

	inline char* int64bufptr(IFI64 n, char* p)
	{
		if (n < 0)
		{
			*p++ = '-';
			n = -n;;
		}
		IFI64 hightpart = n / 1000000000;
		IFI64 lowpart = n % 1000000000;
		if (hightpart)
		{
			p = int32bufptr((int)hightpart, p);
	
			return  int32buf2((unsigned int)lowpart, p);
	
		}
		else
		{
			return int32bufptr((unsigned int)lowpart, p);
		}
	}



	//static const float power10[] =
	//{
	//	1,1e1,1e2,1e3,1e4,1e5,1e6,1e7,1e8,1e9,1e10,1e11,1e12,1e13,1e14,1e15,1e16,1e17,1e18,1e19,1e20,1e21,1e22,1e23,1e24,1e25,1e26,1e27,1e28,1e29,1e30,1e31,1e32,1e33,1e34,1e35,1e36,
	//};
	//static const float power10_inv[] =
	//{
	//	1,1e-1,1e-2,1e-3,1e-4,1e-5,1e-6,1e-7,1e-8,1e-9,1e-10,1e-11,1e-12,1e-13,1e-14,1e-15,1e-16,1e-17,1e-18,1e-19,1e-20,1e-21,1e-22,1e-23,1e-24,1e-25,1e-26,1e-27,1e-28,1e-29,1e-30,1e-31,1e-32,1e-33,1e-34,1e-35,1e-36,
	//};

	inline char* float2bufnon0(float f, char* buf)
	{
		int fb = *(int*)&f;
		char* begins = buf;
		if (fb & 0x80000000)
		{
			*buf++ = '-';
		}

		int exp = (fb & 0x7F800000) >> 23;
		int intpart = fb & 0x7fffff | (1 << 23);
		exp -= 127;
		
		if (exp > 0)
		{
			if (exp > 23)
			{
				int exp2n = exp - 23;
				float mul = 1.0f;
				while (exp2n > 26)
				{
					mul *= float(1 << 26);
					exp2n -= 26;
				}
				mul *= 1 << exp2n;
			
				mul += 0.1f;
				float nnewintpart = mul*intpart;
				int e = (int)floor(log10(nnewintpart));
				if (e > 7)
				{
					nnewintpart /= (float)pow(10.0f, e - 7);
					e = e + 7;
				}
				char* c = buf;
				buf = MFJSON::int64bufptr((IFI64)nnewintpart, buf);
				e = e - int(buf - c - 2);
				*buf++ = 'e';
				buf = MFJSON::int1002buf(e, buf);

			}
			else
			{
				int ptlen = 23 - exp;
				//unsigned int ptpart = 0;
				//unsigned int m = 100000000;
				//for (int i = 1; i < ptlen; i++)
				//{
				//	if (intpart & (1 << (ptlen - i)))
				//	{
				//		ptpart += (m >> i);

				//	}
				//}
				//ptpart += 500;
				//ptpart /= 1000;
				//ptpart *= 1000;
				if (f < 0.0f)
					f = -f;

				intpart >>= ptlen;
				unsigned int ptpart = (unsigned int)((f - (float)(intpart)) * 100000000);

				buf = MFJSON::int32buf(intpart, buf);

				*buf++ = '.';
				if (ptpart == 0)
				{
					*buf++ = '0';
				}
				else
				{
					
					
					buf = MFJSON::int180buf2(ptpart, buf);
					while (*(buf - 1) == '0')buf--;

				}
			}
		}
		else
		{


			if (f < 0.0f)
				f = -f;

			int e = (int)floor(log10(f));
			if (e > -7)
			{

				int ipt = int(f * 100000000);//pow(10.0, -22);
				ipt += 50;
				ipt /= 100;
				ipt *= 100;
	
				if (ipt >= 100000000)
				{
					*buf++ = ipt / 100000000 + '0';
					ipt -= 100000000;
				}
				else
				{
					*buf++ = '0';

				}
				if (ipt)
				{
					*buf++ = '.';

					buf = MFJSON::int180buf2(ipt, buf);
					while (*(buf - 1) == '0')buf--;
				}
				else
				{
					*buf++ = '.';
					*buf++ = '0';

				}

			}
			else
			{
				f *= (float)pow(10.0, - e + 7);
				f += 1.f;
				//ptpart = ptpart/mul;
				char* c = buf;
				buf = MFJSON::int32bufptr((int)f, buf);
				while (*(buf - 1) == '0')buf--;
				if (*(buf - 1) == '.')buf--;
				//e = buf - c - e + 1;
				*buf++ = 'e';
				*buf++ = '-';
				buf = MFJSON::int1002buf(-e, buf);
			}
			

		}

		return buf;
	}

	inline char* float2buf(float f, char* buf)
	{
		int fb = *(int*)&f;
		if (fb)
		{
			return float2bufnon0(f, buf);
		}
		else
		{
			*(int*)buf = *(int*)"0.0";
			return buf + 3;
		}
	}

	inline char* double2bufnon0(double f, char* buf)
	{
		long long fb = *(long long*)&f;

		char* begins = buf;
		if (fb & 0x8000000000000000)
		{
			*buf++ = '-';
		}

		int exp = (fb & 0x7FF0000000000000ll) >> 52;
		long long intpart = (fb & 0xFFFFFFFFFFFFFll) | (1ll << 52);
		exp -= 1023;

		if (exp > 0)
		{
			if (exp > 52)
			{
				int exp2n = exp - 52;
				double mul = 1;
				while (exp2n > 60)
				{
					mul *= pow(2, 60);
					exp2n -= 60;
				}
				mul *= pow(2, exp2n);

				mul += 0.1f;
				double nnewintpart = mul*intpart;
				int e = (int)floor(log10(nnewintpart));
				if (e > 16)
				{
					nnewintpart /= pow(10.0, e - 16);
					e = e + 16;
				}
				char* c = buf;
				buf = MFJSON::int64bufptr((IFI64)nnewintpart, buf);
				e = e - int(buf - c - 2);
				*buf++ = 'e';
				buf = MFJSON::int1002buf(e, buf);

			}
			else
			{
				int ptlen = 52 - exp;
				//unsigned long long ptpart = 0;
				//unsigned long long m = 10000000000000000;
				//for (int i = 1; i < ptlen; i++)
				//{
				//	if (intpart & (1ll << (ptlen - i)))
				//	{
				//		ptpart += m >> i;

				//	}
				//}
				//ptpart += 500;
				//ptpart /= 1000;
				//ptpart *= 1000;

				if (f < 0.0)
					f = -f;

				intpart >>= ptlen;
				unsigned long long ptpart = (unsigned long long)((f - (double)(intpart)) * 10000000000000000);
				buf = MFJSON::int64buf(intpart, buf);

				*buf++ = '.';
				if (ptpart == 0)
				{
					*buf++ = '0';
				}
				else
				{
					//int high = ptpart / 10000000000000000;
					//ptpart = ptpart % 100000000000000000;

					buf = MFJSON::int160buf2(ptpart, buf);
					while (*(buf - 1) == '0')buf--;

				}
			}
		}
		else
		{



			if (f < 0.0)
				f = -f;
			int e = (int)floor(log10(f));
			if (e > -15)
			{

				long long ipt = (long long)(f * 10000000000000000);//pow(10.0, -22);
				ipt += 50;
				ipt /= 100;
				ipt *= 100;

				if (ipt >= 10000000000000000)
				{
					*buf++ = char(ipt / 10000000000000000 + '0');
					ipt -= 10000000000000000;
				}
				else
				{
					*buf++ = '0';

				}
				if (ipt)
				{
					*buf++ = '.';

					buf = MFJSON::int160buf2(ipt, buf);
					while (*(buf - 1) == '0')buf--;
				}
				else
				{
					*buf++ = '.';
					*buf++ = '0';

				}

			}
			else
			{
				f *= pow(10.0, -e + 15);
				f += 0.1f;
				//ptpart = ptpart/mul;
				char* c = buf;
				buf = MFJSON::int64bufptr((IFI64)f, buf);
				while (*(buf - 1) == '0')buf--;
				if (*(buf - 1) == '.')buf--;
				//e = buf - c - e + 1;
				*buf++ = 'e';
				*buf++ = '-';
				buf = MFJSON::int1002buf(-e, buf);
			}


		}

		return buf;
	}

	inline char* double2buf(double f, char* buf)
	{
		long long fb = *(long long*)&f;
		if (fb)
		{
			return double2bufnon0(f, buf);
		}
		else
		{
			*(int*)buf = *(int*)("0.0");
			return buf + 3;
		}
	}

	inline void Accessor::dumpInternal(bool bVisibleFormat,std::string& tb)
	{
		switch (m_node->m_Type)
		{
		case NT_NULL:
			m_doc.m_string_builder.append("null", 4);
			break;

		case NT_BOOL:
			m_node->m_b ? m_doc.m_string_builder.append("true", 4) : m_doc.m_string_builder.append("false", 5);
			break;

		case NT_NUMBER:
		{
			char buf[64];
			if (m_node->m_Flag.flag == NF_FLOAT)
			{
				int nlen = int(float2buf(m_node->m_f, buf) - buf);//sprintf(buf, "%g", m_node->m_f);
				m_doc.m_string_builder.append(buf, nlen);
				break;
			}
			else if (m_node->m_Flag.flag == NF_INT32)
			{
				int nlen = int(int32buf(m_node->m_int32, buf) - buf);//sprintf(buf, "%d", m_node->m_int32);
				m_doc.m_string_builder.append(buf, nlen);
				break;
			}
			else if (m_node->m_Flag.flag == NF_INT64)
			{
				int nlen = int(int64buf(m_node->m_int64,buf)-buf);//sprintf(buf, "%lld", m_node->m_int64);
				m_doc.m_string_builder.append(buf, nlen);
				break;
			}
			else
			{
				int nlen = int(double2buf(m_node->m_df,buf)-buf);//sprintf(buf, "%g", m_node->m_df);
				m_doc.m_string_builder.append(buf, nlen);
				break;
			}
		}
		break;
		case NT_STRING:
		{
			m_doc.m_string_builder.append("\"", 1);
			m_doc.m_string_builder.append(getString(), (int)strlen(getString()));
			m_doc.m_string_builder.append("\"", 1);

		}
		break;
		case NT_ARRAY:
		{
			m_doc.m_string_builder.append("[", 1);
			if (bVisibleFormat)
			{
				m_doc.m_string_builder.append("\r\n", 2);
				tb.push_back('\t');
			}

			for (int i = 0; i < getArrayLength(); i ++)
			{
				if (i > 0)
				{
					m_doc.m_string_builder.append(",", 1);
					if (bVisibleFormat)
					{
						m_doc.m_string_builder.append("\r\n", 2);
					}
				}
				if(bVisibleFormat)
					m_doc.m_string_builder.append(tb.c_str(), (int)tb.length());

				getArrayValue(i).dumpInternal(bVisibleFormat,tb);
				
			}
			if (bVisibleFormat)
			{
				m_doc.m_string_builder.append("\r\n", 2);
				tb.pop_back();
				m_doc.m_string_builder.append(tb.c_str(), (int)tb.length());
			}
			m_doc.m_string_builder.append("]", 1);
		}
		break;
		case NT_MAP:
		{
			m_doc.m_string_builder.append("{", 1);
			if (bVisibleFormat)
			{
				m_doc.m_string_builder.append("\r\n", 2);
				tb.push_back('\t');
			}
			for (int i = 0; i < getMapLength(); i++)
			{
				if (i > 0)
				{
					m_doc.m_string_builder.append(",", 1);
					if (bVisibleFormat)
					{
						m_doc.m_string_builder.append("\r\n", 2);
					}
				}

				if (bVisibleFormat)
					m_doc.m_string_builder.append(tb.c_str(), (int)tb.length());


				auto kv = getMapKeyValue(i);
				m_doc.m_string_builder.append("\"", 1);
				m_doc.m_string_builder.append(kv.first, (int)strlen(kv.first));
				m_doc.m_string_builder.append("\":", 2);
				kv.second.dumpInternal(bVisibleFormat,tb);
#ifdef _DEBUG
				m_doc.m_string_builder.append(" ", 1);
#endif
			}
			if (bVisibleFormat)
			{
				m_doc.m_string_builder.append("\r\n", 2);
				tb.pop_back();
				m_doc.m_string_builder.append(tb.c_str(), (int)tb.length());
			}
			m_doc.m_string_builder.append("}", 1);
		}
		break;
		default:
			break;
		}
	}

#pragma warning(pop)

	//im
	inline bool string_cmp(const char* a, const char* b, int len)
	{
		while (*a++ == *b++&&len)len--;
		return len == 0;

	}

	inline bool StringEqual::operator()(const String& a, const String& b) const
	{
		//m_scounternum++;

		if (a.nBuf == b.nBuf)
			return true;
		else if (a.nBuf == -1)
		{
			if (a.nHash == lasthash)
				return lastresult;
			int len;
			lasthash = a.nHash;

			const char* pb = pool.getString(b.nBuf,len);
			if (pool.m_temp_short_string_len == len)
			{
				lastresult = string_cmp(pool.m_temp_short_string, pb, len);
				return lastresult;
			}

		}
		else if (b.nBuf == -1)
		{
			if (a.nHash == lasthash)
				return lastresult;
			lasthash = a.nHash;
			int len;
			const char* pb = pool.getString(a.nBuf, len);
			if (pool.m_temp_short_string_len == len)
			{
				lastresult = string_cmp(pool.m_temp_short_string, pb, len);
				return lastresult;
			}
			
		}
		lastresult = false;
		return false;
		
	}


	//__declspec(selectany) int StringEqual::m_scounternum = 0;

#define SKIP_SPACE(c) 	while (*c==' ' || *c == '\r' || *c == '\n' || *c=='\t')c++;
#define IS_NAME_FIRST_VALID_CHAR(c) (c>='a'&& c<='z' || c>='A' && c <='Z' || c=='_')
#define IS_NAME_VALID_CHAR(c) (IS_NAME_FIRST_VALID_CHAR(c)||(c>='0'&&c<='9'))

	const int cvalue_null = *(int*)"null";
	const int cvalue_true = *(int*)"true";
	const int cvalue_false = *(int*)"fals";
	inline const char* Node::parse(Doc& doc, const char*& sUTF8)
	{
		SKIP_SPACE(sUTF8);
		switch (*sUTF8)
		{
		case 'n':
			if (*(int*)sUTF8==cvalue_null)
			{
				m_Type = NT_NULL;
				sUTF8 += 4;
				return NULL;
			}

		case 't':
			if (*(int*)sUTF8 == cvalue_true)
			{
				m_Type = NT_BOOL;
				m_b = true;
				sUTF8 += 4;
				return nullptr;
			}
	
		case 'f':
			if (*(int*)sUTF8 == cvalue_false && *(sUTF8+4)=='e')
			{
				m_Type = NT_BOOL;
				m_b = false;
				sUTF8 += 5;
				return nullptr;
			}

		case '\"':
			return parseString(doc, sUTF8);
		case '{':
			return parseMap(doc, sUTF8);
		case '[':
			return parseArray(doc, sUTF8);
		default:
		{
			if (*sUTF8 == '-' || (*sUTF8 >= '0' && *sUTF8 <= '9'))
			{
				return parseNumber(doc,sUTF8);
			}
		}
			break;
		}
		return sUTF8;
	}
	inline const char* Node::parseMap(Doc& doc, const char*& sUTF8)
	{

		sUTF8++;
		m_Type = NT_MAP;
		m_array = doc.map_begin();
		while (true)
		{
			//IFString sKey(IFString::EC_UTF8);

			SKIP_SPACE(sUTF8);

			if (*sUTF8 == '}')
			{
				sUTF8++;
				break;
			}


			String ss;
			//int ls;

			if (auto err = doc.parseKeyString(sUTF8, ss))
			{
				m_array.nLen = doc.map_end();
				return err;
			}
		
			auto& kv = doc.map_push();
			kv.skey = ss.nBuf;
			SKIP_SPACE(sUTF8);

			if (*sUTF8 != ':')
			{
				m_array.nLen = doc.map_end();

				return sUTF8;
			}
			sUTF8++;
			SKIP_SPACE(sUTF8);

			if (const char* sError = kv.value.parse(doc, sUTF8))
			{
				m_array.nLen = doc.map_end();

				return sError;
			}
			SKIP_SPACE(sUTF8);
			if (*sUTF8 == ',')
			{
				sUTF8++;
				SKIP_SPACE(sUTF8);
				if (*sUTF8 == '}')
				{
					sUTF8++;
					SKIP_SPACE(sUTF8);
					break;
				}
				continue;
			}
			else if (*sUTF8 == '}')
			{
				sUTF8++;
				SKIP_SPACE(sUTF8);
				break;
			}
			else
			{
				m_array.nLen = doc.map_end();

				return sUTF8;
			}
		}


		m_array.nLen = doc.map_end();

		return NULL;
	}

	inline const char* Node::parseArray(Doc& doc, const char*& sUTF8)
	{

		sUTF8++;
		m_Type = NT_ARRAY;
		m_array = doc.array_begin();
		SKIP_SPACE(sUTF8);
		while (true)
		{
			if (*sUTF8 == ',')
			{
				sUTF8++;
				SKIP_SPACE(sUTF8);
				if (*sUTF8 == ']')
				{
					sUTF8++;
					SKIP_SPACE(sUTF8);
					m_array.nLen = doc.array_end();
					return NULL;
				}
				//continue;
			}
			else if (*sUTF8 == ']')
			{
				sUTF8++;
				SKIP_SPACE(sUTF8);
				m_array.nLen = doc.array_end();

				return NULL;
			}

	
			auto& ary = doc.array_push();
			m_array.nLen++;
			SKIP_SPACE(sUTF8);
			if (const char* err = ary.parse(doc, sUTF8))
			{
				m_array.nLen = doc.array_end();

				return err;
			}
			SKIP_SPACE(sUTF8);

		}


		m_array.nLen = doc.array_end();
		return NULL;
	}


	inline char readHexChar(const char* s)
	{
		char a = *s - '0';
		char b = *s - 'a';
		char c = *s - 'A';
		if (a >= 0 && a < 10)
			return a;
		else if (b >= 0 && b < 6)
			return b + 10;
		else if (c >= 0 && c < 6)
			return c + 10;
		else
			return 0;
	}

	inline int readHexShort(const char* s)
	{
		char a = readHexChar(s);
		char b = readHexChar(s+1);
		char c = readHexChar(s+2);
		char d = readHexChar(s+3);
		return a << 12 | b << 8 || c << 4 | d;
	}

	inline char* parseConverString(char* ptr2, const char* ptr)
	{
		static const unsigned char firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
		int len = 0;
		unsigned uc, uc2;

		while (*ptr != '\"' && *ptr) {
			if (*ptr != '\\')
				*ptr2++ = *ptr++;
			else {
				ptr++;
				switch (*ptr) {
				case 'b':
					*ptr2++ = '\b';
					break;
				case 'f':
					*ptr2++ = '\f';
					break;
				case 'n':
					*ptr2++ = '\n';
					break;
				case 'r':
					*ptr2++ = '\r';
					break;
				case 't':
					*ptr2++ = '\t';
					break;
				case 'u': /* transcode utf16 to utf8. */
						  //sscanf(ptr + 1, "%4x", &uc);
					uc = readHexShort(ptr + 1);
					ptr += 4; /* get the unicode char. */

					if ((uc >= 0xDC00 && uc <= 0xDFFF) || uc == 0) break; /* check for invalid.	*/

					if (uc >= 0xD800 && uc <= 0xDBFF) /* UTF16 surrogate pairs.	*/
					{
						if (ptr[1] != '\\' || ptr[2] != 'u') break; /* missing second-half of surrogate.	*/
						uc2 = readHexShort(ptr + 3);
						//sscanf(ptr + 3, "%4x", &uc2);
						ptr += 6;
						if (uc2 < 0xDC00 || uc2 > 0xDFFF) break; /* invalid second-half of surrogate.	*/
						uc = 0x10000 + (((uc & 0x3FF) << 10) | (uc2 & 0x3FF));
					}

					len = 4;
					if (uc < 0x80)
						len = 1;
					else if (uc < 0x800)
						len = 2;
					else if (uc < 0x10000) len = 3;
					ptr2 += len;

					switch (len) {
					case 4:
						*--ptr2 = ((uc | 0x80) & 0xBF);
						uc >>= 6;
					case 3:
						*--ptr2 = ((uc | 0x80) & 0xBF);
						uc >>= 6;
					case 2:
						*--ptr2 = ((uc | 0x80) & 0xBF);
						uc >>= 6;
					case 1:
						*--ptr2 = (uc | firstByteMark[len]);
					}
					ptr2 += len;
					break;
				default:
					*ptr2++ = *ptr;
					break;
				}
				ptr++;
			}
		}
		return ptr2;
	}

	inline int statestrlen(const char*& ptr)
	{
		int len = 0;
		while (*ptr != '\"' && *ptr && ++len)
			if (*ptr++ == '\\') ptr++;
		return len;
	}

	inline const char* Doc::parseKeyString(const char*& str, String& ss)
	{
		++str;
		const char* ptr = str;

		int len = statestrlen(ptr);
		int rawlen = (int)(ptr - str);
		if (len >= SHORT_STRING_LEN)
			return str;
		if (rawlen==len)
		{
			ss = short_string_pool_intance.addShortString(str, len);
		}
		else
		{
			char shortbuf[SHORT_STRING_LEN];
			len =(int)(parseConverString(shortbuf, str)-shortbuf);

			ss = short_string_pool_intance.addShortString(shortbuf, len);
		}

		str += rawlen+1;
		return NULL;
	}

	inline const char* Doc::parseString( const char*& str, String& ss, int& ls, IFI64* l8s)
	{
		char* out;

		if (*str != '\"') 
			return str;
		++str;
		const char* ptr = str;

		int len = 0;
		while (*ptr != '\"' && *ptr && ++len)
			if (*ptr++ == '\\') ptr++; /* Skip escaped quotes. */
									   //IFString sValue;
		int rawlen = (int)(ptr - str);

		char shortbuf[SHORT_STRING_LEN];
		if (len > SHORT_STRING_LEN)
		{
			
			out = new char[len];
			ls = addLongString(out);
		}
		else if (len < 8 && l8s)
		{
			out = (char*)l8s;
		}
		else
		{
			out = shortbuf;

		}
		if (rawlen == len)
		{
			memcpy(out, str, len);
		}
		else
		{
			auto pEnd = parseConverString(out, str);
			*pEnd = 0;
			len = (int)(pEnd - out);
		}

	
		if (out == shortbuf)
		{
			ss = addShortString(out, len);
			ls = 0;
			if (l8s)
				*l8s = 0;
		}
		else if (out == (char*)l8s)
		{
			ls = 0;
		}
		else
		{
			if (l8s)
				*l8s = len;
			//ls = out;
		}
		ptr++;
		str = ptr;

		return NULL;
	}


	inline const char* Node::parseString(Doc& doc, const char*& str)
	{
		String ss;
		int ls;
		IFI64 l8s=0;
		if (auto err = doc.parseString(str, ss, ls,&l8s))
			return err;
		m_Type = NT_STRING;

		if (ls)
		{
			m_Flag.flag = NF_LONG_STRING;
			m_ls.nidx = ls;
			m_ls.nlen = (int)l8s;
		}
		else if (l8s)
		{
			m_Flag.flag = NF_L8_STRING;
			m_int64 = l8s;
		}
		else 
		{
			m_Flag.flag = NF_SHORT_STRING;
			m_ss = ss;
		}
		return nullptr;
	}


	inline const char* Node::parseNumber(Doc& doc, const char*& sUTF8)
	{
		int nSlen = 0;
		const char* sBegin = sUTF8;

		bool bnag = false;
		m_Type = NT_NUMBER;
		if (*sUTF8 == '-')
		{
			bnag = true;
			++sUTF8;
		}

		IFI64 d = 0;
		while (*sUTF8 >= '0'&&*sUTF8 <= '9')
		{
			d *= 10;
			d += *sUTF8 - '0';
			++sUTF8;
		}
		IFI64 decimal = 0;
		double decimal_w = 1;
		if (*sUTF8 == '.')
		{
			++sUTF8;
			while (*sUTF8 >= '0'&&*sUTF8 <= '9')
			{
				decimal *= 10;
				decimal_w *= 10;
				decimal += *sUTF8 - '0';
				++sUTF8;
			}
		}
		int E = 0;
		if (*sUTF8 == 'e' || *sUTF8 == 'E')
		{
			++sUTF8;
			bool enag = false;
			if (*sUTF8 == '-')
			{
				enag = true;
				++sUTF8;
			}
			else if (*sUTF8 == '+')
				sUTF8++;


			while (*sUTF8 >= '0'&&*sUTF8 <= '9')
			{
				E *= 10;
				E += *sUTF8 - '0';
				++sUTF8;
			}
			if (enag)
				E = -E;
		}

		if (decimal || E)
		{
			double db = (double)d;
			db += double(decimal) / decimal_w;
			if (E)
			{
				db *= pow(10, (double)E);
			}
			if (bnag)
				db = -db;
			//setValue(db);
			if (sUTF8- sBegin < 8)
			{
				m_f = (float)db;
				m_Flag.flag = NF_FLOAT;

			}
			else
			{
				m_df = db;
				m_Flag.flag = NF_DOUBLE;
			}


		}
		else
		{

			if (bnag)
				d = -d;
			if (d >> 32)
			{
				m_int64 = d;
				m_Flag.flag = NF_INT64;

			}
			else
			{
				m_int32 = (int)d;
				m_Flag.flag = NF_INT32;

			}
		}

		return NULL;
	}


	template<typename T>
	inline T MFJSON::Accessor::iteratorT<T>::operator*()
	{
		return array_get<T>(*m_accessor, m_idx);
	}


	template<typename T>
	inline void accessor_generic_setter(Accessor& acc, const std::vector<T>& ar)
	{
		acc.arrayBegin();
		for (auto& o : ar)
		{
			acc.arrayPush().set(o);
		}
		acc.arrayEnd();
	}

	template<typename T>
	inline void accessor_generic_setter(Accessor& acc, const std::unordered_map<int,T>& ar)
	{
		acc.arrayBegin();
		for (int i = 0; i < ar.size(); i ++)
		{
			auto it = ar.find(i);
			if(it!=ar.end())
				acc.arrayPush().set(it->second);
		}
		acc.arrayEnd();
	}

	template<typename T>
	inline void accessor_generic_setter(Accessor& acc, const std::map<std::string, T>& ar)
	{
		acc.mapBegin();
		for (auto& o : ar)
		{
			acc.mapPush(o.first.c_str()).set(o.second);
		}
		acc.mapEnd();
	}

	template<>
	inline void accessor_generic_setter(Accessor& acc, const std::string& s)
	{
		acc.set(s.c_str(), (int)s.length());
	}

	template<>
	inline void accessor_generic_setter(Accessor& acc, const int& i)
	{
		acc.set(i);
	}
	template<>
	inline void accessor_generic_setter(Accessor& acc, const float& f)
	{
		acc.set(f);
	}

	inline void accessor_generic_getter(Accessor& acc, int& val)
	{
		val = acc.getInt();
	}
	inline void accessor_generic_getter(Accessor& acc, IFI64& val)
	{
		val = acc.getInt64();
	}
	inline void accessor_generic_getter(Accessor& acc, float& val)
	{
		val = acc.getFloat();
	}
	inline void accessor_generic_getter(Accessor& acc, double& val)
	{
		val = acc.getDouble();
	}
	inline void accessor_generic_getter(Accessor& acc, std::string& val)
	{
		val = acc.getString();
	}
	inline void accessor_generic_getter(Accessor& acc, const char*& val)
	{
		val = acc.getString();
	}
	template<typename T>
	inline void accessor_generic_getter(Accessor& acc, std::vector<T>& val)
	{
		val.resize(acc.getArrayLength());
		for (int i = 0; i < acc.getArrayLength(); i ++)
		{
			acc.getArrayValue(i).get(val[i]);
		}
	}
	template<typename T>
	inline void accessor_generic_getter(Accessor& acc, std::map<std::string,T>& val)
	{

		for (int i = 0; i < acc.getMapLength(); i++)
		{
			auto kv = acc.getMapKeyValue(i);
			kv.second.get(val[kv.first]);
		}
	}

	const char* parse_skip(Doc& doc, const char*& sUTF8);

	template<typename T>
	class SetterGetter;


	template<typename T>
	class MemberAccessor
	{
	public:
		virtual ~MemberAccessor()
		{

		}

		virtual const char*  set(Doc& doc, T& o, const char*& sUTF8) = 0;
		virtual void get(Accessor& acc, const T& o, const char* sKeyName) = 0;

		const char* m_sMemberName;
	};

	template<typename T, typename MEMT>
	class MemberAccessorImp : public MemberAccessor<T>
	{
	public:
		MemberAccessorImp(const char* memberName, MEMT T::*ptr)
			:member_ptr(ptr)
		{
			m_sMemberName = memberName;
			//m_executer.insert(std::make_pair<std::string, SetterGetterExecuter<Value>*>("crit", &Value::crit));
		}
		virtual const char* set(Doc& doc, T& o, const char*& sUTF8)
		{
			 return parse_generic(doc, sUTF8, o.*member_ptr);

		}

		virtual void get(Accessor& acc, const T& o, const char* sKeyName)
		{
			acc.mapPush(sKeyName).set(o.*member_ptr);
		}

		MEMT T::*member_ptr;
	};

	class FullStringEqual
	{
	public:
		FullStringEqual(std::vector<std::string>& names, ShortStringPool*& pool):
			m_names(names), m_pool(pool),lastabuf(0),lastbbuf(0),lastresult(false)
		{

		}

		inline bool cmp(const String& a, const String& b) const
		{
			const char* pa;
			int alen;
			if (a.nBuf < 0)
			{
				pa = m_names[-a.nBuf - 1].c_str();
				alen = (int)m_names[-a.nBuf - 1].length();
			}
			else
				pa = m_pool->getString(a.nBuf, alen);

			const char* pb;
			int blen;
			if (b.nBuf < 0)
			{
				pb = m_names[-b.nBuf - 1].c_str();
				blen = (int)m_names[-b.nBuf - 1].length();
			}
			else
				pb = m_pool->getString(b.nBuf, blen);
			lastabuf = a.nBuf;
			lastbbuf = b.nBuf;
			if (alen == blen)
			{

				lastresult = string_cmp(pa, pb, alen);
				return lastresult;
			}
			else
			{
				lastresult = false;
				return false;
			}
		}

		inline bool operator()(const String& a, const String& b) const
		{
			//return a.nHash == b.nHash;
			if ((a.nBuf == lastabuf || a.nBuf == lastbbuf) && (b.nBuf == lastabuf || b.nBuf == lastabuf))
				return lastresult;
			else
				return cmp(a, b);
		}
		std::vector<std::string>& m_names;
		ShortStringPool*& m_pool;
		mutable int lastabuf;
		mutable int lastbbuf;
		mutable bool lastresult;
	};

	
	inline size_t hash_fun(const String& s)
	{
		return s.nHash;
	}

	class NullClass
	{

	};

	template<typename T>
	inline SetterGetter<T>* GetGetterSetter()
	{
		return &SetterGetter<T>::getInstance();
	}
	
	//template<>
	//inline SetterGetter<NullClass>* GetGetterSetter<NullClass>()
	//{
	//	return NULL;
	//}


	template<typename T, typename... SuperT>
	class SetterGetterBase
	{

	};
	template<typename T>
	class SetterGetterBase<T>
	{
	protected:
		std::unordered_map<String, MemberAccessor<T>*,StringHash, FullStringEqual> m_executer;
		std::vector<std::string> m_names;
		ShortStringPool* m_pool;
		template<typename MEMT>
		void addAccessor(const char* sName, MEMT T::* p)
		{
			int len = (int)strlen(sName);
			assert(len < SHORT_STRING_LEN);
			//auto& s = ShortStringPool::InstanceForObjRead().addShortString(sName, len);
			//const char* sKey = ShortStringPool::Instance().getString(s.nBuf);
			String s;
			m_names.push_back(sName);
			s.nBuf = -(int)m_names.size();
			s.nHash = rs_hash(sName, len);
			m_executer.insert(std::make_pair(s, new MemberAccessorImp<T, MEMT>(sName,p)));

		}
	public:
		SetterGetterBase()
			:
			m_executer(2, StringHash(), FullStringEqual(m_names, m_pool))
		{

		}
		~SetterGetterBase()
		{
			for (auto& kv : m_executer)
			{
				delete kv.second;
			}
		}

		static SetterGetter<T>& getInstance()
		{
			static SetterGetter<T> instance;
			return instance;
		}



		static const char* set(Doc& doc, T& o, const char*& sUTF8, const String& nkey)
		{
			auto& instance = getInstance();
			instance.m_pool = &doc.getStringPool();
			auto it = instance.m_executer.find(nkey);
			if (it != instance.m_executer.end())
			{
				return it->second->set(doc, o, sUTF8);
			}
			else
			{
				return parse_skip(doc, sUTF8);
			}
		}

		static void fillMember(Accessor& acc, const T& o)
		{
			auto& instance = getInstance();
			for (auto& kv : instance.m_executer)
			{
				kv.second->get(acc, o, kv.second->m_sMemberName);
			}
		}

		static void get(Accessor& acc, const T& o)
		{
			auto& instance = getInstance();
			acc.mapBegin();
			fillMember(acc, o);
			acc.mapEnd();
		}
	};

	template<typename T, typename SuperT>
	class SetterGetterBase<T,SuperT> : public SetterGetterBase<T>
	{
	public:
		static const char* set(Doc& doc, T& o, const char*& sUTF8, const String& nkey)
		{
			auto& instance = getInstance();
			instance.m_pool = &doc.getStringPool();
			auto it = instance.m_executer.find(nkey);
			if (it != instance.m_executer.end())
			{
				return it->second->set(doc, o, sUTF8);
			}
			else
			{
				return SetterGetter<SuperT>::getInstance().set(doc, o, sUTF8, nkey);
			}
			
		}

		

		static void get(Accessor& acc, const T& o)
		{
			acc.mapBegin();

			SetterGetter<SuperT>::getInstance().fillMember(acc, o);
			fillMember(acc, o);
			
			acc.mapEnd();

		}
	};




#define MFJSON_ACCESSOR_VALUE(name) addAccessor(#name, &OBJ_TYPE::name);
	
	


#define MFJSON_ACCESSOR_VALUE_BEGIN(TYPE,...) \
namespace MFJSON\
	{\
		template<>\
		class SetterGetter<TYPE> : public SetterGetterBase<TYPE, __VA_ARGS__>\
		{\
		public:\
			typedef TYPE OBJ_TYPE;\
			SetterGetter()\
			{\



#define MFJSON_ACCESSOR_VALUE_END() 	}\
	};}


	//template<typename T>
	//const char*  object_value_setter_getter(Accessor* pAcc, MFJSON::Doc* pDoc, T* o, const T* co, const char*& sUTF8, const char* sKeyName);

	template<typename T>
	const char* object_value_setter(Doc& doc, T& o, const char*& sUTF8, const String& sKey)
	{
		return SetterGetter<T>::set(doc, o, sUTF8, sKey);
		//return object_value_setter_getter(NULL, &doc, &o,NULL,  sUTF8, sKeyName);
	}

	template<class T>
	void accessor_generic_setter(Accessor& acc, const T& val)
	{
		const char* sutf8hodler = NULL;
		SetterGetter<T>::get(acc, val);
		//object_value_setter_getter(&acc, NULL, NULL,&val, sutf8hodler, NULL);
	}
	
	template<>
	inline const char* parse_generic(Doc& doc, const char*& sUTF8, int& o)
	{
		SKIP_SPACE(sUTF8);
		if (*sUTF8 == '-' || (*sUTF8 >= '0' && *sUTF8 <= '9'))
		{
			Node& nd = doc.getTempNode();
			if (nd.parseNumber(doc, sUTF8) || nd.m_Type != NT_NUMBER)
			{
				return sUTF8;
			}
			else
			{
				o = Accessor(doc, &nd).getInt();
				return NULL;
			}
		}
		return sUTF8;
	}

	template<>
	inline const char* parse_generic(Doc& doc, const char*& sUTF8, float& o)
	{
		SKIP_SPACE(sUTF8);
		if (*sUTF8 == '-' || (*sUTF8 >= '0' && *sUTF8 <= '9'))
		{
			auto& nd = doc.getTempNode();
			if (nd.parseNumber(doc, sUTF8) || nd.m_Type != NT_NUMBER)
			{
				return sUTF8;
			}
			else
			{
				o = Accessor(doc, &nd).getFloat();
				return NULL;
			}
		}
		else
			return sUTF8;

	}

	template<>
	inline const char* parse_generic(Doc& doc, const char*& sUTF8, double& o)
	{
		SKIP_SPACE(sUTF8);
		if (*sUTF8 == '-' || (*sUTF8 >= '0' && *sUTF8 <= '9'))
		{
			Node& nd = doc.getTempNode();
			if (nd.parseNumber(doc, sUTF8) || nd.m_Type != NT_NUMBER)
			{
				return sUTF8;
			}
			else
			{
				o = Accessor(doc, &nd).getFloat();
				return NULL;
			}
		}
		else
			return sUTF8;

	}

	template<>
	inline const char* parse_generic(Doc& doc, const char*& sUTF8, const char*& o)
	{
		SKIP_SPACE(sUTF8);
		if (*sUTF8 == '\"')
		{
			Node& nd = doc.getTempNode();
			if (nd.parseString(doc, sUTF8) && nd.m_Type == NT_STRING)
				return sUTF8;
			else
			{
				o = Accessor(doc, &nd).getString();
				return NULL;
			}
		}
		else
			return sUTF8;
	}
	template<>
	inline const char* parse_generic(Doc& doc, const char*& sUTF8, std::string& o)
	{
		const char* s;
		if (const char* serr = parse_generic(doc, sUTF8, s))
		{
			return serr;
		}
		o = s;
		return NULL;
	}


	template<typename T>
	const char* parse_array(Doc& doc, const char*& sUTF8, const T& fun)
	{
		if (*sUTF8 != '[')
		{
			return parse_skip(doc, sUTF8);
		}
		sUTF8++;
		SKIP_SPACE(sUTF8);
		while (true)
		{
			if (*sUTF8 == ',')
			{
				sUTF8++;
				SKIP_SPACE(sUTF8);
				if (*sUTF8 == ']')
				{
					sUTF8++;
					SKIP_SPACE(sUTF8);
					return NULL;
				}
			}
			else if (*sUTF8 == ']')
			{
				sUTF8++;
				SKIP_SPACE(sUTF8);

				return NULL;
			}

			//fun();
			//o.resize(o.size() + 1);

			SKIP_SPACE(sUTF8);
			if (const char* err = fun(sUTF8))//parse_generic(doc, sUTF8, o.back()))
			{
				return err;
			}
			SKIP_SPACE(sUTF8);

		}
		return NULL;
	}

	template<typename T>
	const char* parse_generic(Doc& doc, const char*& sUTF8, std::vector<T>& o)
	{
		return parse_array(doc, sUTF8, [&](const char*& sutf8) 
		{
			o.resize(o.size() + 1);
			return parse_generic(doc, sUTF8, o.back());
		});
	}

	template<typename T>
	const char* parse_generic(Doc& doc, const char*& sUTF8, std::unordered_map<int, T>& o)
	{
		return parse_array(doc, sUTF8, [&](const char*& sutf8)->const char*
		{
			T d;
			if (auto p = parse_generic(doc, sUTF8, d))
				return p;

			o.insert(std::make_pair((int)o.size(),d));
			return NULL;
		});
	}

	template<typename T>
	const char* object_value_setter(Doc& doc, std::map<std::string,T>& o, const char*& sUTF8, const String& sKey)
	{
		auto sKeyName = doc.getString(sKey.nBuf);
		return parse_generic(doc,sUTF8, o[sKeyName]);
	}

	template<typename TFUN >
	inline const char* parse_map(Doc& doc, const char*& sUTF8, TFUN parseFun)
	{
		if (*sUTF8 != '{')
		{
			return parse_skip(doc,sUTF8);
		}
		sUTF8++;
		while (true)
		{
			SKIP_SPACE(sUTF8);
			if (*sUTF8 == '}')
			{
				sUTF8++;
				break;
			}

			String ss;

			if (auto err = doc.parseKeyString(sUTF8, ss ))
			{
				return err;
			}

			//const char* sKey = doc.getString(ss.nBuf);

			SKIP_SPACE(sUTF8);
			if (*sUTF8 != ':')
			{
				return sUTF8;
			}
			sUTF8++;
			SKIP_SPACE(sUTF8);


			if (const char* serr = parseFun(sUTF8, ss))
				return serr;

			SKIP_SPACE(sUTF8);
			if (*sUTF8 == ',')
			{
				sUTF8++;
				SKIP_SPACE(sUTF8);
				if (*sUTF8 == '}')
				{
					sUTF8++;
					SKIP_SPACE(sUTF8);
					break;
				}
				continue;
			}
			else if (*sUTF8 == '}')
			{
				sUTF8++;
				SKIP_SPACE(sUTF8);
				break;
			}
			else
			{

				return sUTF8;
			}
		}



		return NULL;
	}


	template<typename T>
	inline const char* parse_generic(Doc& doc, const char*& sUTF8, T& o)
	{
		
		return parse_map(doc, sUTF8, [&](const char*& sutf8, const String& sKey) ->const char*
		{
			return object_value_setter(doc, o, sutf8, sKey);
		});
	
		
	}


	inline const char* parse_skip_map(Doc& doc, const char*& sUTF8)
	{
		return parse_map(doc, sUTF8, [&](const char*& sutf8, const String& sKey)
		{
			return parse_skip(doc, sUTF8);
		});
	}
	inline const char* parse_skip_number(Doc& doc, const char*& sUTF8)
	{
		if (*sUTF8 == '-')
			sUTF8++;
		while (*sUTF8 >= '0'&&*sUTF8 <= '9')
		{
			++sUTF8;
		}
		SKIP_SPACE(sUTF8);
		if (*sUTF8 == '.' || *sUTF8 == 'e' || *sUTF8 == 'E')
		{
			++sUTF8;
			if (*sUTF8 == '-')
				sUTF8++;
			while (*sUTF8 >= '0'&&*sUTF8 <= '9')
			{
				++sUTF8;
			}
		}
		return NULL;
	}
	inline const char* parse_skip_string(Doc& doc, const char*& sUTF8)
	{
		if (*sUTF8!='\"')
		{
			return sUTF8;
		}
		++sUTF8;
		while (*sUTF8)
		{
			if (*sUTF8 == '\\') 
				sUTF8++;
			else if (*sUTF8 == '\"')
			{
				return NULL;
			}
			++ sUTF8;
		}

		return sUTF8;
	}

	inline const char* parse_skip_array(Doc& doc, const char*& sUTF8)
	{
		return parse_array(doc, sUTF8, [&](const char*& sutf8)
		{
			return parse_skip(doc, sutf8);
		});
	}
	inline const char* parse_skip(Doc& doc, const char*& sUTF8)
	{
		SKIP_SPACE(sUTF8);
		switch (*sUTF8)
		{
		case 'n':
			if (*(int*)sUTF8 == cvalue_null)
			{
				sUTF8 += 4;
				return NULL;
			}

		case 't':
			if (*(int*)sUTF8 == cvalue_true)
			{

				sUTF8 += 4;
				return nullptr;
			}

		case 'f':
			if (*(int*)sUTF8 == cvalue_false && *(sUTF8 + 4) == 'e')
			{
				sUTF8 += 5;
				return nullptr;
			}

		case '\"':
			return parse_skip_string(doc, sUTF8);
		case '{':
			return parse_skip_map(doc, sUTF8);
		case '[':
			return parse_skip_array(doc, sUTF8);
		default:
		{
			if (*sUTF8 == '-' || (*sUTF8 >= '0' && *sUTF8 <= '9'))
			{
				return parse_skip_number(doc, sUTF8);
			}
		}
		break;
		}
		return sUTF8;
	}

}

