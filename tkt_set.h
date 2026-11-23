#pragma once
#ifndef _TKT_SET_H_
#define _TKT_SET_H_

#include <vector>
#include <cstddef>

template <typename T> class tkt_set;

//================================================================
// tkt_selection_listener<T>
//
// Observes a tkt_set's selection, e.g. a property editor. The set
// doesn't own its listeners; a listener removes itself when it goes
// away, and is told if the set goes away first.
//
template <typename T>
class tkt_selection_listener
{
public:
    //only called when the selection actually changed
    virtual void onSetSelectionChanged(tkt_set<T>* set) = 0;

    //the items' values may have changed (e.g. a node was dragged); re-read them
    virtual void onSetItemsChanged(tkt_set<T>* /*set*/) {}

    //the set is being destroyed; drop any pointer to it (don't call back into it)
    virtual void onSetDestroyed(tkt_set<T>* /*set*/) {}

    virtual ~tkt_selection_listener() = default;
};

//================================================================
// tkt_set<T>
//
// Selection state for a collection of T. Says nothing about where the
// items live or who owns them: geometry_scene's nodes are owned by the
// Irrlicht scene graph, a tkt_owning_set's by the set itself.
//
template <typename T>
class tkt_set
{
public:
    std::vector<T*> getSelected() const { return m_selected; }

    void setSelected(std::vector<T*> new_sel)
    {
        std::vector<T*> old_sel = m_selected;
        m_selected = new_sel;
        onSelectionChanged(old_sel, m_selected);

        if (old_sel != m_selected)
        {
            //copy: a listener may add/remove listeners while being notified
            std::vector<tkt_selection_listener<T>*> listeners = m_listeners;
            for (tkt_selection_listener<T>* l : listeners)
                l->onSetSelectionChanged(this);
        }
    }

    //call after changing items outside the property editor, so listeners re-read them
    void notifyItemsChanged()
    {
        std::vector<tkt_selection_listener<T>*> listeners = m_listeners;
        for (tkt_selection_listener<T>* l : listeners)
            l->onSetItemsChanged(this);
    }

    void addSelectionListener(tkt_selection_listener<T>* l)
    {
        for (tkt_selection_listener<T>* x : m_listeners)
            if (x == l)
                return;
        m_listeners.push_back(l);
    }

    void removeSelectionListener(tkt_selection_listener<T>* l)
    {
        for (size_t i = 0; i < m_listeners.size(); i++)
        {
            if (m_listeners[i] == l)
            {
                m_listeners.erase(m_listeners.begin() + i);
                return;
            }
        }
    }

    void setSelected_ShiftAdd(T* item)
    {
        std::vector<T*> new_sel;
        bool found = false;
        for (T* n : m_selected) {
            if (n == item) found = true;
            else new_sel.push_back(n);
        }
        if (!found) new_sel.push_back(item);
        setSelected(new_sel);
    }

    //subclass hook; unlike listeners, called on every setSelected
    virtual void onSelectionChanged(const std::vector<T*>& /*old_sel*/,
                                    const std::vector<T*>& /*new_sel*/) {}

    tkt_set() = default;
    tkt_set(const tkt_set&) = delete;            //listeners point at this set
    tkt_set& operator=(const tkt_set&) = delete;

    virtual ~tkt_set()
    {
        std::vector<tkt_selection_listener<T>*> listeners = m_listeners;
        m_listeners.clear();
        for (tkt_selection_listener<T>* l : listeners)
            l->onSetDestroyed(this);
    }

protected:
    //call before an item goes away, so the selection never holds a dangling pointer
    void dropFromSelection(T* item)
    {
        for (T* n : m_selected)
        {
            if (n == item)
            {
                std::vector<T*> new_sel;
                for (T* m : m_selected)
                    if (m != item)
                        new_sel.push_back(m);
                setSelected(new_sel);
                return;
            }
        }
    }

private:
    std::vector<T*> m_selected;
    std::vector<tkt_selection_listener<T>*> m_listeners;
};

//================================================================
// tkt_owned<T>
//
// An owning slot in a tkt_owning_set. Deliberately a bare pointer (no
// destructor or move semantics) so the reflection system can serialize it
// as a typeless reference and resize vectors of it; the set does the deleting.
// Converts to T*, so "for (T* item : set.items)" works.
//
template <typename T>
struct tkt_owned
{
    T* X = NULL;

    operator T*() const { return X; }
    T* operator->() const { return X; }
};

//================================================================
// tkt_owning_set<T>
//
// A tkt_set that owns its items. The subclass reflects "items"
// (REFLECT_STRUCT_MEMBER(items)) to serialize them; this needs a
// getPrimitiveDescriptor<tkt_owned<T>> specialization returning a
// TypeDescriptor_Typeless_Reference<T>, so the right subclass is created on load.
//
// T must have UID() for find().
//
template <typename T>
class tkt_owning_set : public tkt_set<T>
{
public:
    std::vector<tkt_owned<T>> items;

    tkt_owning_set() = default;
    tkt_owning_set(const tkt_owning_set&) = delete;
    tkt_owning_set& operator=(const tkt_owning_set&) = delete;

    ~tkt_owning_set() override
    {
        //no hooks or selection events here: the subclass is already destroyed
        for (tkt_owned<T>& ref : items)
            delete ref.X;
    }

    //takes ownership
    T* add(T* item)
    {
        items.push_back(tkt_owned<T>{ item });
        onItemAdded(item);
        return item;
    }

    //deletes the item; returns false if it isn't in the set
    bool remove(T* item)
    {
        for (size_t i = 0; i < items.size(); i++)
        {
            if (items[i].X == item)
            {
                this->dropFromSelection(item);
                onItemRemoved(item);
                items.erase(items.begin() + i);
                delete item;
                return true;
            }
        }
        return false;
    }

    //deletes every item
    void clear()
    {
        this->setSelected({});
        for (tkt_owned<T>& ref : items)
        {
            onItemRemoved(ref.X);
            delete ref.X;
        }
        items.clear();
    }

    template <typename ID>
    T* find(const ID& uid) const
    {
        for (const tkt_owned<T>& ref : items)
            if (ref.X && ref.X->UID() == uid)
                return ref.X;
        return NULL;
    }

    size_t size() const { return items.size(); }

    //called after an item is added, and before one is removed (while it still exists).
    //Items created by deserializing "items" don't go through add().
    virtual void onItemAdded(T* /*item*/) {}
    virtual void onItemRemoved(T* /*item*/) {}
};

#endif
