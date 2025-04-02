/*
* The 'ErrorLog' class generalizes errors and logging in a logger-object for use in each module of the assembler.
* General logs and warnings can be output/saved, as well as fatal errors.
* Errors will set the 'good' flag to false, which can be checked later to see if something ever went wrong.
*/

export module errorlog;

import std;

/*
* To better control how much info should be logged, I came up with this:
* When a logger is made, it takes a logging level. This level determines which types of
* messages that should be printed, and which should be skipped.
* 
* What messages gets included in what level?
* None:		No messages, logging is disabled
* Error:	Only fatal errors
* Warning:	Warnings and errors
* All:		All messages, including regular logs
*/

export enum class LoggingLevel {
	NONE = 0,
	ERROR,
	WARNING,
	ALL
};

export class ErrorLog {
public:
	// The output stream determines where the log goes. It can be to the terminal (std::cout), 
	// to a file or somewhere else, your choice :)
	ErrorLog(const std::string& name, std::ostream& output_stream, LoggingLevel logging_level);
	
	// A scuffed way to copy a logger :/
	// Kinda like a copy constructor, but you have to give the new name of the logger aswell
	ErrorLog(const std::string& name, const ErrorLog& other);	

	// I feel like this is really messy, can this be cleaned up?
	void Log(std::string_view msg, std::string_view desc = "", const std::optional<size_t>& line = std::nullopt) const;
	void Warning(std::string_view msg, std::string_view desc = "", const std::optional<size_t>& line = std::nullopt) const;
	void Error(std::string_view msg, std::string_view desc = "", const std::optional<size_t>& line = std::nullopt);
	void FinalError();
	
	bool Good() const;

	std::ostream& GetOutputStream() const;
	LoggingLevel GetLoggingLevel() const;
private:
	// Used to format the output in the GenericLog() method
	enum class LogType {
		LOG,
		WARNING,
		ERROR
	};

	void GenericLog(LogType type, std::string_view msg, std::string_view desc = "", const std::optional<size_t>& line = std::nullopt) const;

	std::ostream& ost;

	std::string name;
	LoggingLevel level;
	bool good = true;
};

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
