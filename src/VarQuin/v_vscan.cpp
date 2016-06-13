#include "VarQuin/v_vscan.hpp"
#include "writers/file_writer.hpp"
#include "parsers/parser_varscan.hpp"

using namespace Anaquin;

VVScan::Stats VVScan::analyze(const FileName &file,
                              const FileName &output,
                              const Options &o)
{
    FileWriter out(o.work);
    out.open(output);
    
    VVScan::Stats stats;

    const auto format = "%1%\t%2%\t%3%\t%4%\t%5%\t%6%\t%7%\t%8%\t%9%";
    out.write((boost::format(format) % "Chrom"
                                     % "Position"
                                     % "Ref"
                                     % "Allele"
                                     % "ReadsR"
                                     % "ReadsV"
                                     % "QualR"
                                     % "QualV"
                                     % "Pvalue").str());
    
    ParserVarScan::parse(Reader(file), [&](const ParserVarScan::Data &x, const ParserProgress &)
    {
        if (Standard::isSynthetic(x.cID))
        {
            stats.n_syn++;
        }
        else
        {
            stats.n_gen++;
        }

        out.write((boost::format(format) % x.cID
                                         % x.l.start
                                         % x.ref
                                         % x.alt
                                         % x.readR
                                         % x.readV
                                         % x.qualR
                                         % x.qualV
                                         % x.p).str());
    });
    
    out.close();
    
    return stats;
}

void VVScan::report(const FileName &file, const Options &o)
{
    analyze(file, "VarQuin_variants.txt", o);
}