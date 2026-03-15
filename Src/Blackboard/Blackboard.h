#pragma once
#pragma once

#include "Crc.h"
#include "TypeCodes.h"
#include "Types.h"
#include <functional>
#include <unordered_map>
#include <cstring>
#include <algorithm>

using BlackboardID = uint32_t;

BlackboardID BLACKBOARD_ID(const char* name)
{
	return Crc::Calc32(name);
}

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

	constexpr Token() :
		m_id()
	{

	}

	bool IsValid() const
	{
		return m_id != 0;
	}

private:
	BlackboardID m_id;
};

struct InputToken : public Token
{
public:
	constexpr InputToken(const TypeCode type) :
		Token(),
		Type(type)
	{

	}
	const TypeCode Type;
};

struct OutputToken : public Token
{
public:
	constexpr OutputToken(const TypeCode type) :
		Token(),
		Type(type)
	{

	}
	const TypeCode Type;
};

struct StateToken : public Token
{
public:
	constexpr StateToken(const TypeCode type) :
		Token(),
		Type(type)
	{

	}
	const TypeCode Type;
};

template <typename T>
class Input : public Token
{
public:
	using T_Value = T;
	constexpr Input() :
		Token()
	{

	}
	static constexpr bool CanRead = true;
	static constexpr bool CanWrite = false;
};

template <typename T>
class Output : public Token
{
public:
	using T_Value = T;
	constexpr Output() :
		Token()
	{

	}
	static constexpr bool CanRead = false;
	static constexpr bool CanWrite = true;
};

template <typename T>
class State : public Token
{
public:
	using T_Value = T;
	constexpr State() :
		Token()
	{

	}
	static constexpr bool CanRead = true;
	static constexpr bool CanWrite = true;
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

	constexpr Value(const T& init) :
		m_value(init),
		m_callback()
	{

	}

	constexpr Value(const T& init, const ChangedCallback& callback) :
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

	T& Ref()
	{
		return m_value;
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
	constexpr BlackboardImpl() :
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

	T& Ref(const BlackboardID id)
	{
		return m_elements.at(id).Ref();
	}

	size_t Count() const
	{
		return m_elements.size();
	}

private:
	std::unordered_map<BlackboardID, Value<T>> m_elements;
};

/*
 * Blackboard.
 */
template <typename... Types>
class Blackboard
{
public:
	constexpr Blackboard() : m_elements()
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

	template <typename T>
	T& Value(const BlackboardID id)
	{
		return Get<T>().Ref(id);
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
	constexpr BlackboardView() :
		m_blackboard()
	{

	}

	constexpr BlackboardView(BlackboardInst* blackboard) :
		m_blackboard(blackboard)
	{

	}

	template <typename T>
	const typename T::T_Value& Get(const T& token) const
	{
		static_assert(T::CanRead);

		return m_blackboard->Get<typename T::T_Value>(token.m_id);
	}

	template <typename T_Value>
	const T_Value& Get(const InputToken& token) const
	{
		return m_blackboard->Get<T_Value>(token.m_id);
	}

	template <typename T_Value>
	const T_Value& Get(const StateToken& token) const
	{
		return m_blackboard->Get<T_Value>(token.m_id);
	}

	template <typename T>
	const typename T::T_Value& operator[](const T& token) const
	{
		static_assert(T::CanRead);

		return m_blackboard->Get<typename T::T_Value>(token.m_id);
	}

	template <typename T>
	void Set(const T& token, const typename T::T_Value value) const
	{
		static_assert(T::CanWrite);

		return m_blackboard->Set<typename T::T_Value>(token.m_id, value);
	}

	template <typename T_Value>
	void Set(const OutputToken& token, const T_Value value) const
	{
		return m_blackboard->Set<T_Value>(token.m_id, value);
	}

	template <typename T_Value>
	void Set(const StateToken& token, const T_Value value) const
	{
		return m_blackboard->Set<T_Value>(token.m_id, value);
	}

	template <typename T>
	typename T::T_Value& operator[](const T& token)
	{
		return m_blackboard->Value<typename T::T_Value>(token.m_id);
	}

private:
	BlackboardInst* m_blackboard;
};

/*
 * Blackboard Store.
 */
struct BlackboardStore
{
	constexpr BlackboardStore() : Blackboard()
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
		m_path()
	{

	}
	
	template <size_t N>
	BlackboardBuilder(BlackboardStore& store, const char (&path)[N], const Operation onOperation = nullptr) :
		m_store(store),
		m_onOperation(onOperation),
		m_path()
	{
		const size_t copy = std::clamp<size_t>(N, 0, MaxPath);
		memcpy(m_path, path, copy);
		m_path[MaxPath] = '\0';
	}

