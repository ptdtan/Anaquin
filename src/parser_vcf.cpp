#include <map>
#include <assert.h>
#include "file.hpp"
#include "tokens.hpp"
#include "parser_vcf.hpp"

/*
 * Refer to http://samtools.github.io/hts-specs/VCFv4.1.pdf for more details
 */

enum VCFField
{
    CHROMO,
    POS,
    ID,
    REF,
    ALT,
    QUAL,
    FILTER,
    INFO,
    FORMAT,
    FORMAT_DATA_1
};

static std::map<std::string, AlleleType> alleleParser =
{
    { "0/0", HomozygousRef } , { "1/1", HomozygousAlt } , { "0/1", Heterzygous }
};

void ParserVCF::parse(const std::string &file, VCFVariantF fv)
{
    std::string line;
    File f(file);

    VCFVariant v;

    std::vector<std::string> t1;
    std::vector<std::string> t2;
    std::vector<std::string> t3;

    while (f.nextLine(line))
    {
		if (line.empty() || line[0] == '#')
		{
			continue;
		}
        
        Tokens::split(line, "\t", t1);

        v.chromoID = t1[CHROMO];
        v.pos = stod(t1[POS]);
        v.varID = t1[ID];
        v.ref = t1[REF];

        /*
         * Example:
         *
         *     G,T
         */
        
        v.alts.clear();
        Tokens::split(t1[ALT], ",", v.alts);

        /*
         * Example:
         *
         *     GT:AD:DP:GQ:PL
         */
        
        t2.clear();
        Tokens::split(t1[FORMAT], ":", t2);
        
        /*
         * Example:
         *
         *     1/1:0,128:128:99:5206,436,0
         */
        
        t3.clear();
        Tokens::split(t1[FORMAT_DATA_1], ":", t3);

        assert(t2.size() == t3.size());
        
        for (auto i = 0; i < t2.size(); i++)
        {
            if (t2[i] == "GT")
            {
                v.type = alleleParser.at(t3[i]);
            }
        }
        
        fv(v);
	}
}