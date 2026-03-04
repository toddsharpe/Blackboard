#pragma once
#pragma once

#include "Crc.h"
#include "TypeCodes.h"
#include <functional>
#include <unordered_map>

using BlackboardID = uint32_t;

BlackboardID BLACKBOARD_ID(const char* name)
{
	return Crc::Calc32(name);
}

/*
 * Access.
 */
struct Access
{
	constexpr Access(bool read, bool write) :
		Read(read),
		Write(write)
	{

	}

	const bool Read;
	const bool Write;
};

static inline constexpr Access ReadOnly(true, false);
static inline constexpr Access WriteOnly(false, true);
static inline constexpr Access ReadWrite(true, true);

class BlackboardBuilder;
class BlackboardView;

/*
 * Token.
 */
struct Token
{
public:
	friend BlackboardBuilder;
	friend BlackboardView;

private:
	BlackboardID m_id;
};

template <typename T>
class Input : public Token
{
public:
	using T_Value = T;
	constexpr Input() : Token()
	{

	}
	static constexpr Access Access = ReadOnly;
};

template <typename T>
class Output : public Token
{
public:
	using T_Value = T;
	constexpr Output() : Token()
	{

	}
	static constexpr Access Access = WriteOnly;
};

template <typename T>
class State : public Token
{
public:
	using T_Value = T;
	constexpr State() : Token()
	{

	}
	static constexpr Access Access = ReadWrite;
};

/*
 * Value.
 */
template <typename T>
class Value
{
public:
	using ChangedCallback = std::function<bool(const T&, const T&)>;

	constexpr Value() :
		m_value(),
		m_callback()
	{

	}

	Value(const T& init) :
		m_value(init),
		m_callback()
	{

	}

	Value(const T& init, const ChangedCallback& callback) :
		m_value(init),
		m_callback(callback)
	{

	}

	const T& Get() const
	{
		return m_value;
	}

	void Set(const T& value)
	{
		const bool allowed = m_callback ? m_callback(m_value, value) : true;
		if (allowed)
			m_value = value;
	}

private:
	T m_value;
	ChangedCallback m_callback;
};

/*
 * Blackboard Impl.
 */
template <typename T>
class BlackboardImpl
{
public:
	BlackboardImpl() :
		m_elements()
	{

	}

	bool Add(const BlackboardID id, const T& init = T())
	{
		return m_elements.emplace(id, Value<T>(init)).second;
	}

	bool Add(const BlackboardID id, const typename Value<T>::ChangedCallback& callback, const T& init = T())
	{
		return m_elements.emplace(id, Value<T>(init, callback)).second;
	}

	bool Contains(const BlackboardID id) const
	{
		return m_elements.count(id) != 0;
	}

	void Set(const BlackboardID id, const T& value)
	{
		m_elements.at(id).Set(value);
	}

	const T& Get(const BlackboardID id) const
	{
		return m_elements.at(id).Get();
	}

	size_t Count() const
	{
		return m_elements.size();
	}

private:
	std::unordered_map<BlackboardID, typename Value<T>> m_elements;
};

/*
 * Blackboard.
 */
template <typename... Types>
class Blackboard
{
public:
	Blackboard() : m_elements()
	{

	}

	template <typename T>
	bool Add(const BlackboardID id, const T& init = T())
	{
		return Get<T>().Add(id, init);
	}

	template <typename T>
	bool Add(const BlackboardID id, const typename Value<T>::ChangedCallback& callback, const T& init = T())
	{
		return Get<T>().Add(id, callback, init);
	}

	template <typename T>
	bool Contains(const BlackboardID id) const
	{
		return Get<T>().Contains(id);
	}

	template <typename T>
	void Set(const BlackboardID id, const T& value)
	{
		Get<T>().Set(id, value);
	}

	template <typename T>
	const T& Get(const BlackboardID id) const
	{
		return Get<T>().Get(id);
	}

private:
	std::tuple<BlackboardImpl<Types>...> m_elements;

	template <unsigned int Index>
	using getTypeOfElement = typename std::tuple_element<Index, decltype(m_elements)>::type;

