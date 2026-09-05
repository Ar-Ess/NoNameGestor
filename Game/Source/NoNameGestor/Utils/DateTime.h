#pragma once

//TODO: Framework: Add this class to the framework
#include "Framework/Engine/Debug.h"

typedef long long int64_t;
typedef unsigned long long uint64_t;
struct _FILETIME;
template<short int, short int>
class TimeSpan;

//TODO: make a cpp file
class DateTime
{

	template<short int, short int>
	friend class TimeSpan;

public:

	struct From
	{
		From() = delete;
		From(const From&) = delete;
		From& operator=(const From&) = delete;
		From(From&&) = delete;
		From& operator=(From&&) = delete;
	
		static DateTime Now();

		static DateTime Year(uint64_t year);

		static DateTime Month(uint64_t year, uint64_t month);

		static DateTime Date(uint64_t year, uint64_t month, uint64_t day);

		static DateTime Date(uint64_t year, uint64_t month, uint64_t day, uint64_t hour, uint64_t minute = 0, uint64_t second = 0, uint64_t millisecond = 0);

		static DateTime WindowsFileTime(const _FILETIME& date);

	};

private:

    struct Defs
    {
		Defs() = delete;
		Defs(const Defs&) = delete;
		Defs& operator=(const Defs&) = delete;
		Defs(Defs&&) = delete;
		Defs& operator=(Defs&&) = delete;

		static constexpr uint64_t MsPerSecond = 1000;
		static constexpr uint64_t MsPerMinute = 60 * MsPerSecond;
		static constexpr uint64_t MsPerHour = 60 * MsPerMinute;
		static constexpr uint64_t MsPerDay = 24 * MsPerHour;

		static constexpr uint64_t SecondsPerMinute = 60;
		static constexpr uint64_t SecondsPerHour = 60 * SecondsPerMinute;
		static constexpr uint64_t SecondsPerDay = 24 * SecondsPerHour;

		static constexpr uint64_t MinutesPerHour = 60;
		static constexpr uint64_t MinutesPerDay = 24 * MinutesPerHour;

		static constexpr uint64_t HoursPerDay = 24;
		static constexpr uint64_t MonthsPerYear = 12;

        // Base Epoch 0001-01-01 to Windows Epoch 1601-01-01
		static constexpr uint64_t DaysFromBaseEpochToWindowsEpoch = 584388;
		
		static constexpr uint64_t Windows100NsPerMs = 10000;

		// Gregorian Cycle is 400 years, which mean exactly 146097 days
		static constexpr uint64_t DaysPerGregorianCycle = 146097;

    };

	struct CalendarSpan
	{
		CalendarSpan(int64_t years, short int daysInYear, int64_t months, short int daysInMonth, int64_t days, int64_t hours, int64_t minutes, int64_t seconds, int64_t milliseconds, bool negative) :
			days(days),
			hours(hours),
			minutes(minutes),
			seconds(seconds),
			milliseconds(milliseconds),
			negative(negative)
		{
			this->days += (years * daysInYear) + (months * daysInMonth);
		}

	private:

		CalendarSpan(int64_t days, int64_t hours, int64_t minutes, int64_t seconds, int64_t milliseconds, bool negative) :
			days(days),
			hours(hours),
			minutes(minutes),
			seconds(seconds),
			milliseconds(milliseconds),
			negative(negative)
		{
		}

	public:

		CalendarSpan Abs() const { return CalendarSpan{ days, hours, minutes, seconds, milliseconds, false }; }
		int64_t days;
		int64_t hours;
		int64_t minutes;
		int64_t seconds;
		int64_t milliseconds;
		bool negative;

	};

public:

	static const DateTime BaseEpoch;

	static const char* MonthName(uint64_t month);

	static const char* DayOfWeekName(int day);

	static const char* DayOfWeekName(uint64_t day, uint64_t month, uint64_t year);

	static int DayOfWeek(uint64_t day, uint64_t month, uint64_t year);
	
	static int DayOfYear(uint64_t day, uint64_t month, uint64_t year);

	static bool IsLeapYear(uint64_t year);

