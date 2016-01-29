#include <string>

/*
 * Anaquin R-bioconductor
 */

#include "resources/plotMA.R"
#include "resources/plotROC.R"
#include "resources/plotLODR.R"
#include "resources/plotSplice.R"
#include "resources/plotScatter.R"
#include "resources/plotScatterPool.R"

/*
 * Scripts
 */

#include "resources/viewer.py"
#include "resources/manual.txt"

/*
 * Fusion Resources
 */

#include "resources/AFU004.v032.ref"
#include "resources/AFU005.v032.bed"
#include "resources/MFU007.v013.csv"

/*
 * Ladder Resources
 */

#include "resources/MLA014.v013.csv"
#include "resources/MLA016.v013.csv"
#include "resources/MLA020.v013.csv"

/*
 * Transcriptome Resources
 */

#include "resources/ATR001.v032.gtf"
#include "resources/MTR002.v013.csv"
#include "resources/MTR003.v013.csv"
#include "resources/MTR004.v013.csv"
#include "resources/MTR005.v013.csv"

/*
 * Metagenomics Resources
 */

#include "resources/MME023.v013.csv"
#include "resources/AME015.v032.bed"

/*
 * Variant Resources
 */

#include "resources/AVA009.v032.vcf"
#include "resources/MVA011.v013.csv"
#include "resources/MVA012.v013.csv"
#include "resources/AVA008.v032.bed"

#define ToString(x) std::string(reinterpret_cast<char*>(x))

typedef std::string Scripts;

std::string ViewerScript()
{
    return ToString(scripts_viewer_py);
}

std::string Manual()
{
    return ToString(data_manual_txt);
}

/*
 * Scripts for Anaquin R-Bioconductor
 */

Scripts PlotScatterPool()
{
    return ToString(src_r_plotScatterPool_R);
}

Scripts PlotSplice()
{
    return ToString(src_r_plotSplice_R);
}

Scripts PlotScatter()
{
    return ToString(src_r_plotScatter_R);
}

Scripts PlotMA()
{
    return ToString(src_r_plotMA_R);
}

Scripts PlotROC()
{
    return ToString(src_r_plotROC_R);
}

Scripts PlotLODR()
{
    return ToString(src_r_plotLODR_R);
}

/*
 * Fusion Resources
 */

std::string FusionDataMixA()
{
    return ToString(data_fusion_MFU007_v013_csv);
}

std::string FusionDataRef()
{
    return ToString(data_fusion_AFU004_v032_ref);
}

/*
 * Ladder Resources
 */

std::string LadderDataMixA()
{
    return ToString(data_ladder_MLA014_v013_csv);
}

std::string LadderDataMixB()
{
    return ToString(data_ladder_MLA016_v013_csv);
}

std::string LadderDataMixAB()
{
    return ToString(data_ladder_MLA020_v013_csv);
}

/*
 * Metagenomics Resources
 */

std::string MetaDataMix()
{
    return ToString(data_meta_MME023_v013_csv);
}

std::string MetaDataBed()
{
    return ToString(data_meta_AME015_v032_bed);
}

/*
 * Transcriptome Resources
 */

std::string TransStandGTF()
{
    return ToString(data_trans_ATR001_v032_gtf);
}

std::string TransDataMixA()
{
    return ToString(data_trans_MTR002_v013_csv);
}

std::string TransDataMixB()
{
    return ToString(data_trans_MTR003_v013_csv);
}

std::string TransDataMixF()
{
    return ToString(data_trans_MTR005_v013_csv);
}

std::string TransDataMixAB()
{
    return ToString(data_trans_MTR004_v013_csv);
}

/*
 * Variant Resources
 */

std::string VarDataMixA()
{
    return ToString(data_var_MVA011_v013_csv);
}

std::string VarDataMixF()
{
    return ToString(data_var_MVA012_v013_csv);
}

std::string VarDataVCF()
{
    return ToString(data_var_AVA009_v032_vcf);
}

std::string VarDataBed()
{
    return ToString(data_var_AVA008_v032_bed);
}