#include "DateTime.h"

#include "NoNameGestor/Utils/TimeSpan.h"

#include <windows.h>
#include <ctime>
#include <bit>

const DateTime DateTime::BaseEpoch = DateTime();

const char* DateTime::monthNames[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
const char* DateTime::weekDayNames[7] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };

DateTime DateTime::From::Now()
{
	time_t now = time(0);
	tm* ltm = localtime(&now);

	int year = ltm->tm_year;
	int month = ltm->tm_mon + 1;
	int day = ltm->tm_mday;
	int hour = ltm->tm_hour;
	int min = ltm->tm_min;
	int sec = ltm->tm_sec == 60 ? 0 : ltm->tm_sec;

	return Date(year, month, day, hour, min, sec, 0);
}

DateTime DateTime::From::Year(uint64_t year)
{
	Debug::Assert(year > 0, "DateTime Date Error: year must be a value greater than 0.");
	return Date(year, 1, 1);
}

DateTime DateTime::From::Month(uint64_t year, uint64_t month)
{
	Debug::Assert(year > 0, "DateTime Date Error: year must be a value greater than 0.");
	Debug::Assert(month > 0 && month < 13, "DateTime Date Error: month must be a value between 1 and 12 inclusive.");
	return Date(year, month, 1);
}

DateTime DateTime::From::Date(uint64_t year, uint64_t month, uint64_t day)
{
	Debug::Assert(year > 0, "DateTime Date Error: year must be a value greater than 0.");
	Debug::Assert(month > 0 && month < 13, "DateTime Date Error: month must be a value between 1 and 12 inclusive.");

	uint64_t maxDay = DaysInMonth(year, month);
	Debug::Assert<const char*, int>(day > 0 && day <= maxDay, "DateTime Date Error: day in %s must be a value between 1 and %d inclusive.", MonthNameInternal(month), maxDay);

	uint64_t days = (year - 1) * 365;
	days += (year - 1) / 4;
	days -= (year - 1) / 100;
	days += (year - 1) / 400;

	for (uint64_t i = 1; i < month; ++i)
		days += DaysInMonth(year, i);

	days += (day - 1);

	return DateTime(days * Defs::MsPerDay);
}

DateTime DateTime::From::Date(uint64_t year, uint64_t month, uint64_t day, uint64_t hour, uint64_t minute, uint64_t second, uint64_t millisecond)
{
	Debug::Assert(hour < 24, "DateTime Date Error: hour must be a value between 0 and 23 inclusive.");
	Debug::Assert(minute < 60, "DateTime Date Error: minute must be a value between 0 and 59 inclusive.");
	Debug::Assert(second < 60, "DateTime Date Error: second must be a value between 0 and 59 inclusive.");
	Debug::Assert(millisecond < 1000, "DateTime Date Error: millisecond must be a value between 0 and 999 inclusive.");

	DateTime ret = Date(year, month, day);
	ret.ms += (hour * Defs::MsPerHour) + (minute * Defs::MsPerMinute) + (second * Defs::MsPerSecond) + millisecond;
	return ret;
}

DateTime DateTime::From::WindowsFileTime(const _FILETIME& date)
{
	uint64_t ms = std::bit_cast<uint64_t>(date) / Defs::Windows100NsPerMs;
	return DateTime(ms + (Defs::DaysFromBaseEpochToWindowsEpoch * Defs::MsPerDay));
}

const char* DateTime::MonthName(uint64_t month)
{
	Debug::Assert(month > 0 && month < 13, "const char* MonthName(uint64_t) Error: invalid value for month. It must range between 1 and 12 inclusive.");
	return MonthNameInternal(month);
}

const char* DateTime::DayOfWeekName(int day)
{
	//TODO: Framework: add _Analysis_noreturn_ to my assert
	Debug::Assert(day >= 1 && day <= 7, "const char* WeekDayName(uint64_t) Error: invalid value for day. It must range between 1 and 7 inclusive.");
	return weekDayNames[day - 1];
}

const char* DateTime::DayOfWeekName(uint64_t day, uint64_t month, uint64_t year)
{
	return DateTime::From::Date(year, month, day).DayOfWeekName();
}

int DateTime::DayOfWeek(uint64_t day, uint64_t month, uint64_t year)
{
	return DateTime::From::Date(year, month, day).DayOfWeek();
}

