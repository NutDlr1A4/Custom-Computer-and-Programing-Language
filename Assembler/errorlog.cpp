#include "errorlog.h"

ErrorLog::ErrorLog(const std::string& name, std::ostream& output_stream, LoggingLevel logging_level)
	:
	name(name),
	ost(output_stream),
	level(logging_level) {
}

ErrorLog::ErrorLog(const std::string& name, const ErrorLog& other)
	:
	name(name),
	ost(other.GetOutputStream()),
	level(other.GetLoggingLevel())
{}

void ErrorLog::Log(std::string_view msg, std::string_view desc, const std::optional<size_t>& line) const {
	if ((int)level < (int)LoggingLevel::ALL) return;
	GenericLog(LogType::LOG, msg, desc, line);
}

void ErrorLog::Warning(std::string_view msg, std::string_view desc, const std::optional<size_t>& line) const {
	if ((int)level < (int)LoggingLevel::WARNING) return;
	GenericLog(LogType::WARNING, msg, desc, line);
}

void ErrorLog::Error(std::string_view msg, std::string_view desc, const std::optional<size_t>& line) {
	if ((int)level < (int)LoggingLevel::ERROR) return;
	GenericLog(LogType::ERROR, msg, desc, line);
	good = false;
}

void ErrorLog::FinalError() {
	ost << "One or more fatal errors have occured, and the program could not assemble succesfully." << std::endl;
	good = false;
}

bool ErrorLog::Good() const {
	return good;
}

std::ostream& ErrorLog::GetOutputStream() const {
	return ost;
}

LoggingLevel ErrorLog::GetLoggingLevel() const {
	return level;
}

void ErrorLog::GenericLog(LogType type, std::string_view msg, std::string_view desc, const std::optional<size_t>& line) const {
	// Some yummy sphaggeti formatting!

	ost << "[" << name << "] ";
	switch (type) {
	case ErrorLog::LogType::LOG:
		break;
	case ErrorLog::LogType::WARNING:
		ost << "Warning";
		break;
	case ErrorLog::LogType::ERROR:
		ost << "Error";
		break;
	default:
		break;
	}

	if (line.has_value()) {
		ost << " (line " << std::to_string(line.value()) << "): ";
	}
	else if (type != LogType::LOG) {
		ost << ": ";
	}

	ost << msg << std::endl;

	if (!desc.empty()) {
		ost << "\t" << desc << std::endl;
	}
}
