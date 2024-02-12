
#include <irrlicht.h>
#include <iostream>
#include "Reflection.h"
#include "GUI_tools.h"
#include "edit_classes.h"
#include "edit_env.h"
#include "scrollbar2.h"

using namespace irr;
using namespace gui;
extern IrrlichtDevice* device;


//===========================================================
//  Reflected Widget Edit Area
//

void Reflected_Widget_EditArea::show(bool editable, void* obj)
{

    int x_offset = 16;

    core::rect<s32> pr(0, 0, getRelativePosition().getWidth(), getMaxFormsHeight());
    edit_panel = new gui::IGUIElement(gui::EGUIET_ELEMENT, Environment, this, -1, pr);

    form = new Reflected_GUI_Edit_Form(Environment, edit_panel, g_scene, my_ID + 1,
        core::rect<s32>(core::vector2di(x_offset, 4), core::dimension2du(120, 128)));

	if(override_columns.size() > 0)
		form->setColumns(override_columns);

    if (!m_typeDesc)
    {
        m_typeDesc = my_base->getFlatTypeDescriptor();
    }

    my_base->write_attributes(m_typeDesc);

    scrollbar_ID = my_ID + 2;

    m_typeDesc->addFormWidget(form, NULL, std::vector<int>{}, 0, true, editable, 0);

    form->ShowWidgets(my_ID + 3);
 
    if (!temp_object)
    {
       // std::cout << "reflected widget allocating " << m_typeDesc->size << " bytes\n";
        temp_object = malloc(m_typeDesc->size);
        //std::cout << " @ " << temp_object << "\n";
       
    }

    if (obj)
    {
        my_base->serialize_flat_obj(temp_object);
       // m_typeDesc->dump(temp_object, 0);
    }

    {
        FormField* f = form->edit_fields;
        while (f)
        {
            my_base->init_member(m_typeDesc, f->tree_pos);
            f = f->next;
        }
    }

    f32 lr = (f32)getEditAreaHeight() / (f32)getFormsHeight();

    if (lr < 1.0)
    {
        my_scrollbar = new CGUIScrollBar2(false, Environment, edit_panel, scrollbar_ID, core::rect<s32>(0, 0, 16, getEditAreaHeight()));
        my_scrollbar->drop();

        my_scrollbar->setMax(getFormsHeight());
        my_scrollbar->setSmallStep(24);

        int old_offset = scroll_offset;
        
        my_scrollbar->setPos(scroll_offset);

        scroll_offset = 0;
        setEditAreaScrollPos(old_offset);
    }
    else
    {
        my_scrollbar = NULL;
    }

    FormField* f = form->edit_fields;
    while (f)
    {
        bool rw = m_typeDesc->getTreeNode(f->tree_pos)->readwrite;
        bool modified = m_typeDesc->getTreeNode(f->tree_pos)->modified;
        if (modified)
        {
            f->readValue(temp_object);
            f->setActive(CELL_STATUS_MODIFIED);
        }
        else if (rw)
        {
            f->readValue(temp_object);
            f->setActive(CELL_STATUS_ACTIVE);
        }
        else
        {
            f->setActive(CELL_STATUS_NONE);
        }
        f = f->next;
    }

    b_editable = editable;
}

void Reflected_Widget_EditArea::setEditAreaScrollPos(int pos)
{
    core::list<gui::IGUIElement*> child_list = edit_panel->getChildren();
    core::list<gui::IGUIElement*>::Iterator it = child_list.begin();

    int move_dist = scroll_offset - pos;
    scroll_offset -= move_dist;

    for (; it != child_list.end(); ++it)
    {
        if (*it != my_scrollbar)
        {
            (*it)->move(core::vector2di(0, (move_dist)));
        }
    }
}

void Reflected_Widget_EditArea::refresh()
{
    core::list<gui::IGUIElement*> child_list = this->getChildren();
    core::list<gui::IGUIElement*>::Iterator it = child_list.begin();

    for (; it != child_list.end(); ++it)
    {
        this->removeChild(*it);
    }

    if (this->temp_object)
    {
        form->write(temp_object);
    }
    form->drop();

    my_base->widget_closing(this);

    show(b_editable, NULL);
}

