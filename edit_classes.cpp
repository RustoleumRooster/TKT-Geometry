#include "edit_env.h"
#include <irrlicht.h>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "CameraPanel.h"
#include "CGUIWindow.h"
#include "csg_classes.h"
#include "utils.h"
#include "edit_classes.h"
#include "ex_gui_elements.h"
#include "CGUIEditBox2.h"
#include "edit_env.h"
//#include "Reflection.h"
#include "geometry_scene.h"
#include "texture_picker.h"
#include "node_properties.h"

extern IrrlichtDevice* device;
using namespace irr;
using namespace core;
using namespace gui;

namespace reflect
{

    void TypeDescriptor_StdVector::addFormWidget(Reflected_GUI_Edit_Form* win, TypeDescriptor_Struct* type_struct, std::vector<int> tree_0, size_t offset_base, bool bVisible, bool bEditable, int tab)
    {
       //Unused 
    }

    void TypeDescriptor_Struct::addFormWidget(Reflected_GUI_Edit_Form* win, TypeDescriptor_Struct* type_struct, std::vector<int> tree_0, size_t offset_base, bool bVisible, bool bEditable, int tab)
    {
        int m_i = tree_0.size() > 0 ? tree_0[tree_0.size()-1] : 0;

        //std::cout << std::string(4 * (tab + 1), ' ') << "struct ";
        //for (int i = 0; i < tree_0.size(); i++)
        //    std::cout << tree_0[i] << " ";
        //std::cout << " [" << offset_base << "]\n";


        if(type_struct != NULL)
        {
            ExButton_FormField* ff = new ExButton_FormField();

            ff->tree_pos = tree_0;
            ff->text = type_struct->members[m_i].expanded ? "-" : "+";
            ff->tab = (tab == -1 ? 0: tab);
            ff->setVisible(bVisible);
            win->addEditField(ff);
        }

        if(type_struct != NULL)
        {
            Text_ExpandableLabel* f = new Text_ExpandableLabel();
            //Text_StaticField* f = new Text_StaticField();

            f->setText(type_struct->members[m_i].name);
            f->tab = (tab == -1 ? 0: tab);
            f->setVisible(bVisible);
            f->tree_pos = tree_0;
            f->bCanSelect = true;

            win->addEditField(f);
        }

        if(type_struct == NULL) 
            for(int i=0;i<members.size();i++)
            {
                std::vector<int> tree{i};
                members[i].type->addFormWidget(win,this,tree,0, bVisible, bEditable && (members[i].flags & FLAG_NON_EDITABLE) == 0, tab+1);
            }
        else 
            for(int i=0;i<members.size();i++)
            {
                std::vector<int> tree = tree_0;
                tree.push_back(i);
                members[i].type->addFormWidget(win,this,tree,type_struct->members[m_i].offset + offset_base, type_struct->members[m_i].expanded && bVisible, bEditable && (members[i].flags & FLAG_NON_EDITABLE) == 0, tab+1);
            }
    }

    void TypeDescriptor_Texture::addFormWidget(Reflected_GUI_Edit_Form* win, TypeDescriptor_Struct* type_struct, std::vector<int> tree, size_t offset_base, bool bVisible, bool bEditable, int tab)
    {
        int m_i = tree[tree.size() - 1];
        std::string name = type_struct->members[m_i].name;
        size_t offset = type_struct->members[m_i].offset + offset_base;

        Texture_FormField* f;

        {
            if (bEditable)
            {
                f = new Texture_EditField();
                f->init(name, tree, offset, tab, bVisible);
                win->addEditField(f);
            }
            else
            {
                f = new Texture_StaticField();
                f->init(name, tree, offset, tab, bVisible);
                win->addEditField(f);
            }
        }
    }

    void TypeDescriptor_MultiOption::addFormWidget(Reflected_GUI_Edit_Form* win, TypeDescriptor_Struct* type_struct, std::vector<int> tree, size_t offset_base, bool bVisible, bool bEditable, int tab)
    {
        int m_i = tree[tree.size()-1];
        std::string name = type_struct->members[m_i].name;
        size_t offset = type_struct->members[m_i].offset+offset_base;

        ComboBox_FormField* f;

        if(bEditable)
            f = new ComboBox_EditField();
        else
            f = new ComboBox_StaticField();

        f->init(name, tree, offset, tab, bVisible);
        for(std::string str : labels)
            f->AddItem(str);


        win->addEditField(f);
    }

    void TypeDescriptor_U32::addFormWidget(Reflected_GUI_Edit_Form* win, TypeDescriptor_Struct* type_struct, std::vector<int> tree, size_t offset_base, bool bVisible, bool bEditable, int tab)
    {
        int m_i = tree[tree.size() - 1];
        std::string name = type_struct->members[m_i].name;
        size_t offset = type_struct->members[m_i].offset + offset_base;

        if (type_struct->members[m_i].flags & FLAG_UINT_WIDGET_POWER2)
        {
            Pwr2_FormField* f;

            if (bEditable)
                f = new Pwr2_EditField();
            else
                f = new Pwr2_StaticField();

            f->init(name, tree, offset, tab, bVisible);

            win->addEditField(f);
        }
        else
        {
            Int_FormField* f;

            if (bEditable)
                f = new Int_EditField();
            else
                f = new Int_StaticField();

            f->init(name, tree, offset, tab, bVisible);

            win->addEditField(f);
        }
    }

    void TypeDescriptor_Int::addFormWidget(Reflected_GUI_Edit_Form* win, TypeDescriptor_Struct* type_struct, std::vector<int> tree, size_t offset_base, bool bVisible, bool bEditable, int tab)
    {
        int m_i = tree[tree.size()-1];
        std::string name = type_struct->members[m_i].name;
        size_t offset = type_struct->members[m_i].offset+offset_base;

        if (type_struct->members[m_i].flags & FLAG_UINT_WIDGET_POWER2)
        {
            Pwr2_FormField* f;

            if (bEditable)
                f = new Pwr2_EditField();
            else
                f = new Pwr2_StaticField();

            f->init(name, tree, offset, tab, bVisible);

            win->addEditField(f);
        }
        else
        {
            Int_FormField* f;

            if (bEditable)
                f = new Int_EditField();
            else
                f = new Int_StaticField();

            f->init(name, tree, offset, tab, bVisible);

            win->addEditField(f);
        }
    }

    void TypeDescriptor_Byte::addFormWidget(Reflected_GUI_Edit_Form* win, TypeDescriptor_Struct* type_struct, std::vector<int> tree, size_t offset_base, bool bVisible, bool bEditable, int tab)
    {
        int m_i = tree[tree.size()-1];
        std::string name = type_struct->members[m_i].name;
        size_t offset = type_struct->members[m_i].offset+offset_base;

        Byte_FormField* f;

        if(bEditable)
            f = new Byte_EditField();
        else
            f = new Byte_StaticField();

        f->init(name, tree, offset, tab, bVisible);

        win->addEditField(f);
    }

