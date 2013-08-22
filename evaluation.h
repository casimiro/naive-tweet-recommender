#ifndef EVALUATION_H
#define EVALUATION_H

#include <boost/date_time/posix_time/posix_time.hpp>
#include "utils.h"
#include "userprofile.h"

namespace casimiro {

using namespace boost::posix_time;

enum EvaluationType {
    RECENCY_EVAL,
    HASHTAG_EVAL,
    TOPIC_EVAL
};

struct Result {
    float mrr;
    float sAt5;
    float sAt10;

    Result():
        mrr(0.0),
        sAt5(0.0),
        sAt10(0.0)
    {
    }

    Result(float _mrr, float _sAt5, float _sAt10):
        mrr(_mrr),
        sAt5(_sAt5),
        sAt10(_sAt10)
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
    ptime m_startTraining;
    ptime m_endTraining;
    ptime m_startEvaluation;
    ptime m_endEvaluation;
    double m_mrr = 0.0;
    double m_successAtK = 0.0;
    
    std::map<int, double> m_bestTimeframe;

    virtual LongVectorPtr rankCandidates(TweetProfileVectorPtr _candidates, UserProfilePtr _userProfile, ptime _until);
    virtual LongVectorPtr rankCandidatesByDate(TweetProfileVectorPtr _candidates, UserProfilePtr _userProfile, ptime _until);

public:
    virtual EvaluationResults run(LongVectorPtr _userIds,
                     ptime _startTraining,
                     ptime _endTraining,
                     ptime _startEvaluation,
                     ptime _endEvaluation,
                     EvaluationType _evaluationType);
};

}
#endif // EVALUATION_H
