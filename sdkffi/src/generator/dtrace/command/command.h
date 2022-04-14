#pragma once

#include "prob.h"
#include "predicate.h"
#include "comment.h"
#include "action.h"

namespace sdkffi {

namespace generator {
namespace dtrace {
struct Command
{
    Comment   comment;
    Probe     probe;
    Predicate predicate;
    Action    action;

    Command(){};

    Command(const Comment & comment, const Probe & probe, const Predicate & predicate, const Action & action) :
        comment(comment), probe(probe), predicate(predicate), action(action) {}

    std::string ToString() const
    {
        std::stringstream ss;

        if (predicate.value.empty() == false)
        {
            ss << comment.value << "\n"
               << probe.ToString() << "\n"
               << predicate.value << "\n"
               << action.value << "\n\n";
        }
        else
        {
            ss << comment.value << "\n"
               << probe.ToString() << "\n"
               << action.value << "\n\n";
        }

        return ss.str();
    }
};
//
}  // namespace dtrace
}  // namespace generator
}  // namespace sdkffi