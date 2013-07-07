#include "evaluation.h"
#include <iostream>
#include <algorithm>
#include <fstream>

namespace casimiro {

Evaluation::Evaluation(PqConnectionPtr _con,
                       LongVectorPtr _userIds,
                       std::tm _startTraining,
                       std::tm _endTraining,
                       std::tm _startEvaluation,
                       std::tm _endEvaluation):
    m_con(_con),
    m_userIds(_userIds),
    m_startTraining(_startTraining),
    m_endTraining(_endTraining),
    m_startEvaluation(_startEvaluation),
    m_endEvaluation(_endEvaluation)
{
    std::ifstream users("users_timeframe");
    std::string line;
    while(std::getline(users, line))
    {
        size_t commaPos = line.find(",");
        int userId = atoi(line.substr(0, commaPos).c_str());
        double timeFrame = atof(line.substr(commaPos+1, line.size() - commaPos).c_str());
        m_bestTimeframe[userId] = timeFrame;
    }
        
}

Evaluation::~Evaluation()
{
}

double Evaluation::cosineSimilarity(ConceptMapPtr _profile1, ConceptMapPtr _profile2)
{
    float aNorm,bNorm,dot;
    aNorm = bNorm = dot = 0;

    auto uIt = _profile1->begin();
    auto nIt = _profile2->begin();

    auto uEnd = _profile1->end();
    auto nEnd = _profile2->end();

    int compare;
    while (uIt != uEnd && nIt != nEnd)
    {
        compare = uIt->first.compare(nIt->first);
        if(compare == 0)
        {
            dot += uIt->second * nIt->second;
            uIt++;
            nIt++;
        }
        else if(compare < 0)
            uIt++;
        else if(compare > 0)
            nIt++;
    }

    for (uIt = _profile1->begin(); uIt != uEnd; uIt++)
        aNorm += pow(uIt->second, 2);
    aNorm = sqrt(aNorm);

    for (nIt = _profile2->begin(); nIt != nEnd; nIt++)
        bNorm += pow(nIt->second, 2);
    bNorm = sqrt(bNorm);

    return dot / (aNorm * bNorm);
}

LongVectorPtr Evaluation::rankCandidates(TweetProfileVectorPtr _candidates,
                                         UserProfilePtr _userProfile,
                                         std::tm _until)
{
    std::map<double, std::vector<long>> aux;
    auto end = aux.end();
    
    std::tm from = _until;
    from.tm_sec -= m_bestTimeframe[_userProfile->getUserId()];

    double sim;
    for (auto candidate : *_candidates)
    {
        std::tm candidateTime = candidate->getPublishDateTime();

        if(mktime(&candidateTime) < mktime(&from))
            continue;
        if(mktime(&candidateTime) > mktime(&_until))
            break;

        sim = cosineSimilarity(_userProfile->getProfile(), candidate->getProfile());

        if(aux.find(sim) == end)
            aux[sim] = std::vector<long>();

        aux[sim].push_back(candidate->getTweetId());
    }

    LongVector rankedCandidates;
    for (auto pair : aux)
        for (auto tweetId : pair.second)
            rankedCandidates.push_back(tweetId);

    return std::make_shared<LongVector>(rankedCandidates);
}

LongVectorPtr Evaluation::rankCandidatesByDate(TweetProfileVectorPtr _candidates, std::tm _until)
{
    LongVector rankedCandidates;

    std::tm from = _until;
    from.tm_mday -= 1;

    for (auto candidate : *_candidates)
    {
        std::tm candidateTime = candidate->getPublishDateTime();

        if(mktime(&candidateTime) < mktime(&from))
            continue;
        if(mktime(&candidateTime) > mktime(&_until))
            break;

        rankedCandidates.push_back(candidate->getTweetId());
    }
    
    std::reverse(rankedCandidates.begin(), rankedCandidates.end());
    return std::make_shared<LongVector>(rankedCandidates);
}


void Evaluation::run()
{
    std::cout << "Start running" << std::endl;
    double meanMrr = 0;
    double userMeanMrr;
    double mrr;
    for (auto userId : *m_userIds)
    {
        try
        {
            auto userProfile = UserProfile::getHashtagProfile(m_con, userId, m_startTraining, m_endTraining, true);
            auto retweets = userProfile->getRetweets(m_startEvaluation, m_endEvaluation);
            if(retweets->size() == 0)
            {
                std::cout << userId << "," << -1 << std::endl;
                continue;
            }

            auto candidateTweets = userProfile->getCandidateTweets(m_startEvaluation, m_endEvaluation);
            userMeanMrr = 0.0;

            for (auto retweet : *retweets)
            {
                auto ranked = rankCandidates(candidateTweets, userProfile, retweet.first);
                auto found = std::find(ranked->begin(), ranked->end(), retweet.second);
                double index = (double) std::distance(ranked->begin(), found);

                if(index == ranked->size())
                    mrr = 0.0;
                else
                    mrr = 1.0 / (index + 1.0);

                userMeanMrr += mrr;
            }
            userMeanMrr = userMeanMrr / retweets->size();
            std::cout << userId << "," << userMeanMrr << std::endl;
            meanMrr += userMeanMrr;
        }
        catch(...)
        {
            std::cout << userId << "," << -2 << std::endl;
            continue;
        }
    }
    meanMrr = meanMrr / m_userIds->size();
    std::cout << "General Mean MRR: " << meanMrr << std::endl;
}

}