    void TypeDescriptor_Bool::addFormWidget(Reflected_GUI_Edit_Form* win, TypeDescriptor_Struct* type_struct, std::vector<int> tree, size_t offset_base, bool bVisible, bool bEditable, int tab)
    {
        int m_i = tree[tree.size()-1];
        std::string name = type_struct->members[m_i].name;
        size_t offset = type_struct->members[m_i].offset+offset_base;

        CheckBox_FormField* f;

        if(bEditable)
            f = new CheckBox_EditField();
        else
            f = new CheckBox_StaticField();

        f->init(name, tree, offset, tab, bVisible);

        win->addEditField(f);
    }

    void TypeDescriptor_Float::addFormWidget(Reflected_GUI_Edit_Form* win, TypeDescriptor_Struct* type_struct, std::vector<int> tree, size_t offset_base, bool bVisible, bool bEditable, int tab)
    {
        int m_i = tree[tree.size()-1];
        std::string name = type_struct->members[m_i].name;
        size_t offset = type_struct->members[m_i].offset+offset_base;

        Float_FormField* f;

        if(bEditable)
            f = new Float_EditField();
        else
            f = new Float_StaticField();

        f->init(name, tree, offset, tab, bVisible);

        win->addEditField(f);
    }

	void color3::my_typeDesc::addFormWidget(Reflected_GUI_Edit_Form* win, TypeDescriptor_Struct* type_struct, std::vector<int> tree, size_t offset_base, bool bVisible, bool bEditable, int tab)
	{

		int m_i = tree[tree.size() - 1];
		std::string name = type_struct->members[m_i].name;
		size_t offset = type_struct->members[m_i].offset + offset_base;

		Color_FormField* cf;

		cf = new Color_StaticField();
		cf->init(name, tree, offset, tab, bVisible);
		win->addEditField(cf);

		TypeDescriptor_Struct::addFormWidget(win, type_struct, tree, offset_base, bVisible, bEditable, tab);
	}

    void uid_reference::my_typeDesc::addFormWidget(Reflected_GUI_Edit_Form* win, TypeDescriptor_Struct* type_struct, std::vector<int> tree, size_t offset_base, bool bVisible, bool bEditable, int tab)
    {
        int m_i = tree[tree.size() - 1];
        std::string name = type_struct->members[m_i].name;
        size_t offset = type_struct->members[m_i].offset + offset_base;

        UID_Reference_FormField* f;

        {
            if (bEditable)
            {
                f = new UID_Reference_EditField();
                f->init(name, tree, offset, tab, bVisible);
                win->addEditField(f);
            }
            else
            {
                f = new UID_Reference_StaticField();
                f->init(name, tree, offset, tab, bVisible);
                win->addEditField(f);
            }
        }
    }

} // namespace reflect


void EditWindow::Show(reflect::TypeDescriptor_Struct* typeDesc, void* obj)
{
    form = new Reflected_GUI_Edit_Form(Environment,this,g_scene,GUI_ID_REFLECTED_BASE,
                                            core::rect<s32>(core::vector2di(0,28),core::dimension2du(120,128)));

    typeDesc->addFormWidget(form,NULL,std::vector<int>{},0,true,true,-1);

    OK_BUTTON_ID = form->ShowWidgets(GUI_ID_REFLECTED_BASE+1);

    if(obj)
        form->read(obj);

    core::vector2di pos = getRelativePosition().UpperLeftCorner;
    this->DesiredRect = core::rect<s32>(pos,core::dimension2d<u32>(196,form->getTotalHeight()+86));
    this->recalculateAbsolutePosition(true);

    int ypos = form->getTotalHeight()+40;

    Environment->addButton(core::rect<s32>(core::vector2di(120,ypos),core::vector2di(180,ypos+28)),this,OK_BUTTON_ID,L"Ok");

    bringToFront(form);

    my_typeDesc = typeDesc;
    my_obj = obj;
}


void EditWindow::refresh()
{
    core::list<gui::IGUIElement*> child_list = this->getChildren();
    core::list<gui::IGUIElement*>::Iterator it = child_list.begin();

    for(;it != child_list.end();++it)
    {
        if(*it != this->getCloseButton())
            this->removeChild(*it);
    }

    form->drop();

    Show(my_typeDesc, my_obj);
}

void EditWindow::click_OK()
{
    this->remove();
}

void EditWindow::write(void* obj)
{
    if(form && obj)
        form->write(obj);
}

bool EditWindow::OnEvent(const SEvent& event)
{
    if(event.EventType == EET_GUI_EVENT)
    {
        s32 id = event.GUIEvent.Caller->getID();
        gui::IGUIEnvironment* env = device->getGUIEnvironment();

        switch(event.GUIEvent.EventType)
        {
            case EGET_BUTTON_CLICKED:
            {
                if(id==OK_BUTTON_ID)
                {
                    click_OK();
                    return true;
                }
                else
                {
                    FormField* field = form->getFieldFromId(id);
                    if(field)
                    {
                        if(field->getButtonType() == FORM_FIELD_EXPAND_BUTTON)
                        {
                           // ((ExButton_FormField*)field)->toggle(my_typeDesc);
                            refresh();
                        }
                        else if(field->getButtonType() ==  FORM_FIELD_BUTTON)
                        {
                            field->clickButton(id, NULL);
                        }
                    }
                break;
                }

            } break;
        }
    }

    return gui::CGUIWindow::OnEvent(event);
}

EditWindow::~EditWindow()
{
    if(form)
        form->drop();
}


//=======================================================
//  Reflected GUI Form
//
//

Reflected_GUI_Edit_Form::~Reflected_GUI_Edit_Form()
{
    if(this->edit_fields)
        delete this->edit_fields;
    this->edit_fields=NULL;

    if (my_columns)
        delete[] my_columns;

    if (cell_by_rc)
        delete[] cell_by_rc;

    cell_by_rc = NULL;

}

bool Reflected_GUI_Edit_Form::OnEvent(const SEvent& event)
{
    if (event.EventType == EET_GUI_EVENT)
    {
        s32 id = event.GUIEvent.Caller->getID();

        switch (event.GUIEvent.EventType)
        {
        case EGET_BUTTON_CLICKED:
            {
                FormField* field = getFieldFromId(id);

                if (field)
                {
                    field->clickButton(id);
                }
            }
        }
    }
	return gui::IGUIElement::OnEvent(event);
}

int Reflected_GUI_Edit_Form::getTotalHeight()
{
    int h=0;
    FormField* f = edit_fields;
    while(f)
    {
        h+=f->getHeight();
        f=f->next;
    }
    return h*line_height+1;
}

int Reflected_GUI_Edit_Form::getTotalWidth()
{
    return my_columns[n_columns - 1].right_pos;
}

