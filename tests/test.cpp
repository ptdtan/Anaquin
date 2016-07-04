#include <vector>
#include <string>
#include <sstream>
#include "unit/test.hpp"
#include "data/standard.hpp"
#include <boost/algorithm/string.hpp>

// Defined in main.cpp
extern int parse_options(int argc, char ** argv);

extern std::string LadderDataMixA();
extern std::string LadderDataMixB();
extern std::string LadderDataMixAB();

extern std::string FusionDataMixA();
extern std::string FusionDataRef();

extern std::string TransDataMixA();
extern std::string TransDataMixB();
extern std::string TransDataMixF();
extern std::string TransDataMixAB();
extern std::string TransStandGTF();

extern std::string MetaDataBed();
extern std::string MetaDataMix();

extern std::string VarDataBed();
extern std::string VarDataVCF();
extern std::string VarDataMixA();
extern std::string VarDataMixF();

using namespace Anaquin;

void Test::clear()
{
    Standard::instance(true);
}

void Test::fusionA()
{
    Test::clear();
    Standard::instance().addFRef(Reader(FusionDataRef(), DataMode::String));
    Standard::instance().addFMix(Reader(FusionDataMixA(), DataMode::String));
    Standard::instance().r_fus.finalize();
}

void Test::meta()
{
    Test::clear();
    Standard::instance().addMMix(Reader(MetaDataMix(), DataMode::String));
    Standard::instance().r_meta.finalize();
}

void Test::ladderA()
{
    Test::clear();
    Standard::instance().addLMix(Reader(LadderDataMixA(), DataMode::String));
    Standard::instance().r_lad.finalize();
}

void Test::transA()
{
    Test::clear();
    Standard::instance().addTRef(Reader(TransStandGTF(), DataMode::String));
    Standard::instance().addTMix(Reader(TransDataMixA(), DataMode::String));
    Standard::instance().r_trans.finalize();
}

void Test::transB()
{
    Test::clear();
    Standard::instance().addTRef(Reader(TransStandGTF(), DataMode::String));
    Standard::instance().addTMix(Reader(TransDataMixB(), DataMode::String));
    Standard::instance().r_trans.finalize();
}

void Test::RnaFoldChange()
{
    Test::clear();
    Standard::instance().addTDMix(Reader(TransDataMixAB(), DataMode::String));
    Standard::instance().r_trans.finalize();
}

void Test::VarQuinBed()
{
    Test::clear();
    Standard::instance().addVStd(Reader(VarDataBed(),  DataMode::String));
    Standard::instance().r_var.finalize();
}

void Test::variantA()
{
    Test::clear();
    Standard::instance().addVVar(Reader(VarDataVCF(),  DataMode::String));
    Standard::instance().addVStd(Reader(VarDataBed(),  DataMode::String));
    Standard::instance().addVMix(Reader(VarDataMixA(), DataMode::String));
    Standard::instance().r_var.finalize();
}

void Test::variantF()
{
    Test::clear();
    Standard::instance().addVVar(Reader(VarDataVCF(),  DataMode::String));
    Standard::instance().addVStd(Reader(VarDataBed(),  DataMode::String));
    Standard::instance().addVMix(Reader(VarDataMixF(), DataMode::String));
    Standard::instance().r_var.finalize();
}

Test Test::test(const std::string &command)
{
    std::vector<std::string> tokens;
    boost::split(tokens, command, boost::is_any_of(" "));

    char *argv[tokens.size() + 1];

    argv[0] = new char(10);
    strcpy(argv[0], "anaquin");

    for (std::size_t i = 0; i < tokens.size(); i++)
    {
        argv[i+1] = new char[tokens[i].size() + 1];
        strcpy(argv[i+1], tokens[i].c_str());
    }
    
    std::stringstream outputBuffer, errorBuffer;

    std::streambuf * const _errorBuffer  = std::cerr.rdbuf(errorBuffer.rdbuf());
    std::streambuf * const _outputBuffer = std::cout.rdbuf(outputBuffer.rdbuf());

    Test t;

    t.status = parse_options(static_cast<int>(tokens.size()) + 1, argv);
    t.error  = errorBuffer.str();
    t.output = outputBuffer.str();

    std::cout.rdbuf(_errorBuffer);
    std::cout.rdbuf(_outputBuffer);

    for (std::size_t i = 0; i < tokens.size(); i++)
    {
        delete[] argv[i];
    }
    
    // Clear off anything made by a unit-test
    Standard::instance(true);

    return t;
}