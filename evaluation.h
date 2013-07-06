#ifndef EVALUATION_H
#define EVALUATION_H

#include <QDateTime>
#include "utils.h"
#include "userprofile.h"

namespace casimiro {

class Evaluation
{
public:
    Evaluation(LongVectorPtr _userIds,
               QDateTime _startTraining,
               QDateTime _endTraining,
               QDateTime _startEvaluation,
               QDateTime _endEvaluation);

    virtual ~Evaluation();

private:
    LongVectorPtr m_userIds;
    QDateTime m_startTraining;
    QDateTime m_endTraining;
    QDateTime m_startEvaluation;
    QDateTime m_endEvaluation;
    double m_mrr = 0.0;
    double m_successAtK = 0.0;
    
    std::map<int, double> m_bestTimeframe;

    virtual double cosineSimilarity(ConceptMapPtr _profile1, ConceptMapPtr _profile2);
    virtual LongVectorPtr rankCandidates(TweetProfileVectorPtr _candidates, UserProfilePtr _userProfile, QDateTime _until);
    virtual LongVectorPtr rankCandidatesByDate(TweetProfileVectorPtr _candidates, QDateTime _until);

public:
    virtual void run();
};

}
#endif // EVALUATION_H
