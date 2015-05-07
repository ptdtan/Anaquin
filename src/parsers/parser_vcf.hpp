#ifndef GI_PARSER_VCF_HPP
#define GI_PARSER_VCF_HPP

#include <set>
#include "standard.hpp"
#include "parsers/parser.hpp"

namespace Spike
{
    typedef Variation VCFVariant;

    struct ParserVCF
    {
        static void parse(const std::string &file, std::function<void (const VCFVariant &, const ParserProgress &)>);
    };
}

#endif