int Reflected_GUI_Edit_Form::getEditFieldsCount()
{
    int i=0;
    FormField* f = edit_fields;
    while(f)
    {
        i+=1;
        f=f->next;
    }

    return i;
}

FormField* Reflected_GUI_Edit_Form::getParentField(FormField* field)
{
    if(field)
    {
        std::vector<int> pos = field->tree_pos;
        pos.pop_back();

        FormField* f = edit_fields;

        while(f)
        {
            if(f->tree_pos == pos)
                return f;
            f=f->next;
        }
    }
    return NULL;
}

core::rect<s32> Reflected_GUI_Edit_Form::getCell(int row, int column, int tab)
{
    int tabdist = tab == 1 ? 24 : tab * 16;

    if (column == 0)
        return core::rect<s32>(
            core::vector2di(my_columns[0].left_pos + tabdist, row * line_height),
            core::vector2di(my_columns[0].right_pos, (row+1) * line_height));

    return core::rect<s32>();
}

core::rect<s32> Reflected_GUI_Edit_Form::getCell(int row, int column)
{
    if (column < n_columns)
    {
        return core::rect<s32>(
            core::vector2di(my_columns[column].left_pos, row * line_height),
            core::vector2di(my_columns[column].right_pos, (row + 1) * line_height));
    }
    
    return core::rect<s32>();
}

cell_background* Reflected_GUI_Edit_Form::getCellPanel(int row, int column)
{
    if (column < n_columns && row < n_rows && cell_by_rc != NULL)
    {
        if(cell_by_rc[row * n_columns + column])
            return cell_by_rc[row * n_columns + column]->cell;
    }
    return NULL;
}

FormField* Reflected_GUI_Edit_Form::getFieldFromId(int ID_)
{
    FormField* f = edit_fields;
    while(f)
    {
        for(int i=0;i<f->getNumIds();i++)
        {
            if(f->my_ID+i == ID_)
                return f;
        }
        f=f->next;
    }
    return NULL;
}

int Reflected_GUI_Edit_Form::ShowWidgets(int start_ID)
{
    if (my_columns == NULL)
    {
        setColumns(std::vector<s32>{60, 60,60});
    }
	calculateSize();

    core::vector2di pos = getRelativePosition().UpperLeftCorner;
    this->DesiredRect = core::rect<s32>(pos, core::dimension2d<u32>(getRelativePosition().getWidth(), getTotalHeight()));
    this->recalculateAbsolutePosition(false);
  
    n_rows = 0;

    FormField* f = edit_fields;
    while (f)
    {
        n_rows += f->getHeight();
        f = f->next;
    }

    if (n_rows > 0)
    {
        cell_by_rc = new Form_Cell* [n_rows * n_columns];

        for (int i = 0; i < n_rows; i++)
            for (int j = 0; j < n_columns; j++)
                cell_by_rc[i * n_columns + j] = NULL;
    }

    int cur_ID = start_ID;
    int row = 0;

    f = edit_fields;
    while(f)
    {
		int inc = 0;

		if(f->bInline)
			f->addInlineWidget(this, cur_ID, row);
		else
			inc = f->addWidget(this, cur_ID, row);

        row+= inc;

        cur_ID+=f->getNumIds();
        f=f->next;
    }

    adjustCellDimensions();

    return cur_ID;
}

void Reflected_GUI_Edit_Form::setColumns(std::vector<s32> width)
{
    if (my_columns != NULL)
        delete[] my_columns;

    n_columns = width.size();

    my_columns = new column_info[n_columns];

    for (int i = 0; i < n_columns; i++)
    {
        my_columns[i].left_pos = 0;
        my_columns[i].right_pos = width[i];
    }
}

void Reflected_GUI_Edit_Form::calculateSize()
{
    if (n_columns > 0)
    {
        FormField* f = edit_fields;
        my_columns[0].left_pos = 0;
        my_columns[0].right_pos = 0;

        while (f)
        {
            my_columns[0].right_pos = std::max(my_columns[0].right_pos, f->getWidth(0));
            f = f->next;
        }

        if (n_columns > 1)
        {
            my_columns[1].left_pos += my_columns[0].right_pos+24;
            my_columns[1].right_pos += my_columns[0].right_pos+24;
        }

        if(n_columns > 2)
            for (int i = 2; i < n_columns; i++)
            {
                my_columns[i].left_pos += my_columns[i-1].right_pos;
                my_columns[i].right_pos += my_columns[i-1].right_pos;
				//std::cout<< my_columns[i].left_pos<<" - "<< my_columns[i].right_pos<<"\n";
            }

       // std::cout << "columns: " << my_columns[0].left_pos << ", " << column_left_end << " / " << my_columns[1].left_pos << ", " << column_right_end << "\n";
    }
}

void Reflected_GUI_Edit_Form::adjustCellDimensions()
{
    vector<int> desired_column_width;
    desired_column_width.assign(3,0);
    int desired_column_0_width_min_tab = 0;
    int total_width = this->AbsoluteRect.getWidth();
    int tab_dist = Environment->getSkin()->getSize(EGDS_WINDOW_BUTTON_WIDTH);

    for (int i = 0; i < n_rows; i++)
    {
        int n_cells = 0;
        for (int j = 0; j < n_columns; j++)
        {
            if (cell_by_rc[i * n_columns + j])
            {
                n_cells += 1;
            }
        }

        if (n_cells > 1)
        {
            desired_column_width[0] = std::max(desired_column_width[0],
                cell_by_rc[i * n_columns + 0]->desired_width +
                cell_by_rc[i * n_columns + 0]->desired_tab * tab_dist);

            desired_column_0_width_min_tab = std::max(desired_column_0_width_min_tab,
                cell_by_rc[i * n_columns + 0]->desired_width +
                cell_by_rc[i * n_columns + 0]->min_tab * tab_dist);

            for (int j = 1; j < n_columns; j++)
            {
                if (cell_by_rc[i * n_columns + j])
                {
                    desired_column_width[j] = std::max(desired_column_width[j],
                        cell_by_rc[i * n_columns + j]->desired_width);
                }
            }
        }
    }

    if (desired_column_width[0] + desired_column_width[1] + desired_column_width[2] >= total_width)
    {
        desired_column_width[0] = total_width - (desired_column_width[1] + desired_column_width[2]);
    }

    {
        for (int i = 0; i < n_rows; i++)
        {
            int begin_x = 0;
            for (int j = 0; j < n_columns; j++)
            {
                if (Form_Cell* cell = cell_by_rc[i * n_columns + j])
                {
                    int x = begin_x + cell->desired_tab * tab_dist;
                    int x1 = x + cell->desired_width;
                    if(j == 0 && cell_by_rc[i * n_columns + 1])
                        x1 = std::min(x1, begin_x + desired_column_width[j]);
                    int y = i * line_height;
                    int y1 = y + line_height;

                    cell->draw_rect = recti(x, y, x1, y1);
                    cell->createElements(this);

                    int z = 0;
                }
                begin_x += desired_column_width[j] + tab_dist/2;
            }
        }
    }
}

