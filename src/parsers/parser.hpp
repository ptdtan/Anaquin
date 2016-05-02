#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <stdexcept>

namespace Anaquin
{
    typedef std::string Line;
    
    struct ParserProgress
    {
        long long i = 0;
    };

    struct InvalidFileError : public std::exception
    {
        InvalidFileError(const std::string &file) : file(file) {}

        const std::string file;
    };
    
    struct Parser
    {
        
        protected:
        
//            virtual
        
        //        static void parse(const FileName &file, Functor);
        
        //        static void parse(const std::string &, std::function<void (const TMap &, const ParserProgress &)>);
    };
}

#endif