	static uint64_t DaysInYear(uint64_t year);

	static uint64_t DaysInMonth(uint64_t year, uint64_t month);

private:

	static const char* MonthNameInternal(uint64_t month) { return month == 0 || month > 12 ? nullptr : monthNames[month - 1]; }

public:

	DateTime() = default;
	DateTime(uint64_t msSinceBaseEpoch);
	DateTime(const DateTime& date) = default;
	DateTime& operator=(const DateTime& date) = default;
	DateTime(DateTime&& date) = default;
	DateTime& operator=(DateTime&& date) = default;

	DateTime StartOfYear() const;
	DateTime EndOfYear() const;

	DateTime StartOfMonth() const;
	DateTime EndOfMonth() const;

	DateTime StartOfDay() const;
	DateTime EndOfDay() const;

	uint64_t Ms() const;
	void Ms(uint64_t ms);

	uint64_t Seconds() const;
	void Seconds(uint64_t seconds);

	uint64_t Minutes() const;
	void Minutes(uint64_t minutes);

	uint64_t Hour(bool f24h = true) const;
	void Hour(uint64_t hours);
	void Hour(uint64_t hours, bool am);

	uint64_t Day() const;
	void Day(uint64_t day);
	const char* DayOfWeekName() const;
	int DayOfWeek() const;
	int DayOfYear() const;

	uint64_t Month() const;
	void Month(uint64_t month);
	const char* MonthName() const;
	uint64_t DaysInMonth() const;

	uint64_t Year() const;
	void Year(uint64_t year);
	uint64_t DaysInYear() const;
	bool IsLeapYear() const;