bool Reflected_Widget_EditArea::OnEvent(const SEvent& event)
{
    if (event.EventType == EET_MOUSE_INPUT_EVENT)
    {
        if (event.MouseInput.Event == EMIE_MOUSE_WHEEL)
        {
            if (my_scrollbar)
                my_scrollbar->OnEvent(event);
            return false;
        }
    }
    else if(event.EventType == EET_GUI_EVENT)
    {
        s32 id = event.GUIEvent.Caller->getID();

        switch (event.GUIEvent.EventType)
        {
            case EGET_BUTTON_CLICKED:
            {
                FormField* field = form->getFieldFromId(id);

                if (field)
                {
                    if (field->getButtonType() == FORM_FIELD_EXPAND_BUTTON)
                    {
                        //((ExButton_FormField*)field)->toggle(m_typeDesc);
                        my_base->toggle_expanded_struct(m_typeDesc, field->tree_pos);
                        refresh();
                        if (Parent)
                        {
                            SEvent e;
                            e.EventType = EET_USER_EVENT;
                            e.UserEvent.UserData1 = USER_EVENT_REFLECTED_FORM_REFRESHED;
                            return Parent->OnEvent(e);
                        }
                        return true;
                    }
                    else if (field->getButtonType() == FORM_FIELD_EXPAND_LABEL)
                    {

                        //((Text_ExpandableLabel*)field)->toggle(m_typeDesc);
                        my_base->toggle_expanded_struct(m_typeDesc, field->tree_pos);
                        refresh();
                        if (Parent)
                        {
                            SEvent e;
                            e.EventType = EET_USER_EVENT;
                            e.UserEvent.UserData1 = USER_EVENT_REFLECTED_FORM_REFRESHED;
                            return Parent->OnEvent(e);
                        }
                        return true;
                    }
                    else if (field->getButtonType() == FORM_FIELD_BUTTON)
                    {
                        field->clickButton();
                        return true;
                    }
                }
            }break;
            case EGET_COMBO_BOX_CHANGED:
            case EGET_CHECKBOX_CHANGED:
            case EGET_EDITBOX_CHANGED:
            {
                FormField* field = form->getFieldFromId(id);
                if (field)
                {
                    field->setActive(CELL_STATUS_MODIFIED);
                    
                    m_typeDesc->getTreeNode(field->tree_pos)->modified = true;

                    int f_c = 0;

                    FormField* f = form->edit_fields;
                    while (f)
                    {
                        f_c++;
                        f = f->next;
                    }
                    field->writeValue(temp_object);
                }
            } break;
            case EGET_ELEMENT_FOCUSED:
            {
                if (event.GUIEvent.Caller->getType() == gui::EGUIET_EDIT_BOX ||
                    event.GUIEvent.Caller->getType() == gui::EGUIET_COMBO_BOX)
                {

                    FormField* field = form->getFieldFromId(id);
                    if (field)
                        field->setActive(CELL_STATUS_SELECT);
                }
                if (event.GUIEvent.Element &&
                    (event.GUIEvent.Element->getType() == gui::EGUIET_EDIT_BOX))// ||
                    // (event.GUIEvent.Element->getType() == gui::EGUIET_COMBO_BOX)    )
                {
                    s32 lost_focus_id = event.GUIEvent.Element->getID();
                    //int form_i = getFormFromId(lost_focus_id);

                    FormField* field = form->getFieldFromId(lost_focus_id);

                    if (field)
                    {
                        field->setActive(CELL_STATUS_UNSELECT);

                        bool modified = m_typeDesc->getTreeNode(field->tree_pos)->modified;

                        if(modified)
                        {
                            while (field)
                            {
                                //note: this supports the "color" widget
                                field->readValue(temp_object);
                                field = form->getParentField(field);
                            }
                        }
                    }
                }
            } break;
            case EGET_SCROLL_BAR_CHANGED:
            {
                if (id == scrollbar_ID)
                {
                    setEditAreaScrollPos(my_scrollbar->getPos());
                    return true;
                }
            } break;
        }
    }

    return gui::IGUIElement::OnEvent(event);
}

void Reflected_Widget_EditArea::write()
{
    if (form && temp_object)
    {
        form->write(temp_object);

        my_base->deserialize_flat_obj(temp_object);
    }
}

void Reflected_Widget_EditArea::write_by_field()
{
    FormField* f = form->edit_fields;
    while (f)
    {
        my_base->deserialize_by_field(m_typeDesc, f->tree_pos, temp_object);
        f = f->next;
    }
    refresh();
}

void Reflected_Widget_EditArea::save_expanded_status()
{
    FormField* f = form->edit_fields;
    while (f)
    {
        if (f->getButtonType() == FORM_FIELD_EXPAND_BUTTON)
        {
            my_base->save_expanded_status(m_typeDesc, f->tree_pos);
        }

        f = f->next;
    }
}

void Reflected_Widget_EditArea::setColumns(std::vector<int> widths)
{
	override_columns = widths;
}

