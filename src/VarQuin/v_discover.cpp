#include "data/convert.hpp"
#include "VarQuin/v_discover.hpp"

using namespace Anaquin;

// Defined in resources.cpp
extern Scripts PlotVLODR();

// Defined in resources.cpp
extern Scripts PlotVGROC();

// Defined in resources.cpp
extern Scripts PlotVCROC();

static Counts __countD__ = 0;
static Counts __countP__ = 0;

// Defined in main.cpp
extern Path __output__;

// Defined in main.cpp
extern std::string __full_command__;

static Scripts createVGROC(const FileName &file, const std::string &score, const std::string &refRat)
{
    return (boost::format(PlotVGROC()) % date()
                                       % __full_command__
                                       % __output__
                                       % file
                                       % score
                                       % refRat).str();
}

static Scripts createVCROC(const FileName &file, const std::string &score, const std::string &refRat)
{
    return (boost::format(PlotVCROC()) % date()
                                       % __full_command__
                                       % __output__
                                       % file
                                       % score
                                       % refRat).str();
}

struct VDiscoverImpl : public VCFDataUser
{
    VDiscover::Stats *stats;
    const VDiscover::Options *o;

    void variantProcessed(const ParserVCF::Data &x, const ParserProgress &p) override
    {
        if (p.i && !(p.i % 100000))
        {
            o->wait(std::to_string(p.i));
        }
        
        const auto &r = Standard::instance().r_var;
        
        VariantMatch m;

        auto match = [&](const ParserVCF::Data &query)
        {
            m.query = query;
            m.match = nullptr;
            
            const auto isSyn = isVarQuin(query.cID);
            
            if (isSyn || Standard::isGenomic(query.cID))
            {
                // Can we match by position?
                m.match = r.findVar(query.cID, query.l);
                
                if (m.match)
                {
                    m.ref = m.match->ref == query.ref;
                    m.alt = m.match->alt == query.alt;
                }
            }
            
            return m.match;
        };
     
        match(x);
        
        const auto &cID = m.query.cID;
        
        auto f = [&]()
        {
            if (!isnan(m.query.p))     { __countP__++; }
            if (!isnan(m.query.depth)) { __countD__++; }

            // Only matching if the position and alleles agree
            const auto matched = m.match && m.ref && m.alt;
            
            if (matched)
            {
                const auto key = m.match->key();
                stats->hist.at(cID).at(key)++;
                
                stats->data[cID].tps.push_back(m);
                stats->data[cID].tps_[key] = stats->data[cID].tps.back();
                
                stats->data.at(cID).af = m.query.alleleFreq();
                
                const auto exp = r.findAFreq(baseID(m.match->id));
                const auto obs = m.query.alleleFreq();
                
                // Eg: 2821292107
                const auto id = toString(key);
                
                // Add for all variants
                stats->vars.add(id, exp, obs);
                
                switch (m.query.type())
                {
                    case Mutation::SNP:       { stats->snp.add(id, exp, obs); break; }
                    case Mutation::Deletion:
                    case Mutation::Insertion: { stats->ind.add(id, exp, obs); break; }
                }
                
                stats->readR[key] = m.query.readR;
                stats->readV[key] = m.query.readV;
                stats->depth[key] = m.query.depth;
                
                if (isnan(stats->vars.limit.abund) || exp < stats->vars.limit.abund)
                {
                    stats->vars.limit.id = m.match->id;
                    stats->vars.limit.abund = exp;
                }
            }
            else
            {
                // FP because the variant is not found in the reference
                stats->data[cID].fps.push_back(m);
            }
        };
        
        // Always work on the queries
        stats->query[cID].af.insert(m.query.alleleFreq());

        if (isVarQuin(cID))
        {
            stats->nSyn++;
            f();
        }
        else
        {
            stats->nGen++;
            
            if (Standard::isGenomic(cID))
            {
                f();
            }
        }
    }
};

