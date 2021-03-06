
#include <string>
#include <algorithm>

/*
 * Manuals
 */

#include "resources/anaquin.txt"
#include "resources/VarFlip.txt"
#include "resources/VarAlign.txt"
#include "resources/RnaAlign.txt"
#include "resources/RnaKReport.txt"
#include "resources/VarKReport.txt"
#include "resources/RnaAssembly.txt"
#include "resources/VarDiscover.txt"
#include "resources/RnaSubsample.txt"
#include "resources/RnaExpression.txt"
#include "resources/RnaFoldChange.txt"
#include "resources/VarSubsample.txt"
#include "resources/MetaAlign.txt"
#include "resources/MetaSubsample.txt"
#include "resources/MetaAssembly.txt"
#include "resources/MetaAbund.txt"
#include "resources/MetaFoldChange.txt"

/*
 * Scripts
 */

#include "resources/plotFold.R"
#include "resources/plotLinear.R"
#include "resources/plotLogistic.R"

/*
 * RnaQuin Resources
 */

#include "resources/plotTROC.R"
#include "resources/plotTLODR.R"

#include "resources/A.R.1.gtf"
#include "resources/MRN027_v001.csv"
#include "resources/MRN028_v001.csv"
#include "resources/MRN029_v001.csv"
#include "resources/MRN030_v001.csv"

/*
 * VarQuin Resources
 */

#include "resources/plotVLODR.R"
#include "resources/plotVGROC.R"
#include "resources/plotVCROC.R"

#include "resources/sampled.bed"
#include "resources/AVA033_v001.bed"
#include "resources/AVA009_v001.vcf"
#include "resources/MVA011_v001.csv"
#include "resources/MVA012_v001.csv"

typedef std::string Scripts;

#define ToString(x) std::string(reinterpret_cast<char*>(x))

Scripts Manual()
{
    return ToString(data_manuals_anaquin_txt);
}

Scripts PlotLinear()
{
    return ToString(src_r_plotLinear_R);
}

Scripts PlotFold()
{
    return ToString(src_r_plotFold_R);
}

Scripts PlotLogistic()
{
    return ToString(src_r_plotLogistic_R);
}

/*
 * Manuals
 */

Scripts RnaSubsample()
{
    return ToString(data_manuals_RnaSubsample_txt);
}

Scripts RnaAlign()
{
    return ToString(data_manuals_RnaAlign_txt);
}

Scripts RnaAssembly()
{
    return ToString(data_manuals_RnaAssembly_txt);
}

Scripts RnaReport()
{
    return ToString(data_manuals_RnaKReport_txt);
}

Scripts RnaExpression()
{
    return ToString(data_manuals_RnaExpression_txt);
}

Scripts RnaFoldChange()
{
    return ToString(data_manuals_RnaFoldChange_txt);
}

Scripts VarAlign()
{
    return ToString(data_manuals_VarAlign_txt);
}

Scripts VarFlip()
{
    return ToString(data_manuals_VarFlip_txt);
}

Scripts VarSubsample()
{
    return ToString(data_manuals_VarSubsample_txt);
}

Scripts VarDiscover()
{
    return ToString(data_manuals_VarDiscover_txt);
}

Scripts VarReport()
{
    return ToString(data_manuals_VarKReport_txt);
}

Scripts MetaAlign()
{
    return ToString(data_manuals_MetaAlign_txt);
}

Scripts MetaSubsample()
{
    return ToString(data_manuals_MetaSubsample_txt);
}

Scripts MetaAssembly()
{
    return ToString(data_manuals_MetaAssembly_txt);
}

Scripts MetaAbund()
{
    return ToString(data_manuals_MetaAbund_txt);
}

Scripts MetaFoldChange()
{
    return ToString(data_manuals_MetaFoldChange_txt);
}

/*
 * RnaSeq Resources
 */

Scripts PlotTROC()
{
    return ToString(src_r_plotTROC_R);
}

Scripts PlotTLODR()
{
    return ToString(src_r_plotTLODR_R);
}

Scripts RnaStandGTF()
{
    return ToString(data_RnaQuin_A_R_1_gtf);
}

Scripts RnaDataMixA()
{
    return ToString(data_RnaQuin_MRN027_v001_csv);
}

Scripts RnaDataMixB()
{
    return ToString(data_RnaQuin_MRN028_v001_csv);
}

Scripts RnaDataMixF()
{
    return ToString(data_RnaQuin_MRN030_v001_csv);
}

Scripts RnaDataMixAB()
{
    return ToString(data_RnaQuin_MRN029_v001_csv);
}

/*
 * Variant Resources
 */

Scripts PlotVLODR()
{
    return ToString(src_r_plotVLODR_R);
}

Scripts PlotVGROC()
{
    return ToString(src_r_plotVGROC_R);
}

Scripts PlotVCROC()
{
    return ToString(src_r_plotVCROC_R);
}

Scripts AVA033Bed()
{
    return ToString(data_VarQuin_AVA033_v001_bed);
}

Scripts VarDataMixA()
{
    return ToString(data_VarQuin_MVA011_v001_csv);
}

Scripts VarDataMixF()
{
    return ToString(data_VarQuin_MVA012_v001_csv);
}

Scripts VarDataVCF()
{
    return ToString(data_VarQuin_AVA009_v001_vcf);
}

Scripts VarDataBed()
{
    return ToString(data_VarQuin_sampled_bed);
}
