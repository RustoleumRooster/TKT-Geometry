
#include <irrlicht.h>
#include <iostream>
#include "Reflection.h"
#include "tree_struct.h"
#include "reflect_custom_types.h"
#include "file_open.h"
#include "GUI_tools.h"
#include "ex_gui_elements.h"
#include "edit_env.h"
#include "edit_classes.h"
#include "coreutil.h"

using namespace gui;

extern IrrlichtDevice* device;

File_Open_Base* File_Open_Tool::base = NULL;
multi_tool_panel* File_Open_Tool::panel = NULL;

REFLECT_CUSTOM_STRUCT_BEGIN(folder_tree_item)
    REFLECT_STRUCT_MEMBER(id)
    REFLECT_STRUCT_MEMBER(name)
    REFLECT_TREE_STRUCT_MEMBERS()
REFLECT_STRUCT_END()

DEFINE_TREE_FUNCTIONS(folder_tree_item)


void folder_tree_item::write_attributes(reflect::Member* m_struct)
{
    folder_tree_item::my_typeDesc* m_type = (folder_tree_item::my_typeDesc*)(m_struct->type);

    m_type->my_attributes.isDirectory = this->isDirectory;
    m_type->my_attributes.selected = this->selected;
    m_type->hasSubs = this->sub_classes.size() > 0;
}

void folder_tree_item::my_typeDesc::addFormWidget(Reflected_GUI_Edit_Form* win, TypeDescriptor_Struct* type_struct, std::vector<int> tree_0, size_t offset_base, bool bVisible, bool bEditable, int tab)
{
    
    TREE_STRUCT_ADD_FORMWIDGET_HEADER()

        if (type_struct != NULL)
        {
            size_t offset = type_struct->members[m_i].offset + offset_base;

            if (my_attributes.isDirectory)
            {
                Icon_FormField* ff = new Icon_FormField();
                ff->init("", tree_0, offset, tab, bVisible);
                win->addEditField(ff);
            }

            String_StaticField* f = new String_StaticField();
            f->setText(type_struct->members[m_i].name);
            f->init("                ", tree_0, offset + ALIGN_BYTES, tab, bVisible);
            f->bCanSelect = true;

            if (my_attributes.isDirectory)
            {
                f->text_color = FORM_TEXT_COLOR_GREY;
                
            }
            if (my_attributes.selected)
            {
                //f->bBorder = true;
                f->bHighlight = true;
                
            }
            win->addEditField(f);
        }

    TREE_STRUCT_ADD_FORMWIDGET_FOOTER()
}

//==============================================================
// File_Open_Widget
//
//

void File_Open_Widget::show()
{
    core::rect<s32> pr(0, 0, getRelativePosition().getWidth(), getRelativePosition().getHeight());
    edit_panel = new gui::IGUIElement(gui::EGUIET_ELEMENT, Environment, this, -1, pr);

    OK_BUTTON_ID = my_ID + 1;
    EDIT_BOX_ID = my_ID + 1;
    STATIC_TEXT_ID = EDIT_BOX_ID + 1;

    pr.UpperLeftCorner.Y += 24;

    my_widget = new Reflected_Widget_EditArea(Environment, edit_panel, g_scene, my_base, STATIC_TEXT_ID + 1, pr);
    my_widget->show(false, my_base->getObj());

    int ypos = my_widget->getEditAreaHeight() + 8 + 24;
    core::rect<s32> br = core::rect<s32>(getRelativePosition().getWidth() - 80, ypos, getRelativePosition().getWidth() - 8, ypos + 36);

    my_button = new Flat_Button(Environment, this, OK_BUTTON_ID, br);
    my_button->setText(L"Ok");
    my_button->drop();

    core::rect<s32> sr = core::rect<s32>(24, 4, getRelativePosition().getWidth(), 24);
    std::string str = my_base->getWorkingDirStr();
    std::wstring s(str.begin(), str.end());

    my_static_text = Environment->addStaticText(s.c_str(), sr, false, false, this, STATIC_TEXT_ID);

    core::rect<s32> er = core::rect<s32>(24, ypos, getRelativePosition().getWidth() - 100, ypos + 24);
    my_editbox = Environment->addEditBox(L"", er, true, edit_panel, EDIT_BOX_ID);


    edit_panel->drop();
    my_widget->drop();
}


