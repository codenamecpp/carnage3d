#pragma once

#include "GameDefs.h"
#include "GameObject.h"
#include "Pedestrian.h"

//////////////////////////////////////////////////////////////////////////

// When something is happening on game map broadcast event gets fired up
enum eBroadcastEvent
{
    eBroadcastEvent_GunShot,
    eBroadcastEvent_PedestrianInjured,
    eBroadcastEvent_PedestrianDead,
    eBroadcastEvent_StealCar,
    eBroadcastEvent_Explosion,
    eBroadcastEvent_CarBurns,

    eBroadcastEvent_StartDriveCar,
    eBroadcastEvent_StopDriveCar,
};

decl_enum_strings(eBroadcastEvent);

//////////////////////////////////////////////////////////////////////////

// Broadcast event data
struct BroadcastEvent
{
public:
    eBroadcastEvent mEventType;
    enum Status
    {
        Status_Active, // event is ok
        Status_CanDelete, // event will be deleted at end of current frame
    };
    Status mStatus = Status_Active;

    float mEventTimestamp; // time when event was created
    float mEventDurationTime; // how long event lives, seconds

    glm::vec2 mPosition; // position in the world where event did happen, meters

    GameObjectHandle mSubject; // object that was affected
    PedestrianHandle mCharacter; // character which causes event, afflictor
};

//////////////////////////////////////////////////////////////////////////

class BroadcastEventsIterator
{
public:
    BroadcastEventsIterator() = default;
    void Reset();
    bool NextEvent(eBroadcastEvent eventType, BroadcastEvent& outputEventData);
    bool NextEventInDistance(eBroadcastEvent eventType, const glm::vec2& position, float maxDistance, BroadcastEvent& outputEventData);
    void DeleteCurrentEvent();
private:
    int mCurrentIndex = -1;
};

//////////////////////////////////////////////////////////////////////////

// Broadcast events manager
class BroadcastEventsManager final: public cxx::noncopyable
{
    friend class BroadcastEventsIterator;
public:
    BroadcastEventsManager() = default;
    void ClearEvents();
    void UpdateFrame();
    // broadcasting event
    void ReportEvent(eBroadcastEvent eventType, GameObject* subject, Pedestrian* character, float durationTime);
    void ReportEvent(eBroadcastEvent eventType, const glm::vec2& position, float durationTime);
private:
    std::vector<BroadcastEvent> mEventsList;
};

extern BroadcastEventsManager gBroadcastEvents;