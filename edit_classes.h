#ifndef _EDIT_CLASSES_H_
#define _EDIT_CLASSES_H_

#include <vector>
#include <iostream>
#include <irrlicht.h>
#include "reflection.h"
#include "CGUIWindow.h"

using namespace irr;

class FormField;
class Reflected_GUI_Edit_Form;
class geometry_scene;

namespace reflect
{
class TypeDescriptor;
class Member;
}


class EditWindow : public irr::gui::CGUIWindow
{
public:
    EditWindow(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene* g_scene_, s32 id,core::rect<s32> rect)
    : gui::CGUIWindow(env,parent,id,rect) ,g_scene(g_scene_) {}
    ~EditWindow();

    void Show(reflect::TypeDescriptor_Struct* typeDesc, void* obj);
    void refresh();
    virtual void click_OK();
    virtual bool OnEvent(const SEvent& event);
    virtual void write(void* obj);

    int OK_BUTTON_ID = 0;
    Reflected_GUI_Edit_Form* form = NULL;

    reflect::TypeDescriptor_Struct* my_typeDesc = NULL;
    void* my_obj=NULL;
    geometry_scene* g_scene=NULL;
};

enum
{
    CELL_TYPE_LABEL,
    CELL_TYPE_EDIT,
    CELL_TYPE_STATIC,
    CELL_TYPE_UNK
};

enum
{
    CELL_STATUS_NONE,
    CELL_STATUS_ACTIVE,
    CELL_STATUS_SELECT,
    CELL_STATUS_MODIFIED,
    CELL_STATUS_UNSELECT
};

class cell_background : public gui::IGUIElement
{
public:
    cell_background(gui::IGUIEnvironment* env, gui::IGUIElement* parent, FormField* field, int type, s32 id, core::rect<s32> rect);

    virtual void draw();
    virtual bool OnEvent(const SEvent& event);

    void setStatus(int status);

    int my_status = CELL_STATUS_NONE;
    bool can_select = true;
    bool selected = false;
    bool hovered = false;
    bool highlight = false;
	bool border = false;
    bool shiftDown = false;
    int cell_type = CELL_TYPE_UNK;
    FormField* my_field = NULL;
    gui::IGUIElement* my_element = NULL;

    int count = 0;
};

class Reflected_GUI_Edit_Form : public gui::IGUIElement
{
    struct column_info
    {
        int left_pos;
        int right_pos;
    };

public:

    Reflected_GUI_Edit_Form(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene* g_scene_, s32 id,core::rect<s32> rect)
    : gui::IGUIElement(gui::EGUIET_ELEMENT,env,parent,id,rect) , g_scene(g_scene_)
    {}
    ~Reflected_GUI_Edit_Form();

    virtual bool OnEvent(const SEvent& event);

    void addEditField(FormField*);
    void calculateSize();
    int ShowWidgets(int start_ID);
    int getTotalHeight();
    int getTotalWidth();
    int getEditFieldsCount();
    void read(void* obj);
    void write(void* obj);
    FormField* getField(int row, int column);
    FormField* getFieldFromId(int);
    FormField* getParentField(FormField*);

    core::rect<s32> getCell(int row, int column, int tab);
    core::rect<s32> getCell(int row, int column);
    cell_background* getCellPanel(int row, int column);

    void addStaticTextLabel(FormField* field, std::string text, int row, int tab, int ID);
    void addStaticTextCell(FormField* field, std::string text, int row, int column, int ID);
    void addTextEditCell(FormField* field, int row, int column, int ID);
    void addComboBoxCell(FormField* field, int row, int column, int ID);
    void addCheckBoxCell(FormField* field, int row, int column, int ID);
    void setColumns(std::vector<s32> width);

    gui::IGUIEnvironment* getEnvironment(){return this->Environment;}

    FormField* edit_fields = NULL;
    geometry_scene* g_scene = NULL;

    int line_height = 23;
    int text_height = 22;

    column_info* my_columns = NULL;
private: 
    int n_columns = 0;
    int n_rows = 0;
    
