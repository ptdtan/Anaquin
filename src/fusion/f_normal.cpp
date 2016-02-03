#include "fusion/f_normal.hpp"
#include "parsers/parser_stab.hpp"

using namespace Anaquin;

FNormal::Stats FNormal::analyze(const FileName &splice, const Options &o)
{
    FNormal::Stats stats;
    const auto &r = Standard::instance().r_fus;

    ParserSTab::parse(Reader(splice), [&](const ParserSTab::Chimeric &c, const ParserProgress &)
    {
//        if (c.id == ChrT) { stats.chrT->n_chrT++; }
//        else                        { stats.chrT->n_endo++; }
//        
//        const SequinData *match;
// 
//        if ((match = r.findSplice(c.l)))
//        {
//            const auto expected = match->mixes.at(Mix_1);
//            const auto measured = c.unique;
//
//            stats.chrT->add(match->id, expected, measured);
//            stats.chrT->h.at(match->id)++;
//        }
    });

    //stats.chrT->ss = Standard::instance().r_fus.limit(stats.chrT->h);
    
    return stats;
}

void FNormal::report(const FileName &file, const Options &o)
{
    const auto stats = FNormal::analyze(file, o);

    o.info("Generating statistics");
    
    /*
     * Generating summary statistics
     */

    //AnalyzeReporter::linear("FusionNormal_summary.stats", file, stats, "introns", o.writer);

    /*
     * Generating Bioconductor
     */
    
    //AnalyzeReporter::scatter(stats, "", "FusionNormal", "Expected concentration (attomol/ul)", "Measured coverage (reads)", "Expected concentration (log2 attomol/ul)", "Measured coverage (log2 reads)", o.writer);
}