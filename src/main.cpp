#include <map>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <unistd.h>
#include <getopt.h>
#include <execinfo.h>

#include "rna/r_diffs.hpp"
#include "rna/r_align.hpp"
#include "rna/r_abund.hpp"
#include "rna/r_assembly.hpp"

#include "var/v_align.hpp"
#include "var/v_variant.hpp"

#include "meta/m_blast.hpp"
#include "meta/m_diffs.hpp"
#include "meta/m_assembly.hpp"

#include "ladder/l_diffs.hpp"
#include "ladder/l_abund.hpp"

#include "fusion/f_fusion.hpp"

#include "parsers/parser_csv.hpp"
#include "parsers/parser_sequins.hpp"

#include "writers/file_writer.hpp"
#include "writers/terminal_writer.hpp"

#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

typedef int Mode;
typedef int Option;
typedef int Command;

typedef std::string Value;
typedef std::set<Value> Range;

#define CMD_VER    'v'
#define CMD_TEST   't'
#define CMD_RNA    265
#define CMD_VAR    266
#define CMD_META   267
#define CMD_LADDER 268
#define CMD_FUSION 269
#define CMD_FETAL  270
#define CMD_CANCER 271
#define CMD_STRUCT 272
#define CMD_CLINIC 273

#define MODE_BLAST    281
#define MODE_ALIGN    283
#define MODE_ASSEMBLY 284
#define MODE_ABUND    285
#define MODE_DIFFS    286
#define MODE_VARIANT  287
#define MODE_FUSION   290
#define MODE_SEQUINS  291

#define OPT_CMD     320
#define OPT_MIN     321
#define OPT_MAX     322
#define OPT_LOS     323
#define OPT_OUTPUT  324
#define OPT_REF     325
#define OPT_MIXTURE 326
#define OPT_FILTER  327
#define OPT_THREAD  328
#define OPT_MODE    329
#define OPT_PSL_1   330
#define OPT_PSL_2   331

using namespace Anaquin;

// Shared with other modules
std::string __full_command__;

// Shared with other modules
std::string date()
{
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];
    
    time (&rawtime);
    timeinfo = localtime(&rawtime);
    
    strftime(buffer, 80, "%d-%m-%Y %I:%M:%S", timeinfo);
    std::string str(buffer);
    
    return str;
}

/*
 * Defines the possible commands
 */

static std::map<Value, Command> _cmds =
{
    { "rna",    CMD_RNA    },
    { "var",    CMD_VAR    },
    { "meta",   CMD_META   },
    { "ladder", CMD_LADDER },
    { "cancer", CMD_CANCER },
    { "fusion", CMD_FUSION },
    { "clinic", CMD_CLINIC },
};

/*
 * Defines the possible modes
 */

static std::map<Value, Mode> _modes =
{
    { "seqs",     MODE_SEQUINS  },
    { "sequins",  MODE_SEQUINS  },
    { "align",    MODE_ALIGN    },
    { "assembly", MODE_ASSEMBLY },
    { "abund",    MODE_ABUND    },
    { "diffs",    MODE_DIFFS    },
    { "variant",  MODE_VARIANT  },
    { "fusion",   MODE_FUSION   },
    { "blast",    MODE_BLAST    },
};

/*
 * Defines the modes supported for each command
 */

static std::map<Command, std::set<Mode>> _supported =
{
    { CMD_RNA,    std::set<Mode> { MODE_SEQUINS, MODE_ALIGN, MODE_ASSEMBLY, MODE_ABUND, MODE_DIFFS } },
    { CMD_VAR,    std::set<Mode> { MODE_SEQUINS, MODE_ALIGN, MODE_VARIANT,  MODE_ABUND, MODE_DIFFS } },
    { CMD_FUSION, std::set<Mode> { MODE_SEQUINS, MODE_FUSION } },
    { CMD_LADDER, std::set<Mode> { MODE_SEQUINS, MODE_ABUND, MODE_DIFFS } },
    { CMD_META,   std::set<Mode> { MODE_SEQUINS, MODE_BLAST, MODE_DIFFS, MODE_ASSEMBLY } },
};