int DateTime::DayOfYear(uint64_t day, uint64_t month, uint64_t year)
{
	return DateTime::From::Date(year, month, day).DayOfYear();
}

bool DateTime::IsLeapYear(uint64_t year)
{
	return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

uint64_t DateTime::DaysInYear(uint64_t year)
{
	return IsLeapYear(year) ? 366 : 365;
}

uint64_t DateTime::DaysInMonth(uint64_t year, uint64_t month)
{
	if (month == 0 || month > 12) return 0;
	if (month == 2) return IsLeapYear(year) ? 29 : 28;
	if (month > 7) return month % 2 == 0 ? 31 : 30;
	return month % 2 == 0 ? 30 : 31;
}

DateTime::DateTime(uint64_t msSinceBaseEpoch)
	: ms(msSinceBaseEpoch)
{
}

DateTime DateTime::StartOfDay() const
{
	return From::Date(Year(), Month(), Day());
}

DateTime DateTime::EndOfDay() const
{
	return From::Date(Year(), Month(), Day(), 23, 59, 59, 999);
}

DateTime DateTime::StartOfMonth() const
{
	return From::Date(Year(), Month(), 1);
}

DateTime DateTime::EndOfMonth() const
{
	uint64_t year = Year();
	uint64_t month = Month();

	return From::Date(year, month, DaysInMonth(year, month), 23, 59, 59, 999);
}

DateTime DateTime::StartOfYear() const
{
	return From::Date(Year(), 1, 1);
}

DateTime DateTime::EndOfYear() const
{
	uint64_t year = Year();

	return From::Date(year, 12, DaysInMonth(year, 12), 23, 59, 59, 999);
}

uint64_t DateTime::Ms() const
{
	return ms % Defs::MsPerSecond;
}

void DateTime::Ms(uint64_t milliseconds)
{
	Debug::Assert(milliseconds < Defs::MsPerSecond, "DateTime Ms(uint64_t) Error: invalid value for milliseconds. It must not be higher than 999.");
	ms = ((ms / Defs::MsPerSecond) * Defs::MsPerSecond) + milliseconds;
}

uint64_t DateTime::Seconds() const
{
	return (ms / Defs::MsPerSecond) % Defs::SecondsPerMinute;
}

void DateTime::Seconds(uint64_t seconds)
{
	Debug::Assert(seconds < Defs::SecondsPerMinute, "DateTime Seconds(uint64_t) Error: invalid value for seconds. It must not be higher than 59.");
	uint64_t milliseconds = Ms() + (seconds * Defs::MsPerSecond);
	ms = ((ms / Defs::MsPerMinute) * Defs::MsPerMinute) + milliseconds;
}

uint64_t DateTime::Minutes() const
{
	return (ms / Defs::MsPerMinute) % Defs::MinutesPerHour;
}

void DateTime::Minutes(uint64_t minutes)
{
	Debug::Assert(minutes < Defs::MinutesPerHour, "DateTime Minutes(uint64_t) Error: invalid value for minutes. It must not be higher than 59.");
	uint64_t milliseconds = (ms % Defs::MsPerMinute) + (minutes * Defs::MsPerMinute);
	ms = ((ms / Defs::MsPerHour) * Defs::MsPerHour) + milliseconds;
}

uint64_t DateTime::Hour(bool f24h) const
{
	uint64_t h = (ms / Defs::MsPerHour) % Defs::HoursPerDay;
	return f24h ? h : To12h(h);
}

void DateTime::Hour(uint64_t hours)
{
	Debug::Assert(hours < Defs::HoursPerDay, "DateTime Hour(uint64_t) Error: invalid value for hours. It must not be higher than 23.");
	uint64_t milliseconds = (ms % Defs::MsPerHour) + (hours * Defs::MsPerHour);
	ms = ((ms / Defs::MsPerDay) * Defs::MsPerDay) + milliseconds;
}

void DateTime::Hour(uint64_t hours, bool am)
{
	Debug::Assert(hours > 0 && hours < 12, "DateTime Hour(uint64_t, bool) Error: invalid value for hours. It must range between 1 and 12 inclusive.");
	uint64_t milliseconds = (ms % Defs::MsPerHour) + (To24h(hours, am) * Defs::MsPerHour);
	ms = ((ms / Defs::MsPerDay) * Defs::MsPerDay) + milliseconds;
}

uint64_t DateTime::Day() const
{
	uint64_t day = 0;
	uint64_t year = 0;
	FindMonth(day, year);
	return day;
}

void DateTime::Day(uint64_t day)
{
	uint64_t days = 0, year = 0;
	uint64_t month = FindMonth(days, year);
	if (day == days) return;

	uint64_t daysInMonth = DaysInMonth(year, month);
	Debug::Assert<uint64_t>(day > 0 && day <= DaysInMonth(year, month), "DateTime Day(uint64_t) Error: invalid value for day. It must range between 1 and %d inclusive.", daysInMonth);

	ms -= (days * Defs::MsPerDay);
	ms += (day * Defs::MsPerDay);
}

const char* DateTime::DayOfWeekName() const
{
	return weekDayNames[DayOfWeek() - 1];
}

int DateTime::DayOfWeek() const
{
	uint64_t days = ms / Defs::MsPerDay;
	return (days % 7) + 1;
}

int DateTime::DayOfYear() const
{
	uint64_t days = 0;
	FindYear(days);
	return days + 1;
}

uint64_t DateTime::Month() const
{
	uint64_t days = 0;
	uint64_t year = 0;
	return FindMonth(days, year);
}

void DateTime::Month(uint64_t m)
{
	uint64_t days = 0, year = 0;
	uint64_t month = FindMonth(days, year);
	if (month == m) return;
	Debug::Assert(m > 0 && m < 13, "DateTime Month(uint64_t) Error: invalid value for month. It must range between 1 and 12 inclusive.");
	
	uint64_t dInM = DaysInMonth(year, month);
	uint64_t nDInM = DaysInMonth(year, m);
	uint64_t newDays = Maths::Clamp(days, 1, nDInM);

	ms -= (dInM * Defs::MsPerDay); 
	ms += (nDInM * Defs::MsPerDay);
	ms += ((newDays - 1) * Defs::MsPerDay);
}

const char* DateTime::MonthName() const
{
	return MonthNameInternal(Month());
}

uint64_t DateTime::DaysInMonth() const
{
	uint64_t day = 0, year = 0;
	uint64_t month = FindMonth(day, year);
	return DaysInMonth(year, month);
}

void DateTime::Year(uint64_t y)
{
	uint64_t days = 0, year = 0;
	uint64_t month = FindMonth(days, year);
	if (year == y) return;

	Debug::Assert<uint64_t>(y > 0, "DateTime Year(uint64_t) Error: invalid value for year. It must be greater than 0.", y);

	uint64_t dInM = DaysInMonth(year, month);
	uint64_t nDInM = DaysInMonth(y, month);
	uint64_t newDays = Maths::Clamp(days, 1, nDInM);

	uint64_t yearDays = 0;
	for (uint64_t i = 1; i < year; ++i) yearDays += DaysInYear(i);

	uint64_t newYearDays = 0;
	for (uint64_t i = 1; i < y; ++i) newYearDays += DaysInYear(i);

	ms -= (yearDays * Defs::MsPerDay);
	ms += (newYearDays * Defs::MsPerDay);
	ms -= ((days - 1) * Defs::MsPerDay);
	ms += ((newDays - 1) * Defs::MsPerDay);
}

uint64_t DateTime::DaysInYear() const
{
	uint64_t days = 0;
	uint64_t year = FindYear(days);
	return DaysInYear(year);
}

bool DateTime::IsLeapYear() const
{
	uint64_t days = 0;
	uint64_t year = FindYear(days);
	return IsLeapYear(year);
}

uint64_t DateTime::Year() const
{
	uint64_t days = 0;
	return FindYear(days);
}

void DateTime::operator+=(const CalendarSpan& other)
{
	if (other.negative)
	{
		*this -= other.Abs();
		return;
	}

	uint64_t newMs = Ms() + other.milliseconds;

	uint64_t newSecond = Seconds() + other.seconds;
	newSecond += newMs / Defs::MsPerSecond;
	newMs %= Defs::MsPerSecond;

	uint64_t newMinute = Minutes() + other.minutes;
	newMinute += newSecond / Defs::SecondsPerMinute;
	newSecond %= Defs::SecondsPerMinute;

	uint64_t newHour = Hour() + other.hours;
	newHour += newMinute / Defs::MinutesPerHour;
	newMinute %= Defs::MinutesPerHour;

	uint64_t newDay = Day() + other.days;
	newDay += (newHour / Defs::HoursPerDay);
	newHour %= Defs::HoursPerDay;

	uint64_t newMonth = Month();
	uint64_t newYear = Year();
	uint64_t d, daysInMonth = d = this->DaysInMonth(newYear, newMonth);
	uint64_t totalDays = newDay;
	while (d < totalDays)
	{
		newDay -= daysInMonth;
		uint64_t m = newMonth + 1;
		uint64_t y = newYear;
		if (m > 12)
		{
			m = 1;
			++y;
		}
		daysInMonth = this->DaysInMonth(newYear, newMonth);
		d += daysInMonth;
		newMonth = m;
		newYear = y;
	}

	*this = From::Date(newYear, newMonth, newDay, newHour, newMinute, newSecond, newMs);
}

void DateTime::operator-=(const CalendarSpan& other)
{
	if (other.negative)
	{
		*this += other.Abs();
		return;
	}

	uint64_t newMs = 0, extraSeconds = 0, currMs = Ms();
	if (currMs < other.milliseconds)
	{
		newMs = Defs::MsPerSecond - (other.milliseconds - currMs);
		extraSeconds = 1;
	}
	else newMs = currMs - other.milliseconds;

	uint64_t newSeconds = 0, extraMinutes = 0, totalSeconds = extraSeconds + other.seconds, currSeconds = Seconds();
	if (currSeconds < totalSeconds)
	{
		newSeconds = Defs::MsPerSecond - (totalSeconds - currSeconds);
		extraMinutes = 1;
	}
	else newSeconds = currSeconds - totalSeconds;

	uint64_t newMinutes = 0, extraHours = 0, totalMinutes = extraMinutes + other.minutes, currMinutes = Minutes();
	if (currMinutes < totalMinutes)
	{
		newMinutes = Defs::SecondsPerMinute - (totalMinutes - currMinutes);
		extraHours = 1;
	}
	else newMinutes = currMinutes - totalMinutes;

	uint64_t newHour = 0, extraDays = 0, totalHours = extraHours + other.hours, currHour = Hour();
	if (currHour < totalHours)
	{
		newHour = Defs::HoursPerDay - (totalHours - currHour);
		extraDays = 1;
	}
	else newHour = currHour - totalHours;

	uint64_t totalDays = extraDays + other.days, d = Day();
	uint64_t newMonth = Month();
	uint64_t newYear = Year();
	while (d < totalDays)
	{
		--newMonth;
		if (newMonth < 1)
		{
			newMonth = 12;
			--newYear;
		}

		d += DaysInMonth(newYear, newMonth);
	}

	uint64_t newDay = d - totalDays;

	*this = From::Date(newYear, newMonth, newDay, newHour, newMinutes, newSeconds, newMs);
}

DateTime DateTime::operator+(const CalendarSpan& other)
{
	DateTime result(*this);
	other.negative ?
		result -= other.Abs() :
		result += other;
	return result;
}

DateTime DateTime::operator-(const CalendarSpan& other)
{
	DateTime result(*this);
	other.negative ?
		result += other.Abs() :
		result -= other;
	return result;
}

uint64_t DateTime::FindYear(uint64_t& remainderDays) const
{
	uint64_t days = ms / Defs::MsPerDay;
	uint64_t year = (days / Defs::DaysPerGregorianCycle) * 400;
	days %= Defs::DaysPerGregorianCycle;

	uint64_t d = 0, i = 0;
	while (d + DaysInYear(year + 1 + i) <= days) d += DaysInYear(year + 1 + (i++));

	remainderDays = days - d;
	return year + i + 1;
}

uint64_t DateTime::FindMonth(uint64_t& day, uint64_t& year) const
{
	day = ms / Defs::MsPerDay;
	year = (day / Defs::DaysPerGregorianCycle) * 400;
	day %= Defs::DaysPerGregorianCycle;

	uint64_t d = 0, i = 0;
	while (d + DaysInYear(year + 1 + i) <= day) d += DaysInYear(year + 1 + (i++));

	day -= d;
	year += i + 1;

	i = 1;  d = 0;
	while (d + DaysInMonth(year, i) <= day) d += DaysInMonth(year, i++);

	day -= d + 1;
	return i;
}

uint64_t DateTime::To24h(uint64_t hour, bool am) const
{
	return am ? hour - 1 : hour == 12 ? 0 : hour + 12;
}

uint64_t DateTime::To12h(uint64_t hour) const
{
	return hour == 0 ? 12 : hour - 12;
}
