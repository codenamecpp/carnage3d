#include "stdafx.h"
#include "BroadcastEventsManager.h"
#include "TimeManager.h"
#include "CarnageGame.h"

BroadcastEventsManager gBroadcastEvents;

//////////////////////////////////////////////////////////////////////////

void BroadcastEventsIterator::Reset()
{
    mCurrentIndex = -1;
}

bool BroadcastEventsIterator::NextEvent(eBroadcastEvent eventType, BroadcastEvent& outputEventData)
{
    int currIndex = 0;
    if (mCurrentIndex != -1)
    {
        currIndex = mCurrentIndex;
    }
    const int eventsCount = (int) gBroadcastEvents.mEventsList.size();
    for (;currIndex < eventsCount; ++currIndex)
    {
        const BroadcastEvent& currEvent = gBroadcastEvents.mEventsList[currIndex];
        if (currEvent.mEventType == eventType)
        {
            mCurrentIndex = currIndex;
            outputEventData = currEvent;
            return true;
        }
    }
    mCurrentIndex = eventsCount;
    return false;
}

bool BroadcastEventsIterator::NextEventInDistance(eBroadcastEvent eventType, const glm::vec2& position, float maxDistance, BroadcastEvent& outputEventData)
{
    int currIndex = 0;
    if (mCurrentIndex != -1)
    {
        currIndex = mCurrentIndex;
    }
    const float maxDistance2 = (maxDistance * maxDistance);
    const int eventsCount = (int) gBroadcastEvents.mEventsList.size();
    for (;currIndex < eventsCount; ++currIndex)
    {
        const BroadcastEvent& currEvent = gBroadcastEvents.mEventsList[currIndex];
        if (currEvent.mEventType == eventType)
        {
            float currDistance2 = glm::distance2(position, currEvent.mPosition);
            if (currDistance2 < maxDistance2)
            {
                mCurrentIndex = currIndex;
                outputEventData = currEvent;
                return true;
            }
        }
    }
    mCurrentIndex = eventsCount;
    return false;
}

void BroadcastEventsIterator::DeleteCurrentEvent()
{
    if ((mCurrentIndex == -1) || (mCurrentIndex >= (int)gBroadcastEvents.mEventsList.size()))
    {
        debug_assert(false);
        return;
    }
    gBroadcastEvents.mEventsList[mCurrentIndex].mStatus = BroadcastEvent::Status_CanDelete;
}

//////////////////////////////////////////////////////////////////////////

void BroadcastEventsManager::ClearEvents()
{
    mEventsList.clear();
}

void BroadcastEventsManager::UpdateFrame()
{
    float currentGameTime = gTimeManager.mGameTime;

    // remove obsolete events from list
    for (auto curr_event_iterator = mEventsList.begin(); 
        curr_event_iterator != mEventsList.end(); )
    {
        BroadcastEvent& eventData = *curr_event_iterator;
        if ((eventData.mStatus == BroadcastEvent::Status_Active) && 
            ((eventData.mEventTimestamp + eventData.mEventDurationTime) > currentGameTime))
        {
            ++curr_event_iterator;
            continue;
        }
        // remove expired event
        curr_event_iterator = mEventsList.erase(curr_event_iterator);
    }
}

void BroadcastEventsManager::ReportEvent(eBroadcastEvent eventType, GameObject* subject, Pedestrian* character, float durationTime)
{
    debug_assert(subject);

    if (subject == nullptr)
        return;

    float currentGameTime = gTimeManager.mGameTime;

    const glm::vec2 position2 = subject->mTransform.GetPosition2();
    // check same event is exists
    for (BroadcastEvent& evData: mEventsList)
    {
        if ((evData.mEventType == eventType) && (evData.mSubject == subject) && (evData.mPosition == position2))
        {
            evData.mEventTimestamp = currentGameTime;
            evData.mEventDurationTime = durationTime;
            return;
        }
    }

    mEventsList.emplace_back();
    BroadcastEvent& evData = mEventsList.back();
    // fill event data
    evData.mEventType = eventType;
    evData.mEventTimestamp = currentGameTime;
    evData.mEventDurationTime = durationTime;
    evData.mPosition = position2;
    evData.mSubject = subject;
    evData.mCharacter = character;

    // notify current gamestate controller
    if (gCarnageGame.mCurrentGamestate)
    {
        gCarnageGame.mCurrentGamestate->OnGamestateBroadcastEvent(evData);
    }
}

void BroadcastEventsManager::ReportEvent(eBroadcastEvent eventType, const glm::vec2& position, float durationTime)
{
    float currentGameTime = gTimeManager.mGameTime;

    // update time if same event is exists
    for (BroadcastEvent& evData: mEventsList)
    {
        if ((evData.mEventType == eventType) && (evData.mPosition == position) && (evData.mSubject == nullptr))
        {
            evData.mEventTimestamp = currentGameTime;
            evData.mEventDurationTime = durationTime;
            return;
        }
    }

    mEventsList.emplace_back();
    BroadcastEvent& evData = mEventsList.back();
    // fill event data
    evData.mEventType = eventType;
    evData.mEventTimestamp = currentGameTime;
    evData.mEventDurationTime = durationTime;
    evData.mPosition = position;

    // notify current gamestate controller
    if (gCarnageGame.mCurrentGamestate)
    {
        gCarnageGame.mCurrentGamestate->OnGamestateBroadcastEvent(evData);
    }
}