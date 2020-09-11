#pragma once

#include "GameDefs.h"
#include "GameObject.h"
#include "Pedestrian.h"

// When something is happening on game map broadcast event gets fired up
enum eBroadcastEvent
{
    eBroadcastEvent_GunShot,
    eBroadcastEvent_PedestrianInjured,
    eBroadcastEvent_PedestrianDead,
    eBroadcastEvent_StealCar,
    eBroadcastEvent_Explosion,
    eBroadcastEvent_CarBurns,
};

decl_enum_strings(eBroadcastEvent);

enum eBroadcastEventSubject
{
    eBroadcastEventSubject_None,
    eBroadcastEventSubject_Pedestrian,
    eBroadcastEventSubject_Vehicle,
    eBroadcastEventSubject_Object
};

decl_enum_strings(eBroadcastEventSubject);

// Broadcast event data
struct BroadcastEvent
{
public:
    BroadcastEvent() = default;

public:
    eBroadcastEvent mEventType; 
    eBroadcastEventSubject mEventSubject;

    float mEventTimestamp; // time when event was created
    float mEventDurationTime; // how long event lives

    glm::vec2 mPosition; // position in the world where event did happen, meters

    GameObjectHandle mSubject; // object that was affected
    PedestrianHandle mCharacter; // character which causes event 
};

// Broadcast events manager
class BroadcastEventsManager final: public cxx::noncopyable
{
public:
    BroadcastEventsManager();

    void ClearEvents();
    void UpdateFrame();
    // broadcasting event
    void RegisterEvent(eBroadcastEvent eventType, GameObject* subject, Pedestrian* character, float durationTime);
    void RegisterEvent(eBroadcastEvent eventType, const glm::vec2& position, float durationTime);
    // Finds event with specific type but don't removes it from list
    bool PeekEvent(eBroadcastEvent eventType, BroadcastEvent& outputEventData) const;
    bool PeekClosestEvent(eBroadcastEvent eventType, const glm::vec2& position, BroadcastEvent& outputEventData) const;
    // Finds event with specific type and removes it from list
    bool GetEvent(eBroadcastEvent eventType, BroadcastEvent& outputEventData);
    bool GetClosestEvent(eBroadcastEvent eventType, const glm::vec2& position, BroadcastEvent& outputEventData);

private:
    std::vector<BroadcastEvent> mEventsList;
};

extern BroadcastEventsManager gBroadcastEvents;