Reflected_Widget_EditArea::Reflected_Widget_EditArea(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene* g_scene_, reflected_tool_base* base_, s32 id, core::rect<s32> rect)
    : gui::IGUIElement(gui::EGUIET_ELEMENT, env, parent, id, rect), my_base(base_), g_scene(g_scene_), my_ID(id)
{
}

Reflected_Widget_EditArea::~Reflected_Widget_EditArea()
{
    //std::cout << "Out of scope (Reflected Edit Area)\n";

    my_base->widget_closing(this);

    if (m_typeDesc)
        m_typeDesc->suicide(temp_object);

    if (form)
        form->drop();

    if (temp_object)
    {
        free(temp_object);
    }
}


int Reflected_Widget_EditArea::getEditAreaHeight()
{
    return std::min(getFormsHeight(), getMaxFormsHeight());
}

int Reflected_Widget_EditArea::getFormsHeight()
{
    int y = 1 * 0 + 24;

    if (form)
        y += form->getTotalHeight();

    return y;
}

int  Reflected_Widget_EditArea::getMaxFormsHeight()
{
    return getRelativePosition().getHeight() - 50;
    //return 240;
}

void Reflected_Widget_EditArea::draw()
{
    core::rect<s32> r = getAbsolutePosition();
    r.LowerRightCorner.Y = r.UpperLeftCorner.Y + getEditAreaHeight();
    gui::IGUISkin* skin = Environment->getSkin();

    r.LowerRightCorner.X -= 1;
    skin->draw3DSunkenPane(this, skin->getColor(EGDC_3D_FACE), true, true, r);

    gui::IGUIElement::draw();
}


//===========================================================
//  Multi Tool Panel
//


multi_tool_panel::multi_tool_panel(gui::IGUIEnvironment* env, gui::IGUIElement* parent,s32 id,core::rect<s32> rect) :
    gui::IGUIElement(gui::EGUIET_WINDOW,env,parent,id,rect)
{
    clientRect = NULL;
}

void multi_tool_panel::resize(core::rect<s32> rect)
{
    DesiredRect = rect;
    updateAbsolutePosition();
    do_layout();
}

gui::IGUIElement* multi_tool_panel::getClientRect()
{
    return clientRect;
}

void multi_tool_panel::add_tool(tool_base* tool)
{

    for(int i=0;i<my_tools.size();i++)
    {
        if(tool->getID() == my_tools[i]->getID())
        {
            show_tool(i);
            return;
        }
    }

    my_tools.push_back(tool);

    do_layout();

}

void multi_tool_panel::remove_tool(int tool_no)
{
    std::vector<tool_base*> new_tools;

    for(int i=0;i<my_tools.size();i++)
        if(i != tool_no)
            new_tools.push_back(my_tools[i]);

    my_tools = new_tools;

    do_layout();
}

void multi_tool_panel::nameChange(tool_base* tool)
{
    for(int i=0;i<my_tools.size();i++)
    {
        if(my_tools[i] == tool)
        {
            if(i<my_headers.size())
                my_headers[i]->setText(tool->getName());
        }
    }
}

void multi_tool_panel::do_layout()
{
    core::list<gui::IGUIElement*> child_list = this->getChildren();

    core::list<gui::IGUIElement*>::Iterator it = child_list.begin();

    for(;it != child_list.end();++it)
    {
        //int id = (*it)->getID();
        //if( id != GUI_ID_TEXTURE_SCROLL_BAR )
            this->removeChild(*it);
    }

    this->clientRect = NULL; //we just removed it

    my_headers.clear();

    for(int i=0;i<my_tools.size(); i++)
    {
        core::rect<s32> header_rect(core::vector2di(0,24*i),core::vector2di(getRelativePosition().getWidth(),24*(i+1)));
        tool_header* header = new tool_header(Environment,this,ID+1+i,header_rect);
        header->tool_no=i;
        header->setText(my_tools[i]->getName());
        my_headers.push_back(header);
    }

    updateClientRect();

    if(my_tools.size() > 0)
    {
        my_tools[my_tools.size() -1 ]->show();
        my_headers[my_tools.size() -1 ]->can_select = false;
        my_headers[my_tools.size() - 1]->is_selected = true;
    }
}

void multi_tool_panel::draw()
{
    if (!IsVisible)
		return;

	gui::IGUISkin* skin = Environment->getSkin();
	video::IVideoDriver* driver = Environment->getVideoDriver();

    //skin->draw2DRectangle(this, video::SColor(255,80,80,80),
    //                     AbsoluteRect,
    //                     &AbsoluteClippingRect);
    skin->draw3DToolBar(this,
                         AbsoluteRect,
                         &AbsoluteClippingRect);
    IGUIElement::draw();
}

