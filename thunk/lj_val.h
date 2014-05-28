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

	enum StatementResultType {
		NORMAL_STATEMENT_RESULT = 1,
		RETURN_STATEMENT_RESULT,
		BREAK_STATEMENT_RESULT,
		CONTINUE_STATEMENT_RESULT,
		STATEMENT_RESULT_TYPE_COUNT_PLUS_1
	};

	class StatementResult {
	public:
		StatementResult(StatementResultType t, ValueBase *v) :
		value_(v), type_(t) {}
		~StatementResult() {}

		ValueBase *value_;
		StatementResultType type_;
	};

	typedef Value<boolean, BOOLEAN_VALUE>		BooleanValue;
	typedef Value<__int64, INT_VALUE>			IntValue;
	typedef Value<double, DOUBLE_VALUE>			DoubleValue;
	typedef Value<std::string, STRING_VALUE>	StringValue;
	typedef Value<NullType, NULL_VALUE>			NullValue;

}




#endif