/*
 * Defines the mode requires a mixture file
 */

static std::map<Mode, bool> _needMix =
{
    { MODE_SEQUINS,  false },
    { MODE_ALIGN,    false },
    { MODE_ASSEMBLY, true  },
    { MODE_ABUND,    true  },
    { MODE_DIFFS,    true  },
    { MODE_FUSION,   true  },
    { MODE_VARIANT,  true  },
    { MODE_BLAST,    true  },
};

/*
 * Defines the mode requires a reference file
 */

static std::map<Mode, bool> _needRef =
{
    { MODE_SEQUINS,  false },
    { MODE_ALIGN,    true  },
    { MODE_ASSEMBLY, true  },
    { MODE_ABUND,    true  },
    { MODE_DIFFS,    true  },
    { MODE_FUSION,   true  },
    { MODE_VARIANT,  true  },
    { MODE_BLAST,    false },
};

/*
 * Variables used in argument parsing
 */

struct Parsing
{
    // The path that output files are written
    std::string output = "output";
    
    // PSL alignment for the mixture A generated by BLAST
    std::string pA;
    
    // PSL alignment for the mixtuee B generated by BLAST
    std::string pB;
    
    // Number of threads
    unsigned threads = 1;
    
    // Custom minmium concentration
    double min = 0;
    
    // Custom maximum concentration
    double max = std::numeric_limits<double>::max();
    
    // Custom sensivitiy
    double los;
    
    // Custom reference file
    std::string ref;
    
    // Custom mixture file
    std::string mix;
    
    // The sequins that have been filtered
    std::set<SequinID> filters;
    
    // One or more operands
    std::vector<std::string> opts;
    
    // How the software is invoked
    std::string invoked;
    
    Mode mode   = 0;
    Command cmd = 0;
};

// Wrap the variables so that it'll be easier to reset them
static Parsing _p;

template<typename T> std::string concat(const std::map<Value, T> &m)
{
    std::string str;
    
    for (const auto i : m)
    {
        str += i.first + "|";
    }
    
    return str.substr(0, str.length() - 2);
}

// Return a string representation for the commands
static std::string cmdRange()
{
    return concat(_cmds);
}

struct InvalidCommandException : public std::exception
{
    InvalidCommandException(const std::string &data) : data(data) {}

    // The exact meaning is context-specific
    std::string data;
};

struct InvalidOptionException : public std::exception
{
    InvalidOptionException(const std::string &opt) : opt(opt) {}
    
    std::string opt;
};

/*
 * The type of the argument is invalid, the expected type is integer. For example, giving
 * "ABCD" as the number of threads.
 */

struct InvalidIntegerError : public InvalidCommandException
{
    InvalidIntegerError(const std::string &arg) : InvalidCommandException(arg) {}
};

// An option is being given more than once
struct RepeatOptionError : public InvalidCommandException
{
    RepeatOptionError(const std::string &opt) : InvalidCommandException(opt) {}
};

/*
 * The option value isn't one of the expected. The most common scenario is failing
 * to specifying an expected command.
 */

struct InvalidValueError : public std::exception
{
    InvalidValueError(const Value &value, const std::string &range) : value(value), range(range) {}

    const Value value;
    const std::string range;
};

// A mandatory option is missing, for instance, failing to specify the command
struct MissingOptionError : public std::exception
{
    MissingOptionError(const std::string &opt) : opt(opt) {}
    MissingOptionError(const std::string &opt, const std::string &range) : opt(opt), range(range) {}

    // Option that is missing
    const std::string opt;
    
    // Possible values for the missing option
    const std::string range;
};

struct MissingMixtureError   : public std::exception {};
struct MissingReferenceError : public std::exception {};
struct MissingInputError     : public std::exception {};
struct InvalidModeError      : public std::exception {};