	String ToString(bool onlyDate = true) const
	{
		if (ms == 0) return onlyDate ? "0001-01-01" : "0001-01-01 00:00:00.000";

		String result = String::Empty;

		static const char* formats[7] = {
			 "%04d",
			"-%02d",
			"-%02d",
			" %02d",
			":%02d",
			":%02d",
			".%03d"
		};
		int64_t array[7] = {
			Year(),
			Month(),
			Day(),
			onlyDate ? 0 : Hour(),
			onlyDate ? 0 : Minutes(),
			onlyDate ? 0 : Seconds(),
			onlyDate ? 0 : Ms()
		};

		for (int i = 0; i < 7; ++i)
		{			
			if (onlyDate && i >= 3)
				break;

			result += String::Format(formats[i], array[i]);
		}

		return result;
	}
	/// <summary>
	/// yyyy: Four digit year.
	/// MM: Two digit month(01 - 12).
	/// dd: Two digit day(01 - 31).
	/// HH: Hour in 24 hour format(00 - 23).
	/// hh: Hour in 12 hour format(01 - 12).
	/// mm: Minutes(00 - 59).
	/// ss: Seconds(00 - 59).
	/// fff: Milliseconds(3 digits).
	/// </summary>
	String ToString(const char* format) const
	{
		return ToString(String(format));
	}
	/// <summary>
	/// yyyy: Four digit year.
	/// MM: Two digit month(01 - 12).
	/// dd: Two digit day(01 - 31).
	/// HH: Hour in 24 hour format(00 - 23).
	/// hh: Hour in 12 hour format(01 - 12).
	/// mm: Minutes(00 - 59).
	/// ss: Seconds(00 - 59).
	/// fff: Milliseconds(3 digits).
	/// </summary>
	String ToString(String format) const
	{
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

				f = String::Format("%04d", Year());
				break;
			}
			case 'M':
			{
				if (*(c + 1) != 'M')
					continue;
				f = String::Format("%02d", Month());
				break;
			}
			case 'd':
			{
				if (*(c + 1) != 'd')
					continue;
				f = String::Format("%02d", Day());
				break;
			}
			case 'H':
			{
				if (*(c + 1) != 'H')
					continue;
				f = String::Format("%02d", Hour());
				break;
			}
			case 'h':
			{
				if (*(c + 1) != 'h')
					continue;
				f = String::Format("%02d", Hour(false));
				break;
			}
			case 'm':
			{
				if (*(c + 1) != 'm')
					continue;
				f = String::Format("%02d", Minutes());
				break;
			}
			case 's':
			{
				if (*(c + 1) != 's')
					continue;
				f = String::Format("%02d", Seconds());
				break;
			}
			case 'f':
			{
				if (!String::StartsWith("fff", c))
					continue;
				f = String::Format("%03d", Ms());
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

public:

	// Conditional operators
	bool operator==(const DateTime& other) const { return ms == other.ms; }
	bool operator!=(const DateTime& other) const { return ms != other.ms; }
	bool operator< (const DateTime& other) const { return ms < other.ms; }
	bool operator<=(const DateTime& other) const { return ms <= other.ms; }
	bool operator> (const DateTime& other) const { return ms > other.ms; }
	bool operator>=(const DateTime& other) const { return ms >= other.ms; }

	// Modification operators
	template<short int DaysInMonth = 30, short int DaysInYear = 365>
	void operator+=(const TimeSpan<DaysInMonth, DaysInYear>& other)
	{
		if (other.IsNegative())
		{
			*this -= other.Abs();
			return;
		}

		int64_t oms = other.Abs().Ms();
		if (oms > 0 && ms > UINT64_MAX - oms)
		{
			ms = UINT64_MAX;
			return;
		}

		ms += oms;
	}
	template<short int DaysInMonth = 30, short int DaysInYear = 365>
	void operator-=(const TimeSpan<DaysInMonth, DaysInYear>& other)
	{
		if (other.IsNegative())
		{
			*this += other.Abs();
			return;
		}

		int64_t oms = other.Abs().Ms();
		if (oms > 0 && ms < oms)
		{
			ms = 0;
			return;
		}

		ms -= oms;
	}

	template<short int DaysInMonth = 30, short int DaysInYear = 365>
	DateTime operator+(const TimeSpan<DaysInMonth, DaysInYear>& other) const
	{
		DateTime result(*this);
		other.IsNegative() ?
			result -= other.Abs() :
			result += other;
		return result;
	}
	template<short int DaysInMonth = 30, short int DaysInYear = 365>
	DateTime operator-(const TimeSpan<DaysInMonth, DaysInYear>& other) const
	{
		DateTime result(*this);
		other.IsNegative() ?
			result += other.Abs() :
			result -= other;
		return result;
	}

	template<short int DaysInMonth = 30, short int DaysInYear = 365>
	TimeSpan<DaysInMonth, DaysInYear> operator-(const DateTime& other) const
	{
		if (ms == other.ms)
			return TimeSpan();

		if (other.ms == 0)
			return TimeSpan(ms > INT64_MAX ? INT64_MAX : ms);

		if (ms == 0)
			return TimeSpan(-other.ms < INT64_MIN ? INT64_MIN : -other.ms);

		// Later (ex. 2050) - Earlier (ex. 2010)
		if (ms > other.ms)
		{
			uint64_t ret = ms - other.ms;
			return TimeSpan(ret > INT64_MAX ? INT64_MAX : ret);
		}

		uint64_t magnitude = other.ms - ms;

		if (magnitude > static_cast<uint64_t>(INT64_MAX) + 1)
			return TimeSpan(INT64_MIN);
		
		if (magnitude == static_cast<uint64_t>(INT64_MAX) + 1)
			return TimeSpan(INT64_MIN);
		
		return TimeSpan(-static_cast<int64_t>(magnitude));
	}

	// Modification operators
	void operator+=(const CalendarSpan& other);
	void operator-=(const CalendarSpan& other);

	DateTime operator+(const CalendarSpan& other);
	DateTime operator-(const CalendarSpan& other);

private:

	uint64_t FindYear(uint64_t& remainderDays) const;
	uint64_t FindMonth(uint64_t& day, uint64_t& year) const;
	uint64_t To24h(uint64_t hour, bool am) const;
	uint64_t To12h(uint64_t hour) const;


private:

	static const char* monthNames[12];
	static const char* weekDayNames[7];
	uint64_t ms = 0; // ms since 0001-01-01 00:00:00.000

};