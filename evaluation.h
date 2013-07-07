#ifndef EVALUATION_H
#define EVALUATION_H

#include <ctime>
#include "utils.h"
#include "userprofile.h"

namespace casimiro {

class Evaluation
{
public:
    Evaluation(PqConnectionPtr _con,
               LongVectorPtr _userIds,
               std::tm _startTraining,
               std::tm _endTraining,
               std::tm _startEvaluation,
               std::tm _endEvaluation);

    virtual ~Evaluation();

private:
    PqConnectionPtr m_con;
    LongVectorPtr m_userIds;
    std::tm m_startTraining;
    std::tm m_endTraining;
    std::tm m_startEvaluation;
    std::tm m_endEvaluation;
    double m_mrr = 0.0;
    double m_successAtK = 0.0;
    
    std::map<int, double> m_bestTimeframe;

    virtual double cosineSimilarity(ConceptMapPtr _profile1, ConceptMapPtr _profile2);
    virtual LongVectorPtr rankCandidates(TweetProfileVectorPtr _candidates, UserProfilePtr _userProfile, std::tm _until);
    virtual LongVectorPtr rankCandidatesByDate(TweetProfileVectorPtr _candidates, std::tm _until);

public:
    virtual void run();
};

}
#endif // EVALUATION_H
