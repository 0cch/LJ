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

	template<class T, ValueType N>
	class Value : public ValueBase {
	public:
		Value() {}
		~Value() {}

		ValueType GetType() const override {
			return N;
		}

		T value_;
	};

	template<ValueType N>
	class Value<NullType, N> :  public ValueBase {
	public:
		Value() {}
		~Value() {}

		ValueType GetType() const override {
			return N;
		}
	};

}


#endif