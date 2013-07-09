#ifndef EVALUATION_H
#define EVALUATION_H

#include <ctime>
#include "utils.h"
#include "userprofile.h"

namespace casimiro {

struct Result {
    float mrr;
    float sAt5;

    Result():
        mrr(0.0),
        sAt5(0.0)
    {
    }

    Result(float _mrr, float _sAt5):
        mrr(_mrr),
        sAt5(_sAt5)
    {
    }

};

typedef std::map<long, Result> ResultMap;

class EvaluationResults
{
public:
    EvaluationResults() {};
    virtual ~EvaluationResults() {};

    void setUserResult(long _userId, Result _userResult)
    {
        m_resultMap[_userId] = _userResult;
    }

    ResultMap resultMap()
    {
        return m_resultMap;
    }

    Result generalResult()
    {
        return m_generalResult;
    }

    void setGeneralResult(Result _generalResult)
    {
        m_generalResult = _generalResult;
    }

private:
    ResultMap m_resultMap;
    Result m_generalResult;

};

class Evaluation
{
public:
    Evaluation(PqConnectionPtr _con);
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
    virtual EvaluationResults run(LongVectorPtr _userIds,
                     std::tm _startTraining,
                     std::tm _endTraining,
                     std::tm _startEvaluation,
                     std::tm _endEvaluation);
};

}
#endif // EVALUATION_H
