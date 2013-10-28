#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cstring>
#include <string>
#include <sstream>
#include <boost/regex.hpp>
#include <boost/regex/icu.hpp>
#include <iterator>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <iconv.h>

extern "C" {
    #include "rslpStemmer.h"
}

typedef std::unordered_set<std::string> StringSet;

using namespace boost::posix_time;

int RUN_SIZE = 500000;

size_t BUFFER_SIZE = 512;

void loadVocabulary(StringSet &_vocabulary)
{
    std::ifstream file("voc.txt");
    std::string line;
    if(file.is_open())
    {
        while(file.good())
        {
            std::getline(file, line);
            _vocabulary.insert(line);
        }
    }
    std::cout << "Vocabulary size: " << _vocabulary.size() << std::endl;
}

int main(int /*argc*/, char** /*argv*/) {

    StringSet vocabulary;

    std::ifstream infile("tweets.clear");
    std::ofstream file("tweets_dump");

    char buff[BUFFER_SIZE];

    // regex stuff
    boost::u32regex rmRx = boost::make_u32regex("[[:N*:][:P*:][:Sc:][:Sm:][:So:]]");
    boost::regex rmMention("@[a-zA-Z]{3,}[ $]");
    boost::regex rmLinks("https?://[\\.\\w/?]*");

    boost::u32regex rx = boost::make_u32regex("\\w{3,}");
    boost::match_results<std::string::const_iterator> match;
    std::string::const_iterator start;

    std::vector<std::string> cols;
    std::vector<std::string> words;
    std::stringstream ssData;

    loadVocabulary(vocabulary);
    
    rslpLoadStemmer(&rslpMainStruct, "rslpconfig.txt");

    ptime limit = time_from_string("2013-04-01 00:00:00");
    
    std::cout << "Let the games begin!" << std::endl;
    for(std::string line; getline(infile, line);)
    {
        words.clear();
        cols.clear();
        boost::split(cols, line, boost::is_any_of("\t"));

        ptime creation = time_from_string(cols.at(2));
        if(creation > limit)
            continue;

        std::string content = cols.at(1);
        boost::erase_all_regex(content, rmLinks);
        boost::erase_all_regex(content, rmMention);
        boost::erase_all_regex(content, rmRx);

        start = content.cbegin();
        while(boost::u32regex_search(start, content.cend(), match, rx, boost::match_default))
        {
            std::string found(match[0].first, match[0].second);
            std::transform(found.begin(), found.end(), found.begin(), ::tolower);

            if(vocabulary.find(found) != vocabulary.end())
            {
                bzero(buff, BUFFER_SIZE);
                memcpy(buff, found.c_str(), found.size());

                rslpProcessWord(buff, &rslpMainStruct);
                std::string stemmed(buff);
                words.push_back(stemmed);
            }
            start = match[0].second;
        }

        if(words.size() == 0)
            continue;

        ssData.str(std::string());
        ssData << cols.at(0) << " ";
        ssData << cols.at(2) << " ";
        ssData << cols.at(3) << " ";
        if(cols.at(4) != std::string("\\N"))
            ssData << cols.at(4) << " ";
        else
            ssData << "0 ";

        for(auto word : words)
            ssData << word << " ";

        auto data = ssData.str();
        file << data << std::endl;
    }

    file.close();
    return 0;
}