void Reflected_GUI_Edit_Form::addEditField(FormField* new_f)
{
    if(!this->edit_fields)
        this->edit_fields=new_f;
    else
    {
        FormField* f = this->edit_fields;
        while(f->next != NULL)
            f=f->next;
        f->next = new_f;
    }
}

void Reflected_GUI_Edit_Form::read(void* obj)
{
    FormField* f = edit_fields;
    while(f)
    {
        f->readValue(obj);
        f=f->next;
    }
}

void Reflected_GUI_Edit_Form::write(void* obj)
{
    FormField* f = edit_fields;
    while(f)
    {
        f->writeValue(obj);
        f=f->next;
    }
}

FormField* Reflected_GUI_Edit_Form::getField(int row, int column)
{
    return nullptr;
}

void Form_Cell::createElements(Reflected_GUI_Edit_Form* form)
{
    IGUIEnvironment* env = device->getGUIEnvironment();
    cell_background* cell_panel = new cell_background(env, form, field, -1, draw_rect);

    video::SColor color;

    switch (this->text_color)
    {
    case FORM_TEXT_COLOR_GREY:
        color = video::SColor(255, 100, 80, 120);
        break;
    default:
        color = video::SColor(255, 235, 235, 235);
    }

    switch (element_type)
    {
    case CELL_ELEMENT_TEXT:
        {
            gui::IGUIStaticText* stxt = env->addStaticText(text.c_str(),
                recti(0,0,draw_rect.getWidth(),draw_rect.getHeight()), false, false, cell_panel, ID);

            cell_panel->my_element = stxt;
            cell_panel->border = field->bBorder;
            cell_panel->highlight = field->bHighlight;
            stxt->setOverrideColor(color);

            break;
        }
    case CELL_ELEMENT_EDITBOX:
        {
            gui::IGUIEditBox* box = env->addEditBox(L"", recti(0, 0, draw_rect.getWidth(), draw_rect.getHeight()), false, cell_panel, ID);

            box->setDrawBackground(false);

            cell_panel->my_element = box;
            cell_panel->border = true;
            cell_panel->can_select = true;
            cell_panel->highlight = field->bHighlight;
            box->setOverrideColor(video::SColor(255, 128, 190, 100));

            break;
        }
    case CELL_ELEMENT_CHECKBOX:
        {
        gui::IGUICheckBox* box = env->addCheckBox(false, recti(0, 0, draw_rect.getWidth(), draw_rect.getHeight()), cell_panel, ID);

        box->setDrawBackground(false);

        cell_panel->my_element = box;
        cell_panel->can_select = true;
        break;
        }
    }

    cell_panel->drop();
}

void Reflected_GUI_Edit_Form::addCell(FormField* field, int element_type, std::wstring text, int min_tab, int tab, int row, int column, int ID)
{
    Form_Cell* cell = new Form_Cell;
    cell->field = field;
    cell->text = text;
    cell->element_type = element_type;
    cell->desired_width = std::max(24u, Environment->getSkin()->getFont()->getDimension(text.c_str()).Width);
    cell->min_tab = min_tab;
    cell->desired_tab = tab;
    cell->ID = ID;
    cell->text_color = field->text_color;
    cell_by_rc[row * n_columns + column] = cell;

   // if (field->text_color == FORM_TEXT_COLOR_GREY)
   //     stxt->setOverrideColor(video::SColor(255, 100, 80, 120));
}


//=====================================================
//  Widgets
//
//

void FormField::addStaticTextLabel(std::string text, int row, int tab, int ID)
{
    owner->addCell(this, CELL_ELEMENT_TEXT, wstring(text.begin(),text.end()), tab, tab, row, 0, ID);
}
void FormField::addTextLabelEdit(int row, int column, int tab, int ID)
{
    owner->addCell(this, CELL_ELEMENT_TEXT, wstring(text.begin(), text.end()), tab, tab, row, column, ID);
}
void FormField::addStaticTextCell(std::string text, int row, int column, int ID)
{
    owner->addCell(this, CELL_ELEMENT_TEXT, wstring(text.begin(), text.end()), tab, tab, row, column, ID);
}
   
void FormField::addTextEditCell(std::string text, int row, int column, int ID)
{
    owner->addCell(this, CELL_ELEMENT_EDITBOX, wstring(text.begin(), text.end()), 0, 0, row, column, ID);
}
void FormField::addComboBoxCell(int row, int column, int ID)
{
    //owner->addComboBoxCell(this, row, column, ID);
}
void FormField::addCheckBoxCell(int row, int column, int ID)
{
    owner->addCell(this, CELL_ELEMENT_CHECKBOX, L"", 0, 0, row, column, ID);
}

gui::IGUIElement* FormField::getEditElement(int n)
{
    return owner->getElementFromId(my_ID+n+1, true);
}

gui::IGUIElement* FormField::getStaticElement(int)
{
    return owner->getElementFromId(my_ID, true);
}

#define BEGIN_WIDGET() \
    owner = win; \
    my_ID = ID; \
    my_row = row; \
    gui::IGUIEnvironment* env = device->getGUIEnvironment(); \
    if(bVisible) {

#define END_WIDGET() \
    }\
    return this->getHeight();


void FormField::init(std::string text_, std::vector<int> tree_pos_, size_t offset_, int tab_, bool visible)
{
    setText(text_);
    tree_pos = tree_pos_;
    offset = offset_;
    tab = tab_;
    bVisible = visible;
}

void FormField::initInline(std::string text_, std::vector<int> tree_pos_, size_t offset_, int column, bool visible)
{
	setText(text_);
	tree_pos = tree_pos_;
	offset = offset_;
	my_column = column;
	bVisible = visible;
	bInline = true;
}

//DEPRECATED
int FormField::getWidth(int column)
{
    if(column == 0)
    {
        int tabdist = tab == 1 ? 20 : tab * 16;
        return tabdist + text.length() * 6 + 16;
    }
    else if (column > 0)
    {
        return 48;
    }

    return 0;
}

//
//  Icon
//

int Icon_FormField::addWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
    BEGIN_WIDGET()

    video::IVideoDriver* driver = owner->getEnvironment()->getVideoDriver();
    video::ITexture* texture = driver->getTexture("C:\\Projects\\TKT-dev\\folder_icon.png");

    if (texture)
    {
        int ypos = win->line_height * row;
        int tabdist = tab * env->getSkin()->getSize(EGDS_WINDOW_BUTTON_WIDTH);

        gui::IGUIImage* img = env->addImage(
            core::rect<s32>(core::vector2di(win->my_columns[0].left_pos - win->line_height + tabdist, ypos),
                core::vector2di(win->my_columns[0].left_pos + tabdist, ypos + win->line_height)),
            this->owner, my_ID, NULL, false);

        img->setImage(texture);
        //img->setScaleImage(true);

    }

    END_WIDGET()
}

