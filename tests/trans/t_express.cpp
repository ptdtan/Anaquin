#include <catch.hpp>
#include "unit/test.hpp"
#include "trans/t_express.hpp"

using namespace Anaquin;

TEST_CASE("TExpress_T_1000_Isoforms")
{
    Test::transA();
    
    TExpress::Options o;
    o.level = Anaquin::TExpress::RNALevel::Isoform;
    
    const auto r  = TExpress::report("tests/data/T_1000/B/G/isoforms.fpkm_tracking", o);
    const auto lm = r.linear();
    
    REQUIRE(lm.r  == Approx(0.5340634212));
    REQUIRE(lm.m  == Approx(0.4511315372));
    REQUIRE(lm.r2 == Approx(0.2852237379));
    
    REQUIRE(r.ss.id == "R2_38_1");
    REQUIRE(r.ss.counts == 1);
    REQUIRE(r.ss.abund == Approx(0.0000047454));
}

TEST_CASE("TExpress_T_1000_Genes")
{
    Test::transA();
    
    const auto r1 = Test::test("-t TransExpress -m data/trans/MTR002.v013.csv -rgtf data/trans/ATR001.v032.gtf -ugtrack tests/data/T_1000/B/G/genes.fpkm_tracking");
    
    REQUIRE(r1.status == 0);
    
    Test::transA();
    
    TExpress::Options o;
    o.level = Anaquin::TExpress::RNALevel::Gene;
    
    const auto r2 = TExpress::report("tests/data/T_1000/B/G/genes.fpkm_tracking", o);
    const auto lm = r2.linear();
    
    REQUIRE(lm.r  == Approx(0.6294136279));
    REQUIRE(lm.m  == Approx(0.5927763272));
    REQUIRE(lm.r2 == Approx(0.396161515));
    
    REQUIRE(r2.ss.id == "R2_53");
    REQUIRE(r2.ss.counts == 1);
    REQUIRE(r2.ss.abund == Approx(0.000066038));
}