void File_Open_Widget::onRefresh()
{
    if (my_button)
        my_button->remove();

    if (my_static_text)
        my_static_text->remove();

    if (my_editbox)
        my_editbox->remove();

    int ypos = my_widget->getEditAreaHeight() + 8 + 24;
    core::rect<s32> br = core::rect<s32>(getRelativePosition().getWidth() - 80, ypos, getRelativePosition().getWidth() - 8, ypos + 36);

    my_button = new Flat_Button(Environment, this, OK_BUTTON_ID, br);
    my_button->setText(L"Ok");

    core::rect<s32> sr = core::rect<s32>(24, 4, getRelativePosition().getWidth(), 24);
    std::string str = my_base->getWorkingDirStr();
    std::wstring s(str.begin(), str.end());

    my_static_text = Environment->addStaticText(s.c_str(), sr, false, false, this, STATIC_TEXT_ID);

    core::rect<s32> er = core::rect<s32>(24, ypos, getRelativePosition().getWidth() - 100, ypos + 24);
    my_editbox = Environment->addEditBox(L"", er, true, edit_panel, EDIT_BOX_ID);
}

void File_Open_Widget::refresh()
{
    if (my_widget)
    {
        my_widget->remove();
    }

    core::rect<s32> pr(0, 0, getRelativePosition().getWidth(), getRelativePosition().getHeight());
    pr.UpperLeftCorner.Y += 24;
    my_widget = new Reflected_Widget_EditArea(Environment, edit_panel, g_scene, my_base, my_ID + 2, pr);

    my_widget->show(false, my_base->getObj());
    my_widget->drop();

    onRefresh();
}

void File_Open_Widget::click_OK()
{
    //this->remove();
}

bool File_Open_Widget::OnEvent(const SEvent& event)
{
    if (event.EventType == EET_USER_EVENT)
    {
        if (event.UserEvent.UserData1 == USER_EVENT_REFLECTED_FORM_REFRESHED)
        {
            std::cout << "never gets called lol\n";
            onRefresh();
            return true;
        }
    }
    else if (event.EventType == EET_GUI_EVENT)
    {

        s32 id = event.GUIEvent.Caller->getID();

        if (event.GUIEvent.EventType == EGET_BUTTON_CLICKED ||
            event.GUIEvent.EventType == (gui::EGUI_EVENT_TYPE)GUI_BUTTON_DOUBLE_CLICKED)
        {
            if (event.GUIEvent.Caller == my_button)
            {
                my_base->click_OK();

                if (Parent)
                {
                    SEvent e;
                    e.EventType = EET_GUI_EVENT;
                    e.GUIEvent.Caller = this;
                    e.GUIEvent.EventType = (gui::EGUI_EVENT_TYPE)GUI_REFLECTED_FORM_CLOSED;
                    Parent->OnEvent(e);
                }  
                return true;
            }
            else
            {
                FormField* field =  my_widget->form->getFieldFromId(id);

                if (field)
                {
                    if (field->getButtonType() == FORM_FIELD_LABEL)
                    {
                        std::vector<int> leaf = field->tree_pos;
                        leaf.push_back(0); //folder_tree_item.id

                        reflect::Member* m = my_widget->m_typeDesc->getTreeNode(leaf);
                        size_t offset = my_widget->m_typeDesc->getTreeNodeOffset(leaf);

                        int sel;

                        m->type->copy(&sel, (char*)((char*)my_widget->temp_object) + offset);

                        if (event.GUIEvent.EventType == (gui::EGUI_EVENT_TYPE)GUI_BUTTON_DOUBLE_CLICKED)
                        {

                            my_base->select_file_dbl(sel);
                            refresh();
                        }
                        else
                        {
                            my_base->select_file(sel);
                            if (my_editbox)
                            {

                                my_editbox->setText(my_base->getFileBaseNameW().c_str());
                            }
                            my_widget->refresh();
                        }

                        return true;
                    }
                }
            }
        }
        else if( event.GUIEvent.EventType == EGET_EDITBOX_CHANGED)
        {
            if (event.GUIEvent.Caller == my_editbox)
            {
                core::stringw str(my_editbox->getText());
                io::path p(str.c_str());
                my_base->setFileNameOnly(p);
                my_base->setDirectoryName("");
            }
        }
    }

    return IGUIElement::OnEvent(event);
}

