#pragma once

inline void
PrintLogWarning(const std::string & message)
{
    //  std::cout << rang::bg::yellow << rang::fg::black << "[!] Warning: " <<
    //  rang::style::reset << " [ " << rang::bg::black << rang::fg::red << message
    //  << rang::style::reset << " ]" << std::endl;
    std::cout << "[" << rang::bg::yellow << rang::fg::black << " Warning  " << rang::style::reset
              << "] : [ " << rang::bg::black << rang::fg::yellow << message << rang::style::reset
              << " ]" << std::endl;
}

inline void
PrintLogError(const std::string & message)
{
    //  std::cout << rang::bg::red << rang::fg::black << "[!] Error  : " <<
    //  rang::style::reset << " [ " << rang::bg::black << rang::fg::red << message
    //  << rang::style::reset << " ]" << std::endl;
    std::cout << "[" << rang::bg::red << rang::fg::black << "  Error   " << rang::style::reset
              << "] : [ " << rang::bg::black << rang::fg::red << message << rang::style::reset << " ]"
              << std::endl;
}

inline void
PrintLogNormal(const std::string & message)
{
    std::cout << "[" << rang::bg::green << rang::fg::black << "    OK    " << rang::style::reset
              << "] : [ " << rang::bg::black << rang::fg::green << message << rang::style::reset
              << " ]" << std::endl;
    // std::cout << rang::bg::green << rang::fg::black << "[-] OK     : " <<
    // rang::style::reset << " [ " << rang::bg::black << rang::fg::cyan << message
    // << rang::style::reset << " ]" << std::endl;
}

inline void
PrintLogStatus(const std::string & message)
{
    std::cout << "[" << rang::bg::cyan << rang::fg::black << "  Status  " << rang::style::reset
              << "] : [ " << rang::bg::black << rang::fg::cyan << message << rang::style::reset
              << " ]" << std::endl;
    // std::cout << rang::bg::green << rang::fg::black << "[-] OK     : " <<
    // rang::style::reset << " [ " << rang::bg::black << rang::fg::cyan << message
    // << rang::style::reset << " ]" << std::endl;
}

inline void
PrintLogDebug(const std::string & message)
{
    std::cout << "[" << rang::bg::gray << rang::fg::black << "  Debug  " << rang::style::reset
              << "] : [ " << rang::bg::black << rang::fg::gray << message << rang::style::reset
              << " ]" << std::endl;
    // std::cout << rang::bg::green << rang::fg::black << "[-] OK     : " <<
    // rang::style::reset << " [ " << rang::bg::black << rang::fg::cyan << message
    // << rang::style::reset << " ]" << std::endl;
}

enum class LogType
{
    kInformation,
    kWarning,
    kError,
    kStatus,
    kDebug
};

inline void
PrintLog(const std::string & message, const LogType logType)
{
    switch (logType)
    {
    case LogType::kInformation:
        return PrintLogNormal(message);
    case LogType::kWarning:
        return PrintLogWarning(message);
    case LogType::kError:
        return PrintLogError(message);
    case LogType::kStatus:
        return PrintLogStatus(message);
    case LogType::kDebug:
        return PrintLogDebug(message);
    default:
        break;
    }
}