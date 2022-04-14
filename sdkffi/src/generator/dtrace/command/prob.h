#pragma once

namespace sdkffi {
namespace generator {
namespace dtrace {

///
/// dtrace -l to get list of available probes
///
struct Probe
{
    /// \brief Providers are libraries of probes that instrument a specific area
    /// of the system (for example, sched) or a mode of tracing (for example,
    /// fbt). New providers are written over time and added to newer releases (for
    /// example, ip, tcp, perl, python, mysql, and so on).
    std::string provider;

    /// \brief This is the kernel module where the probe is located. For user-land
    /// probes, it reflects the shared object library that contains the probe.
    std::string module;

    /// \brief This is the software function that contains this probe.
    std::string function;

    ///
    /// \brief This is a meaningful alias to describe the probe. For example, names
    /// such as entry and return are probes that fire at the entry and return of
    /// the corresponding function.
    ///
    std::string name;

    Probe() = default;

    Probe(const std::string & provider, const std::string & module, const std::string & function, const std::string & name) :
        provider(provider), module(module), function(function), name(name) {}

    [[nodiscard]] std::string ToString() const;
};
}  // namespace dtrace
}  // namespace generator
}  // namespace sdkffi