struct InvalidInputCountError : std::exception
{
    InvalidInputCountError(std::size_t expected, std::size_t actual) : expected(expected), actual(actual) {}
    
    // Number of inputs detected
    std::size_t actual;
    
    // Number of inputs expected
    std::size_t expected;
};

struct TooLessInputError : std::exception
{
    TooLessInputError(std::size_t n) : n(n) {}
    
    // Number of inputs expected
    std::size_t n;
};

struct TooManyOptionsError : public std::runtime_error
{
    TooManyOptionsError(const std::string &msg) : std::runtime_error(msg) {}
};

struct InvalidFilterError : public std::runtime_error
{
    InvalidFilterError(const std::string &msg) : std::runtime_error(msg) {}
};

/*
 * Argument options
 */

static const char *short_options = "";

static const struct option long_options[] =
{
    { "v", no_argument, 0, CMD_VER  },
    { "t", no_argument, 0, CMD_TEST },

    { "c", required_argument, 0, OPT_CMD },

    { "min",     required_argument, 0, OPT_MIN },
    { "max",     required_argument, 0, OPT_MAX },

    { "p1",      required_argument, 0, OPT_PSL_1 },
    { "p2",      required_argument, 0, OPT_PSL_2 },

    { "los",     required_argument, 0, OPT_LOS },

    { "r",       required_argument, 0, OPT_REF },
    { "ref",     required_argument, 0, OPT_REF },

    { "t",       required_argument, 0, OPT_THREAD },
    { "threads", required_argument, 0, OPT_THREAD },

    { "m",       required_argument, 0, OPT_MIXTURE },
    { "mix",     required_argument, 0, OPT_MIXTURE },
    { "mixture", required_argument, 0, OPT_MIXTURE },

    { "p",       required_argument, 0, OPT_MODE },

    { "o",       required_argument, 0, OPT_OUTPUT  },
    { "output",  required_argument, 0, OPT_OUTPUT  },

    { "f",        required_argument, 0, OPT_FILTER },
    { "filter",   required_argument, 0, OPT_FILTER },

    { "blast", required_argument, 0, MODE_BLAST },

    {0, 0, 0, 0 }
};

static void printUsage()
{
    extern std::string Manual();
    std::cout << Manual() << std::endl;
}

static void printVersion()
{
    std::cout << "Anaquin v1.0.00" << std::endl;
}

// Print a file of mixture A and B
static void print(Reader &r)
{
    /*
     * Format: <ID, Mix A, Mix B>
     */

    std::string l;
    
    // Skip the first line
    r.nextLine(l);

    std::cout << "ID\tMix A\tMix B" << std::endl;

    while (r.nextLine(l))
    {
        if (l == "\r" || l == "\n" || l == "\r\n")
        {
            continue;
        }

        std::vector<std::string> tokens;
        Tokens::split(l, "\t", tokens);

        std::cout << tokens[0] << "\t" << tokens[2] << "\t" << tokens[3] << std::endl;
    }
}

static void printMixture()
{
    Reader r(_p.mix);
    print(r);
}

template <typename Mixture> void applyMix(Mixture mix)
{
    if (_p.mix.empty())
    {
#ifndef DEBUG
        if (_needMix.at(_p.mode))
        {
            throw MissingMixtureError();
        }
#endif
    }
    else
    {
        std::cout << "[INFO]: Mixture: " << _p.mix << std::endl;
        mix(Reader(_p.mix));
    }
}

template <typename Reference> void applyRef(Reference ref)
{
    if (_p.ref.empty())
    {
#ifndef DEBUG
        if (_needRef.at(_p.mode))
        {
            throw MissingReferenceError();
        }
#endif
    }
    else
    {
        std::cout << "[INFO]: Reference: " << _p.ref << std::endl;
        ref(Reader(_p.ref));
    }
}

