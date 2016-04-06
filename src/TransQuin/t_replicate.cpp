#include <stdexcept>
#include "writers/r_writer.hpp"
#include "TransQuin/t_replicate.hpp"

using namespace Anaquin;

// Defined in resources.cpp
extern Scripts PlotRAbundAbund();

// Defined in resources.cpp
extern Scripts PlotMajor();

typedef TReplicate::Metrics  Metrics;
typedef TReplicate::Software Software;

std::vector<TReplicate::Stats> TReplicate::analyze(const std::vector<FileName> &files, const Options &o)
{
    if (files.size() < 2)
    {
        throw std::runtime_error("Two or more replicates required");
    }
    
    std::vector<TReplicate::Stats> stats;
    
    for (const auto &file : files)
    {
        stats.push_back(TExpress::analyze(file, o));
    }
    
    return stats;
}

static Scripts writeCSV(const std::vector<TReplicate::Stats> &stats)
{
    std::stringstream ss;
    ss << ((boost::format("Sequin\tSample\tEAbund\tMAbund\n")).str());
    
    for (auto i = 0; i < stats.size(); i++)
    {
        const auto &data = stats[i].data;

        for (const auto &j : data)
        {
            ss << ((boost::format("%1%\t%2%\t%3%t%4%\n") % j.first
                                                         % (i+1)
                                                         % j.second.x
                                                         % j.second.y).str());
        }
    }
    
    return ss.str();
}

void TReplicate::report(const std::vector<FileName> &files, const Options &o)
{
    const auto m = std::map<TReplicate::Metrics, std::string>
    {
        { TReplicate::Metrics::Gene,    "genes"    },
        { TReplicate::Metrics::Isoform, "isoforms" },
    };
    
    const auto units = m.at(o.metrs);
    const auto stats = analyze(files, o);
    
    std::vector<SequinHist>   sHists;
    std::vector<LinearStats>  lStats;
    std::vector<MappingStats> mStats;
    
    for (auto i = 0; i < files.size(); i++)
    {
        mStats.push_back(stats[i]);
        sHists.push_back(stats[i].hist);
        lStats.push_back(stats[i].data);
    }
    
    /*
     * Generating summary statistics for the replicates
     */
    
    o.info("Generating TransReplicate_summary.stats");
    o.writer->open("TransReplicate_summary.stats");
    o.writer->write(StatsWriter::inflectSummary(o.rChrT, o.rEndo, files, sHists, mStats, lStats, units));
    o.writer->close();
    
    /*
     * Generating detailed statistics for the sequins
     */
    
    o.info("Generating TransReplicate_quin.csv");
    o.writer->open("TransReplicate_quin.csv");
    o.writer->write(writeCSV(stats));
    o.writer->close();
    
    /*
     * Generating abundance vs abundance (with standard deviation)
     */
    
    o.info("Generating TransReplicate_abundAbund.R");
    o.writer->open("TransReplicate_abundAbund.R");
    o.writer->write(RWriter::createScript("TransReplicate_quin.csv", PlotRAbundAbund()));
    o.writer->close();
    
    /*
     * Generating major plot for all samples (but only if we have the isoforms...)
     */
    
    if (o.metrs == TReplicate::Metrics::Isoform)
    {
        o.writer->open("TransReplicate_major.R");
        o.writer->write(RWriter::createScript("TransReplicate_quin.csv", PlotMajor()));
        o.writer->close();
    }
}