void multi_tool_panel::show_tool(int tool_no)
{
    if(tool_no != my_tools.size()-1)
    {
        std::vector<tool_base*> new_list;

        for(int i=0;i<my_tools.size();i++)
            if(i != tool_no)
                new_list.push_back(my_tools[i]);

        new_list.push_back(my_tools[tool_no]);
        my_tools = new_list;

        do_layout();
    }
}

bool multi_tool_panel::OnEvent(const SEvent& event)
{
    if (isEnabled())
	{
	    if(event.EventType == EET_GUI_EVENT)
        {
            if(event.GUIEvent.EventType == gui::EGET_BUTTON_CLICKED)
            {
                for(tool_header* header: my_headers)
                {
                    if(event.GUIEvent.Caller == header->CloseButton)
                    {
                     remove_tool(header->tool_no);
                     return true;
                    }
                }
                tool_header* th = (tool_header*)event.GUIEvent.Caller;
                show_tool(th->tool_no);

                return true;
            }
        }
        else if(event.EventType ==  EET_MOUSE_INPUT_EVENT)
        {
            switch(event.MouseInput.Event)
            {
                case EMIE_RMOUSE_PRESSED_DOWN:
                    {
                        //if (Environment->hasFocus(this))
                        if(!AbsoluteClippingRect.isPointInside( core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y ) ))
                        {
                            Environment->removeFocus(this);
                            return true;
                        }
                    }
            }
        }
	}

	return false;
}

void multi_tool_panel::updateClientRect()
{
    int nHeaders = my_tools.size();

    if(clientRect)
    {
        clientRect->remove();
    }

    core::vector2di v1 = AbsoluteRect.LowerRightCorner - AbsoluteRect.UpperLeftCorner;
    v1-=core::vector2di(1,1);

    clientRect = new gui::IGUIElement(gui::EGUIET_ELEMENT,Environment,this,-1,
                        core::rect<s32>(core::vector2di(1,nHeaders*24),v1));
    clientRect->drop();
}


//===========================================================
//  Tool Base
//


void tool_base::setName(std::wstring txt)
{
    name=txt;
    panel->nameChange(this);
}

void tool_base::initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_,multi_tool_panel* panel_ )
{
    name = name_;
    env=env_;
    g_scene= g_scene_;
    panel = panel_;
    my_ID = my_id;
}


//===========================================================
//  Tool Header
//


void tool_header::setText(std::wstring txt)
{
    if(my_text)
        my_text->remove();
    my_text = Environment->addStaticText(txt.c_str(),core::rect<s32>(core::vector2di(0+4,0 + 2),core::vector2di(200,24)),false,false,this,-1);
    //my_text->setOverrideColor(video::SColor(255,255,255,255));
}

tool_header::tool_header(gui::IGUIEnvironment* env, gui::IGUIElement* parent,s32 id,core::rect<s32> rect)
    :gui::IGUIElement(gui::EGUIET_ELEMENT,env,parent,id,rect)
{
    gui::IGUISkin* skin = Environment->getSkin();
    int buttonw = buttonw = skin->getSize(gui::EGDS_WINDOW_BUTTON_WIDTH);
    s32 posx = RelativeRect.getWidth() - buttonw - 4;

    CloseButton = Environment->addButton(core::rect<s32>(posx, 3, posx + buttonw, 3 + buttonw), this, -1,
		L"", skin ? skin->getDefaultText(gui::EGDT_WINDOW_CLOSE) : L"Close" );
	CloseButton->setSubElement(true);

	gui::IGUISpriteBank* sprites = skin->getSpriteBank();
	if ( sprites )
    {
        CloseButton->setSpriteBank(sprites);
		CloseButton->setSprite(gui::EGBS_BUTTON_UP, skin->getIcon(gui::EGDI_WINDOW_CLOSE), skin->getColor(gui::EGDC_WINDOW_SYMBOL));
		CloseButton->setSprite(gui::EGBS_BUTTON_DOWN, skin->getIcon(gui::EGDI_WINDOW_CLOSE), skin->getColor(gui::EGDC_WINDOW_SYMBOL));
    }
}

