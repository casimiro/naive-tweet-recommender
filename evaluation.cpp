#include "evaluation.h"
#include <iostream>

namespace casimiro {

Evaluation::Evaluation(LongVectorPtr _userIds,
                       QDateTime _startTraining,
                       QDateTime _endTraining,
                       QDateTime _startEvaluation,
                       QDateTime _endEvaluation):
    m_userIds(_userIds),
    m_startTraining(_startTraining),
    m_endTraining(_endTraining),
    m_startEvaluation(_startEvaluation),
    m_endEvaluation(_endEvaluation)
{
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
                                         QDateTime _until)
{
    std::map<double, std::vector<long>> aux;
    auto end = aux.end();

    double sim;
    for (auto candidate : *_candidates)
    {
        if(candidate->getPublishDateTime() > _until)
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

void Evaluation::run()
{
    double meanMrr = 0;
    double mrr;
    for (auto userId : *m_userIds)
    {
        auto userProfile = UserProfile::getHashtagProfile(userId, m_startTraining, m_endTraining);
        auto candidateTweets = userProfile->getCandidateTweets(m_startEvaluation, m_endEvaluation);
        auto retweets = userProfile->getRetweets(m_startEvaluation, m_endEvaluation);

        for (auto retweet : *retweets)
        {
            auto ranked = rankCandidates(candidateTweets, userProfile, retweet.first);
            auto found = std::find(ranked->begin(), ranked->end(), retweet.second);
            size_t index = std::distance(ranked->begin(), found);
            
            if(index == ranked->size())
                mrr = 0.0;
            else
                mrr = 1 / (index + 1);

            meanMrr += mrr;
        }
    }
    meanMrr = meanMrr / m_userIds->size();
    std::cout << "Mean MRR: " << meanMrr << std::endl;
}

}
