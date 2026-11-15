#pragma once

typedef long long int64_t;
typedef unsigned long long uint64_t;
struct _FILETIME;
template<short int, short int>
class TimeSpan;

#include "Framework/Data/String.h"

/// Represents a calendar date and time using the proleptic Gregorian calendar.
///
/// The internal representation stores milliseconds elapsed since
/// 0001-01-01 00:00:00.000. DateTime does not contain timezone information.
///
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
	
		/// Returns the current local date and time.
		static DateTime Now();

		/// Creates a DateTime representing the first moment of the specified year.
		static DateTime Year(uint64_t year);

		/// Creates a DateTime representing the first moment of the specified month.
		static DateTime Month(uint64_t year, uint64_t month);

		/// Creates a DateTime from a calendar date.
		static DateTime Date(uint64_t year, uint64_t month, uint64_t day);

		/// Creates a DateTime from a calendar date and time.
		static DateTime Date(uint64_t year, uint64_t month, uint64_t day, uint64_t hour, uint64_t minute = 0, uint64_t second = 0, uint64_t millisecond = 0);

		/// Creates a DateTime from a Windows FILETIME value.
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

	/// Returns whether the specified year is a leap year according to the
	/// Gregorian calendar rules.
	static bool IsLeapYear(uint64_t year);

	/// Returns the number of days in the specified month of the specified year.
	static uint64_t DaysInMonth(uint64_t year, uint64_t month);

	/// Returns the number of days in the specified year.
	static uint64_t DaysInYear(uint64_t year);

	/// Returns the name of the specified month.
	static const char* MonthName(uint64_t month);

	/// The base calendar epoch represented by 0001-01-01 00:00:00.000.
	static const DateTime BaseEpoch;

	/// Returns the name of the specified day of the week.
    /// The day value must range from 1 to 7.
	static const char* DayOfWeekName(int day);

	/// Returns the name of the day of the week for the specified calendar date.
	static const char* DayOfWeekName(uint64_t day, uint64_t month, uint64_t year);

	/// Returns the day of the week for the specified calendar date.
	/// The returned value ranges from 1 to 7.
	static int DayOfWeek(uint64_t day, uint64_t month, uint64_t year);
	
	/// Returns the day of the year for the specified calendar date.
	/// The returned value ranges from 1 to 365, or 366 in a leap year.
	static int DayOfYear(uint64_t day, uint64_t month, uint64_t year);

private:

	static const char* MonthNameInternal(uint64_t month);

public:

	DateTime() = default;
	DateTime(uint64_t msSinceBaseEpoch);
	DateTime(const DateTime& other) = default;
	DateTime& operator=(const DateTime& other) = default;
	DateTime(DateTime&& other) noexcept;
	DateTime& operator=(DateTime&& other) noexcept;

	/// Returns the first moment of the current year.
	DateTime StartOfYear() const;
	/// Returns the last representable millisecond of the current year.
	DateTime EndOfYear() const;

	/// Returns the first moment of the current month.
	DateTime StartOfMonth() const;
	/// Returns the last representable millisecond of the current month.
	DateTime EndOfMonth() const;

	/// Returns the first moment of the current day.
	DateTime StartOfDay() const;
	/// Returns the last representable millisecond of the current day.
	DateTime EndOfDay() const;

	/// Returns the millisecond component of the time.
	uint64_t Ms() const;
	/// Sets the millisecond component of the time.
	void Ms(uint64_t ms);

	/// Returns the second component of the time.
	uint64_t Seconds() const;
	/// Sets the second component of the time.
	void Seconds(uint64_t seconds);

	/// Returns the minute component of the time.
	uint64_t Minutes() const;
	/// Sets the minute component of the time.
	void Minutes(uint64_t minutes);

	/// Returns the hour component of the time.
	/// When f24h is true, returns the hour using the 24-hour clock.
	/// When f24h is false, returns the hour using the 12-hour clock.
	uint64_t Hour(bool f24h = true) const;
	/// Sets the hour using the 24-hour clock.
	void Hour(uint64_t hours);
	/// Sets the hour using the 12-hour clock and the specified AM/PM period.
	void Hour(uint64_t hours, bool am);

	/// Returns the day of the month.
	uint64_t Day() const;
	/// Sets the day of the month.
	void Day(uint64_t day);
	/// Returns the name of the current day of the week.
	const char* DayOfWeekName() const;
	/// Returns the day of the week as a numeric value.
	int DayOfWeek() const;
	/// Returns the day of the year.
	int DayOfYear() const;

	/// Returns the month of the year.
	uint64_t Month() const;
	/// Sets the month of the year.
	void Month(uint64_t month);
	const char* MonthName() const;
	/// Returns the number of days in the current month.
	uint64_t DaysInMonth() const;

	/// Returns the year.
	uint64_t Year() const;
	/// Sets the year.
	void Year(uint64_t year);
	/// Returns the number of days in the current year.
	uint64_t DaysInYear() const;
	/// Returns whether the current year is a leap year.
	bool IsLeapYear() const;

	void Date(uint64_t& year, uint64_t& month, uint64_t& day) const;
	void Date(int& year, int& month, int& day) const;

	/// Returns the formatted date and time as a string.
	/// When onlyDate is true, returns the date using the default date format.
	/// Otherwise, returns the date and time including milliseconds.
	String ToString(bool onlyDate = true) const;
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
	String ToString(const char* format) const;
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
	String ToString(String format) const;
	//TODO: This has to input a string??

	/// Returns the number of complete calendar years between this date and another date.
	/// Month, day and time components are ignored.
	/// The result is positive when other is in a later year and negative when it is earlier.
	int64_t YearsUntil(const DateTime& other) const;
	/// Returns the number of complete calendar months between this date and another date.
	/// Day and time components are ignored.
	/// The result is based only on the year and month components.
	int64_t MonthsUntil(const DateTime& other) const;
	/// Returns the number of complete elapsed days between this date and another date.
	/// Partial days are discarded.
	int64_t DaysUntil(const DateTime& other) const;
	/// Returns the number of complete elapsed hours between this date and another date.
	/// Partial hours are discarded.
	int64_t HoursUntil(const DateTime& other) const;
	/// Returns the number of complete elapsed minutes between this date and another date.
	/// Partial minutes are discarded.
	int64_t MinutesUntil(const DateTime& other) const;
	/// Returns the number of complete elapsed seconds between this date and another date.
	/// Partial seconds are discarded.
	int64_t SecondsUntil(const DateTime& other) const;
	/// Returns the exact elapsed milliseconds between this date and another date.
	int64_t MsUntil(const DateTime& other) const;

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

	DateTime operator+(const CalendarSpan& other) const;
	DateTime operator-(const CalendarSpan& other) const;

private:

	uint64_t FindYear(uint64_t& remainderDays) const;
	uint64_t FindMonth(uint64_t& remainderDays, uint64_t& year) const;
	uint64_t To24h(uint64_t hour, bool am) const;
	uint64_t To12h(uint64_t hour) const;

private:

	static const char* monthNames[12];
	static const char* weekDayNames[7];
	uint64_t ms = 0; // ms since 0001-01-01 00:00:00.000

};