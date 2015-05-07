#ifndef GI_R_ALIGN_HPP
#define GI_R_ALIGN_HPP

#include "r_analyzer.hpp"

namespace Spike
{
    struct RAlignStats
    {
        // Number of times that each exon is positively identified
        LocusCounter e_lc = RAnalyzer::exonCounter();

        // Number of times that each intron is positively identified
        LocusCounter i_lc = RAnalyzer::intronCounter();

        Counter cb = RAnalyzer::geneCounter();
        Counter ce = RAnalyzer::geneCounter();
        Counter ci = RAnalyzer::geneCounter();

        Confusion   mb, me, mi;
        Sensitivity sb, se, si;
    };

    struct RAlign : public RAnalyzer
    {
        struct Options : public SingleMixtureOptions
        {
            // Empty Implementation
        };

        static RAlignStats analyze(const std::string &file, const Options &options = Options());
    };
}

#endif