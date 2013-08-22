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

LongVectorPtr Evaluation::rankCandidates(TweetProfileVectorPtr _candidates,
                                         UserProfilePtr _userProfile,
                                         ptime _until)
{
    std::map<double, std::vector<long>> aux;
    auto end = aux.end();
    
    //ptime from = _until- seconds((int)m_bestTimeframe[_userProfile->getUserId()]);
    ptime from = _until - minutes(60*24*5);
    double sim;
    for (auto candidate : *_candidates)
    {
        ptime candidateTime = candidate->getPublishDateTime();

        if(candidateTime < from)
            continue;
        if(candidateTime > _until)
            break;

        sim = _userProfile->cosineSimilarity(candidate->getProfile());

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

LongVectorPtr Evaluation::rankCandidatesByDate(TweetProfileVectorPtr _candidates, UserProfilePtr _userProfile, ptime _until)
{
    LongVector rankedCandidates;

    //ptime from = _until- seconds((int)m_bestTimeframe[_userProfile->getUserId()]);
    ptime from = _until - minutes(60*24*5);

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
    //std::random_shuffle(rankedCandidates.begin(), rankedCandidates.end());
    return std::make_shared<LongVector>(rankedCandidates);
}


EvaluationResults Evaluation::run(LongVectorPtr _userIds,
                     ptime _startTraining,
                     ptime _endTraining,
                     ptime _startEvaluation,
                     ptime _endEvaluation,
                     EvaluationType _evaluationType)
{
    std::cout << "Start running" << std::endl;
    EvaluationResults results;

    //ptime startCandidates = _startEvaluation - minutes(60*24*5);
    ptime startCandidates = _startEvaluation;

    double meanMrr = 0;
    double sAt5 = 0;
    double sAt10 = 0;

    double userMeanMrr;
    double usersAt5;
    double usersAt10;
    for (auto userId : *_userIds)
    {
        //auto con = std::make_shared<pqxx::connection>("postgresql://tweetsbr:zxc123@localhost:5432/tweetsbr2");
        auto con = m_con;
        try
        {
            auto userProfile = UserProfile::getHashtagProfile(con, userId, _startTraining, _endTraining, false);
            auto retweets = userProfile->getRetweets(_startEvaluation, _endEvaluation);
            if(retweets->size() == 0)
            {
                std::cout << userId << "," << -1 << "," << -1 << "," << -1 << std::endl;
                continue;
            }

            if(_evaluationType != RECENCY_EVAL)
                userProfile->loadProfile();

            auto candidateTweets = userProfile->getCandidateTweets(startCandidates, _endEvaluation);
            userMeanMrr = 0.0;
            usersAt5 = 0.0;
            usersAt10 = 0.0;

            for (auto retweet : *retweets)
            {
                auto ranked = rankCandidates(candidateTweets, userProfile, retweet.first);
                auto found = std::find(ranked->begin(), ranked->end(), retweet.second);
                double index = (double) std::distance(ranked->begin(), found);

                if(index < ranked->size())
                {
                    userMeanMrr += 1.0 / (index + 1.0);

                    if(index < 10)
                        usersAt10 += 1;

                    if(index < 5)
                        usersAt5 += 1;
                }

            }
            userMeanMrr = userMeanMrr / (double)retweets->size();
            usersAt5 = usersAt5 / (double)retweets->size();
            usersAt10 = usersAt10 / (double)retweets->size();

            std::cout << userId << "," << userMeanMrr << "," << usersAt5 << "," << usersAt10 << std::endl;
            results.setUserResult(userId, Result(userMeanMrr, usersAt5, usersAt10));
            meanMrr += userMeanMrr;
            sAt5 += usersAt5;
            sAt10 += usersAt10;
        }
        catch(...)
        {
            std::cout << userId << "," << -2 << "," << -2 << "," << -2 << std::endl;
            continue;
        }
    }
    meanMrr = meanMrr / (double)_userIds->size();
    sAt5 = sAt5 / (double)_userIds->size();
    sAt10 = sAt10 / (double)_userIds->size();

    results.setGeneralResult(Result(meanMrr, sAt5, sAt10));

    std::cout << "General Mean MRR: " << meanMrr << std::endl;
    std::cout << "General Mean sAt5: " << sAt5 << std::endl;
    std::cout << "General Mean sAt10: " << sAt10 << std::endl;
    return results;
}



}
