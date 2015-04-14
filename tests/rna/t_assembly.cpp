#include <catch.hpp>
#include "assembly.hpp"

using namespace Spike;

TEST_CASE("Assembly_RNA_Simulation_2")
{
    const auto r = Assembly::analyze("tests/data/rna_sims_2/assembly/transcripts.gtf");

    REQUIRE(r.m_trans.tp == 62);
    REQUIRE(r.m_trans.tn == 0);
    REQUIRE(r.m_trans.fp == 0);
    REQUIRE(r.m_trans.fn == 0);
    REQUIRE(r.m_trans.sp() == 1.0);
}