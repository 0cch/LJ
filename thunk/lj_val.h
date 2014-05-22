#ifndef __LJ_VALUE_H__
#define __LJ_VALUE_H__

namespace LJ {
	enum ValueType {
		BOOLEAN_VALUE = 1,
		INT_VALUE,
		DOUBLE_VALUE,
		STRING_VALUE,
		NULL_VALUE,
	};

	struct NullType;

	class ValueBase {
	public:
		ValueBase() {}
		virtual ~ValueBase() {}

		virtual ValueType GetType() const = 0;
	};

	template<class T, ValueBase N>
	class Value : ValueBase {
	public:
		Value() {}
		~Value() {}

		ValueType GetType() const override {
			return N;
		}

		T value_;
	};

	template<ValueBase N>
	class Value<NullType, N> : ValueBase {
	public:
		Value() {}
		~Value() {}

		ValueType GetType() const override {
			return N;
		}
	};

}


#endif