VDiscover::Stats VDiscover::analyze(const FileName &file, const Options &o)
{
    const auto &r = Standard::instance().r_var;

    VDiscover::Stats stats;
    stats.hist = r.vHist();

    for (const auto &i : stats.hist)
    {
        stats.data[i.first];
    }

    o.analyze(file);

    VDiscoverImpl impl;
    impl.o = &o;
    impl.stats = &stats;

    // Read the input variants
    stats.vData = vcfData(file, o.format, &impl);

    o.info("Aggregating statistics");

    for (const auto &i : stats.data)
    {
        const auto &cID = i.first;
        
        auto &x = stats.data[cID];
        
        for (const auto &j : i.second.tps)
        {
            i.second.m.tp()++;
            
            switch (j.query.type())
            {
                case Mutation::SNP:       { x.m_snp.tp()++; break; }
                case Mutation::Deletion:
                case Mutation::Insertion: { x.m_ind.tp()++; break; }
            }
        }
        
        for (const auto &j : i.second.fps)
        {
            i.second.m.fp()++;
            
            switch (j.query.type())
            {
                case Mutation::SNP:       { x.m_snp.fp()++; break; }
                case Mutation::Deletion:
                case Mutation::Insertion: { x.m_ind.fp()++; break; }
            }
        }
        
        x.m_snp.nq() = x.dSNP();
        x.m_snp.nr() = r.countSNP(cID);
        x.m_ind.nq() = x.dInd();
        x.m_ind.nr() = r.countInd(cID);

        x.m.nq() = x.m_snp.nq() + x.m_ind.nq();
        x.m.nr() = x.m_snp.nr() + x.m_ind.nr();

        x.m.fn()     = x.m.nr()     - x.m.tp();
        x.m_snp.fn() = x.m_snp.nr() - x.m_snp.tp();
        x.m_ind.fn() = x.m_ind.nr() - x.m_ind.tp();

        assert(x.m.nr() >= x.m.fn());
        assert(x.m_snp.nr() >= x.m_snp.fn());
        assert(x.m_ind.nr() >= x.m_ind.fn());
    }
    
    return stats;
}

static void writeQuins(const FileName &file,
                       const VDiscover::Stats &stats,
                       const VDiscover::Options &o)
{
    const auto &r = Standard::instance().r_var;
    const auto format = "%1%\t%2%\t%3%\t%4%\t%5%\t%6%\t%7%\t%8%\t%9%\t%10%\t%11%\t%12%\t%13%\t%14%\t%15%\t%16%";

    o.generate(file);
    o.writer->open(file);
    o.writer->write((boost::format(format) % "ID"
                                           % "ChrID"
                                           % "Position"
                                           % "Label"
                                           % "ReadR"
                                           % "ReadV"
                                           % "Depth"
                                           % "ExpRef"
                                           % "ExpVar"
                                           % "ExpFreq"
                                           % "ObsFreq"
                                           % "Pval"
                                           % "Qual"
                                           % "QualR"
                                           % "QualV"
                                           % "Type").str());
    for (const auto &i : stats.hist)
    {
        const auto &cID = i.first;
        
        // Search all query variants...
        for (const auto &j : i.second)
        {
            const auto &key = j.first;

            // Detected the sequin?
            if (j.second)
            {
                const auto m = r.findVar(cID, key);
                assert(m);
                
                // Eg: "SNP"
                const auto type = type2str(m->type());

                // Unique ID for the variant
                const auto id = (m->id + "_" + std::to_string(m->l.start) + "_" + type);

                /*
                 * Now we need to know the label for this reference variant
                 */
                
                auto f = [&](const std::map<long, VariantMatch> &x, const std::string &label)
                {
                    if (x.count(key))
                    {
                        const auto &t = x.at(key);
                        
                        o.writer->write((boost::format(format) % id
                                                               % m->cID
                                                               % m->l.start
                                                               % label
                                                               % t.query.readR
                                                               % t.query.readV
                                                               % t.query.depth
                                                               % r.findRCon(m->id)
                                                               % r.findVCon(m->id)
                                                               % r.findAFreq(m->id)
                                                               % t.query.alleleFreq()
                                                               % ld2ss(t.query.p)
                                                               % x2ns(t.query.qual)
                                                               % x2ns(t.query.qualR)
                                                               % x2ns(t.query.qualV)
                                                               % type).str());
                        return true;
                    }
                    
                    return false;
                };

                if (!f(stats.data.at(i.first).tps_, "TP") && !f(stats.data.at(i.first).fns_, "FN"))
                {
                    throw std::runtime_error("Failed to find hash key in writeQuins()");
                }
            }
            
            // Failed to detect the variant
            else
            {
                const auto m = r.findVar(i.first, j.first);
                assert(m);
                
                // Eg: "SNP"
                const auto type = type2str(m->type());
                
                // Unique ID for the variant
                const auto id = (m->id + "_" + std::to_string(m->l.start) + "_" + type);
                
                o.writer->write((boost::format(format) % id
                                                       % m->cID
                                                       % m->l.start
                                                       % "FN"
                                                       % "NA"
                                                       % "NA"
                                                       % "NA"
                                                       % r.findRCon(m->id)
                                                       % r.findVCon(m->id)
                                                       % r.findAFreq(m->id)
                                                       % "NA"
                                                       % "NA"
                                                       % "NA"
                                                       % "NA"
                                                       % "NA"
                                                       % type).str());
            }
        }
    }
    
    o.writer->close();
}

