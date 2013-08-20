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

#include <iconv.h>

extern "C" {
    #include "rslpStemmer.h"
}

typedef std::unordered_set<std::string> StringSet;

using namespace boost::posix_time;

int RUN_SIZE = 500000;

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

    iconv_t cd = iconv_open("ISO_8859-1", "UTF-8");
    
    StringSet vocabulary;

    std::ifstream infile("tweets.clear");
    std::ofstream file("tweets_dump");

    char buff[256];
    char* buffPtr = &buff[0];
    size_t buffSize = 0;
    char conv[256];
    char* convPtr = &conv[0];
    size_t convSize = 255;

    // regex stuff
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
        start = content.cbegin();
        while(boost::u32regex_search(start, content.cend(), match, rx, boost::match_default))
        {
            std::string found(match[0].first, match[0].second);
            std::transform(found.begin(), found.end(), found.begin(), ::tolower);
            if(vocabulary.find(found) != vocabulary.end())
            {
                bzero(buff, 256);
                bzero(conv, 256);
                memcpy(buff, found.c_str(), found.size());
                buffSize = found.size();
                convSize = 255;
                buffPtr = &buff[0];
                convPtr = &conv[0];
                
                
                if (iconv(cd, &buffPtr, &buffSize, &convPtr, &convSize) == (size_t) -1)
                {
                    std::cout << "Fodeu! " << found << std::endl;
                    return 1;
                }
                
                rslpProcessWord(conv, &rslpMainStruct);
                
                std::string stemmed(conv);
                words.push_back(std::string(stemmed.begin(), stemmed.end()));
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