// Read sequins from a file, one per line. The identifiers must match.
static void readFilters(const std::string &file)
{
    Reader r(file);
    std::string line;
    
    // We'll use it to compare the sequins
    const auto &s = Standard::instance();

    while (r.nextLine(line))
    {
        switch (_p.cmd)
        {
            case CMD_FUSION: { break; }
            case CMD_LADDER: { break; }

            case CMD_RNA:
            {
                assert(s.r_seqs_A.size() == s.r_seqs_B.size());

                if (!s.r_seqs_A.count(line))
                {
                    throw InvalidFilterError("Unknown sequin for RNA: " + line);
                }
                
                _p.filters.insert(line);
                break;
            }

            case CMD_VAR:
            {
                assert(s.d_seqs_A.size() == s.d_seqs_B.size());
                
                if (!s.d_seqs_A.count(line))
                {
                    throw InvalidFilterError("Unknown sequin for DNA: " + line);
                }
                
                _p.filters.insert(line);
                break;
            }

            case CMD_META:
            {
                assert(s.m_seqs_A.size() == s.m_seqs_B.size());

                if (!s.m_seqs_A.count(line))
                {
                    throw InvalidFilterError("Unknown sequin for metagenomics: " + line);
                }

                _p.filters.insert(line);
                break;
            }

            default: { assert(false); }
        }
    }

    if (_p.filters.empty())
    {
        throw InvalidFilterError("No sequin found in: " + file);
    }
}

template <typename Analyzer, typename F> void analyzeF(F f, typename Analyzer::Options o)
{
    const auto path = _p.output;

    // This might be needed while scripting
    __full_command__ = _p.invoked;

#ifndef DEBUG
    o.writer = std::shared_ptr<FileWriter>(new FileWriter(path));
    o.logger = std::shared_ptr<FileWriter>(new FileWriter(path));
    o.output = std::shared_ptr<TerminalWriter>(new TerminalWriter());
    o.logger->open("anaquin.log");
#endif

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    o.info(_p.invoked);
    o.info(date());
    o.info("Path: " + path + "\n");

    for (const auto &filter : (o.filters = _p.filters))
    {
        std::cout << "Filter: " << filter << std::endl;
    }

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "------------- Sequin Analysis -----------" << std::endl;
    std::cout << "-----------------------------------------" << std::endl << std::endl;
    
    std::clock_t begin = std::clock();
    
    f(o);
    
    std::clock_t end = std::clock();
    
    const auto elapsed = (boost::format("Completed. Elpased %1% seconds")
                                        % (double(end - begin) / CLOCKS_PER_SEC)).str();
    o.info(elapsed);

#ifndef DEBUG
    o.logger->close();
#endif
}

// Analyze for a single-sample input
template <typename Analyzer> void analyze_1(typename Analyzer::Options o = typename Analyzer::Options())
{
    if (_p.opts.size() != 1)
    {
        throw InvalidInputCountError(1, _p.opts.size());
    }

    return analyzeF<Analyzer>([&](const typename Analyzer::Options &o) { Analyzer::analyze(_p.opts[0], o); }, o);
}

// Analyze for two-samples input
template < typename Analyzer> void analyze_2(typename Analyzer::Options o = typename Analyzer::Options())
{
    if (_p.opts.size() != 2)
    {
        throw InvalidInputCountError(2, _p.opts.size());
    }
    
    return analyzeF<Analyzer>([&](const typename Analyzer::Options &o) { Analyzer::analyze(_p.opts[0], _p.opts[1], o); }, o);
}

template <typename Options> static Options detect(const std::string &file)
{
    const bool found_gene = file.find("gene") != std::string::npos;
    const bool found_isoform = file.find("isoform") != std::string::npos;

    Options o;
    
    if (found_gene && !found_isoform)
    {
        std::cout << "[INFO]: Gene tracking assumed" << std::endl;
        o.level = RNALevel::Gene;
    }
    else if (!found_gene && found_isoform)
    {
        std::cout << "[INFO]: Isoform tracking assumed" << std::endl;
        o.level = RNALevel::Isoform;
    }
    else
    {
        throw std::runtime_error("Invalid file. Only genes.fpkm_tracking or isoforms.fpkm_tracking are accepted.");
    }

    return o;
}

