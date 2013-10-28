#include "evaluation.h"
#include <iostream>
#include <algorithm>
#include <fstream>

namespace casimiro {

Evaluation::Evaluation(std::string _connectionString,
                       LongVectorPtr _userIds,
                       ptime _startTraining,
                       ptime _endTraining,
                       ptime _startEvaluation,
                       ptime _endEvaluation,
                       EvaluationType _evaluationType,
                       bool _social):
    m_stringConnection(_connectionString),
    m_userIds(_userIds),
    m_startTraining(_startTraining),
    m_endTraining(_endTraining),
    m_startEvaluation(_startEvaluation),
    m_endEvaluation(_endEvaluation),
    m_evaluationType(_evaluationType),
    m_social(_social)
{
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
    
    ptime from = _until - minutes(60*24*1);
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

    LongVector first50(&rankedCandidates[0], &rankedCandidates[std::min(50, (int)rankedCandidates.size())]);
    return std::make_shared<LongVector>(first50);
}

LongVectorPtr Evaluation::rankCandidatesByDate(TweetProfileVectorPtr _candidates, ptime _until)
{
    LongVector rankedCandidates;

    ptime from = _until - minutes(60*24*1);

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

    LongVector first50(&rankedCandidates[0], &rankedCandidates[50]);
    return std::make_shared<LongVector>(first50);
}

LongVectorPtr Evaluation::rankCandidatesRandomly(TweetProfileVectorPtr _candidates, ptime _until)
{
    LongVector rankedCandidates;
    ptime from = _until - minutes(60*24*1);

    for (auto candidate : *_candidates)
    {
        ptime candidateTime = candidate->getPublishDateTime();

        if(candidateTime < from)
            continue;
        if(candidateTime > _until)
            break;

        rankedCandidates.push_back(candidate->getTweetId());
    }
    std::random_shuffle(rankedCandidates.begin(), rankedCandidates.end());

    LongVector first50(&rankedCandidates[0], &rankedCandidates[50]);
    return std::make_shared<LongVector>(first50);
}

UserProfilePtr Evaluation::getUserProfile(long int _userId, PqConnectionPtr _con)
{
    switch(m_evaluationType)
    {
        case HASHTAG_EVAL:
            return UserProfile::getHashtagProfile(_con, _userId, m_startTraining, m_endTraining, m_social);
        case BOW_EVAL:
            return UserProfile::getBagOfWordsProfile(_con, _userId, m_startTraining, m_endTraining, m_social);
        case TOPIC_EVAL:
            return UserProfile::getTopicsProfile(_con, _userId, m_startTraining, m_endTraining, m_social);
        case RECENCY_EVAL:
        case RANDOM_EVAL:
            // Just return the simplest profile
            //return UserProfile::getBagOfWordsProfile(_con, _userId, m_startTraining, m_endTraining, m_social);
            return UserProfile::getTopicsProfile(_con, _userId, m_startTraining, m_endTraining, m_social);
        default:
            return nullptr;
    }
}


EvaluationResults Evaluation::run()
{
    std::cout << "Start running" << std::endl;
    EvaluationResults results;


    double meanMrr = 0;
    double sAt5 = 0;
    double sAt10 = 0;

    double userMeanMrr;
    double usersAt5;
    double usersAt10;

    double usersEvaluated = 0;
    for (auto userId : *m_userIds)
    {
        auto con = std::make_shared<pqxx::connection>(m_stringConnection);
        try
        {
            auto userProfile = getUserProfile(userId, con);
            auto retweets = userProfile->getRetweets(m_startEvaluation, m_endEvaluation);
            if(retweets->size() == 0)
            {
                std::cout << userId << "," << -1 << "," << -1 << "," << -1 << std::endl;
                continue;
            }

            if(m_evaluationType != RECENCY_EVAL && m_evaluationType != RANDOM_EVAL)
                userProfile->loadProfile();

            ptime startCandidates = retweets->at(0).first - minutes(60*24*1);
            ptime endCandidates = retweets->at(retweets->size()-1).first;
            auto candidateTweets = userProfile->getCandidateTweets(startCandidates, endCandidates);
            userMeanMrr = 0.0;
            usersAt5 = 0.0;
            usersAt10 = 0.0;

            for (auto retweet : *retweets)
            {
                LongVectorPtr ranked = nullptr;
                switch(m_evaluationType)
                {
                    case HASHTAG_EVAL:
                    case TOPIC_EVAL:
                    case BOW_EVAL:
                        ranked = rankCandidates(candidateTweets, userProfile, retweet.first);
                        break;
                    case RECENCY_EVAL:
                        ranked = rankCandidatesByDate(candidateTweets, retweet.first);
                        break;
                    case RANDOM_EVAL:
                        ranked = rankCandidatesRandomly(candidateTweets, retweet.first);
                        break;
                }
                
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

            usersEvaluated++;
        }
        catch(...)
        {
            std::cout << userId << "," << -2 << "," << -2 << "," << -2 << std::endl;
            continue;
        }
    }
    meanMrr = meanMrr / usersEvaluated;
    sAt5 = sAt5 / usersEvaluated;
    sAt10 = sAt10 / usersEvaluated;

    results.setGeneralResult(Result(meanMrr, sAt5, sAt10));

    std::cout << "General Mean MRR: " << meanMrr << std::endl;
    std::cout << "General Mean sAt5: " << sAt5 << std::endl;
    std::cout << "General Mean sAt10: " << sAt10 << std::endl;
    return results;
}



}