    //FormField** field_by_rc = NULL;
    cell_background** cell_by_rc = NULL;
};

//
//  Base for all form edit / static fields
//
enum
{
    FORM_FIELD_NO_BUTTON,
    FORM_FIELD_EXPAND_BUTTON,
    FORM_FIELD_EXPAND_LABEL,
    FORM_FIELD_LABEL,
    FORM_FIELD_BUTTON

};

enum
{
    FORM_TEXT_COLOR_NORMAL,
    FORM_TEXT_COLOR_GREY
};

class FormField
{
public:
    ~FormField(){if(next)delete next; next=NULL;}
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int row) = 0;
	virtual int addInlineWidget(Reflected_GUI_Edit_Form* win, int ID, int row) {return 0;}
    virtual void init(std::string text_, std::vector<int> tree_pos_, size_t offset_, int tab_, bool visible);
	virtual void initInline(std::string text_, std::vector<int> tree_pos_, size_t offset_, int column, bool visible);
	virtual void setText(std::string text_){this->text = text_;}
    virtual void writeValue(void* obj) = 0;
    virtual void readValue(void* obj) = 0;
    virtual void setVisible(bool visible) {bVisible = visible;}
    virtual bool isVisible() {return bVisible;}
    virtual int getHeight(){return (bVisible&&(!bInline))?1:0;}
    virtual int getNumIds(){return 2;}
    virtual int getButtonType() {return FORM_FIELD_NO_BUTTON;}
    virtual void clickButton() {}
    virtual void copy(void*, void*) {}
    virtual int getWidth(int column);

    void addStaticTextLabel(std::string text, int row, int tab, int ID);
    void addStaticTextCell(std::string text, int row, int column, int ID);
    void addTextEditCell(int row, int column, int ID);
    void addComboBoxCell(int row, int column, int ID);
    void addCheckBoxCell(int row, int column, int ID);

    virtual void onClick(int cell_type) { /*std::cout << "field clicked\n";*/ }

    virtual bool is_equal(void* a, void* b) = 0;
    gui::IGUIElement* getEditElement(int);
    gui::IGUIElement* getStaticElement(int);
    virtual void setActive(int) {};

    std::string text;
    FormField* next = NULL;
    Reflected_GUI_Edit_Form* owner = NULL;
    int my_ID = 0;
    int expand_button_ID = 0;

	bool bInline = false;
    bool bVisible = true;
	bool bBorder = false;
    bool bHighlight = false;
    bool bCanSelect = false;
    int text_color = FORM_TEXT_COLOR_NORMAL;
    size_t offset = 0;
    std::vector<int> tree_pos;
    int tab = 0;
    int my_row = 0;
	int my_column = 0;
};

class Icon_FormField : public FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int row);
    virtual void readValue(void* obj) {};
    virtual void writeValue(void* obj) {};
    virtual void setActive(int) {};
    virtual bool is_equal(void* a, void* b) { return false; }
    virtual int getHeight() { return 0; }
    virtual int getNumIds() { return 1; }
    virtual int getButtonType() { return FORM_FIELD_EXPAND_BUTTON; }
};

class ExButton_FormField : public FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int row);
    virtual void readValue(void* obj){};
    virtual void writeValue(void* obj){};
    virtual void setActive(int) {};
    virtual bool is_equal(void* a, void* b) {return false;}
    virtual int getHeight(){return 0;}
    virtual int getNumIds(){return 1;}
    virtual int getButtonType() {return FORM_FIELD_EXPAND_BUTTON;}
};

class Text_FormField : public FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos)=0;
	virtual int addInlineWidget(Reflected_GUI_Edit_Form* win, int ID, int row) {return 0;}
    virtual void readValue(void* obj){}
    virtual void writeValue(void* obj){}
    virtual void setActive(int) {}
    virtual bool is_equal(void* a, void* b) {return false;}
    virtual int getButtonType() = 0;
};

class Text_StaticField : public Text_FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos);
	virtual int addInlineWidget(Reflected_GUI_Edit_Form* win, int ID, int row);
    virtual int getButtonType() { return FORM_FIELD_NO_BUTTON; }
};

