/*
 * Copyright (C) 2016 - Garvan Institute of Medical Research
 *
 *  Ted Wong, Bioinformatic Software Engineer at Garvan Institute.
 */

#ifndef R_ASSEMBLY_HPP
#define R_ASSEMBLY_HPP

#include "stats/analyzer.hpp"

namespace Anaquin
{
    struct RAssembly : Analyzer
    {
        typedef FuzzyOptions Options;
        
        struct Stats
        {
            struct Data
            {
                /*
                 * Base statistics
                 */
                
                double bSP, bSN;
                
                /*
                 * Exon statistics
                 */
                
                double eSP, eSN, eFSP, eFSN;

                /*
                 * Intron statistics
                 */
                
                double iSP, iSN, iFSP, iFSN;

                /*
                 * Intron-chain statistics
                 */
                
                double cSP, cSN, cFSP, cFSN;
                
                /*
                 * Transcript statistics
                 */
                
                double tSP, tSN, tFSP, tFSN;
                
                /*
                 * Missing statistics
                 */
                
                double mExonN,   mExonR,   mExonP;
                double mIntronN, mIntronR, mIntronP;
                
                /*
                 * Novel statistics
                 */
                
                double   nExonP, nIntronP;
                unsigned nExonN, nExonR, nIntronN, nIntronR;
            };
            
            std::map<ChrID, Data> data;
            
            // Sensitivity for each sequin
            std::map<SequinID, Proportion> tSPs;

            Counts sExons   = 0;
            Counts sIntrons = 0;
            Counts sTrans   = 0;
            Counts sGenes   = 0;

            Counts gExons   = 0;
            Counts gIntrons = 0;
            Counts gTrans   = 0;
            Counts gGenes   = 0;
        };

        // Analyze for a single sample
        static Stats analyze(const FileName &, const Options &o = Options());

        // Report for a single sample
        static void report(const FileName &, const Options &o = Options());
    };
}

#endif