void parse(int argc, char ** argv)
{
    auto &cmd  = _p.cmd;
    auto &mode = _p.mode;
    
    _p = Parsing();

    if (argc <= 1)
    {
        printUsage();
    }

    int next, index;

#ifdef UNIT_TESTING
    optind = optreset = 1;
#endif

    /*
     * Reconstruct the overall command
     */
    
    for (int i = 0; i < argc; i++)
    {
        _p.invoked += std::string(argv[i]) + " ";
    }

    assert(!_p.invoked.empty());

    // Attempt to parse and store a floating point from string
    auto parseDouble = [&](const std::string &str, double &r)
    {
        assert(next);
        
        try
        {
            r = stof(str);
        }
        catch (...)
        {
            throw std::runtime_error("ddddd");
        }
    };
    
    // Attempt to parse and store an integer from string
    auto parseInt = [&](const std::string &str, unsigned &r)
    {
        assert(next);
        
        try
        {
            r = stoi(str);
        }
        catch (...)
        {
            throw std::runtime_error("eeee");
        }
    };
    
    auto checkFile = [&](const std::string &file)
    {
        if (!std::ifstream(file).good())
        {
            throw InvalidFileError(file);
        }
    };

    /*
     * Pre-process arguments. This way, we can examine the options in whatever order we'd like to
     */

    std::vector<Option> opts;
    std::vector<Value>  vals;

    int n = 1;

    while ((next = getopt_long_only(argc, argv, short_options, long_options, &index)) != -1)
    {
        opts.push_back(next);

        // Whether this option has an value
        const bool hasValue = optarg;
        
        vals.push_back(hasValue ? std::string(optarg) : "");

        // We'll need it to parse the inputs
        n += (hasValue ? 2 : 1);
    }

    /*
     * Here, we move the command option to the front. Therefore, we also check
     * if we've at least specified the command.
     */
    
    // Find the index for the command
    auto iter = std::find(opts.begin(), opts.end(), OPT_CMD);

    if (iter == opts.end() &&
       (iter  = std::find(opts.begin(), opts.end(), CMD_VER))  == opts.end() &&
       (iter  = std::find(opts.begin(), opts.end(), CMD_TEST)) == opts.end())
    {
        throw MissingOptionError("-c", cmdRange());
    }

    // This is the index that we'll need to swap
    const auto i = std::distance(opts.begin(), iter);

    std::swap(opts[0], opts[i]);
    std::swap(vals[0], vals[i]);

    /*
     * Parse the inputs, could be a single input or multiple inputs
     */
    
    for (; n < argc; n++)
    {
        std::string s = argv[n];
        _p.opts.push_back(argv[n]);
    }
    
    for (std::size_t i = 0; i < opts.size(); i++)
    {
        const auto opt = opts[i];
        const auto val = vals[i];

        switch (opt)
        {
            case CMD_VER:
            case CMD_TEST:
            {
                _p.cmd = opt;
                
                if (argc != 2)
                {
                    switch (opt)
                    {
                        case CMD_VER:  { throw TooManyOptionsError("Too many options given for -v"); }
                        case CMD_TEST: { throw TooManyOptionsError("Too many options given for -t"); }
                    }
                }

                break;
            }

            case OPT_REF:     { checkFile(_p.ref = val);   break; }
            case OPT_MIXTURE: { checkFile(_p.mix = val);   break; }
            case OPT_OUTPUT:  { _p.output = val;           break; }
            case OPT_FILTER:  { readFilters(val);          break; }
            case OPT_MAX:     { parseDouble(val, _p.max);  break; }
            case OPT_MIN:     { parseDouble(val, _p.min);  break; }
            case OPT_LOS:     { parseDouble(val, _p.los);  break; }
            case OPT_THREAD:  { parseInt(val, _p.threads); break; }
            case OPT_PSL_1:   { checkFile(_p.pA = val);    break; }
            case OPT_PSL_2:   { checkFile(_p.pB = val);    break; }

            case OPT_CMD:
            {
                if (!_cmds.count(val))
                {
                    throw MissingOptionError("-c", cmdRange());
                }

                // We'll work with it's integer representation
                _p.cmd = _cmds.at(val);

                break;
            }

            case OPT_MODE:
            {
                if (_p.mode)
                {
                    throw RepeatOptionError("-p");
                }
                // Either there's no such mode, or it's not supported by the command
                else if (!_modes.count(val) || !_supported.at(cmd).count(_p.mode = _modes.at(val)))
                {
                    throw InvalidModeError();
                }

                break;
            }

            default:
            {
                throw InvalidOptionException(argv[index]);
            }
        }
    }
    
    // Exception should've already been thrown if command is not specified
    assert(_p.cmd);

    if (_p.cmd != CMD_TEST && _p.cmd != CMD_VER)
    {
        if (_p.opts.empty() && _p.mode != MODE_SEQUINS)
        {
            throw MissingInputError();
        }
        else if (!_p.mode)
        {
            throw MissingOptionError("-p");
        }
    }

    auto &s = Standard::instance();
    
    switch (_p.cmd)
    {
        case CMD_VER:  { printVersion();                break; }
        case CMD_TEST: { Catch::Session().run(1, argv); break; }

        case CMD_CANCER:
        {
            std::cout << "[INFO]: Cancer Analysis" << std::endl;
            break;
        }
            
        case CMD_CLINIC:
        {
            std::cout << "[INFO]: Clinic Analysis" << std::endl;
            break;
        }
            
        case CMD_FUSION:
        {
            std::cout << "[INFO]: Fusion Analysis" << std::endl;

            applyRef(std::bind(&Standard::f_ref, &s, std::placeholders::_1));
            applyMix(std::bind(&Standard::f_mix, &s, std::placeholders::_1));
            
            switch (mode)
            {
                case MODE_SEQUINS: { printMixture();       break; }
                case MODE_FUSION:  { analyze_1<FFusion>(); break; }
            }

            break;
        }

        case CMD_LADDER:
        {
            std::cout << "[INFO]: Ladder Analysis" << std::endl;

            applyMix(std::bind(&Standard::l_mix, &s, std::placeholders::_1));

            switch (mode)
            {
                case MODE_SEQUINS: { printMixture();      break; }
                case MODE_ABUND:   { analyze_1<LAbund>(); break; }
                case MODE_DIFFS:   { analyze_2<LDiffs>(); break; }
            }

            break;
        }

        case CMD_RNA:
        {
            std::cout << "[INFO]: RNA Analysis" << std::endl;
            
            applyRef(std::bind(&Standard::r_ref, &s, std::placeholders::_1));
            applyMix(std::bind(&Standard::r_mix, &s, std::placeholders::_1));

            switch (mode)
            {
                case MODE_SEQUINS:  { printMixture();         break; }
                case MODE_ALIGN:    { analyze_1<RAlign>();    break; }
                case MODE_ASSEMBLY: { analyze_1<RAssembly>(); break; }
                case MODE_ABUND:
                {
                    analyze_1<RAbund>(detect<RAbund::Options>(_p.opts[0]));
                    break;
                }
                case MODE_DIFFS:
                {
                    analyze_1<RDiffs>(detect<RDiffs::Options>(_p.opts[0]));
                    break;
                }
            }

            break;
        }
            
        case CMD_VAR:
        {
            std::cout << "[INFO]: Variant Analysis" << std::endl;
            
            applyRef(std::bind(&Standard::v_ref, &s, std::placeholders::_1));
            applyMix(std::bind(&Standard::v_mix, &s, std::placeholders::_1));
            
            switch (mode)
            {
                case MODE_SEQUINS: { printMixture();        break; }
                case MODE_ALIGN:   { analyze_1<VAlign>();   break; }
                case MODE_VARIANT: { analyze_1<VVariant>(); break; }
            }

            break;
        }
            
        case CMD_META:
        {
            std::cout << "[INFO]: Metagenomics Analysis" << std::endl;
            
            applyRef(std::bind(&Standard::m_ref, &s, std::placeholders::_1));
            applyMix(std::bind(&Standard::m_mix, &s, std::placeholders::_1));
            
            switch (mode)
            {
                case MODE_SEQUINS: { printMixture();      break; }
                case MODE_BLAST:   { analyze_1<MBlast>(); break; }
                case MODE_DIFFS:
                {
                    if (_p.pA.empty () || _p.pB.empty())
                    {
                        throw MissingOptionError("Alignment");
                    }
                    
                    MDiffs::Options o;
                    
                    o.pA = _p.pA;
                    o.pB = _p.pB;
                    
                    analyze_2<MDiffs>(o);
                    break;
                }

                case MODE_ASSEMBLY:
                {
                    MAssembly::Options o;
                    
                    // We'd also take an alignment PSL file from a user
                    o.psl = _p.pA;

                    analyze_1<MAssembly>(o);
                    break;
                }
            }

            break;
        }

        default: { assert(false); }
    }
}

