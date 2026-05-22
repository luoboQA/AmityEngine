#pragma once

namespace Core {

class Entity;

class Component
{
    friend class Entity;

public:
    virtual ~Component() = default;

    Entity* getOwner() const { return m_owner; }

    virtual void init() {}
    virtual void update(double dt) {}

protected:
    void setOwner(Entity* owner) { m_owner = owner; }
    Entity* m_owner{nullptr};
};

} // namespace Core
