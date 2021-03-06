/*
 * Copyright (C) 2016 - Garvan Institute of Medical Research
 *
 *  Ted Wong, Bioinformatic Software Engineer at Garvan Institute.
 */

#ifndef V_FLIP_2_HPP
#define V_FLIP_2_HPP

#include "stats/analyzer.hpp"
#include "VarQuin/VarQuin.hpp"
#include "parsers/parser_sam.hpp"

namespace Anaquin
{
    struct VFlip
    {
        typedef AnalyzerOptions Options;
        
        struct Stats : public MappingStats
        {
            Counts nPaired = 0;
            Counts nSingle = 0;
            Counts nHanging = 0;

            Proportion pPaired;
            Proportion pSingle;
            Proportion pHanging;
        };

        struct Impl
        {
            virtual bool isReverse(const ChrID &) = 0;
            
            // Paired-end reads both mapped and complemented
            virtual void paired(const ParserSAM::Data &, const ParserSAM::Data &) = 0;
            
            // Non paired-end reads
            virtual void nonPaired(const ParserSAM::Data &) = 0;

            // Paired-end reads that the other mate not found
            virtual void unknownPaired(const ParserSAM::Data &) = 0;
        };

        static Stats analyze(const FileName &, const Options &, Impl &);
        static void  report (const FileName &, const Options &o = Options());
    };
}

#endif