int parse_options(int argc, char ** argv)
{
    auto printError = [&](const std::string &file)
    {
        std::cerr << std::endl;
        std::cerr << "*********************************************************************" << std::endl;
        std::cerr << file << std::endl;
        std::cerr << "*********************************************************************" << std::endl;
    };
    
    try
    {
        parse(argc, argv);
        return 0;
    }
    catch (const InvalidModeError &ex)
    {
        std::stringstream ss;
        
        const auto &modes = _supported.at(_p.cmd);
        
        for (const auto &mode : modes)
        {
            for (const auto &i : _modes)
            {
                if (i.second == mode)
                {
                    ss << i.first + "|";
                    break;
                }
            }
        }
        
        auto str = ss.str();
        
        // Remove the last "|"
        str = str.substr(0, str.length() - 2);

        for (const auto i : _cmds)
        {
            if (i.second == _p.cmd)
            {
                const auto format = "Invalid mode for %1%. Possibles are: %2%.";
                printError((boost::format(format) % i.first % str).str());
            }
        }
    }
    catch (const InvalidOptionException &ex)
    {
        const auto format = "Unknown option: %1%";
        printError((boost::format(format) % ex.opt).str());
    }
    catch (const InvalidValueError &ex)
    {
        printError(ex.value);
    }
    catch (const MissingOptionError &ex)
    {
        if (!ex.range.empty())
        {
            const auto format = "A mandatory option is missing. Please specify %1%. Possibles are %2%";
            printError((boost::format(format) % ex.opt % ex.range).str());
        }
        else
        {
            const auto format = "A mandatory option is missing. Please specify %1%.";
            printError((boost::format(format) % ex.opt).str());
        }
    }
    catch (const RepeatOptionError &ex)
    {
        printError((boost::format("The option %1% has been repeated. Please check and try again.") % ex.data).str());
    }
    catch (const MissingInputError &ex)
    {
        printError("No input file given. Please give an input file and try again.");
    }
    catch (const MissingMixtureError &ex)
    {
        printError("Mixture file is missing. Please specify it with -m.");
    }
    catch (const MissingReferenceError &ex)
    {
        printError("Reference file is missing. Please specify it with -r.");
    }
    catch (const InvalidFileError &ex)
    {
        printError((boost::format("%1%%2%") % "Invalid file: " % ex.file).str());
    }
    catch (const InvalidFilterError &ex)
    {
        printError((boost::format("%1%%2%") % "Invalid filter: " % ex.what()).str());
    }
    catch (const std::runtime_error &ex)
    {
        printError(ex.what());
    }

    return 1;
}

int main(int argc, char ** argv)
{
    return parse_options(argc, argv);
}