//
//  Expand/Collapse Button
//

int ExButton_FormField::addWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
    BEGIN_WIDGET()

    std::wstring txt(text.begin(),text.end());

    int ypos = win->line_height * row;

    int tabdist = tab * env->getSkin()->getSize(EGDS_WINDOW_BUTTON_WIDTH);

    int box_size = env->getSkin()->getSize(EGDS_WINDOW_BUTTON_WIDTH);
    int x = win->my_columns[0].left_pos + tabdist - (box_size * 1.0);
    int y = ypos + (win->line_height - box_size) * 0.5;

    core::rect<s32> rect(x, y, x + box_size, y + box_size);

    Flat_Button* ex_button = new Flat_Button(env,win,ID,rect);
    ex_button->setText(txt.c_str());
    ex_button->drop();

/*
    gui::IGUIButton* button = env->addButton(core::rect<s32>(core::vector2di(win->my_columns[0].left_pos-20+tab*24,ypos+0),
                                           core::vector2di(win->my_columns[0].left_pos-4+tab*24,ypos+16)),
                                            win,ID,L"");*/
/*
    gui::IGUISkin *skin = env->getSkin();
    gui::IGUISpriteBank* sprites = skin->getSpriteBank();
    button->setSpriteBank(sprites);
    EGDI_
    button->setSprite(EGBS_BUTTON_UP, skin->getIcon(EGDI_EXPAND), skin->getColor(EGDC_WINDOW_SYMBOL));
    button->setSprite(EGBS_BUTTON_DOWN, skin->getIcon(EGDI_EXPAND), skin->getColor(EGDC_WINDOW_SYMBOL));*/
   //
    //gui::EGDI_EXPAND
   // skin->
   //core::rect<s32> r = win->getAbsolutePosition();
   // skin->drawIcon(win, EGDI_EXPAND, core::vector2di(win->my_columns[0].left_pos-12+tab*24,ypos+8));

    END_WIDGET()
}

//
//  String 
//

int String_StaticField::addWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
    BEGIN_WIDGET()

    addStaticTextLabel(text, row, tab, ID);

    END_WIDGET()
}

void String_FormField::readValue(void* obj)
{
    if (this->owner && obj && bVisible)
    {
        gui::IGUIElement* textbox = (gui::IGUIElement*)(owner->getElementFromId(bInline ? my_ID : my_ID, true));
        if (textbox)
        {
            std::wstringstream ss;
            ss << get(obj)->c_str();
            textbox->setText(ss.str().c_str());
        }
    }
}

void String_EditField::setActive(int status)
{
    if (bVisible) {
        cell_background* cell = owner->getCellPanel(my_row, 1);
        if (cell)
            cell->setStatus(status);
    }
}

int String_EditField::addWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
    BEGIN_WIDGET()

    addTextLabelEdit(row, 0, tab, ID);

    END_WIDGET()
}

int String_EditField::addInlineWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
    BEGIN_WIDGET()

    addTextEditCell("000.000", row, my_column, ID);

    END_WIDGET()
}

void String_EditField::writeValue(void* obj)
{
    if (this->owner && obj && bVisible)
    {
        gui::IGUIEditBox* editbox = (gui::IGUIEditBox*)(owner->getElementFromId(bInline ? my_ID : my_ID, true));
        if (editbox)
        {
            std::string val = std::string((core::string<char>(editbox->getText()).c_str()));
            reflect::TypeResolver<std::string>::get()->copy(get(obj), &val);
            std::string* str = (std::string*)obj;
            int a = 9;
            //*get(obj) = val;
        }
    }
}


//
//  Text Only
//

int Text_StaticField::addWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
    BEGIN_WIDGET()

    addStaticTextLabel(text, row, tab, ID);

    END_WIDGET()
}


int Text_StaticField::addInlineWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
	BEGIN_WIDGET()

	addStaticTextCell(text, row, my_column, ID);

	END_WIDGET()
}

//
//  Float
//

int Float_StaticField::addInlineWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
    BEGIN_WIDGET()

    addStaticTextCell("", row, my_column, ID);

    END_WIDGET()
}

int Float_StaticField::addWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
	BEGIN_WIDGET()

	addStaticTextLabel(text, row, tab, ID);
	addStaticTextCell("", row, 1, ID + 1);

	END_WIDGET()
}

void Float_EditField::setActive(int status)
{
    if (bVisible) {
        cell_background* cell = owner->getCellPanel(my_row, 1);
        if (cell)
            cell->setStatus(status);
    }
}

int Float_EditField::addWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
    BEGIN_WIDGET()

    addStaticTextLabel(text, row, tab, ID);
    addTextEditCell("-00000.000", row, 1, ID+1);

    END_WIDGET()
}


int Float_EditField::addInlineWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
	BEGIN_WIDGET()

	addTextEditCell("-00000.000", row, my_column, ID);

	END_WIDGET()
}

void Float_EditField::writeValue(void* obj)
{
    if(this->owner && obj && bVisible)
    {
        gui::IGUIEditBox* editbox =  (gui::IGUIEditBox*)(owner->getElementFromId(bInline ? my_ID : my_ID + 1,true));
        if(editbox)
        {
            f32 val = std::atof(core::string<char>(editbox->getText()).c_str());
            *get(obj) = val;

        }
    }
}

void Float_FormField::readValue(void* obj)
{
    if(this->owner && obj && bVisible)
    {
        gui::IGUIElement* editbox =  (gui::IGUIElement*)(owner->getElementFromId(bInline ? my_ID : my_ID + 1,true));
        if(editbox)
        {
            std::wstringstream ss;
            ss<<std::fixed << std::setprecision(3) << *get(obj);
            editbox->setText(ss.str().c_str());
        }
    }
}

//
//  Integer
//


void Int_EditField::setActive(int status)
{
    if(bVisible){

        cell_background* cell = owner->getCellPanel(my_row, 1);
        if (cell)
            cell->setStatus(status);
    }
}

int Int_EditField::addWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
    BEGIN_WIDGET()

    addStaticTextLabel(text, row, tab, ID);
    addTextEditCell("00000", row, 1, ID+1);

    END_WIDGET()
}

int Int_EditField::addInlineWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
	BEGIN_WIDGET()

	addTextEditCell("00000", row, my_column, ID);

	END_WIDGET()
}

int Int_StaticField::addWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
    BEGIN_WIDGET()

    addStaticTextLabel(text, row, tab, ID);
    addStaticTextCell("", row, 1, ID+1);

    END_WIDGET()
}