class Text_StaticLabel : public Text_StaticField
{
public:
    virtual int getButtonType() { return FORM_FIELD_LABEL; }
};

class Text_ExpandableLabel : public Text_StaticField
{
public:
    virtual int getButtonType() { return FORM_FIELD_EXPAND_LABEL; }
};

//
//  Integer
//

class Int_FormField : public FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos)=0;
	virtual int addInlineWidget(Reflected_GUI_Edit_Form* win, int ID, int row)=0;
    virtual void readValue(void* obj);
    virtual void writeValue(void* obj)=0;
    virtual void setActive(int) {};
    virtual void copy(void* obj, void* obj2){
        *get(obj) = *get(obj2);
    }
    virtual bool is_equal(void* a, void* b) {
        return *get(a) == *get(b);
    }
    int* get(void* obj_) {return (int*)((char*)obj_ + offset);}
};

class Int_EditField : public Int_FormField
{
public:
    virtual void setActive(int);
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos);
	virtual int addInlineWidget(Reflected_GUI_Edit_Form* win, int ID, int row);
    virtual void writeValue(void* obj);
};

class Int_StaticField : public Int_FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos);
	virtual int addInlineWidget(Reflected_GUI_Edit_Form* win, int ID, int row);
    virtual void writeValue(void* obj){}
};


class Color_FormField : public FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos)=0;
    virtual void readValue(void* obj)=0;
    virtual void writeValue(void* obj){}
    virtual int getHeight(){return 0;}
    //virtual int getNumIds(){return 1;}

    video::ITexture* texture = NULL;
    int ypos_=0;
    virtual bool is_equal(void* a, void* b) {
        return *get(a) == *get(b);
    }
    video::SColor* get(void* obj_) {return (video::SColor*)((char*)obj_ + offset);}
};

class Color_StaticField : public Color_FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos);
    virtual void readValue(void* obj);
};


//
// String
//


class String_FormField : public FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int row) = 0;
    virtual void readValue(void* obj);
    virtual void writeValue(void* obj) = 0;
    virtual void setActive(int) {};
    virtual int getButtonType() { return FORM_FIELD_LABEL; }
    virtual void copy(void* obj, void* obj2) {
        *get(obj) = *get(obj2);
    }
    virtual bool is_equal(void* a, void* b) {
        return *get(a) == *get(b);
    }
    std::string* get(void* obj_) { return (std::string*)((char*)obj_ + offset); }
};


class String_StaticField : public String_FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int row);
    virtual void writeValue(void* obj) {}
};

//
// Byte
//

class Byte_FormField : public FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos)=0;
    virtual void readValue(void* obj);
    virtual void writeValue(void* obj)=0;
    virtual void setActive(int) {};
    virtual void copy(void* obj, void* obj2){
        *get(obj) = *get(obj2);
    }
    virtual bool is_equal(void* a, void* b) {
        return *get(a) == *get(b);
    }
    char* get(void* obj_) {return (char*)((char*)obj_ + offset);}
};

class Byte_EditField : public Byte_FormField
{
public:
    virtual void setActive(int);
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos);
    virtual void writeValue(void* obj);
};

class Byte_StaticField : public Byte_FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos);
    virtual void writeValue(void* obj){}
};

//
// Texture
//

class Texture_FormField : public FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int row)=0;
    virtual void readValue(void* obj);
    virtual void writeValue(void* obj)=0;
    virtual void setActive(int) {};
    //virtual int getHeight(){return 2;}
    virtual int getButtonType() {return FORM_FIELD_BUTTON;}
    virtual void clickButton();
    virtual bool is_equal(void* a, void* b) {
        return *get(a) == *get(b);
    }
    video::ITexture** get(void* obj_) {return (video::ITexture**)((char*)obj_ + offset);}

    video::ITexture* my_texture = NULL;
};

class Texture_EditField : public Texture_FormField
{
public:
    virtual void setActive(int);
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos);
    virtual void writeValue(void* obj);
};