static void writeDetected(const FileName &file, const VDiscover::Stats &stats, const VDiscover::Options &o)
{
    const auto &r = Standard::instance().r_var;
    const auto format = "%1%\t%2%\t%3%\t%4%\t%5%\t%6%\t%7%\t%8%\t%9%\t%10%\t%11%\t%12%\t%13%\t%14%\t%15%\t%16%";
    
    o.generate(file);
    o.writer->open(file);
    o.writer->write((boost::format(format) % "ID"
                                           % "ChrID"
                                           % "Position"
                                           % "Label"
                                           % "ReadR"
                                           % "ReadV"
                                           % "Depth"
                                           % "ExpRef"
                                           % "ExpVar"
                                           % "ExpFreq"
                                           % "ObsFreq"
                                           % "Pval"
                                           % "Qual"
                                           % "QualR"
                                           % "QualV"
                                           % "Type").str());

    for (const auto &i : stats.data)
    {
        const auto &cID = i.first;
        
        auto f = [&](const std::vector<VariantMatch> &x, const std::string &label)
        {
            for (const auto &i : x)
            {
                auto sID = (i.match ? i.match->id : "-");
                
                if (label == "FP")
                {
                    /*
                     * We know this is a FP, but we can trace it to one of the sequins?
                     */
                    
                    if (r.hasInters(cID))
                    {
                        // We'll need it to search for the sequin where the FPs are
                        const auto inters = r.mInters(cID);
                        
                        A_ASSERT(inters.size());
                        
                        const auto m = inters.contains(i.query.l);
                        
                        // Can we find the corresponding region for the FP?
                        if (m)
                        {
                            sID = m->id();
                            
                            // It has to be a sequin (eg: D_3_12)
                            A_ASSERT(!sID.empty());
                        }
                    }
                }
                
                const auto eRef  = sID != "-" ? r.findRCon(sID)  : NAN;
                const auto eVar  = sID != "-" ? r.findVCon(sID)  : NAN;
                const auto eFreq = sID != "-" ? r.findAFreq(sID) : NAN;
                
                o.writer->write((boost::format(format) % sID
                                                       % i.query.cID
                                                       % i.query.l.start
                                                       % label
                                                       % i.query.readR
                                                       % i.query.readV
                                                       % i.query.depth
                                                       % eRef
                                                       % eVar
                                                       % eFreq
                                                       % i.query.alleleFreq()
                                                       % ld2ss(i.query.p)
                                                       % x2ns(i.query.qual)
                                                       % x2ns(i.query.qualR)
                                                       % x2ns(i.query.qualV)
                                                       % type2str(i.query.type())).str());
            }
        };
        
        f(i.second.tps, "TP");
        f(i.second.fps, "FP");
    }

    o.writer->close();
}