int Int_StaticField::addInlineWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
	BEGIN_WIDGET()

	addStaticTextCell("", row, my_column, ID);

	END_WIDGET()
}

void Int_FormField::readValue(void* obj)
{
    if(this->owner && obj && bVisible)
    {
        gui::IGUIElement* editbox =  (gui::IGUIElement*)(owner->getElementFromId(bInline ? my_ID : my_ID + 1,true));
        if(editbox)
        {
            std::wstringstream ss;
            ss<<*get(obj);
            editbox->setText(ss.str().c_str());
        }
    }
}

void Int_EditField::writeValue(void* obj)
{
    if(this->owner && obj && bVisible)
    {
        gui::IGUIEditBox* editbox =  (gui::IGUIEditBox*)(owner->getElementFromId(bInline ? my_ID : my_ID + 1,true));
        if(editbox)
        {
            int val = core::strtoul10(core::string<char>(editbox->getText()).c_str());
            *get(obj) = val;
        }
    }
}

//
//  Texture
//

void Texture_EditField::setActive(int status)
{
    //gui::IGUIEditBox* box = (gui::IGUIEditBox*)getEditElement();
    //box->setDrawBackground(b);
    if (bVisible)
    {
        cell_background* cell = owner->getCellPanel(my_row, 1);
        if (cell)
            cell->setStatus(status);
    }
}

int Texture_EditField::addWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
    BEGIN_WIDGET()

    addStaticTextLabel(text, row, tab, ID);
    env->addButton(win->getCell(row, 1),win,ID+1,L"Set Texture");

    END_WIDGET()
}

int Texture_StaticField::addWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
    BEGIN_WIDGET()

    addStaticTextLabel(text, row, tab, ID);
    addStaticTextCell("", row, 1, ID+1);

    END_WIDGET()
}

void Texture_FormField::readValue(void* obj)
{
    if(this->owner && obj && bVisible)
    {
        gui::IGUIElement* editbox =  (gui::IGUIElement*)(owner->getElementFromId(my_ID+1,true));
        if(editbox)
        {
            my_texture = *get(obj);

            if(my_texture != NULL)
            {
                editbox->setText(my_texture->name.c_str());
            }
            else
                editbox->setText(L"no texture");
        }
    }
}

bool Texture_FormField::clickButton(s32 id, reflected_tool_base* base)
{
    if(bVisible)
    {
        my_texture = TexturePicker_Tool::getCurrentTexture();

        gui::IGUIElement* editbox =  (gui::IGUIElement*)(owner->getElementFromId(my_ID+1,true));
        if(editbox)
        {
            if(my_texture != NULL)
            {
                editbox->setText(my_texture->name.c_str());
            }
            else
                editbox->setText(L"no texture");
        }
    }

    return false;
}

void Texture_EditField::writeValue(void* obj)
{
    if(this->owner && obj && bVisible)
    {
        *get(obj) = my_texture;
    }
}

//
// UID_Reference
//

void UID_Reference_EditField::setActive(int status)
{
    //gui::IGUIEditBox* box = (gui::IGUIEditBox*)getEditElement();
    //box->setDrawBackground(b);
    if (bVisible)
    {
        cell_background* cell = owner->getCellPanel(my_row, 1);
        if (cell)
            cell->setStatus(status);
    }
}

int UID_Reference_EditField::addWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
    BEGIN_WIDGET()

    addStaticTextLabel(text, row, tab, ID);
    env->addButton(win->getCell(row, 1), win, ID + 1, L"Choose");
    env->addButton(win->getCell(row, 2), win, ID + 2, L"Clear");

    addStaticTextLabel("0 selected", row + 1, tab + 2, ID + 3);

    END_WIDGET()
}

void UID_Reference_EditField::writeValue(void* obj)
{
    if (this->owner && obj && bVisible)
    {
        //Write from Vector into Object
        reflect::TypeDescriptor* td = reflect::TypeResolver<std::vector<u64>>::get();

        td->copy((char*)get(obj), &target_uid);
    }
}

void UID_Reference_FormField::readValue(void* obj)
{
    if (this->owner && obj && bVisible)
    {
        gui::IGUIElement* editbox = (gui::IGUIElement*)(owner->getElementFromId(my_ID + 1, true));
        if (editbox)
        {
            //Read from Object into Vector
            reflect::TypeDescriptor* td = reflect::TypeResolver<std::vector<u64>>::get();

            td->copy(&target_uid,(char*)get(obj));

            std::wstringstream ss;
            ss << target_uid.size();
            ss << " selected";

            gui::IGUIElement* editbox = (gui::IGUIElement*)(owner->getElementFromId(my_ID + 3, true));
            editbox->setText(ss.str().c_str());
        }
    }
}

int UID_Reference_FormField::getHeight()
{
    return 2;
}

bool UID_Reference_FormField::clickButton(s32 id, reflected_tool_base* base)
{
    if (this->owner && this->owner->g_scene && bVisible)
    {
        Node_Properties_Base* node_properties_base = (Node_Properties_Base*)base;

        if (id == my_ID + 1)
        {
            node_properties_base->choose_node_selection(this->tree_pos);
        }
        else if (id == my_ID + 2)
        {
            node_properties_base->clear_selection(this->tree_pos);
        }

        return true;
    }

    return false;
}

int UID_Reference_StaticField::addWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
    BEGIN_WIDGET()

    addStaticTextLabel(text, row, tab, ID);

    END_WIDGET()
}

//
// Power 2
//


void Pwr2_EditField::setActive(int status)
{
    if (bVisible) {

        cell_background* cell = owner->getCellPanel(my_row, 1);
        if (cell)
            cell->setStatus(status);
    }
}

int Pwr2_EditField::addWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
    BEGIN_WIDGET()

    addStaticTextLabel(text, row, tab, ID);

    addStaticTextCell("0", row, 1, ID + 1);

    core::recti r = win->getCell(row, 2);

    core::recti r0 = r;
    core::recti r1 = r;

    u32 midpoint = r.UpperLeftCorner.X + (r.LowerRightCorner.X - r.UpperLeftCorner.X) * 0.5;

    r0.LowerRightCorner.X = midpoint;
    r1.UpperLeftCorner.X = midpoint;

    env->addButton(r0, win, ID + 2, L"-");
    env->addButton(r1, win, ID + 3, L"+");

    END_WIDGET()
}

unsigned get_dimension_base2(unsigned dim, int n = 1)
{
    double log_width = log2(dim);
    u16 lwi = static_cast<u16>(floor(log_width));
    lwi += n;
    lwi = lwi > 1 ? lwi : 1;
    lwi = lwi > 10 ? 10 : lwi;

    return exp2(lwi);
}

