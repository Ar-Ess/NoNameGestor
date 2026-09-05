#pragma once

#include "Framework/Utils/Maths.h"
#include "Framework/Data/String.h"
#include "NoNameGestor/Utils/DateTime.h"
#include <stdint.h>
#include <cmath>
#include <concepts>
#include <type_traits>


template<typename T>
concept Numeric =
std::is_same_v<T, int> ||
std::is_same_v<T, int64_t> ||
std::is_same_v<T, float> ||
std::is_same_v<T, double>;

//TODO: Framework: Add this class to the framework
template<short int DaysPerMonth = 30, short int DaysPerYear = 365>
class TimeSpan
{
public:

	struct Convert
	{
		Convert() = delete;
		Convert(const Convert&) = delete;
		Convert& operator=(const Convert&) = delete;
		Convert(Convert&&) = delete;
		Convert& operator=(Convert&&) = delete;

		struct FromMs
		{
			FromMs() = delete;
			FromMs(const FromMs&) = delete;
			FromMs& operator=(const FromMs&) = delete;
			FromMs(FromMs&&) = delete;
			FromMs& operator=(FromMs&&) = delete;

			template<Numeric Output = int64_t, Numeric Input>
			static Output ToSeconds(Input ms) { return ConvertUp<Output>(ms, Defs::MsPerSecond); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToMinutes(Input ms) { return ConvertUp<Output>(ms, Defs::MsPerMinute); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToHours(Input ms) { return ConvertUp<Output>(ms, Defs::MsPerHour); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToDays(Input ms) { return ConvertUp<Output>(ms, Defs::MsPerDay); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToMonths(Input ms) { return ConvertUp<Output>(ms, Defs::MsPerMonth); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToYears(Input ms) { return ConvertUp<Output>(ms, Defs::MsPerYear); }
		};

		struct FromSeconds
		{
			FromSeconds() = delete;
			FromSeconds(const FromSeconds&) = delete;
			FromSeconds& operator=(const FromSeconds&) = delete;
			FromSeconds(FromSeconds&&) = delete;
			FromSeconds& operator=(FromSeconds&&) = delete;

			template<Numeric Output = int64_t, Numeric Input>
			static Output ToMs(Input seconds) { return ConvertDown<Output>(seconds, Defs::MsPerSecond); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToMinutes(Input seconds) { return ConvertUp<Output>(seconds, Defs::SecondsPerMinute); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToHours(Input seconds) { return ConvertUp<Output>(seconds, Defs::SecondsPerHour); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToDays(Input seconds) { return ConvertUp<Output>(seconds, Defs::SecondsPerDay); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToMonths(Input seconds) { return ConvertUp<Output>(seconds, Defs::SecondsPerMonth); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToYears(Input seconds) { return ConvertUp<Output>(seconds, Defs::SecondsPerYear); }
		};

		struct FromMinutes
		{
			FromMinutes() = delete;
			FromMinutes(const FromMinutes&) = delete;
			FromMinutes& operator=(const FromMinutes&) = delete;
			FromMinutes(FromMinutes&&) = delete;
			FromMinutes& operator=(FromMinutes&&) = delete;

			template<Numeric Output = int64_t, Numeric Input>
			static Output ToMs(Input minutes) { return ConvertDown<Output>(minutes, Defs::MsPerMinute); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToSeconds(Input minutes) { return ConvertDown<Output>(minutes, Defs::SecondsPerMinute); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToHours(Input minutes) { return ConvertUp<Output>(minutes, Defs::MinutesPerHour); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToDays(Input minutes) { return ConvertUp<Output>(minutes, Defs::MinutesPerDay); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToMonths(Input minutes) { return ConvertUp<Output>(minutes, Defs::MinutesPerMonth); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToYears(Input minutes) { return ConvertUp<Output>(minutes, Defs::MinutesPerYear); }
		};

		struct FromHours
		{
			FromHours() = delete;
			FromHours(const FromHours&) = delete;
			FromHours& operator=(const FromHours&) = delete;
			FromHours(FromHours&&) = delete;
			FromHours& operator=(FromHours&&) = delete;

			template<Numeric Output = int64_t, Numeric Input>
			static Output ToMs(Input hours) { return ConvertDown<Output>(hours, Defs::MsPerHour); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToSeconds(Input hours) { return ConvertDown<Output>(hours, Defs::SecondsPerHour); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToMinutes(Input hours) { return ConvertDown<Output>(hours, Defs::MinutesPerHour); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToDays(Input hours) { return ConvertUp<Output>(hours, Defs::HoursPerDay); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToMonths(Input hours) { return ConvertUp<Output>(hours, Defs::HoursPerMonth); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToYears(Input hours) { return ConvertUp<Output>(hours, Defs::HoursPerYear); }
		};

		struct FromDays
		{
			FromDays() = delete;
			FromDays(const FromDays&) = delete;
			FromDays& operator=(const FromDays&) = delete;
			FromDays(FromDays&&) = delete;
			FromDays& operator=(FromDays&&) = delete;

			template<Numeric Output = int64_t, Numeric Input>
			static Output ToMs(Input days) { return ConvertDown<Output>(days, Defs::MsPerDay); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToSeconds(Input days) { return ConvertDown<Output>(days, Defs::SecondsPerDay); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToMinutes(Input days) { return ConvertDown<Output>(days, Defs::MinutesPerDay); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToHours(Input days) { return ConvertDown<Output>(days, Defs::HoursPerDay); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToMonths(Input days) { return ConvertUp<Output>(days, Defs::DaysPerMonth); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToYears(Input days) { return ConvertUp<Output>(days, Defs::DaysPerYear); }
		};

		struct FromMonths
		{
			FromMonths() = delete;
			FromMonths(const FromMonths&) = delete;
			FromMonths& operator=(const FromMonths&) = delete;
			FromMonths(FromMonths&&) = delete;
			FromMonths& operator=(FromMonths&&) = delete;

			template<Numeric Output = int64_t, Numeric Input>
			static Output ToMs(Input months) { return ConvertDown<Output>(months, Defs::MsPerMonth); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToSeconds(Input months) { return ConvertDown<Output>(months, Defs::SecondsPerMonth); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToMinutes(Input months) { return ConvertDown<Output>(months, Defs::MinutesPerMonth); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToHours(Input months) { return ConvertDown<Output>(months, Defs::HoursPerMonth); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToDays(Input months) { return ConvertDown<Output>(months, Defs::DaysPerMonth); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToYears(Input months) { return ConvertUp<Output>(FromMonths::ToDays<Output>(months), Defs::DaysPerYear); }
		};

		struct FromYears
		{
			FromYears() = delete;
			FromYears(const FromYears&) = delete;
			FromYears& operator=(const FromYears&) = delete;
			FromYears(FromYears&&) = delete;
			FromYears& operator=(FromYears&&) = delete;

			template<Numeric Output = int64_t, Numeric Input>
			static Output ToMs(Input years) { return ConvertDown<Output>(years, Defs::MsPerYear); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToSeconds(Input years) { return ConvertDown<Output>(years, Defs::SecondsPerYear); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToMinutes(Input years) { return ConvertDown<Output>(years, Defs::MinutesPerYear); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToHours(Input years) { return ConvertDown<Output>(years, Defs::HoursPerYear); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToDays(Input years) { return ConvertDown<Output>(years, Defs::DaysPerYear); }
			template<Numeric Output = int64_t, Numeric Input>
			static Output ToMonths(Input years) { return ConvertUp<Output>(FromYears::ToDays<Output>(years), Defs::DaysPerMonth); }
		};

	};

	struct From
	{
		template<Numeric T>
		static TimeSpan Ms(T msecs) { return TimeSpan(ConvertDown<int64_t>(msecs)); }

		template<Numeric T>
		static TimeSpan Seconds(T seconds) { return TimeSpan(ConvertDown<int64_t>(seconds, Defs::MsPerSecond)); }

		template<Numeric T>
		static TimeSpan Minutes(T minutes) { return TimeSpan(ConvertDown<int64_t>(minutes, Defs::MsPerMinute)); }

		template<Numeric T>
		static TimeSpan Hours(T hours) { return TimeSpan(ConvertDown<int64_t>(hours, Defs::MsPerHour)); }

		template<Numeric T>
		static TimeSpan Days(T days) { return TimeSpan(ConvertDown<int64_t>(days, Defs::MsPerDay)); }

		template<Numeric T>
		static TimeSpan Months(T months) { return TimeSpan(ConvertDown<int64_t>(months, Defs::MsPerMonth)); }

		template<Numeric T>
		static TimeSpan Years(T years) { return TimeSpan(ConvertDown<int64_t>(years, Defs::MsPerYear)); }

		template<Numeric T1, Numeric T2, Numeric T3, Numeric T4, Numeric T5, Numeric T6, Numeric T7>
		static TimeSpan Time(T1 msecs = 0, T2 seconds = 0, T3 minutes = 0, T4 hours = 0, T5 days = 0, T6 months = 0, T7 years = 0)
		{
			return TimeSpan(
				(msecs   != 0 ? ConvertDown  <int64_t, T1>(msecs  ) : 0) +
				(seconds != 0 ? Convert::FromSeconds::ToMs(seconds) : 0) +
				(minutes != 0 ? Convert::FromMinutes::ToMs(minutes) : 0) +
				(hours   != 0 ? Convert::FromHours  ::ToMs(hours  ) : 0) +
				(days    != 0 ? Convert::FromDays   ::ToMs(days   ) : 0) +
				(months  != 0 ? Convert::FromMonths ::ToMs(months ) : 0) +
				(years   != 0 ? Convert::FromYears  ::ToMs(years  ) : 0)
			);
		}
	};

private:

	struct Defs
	{
		Defs() = delete;
		Defs(const Defs&) = delete;
		Defs& operator=(const Defs&) = delete;
		Defs(Defs&&) = delete;
		Defs& operator=(Defs&&) = delete;

		static constexpr int64_t MsPerSecond = 1000;
		static constexpr int64_t MsPerMinute = 60 * MsPerSecond;
		static constexpr int64_t MsPerHour = 60 * MsPerMinute;
		static constexpr int64_t MsPerDay = 24 * MsPerHour;
		static constexpr int64_t MsPerMonth = MsPerDay * static_cast<int64_t>(DaysPerMonth);
		static constexpr int64_t MsPerYear = MsPerDay * static_cast<int64_t>(DaysPerYear);

		static constexpr int64_t SecondsPerMinute = 60;
		static constexpr int64_t SecondsPerHour = 60 * SecondsPerMinute;
		static constexpr int64_t SecondsPerDay = 24 * SecondsPerHour;
		static constexpr int64_t SecondsPerMonth = SecondsPerDay * static_cast<int64_t>(DaysPerMonth);
		static constexpr int64_t SecondsPerYear = SecondsPerDay * static_cast<int64_t>(DaysPerYear);

		static constexpr int64_t MinutesPerHour = 60;
		static constexpr int64_t MinutesPerDay = 24 * MinutesPerHour;
		static constexpr int64_t MinutesPerMonth = MinutesPerDay * static_cast<int64_t>(DaysPerMonth);
		static constexpr int64_t MinutesPerYear = MinutesPerDay * static_cast<int64_t>(DaysPerYear);

		static constexpr int64_t HoursPerDay = 24;
		static constexpr int64_t HoursPerMonth = HoursPerDay * static_cast<int64_t>(DaysPerMonth);
		static constexpr int64_t HoursPerYear = HoursPerDay * static_cast<int64_t>(DaysPerYear);

		static constexpr int64_t DaysPerMonth = static_cast<int64_t>(DaysPerMonth);
		static constexpr int64_t DaysPerYear = static_cast<int64_t>(DaysPerYear);

	};

public:

	static const TimeSpan Zero;

public:

	static_assert(DaysPerMonth > 0, "TimeSpan Error: a month can't imply negative or 0 days.");
	static_assert(DaysPerYear > DaysPerMonth, "TimeSpan Error: a year can't imply less days than a month.");

	TimeSpan() = default;
	explicit TimeSpan(int64_t ms) : ms(ms) {}
	TimeSpan(const TimeSpan& span) = default;
	TimeSpan& operator=(const TimeSpan& span) = default;
	TimeSpan(TimeSpan&& span) = default;
	TimeSpan& operator=(TimeSpan&& span) = default;

	const DateTime::CalendarSpan AsDate() const
	{
		bool negative = IsNegative();
		TimeSpan span = Abs();
		return DateTime::CalendarSpan
		{
			span.YearPart(),
			DaysPerYear,
			span.MonthPart(),
			DaysPerMonth,
			span.DayPart(),
			span.HourPart(),
			span.MinutePart(),
			span.SecondPart(),
			span.MsPart(),
			negative
		};
	}

	template<Numeric T = int64_t>
	T Ms() const { return ConvertUp<T>(ms); }
	template<Numeric T>
	void Ms(T msecs) { ms = ConvertDown<T>(msecs); }

	// TODO: Framework: Floor should accept a return template for returning bigger ints
	// TODO: Framework: Maths must be able to accept a templating to decide if float or double or int
	template<Numeric T = int64_t>
	T Seconds() const { return ConvertUp<T>(ms, Defs::MsPerSecond); }
	template<Numeric T>
	void Seconds(T seconds) { ms = ConvertDown<T>(seconds, Defs::MsPerSecond); }

	template<Numeric T = int64_t>
	T Minutes() const { return ConvertUp<T>(ms, Defs::MsPerMinute); }
	template<Numeric T>
	void Minutes(T minutes) { ms = ConvertDown<T>(minutes, Defs::MsPerMinute); }

	template<Numeric T = int64_t>
	T Hours() const { return ConvertUp<T>(ms, Defs::MsPerHour); }
	template<Numeric T>
	void Hours(T hours) { ms = ConvertDown<T>(hours, Defs::MsPerHour); }

	template<Numeric T = int64_t>
	T Days() const { return ConvertUp<T>(ms, Defs::MsPerDay); }
	template<Numeric T>
	void Days(T days) { ms = ConvertDown<T>(days, Defs::MsPerDay); }

	template<Numeric T = int64_t>
	T Months() const { return ConvertUp<T>(ms, Defs::MsPerMonth); }
	template<Numeric T>
	void Months(T months) { ms = ConvertDown<T>(months, Defs::MsPerMonth); }

	template<Numeric T = int64_t>
	T Years() const { return ConvertUp<T>(ms, Defs::MsPerYear); }
	template<Numeric T>
	void Years(T years) { ms = ConvertDown<T>(years, Defs::MsPerYear); }

	int64_t MsPart() const { return ms % Defs::MsPerSecond; }
	int64_t SecondPart() const { return (ms / Defs::MsPerSecond) % Defs::SecondsPerMinute; }
	int64_t MinutePart() const { return (ms / Defs::MsPerMinute) % Defs::MinutesPerHour; }
	int64_t HourPart() const { return (ms / Defs::MsPerHour) % Defs::HoursPerDay; }
	int64_t DayPart() const { return ((ms / Defs::MsPerDay) % DaysPerYear) % DaysPerMonth; }
	int64_t MonthPart() const { return ((ms / Defs::MsPerDay) % DaysPerYear) / DaysPerMonth; }
	int64_t YearPart() const { return (ms / Defs::MsPerDay) / DaysPerYear; }

	String ToString(bool compact = false) const
	{
		if (ms == 0) return compact ? "0.000" : "0y 0mo 0d 00:00:00.000";

		bool negative = ms < 0;
		String result = negative ? '-' : String::Empty;
		int64_t base = negative ? (ms == INT64_MIN ? INT64_MAX : -ms) : ms;

		static const char* formats[7] = {
			"%dy ",
			"%dmo ",
			"%dd ",
			"%02d:",
			"%02d:",
			"%02d.",
			"%03d"
		};
		int64_t array[7] = {
			(base / Defs::MsPerDay) / DaysPerYear,
			((base / Defs::MsPerDay) % DaysPerYear) / DaysPerMonth,
			((base / Defs::MsPerDay) % DaysPerYear) % DaysPerMonth,
			(base / Defs::MsPerHour) % Defs::HoursPerDay,
			(base / Defs::MsPerMinute) % Defs::MinutesPerHour,
			(base / Defs::MsPerSecond) % Defs::SecondsPerMinute,
			(base % Defs::MsPerSecond)
		};

		bool skipping = true;
		for (int i = 0; i < 7; ++i)
		{
			int64_t v = array[i];

			if (skipping && compact && v == 0 && i < 5)
				continue;

			skipping = false;
			result += String::Format(formats[i], v);
		}

		return result;
	}

	/// <summary>
	/// yy: year amount.
	/// MM: month amount.
	/// dd: day amount.
	/// hh: hour amount(00 - 23).
	/// mm: minute amount(00 - 59).
	/// ss: seconds amount(00 - 59).
	/// ff: milliseconds amount(3 digits).
	/// </summary>
	String ToString(String format) const
	{
		//TODO: TimeSpan formatting
		for (int i = 0; i < format.Length(); ++i)
		{
			const char* c = format.Str() + i;

			String f = String::Empty;
			switch (*c)
			{
			case 'y':
			{
				if (!String::StartsWith("yyyy", c))
					continue;

				f = String::Format("%04d", YearPart());
				break;
			}
			case 'M':
			{
				if (*(c + 1) != 'M')
					continue;
				f = String::Format("%02d", MonthPart());
				break;
			}
			case 'd':
			{
				if (*(c + 1) != 'd')
					continue;
				f = String::Format("%02d", DayPart());
				break;
			}
			case 'H':
			{
				if (*(c + 1) != 'H')
					continue;
				f = String::Format("%02d", HourPart());
				break;
			}
			case 'h':
			{
				if (*(c + 1) != 'h')
					continue;
				f = String::Format("%02d", HourPart());
				break;
			}
			case 'm':
			{
				if (*(c + 1) != 'm')
					continue;
				f = String::Format("%02d", MinutePart());
				break;
			}
			case 's':
			{
				if (*(c + 1) != 's')
					continue;
				f = String::Format("%02d", SecondPart());
				break;
			}
			case 'f':
			{
				if (!String::StartsWith("fff", c))
					continue;
				f = String::Format("%03d", MsPart());
				break;
			}
			default:
				continue;
			}

			for (int j = 0; j < f.Length(); ++j)
				format[i + j] = f[j];
			i += f.Length() - 1;
		}

		return format;
	}

	template<Numeric T1, Numeric T2, Numeric T3, Numeric T4, Numeric T5, Numeric T6, Numeric T7>
	void Set(T1 msecs = 0, T2 seconds = 0, T3 minutes = 0, T4 hours = 0, T5 days = 0, T6 months = 0, T7 years = 0)
	{
		ms = (msecs != 0 ? ConvertDown    <int64_t, T1>(msecs  ) : 0) +
			 (seconds != 0 ? Convert::FromSeconds::ToMs(seconds) : 0) +
			 (minutes != 0 ? Convert::FromMinutes::ToMs(minutes) : 0) +
			 (hours   != 0 ? Convert::FromHours  ::ToMs(hours  ) : 0) +
			 (days    != 0 ? Convert::FromDays   ::ToMs(days   ) : 0) +
			 (months  != 0 ? Convert::FromMonths ::ToMs(months ) : 0) +
			 (years   != 0 ? Convert::FromYears  ::ToMs(years  ) : 0);
	}

	bool IsZero() const { return ms == 0; }
	bool IsPositive() const { return ms > 0; }
	bool IsNegative() const { return ms < 0; }

	TimeSpan Abs() const { return IsNegative() ? TimeSpan(ms == INT64_MIN ? INT64_MAX : -ms) : *this; }

public:

	// Conditional operators
	bool operator==(const TimeSpan& other) const { return ms == other.ms; }
	bool operator!=(const TimeSpan& other) const { return ms != other.ms; }
	bool operator< (const TimeSpan& other) const { return ms <  other.ms; }
	bool operator<=(const TimeSpan& other) const { return ms <= other.ms; }
	bool operator> (const TimeSpan& other) const { return ms >  other.ms; }
	bool operator>=(const TimeSpan& other) const { return ms >= other.ms; }

	// Modification operators
	void operator+=(const TimeSpan& other) 
	{
		if (other.ms > 0 && ms > INT64_MAX - other.ms)
		{
			ms = INT64_MAX;
			return;
		}

		if (other.ms < 0 && ms < INT64_MIN - other.ms)
		{
			ms = INT64_MIN;
			return;
		}

		ms += other.ms;
	}
	void operator-=(const TimeSpan& other)
	{
		if (other.ms < 0 && ms > INT64_MAX + other.ms)
		{
			ms = INT64_MAX;
			return;
		}

		if (other.ms > 0 && ms < INT64_MIN + other.ms)
		{
			ms = INT64_MIN;
			return;
		}

		ms -= other.ms;
	}
	void operator*=(float v)
	{
		if (std::isnan(v))
		{
			ms = 0;
			return;
		}

		if (std::isinf(v))
		{
			if (ms == 0)
			{
				ms = 0;
				return;
			}

			ms = ((ms > 0) == (v > 0)) ? INT64_MAX : INT64_MIN;

			return;
		}

		const long double result = static_cast<long double>(ms) * static_cast<long double>(v);
		const long double positiveLimit = 9223372036854775808.0L; // 2^63
		const long double negativeLimit = -9223372036854775808.0L; // -2^63

		if (result >= positiveLimit)
		{
			ms = INT64_MAX;
			return;
		}

		if (result <= negativeLimit)
		{
			ms = INT64_MIN;
			return;
		}

		ms = static_cast<int64_t>(std::round(result));
	}
	void operator/=(float v)
	{
		const long double result = static_cast<long double>(ms) / static_cast<long double>(v);
		const long double positiveLimit = 9223372036854775808.0L;
		const long double negativeLimit = -9223372036854775808.0L;

		if (std::isnan(result))
		{
			ms = 0;
			return;
		}

		if (result >= positiveLimit)
		{
			ms = INT64_MAX;
			return;
		}

		if (result <= negativeLimit)
		{
			ms = INT64_MIN;
			return;
		}

		ms = static_cast<int64_t>(std::round(result));
	}

	TimeSpan operator+(const TimeSpan& other) const
	{
		TimeSpan result(*this);
		result += other;
		return result;
	}
	TimeSpan operator-(const TimeSpan& other) const
	{
		TimeSpan result(*this);
		result -= other;
		return result;
	}
	TimeSpan operator*(float v) const
	{
		TimeSpan result(*this);
		result *= v;
		return result;
	}
	TimeSpan operator/(float v) const
	{
		TimeSpan result(*this);
		result /= v;
		return result;
	}
	
	TimeSpan operator-() const { return (ms == INT64_MIN) ? TimeSpan(INT64_MAX) : TimeSpan(-ms); }

	//TimeSpan operator*(float v, const TimeSpan& span)
	//{
	//	return span * v;
	//}

private:

	template<Numeric Output, Numeric Input>
	static Output ConvertUp(Input value, int64_t factor = 1)
	{
		if constexpr (std::is_same_v<Output, int64_t>)
		{
			if constexpr (std::is_same_v<Input, int64_t> || std::is_same_v<Input, int>)
			{
				return static_cast<int64_t>(value) / factor;
			}
			else
			{
				if (std::isnan(value))
					return 0;

				if (std::isinf(value))
					return value > 0 ? INT64_MAX : INT64_MIN;

				const long double converted = static_cast<long double>(value) / static_cast<long double>(factor);

				// 2^63 is the first value outside the positive
				// int64_t range.
				const long double positiveLimit = static_cast<long double>(INT64_MAX) + 1.0L;

				// INT64_MIN is exactly -2^63.
				const long double negativeLimit = static_cast<long double>(INT64_MIN);

				if (converted >= positiveLimit)
					return INT64_MAX;

				if (converted <= negativeLimit)
					return INT64_MIN;

				return static_cast<int64_t>(converted);
			}
		}
		else if constexpr (std::is_same_v<Output, int>)
		{
			if constexpr (std::is_same_v<Input, int64_t> || std::is_same_v<Input, int>)
			{
				const int64_t converted = static_cast<int64_t>(value) / factor;

				if (converted > INT_MAX)
					return INT_MAX;

				if (converted < INT_MIN)
					return INT_MIN;

				return static_cast<int>(converted);
			}
			else
			{
				if (std::isnan(value))
					return 0;

				if (std::isinf(value))
					return value > 0 ? INT_MAX : INT_MIN;

				const long double converted = static_cast<long double>(value) / static_cast<long double>(factor);

				// 2^31 is the first value outside the positive
				// int range.
				const long double positiveLimit = static_cast<long double>(INT_MAX) + 1.0L;

				// INT_MIN is exactly -2^31.
				const long double negativeLimit = static_cast<long double>(INT_MIN);

				if (converted >= positiveLimit)
					return INT_MAX;

				if (converted <= negativeLimit)
					return INT_MIN;

				return static_cast<int>(converted);
			}
		}
		else
		{
			return static_cast<Output>(value) / static_cast<Output>(factor);
		}
	}

	template<Numeric Output, Numeric Input>
	static Output ConvertDown(Input value, int64_t factor = 1)
	{
		if constexpr (std::is_same_v<Output, int64_t>)
		{
			if constexpr (std::is_same_v<Input, int64_t> ||
				std::is_same_v<Input, int>)
			{
				if (value > INT64_MAX / factor)
					return INT64_MAX;

				if (value < INT64_MIN / factor)
					return INT64_MIN;

				return static_cast<int64_t>(value) * factor;
			}
			else
			{
				if (std::isnan(value))
					return 0;

				if (std::isinf(value))
					return value > 0 ? INT64_MAX : INT64_MIN;

				const long double converted =
					static_cast<long double>(value) *
					static_cast<long double>(factor);

				const long double positiveLimit =
					static_cast<long double>(INT64_MAX) + 1.0L;

				const long double negativeLimit =
					static_cast<long double>(INT64_MIN);

				if (converted >= positiveLimit)
					return INT64_MAX;

				if (converted <= negativeLimit)
					return INT64_MIN;

				return static_cast<int64_t>(std::round(converted));
			}
		}
		else if constexpr (std::is_same_v<Output, int>)
		{
			if constexpr (std::is_same_v<Input, int64_t> ||
				std::is_same_v<Input, int>)
			{
				if (value > INT_MAX / factor)
					return INT_MAX;

				if (value < INT_MIN / factor)
					return INT_MIN;

				return static_cast<int>(value * factor);
			}
			else
			{
				if (std::isnan(value))
					return 0;

				if (std::isinf(value))
					return value > 0 ? INT_MAX : INT_MIN;

				const long double converted =
					static_cast<long double>(value) *
					static_cast<long double>(factor);

				const long double positiveLimit =
					static_cast<long double>(INT_MAX) + 1.0L;

				const long double negativeLimit =
					static_cast<long double>(INT_MIN);

				if (converted >= positiveLimit)
					return INT_MAX;

				if (converted <= negativeLimit)
					return INT_MIN;

				return static_cast<int>(std::round(converted));
			}
		}
		else
		{
			return static_cast<Output>(value) *
				static_cast<Output>(factor);
		}
	}

private:

	int64_t ms = 0;

};

template <short int DaysPerMonth, short int DaysPerYear>
const TimeSpan<DaysPerMonth, DaysPerYear> TimeSpan<DaysPerMonth, DaysPerYear>::Zero = {};