#pragma once
#ifndef _TKT_SET_H_
#define _TKT_SET_H_

#include <vector>

template <typename T>
class tkt_set
{
public:
    std::vector<T*> items;  // the full collection

    std::vector<T*> getSelected() const { return m_selected; }

    void setSelected(std::vector<T*> new_sel)
    {
        std::vector<T*> old_sel = m_selected;
        m_selected = new_sel;
        onSelectionChanged(old_sel, m_selected);
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

    virtual void onSelectionChanged(const std::vector<T*>& /*old_sel*/,
                                    const std::vector<T*>& /*new_sel*/) {}
    virtual ~tkt_set() = default;

private:
    std::vector<T*> m_selected;
};

#endif