void Pwr2_FormField::clickButton(s32 id)
{
    if (this->owner && this->owner->g_scene && bVisible)
    {
        if (id == my_ID + 2)
        {
            gui::IGUIElement* editbox = (gui::IGUIElement*)(owner->getElementFromId(bInline ? my_ID : my_ID + 1, true));
            if (editbox)
            {
                my_no = get_dimension_base2(my_no, -1);
                std::wstringstream ss;
                ss << my_no;
                editbox->setText(ss.str().c_str());
            }
        }
        else if (id == my_ID + 3)
        {
            gui::IGUIElement* editbox = (gui::IGUIElement*)(owner->getElementFromId(bInline ? my_ID : my_ID + 1, true));
            if (editbox)
            {
                my_no = get_dimension_base2(my_no, 1);
                std::wstringstream ss;
                ss << my_no;
                editbox->setText(ss.str().c_str());
            }
        }
    }
}

void Pwr2_FormField::readValue(void* obj)
{
    if (this->owner && obj && bVisible)
    {
        gui::IGUIElement* editbox = (gui::IGUIElement*)(owner->getElementFromId(bInline ? my_ID : my_ID + 1, true));
        if (editbox)
        {
            my_no = *get(obj);
            std::wstringstream ss;
            ss << my_no;
            editbox->setText(ss.str().c_str());
        }
    }
}

void Pwr2_EditField::writeValue(void* obj)
{
    if (this->owner && obj && bVisible)
    {
        {
            *get(obj) = my_no;
        }
    }
}

int Pwr2_FormField::getHeight()
{
    return 1;
}

int Pwr2_StaticField::addWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
    BEGIN_WIDGET()

        addStaticTextLabel(text, row, tab, ID);

    END_WIDGET()
}

//
//  Color
//

int Color_StaticField::addWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
    BEGIN_WIDGET()

    addStaticTextLabel(text, row, tab, ID);

    END_WIDGET()
}

void Color_StaticField::readValue(void* obj)
{
    video::IVideoDriver* driver = owner->getEnvironment()->getVideoDriver();
    gui::IGUIEnvironment* env = device->getGUIEnvironment();

    if(texture)
    {
        this->owner->getEnvironment()->getVideoDriver()->removeTexture(texture);
        this->texture = NULL;
    }

	/*
    FormField* f = this->owner->edit_fields;

    this->bWrite = true;
    while(f)
    {
        std::vector<int> pos = f->tree_pos;
        pos.pop_back();

        if(pos == this->tree_pos && !f->bWrite)
        {
            this->bWrite = false;
            std::cout<<f->text<<" is off\n";
            break;
        }

        f= f->next;
    }*/

    if(this->owner && obj && bVisible)
    {
        video::SColor col;
        col.setData(get(obj),video::ECF_A8R8G8B8);
        video::IImage* img = makeSolidColorImage(driver,col);
        texture = driver->addTexture(irr::io::path("color"), img);

        Reflected_GUI_Edit_Form* win = this->owner;
        if(texture)
        {
            gui::IGUIImage* img1;
            img1 = env->addImage(
                 core::rect<s32>(core::vector2di(win->my_columns[1].left_pos,ypos_+win->line_height*0-4),core::vector2di(win->my_columns[1].right_pos,ypos_+win->line_height*0+win->text_height-4)),
                                 this->owner,my_ID+1,NULL,false);
            img1->setImage(texture);
            img1->setScaleImage(true);

        }
        img->drop();
    }
}

//
//  Byte
//

void Byte_EditField::setActive(int status)
{
    if(bVisible) {
        cell_background* cell = owner->getCellPanel(my_row, 1);
        if (cell)
            cell->setStatus(status);
    }
}

int Byte_EditField::addWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
    BEGIN_WIDGET()

    addStaticTextLabel(text, row, tab, ID);
    addTextEditCell("00000", row, 1, ID+1);

    END_WIDGET()
}

int Byte_StaticField::addWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
    BEGIN_WIDGET()

    addStaticTextLabel(text, row, tab, ID);
    addStaticTextCell("", row, 1, ID+1);

    END_WIDGET()
}

void Byte_FormField::readValue(void* obj)
{
    if(this->owner && obj && bVisible)
    {
        gui::IGUIElement* editbox =  (gui::IGUIElement*)(owner->getElementFromId(my_ID+1,true));
        if(editbox)
        {
            std::wstringstream ss;
            u32 n=(u32)*get(obj);
            n = n & 0x000000FF;
            ss<<n;
            editbox->setText(ss.str().c_str());
        }
    }
}

void Byte_EditField::writeValue(void* obj)
{
    if(this->owner && obj && bVisible)
    {
        gui::IGUIEditBox* editbox =  (gui::IGUIEditBox*)(owner->getElementFromId(my_ID+1,true));
        if(editbox)
        {
            u32 val = core::strtoul10(core::string<char>(editbox->getText()).c_str());
            val = std::min((u32)255,val);
            *get(obj) = *(char*)&val;
        }
    }
}

//
//  ComboBox
//

void ComboBox_FormField::AddItem(std::string text)
{
    items.push_back(text);
}

int ComboBox_EditField::addWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
    BEGIN_WIDGET()

    addStaticTextLabel(text, row, tab, ID);
    gui::IGUIComboBox* box = env->addComboBox(win->getCell(row, 1),win,ID+1);

    for(int i=0;i<this->items.size();i++)
    {
        std::wstring s(items[i].begin(),items[i].end());
        box->addItem(s.c_str(),i);
    }

    END_WIDGET()
}

int ComboBox_StaticField::addWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
    BEGIN_WIDGET()

    addStaticTextLabel(text, row, tab, ID);
    addStaticTextCell("", row, 1, ID+1);

    END_WIDGET()
}

void ComboBox_EditField::readValue(void* obj)
{
    if(this->owner && obj && bVisible)
    {
        gui::IGUIComboBox* editbox =  (gui::IGUIComboBox*)(owner->getElementFromId(my_ID+1,true));
        if(editbox)
        {
            editbox->setSelected(*get(obj));
        }
    }
}

void ComboBox_StaticField::readValue(void* obj)
{
    if(this->owner && obj && bVisible)
    {/*
        gui::IGUIElement* editbox =  (gui::IGUIElement*)(owner->getElementFromId(my_ID+1,true));
        if(editbox)
        {
            std::wstring txt(items[*get(obj)].begin(),items[*get(obj)].end());
            editbox->setText(txt.c_str());
        }*/
    }
}

void ComboBox_EditField::writeValue(void* obj)
{
    if(this->owner && obj && bVisible)
    {
        gui::IGUIComboBox* box =  (gui::IGUIComboBox*)(owner->getElementFromId(my_ID+1,true));
        if(box)
        {
            int val = box->getSelected();
            *get(obj) = val;
        }
    }
}