File_Open_Widget::File_Open_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene* g_scene_, File_Open_Base* base_, s32 id, core::rect<s32> rect)
    : gui::IGUIElement(gui::EGUIET_ELEMENT, env, parent, id, rect), my_base(base_), g_scene(g_scene_), my_ID(id)
{
    //MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
    //receiver->Register(this);
}


File_Open_Widget::~File_Open_Widget()
{
    //std::cout << "Out of scope (File Open Widget)\n";
    //MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
    //receiver->UnRegister(this);
}

//==============================================================
// File_Open_Window
//
//

File_Open_Window::File_Open_Window(gui::IGUIEnvironment* env, gui::IGUIElement* parent, File_Open_Base* base, geometry_scene* g_scene_, s32 id, core::rect<s32> rect)
    : gui::CGUIWindow(env, parent, id, rect), my_base(base)
{
    //MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
    //receiver->Register(this);
}

File_Open_Window::~File_Open_Window()
{
    //MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
    //receiver->UnRegister(this);

    my_base->window_closed();
}

void File_Open_Window::show()
{

}

bool File_Open_Window::OnEvent(const SEvent& event)
{
    if (event.EventType == EET_GUI_EVENT)
    {
        if (event.GUIEvent.EventType == (gui::EGUI_EVENT_TYPE)GUI_REFLECTED_FORM_CLOSED)
        {
            remove();
            return true;
        }
    }
    return gui::CGUIWindow::OnEvent(event);
}


//==============================================================
// File_Open_Base
//
//


File_Open_Base::File_Open_Base(std::wstring name, int my_id, gui::IGUIEnvironment* env, multi_tool_panel* panel)
    : simple_reflected_tree_tool_base(name, my_id, env, panel)
{
    FileSystem = env ? env->getFileSystem() : 0;
}

void File_Open_Base::show()
{
    if (restoreCWD)
        RestoreDirectory = FileSystem->getWorkingDirectory();
    if (StartDirectory != L"")
    {
        FileSystem->changeWorkingDirectoryTo(StartDirectory);
    }

    build_struct();
    
    File_Open_Window* win = new File_Open_Window(env, env->getRootGUIElement(), this, g_scene, -1, core::rect<s32>(500, 64, 500 + 400, 64 + 340));
    
    std::wstring s(windowText.begin(), windowText.end());
    win->setText(s.c_str());

    win->show();
    win->drop();

    File_Open_Widget* widget = new File_Open_Widget(env, win, g_scene, this, my_ID, win->getClientRect());

    widget->show();
    widget->drop();
}

void File_Open_Base::widget_closing(Reflected_Widget_EditArea*)
{
    
}
/*
reflect::TypeDescriptor_Struct* File_Open_Base::getFlatTypeDescriptor()
{
    reflect::TypeDescriptor_Struct* tD = (reflect::TypeDescriptor_Struct*)m_typeDescriptor->get_flat_copy(getObj(), 0);
    return tD;
}

void File_Open_Base::write_attributes(reflect::TypeDescriptor_Struct* flat_typeDescriptor)
{
    m_struct.write_attributes_recursive(flat_typeDescriptor, std::vector<int>{});
}*/
/*
void File_Open_Base::init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos)
{
}
*/

void File_Open_Base::pathToStringW(irr::core::stringw& result, const irr::io::path& p)
{
#ifndef _IRR_WCHAR_FILESYSTEM
    char* oldLocale = setlocale(LC_CTYPE, NULL);
    setlocale(LC_CTYPE, "");	// multibyteToWString is affected by LC_CTYPE. Filenames seem to need the system-locale.
    core::multibyteToWString(result, p);
    setlocale(LC_CTYPE, oldLocale);
#else
    result = p.c_str();
#endif
}

std::string File_Open_Base::getWorkingDirStr()
{
    return std::string(FileSystem->getWorkingDirectory().c_str());
}

