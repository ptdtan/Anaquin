#include <limits>
#include <iostream>
#ifdef UNIT_TESTING
#include "gtest/gtest.h"
#endif
#include "AlignerAnalyst.hpp"
#include "AssemblyAnalyst.hpp"
//#include <boost/program_options/parsers.hpp>
//#include <boost/program_options/variables_map.hpp>
//#include <boost/program_options/options_description.hpp>

int main(int argc, char ** argv)
{
	std::cout << "Hello World" << std::endl;
	return 0;
/*
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");

    desc.add_options()
        ("align,a", "Assess alignment performance. Requires SAM or BAM input file generated by alignment programs.")
        ("test,t", "Execute internal tests")
        ("version,v", "Show the version number")
    ;

    po::variables_map vm;
    
    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("version"))
        {
            std::cout << "Basic Command Line Parameter App" << std::endl;
        }
        else if (vm.count("test"))
        {
#ifdef UNIT_TESTING
            ::testing::InitGoogleTest(&argc, argv);
            return RUN_ALL_TESTS();
#endif
        }
        else if (vm.count("align"))
        {
            AlignerAnalyst::base(argv[2], Sequins(), 1000);
        }
        
        po::notify(vm);
    }
    catch (po::error &ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    return 0;
*/
}