void ComboBox_FormField::setActive(int status)
{
    if(bVisible)
    {
        gui::IGUIComboBox* box =  (gui::IGUIComboBox*)(owner->getElementFromId(my_ID+1,true));
        if(box && status == CELL_STATUS_NONE)
        {
            box->setSelected(-1);
        }

        cell_background* cell = owner->getCellPanel(my_row, 1);
        if (cell)
            cell->setStatus(status);
    }
}

//
//  Boolean
//

int CheckBox_EditField::addWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
    BEGIN_WIDGET()

    addStaticTextLabel(text, row, tab, ID);
    addCheckBoxCell(row, 1, ID + 1);

    END_WIDGET()
}

int CheckBox_StaticField::addWidget(Reflected_GUI_Edit_Form* win, int ID, int row)
{
    BEGIN_WIDGET()

    addStaticTextLabel(text, row, tab, ID);
    addStaticTextCell("", row, 1, ID+1);

    END_WIDGET()
}

void CheckBox_EditField::writeValue(void* obj)
{
    if(this->owner && obj && bVisible)
    {
        gui::IGUICheckBox* box =  (gui::IGUICheckBox*)(owner->getElementFromId(my_ID+1,true));
        if(box)
        {
            bool val = box->isChecked();
            *get(obj) = val;
        }
    }
}

void CheckBox_EditField::readValue(void* obj)
{
    if(this->owner && obj && bVisible)
    {
        gui::IGUICheckBox* editbox =  (gui::IGUICheckBox*)(owner->getElementFromId(my_ID+1,true));
        if(editbox)
        {
            editbox->setChecked(*get(obj));
        }
    }
}

void CheckBox_StaticField::readValue(void* obj)
{
    if(this->owner && obj && bVisible)
    {
        gui::IGUIElement* editbox =  (gui::IGUIElement*)(owner->getElementFromId(my_ID+1,true));
        if(editbox)
        {
            std::wstringstream ss;
            if(*get(obj) == true)
                ss<<"True";
            else
                ss<<"False";
            editbox->setText(ss.str().c_str());
        }
    }
}

//===========================================================
//  Cell Background
//

cell_background::cell_background(gui::IGUIEnvironment* env, gui::IGUIElement* parent, FormField* field, s32 id, core::rect<s32> rect)
    :gui::IGUIElement(gui::EGUIET_ELEMENT, env, parent, id, rect), my_field(field)
{  
}

cell_background::~cell_background()
{
}

bool cell_background::OnEvent(const SEvent& event)
{
    if (isEnabled())
    {
        if (event.EventType == EET_MOUSE_INPUT_EVENT && Parent)
        {
            if (event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP)
            {
                if (my_element && can_select)
                {
                    SEvent e;
                    e.EventType = EET_GUI_EVENT;
                    e.GUIEvent.Caller = my_element;

                    if (event.MouseInput.Shift)
                        e.GUIEvent.EventType = (gui::EGUI_EVENT_TYPE)GUI_BUTTON_SHIFT_CLICKED;
                    else
                        e.GUIEvent.EventType = gui::EGET_BUTTON_CLICKED;

                    Parent->OnEvent(e);
                }
                    
                return true;
            }
            else if (event.MouseInput.Event == EMIE_LMOUSE_DOUBLE_CLICK)
            {
                SEvent e;
                e.EventType = EET_GUI_EVENT;
                e.GUIEvent.Caller = my_element;
                e.GUIEvent.EventType = (gui::EGUI_EVENT_TYPE)GUI_BUTTON_DOUBLE_CLICKED;
                Parent->OnEvent(e);

                return true;
            }
            else if (event.MouseInput.Event == EMIE_RMOUSE_LEFT_UP)
            {
                if (my_element && can_select)
                {
                    SEvent e;
                    e.EventType = EET_GUI_EVENT;
                    e.GUIEvent.Caller = my_element;
                    e.GUIEvent.EventType = (gui::EGUI_EVENT_TYPE)GUI_BUTTON_RIGHT_CLICKED;

                    Parent->OnEvent(e);
                }

                return true;
            }
        }
        else if (event.EventType == EET_GUI_EVENT)
        {
            //if (can_select)
            {
                gui::IGUISkin* skin = Environment->getSkin();
                if (event.GUIEvent.EventType == gui::EGET_ELEMENT_HOVERED)
                {
                    //std::cout << "entered "<< count << "\n";
                    hovered = true;
                   // if (my_text)
                   //     my_text->setOverrideColor(skin->getColor(gui::EGDC_HIGH_LIGHT_TEXT));
                }
                else if (event.GUIEvent.EventType == gui::EGET_ELEMENT_LEFT)
                {
                    count = 0;
                    //std::cout << "left\n";
                    if(Environment->getRootGUIElement()->getElementFromPoint(core::vector2di(event.MouseInput.X,event.MouseInput.Y))
                        !=this)
                        hovered = false;
                    //if (my_text)
                    //    my_text->setOverrideColor(skin->getColor(gui::EGDC_BUTTON_TEXT));
                }
            }
        }
    }

    return gui::IGUIElement::OnEvent(event);
}

void cell_background::setStatus(int status)
{
    if (status == CELL_STATUS_SELECT)
        selected = true;
    else if (status == CELL_STATUS_UNSELECT)
        selected = false;
    else
        my_status = status;
}

void cell_background::setDesiredRect(core::recti r)
{
    DesiredRect = r;
}

void cell_background::draw()
{
    //count++;
    gui::IGUISkin* skin = Environment->getSkin();

    core::recti parent_rect = Parent->getAbsoluteClippingRect();

    if (hovered )//&& can_select)
    {
        skin->draw2DRectangle(this, video::SColor(255, 32, 32, 48), getAbsolutePosition(), &parent_rect);
    }
    else if (selected)
    {
        skin->draw2DRectangle(this, video::SColor(255, 32, 32, 48), getAbsolutePosition(), &parent_rect);
    }
    else
    {
        switch (my_status)
        {

        case CELL_STATUS_ACTIVE:
            skin->draw2DRectangle(this, skin->getColor(EGDC_EDITABLE), getAbsolutePosition(), &parent_rect);
            break;
        case CELL_STATUS_MODIFIED:
            skin->draw2DRectangle(this, video::SColor(255, 64, 24, 24), getAbsolutePosition(), &parent_rect);
            break;
        }
    }

    if (highlight)
    {
        core::rect<s32> r = AbsoluteRect;
        r.LowerRightCorner.Y += 1;
        r.LowerRightCorner.X += 1;

        skin->draw2DRectangle(this, video::SColor(255, 32, 48, 32), getAbsolutePosition(), &parent_rect);
    }
    if (border)
    {
        core::rect<s32> r = AbsoluteRect;
        r.LowerRightCorner.Y += 1;
        r.LowerRightCorner.X += 1;


        skin->draw3DSunkenPane(this, skin->getColor(EGDC_EDITABLE), false, false, r, &parent_rect);
    }

    gui::IGUIElement::draw();
}

