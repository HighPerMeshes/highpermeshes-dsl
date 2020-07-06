#ifndef DATA_PARSER_HPP
#define DATA_PARSER_HPP

#include "entry.hpp"

#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>

#include <optional>
#include <sstream>
#include <type_traits>

template <typename Iterator>
struct entry_parser : boost::spirit::qi::grammar<Iterator, std::vector<entry>(), boost::spirit::ascii::space_type>
{

    struct error_handler
    {
        template <typename Args, typename Context, typename Skipper>
        void operator()(Args&& args, Context&& /* context */, Skipper&& /* skipper */) const
        {
            using namespace boost::fusion;
            std::stringstream stream;
            stream  << "Error! Expecting " << at_c<3>(args) << "\n"
                    << "here: \"" << std::string { at_c<2>(args), at_c<1>(args) } << '"' << "\n";
            throw std::logic_error(stream.str());
        }
    };

    entry_parser() : entry_parser::base_type(all_entries)
    {

        using namespace boost;
        using namespace spirit::qi;
        using namespace boost::spirit::qi::labels;

        index %= "index:" > int_;
        time_step %= "time_step:" > int_;
        dof %= "Dof:" > int_;
        values %= "Value:" > +(double_);
        end_of_input %= eoi;
        
        one_entry %= '{' > index > time_step > dof > values > '}';
        all_entries %= expect[+one_entry > end_of_input];

        all_entries.name("all entries");
        one_entry.name("one entry");
        end_of_input.name("end of input");

        on_error<fail>(
            all_entries,
            error_handler {}
        );
    }

    boost::spirit::qi::rule<Iterator, int(), boost::spirit::ascii::space_type> index;
    boost::spirit::qi::rule<Iterator, int(), boost::spirit::ascii::space_type> time_step;
    boost::spirit::qi::rule<Iterator, int(), boost::spirit::ascii::space_type> dof;
    boost::spirit::qi::rule<Iterator, std::vector<double>(), boost::spirit::ascii::space_type> values;
    boost::spirit::qi::rule<Iterator, void(), boost::spirit::ascii::space_type> end_of_input;

    boost::spirit::qi::rule<Iterator, entry(), boost::spirit::ascii::space_type> one_entry;
    boost::spirit::qi::rule<Iterator, std::vector<entry>(), boost::spirit::ascii::space_type> all_entries;

    template <typename BeginIter, typename EndIter>
    std::vector<entry> parse_entries(BeginIter&& begin, EndIter&& end)
    {

        static_assert(std::is_same_v<std::decay_t<BeginIter>, Iterator> && std::is_same_v<std::decay_t<EndIter>, Iterator>);

        std::vector<entry> all_entries;
        phrase_parse(std::forward<BeginIter>(begin), std::forward<EndIter>(end), *this, boost::spirit::ascii::space, all_entries);
        return all_entries;
    }
};

#endif /* DATA_PARSER_HPP */