	template<typename T, unsigned int Index>
	using isRightElement = std::is_same<getTypeOfElement<Index>, T>;

	template<typename T, unsigned int Index = 0>
	struct FindElement : public std::conditional_t<
		isRightElement<T, Index>::value,
		std::integral_constant<decltype(Index), Index>,
		FindElement<T, Index + 1>>
	{};

	template <typename T>
	BlackboardImpl<T>& Get()
	{
		constexpr unsigned int index = FindElement<BlackboardImpl<T>>::value;
		return std::get<index>(m_elements);
	}

	template <typename T>
	const BlackboardImpl<T>& Get() const
	{
		return const_cast<Blackboard*>(this)->Get<T>();
	}
};

/*
 * Blackboard Instance.
 */
using str_t = const char*;
using BlackboardInst = Blackboard<
	uint8_t, uint16_t, uint32_t, uint64_t,
	int8_t, int16_t, int32_t, int64_t,
	float, double,
	str_t>;

/*
 * Blackboard View.
 */
class BlackboardView
{
public:
	BlackboardView(BlackboardInst& blackboard) :
		m_blackboard(blackboard)
	{

	}

	template <typename T>
	const T::T_Value& Get(const T& token) const
	{
		static_assert(T::Access.Read);

		return m_blackboard.Get<typename T::T_Value>(token.m_id);
	}

	template <typename T>
	void Set(const T& token, const T::T_Value value) const
	{
		static_assert(T::Access.Write);

		return m_blackboard.Set<typename T::T_Value>(token.m_id, value);
	}

private:
	BlackboardInst& m_blackboard;
};

/*
 * Blackboard Store.
 */
struct BlackboardStore
{
	BlackboardStore() : Blackboard()
	{

	}

	BlackboardInst Blackboard;
};

/*
 * Blackboard Builder.
 */
class BlackboardBuilder
{
public:
	enum class Op
	{
		Add,
		Get
	};
	typedef void (*Operation)(const Op op, const char*, const TypeCode);
	BlackboardBuilder(BlackboardStore& store, const Operation onOperation = nullptr) :
		m_store(store),
		m_onOperation(onOperation),
		m_path("Root")
	{

	}
	
	template <size_t N>
	BlackboardBuilder(BlackboardStore& store, const char (&path)[N], const Operation onOperation = nullptr) :
		m_store(store),
		m_onOperation(onOperation),
		m_path()
	{
		memcpy(m_path, path, N);
	}

	BlackboardBuilder SubBuilder(const char *name) const
	{
		char full[MaxPath] = {};
		snprintf(full, sizeof(full), "%s.%s", m_path, name);
		return BlackboardBuilder(m_store, full, m_onOperation);
	}

	BlackboardView View() const
	{
		return BlackboardView(m_store.Blackboard);
	}

	template <typename T>
	bool Add(const char* name, T& token, T::T_Value init = T::T_Value()) const
	{
		static_assert(T::Access.Write);
		
		char full[MaxPath] = {};
		const int ret = snprintf(full, sizeof(full), "%s.%s", m_path, name);
		if (ret >= MaxPath)
			return false;

		const BlackboardID id = BLACKBOARD_ID(full);
		if (!m_store.Blackboard.Add<typename T::T_Value>(id, init))
			return false;

		if (m_onOperation)
			m_onOperation(Op::Add, full, GetTypeCode<typename T::T_Value>());

		token.m_id = id;
		return true;
	}

	template <typename T>
	bool Get(const char* name, T& token) const
	{
		static_assert(T::Access.Read);
		
		char full[MaxPath] = {};
		const int ret = snprintf(full, sizeof(full), "%s.%s", m_path, name);
		if (ret >= MaxPath)
			return false;

		const BlackboardID id = BLACKBOARD_ID(full);
		if (!m_store.Blackboard.Contains<typename T::T_Value>(id))
			return false;

		if (m_onOperation)
			m_onOperation(Op::Get, full, GetTypeCode<typename T::T_Value>());

		token.m_id = id;
		return true;
	}

private:
	static constexpr size_t MaxPath = 128;

	BlackboardStore& m_store;
	Operation m_onOperation;
	char m_path[MaxPath];
};
