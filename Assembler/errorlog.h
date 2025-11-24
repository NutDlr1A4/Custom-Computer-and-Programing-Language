/*
* The 'ErrorLog' class generalizes errors and logging in a logger-object for use in each module of the assembler.
* General logs and warnings can be output/saved, as well as fatal errors.
* Errors will set the 'good' flag to false, which can be checked later to see if something ever went wrong.
*/

#pragma once

#include <iostream>
#include <string>
#include <optional>

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

enum class LoggingLevel {
	NONE = 0,
	ERROR,
	WARNING,
	ALL
};

class ErrorLog {
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