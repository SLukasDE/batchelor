/*
 * This file is part of Batchelor.
 * Copyright (C) 2023-2024 Sven Lukas
 *
 * Batchelor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Batchelor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with Batchelor.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <batchelor/common/Timestamp.h>

#include <esl/utility/String.h>

#define ONLY_C_LOCALE 1
#include <date/date.h>

#include <cstdint>
#include <ctime>
#include <stdexcept>
#include <time.h>

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 8)
#include <iomanip>
#include <sstream>
#endif

namespace batchelor {
namespace common {

// creates a string in ISO 8601 format, e.g. '2012-04-21T18:25:43-05:00'
std::string Timestamp::toJSON(const std::chrono::time_point<std::chrono::system_clock>& time_point) {
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 8)
	std::ostringstream ss;
	std::time_t tt = std::chrono::system_clock::to_time_t(time_point);
	std::tm tm = *std::localtime(&tt);
	ss << std::put_time(&tm, "%FT%T%z");
	return ss.str();
#else
	auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(time_point.time_since_epoch());
	std::time_t timestamp = millisecs.count() / 1000;
	int millisec = static_cast<int>(millisecs.count() % 1000);

	std::tm tm;
	localtime_r(&timestamp, &tm);

	char buffer[100];
	char* bufferPtr = &buffer[0];
	std::size_t length;

	length = strftime(bufferPtr, 100, "%FT%T", &tm);
	if(length == 0) {
		return "";
	}
	bufferPtr += length;

	sprintf(bufferPtr, ".%03d", millisec);
	bufferPtr += 4;

	length = strftime(bufferPtr, 100-length-4, "%z", &tm);
	if(length == 0) {
		return "";
	}

	bufferPtr[length+1] = 0;
	bufferPtr[length-0] = bufferPtr[length-1];
	bufferPtr[length-1] = bufferPtr[length-2];
	bufferPtr[length-2] = ':';

	return buffer;
#endif
}

std::chrono::time_point<std::chrono::system_clock> Timestamp::fromJSON(const std::string& str) {
	std::tm timeinfo;
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 8)
	std::istringstream is{str};
	is.imbue(std::locale("de_DE.utf-8"));
	is >> std::get_time(&timeinfo, "%FT%T%z");
    if (is.fail()) {
        throw std::runtime_error("Parse failed");
    }
#else
    strptime(str.c_str(), "%FT%T%z", &timeinfo);
#endif
    std::time_t tt = std::mktime(&timeinfo);
    return std::chrono::system_clock::from_time_t(tt);
}


std::string  Timestamp::toString(const std::chrono::system_clock::time_point& tp) {
	return date::format("%F %T", tp);
}

std::chrono::system_clock::time_point  Timestamp::fromString(const std::string& tpStr) {
	std::chrono::system_clock::time_point tp;

	if(!tpStr.empty()) {
		std::istringstream ss(tpStr);
		date::from_stream(ss, "%F %T", tp);
	}

	return tp;
}

std::chrono::milliseconds Timestamp::toDuration(const std::string& str) {
	std::string unit;
	for(auto iter = str.rbegin(); iter != str.rend() && !std::isdigit(*iter); ++iter) {
		unit = *iter + unit;
	}
	std::string numStr = str.substr(0, str.size() - unit.size());
	unit = esl::utility::String::toLower(esl::utility::String::trim(unit));

	if(unit == "ms") {
		return std::chrono::milliseconds(esl::utility::String::toNumber<unsigned int>(numStr));
	}
	if(unit == "s" || unit == "sec") {
		return std::chrono::seconds(esl::utility::String::toNumber<unsigned int>(numStr));
	}
	if(unit == "m" || unit == "min") {
		return std::chrono::minutes(esl::utility::String::toNumber<unsigned int>(numStr));
	}
	if(unit == "h" || unit == "houres") {
		return std::chrono::hours(esl::utility::String::toNumber<unsigned int>(numStr));
	}

    throw std::runtime_error("Parse failed because of unknown unit '" + unit + "'");
}

} /* namespace common */
} /* namespace batchelor */
