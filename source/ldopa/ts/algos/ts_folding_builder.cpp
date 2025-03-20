#include "xi/ldopa/ts/algos/ts_folding_builder.h"

namespace xi { namespace ldopa { namespace ts {
//==============================================================================
// class TsFoldBuilder
//==============================================================================



TsFoldBuilder::TsFoldBuilder(IEventLog* log, ITsStateFunc* sf, IStateIDsPool* stIDsPool)
    : _log(log)
    , _sf(sf)
    , _stIDsPool(stIDsPool)
    , _ts(nullptr)
    , _tracesNum(0)
    , _progrCB(nullptr)
    //, _logActivities(nullptr)
{
}

//------------------------------------------------------------------------------

TsFoldBuilder::~TsFoldBuilder()
{
    // если TS не была отдетачена, значит тот билдер управляет временем жизни ее
    cleanTS();
}

//------------------------------------------------------------------------------


TsFoldBuilder::TS* TsFoldBuilder::build()
{
    cleanTS();

    // проверяем корректность входных данных
    if (!_log)
        throw LdopaException("Can't build a TS: no event log is set.");
    if (!_sf)
        throw LdopaException("Can't build a TS: no state function is set.");
    if (!_stIDsPool)
        throw LdopaException("Can't build a TS: no state IDs pool is set.");
   

    // подготовим функцию
    _sf->reset();

    XI_LDOPA_ELAPSEDTIME_START(timer)
    _ts = new TS(_stIDsPool);


    _tracesNum = _log->getTracesNum();
    _ts->setTracesNum(_tracesNum);              // сохраняем инф. о числе трасс в TS!

    if (_progrCB)                               // если есть коллбекер прогресса
        _progrCB->progress(0);
    _lastProgress = 0;                          // обнуляем прогресс

    // перебираем множество всех трасс     
    for (int i = 0; i < _tracesNum; ++i)
    {
        IEventTrace* tr = _log->getTrace(i);
        processTrace(tr);
        //processTrace(i);

         
        if (_progrCB)
        {
            // считаем текущий прогресс в процентах и все уведомления шлем только,
            // если он изменился. иначе, как показали эксперименты, из-за очень
            // частых вызовов существенно (порядки) теряем в производительности. Возможно
            // это из-за маршаллинга в C++/CLI
            int curPr = (int)((double)i / (double)_tracesNum * 100.);
            if (curPr != _lastProgress)
            {
                _lastProgress = curPr;
                // уведомляем по числу трасс и смотрим, не надо ли прервать
                IProgressCallback::Res res = _progrCB->progress(curPr);
                if (res == IProgressCallback::ResCancel)
                    throw LdopaException("Interrupted at the request of the caller.");
            }
        }
    }


    if (_progrCB)                               // если есть коллбекер прогресса
        _progrCB->progress(100);

    XI_LDOPA_ELAPSEDTIME_STOP(timer)

    return _ts;
}


//------------------------------------------------------------------------------

void TsFoldBuilder::processTrace(IEventTrace* tr)
{
    // пред состояние: начинаем с начального
    TS::State prevState = _ts->getInitState();

    int traceSize = tr->getSize();   // _log->getTraceSize(traceNum);

    // перебираем все события в трассе
    // здесь нумеруем с 1, подразумевая, что это +1-событие будет в невходящей
    // части полуотрезка [eventNum - ws, eventNum), т.е. по сути это некоторая
    // оптимизация вместо прибавления 1-цы в вызове метода makeState()
    for (int evNum = 1; evNum <= traceSize; ++evNum)
    {
        IEventLog::Attribute actAttr;
        const IStateId* stID = _sf->makeState(tr, evNum, actAttr);
        if (!stID)
            throw LdopaException("Error when creating a state ID in processTrace().");

        // получаем соответствующее состояние и между состояниями навешиваем транзицию
        TS::State curState = _ts->getOrAddState(stID);
        _ts->getOrAddTransPV(prevState, curState, actAttr, 1);

        prevState = curState;
    }
}

//------------------------------------------------------------------------------

void TsFoldBuilder::processTrace(int traceNum)
{
    // пред состояние: начинаем с начального
    TS::State prevState = _ts->getInitState();

    int traceSize = _log->getTraceSize(traceNum);

    // перебираем все события в трассе
    // здесь нумеруем с 1, подразумевая, что это +1-событие будет в невходящей
    // части полуотрезка [eventNum - ws, eventNum), т.е. по сути это некоторая
    // оптимизация вместо прибавления 1-цы в вызове метода makeState()
    for (int evNum = 1; evNum <= traceSize; ++evNum)
    {
        IEventLog::Attribute actAttr;
        const IStateId* stID = _sf->makeState(traceNum, evNum, actAttr);
        if (!stID)
            throw LdopaException("Error when creating a state ID in processTrace().");

        // получаем соответствующее состояние
        TS::State curState = _ts->getOrAddState(stID);

        // между состояниями навешиваем транзицию
        _ts->getOrAddTransPV(prevState, curState, actAttr, 1);
        prevState = curState;

    }
}


//------------------------------------------------------------------------------

void TsFoldBuilder::cleanTS()
{
    if (_ts)        
    {
        delete _ts;
        _ts = nullptr;
    }
}

//------------------------------------------------------------------------------

TsFoldBuilder::TS* TsFoldBuilder::detach()
{
    if (!_ts)
        return nullptr;
    
    TS* ts = _ts;
    _ts = nullptr;

    return ts;
}

}}} // namespace xi { namespace ldopa { namespace ts {