static void writeSummary(const FileName &file, const FileName &src, const VDiscover::Stats &stats, const VDiscover::Options &o)
{
    const auto &r = Standard::instance().r_var;

    extern FileName VCFRef();
    extern FileName BedRef();
    extern FileName MixRef();

    auto germline = [&]()
    {
        const auto summary = "-------VarDiscover Output Results\n\n"
                             "-------VarDiscover Output\n\n"
                             "       Reference variant annotations:    %1%\n"
                             "       Reference coordinate annotations: %2%\n"
                             "       Sequin mixture file:              %3%\n"
                             "       User identified variants:         %4%\n\n"
                             "-------Reference annotated variants\n\n"
                             "       Synthetic: %5% SNPs\n"
                             "       Synthetic: %6% indels\n"
                             "       Synthetic: %7% variants\n\n"
                             "-------User identified variants\n\n"
                             "       Synthetic: %8% SNPs\n"
                             "       Synthetic: %9% indels\n"
                             "       Synthetic: %10% variants\n\n"
                             "-------Identification of synthetic variants\n\n"
                             "       True Positive:  %11% SNPS\n"
                             "       True Positive:  %12% indels\n"
                             "       True Positive:  %13% variants\n\n"
                             "       False Positive: %14% SNPs\n"
                             "       False Positive: %15% indels\n"
                             "       False Positive: %16% variants\n\n"
                             "       False Negative: %17% SNPs\n"
                             "       False Negative: %19% indels\n"
                             "       False Negative: %19% variants\n\n"
                             "-------Diagnostic Performance (Synthetic)\n\n"
                             "       *Variants\n"
                             "       Sensitivity: %20$.4f\n"
                             "       Precision:   %21$.4f\n"
                             "       F1 Score:    %22$.4f\n"
                             "       FDR Rate:    %23$.4f\n\n"
                             "       *SNPs\n"
                             "       Sensitivity: %24$.4f\n"
                             "       Precision:   %25$.4f\n"
                             "       F1 Score:    %26$.4f\n"
                             "       FDR Rate:    %27$.4f\n\n"
                             "       *Indels\n"
                             "       Sensitivity: %28$.4f\n"
                             "       Precision:   %29$.4f\n"
                             "       F1 Score:    %30$.4f\n"
                             "       FDR Rate:    %31$.4f\n";

        o.generate(file);
        o.writer->open("VarDiscover_summary.stats");
        o.writer->write((boost::format(summary) % VCFRef()                   // 1
                                                % BedRef()                   // 2
                                                % MixRef()                   // 3
                                                % src                        // 4
                                                % r.countSNPSyn()            // 5
                                                % r.countIndSyn()            // 6
                                                % (r.countSNPSyn() + r.countIndSyn())
                                                % stats.vData.countSNPSyn()  // 8
                                                % stats.vData.countIndSyn()  // 9
                                                % stats.vData.countVarSyn()  // 10
                                                % stats.countSNP_TP_Syn()    // 11
                                                % stats.countInd_TP_Syn()    // 12
                                                % stats.countVar_TP_Syn()    // 13
                                                % stats.countSNP_FP_Syn()    // 14
                                                % stats.countInd_FP_Syn()    // 15
                                                % stats.countVar_FP_Syn()    // 16
                                                % stats.countSNP_FnSyn()     // 17
                                                % stats.countInd_FnSyn()     // 18
                                                % stats.countVar_FnSyn()     // 19
                                                % stats.countVarSnSyn()      // 20
                                                % stats.countVarPC_Syn()     // 21
                                                % stats.varF1()              // 22
                                                % (1-stats.countVarPC_Syn()) // 23
                                                % stats.countSNPSnSyn()      // 24
                                                % stats.countSNPPC_Syn()     // 25
                                                % stats.SNPF1()              // 26
                                                % (1-stats.countSNPPC_Syn()) // 27
                                                % stats.countIndSnSyn()      // 28
                                                % stats.countIndPC_Syn()     // 29
                                                % stats.indelF1()            // 30
                                                % (1-stats.countIndPC_Syn()) // 31
                         ).str());
    };
    
    auto somatic = [&]()
    {
        const auto lm = stats.vars.linear(true);

        const auto summary = "-------VarDiscover Output Results\n\n"
                             "-------VarDiscover Output\n\n"
                             "       Reference variant annotations:    %1%\n"
                             "       Reference coordinate annotations: %2%\n"
                             "       Sequin mixture file:              %3%\n"
                             "       User identified variants:         %4%\n\n"
                             "-------Reference variant annotations\n\n"
                             "       Synthetic: %5% SNPs\n"
                             "       Synthetic: %6% indels\n"
                             "       Synthetic: %7% variants\n\n"
                             "-------User identified variants\n\n"
                             "       Synthetic: %11% SNPs\n"
                             "       Synthetic: %12% indels\n"
                             "       Synthetic: %13% variants\n\n"
                             "       Detection Sensitivity: %14% (attomol/ul) (%15%)\n\n"
                             "-------Identification of synthetic variants\n\n"
                             "       True Positive:  %16% SNPS\n"
                             "       True Positive:  %17% indels\n"
                             "       True Positive:  %18% variants\n\n"
                             "       False Positive: %19% SNPs\n"
                             "       False Positive: %20% indels\n"
                             "       False Positive: %21% variants\n\n"
                             "       False Negative: %22% SNPs\n"
                             "       False Negative: %23% indels\n"
                             "       False Negative: %24% variants\n\n"
                             "-------Diagnostic Performance (Synthetic)\n\n"
                             "       *Variants\n"
                             "       Sensitivity: %25$.4f\n"
                             "       Precision:   %26$.4f\n"
                             "       FDR Rate:    %27$.4f\n\n"
                             "       *SNPs\n"
                             "       Sensitivity: %28$.4f\n"
                             "       Precision:   %29$.4f\n"
                             "       FDR Rate:    %30$.4f\n\n"
                             "       *Indels\n"
                             "       Sensitivity: %31$.4f\n"
                             "       Precision:   %32$.4f\n"
                             "       FDR Rate:    %33$.4f\n\n"
                             "-------Overall linear regression (log2 scale)\n\n"
                             "      Slope:       %34%\n"
                             "      Correlation: %35%\n"
                             "      R2:          %36%\n"
                             "      F-statistic: %37%\n"
                             "      P-value:     %38%\n"
                             "      SSM:         %39%, DF: %40%\n"
                             "      SSE:         %41%, DF: %42%\n"
                             "      SST:         %43%, DF: %44%\n";
        o.generate(file);
        o.writer->open("VarDiscover_summary.stats");
        o.writer->write((boost::format(summary) % VCFRef()                   // 1
                                                % BedRef()                   // 2
                                                % MixRef()                   // 3
                                                % src                        // 4
                                                % r.countSNPSyn()            // 5
                                                % r.countIndSyn()            // 6
                                                % (r.countSNPSyn() + r.countIndSyn())
                                                % r.countSNPGen()            // 8
                                                % r.countIndGen()            // 9
                                                % (r.countSNPGen() + r.countIndGen())
                                                % stats.vData.countSNPSyn()  // 11
                                                % stats.vData.countIndSyn()  // 12
                                                % stats.vData.countVarSyn()  // 13
                                                % stats.vars.limit.abund     // 14
                                                % stats.vars.limit.id        // 15
                                                % stats.countSNP_TP_Syn()    // 16
                                                % stats.countInd_TP_Syn()    // 17
                                                % stats.countVar_TP_Syn()    // 18
                                                % stats.countSNP_FP_Syn()    // 19
                                                % stats.countInd_FP_Syn()    // 20
                                                % stats.countVar_FP_Syn()    // 21
                                                % stats.countSNP_FnSyn() // 22
                                                % stats.countInd_FnSyn() // 23
                                                % stats.countVar_FnSyn() // 24
                                                % stats.countVarSnSyn()  // 25
                                                % stats.countVarPC_Syn()     // 26
                                                % (1-stats.countVarPC_Syn()) // 27
                                                % stats.countSNPSnSyn()  // 28
                                                % stats.countSNPPC_Syn()     // 29
                                                % (1-stats.countSNPPC_Syn()) // 30
                                                % stats.countIndSnSyn()  // 31
                                                % stats.countIndPC_Syn()     // 32
                                                % (1-stats.countIndPC_Syn()) // 33
                                                % lm.m                       // 34
                                                % lm.r                       // 35
                                                % lm.R2                      // 36
                                                % lm.F                       // 37
                                                % lm.p                       // 38
                                                % lm.SSM                     // 39
                                                % lm.SSM_D                   // 40
                                                % lm.SSE                     // 41
                                                % lm.SSE_D                   // 42
                                                % lm.SST                     // 43
                                                % lm.SST_D                   // 44
                         ).str());
    };
    
    if (r.isGermline())
    {
        germline();
    }
    else
    {
        somatic();
    }

    o.writer->close();
}

