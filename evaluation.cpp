#include "evaluation.h"
#include <iostream>
#include <algorithm>
#include <fstream>

namespace casimiro {

Evaluation::Evaluation(PqConnectionPtr _con):
    m_con(_con)
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

    auto uIt = _profile1->cbegin();
    auto nIt = _profile2->cbegin();

    auto uEnd = _profile1->cend();
    auto nEnd = _profile2->cend();

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
                                         ptime _until)
{
    std::map<double, std::vector<long>> aux;
    auto end = aux.end();
    
    ptime from = _until- seconds((int)m_bestTimeframe[_userProfile->getUserId()]);


    double sim;
    for (auto candidate : *_candidates)
    {
        ptime candidateTime = candidate->getPublishDateTime();

        //if(candidateTime < from)
        //    continue;
        if(candidateTime > _until)
            break;

        sim = cosineSimilarity(_userProfile->getProfile(), candidate->getProfile());

        if(aux.find(sim) == end)
            aux[sim] = std::vector<long>();

        aux[sim].push_back(candidate->getTweetId());
    }

    LongVector rankedCandidates;
    for (auto it = aux.rbegin(); it != aux.crend(); it++)
        for (auto tweetId : it->second)
            rankedCandidates.push_back(tweetId);

    return std::make_shared<LongVector>(rankedCandidates);
}

LongVectorPtr Evaluation::rankCandidatesByDate(TweetProfileVectorPtr _candidates, ptime _until)
{
    LongVector rankedCandidates;

    ptime from = _until - seconds(3600*24);

    for (auto candidate : *_candidates)
    {
        ptime candidateTime = candidate->getPublishDateTime();

        if(candidateTime < from)
            continue;
        if(candidateTime > _until)
            break;

        rankedCandidates.push_back(candidate->getTweetId());
    }
    
    std::reverse(rankedCandidates.begin(), rankedCandidates.end());
    return std::make_shared<LongVector>(rankedCandidates);
}


EvaluationResults Evaluation::run(LongVectorPtr _userIds,
                     ptime _startTraining,
                     ptime _endTraining,
                     ptime _startEvaluation,
                     ptime _endEvaluation)
{
    std::cout << "Start running" << std::endl;
    EvaluationResults results;
    double meanMrr = 0;
    double userMeanMrr;
    double mrr;
    for (auto userId : *_userIds)
    {
        //auto con = std::make_shared<pqxx::connection>("postgresql://tweetsbr:zxc123@localhost:5432/tweetsbr2");
        try
        {
            auto userProfile = UserProfile::getHashtagProfile(m_con, userId, _startTraining, _endTraining, false);
            auto retweets = userProfile->getRetweets(_startEvaluation, _endEvaluation);
            if(retweets->size() == 0)
            {
                std::cout << userId << "," << -1 << std::endl;
                continue;
            }

            userProfile->loadProfile();

            auto candidateTweets = userProfile->getCandidateTweets(_startEvaluation, _endEvaluation);
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
            userMeanMrr = userMeanMrr / (double)retweets->size();
            std::cout << userId << "," << userMeanMrr << std::endl;
            results.setUserResult(userId, Result(userMeanMrr, 0));
            meanMrr += userMeanMrr;
        }
        catch(...)
        {
            std::cout << userId << "," << -2 << std::endl;
            continue;
        }
    }
    meanMrr = meanMrr / _userIds->size();
    results.setGeneralResult(Result(meanMrr,0.0));

    std::cout << "General Mean MRR: " << meanMrr << std::endl;

    return results;
}



}
