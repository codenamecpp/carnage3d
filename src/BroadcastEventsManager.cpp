#include "stdafx.h"
#include "BroadcastEventsManager.h"
#include "TimeManager.h"

BroadcastEventsManager gBroadcastEvents;

BroadcastEventsManager::BroadcastEventsManager()
{
}

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
        if ((eventData.mEventTimestamp + eventData.mEventDurationTime) > currentGameTime)
        {
            ++curr_event_iterator;
            continue;
        }
        // remove expired event
        curr_event_iterator = mEventsList.erase(curr_event_iterator);
    }
}

void BroadcastEventsManager::RegisterEvent(eBroadcastEvent eventType, GameObject* subject, Pedestrian* character, float durationTime)
{
    debug_assert(subject);

    if (subject == nullptr)
        return;

    float currentGameTime = gTimeManager.mGameTime;

    eBroadcastEventSubject subjectType = eBroadcastEventSubject_Object;
    if (subject->IsVehicleClass())
    {
        subjectType = eBroadcastEventSubject_Vehicle;
    }
    if (subject->IsPedestrianClass())
    {
        subjectType = eBroadcastEventSubject_Pedestrian;
    }

    // update time and location if same event is exists
    for (BroadcastEvent& evData: mEventsList)
    {
        if ((evData.mEventType == eventType) &&
            (evData.mEventSubject == subjectType) &&
            (evData.mSubject == subject))
        {
            evData.mEventTimestamp = currentGameTime;
            evData.mEventDurationTime = durationTime;
            evData.mPosition = subject->mTransform.GetPosition2();
            return;
        }
    }

    mEventsList.emplace_back();
    BroadcastEvent& evData = mEventsList.back();
    // fill event data
    evData.mEventType = eventType;
    evData.mEventSubject = subjectType;
    evData.mEventTimestamp = currentGameTime;
    evData.mEventDurationTime = durationTime;
    evData.mPosition = subject->mTransform.GetPosition2();
    evData.mSubject = subject;
    evData.mCharacter = character;
}

void BroadcastEventsManager::RegisterEvent(eBroadcastEvent eventType, const glm::vec2& position, float durationTime)
{
    float currentGameTime = gTimeManager.mGameTime;

    // update time if same event is exists
    for (BroadcastEvent& evData: mEventsList)
    {
        if ((evData.mEventType == eventType) &&
            (evData.mEventSubject == eBroadcastEventSubject_None) &&
            (evData.mPosition == position))
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
    evData.mEventSubject = eBroadcastEventSubject_None;
    evData.mEventTimestamp = currentGameTime;
    evData.mEventDurationTime = durationTime;
    evData.mPosition = position;
}

bool BroadcastEventsManager::PeekEvent(eBroadcastEvent eventType, BroadcastEvent& outputEventData) const
{
    for (const BroadcastEvent& currEvent: mEventsList)
    {
        if (currEvent.mEventType == eventType)
        {
            outputEventData = currEvent;
            return true;
        }
    }
    return false;
}

bool BroadcastEventsManager::PeekClosestEvent(eBroadcastEvent eventType, const glm::vec2& position, BroadcastEvent& outputEventData) const
{
    float closestDistance2 = 0.0f;
    size_t counter = 0;
    size_t bestIndex = 0;
    for (size_t i = 0, Count = mEventsList.size(); i < Count; ++i)
    {
        const BroadcastEvent& currEvent = mEventsList[i];
        if (currEvent.mEventType == eventType)
        {
            ++counter;

            float currDistance2 = glm::distance2(position, currEvent.mPosition);
            if (i == 0) // very first element if closest by default
            {
                closestDistance2 = currDistance2;
                continue;
            }

            if (currDistance2 < closestDistance2)
            {
                closestDistance2 = currDistance2;
                bestIndex = i;
            }
        }
    }
    if (counter)
    {
        outputEventData = mEventsList[bestIndex];
        return true;
    }
    return false;
}

bool BroadcastEventsManager::GetEvent(eBroadcastEvent eventType, BroadcastEvent& outputEventData)
{
    for (auto curr_event_iterator = mEventsList.begin(); 
        curr_event_iterator != mEventsList.end(); ++curr_event_iterator)
    {
        BroadcastEvent& eventData = *curr_event_iterator;
        if (eventData.mEventType == eventType)
        {
            outputEventData = eventData;

            // remove element
            mEventsList.erase(curr_event_iterator);
            return true;
        }
    }
    return false;
}

bool BroadcastEventsManager::GetClosestEvent(eBroadcastEvent eventType, const glm::vec2& position, BroadcastEvent& outputEventData)
{
    float closestDistance2 = 0.0f;
    size_t counter = 0;
    size_t bestIndex = 0;
    for (size_t i = 0, Count = mEventsList.size(); i < Count; ++i)
    {
        const BroadcastEvent& currEvent = mEventsList[i];
        if (currEvent.mEventType == eventType)
        {
            ++counter;

            float currDistance2 = glm::distance2(position, currEvent.mPosition);
            if (i == 0) // very first element if closest by default
            {
                closestDistance2 = currDistance2;
                continue;
            }

            if (currDistance2 < closestDistance2)
            {
                closestDistance2 = currDistance2;
                bestIndex = i;
            }
        }
    }
    if (counter)
    {
        outputEventData = mEventsList[bestIndex];

        // remove element
        mEventsList.erase(mEventsList.begin() + bestIndex);
        return true;
    }
    return false;
}