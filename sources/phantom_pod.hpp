#pragma once
namespace phantom {
    enum traits : unsigned int {
        none = 0,
        comparable = 1 << 0,
        orderable = 1 << 1,
        addable = 1 << 2,
        multipliable = 1 << 3,
        all = comparable | orderable | addable | multipliable
    };

    template <typename T, typename PHANTOM_TYPE, int traitsMask>
    struct TypedPOD
    {
    public:
        explicit constexpr TypedPOD(T value) : m_value(value) { }
        constexpr TypedPOD(const TypedPOD& rhs) : m_value(rhs.m_value) { }

        inline operator T () const { return m_value; }
        inline void operator = (const T& rhs) { m_value = rhs; }

        bool operator == (const TypedPOD& rhs) const
        {
            const bool isOperatorValid = static_cast<bool>(traitsMask & comparable);
            if (!isOperatorValid)
                static_assert(isOperatorValid, "");
            return m_value == rhs.m_value;
        }
        bool operator != (const TypedPOD& rhs) const
        {
            const bool isOperatorValid = static_cast<bool>(traitsMask & comparable);
            if (!isOperatorValid)
                static_assert(isOperatorValid, "");
            return m_value != rhs.m_value;
        }
        bool operator < (const TypedPOD& rhs) const
        {
            const bool isOperatorValid = static_cast<bool>(traitsMask & orderable);
            if (!isOperatorValid)
                static_assert(isOperatorValid, "");
            return m_value < rhs.m_value;
        }
        bool operator > (const TypedPOD& rhs) const
        {
            const bool isOperatorValid = static_cast<bool>(traitsMask & orderable);
            if (!isOperatorValid)
                static_assert(isOperatorValid, "");
            return m_value > rhs.m_value;
        }
        bool operator <= (const TypedPOD& rhs) const
        {
            const bool isOperatorValid = static_cast<bool>(traitsMask & (comparable | orderable) );
            if (!isOperatorValid)
                static_assert(isOperatorValid, "");
            return m_value <= rhs.m_value;
        }
        bool operator >= (const TypedPOD& rhs) const
        {
            const bool isOperatorValid = static_cast<bool>(traitsMask & (comparable | orderable) );
            if (!isOperatorValid)
                static_assert(isOperatorValid, "");
            return m_value >= rhs.m_value;
        }
        TypedPOD operator + (const TypedPOD& rhs) const
        {
            const bool isOperatorValid = static_cast<bool>(traitsMask & addable);
            if (!isOperatorValid)
                static_assert(isOperatorValid, "");
            return TypedPOD(m_value + rhs.m_value);
        }
        TypedPOD& operator += (const TypedPOD& rhs)
        {
            const bool isOperatorValid = static_cast<bool>(traitsMask & addable);
            if (!isOperatorValid)
                static_assert(isOperatorValid, "");
            m_value += rhs.m_value;
            return *this;
        }
        TypedPOD operator - (const TypedPOD& rhs) const
        {
            const bool isOperatorValid = static_cast<bool>(traitsMask & addable);
            if (!isOperatorValid)
                static_assert(isOperatorValid, "");
            return TypedPOD(m_value - rhs.m_value);
        }
        TypedPOD& operator -= (const TypedPOD& rhs)
        {
            const bool isOperatorValid = static_cast<bool>(traitsMask & addable);
            if (!isOperatorValid)
                static_assert(isOperatorValid, "");
            m_value -= rhs.m_value;
            return *this;
        }
        TypedPOD operator * (const TypedPOD& rhs) const
        {
            const bool isOperatorValid = static_cast<bool>(traitsMask & multipliable);
            if (!isOperatorValid)
                static_assert(isOperatorValid, "");
            return TypedPOD(m_value * rhs.m_value);
        }
        TypedPOD& operator *= (const TypedPOD& rhs)
        {
            const bool isOperatorValid = static_cast<bool>(traitsMask & multipliable);
            if (!isOperatorValid)
                static_assert(isOperatorValid, "");
            m_value *= rhs.m_value;
            return *this;
        }
        TypedPOD operator / (const TypedPOD& rhs) const
        {
            const bool isOperatorValid = static_cast<bool>(traitsMask & multipliable);
            if (!isOperatorValid)
                static_assert(isOperatorValid, "");
            return TypedPOD(m_value / rhs.m_value);
        }
        TypedPOD& operator /= (const TypedPOD& rhs)
        {
            const bool isOperatorValid = static_cast<bool>(traitsMask & multipliable);
            if (!isOperatorValid)
                static_assert(isOperatorValid, "");
            m_value /= rhs.m_value;
            return *this;
        }
    private:
        T m_value;
    };
}

#ifdef DECLARE_PHANTOM_TYPE
#pragma error "DECLARE_PHANTOM_TYPE is already defined"
#endif

#ifndef DISABLE_PHANTOM_TYPE
#define DECLARE_PHANTOM_TYPE(base, name, mask) struct phantom_##name {}; typedef phantom::TypedPOD<base, phantom_##name, mask> name;
#else
#define DECLARE_PHANTOM_TYPE(base, name, mask) typedef base name;
#endif