	BlackboardBuilder Root() const
	{
		return BlackboardBuilder(m_store, m_onOperation);
	}

	BlackboardBuilder SubBuilder(const char *name) const
	{
		char full[MaxPath + 1] = {};
		const int ret = strlen(m_path)
			? snprintf(full, sizeof(full), "%s.%s", m_path, name)
			: snprintf(full, sizeof(full), "%s", name);
		full[ret] = '\0';
		
		return BlackboardBuilder(m_store, full, m_onOperation);
	}

	BlackboardView View() const
	{
		return BlackboardView(&m_store.Blackboard);
	}

	template <typename T>
	bool Add(const char* name, T& token, typename T::T_Value init = typename T::T_Value()) const
	{
		static_assert(T::CanWrite);

		char full[MaxPath + 1] = {};
		const int ret = strlen(m_path)
			? snprintf(full, sizeof(full), "%s.%s", m_path, name)
			: snprintf(full, sizeof(full), "%s", name);
		if (static_cast<size_t>(ret) >= MaxPath)
			return false;
		full[ret] = '\0';

		if (m_onOperation)
			m_onOperation(Op::Add, full, GetTypeCode<typename T::T_Value>());

		const BlackboardID id = BLACKBOARD_ID(full);
		if (!m_store.Blackboard.Add<typename T::T_Value>(id, init))
			return false;

		token.m_id = id;
		return true;
	}

	bool Add(const char* name, OutputToken& token) const
	{
		switch (token.Type)
		{
			case TypeCode::u8:
				return Add<uint8_t>(name, token);
			case TypeCode::u16:
				return Add<uint16_t>(name, token);
			case TypeCode::u32:
				return Add<uint32_t>(name, token);
			case TypeCode::u64:
				return Add<uint64_t>(name, token);

			case TypeCode::i8:
				return Add<int8_t>(name, token);
			case TypeCode::i16:
				return Add<int16_t>(name, token);
			case TypeCode::i32:
				return Add<int32_t>(name, token);
			case TypeCode::i64:
				return Add<int64_t>(name, token);

			case TypeCode::f32:
				return Add<float>(name, token);
			case TypeCode::f64:
				return Add<double>(name, token);

			case TypeCode::str:
				return Add<str_t>(name, token);

			default:
				ASSERT(false);
				return false;
		}
	}

	template <typename T>
	bool Add(const char* name, OutputToken& token) const
	{
		Output<T> wrapped;
		if (!Add(name, wrapped))
			return false;

		token.m_id = wrapped.m_id;
		return true;
	}

	template <typename T>
	bool Get(const char* name, T& token) const
	{
		static_assert(T::CanRead);

		char full[MaxPath + 1] = {};
		const int ret = strlen(m_path)
			? snprintf(full, sizeof(full), "%s.%s", m_path, name)
			: snprintf(full, sizeof(full), "%s", name);
		if (static_cast<size_t>(ret) >= MaxPath)
			return false;
		full[ret] = '\0';

		if (m_onOperation)
			m_onOperation(Op::Get, full, GetTypeCode<typename T::T_Value>());

		const BlackboardID id = BLACKBOARD_ID(full);
		if (!m_store.Blackboard.Contains<typename T::T_Value>(id))
			return false;

		token.m_id = id;
		return true;
	}

	bool Get(const char* name, InputToken& token) const
	{
		switch (token.Type)
		{
			case TypeCode::u8:
				return Get<uint8_t>(name, token);
			case TypeCode::u16:
				return Get<uint16_t>(name, token);
			case TypeCode::u32:
				return Get<uint32_t>(name, token);
			case TypeCode::u64:
				return Get<uint64_t>(name, token);

			case TypeCode::i8:
				return Get<int8_t>(name, token);
			case TypeCode::i16:
				return Get<int16_t>(name, token);
			case TypeCode::i32:
				return Get<int32_t>(name, token);
			case TypeCode::i64:
				return Get<int64_t>(name, token);

			case TypeCode::f32:
				return Get<float>(name, token);
			case TypeCode::f64:
				return Get<double>(name, token);
				
			case TypeCode::str:
				return Get<str_t>(name, token);

			default:
				ASSERT(false);
				return false;
		}
	}

	template <typename T>
	bool Get(const char* name, InputToken& token) const
	{
		Input<T> wrapped;
		if (!Get(name, wrapped))
			return false;

		token.m_id = wrapped.m_id;
		return true;
	}

private:
	static constexpr size_t MaxPath = 64;

	BlackboardStore& m_store;
	Operation m_onOperation;
	char m_path[MaxPath + 1];
};
