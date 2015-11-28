#ifndef M_VELVET_HPP
#define M_VELVET_HPP

#include "meta/m_histogram.h"
#include "parsers/parser_fa.hpp"
#include "parsers/parser_tsv.hpp"

namespace Anaquin
{
    struct Contig
    {
        ContigID id;
        
        // Length of the sequence being assembled
        Base len;
        
        // Size of the contig in k-mer
        Base k_len;

        // Unnormalized k-mer coverage
        Coverage k_cov;
        
        // Normalized k-mer coverage
        inline Coverage normalized() const { return k_cov / k_len; }
    };

    struct DAsssembly
    {
        template <typename T> struct Stats : public MappingStats
        {
            Base mean, min, max;
            Base N20, N50, N80;
            
            // Total number of bases in contigs
            Base total;
            
            // Total number of bases in the assembly
            Base sum;
            
            // List of aligned contigs (it's not all contigs)
            std::map<ContigID, T> contigs;

            // Total number of contigs, whether it's aligned or not
            Counts n = 0;
        };

        template <typename C = Contig, typename T = DAsssembly::Stats<C>> static T analyze
                (const FileName &file, const MBlat::Stats *blat, std::function<void (C&)> f)
        {
            T stats;
            Histogram h;
            
            ParserFA::parse(file, [&](const FALine &l, const ParserProgress &)
            {
                stats.n++;
                
                C c;
                
                c.id = l.id;
                
                auto id = c.id;
                
                if (blat)
                {
                    // Eg: "contig-30000000	21269" to "contig-30000000"
                    const auto first = Tokens::first(id, " ");

                    id = blat->aligns.count(first) ? first : "";

                    // Don't bother if the contig isn't defined in the alignment...
                    if (id.empty())
                    {
                        return;
                    }
                }
                
                // Size of the config
                c.len = l.seq.size();
                
                // The histogram needs the size of the sequence
                h.insert(l.seq.size());
                
                // Allows to apply custom operation
                f(c);
                
                stats.contigs[id] = c;
            });
            
            /*
             * https://github.com/bcgsc/abyss/blob/e58e5a6666e0de0e6bdc15c81fe488f5d83085d1/Common/Histogram.h
             */
            
            stats.sum   = h.sum();
            stats.N50   = h.n50();
            stats.min   = h.minimum();
            stats.max   = h.maximum();
            stats.mean  = h.expectedValue();
            stats.N80   = h.weightedPercentile(1 - 0.8);
            stats.N20   = h.weightedPercentile(1 - 0.2);
            stats.sum   = h.sum();
            stats.total = std::accumulate(stats.contigs.begin(), stats.contigs.end(), 0,
                            [&](int sum, const std::pair<std::string, C> &p)
                            {
                                return sum + p.second.len;
                            });
            return stats;
        }
    };

    struct Velvet
    {
        template <typename Stats, typename C> static Stats analyze
                        (const FileName &file, const MBlat::Stats *align = NULL)
        {
            Stats stats;
            
            /*
             * Read coverage from the contig file. The format looks like:
             *
             *      >NODE_77460_length_31_cov_1.129032
             */
            
            std::vector<std::string> toks;

            return DAsssembly::analyze<C, Stats>(file, align, [&](C &node)
            {
                Tokens::split(node.id, "_", toks);

                node.k_len = stoi(toks[3]);
                node.k_cov = stod(toks[toks.size() - 1]) * node.k_len;
            });
        }
    };
    
    struct RayMeta
    {
        template <typename Stats, typename C> static Stats analyze
                    (const FileName &file, const FileName &contigs, const MBlat::Stats *align)
        {
            std::map<ContigID, KMers> covs, lens;
            
            ParseTSV::parse(Reader(contigs), [&](const ParseTSV::TSV &t, const ParserProgress &)
            {
                covs[t.id] = t.kmer;
                lens[t.id] = t.klen;
            });
            
            Stats stats;
            
            return DAsssembly::analyze<C, Stats>(file, align, [&](C &node)
            {
                const auto id = Tokens::first(node.id, " ");
                
                if (covs.count(id))
                {
                    node.k_len = lens.at(id);
                    node.k_cov = covs.at(id);
                }
            });
        }
    };
}

#endif