void File_Open_Base::build_struct()
{
    FileList = FileSystem->createFileList();
    core::stringw s;

    m_struct.sub_classes.clear();

    if (FileList)
    {
        for (u32 i = 0; i < FileList->getFileCount(); ++i)
        {
            if (i > 0)
            {
                io::path p = FileList->getFileName(i);

                folder_tree_item f;
                f.name = std::string(p.c_str());
                f.id = i;
                f.isDirectory = FileList->isDirectory(i);
                if(f.isDirectory)
                    m_struct.sub_classes.push_back(f);
            }
        }
    }
}

void File_Open_Base::setFileName(const irr::io::path& name)
{
    FileName = name;
    pathToStringW(FileNameW, FileName);
    pathToStringW(FileBaseNameW, FileSystem->getFileBasename(name, true));
}

void File_Open_Base::setFileNameOnly(const irr::io::path& name)
{
    FileName = mergeFilename(FileSystem->getWorkingDirectory(),name);
    pathToStringW(FileNameW, FileName);
    pathToStringW(FileBaseNameW, FileSystem->getFileBasename(name, true));
}

void File_Open_Base::setDirectoryName(const irr::io::path& name)
{
    FileDirectory = name;
    FileDirectoryFlat = name;
    FileSystem->flattenFilename(FileDirectoryFlat);
    pathToStringW(FileDirectoryFlatW, FileDirectoryFlat);
}

void File_Open_Base::click_OK()
{
    if (FileDirectory != L"")
    {
        SEvent event;
        event.EventType = EET_USER_EVENT;
        event.UserEvent.UserData1 = USER_EVENT_DIRECTORY_SELECTED;
        (MyEventReceiver*)device->getEventReceiver()->OnEvent(event);

    }
    if (FileName != L"")
    {
        SEvent event;
        event.EventType = EET_USER_EVENT;
        event.UserEvent.UserData1 = USER_EVENT_FILE_SELECTED;
        (MyEventReceiver*)device->getEventReceiver()->OnEvent(event);
    }
}

void File_Open_Base::window_closed()
{
    SEvent event;
    event.EventType = EET_USER_EVENT;
    event.UserEvent.UserData1 = USER_EVENT_FILE_DIALOGUE_CLOSED;
    (MyEventReceiver*)device->getEventReceiver()->OnEvent(event);

    if (restoreCWD)
        FileSystem->changeWorkingDirectoryTo(RestoreDirectory);

    StartDirectory = L"";
}

void File_Open_Base::setFileBaseNameW(core::stringw str)
{
    FileBaseNameW = str;
}

void File_Open_Base::SetCurrentProjectPath(io::path p)
{
    current_project_path = p;
    project_path_loaded = true;
}

io::path File_Open_Base::GetCurrentProjectPath()
{
    return current_project_path;
}

core::stringw File_Open_Base::getFileBaseNameW()
{
    return FileBaseNameW;
}

void File_Open_Base::select_file(int sel)
{
    for (int i = 0; i < m_struct.sub_classes.size(); i++)
    {
        if (m_struct.sub_classes[i].id == sel)
        {
            m_struct.sub_classes[i].selected = true;

            if (FileList->isDirectory(sel))
                {
                    setFileName("");
                    setDirectoryName(FileList->getFullFileName(sel));
                }
            else
                {
                    setDirectoryName("");
                    setFileName(FileList->getFullFileName(sel));
                }
        }
        else
            m_struct.sub_classes[i].selected = false;
    }
}

void File_Open_Base::select_file_dbl(int sel)
{
    if (FileList)
    {
        if (FileList->isDirectory(sel))
        {
            FileSystem->changeWorkingDirectoryTo(FileList->getFileName(sel));
            build_struct();
            
        }
    }
}

/*
void File_Open_Base::initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel*)
{
    simple_reflected_tree_tool_base::initialize(name_, my_id, env_, g_scene_, NULL);

    FileSystem = env ? env->getFileSystem() : 0;

    //build_struct();
}*/

io::path File_Open_Tool::getSelectedFile()
{
    return base->FileName;
}

io::path File_Open_Tool::getSelectedDir()
{
    return base->FileDirectory;
}