bool tool_header::OnEvent(const SEvent& event)
{
    if (isEnabled())
	{
		if(event.EventType == EET_MOUSE_INPUT_EVENT)
        {
            if(event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP)
            {
                SEvent e;
                e.EventType = EET_GUI_EVENT;
                e.GUIEvent.EventType = gui::EGET_BUTTON_CLICKED;
                e.GUIEvent.Caller = this;
                Parent->OnEvent(e);
                return true;
            }
		}
        else if(event.EventType == EET_GUI_EVENT)
        {
            if(can_select)
            {
                gui::IGUISkin* skin = Environment->getSkin();
                if(event.GUIEvent.EventType == gui::EGET_ELEMENT_HOVERED)
                {
                    hovered = true;
                    if(my_text)
                        my_text->setOverrideColor(skin->getColor(gui::EGDC_HIGH_LIGHT_TEXT));
                }
                else if(event.GUIEvent.EventType == gui::EGET_ELEMENT_LEFT)
                {
                    hovered = false;
                    if(my_text)
                        my_text->setOverrideColor(skin->getColor(gui::EGDC_BUTTON_TEXT));
                }
            }
        }
	}

	return gui::IGUIElement::OnEvent(event);
}

void tool_header::draw()
{
    if(hovered)
    {
        gui::IGUISkin* skin = Environment->getSkin();
        skin->draw2DRectangle(this,skin->getColor(gui::EGDC_3D_LIGHT),getAbsolutePosition());
    }
    else if (is_selected)
    {
        gui::IGUISkin* skin = Environment->getSkin();
        skin->draw2DRectangle(this, video::SColor(255,32,32,48), getAbsolutePosition());
    }
    gui::IGUIElement::draw();
}

//
//====================================================
// Reflected Tool Base
//

void reflected_tool_base::toggle_expanded_struct(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos)
{
    reflect::Member* m = flat_typeDescriptor->getTreeNode(tree_pos);

    if(m)
        m->expanded = !m->expanded;
}

//
//====================================================
// Simple Reflected Tool Base
//

reflect::TypeDescriptor_Struct* simple_reflected_tool_base::getFlatTypeDescriptor()
{
    return (reflect::TypeDescriptor_Struct*)m_typeDescriptor->get_flat_copy(getObj(),0);
}

void simple_reflected_tool_base::widget_closing(Reflected_Widget_EditArea* widget)
{
    widget->save_expanded_status();
}

void simple_reflected_tool_base::serialize_flat_obj(void* flat_object)
{
    void* ss = flat_object;
    m_typeDescriptor->serialize_flat(&ss, getObj());
}

void simple_reflected_tool_base::deserialize_flat_obj(void* flat_object)
{
    void* ss = flat_object;
    m_typeDescriptor->deserialize_flat(getObj(), &ss);
}

void simple_reflected_tool_base::init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos)
{
    reflect::Member* m = flat_typeDescriptor->getTreeNode(tree_pos);
    m->readwrite = true;
}

void simple_reflected_tool_base::save_expanded_status(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos)
{
    reflect::Member* m = flat_typeDescriptor->getTreeNode(tree_pos);
    reflect::Member* m2 = m_typeDescriptor->getTreeNode(tree_pos);

    if (m && m2)
        m2->expanded = m->expanded;
}

void simple_reflected_tool_base::toggle_expanded_struct(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos)
{
    reflected_tool_base::toggle_expanded_struct(flat_typeDescriptor, tree_pos);
}

//
//====================================================
// Simple Reflected Tree Tool Base
//


template<typename T>
void simple_reflected_tree_tool_base<T>::widget_closing(Reflected_Widget_EditArea* widget)
{
    m_struct.read_expanded(widget->m_typeDesc, std::vector<int>{}, 0);
}

template<typename T>
void simple_reflected_tree_tool_base<T>::toggle_expanded_struct(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos)
{
    reflected_tool_base::toggle_expanded_struct(flat_typeDescriptor, tree_pos);

    m_struct.read_expanded(flat_typeDescriptor, std::vector<int>{}, 0);
}

template<typename T>
void simple_reflected_tree_tool_base<T>::write_attributes(reflect::TypeDescriptor_Struct* flat_typeDescriptor)
{
    m_struct.write_attributes_recursive(flat_typeDescriptor, std::vector<int>{});
}

template<typename T>
void simple_reflected_tree_tool_base<T>::initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel* panel_)
{
    //std::cout << "initializing tree base\n";
    tool_base::initialize(name_, my_id, env_, g_scene_, panel_);
    m_typeDescriptor = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<T>::get();

   // std::cout << m_typeDescriptor << "\n";
}

//====================================================================
//declare which instantiations we are going to use, making the above 
//templated member functions available to the linker
//

template class simple_reflected_tree_tool_base<folder_tree_item>;