void VDiscover::report(const FileName &file, const Options &o)
{
    const auto &r = Standard::instance().r_var;

    // Statistics for the variants
    const auto stats = analyze(file, o);
    
    o.info("TP: " + std::to_string(stats.countVar_TP_Syn()));
    o.info("FP: " + std::to_string(stats.countVar_FP_Syn()));
    o.info("FN: " + std::to_string(stats.countVar_FnSyn()));

    o.info("Generating statistics");

    /*
     * Generating VarDiscover_sequins.csv
     */
    
    writeQuins("VarDiscover_sequins.csv", stats, o);

    /*
     * Generating VarDiscover_summary.stats
     */
    
    writeSummary("VarDiscover_summary.stats", file, stats, o);
    
    /*
     * Generating VarDiscover_detected.csv
     */
    
    writeDetected("VarDiscover_detected.csv", stats, o);
    
    /*
     * Generating VarDiscover_ROC.R
     */
    
    o.generate("VarDiscover_ROC.R");
    o.writer->open("VarDiscover_ROC.R");
    
    if (__countP__ >= __countD__)
    {
        o.info("P-value for scoring");
        o.writer->write(createVCROC("VarDiscover_detected.csv", "1-data$Pval", "-1"));
    }
    else
    {
        o.info("Depth for scoring");
        o.writer->write(createVGROC("VarDiscover_detected.csv", "data$Depth", "'FP'"));
    }
    
    o.writer->close();
    
    if (!r.isGermline())
    {
        /*
         * Generating VarDiscover_allele.R
         */
        
        o.generate("VarDiscover_allele.R");
        o.writer->open("VarDiscover_allele.R");
        o.writer->write(RWriter::createRLinear("VarDiscover_sequins.csv",
                                               o.work,
                                               "Allele Frequency",
                                               "Expected Allele Frequency (log2)",
                                               "Measured Allele Frequency (log2)",
                                               "log2(data$ExpFreq)",
                                               "log2(data$ObsFreq)",
                                               "input",
                                                true));
        o.writer->close();

        /*
         * Generating VarDiscover_LODR.R
         */
        
        o.generate("VarDiscover_LODR.R");
        o.writer->open("VarDiscover_LODR.R");
        o.writer->write(RWriter::createScript("VarDiscover_detected.csv", PlotVLODR()));
        o.writer->close();
    }
}