class Texture_StaticField : public Texture_FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos);
    virtual void writeValue(void* obj){}
};

//
// Floating Point
//

class Float_FormField : public FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos)=0;
	virtual int addInlineWidget(Reflected_GUI_Edit_Form* win, int ID, int row)=0;
    virtual void readValue(void* obj);
    virtual void writeValue(void* obj)=0;
    virtual void setActive(int) {};
    virtual void copy(void* obj, void* obj2){
        *get(obj) = *get(obj2);
    }
    virtual bool is_equal(void* a, void* b) {
        return *get(a) == *get(b);
    }
    f32* get(void* obj_) {return (f32*)((char*)obj_ + offset);}
};

class Float_EditField : public Float_FormField
{
public:
    virtual void setActive(int);
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos);
	virtual int addInlineWidget(Reflected_GUI_Edit_Form* win, int ID, int row);
    virtual void writeValue(void* obj);
};

class Float_StaticField : public Float_FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos);
	virtual int addInlineWidget(Reflected_GUI_Edit_Form* win, int ID, int row);
    virtual void writeValue(void* obj){}
};

//
//  ComboBox
//

class ComboBox_FormField : public FormField
{
public:
    void AddItem(std::string);

    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos)=0;
    virtual void writeValue(void* obj)=0;
    virtual void readValue(void* obj)=0;
    virtual void setActive(int);
    virtual void copy(void* obj, void* obj2){
        *get(obj) = *get(obj2);
    }
    virtual bool is_equal(void* a, void* b) {
        return *get(a) == *get(b);
    }

    std::vector<std::string> items;
    int* get(void* obj_) {return (int*)((char*)obj_ + offset);}
};

class ComboBox_EditField : public ComboBox_FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos);
    virtual void writeValue(void* obj);
    virtual void readValue(void* obj);
};

class ComboBox_StaticField : public ComboBox_FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos);
    virtual void writeValue(void* obj){}
    virtual void readValue(void* obj);
};

//
//  Boolean
//

class CheckBox_FormField : public FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos)=0;
    virtual void writeValue(void* obj)=0;
    virtual void readValue(void* obj)=0;
    virtual void copy(void* obj, void* obj2){
        *get(obj) = *get(obj2);
    }
    virtual bool is_equal(void* a, void* b) {
        return *get(a) == *get(b);
    }
    bool* get(void* obj_) {return (bool*)((char*)obj_ + offset);}
};

class CheckBox_EditField : public CheckBox_FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos);
    virtual void writeValue(void* obj);
    virtual void readValue(void* obj);
};

class CheckBox_StaticField : public CheckBox_FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos);
    virtual void writeValue(void* obj){}
    virtual void readValue(void* obj);
};

//
//  Vector2
//


class Vector2_FormField : public FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos)=0;
    virtual void writeValue(void* obj)=0;
    virtual void readValue(void* obj);
    virtual int getHeight(){return 2;}
    virtual int getNumIds(){return 2;}
    virtual bool is_equal(void* a, void* b) {
        return *get(a) == *get(b);
    }
    core::vector2df* get(void* obj_) {return (core::vector2df*)((char*)obj_ + offset);}
};

class Vector2_EditField : public Vector2_FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos);
    virtual void writeValue(void* obj);
};

class Vector2_StaticField : public Vector2_FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos);
    virtual void writeValue(void* obj){}
};

//
//  Vector3
//

class Vector3_FormField : public FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos)=0;
    virtual void writeValue(void* obj)=0;
    virtual void readValue(void* obj);
    virtual int getHeight(){return 3;}
    virtual bool is_equal(void* a, void* b) {
        return *get(a) == *get(b);
    }
    core::vector3df* get(void* obj_) {return (core::vector3df*)((char*)obj_ + offset);}
};

class Vector3_EditField : public Vector3_FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos);
    virtual void writeValue(void* obj);
};

class Vector3_StaticField : public Vector3_FormField
{
public:
    virtual int addWidget(Reflected_GUI_Edit_Form* win, int ID, int ypos);
    virtual void writeValue(void* obj){}
};


#endif
